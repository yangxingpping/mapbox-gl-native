#pragma once

#include <mbgl/shader/uniforms.hpp>

#include <array>

namespace mbgl {

namespace style {
class SymbolPropertyValues;
} // namespace style

class RenderTile;
class TransformState;

MBGL_DEFINE_UNIFORM_VECTOR(float, 2, u_texsize);
MBGL_DEFINE_UNIFORM_SCALAR(bool, u_rotate_with_map);
MBGL_DEFINE_UNIFORM_SCALAR(bool, u_pitch_with_map);
MBGL_DEFINE_UNIFORM_SCALAR(gl::TextureUnit, u_texture);
MBGL_DEFINE_UNIFORM_SCALAR(gl::TextureUnit, u_fadetexture);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_buffer);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_gamma);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_aspect_ratio);

struct SymbolIconUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_extrude_scale,
    u_texsize,
    u_zoom,
    u_rotate_with_map,
    u_texture,
    u_fadetexture>
{
    static Values values(const style::SymbolPropertyValues&,
                         const std::array<uint16_t, 2>& texsize,
                         const std::array<float, 2>& pixelsToGLUnits,
                         const RenderTile&,
                         const TransformState&);
};

struct SymbolSDFUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_extrude_scale,
    u_texsize,
    u_zoom,
    u_rotate_with_map,
    u_texture,
    u_fadetexture,
    u_color,
    u_buffer,
    u_gamma,
    u_pitch,
    u_bearing,
    u_aspect_ratio,
    u_pitch_with_map>
{
    static Values haloValues(const style::SymbolPropertyValues&,
                              const std::array<uint16_t, 2>& texsize,
                              const std::array<float, 2>& pixelsToGLUnits,
                              const RenderTile&,
                              const TransformState&,
                              float pixelRatio);

    static Values foregroundValues(const style::SymbolPropertyValues&,
                              const std::array<uint16_t, 2>& texsize,
                              const std::array<float, 2>& pixelsToGLUnits,
                              const RenderTile&,
                              const TransformState&,
                              float pixelRatio);
};

} // namespace mbgl
