#pragma once
#include <glad/glad.h>
#include <string>

class Shader
{
public:
private:
    GLuint programID;
    GLuint compile(GLenum type, const char* src);
}