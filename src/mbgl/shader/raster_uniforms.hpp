#pragma once

#include <mbgl/shader/uniforms.hpp>

namespace mbgl {

MBGL_DEFINE_UNIFORM_SCALAR(gl::TextureUnit, u_image0);
MBGL_DEFINE_UNIFORM_SCALAR(gl::TextureUnit, u_image1);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_opacity0);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_opacity1);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_buffer_scale);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_brightness_low);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_brightness_high);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_saturation_factor);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_contrast_factor);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_scale_parent);
MBGL_DEFINE_UNIFORM_VECTOR(float, 3, u_spin_weights);
MBGL_DEFINE_UNIFORM_VECTOR(float, 2, u_tl_parent);

struct RasterUniforms : gl::Uniforms<
    u_matrix,
    u_image0,
    u_image1,
    u_opacity0,
    u_opacity1,
    u_brightness_low,
    u_brightness_high,
    u_saturation_factor,
    u_contrast_factor,
    u_spin_weights,
    u_buffer_scale,
    u_scale_parent,
    u_tl_parent> {};

} // namespace mbgl
