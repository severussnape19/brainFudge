#include <fstream>
#include <sstream>
#include <string>

std::string loadFile(cosnt std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader File: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss >> file.rdbuf();
    return ss.str();
}

int main() {
    // load shader source from files
    std::string vertexSrc = loadFile("Shaders/fragment_shader.glsl");
    std::string fragmentSrc = loadFile("Shaders/fragment.glsl");

    const char* vSrc = vertexSrc.c_str();
    const char* fSrc = fragmentSrc.c_str();
}