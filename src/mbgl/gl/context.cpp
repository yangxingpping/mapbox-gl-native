#include <mbgl/gl/context.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/util/traits.hpp>
#include <mbgl/util/std.hpp>

#include <boost/functional/hash.hpp>

namespace mbgl {
namespace gl {

static_assert(underlying_type(DrawMode::Points) == GL_POINTS, "OpenGL type mismatch");
static_assert(underlying_type(DrawMode::Lines) == GL_LINES, "OpenGL type mismatch");
static_assert(underlying_type(DrawMode::LineLoop) == GL_LINE_LOOP, "OpenGL type mismatch");
static_assert(underlying_type(DrawMode::LineStrip) == GL_LINE_STRIP, "OpenGL type mismatch");
static_assert(underlying_type(DrawMode::Triangles) == GL_TRIANGLES, "OpenGL type mismatch");
static_assert(underlying_type(DrawMode::TriangleStrip) == GL_TRIANGLE_STRIP, "OpenGL type mismatch");
static_assert(underlying_type(DrawMode::TriangleFan) == GL_TRIANGLE_FAN, "OpenGL type mismatch");

static_assert(std::is_same<ProgramID, GLuint>::value, "OpenGL type mismatch");
static_assert(std::is_same<ShaderID, GLuint>::value, "OpenGL type mismatch");
static_assert(std::is_same<BufferID, GLuint>::value, "OpenGL type mismatch");
static_assert(std::is_same<TextureID, GLuint>::value, "OpenGL type mismatch");
static_assert(std::is_same<VertexArrayID, GLuint>::value, "OpenGL type mismatch");
static_assert(std::is_same<FramebufferID, GLuint>::value, "OpenGL type mismatch");
static_assert(std::is_same<RenderbufferID, GLuint>::value, "OpenGL type mismatch");

Context::~Context() {
    reset();
}

UniqueProgram Context::createProgram() {
    return UniqueProgram{ MBGL_CHECK_ERROR(glCreateProgram()), { this } };
}

UniqueShader Context::createVertexShader() {
    return UniqueShader{ MBGL_CHECK_ERROR(glCreateShader(GL_VERTEX_SHADER)), { this } };
}

UniqueShader Context::createFragmentShader() {
    return UniqueShader{ MBGL_CHECK_ERROR(glCreateShader(GL_FRAGMENT_SHADER)), { this } };
}

UniqueBuffer Context::createVertexBuffer(const void* data, std::size_t size) {
    BufferID id = 0;
    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
    UniqueBuffer result { std::move(id), { this } };
    vertexBuffer = result;
    MBGL_CHECK_ERROR(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
    return result;
}

UniqueBuffer Context::createIndexBuffer(const void* data, std::size_t size) {
    BufferID id = 0;
    MBGL_CHECK_ERROR(glGenBuffers(1, &id));
    UniqueBuffer result { std::move(id), { this } };
    elementBuffer = result;
    MBGL_CHECK_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
    return result;
}

UniqueTexture Context::createTexture() {
    if (pooledTextures.empty()) {
        pooledTextures.resize(TextureMax);
        MBGL_CHECK_ERROR(glGenTextures(TextureMax, pooledTextures.data()));
    }

    TextureID id = pooledTextures.back();
    pooledTextures.pop_back();
    return UniqueTexture{ std::move(id), { this } };
}

UniqueFramebuffer Context::createFramebuffer() {
    FramebufferID id = 0;
    MBGL_CHECK_ERROR(glGenFramebuffers(1, &id));
    return UniqueFramebuffer{ std::move(id), { this } };
}

UniqueTexture
Context::createTexture(uint16_t width, uint16_t height, const void* data, TextureUnit unit) {
    auto obj = createTexture();
    activeTexture = unit;
    texture[unit] = obj;
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    MBGL_CHECK_ERROR(
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
    return obj;
}

void Context::bindTexture(Texture& obj,
                          TextureUnit unit,
                          TextureFilter filter,
                          TextureMipMap mipmap) {
    if (filter != obj.filter || mipmap != obj.mipmap) {
        activeTexture = unit;
        texture[unit] = obj.texture;
        MBGL_CHECK_ERROR(glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            filter == TextureFilter::Linear
                ? (mipmap == TextureMipMap::Yes ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR)
                : (mipmap == TextureMipMap::Yes ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST)));
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                         filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST));
        obj.filter = filter;
        obj.mipmap = mipmap;
    } else if (texture[unit] != obj.texture) {
        // We are checking first to avoid setting the active texture without a subsequent
        // texture bind.
        activeTexture = unit;
        texture[unit] = obj.texture;
    }
}

void Context::reset() {
    std::copy(pooledTextures.begin(), pooledTextures.end(), std::back_inserter(abandonedTextures));
    pooledTextures.resize(0);
    performCleanup();
}

template <typename Fn>
void Context::applyStateFunction(Fn&& fn) {
    fn(stencilFunc);
    fn(stencilMask);
    fn(stencilTest);
    fn(stencilOp);
    fn(depthRange);
    fn(depthMask);
    fn(depthTest);
    fn(depthFunc);
    fn(blend);
    fn(blendFunc);
    fn(blendColor);
    fn(colorMask);
    fn(clearDepth);
    fn(clearColor);
    fn(clearStencil);
    fn(program);
    fn(pointSize);
    fn(lineWidth);
    fn(activeTexture);
    fn(bindFramebuffer);
    fn(viewport);
#if not MBGL_USE_GLES2
    fn(pixelZoom);
    fn(rasterPos);
#endif // MBGL_USE_GLES2
    for (auto& tex : texture) {
        fn(tex);
    }
    fn(vertexBuffer);
    fn(elementBuffer);
    fn(vertexArrayObject);
}

void Context::resetState() {
    applyStateFunction([](auto& state) { state.reset(); });
}

void Context::setDirtyState() {
    applyStateFunction([](auto& state) { state.setDirty(); });
}

void Context::clear(optional<mbgl::Color> color,
                    optional<float> depth,
                    optional<int32_t> stencil) {
    GLbitfield mask = 0;

    if (color) {
        mask |= GL_COLOR_BUFFER_BIT;
        clearColor = *color;
        colorMask = { true, true, true, true };
    }

    if (depth) {
        mask |= GL_DEPTH_BUFFER_BIT;
        clearDepth = *depth;
        depthMask = true;
    }

    if (stencil) {
        mask |= GL_STENCIL_BUFFER_BIT;
        clearStencil = *stencil;
        stencilMask = 0xFF;
    }

    MBGL_CHECK_ERROR(glClear(mask));
}

DrawMode Context::operator()(const Points& points) {
    pointSize = points.pointSize;
    return DrawMode::Points;
}

DrawMode Context::operator()(const Lines& lines) {
    lineWidth = lines.lineWidth;
    return DrawMode::Lines;
}

DrawMode Context::operator()(const LineStrip& lineStrip) {
    lineWidth = lineStrip.lineWidth;
    return DrawMode::LineStrip;
}

DrawMode Context::operator()(const Triangles&) {
    return DrawMode::Triangles;
}

DrawMode Context::operator()(const TriangleStrip&) {
    return DrawMode::TriangleStrip;
}

std::size_t Context::VertexArrayObjectHash::operator()(const VertexArrayObjectKey& key) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, std::get<0>(key));
    boost::hash_combine(seed, std::get<1>(key));
    boost::hash_combine(seed, std::get<2>(key));
    boost::hash_combine(seed, std::get<3>(key));
    return seed;
}

void Context::draw(const Drawable& drawable) {
    DrawMode mode = apply_visitor([&] (auto m) { return (*this)(m); }, drawable.mode);

    if (drawable.depth.func == Depth::Always && !drawable.depth.mask) {
        depthTest = false;
    } else {
        depthTest = true;
        depthFunc = drawable.depth.func;
        depthMask = drawable.depth.mask;
        depthRange = drawable.depth.range;
    }

    if (drawable.stencil.test.is<Stencil::Always>() && !drawable.stencil.mask) {
        stencilTest = false;
    } else {
        stencilTest = true;
        stencilMask = drawable.stencil.mask;
        stencilOp = { drawable.stencil.fail, drawable.stencil.depthFail, drawable.stencil.pass };
        apply_visitor([&] (const auto& test) {
            stencilFunc = { test.func, drawable.stencil.ref, test.mask };
        }, drawable.stencil.test);
    }

    if (drawable.color.blendFunction.is<Color::Replace>()) {
        blend = false;
    } else {
        blend = true;
        blendColor = drawable.color.blendColor;
        apply_visitor([&] (const auto& blendFunction) {
            // TODO: blendEquation = blendFunction.equation;
            blendFunc = { blendFunction.srcFactor, blendFunction.dstFactor };
        }, drawable.color.blendFunction);
    }

    colorMask = drawable.color.mask;

    program = drawable.program;

    drawable.bindUniforms();

    for (const auto& segment : drawable.segments) {
        auto needAttributeBindings = [&] () {
            if (!gl::GenVertexArrays || !gl::BindVertexArray) {
                return true;
            }

            VertexArrayObjectKey vaoKey {
                drawable.program,
                drawable.vertexBuffer,
                drawable.indexBuffer,
                segment.vertexOffset
            };

            VertexArrayObjectMap::iterator it = vaos.find(vaoKey);
            if (it != vaos.end()) {
                MBGL_CHECK_ERROR(gl::BindVertexArray(it->second));
                return false;
            }

            VertexArrayID id = 0;
            MBGL_CHECK_ERROR(gl::GenVertexArrays(1, &id));
            MBGL_CHECK_ERROR(gl::BindVertexArray(id));
            vaos.emplace(vaoKey, UniqueVertexArray(std::move(id), { this }));

            return true;
        };

        if (needAttributeBindings()) {
            vertexBuffer = drawable.vertexBuffer;
            elementBuffer = drawable.indexBuffer;

            for (const auto& binding : drawable.attributeBindings) {
                MBGL_CHECK_ERROR(glEnableVertexAttribArray(binding.location));
                MBGL_CHECK_ERROR(glVertexAttribPointer(
                    binding.location,
                    binding.count,
                    static_cast<GLenum>(binding.type),
                    GL_FALSE,
                    drawable.vertexSize,
                    reinterpret_cast<GLvoid*>(binding.offset + (drawable.vertexSize * segment.vertexOffset))));
            }
        }

        if (drawable.indexBuffer) {
            MBGL_CHECK_ERROR(glDrawElements(
                static_cast<GLenum>(mode),
                drawable.primitiveSize / sizeof(uint16_t) * segment.primitiveLength,
                GL_UNSIGNED_SHORT,
                reinterpret_cast<GLvoid*>(drawable.primitiveSize * segment.primitiveOffset)));
        } else {
            MBGL_CHECK_ERROR(glDrawArrays(
                static_cast<GLenum>(mode),
                segment.vertexOffset,
                segment.vertexLength));
        }
    }
}

void Context::performCleanup() {
    for (auto id : abandonedPrograms) {
        if (program == id) {
            program.setDirty();
        }
        mbgl::util::erase_if(vaos, [&] (const auto& kv) {
            return std::get<0>(kv.first) == id;
        });
        MBGL_CHECK_ERROR(glDeleteProgram(id));
    }
    abandonedPrograms.clear();

    for (auto id : abandonedShaders) {
        MBGL_CHECK_ERROR(glDeleteShader(id));
    }
    abandonedShaders.clear();

    if (!abandonedBuffers.empty()) {
        for (const auto id : abandonedBuffers) {
            if (vertexBuffer == id) {
                vertexBuffer.setDirty();
            } else if (elementBuffer == id) {
                elementBuffer.setDirty();
            }
            mbgl::util::erase_if(vaos, [&] (const auto& kv) {
                return std::get<1>(kv.first) == id
                    || std::get<2>(kv.first) == id;
            });
        }
        MBGL_CHECK_ERROR(glDeleteBuffers(int(abandonedBuffers.size()), abandonedBuffers.data()));
        abandonedBuffers.clear();
    }

    if (!abandonedTextures.empty()) {
        for (const auto id : abandonedTextures) {
            if (activeTexture == id) {
                activeTexture.setDirty();
            }
        }
        MBGL_CHECK_ERROR(glDeleteTextures(int(abandonedTextures.size()), abandonedTextures.data()));
        abandonedTextures.clear();
    }

    if (!abandonedVertexArrays.empty()) {
        for (const auto id : abandonedVertexArrays) {
            if (vertexArrayObject == id) {
                vertexArrayObject.setDirty();
            }
        }
        MBGL_CHECK_ERROR(gl::DeleteVertexArrays(int(abandonedVertexArrays.size()),
                                                abandonedVertexArrays.data()));
        abandonedVertexArrays.clear();
    }

    if (!abandonedFramebuffers.empty()) {
        for (const auto id : abandonedFramebuffers) {
            if (bindFramebuffer == id) {
                bindFramebuffer.setDirty();
            }
        }
        MBGL_CHECK_ERROR(
            glDeleteFramebuffers(int(abandonedFramebuffers.size()), abandonedFramebuffers.data()));
        abandonedFramebuffers.clear();
    }
}

} // namespace gl
} // namespace mbgl
