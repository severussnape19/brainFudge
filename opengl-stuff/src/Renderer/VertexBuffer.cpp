#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer(const void* data, size_t size) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &vbo);
}

void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}