#pragma once

#include <glad/glad.h>
#include "VertexBuffer.hpp"
#include "VertexLayout.hpp"

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void bind() const;
    void unbind() const;

    void addBuffer(const VertexBuffer& vbo, const VertexLayout& layout);
private:
    GLuint vao = 0;
};