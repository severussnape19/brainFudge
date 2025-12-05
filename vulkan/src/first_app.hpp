#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"

namespace lve {
    class FirstApp {

        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            void run();
        private:
            LveWindow lveWindow{WIDTH, HEIGHT, "HELLOO VULKAN!"}; // window will be created with our first app class
            LvePipeline lvePipeline{"../src/Shaders/simple_shader.vert.spv", "../src/Shaders/simple_shader.frag.spv"};
    };
}