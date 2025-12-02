#pragma once

#include <glad/glad.h>
#include <cstddef>

class VertexBuffer {
public:
    VertexBuffer(const void* data, size_t size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
private:    
   GLuint vbo = 0;
};