#pragma once

#include <mbgl/shader/uniforms.hpp>

namespace mbgl {

namespace style {
class LinePaintProperties;
} // namespace style

class RenderTile;
class TransformState;
class LinePatternPos;
class SpriteAtlasPosition;

MBGL_DEFINE_UNIFORM_SCALAR(float, u_ratio);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_linewidth);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_gapwidth);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_antialiasing);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_extra);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_offset);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_tex_y_a);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_tex_y_b);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_sdfgamma);
MBGL_DEFINE_UNIFORM_SCALAR(float, u_fade);
MBGL_DEFINE_UNIFORM_VECTOR(float, 2, u_patternscale_a);
MBGL_DEFINE_UNIFORM_VECTOR(float, 2, u_patternscale_b);
MBGL_DEFINE_UNIFORM_MATRIX(double, 2, u_antialiasingmatrix);

struct LineColorUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_linewidth,
    u_gapwidth,
    u_blur,
    u_offset,
    u_antialiasing,
    u_antialiasingmatrix,
    u_ratio,
    u_extra,
    u_color>
{
    static Values values(const style::LinePaintProperties&,
                         float pixelRatio,
                         const RenderTile&,
                         const TransformState&);
};

struct LineSDFUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_linewidth,
    u_gapwidth,
    u_blur,
    u_offset,
    u_antialiasing,
    u_antialiasingmatrix,
    u_ratio,
    u_extra,
    u_color,
    u_patternscale_a,
    u_patternscale_b,
    u_tex_y_a,
    u_tex_y_b,
    u_mix,
    u_sdfgamma,
    u_image>
{
    static Values values(const style::LinePaintProperties&,
                         float pixelRatio,
                         const RenderTile&,
                         const TransformState&,
                         const LinePatternPos& posA,
                         const LinePatternPos& posB,
                         float dashLineWidth,
                         float atlasWidth);
};

struct LinePatternUniforms : gl::Uniforms<
    u_matrix,
    u_opacity,
    u_linewidth,
    u_gapwidth,
    u_blur,
    u_offset,
    u_antialiasing,
    u_antialiasingmatrix,
    u_ratio,
    u_extra,
    u_pattern_tl_a,
    u_pattern_br_a,
    u_pattern_tl_b,
    u_pattern_br_b,
    u_pattern_size_a,
    u_pattern_size_b,
    u_fade,
    u_image>
{
    static Values values(const style::LinePaintProperties&,
                         float pixelRatio,
                         const RenderTile&,
                         const TransformState&,
                         const SpriteAtlasPosition& posA,
                         const SpriteAtlasPosition& posB);
};

} // namespace mbgl
