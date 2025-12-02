#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vSrc = loadFile(vertexPath);
    std::string fSrc = loadFile(fragmentPath);

    GLuint v = compile(GL_VERTEX_SHADER, vSrc.c_str());
    GLuint f = compile(GL_FRAGMENT_SHADER, fSrc.c_str());

    programID = glCreateProgram();
    glAttachShader(programID, v);
    glAttachShader(programID, f);
    glLinkProgram(programID);

    glDeleteShader(v);
    glDeleteShader(f);
}

void Shader::use() const {
    glUseProgram(programID);
}

GLuint Shader::compile(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &src, nullptr); // uploads the glsl source
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader error: " << log << "\n";
    }
    return shader;
}

void Shader::setFloat(const char* name, const float value) const {
    glUniform1f(glGetUniformLocation(programID, name), value);
}

void Shader::setMat4(const char* name, const float* mat) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, mat);
}

std::string Shader::loadFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open the file!\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

