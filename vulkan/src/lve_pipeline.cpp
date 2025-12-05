#include "lve_pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace lve {

    LvePipeline::LvePipeline(const std::string& vertFilepath, const std::string& fragFilepath) {
        createGraphicsPipeline(vertFilepath, fragFilepath);
    }
    
    std::vector<char> LvePipeline::readFile(const std::string& filepath) {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary}; //std::ios::ate and std::ios::binary are bit flags
        
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file : " + filepath);
        }

        size_t filesize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(filesize);

        file.seekg(0); // goes to the start of the file
        file.read(buffer.data(), filesize); // reads the file

        file.close();
        return buffer;
    };

    void LvePipeline::createGraphicsPipeline(
        const std::string& vertFilepath, const std::string& fragFilepath) {

            auto vertCode = readFile(vertFilepath);
            auto fragCode = readFile(fragFilepath);

            std::cout << "Vertex shader code size: " << vertCode.size() << "\n";
            std::cout << "Fragment shader code size: " << fragCode.size() << "\n";
    }
}