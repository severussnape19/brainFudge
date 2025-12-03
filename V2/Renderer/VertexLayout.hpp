#pragma once

#include <vector>
#include <glad/glad.h>

struct VertexAttribute {
    GLuint count;
    GLenum type;
    GLboolean normalized;
    GLsizei offset;
};

class VertexLayout {
public:
    VertexLayout() = default;

    template<typename T> 
    void push(GLuint count);

    const std::vector<VertexAttribute>& getAttributes() const { return attributes; }
    GLsizei getStride() const { return stride; }
private:
    std::vector<VertexAttribute> attributes;
    GLsizei stride = 0;
};


// For the enums
template<>
inline void VertexLayout::push<float>(GLuint count) {
    attributes.push_back({ count, GL_FLOAT, GL_FALSE, stride });
    stride += count * sizeof(float);
}

template<>
inline void VertexLayout::push<unsigned int>(GLuint count) {
    attributes.push_back({ count, GL_UNSIGNED_INT, GL_FALSE, stride });
    stride += count * sizeof(unsigned int);
}

template<>
inline void VertexLayout::push<unsigned char>(GLuint count) {
    attributes.push_back({ count, GL_UNSIGNED_BYTE, GL_FALSE, stride });
    stride += count * sizeof(unsigned char);
}