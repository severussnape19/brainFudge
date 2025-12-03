#pragma once

#include <glad/glad.h>
#include <vector>

#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexLayout.hpp"
#include "Renderer/VertexBuffer.hpp"

class Mesh {
public:
public:
    Mesh(const float* vertices, size_t count, const VertexLayout& layout);
    ~Mesh() = default;

    void draw() const;
private:
    VertexArray vao;
    VertexBuffer vbo;
    size_t vertexCount = 0;
};