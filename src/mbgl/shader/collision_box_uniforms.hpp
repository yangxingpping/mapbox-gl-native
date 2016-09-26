#pragma once

#include <mbgl/shader/uniforms.hpp>

namespace mbgl {

MBGL_DEFINE_UNIFORM_SCALAR(float, u_scale);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_maxzoom);

struct CollisionBoxUniforms : gl::Uniforms<
    u_matrix,
    u_scale,
    u_zoom,
    u_maxzoom> {};

} // namespace mbgl
