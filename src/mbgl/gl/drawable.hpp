#pragma once

#include <mbgl/gl/types.hpp>
#include <mbgl/gl/mode.hpp>
#include <mbgl/gl/depth.hpp>
#include <mbgl/gl/stencil.hpp>
#include <mbgl/gl/color.hpp>
#include <mbgl/gl/vertex_buffer.hpp>
#include <mbgl/gl/index_buffer.hpp>
#include <mbgl/gl/attribute.hpp>
#include <mbgl/gl/uniform.hpp>
#include <mbgl/gl/segment.hpp>

#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

namespace mbgl {
namespace gl {

template <class Vertex>
class UnindexedVertices {
public:
    using VertexType = Vertex;

    UnindexedVertices(Mode mode_, const VertexBuffer<Vertex>& vertexBuffer_)
        : mode(std::move(mode_)),
          vertexBuffer(vertexBuffer_.buffer),
          segments({{ 0, 0, vertexBuffer_.vertexCount, 0 }}) {}

    Mode mode;
    gl::BufferID vertexBuffer;
    static constexpr std::size_t vertexSize = sizeof(Vertex);
    static constexpr gl::BufferID indexBuffer = 0;
    static constexpr std::size_t primitiveSize = 0;
    std::vector<Segment> segments;
};

template <class Mode, class Vertex, class...Args>
auto Unindexed(const VertexBuffer<Vertex>& vertexBuffer,
               Args&&... modeArguments) {
    return UnindexedVertices<Vertex>(
        Mode { std::forward<Args>(modeArguments)... },
        vertexBuffer);
}

template <class Vertex, class Primitive>
class SegmentedVertices {
public:
    using VertexType = Vertex;

    SegmentedVertices(Mode mode_,
                      const VertexBuffer<Vertex>& vertexBuffer_,
                      const IndexBuffer<Primitive>& indexBuffer_,
                      const std::vector<Segment>& segments_)
        : mode(std::move(mode_)),
          vertexBuffer(vertexBuffer_.buffer),
          indexBuffer(indexBuffer_.buffer),
          segments(segments_) {}

    Mode mode;
    gl::BufferID vertexBuffer;
    static constexpr std::size_t vertexSize = sizeof(Vertex);
    gl::BufferID indexBuffer;
    static constexpr std::size_t primitiveSize = sizeof(Primitive);
    const std::vector<Segment>& segments;
};

template <class Mode, class Vertex, class Primitive, class...Args>
auto Segmented(const VertexBuffer<Vertex>& vertexBuffer,
               const IndexBuffer<Primitive>& indexBuffer,
               const std::vector<Segment>& segments,
               Args&&... modeArguments) {
    static_assert(std::is_same<typename Primitive::Mode, Mode>::value, "primitive mode mismatch");
    return SegmentedVertices<Vertex, Primitive>(
        Mode { std::forward<Args>(modeArguments)... },
        vertexBuffer,
        indexBuffer,
        segments);
}

class Drawable {
public:
    template <class Shader, class UniformValues, class Subject>
    Drawable(Depth depth_,
             Stencil stencil_,
             Color color_,
             const Shader& shader,
             const UniformValues& uniformValues,
             const Subject& subject)
        : mode(subject.mode),
          depth(std::move(depth_)),
          stencil(std::move(stencil_)),
          color(std::move(color_)),
          program(shader.getID()),
          vertexBuffer(subject.vertexBuffer),
          vertexSize(subject.vertexSize),
          indexBuffer(subject.indexBuffer),
          primitiveSize(subject.primitiveSize),
          segments(subject.segments),
          bindUniforms(Shader::UniformsType::binder(shader, uniformValues)),
          attributeBindings(AttributeBindings<Shader, typename Subject::VertexType>()(shader))
    {
        static_assert(std::is_standard_layout<typename Subject::VertexType>::value, "vertex type must use standard layout");
        static_assert(std::is_same<typename Shader::UniformsType::Values, UniformValues>::value, "uniforms type mismatch");
        static_assert(std::is_same<typename Shader::VertexType, typename Subject::VertexType>::value, "vertex type mismatch");
    }

    Mode mode;
    Depth depth;
    Stencil stencil;
    Color color;
    gl::ProgramID program;
    gl::BufferID vertexBuffer;
    std::size_t vertexSize;
    gl::BufferID indexBuffer;
    std::size_t primitiveSize;
    const std::vector<Segment>& segments;
    std::function<void ()> bindUniforms;
    std::vector<AttributeBinding> attributeBindings;
};

} // namespace gl
} // namespace mbgl
