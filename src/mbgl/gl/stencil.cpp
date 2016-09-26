#include <mbgl/gl/stencil.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/util/traits.hpp>

namespace mbgl {
namespace gl {

static_assert(Stencil::Never::func == GL_NEVER, "OpenGL enum mismatch");
static_assert(Stencil::Less::func == GL_LESS, "OpenGL enum mismatch");
static_assert(Stencil::Equal::func == GL_EQUAL, "OpenGL enum mismatch");
static_assert(Stencil::LessEqual::func == GL_LEQUAL, "OpenGL enum mismatch");
static_assert(Stencil::Greater::func == GL_GREATER, "OpenGL enum mismatch");
static_assert(Stencil::NotEqual::func == GL_NOTEQUAL, "OpenGL enum mismatch");
static_assert(Stencil::GreaterEqual::func == GL_GEQUAL, "OpenGL enum mismatch");
static_assert(Stencil::Always::func == GL_ALWAYS, "OpenGL enum mismatch");

static_assert(underlying_type(Stencil::Keep) == GL_KEEP, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::Zero) == GL_ZERO, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::Replace) == GL_REPLACE, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::Increment) == GL_INCR, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::IncrementWrap) == GL_INCR_WRAP, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::Decrement) == GL_DECR, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::DecrementWrap) == GL_DECR_WRAP, "OpenGL enum mismatch");
static_assert(underlying_type(Stencil::Invert) == GL_INVERT, "OpenGL enum mismatch");

} // namespace gl
} // namespace mbgl
