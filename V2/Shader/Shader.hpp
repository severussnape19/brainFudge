#pragma once

#include <string>
#include <glad/glad.h>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    void use() const;

    // Uniform helpers
    void setFloat(const char* name, float value) const;
    void setMat4(const char* name, const float* mat) const;
    void setInt(const char* name, int value) const;
private:
    GLuint programID;

    std::string loadFile(const char* path);
    GLuint compile(GLenum type, const char* src);
};