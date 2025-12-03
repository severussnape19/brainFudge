#include "VertexArray.hpp"

VertexArray::VertexArray() {
    glGenVertexArrays(1, &vao);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &vao);
}

void VertexArray::bind() const {
    glBindVertexArray(vao);
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VertexArray::addBuffer(const VertexBuffer& vbo, const VertexLayout& layout) {
    bind();
    vbo.bind();

    const auto& attributes = layout.getAttributes();
    GLsizei stride = layout.getStride();

    for (GLuint i = 0; i < attributes.size(); i++) {
        const auto& attr = attributes[i];

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(
            i,
            attr.count,
            attr.type,
            attr.normalized,
            stride,
            reinterpret_cast<const void*>(static_cast<uintptr_t>(attr.offset))
        );
    }

    vbo.unbind();
    unbind();
}
