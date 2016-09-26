#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/circle_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/shader/shaders.hpp>
#include <mbgl/shader/circle_uniforms.hpp>
#include <mbgl/gl/context.hpp>

namespace mbgl {

using namespace style;

void Painter::renderCircle(PaintParameters& parameters,
                           CircleBucket& bucket,
                           const CircleLayer& layer,
                           const RenderTile& tile) {
    if (pass == RenderPass::Opaque) {
        return;
    }

    const CirclePaintProperties& properties = layer.impl->paint;

    context.draw({
        gl::Depth { gl::Depth::LessEqual, false, depthRangeForSublayer(0) },
        frame.mapMode == MapMode::Still
            ? stencilForClipping(tile.clip)
            : gl::Stencil::disabled(),
        colorForRenderPass(),
        parameters.shaders.circle,
        CircleUniforms::values(
            tile.translatedMatrix(properties.circleTranslate,
                                  properties.circleTranslateAnchor,
                                  state),
            properties.circleOpacity,
            properties.circleColor,
            properties.circleRadius,
            properties.circleBlur,
            properties.circlePitchScale == CirclePitchScaleType::Map,
            properties.circlePitchScale == CirclePitchScaleType::Map
                ? std::array<float, 2> {{
                    pixelsToGLUnits[0] * state.getAltitude(),
                    pixelsToGLUnits[1] * state.getAltitude()
                  }}
                : pixelsToGLUnits,
            frame.pixelRatio
        ),
        gl::Segmented<gl::Triangles>(
            *bucket.vertexBuffer,
            *bucket.indexBuffer,
            bucket.segments
        )
    });
}

} // namespace mbgl
