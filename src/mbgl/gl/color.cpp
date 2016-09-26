#include <mbgl/gl/color.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/util/traits.hpp>

namespace mbgl {
namespace gl {

static_assert(underlying_type(Color::Zero) == GL_ZERO, "OpenGL enum mismatch");
static_assert(underlying_type(Color::One) == GL_ONE, "OpenGL enum mismatch");
static_assert(underlying_type(Color::SrcColor) == GL_SRC_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusSrcColor) == GL_ONE_MINUS_SRC_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::DstColor) == GL_DST_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusDstColor) == GL_ONE_MINUS_DST_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::SrcAlpha) == GL_SRC_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusSrcAlpha) == GL_ONE_MINUS_SRC_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::DstAlpha) == GL_DST_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusDstAlpha) == GL_ONE_MINUS_DST_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::ConstantColor) == GL_CONSTANT_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusConstantColor) == GL_ONE_MINUS_CONSTANT_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::ConstantAlpha) == GL_CONSTANT_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusConstantAlpha) == GL_ONE_MINUS_CONSTANT_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::SrcAlphaSaturate) == GL_SRC_ALPHA_SATURATE, "OpenGL enum mismatch");

static_assert(underlying_type(Color::Zero) == GL_ZERO, "OpenGL enum mismatch");
static_assert(underlying_type(Color::One) == GL_ONE, "OpenGL enum mismatch");
static_assert(underlying_type(Color::SrcColor) == GL_SRC_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusSrcColor) == GL_ONE_MINUS_SRC_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::DstColor) == GL_DST_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusDstColor) == GL_ONE_MINUS_DST_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::SrcAlpha) == GL_SRC_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusSrcAlpha) == GL_ONE_MINUS_SRC_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::DstAlpha) == GL_DST_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusDstAlpha) == GL_ONE_MINUS_DST_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::ConstantColor) == GL_CONSTANT_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusConstantColor) == GL_ONE_MINUS_CONSTANT_COLOR, "OpenGL enum mismatch");
static_assert(underlying_type(Color::ConstantAlpha) == GL_CONSTANT_ALPHA, "OpenGL enum mismatch");
static_assert(underlying_type(Color::OneMinusConstantAlpha) == GL_ONE_MINUS_CONSTANT_ALPHA, "OpenGL enum mismatch");

} // namespace gl
} // namespace mbgl
