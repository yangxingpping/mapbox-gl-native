#pragma once

#include <mbgl/shader/uniforms.hpp>
#include <mbgl/util/mat4.hpp>

#include <string>

namespace mbgl {

MBGL_DEFINE_UNIFORM_VECTOR(uint16_t, 2, u_world);
MBGL_DEFINE_UNIFORM_SCALAR(Color,       u_outline_color);
MBGL_DEFINE_UNIFORM_SCALAR(float,       u_scale_a);
MBGL_DEFINE_UNIFORM_SCALAR(float,       u_scale_b);
MBGL_DEFINE_UNIFORM_SCALAR(float,       u_tile_units_to_pixels);
MBGL_DEFINE_UNIFORM_VECTOR(float, 2,    u_pixel_coord_upper);
MBGL_DEFINE_UNIFORM_VECTOR(float, 2,    u_pixel_coord_lower);

struct FillColorUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_color,
    u_outline_color,
    u_world> {};

class SpriteAtlasPosition;
class UnwrappedTileID;
class TransformState;

namespace style {
template <class> class Faded;
} // namespace style

struct FillPatternUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_world,
    u_pattern_tl_a,
    u_pattern_br_a,
    u_pattern_tl_b,
    u_pattern_br_b,
    u_pattern_size_a,
    u_pattern_size_b,
    u_scale_a,
    u_scale_b,
    u_mix,
    u_image,
    u_pixel_coord_upper,
    u_pixel_coord_lower,
    u_tile_units_to_pixels> {
    static Values values(mat4 matrix,
                         float opacity,
                         std::array<uint16_t, 2> framebufferSize,
                         const SpriteAtlasPosition&,
                         const SpriteAtlasPosition&,
                         const style::Faded<std::string>&,
                         const UnwrappedTileID&,
                         const TransformState&);
};

} // namespace mbgl
