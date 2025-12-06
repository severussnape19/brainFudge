#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace lve {
    
    class LveWindow {
        public:
            LveWindow(int w, int h, std::string name);
            ~LveWindow();

            // We are deleting/disabling copy constructor and copy assignment operator
            // this makes copying / assigning not possible
            // This is because the class owns resources that cannot be duplicated safely
            LveWindow(const LveWindow &) = delete;
            LveWindow &operator=(const LveWindow &) = delete;

            bool shouldClose() { return glfwWindowShouldClose(window); }

            void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        private:
            void initWindow();

            const int width;
            const int height;

            std::string windowName;
            GLFWwindow *window;
    };
} // namespace lve