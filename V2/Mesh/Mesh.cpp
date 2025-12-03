#include "Mesh.hpp"

Mesh::Mesh(const float* vertices, size_t count, const VertexLayout& layout)
    : vbo(vertices, count * sizeof(float)),
    vertexCount(count / (layout.getStride() / sizeof(float)))
{
    vao.addBuffer(vbo, layout);
}


void Mesh::draw() const {
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0,  vertexCount);
}