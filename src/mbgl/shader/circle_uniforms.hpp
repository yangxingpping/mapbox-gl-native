#pragma once

#include <mbgl/shader/uniforms.hpp>

namespace mbgl {

MBGL_DEFINE_UNIFORM_SCALAR(float, u_radius);
MBGL_DEFINE_UNIFORM_SCALAR(bool, u_scale_with_map);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_devicepixelratio);

struct CircleUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_color,
    u_radius,
    u_blur,
    u_scale_with_map,
    u_extrude_scale,
    u_devicepixelratio> {};

} // namespace mbgl
