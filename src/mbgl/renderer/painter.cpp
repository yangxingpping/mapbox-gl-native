#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tile.hpp>

#include <mbgl/style/source.hpp>
#include <mbgl/style/source_impl.hpp>

#include <mbgl/platform/log.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/debugging.hpp>

#include <mbgl/style/style.hpp>
#include <mbgl/style/layer_impl.hpp>

#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/custom_layer.hpp>
#include <mbgl/style/layers/custom_layer_impl.hpp>

#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/text/glyph_atlas.hpp>

#include <mbgl/shader/shaders.hpp>

#include <mbgl/algorithm/generate_clip_ids.hpp>
#include <mbgl/algorithm/generate_clip_ids_impl.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/mat3.hpp>
#include <mbgl/util/string.hpp>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <unordered_set>

namespace mbgl {

using namespace style;

Painter::Painter(const TransformState& state_)
    : state(state_),
      tileTriangleVertexBuffer(context.createVertexBuffer(std::vector<FillVertex> {{
            { 0,            0 },
            { util::EXTENT, 0 },
            { 0, util::EXTENT },
            { util::EXTENT, 0 },
            { 0, util::EXTENT },
            { util::EXTENT, util::EXTENT }
      }})),
      tileLineStripVertexBuffer(context.createVertexBuffer(std::vector<FillVertex> {{
            { 0, 0 },
            { util::EXTENT, 0 },
            { util::EXTENT, util::EXTENT },
            { 0, util::EXTENT },
            { 0, 0 }
      }})),
      rasterVertexBuffer(context.createVertexBuffer(std::vector<RasterVertex> {{
            { 0, 0, 0, 0 },
            { util::EXTENT, 0, 32767, 0 },
            { 0, util::EXTENT, 0, 32767 },
            { util::EXTENT, util::EXTENT, 32767, 32767 }
      }})) {
#ifndef NDEBUG
    gl::debugging::enable();
#endif

    shaders = std::make_unique<Shaders>(context);
#ifndef NDEBUG
    overdrawShaders = std::make_unique<Shaders>(context, gl::Shader::Overdraw);
#endif

    // Reset GL values
    context.setDirtyState();
}

Painter::~Painter() = default;

bool Painter::needsAnimation() const {
    return frameHistory.needsAnimation(util::DEFAULT_FADE_DURATION);
}

void Painter::cleanup() {
    context.performCleanup();
}

void Painter::render(const Style& style, const FrameData& frame_, SpriteAtlas& annotationSpriteAtlas) {
    if (frame.framebufferSize != frame_.framebufferSize) {
        context.viewport.setDefaultValue(
            { 0, 0, frame_.framebufferSize[0], frame_.framebufferSize[1] });
    }
    frame = frame_;

    PaintParameters parameters {
#ifndef NDEBUG
        paintMode() == PaintMode::Overdraw ? *overdrawShaders : *shaders
#else
        *shaders
#endif
    };

    glyphAtlas = style.glyphAtlas.get();
    spriteAtlas = style.spriteAtlas.get();
    lineAtlas = style.lineAtlas.get();

    RenderData renderData = style.getRenderData(frame.debugOptions);
    const std::vector<RenderItem>& order = renderData.order;
    const std::unordered_set<Source*>& sources = renderData.sources;

    // Update the default matrices to the current viewport dimensions.
    state.getProjMatrix(projMatrix);

    pixelsToGLUnits = {{ 2.0f  / state.getWidth(), -2.0f / state.getHeight() }};
    if (state.getViewportMode() == ViewportMode::FlippedY) {
        pixelsToGLUnits[1] *= -1;
    }

    frameHistory.record(frame.timePoint, state.getZoom(),
        frame.mapMode == MapMode::Continuous ? util::DEFAULT_FADE_DURATION : Milliseconds(0));

    // - UPLOAD PASS -------------------------------------------------------------------------------
    // Uploads all required buffers and images before we do any actual rendering.
    {
        MBGL_DEBUG_GROUP("upload");

        spriteAtlas->upload(context, 0);
        lineAtlas->upload(context, 0);
        glyphAtlas->upload(context, 0);
        frameHistory.upload(context, 0);
        annotationSpriteAtlas.upload(context, 0);

        for (const auto& item : order) {
            if (item.bucket && item.bucket->needsUpload()) {
                item.bucket->upload(context);
            }
        }
    }

    // - CLEAR -------------------------------------------------------------------------------------
    // Renders the backdrop of the OpenGL view. This also paints in areas where we don't have any
    // tiles whatsoever.
    {
        MBGL_DEBUG_GROUP("clear");
        context.bindFramebuffer.reset();
        context.viewport.reset();
        context.clear(paintMode() == PaintMode::Overdraw
                        ? Color::black()
                        : renderData.backgroundColor,
                      1.0f,
                      0);
    }

    // - CLIPPING MASKS ----------------------------------------------------------------------------
    // Draws the clipping masks to the stencil buffer.
    {
        MBGL_DEBUG_GROUP("clip");

        // Update all clipping IDs.
        algorithm::ClipIDGenerator generator;
        for (const auto& source : sources) {
            source->baseImpl->startRender(generator, projMatrix, state);
        }

        MBGL_DEBUG_GROUP("clipping masks");

        for (const auto& stencil : generator.getStencils()) {
            MBGL_DEBUG_GROUP(std::string{ "mask: " } + util::toString(stencil.first));
            renderClippingMask(stencil.first, stencil.second);
        }
    }

    // Actually render the layers
    if (debug::renderTree) { Log::Info(Event::Render, "{"); indent++; }

    // TODO: Correctly compute the number of layers recursively beforehand.
    depthRangeSize = 1 - (order.size() + 2) * numSublayers * depthEpsilon;

    // - OPAQUE PASS -------------------------------------------------------------------------------
    // Render everything top-to-bottom by using reverse iterators. Render opaque objects first.
    renderPass(parameters,
               RenderPass::Opaque,
               order.rbegin(), order.rend(),
               0, 1);

    // - TRANSLUCENT PASS --------------------------------------------------------------------------
    // Make a second pass, rendering translucent objects. This time, we render bottom-to-top.
    renderPass(parameters,
               RenderPass::Translucent,
               order.begin(), order.end(),
               static_cast<GLsizei>(order.size()) - 1, -1);

    if (debug::renderTree) { Log::Info(Event::Render, "}"); indent--; }

    // - DEBUG PASS --------------------------------------------------------------------------------
    // Renders debug overlays.
    {
        MBGL_DEBUG_GROUP("debug");

        // Finalize the rendering, e.g. by calling debug render calls per tile.
        // This guarantees that we have at least one function per tile called.
        // When only rendering layers via the stylesheet, it's possible that we don't
        // ever visit a tile during rendering.
        for (const auto& source : sources) {
            source->baseImpl->finishRender(*this);
        }
    }

    if (frame.contextMode == GLContextMode::Shared) {
        context.setDirtyState();
    }
}

template <class Iterator>
void Painter::renderPass(PaintParameters& parameters,
                         RenderPass pass_,
                         Iterator it, Iterator end,
                         uint32_t i, int8_t increment) {
    pass = pass_;

    MBGL_DEBUG_GROUP(pass == RenderPass::Opaque ? "opaque" : "translucent");

    if (debug::renderTree) {
        Log::Info(Event::Render, "%*s%s {", indent++ * 4, "",
                  pass == RenderPass::Opaque ? "opaque" : "translucent");
    }

    for (; it != end; ++it, i += increment) {
        currentLayer = i;

        const auto& item = *it;
        const Layer& layer = item.layer;

        if (!layer.baseImpl->hasRenderPass(pass))
            continue;

        if (layer.is<BackgroundLayer>()) {
            MBGL_DEBUG_GROUP("background");
            renderBackground(parameters, *layer.as<BackgroundLayer>());
        } else if (layer.is<CustomLayer>()) {
            MBGL_DEBUG_GROUP(layer.baseImpl->id + " - custom");
            context.resetState();
            layer.as<CustomLayer>()->impl->render(state);
            context.setDirtyState();
            context.bindFramebuffer.reset();
            context.viewport.reset();
        } else {
            MBGL_DEBUG_GROUP(layer.baseImpl->id + " - " + util::toString(item.tile->id));
            item.bucket->render(*this, parameters, layer, *item.tile);
        }
    }

    if (debug::renderTree) {
        Log::Info(Event::Render, "%*s%s", --indent * 4, "", "}");
    }
}

mat4 Painter::matrixForTile(const UnwrappedTileID& tileID) {
    mat4 matrix;
    state.matrixFor(matrix, tileID);
    matrix::multiply(matrix, projMatrix, matrix);
    return matrix;
}

Range<float> Painter::depthRangeForSublayer(int n) const {
    float nearDepth = ((1 + currentLayer) * numSublayers + n) * depthEpsilon;
    float farDepth = nearDepth + depthRangeSize;
    return { nearDepth, farDepth };
}

gl::Stencil Painter::stencilForClipping(const ClipID& id) const {
    return gl::Stencil {
        gl::Stencil::Equal { static_cast<uint32_t>(id.mask.to_ulong()) },
        static_cast<int32_t>(id.reference.to_ulong()),
        0,
        gl::Stencil::Keep,
        gl::Stencil::Keep,
        gl::Stencil::Replace
    };
}

gl::Color Painter::colorForRenderPass() const {
    if (paintMode() == PaintMode::Overdraw) {
        const float overdraw = 1.0f / 8.0f;
        return gl::Color {
            gl::Color::Add {
                gl::Color::ConstantColor,
                gl::Color::One
            },
            Color { overdraw, overdraw, overdraw, 0.0f },
            gl::Color::Mask { true, true, true, true }
        };
    } else if (pass == RenderPass::Translucent) {
        return gl::Color::alphaBlended();
    } else {
        return gl::Color::unblended();
    }
}

} // namespace mbgl
