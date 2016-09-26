#include <mbgl/gl/depth.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/util/traits.hpp>

namespace mbgl {
namespace gl {

static_assert(underlying_type(Depth::Never) == GL_NEVER, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::Less) == GL_LESS, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::Equal) == GL_EQUAL, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::LessEqual) == GL_LEQUAL, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::Greater) == GL_GREATER, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::NotEqual) == GL_NOTEQUAL, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::GreaterEqual) == GL_GEQUAL, "OpenGL enum mismatch");
static_assert(underlying_type(Depth::Always) == GL_ALWAYS, "OpenGL enum mismatch");

} // namespace gl
} // namespace mbgl
