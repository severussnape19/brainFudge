#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib> // provides EXIT_FAILURE and EXIT_SUCCESS
#include <vector>
#include <cstring>

// globals
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG // NDEBUG is a macro meaning "not debug"
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

// Proxy function to get the address of vkCreateDebugUtilsMessengerEXT function
// We are using this function to get the address of the vkCreateDebugUtilsMessengerEXT function.
// This is a function that is not a part of the core library as it is an extension function. We need this function to create the VkDebugUtilsMessengerEXT object
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger
) {
        // typecasts the address of a vulkan function and stores it on func
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks *pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class HelloTriangleApplication {
public:
    void run()  {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
        
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello triangle";
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "No engine";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // This structure tells vulkan driver which global extensions and validation layers we want to use.
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Number of extensions
//      uint32_t glfwExtensionCount = 0;
        // Pointer to an array of C strings
//      const char** glfwExtensions;

        // This function returns a list of vulkan instance extensions that the os/window system requires so vulkan can create a surface and present images to the screen
//      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Includes the validation layer names if they are enabled
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create an instance!");
        } else {
            std::cout << "\n\tInstance has made successfully!" << std::endl;
        }

        //optional function to display extensions
        // extensionCheck(glfwExtensions, glfwExtensionCount);
    }

    void setupDebugMessenger() {
        if(!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;

        // This new function is needed to obtain the address of the  vkCreateDebugUtilsMessengerEXT function.
        // This function is not a part of the core library so we cannot access it directly.
        // This function is needed to create the VkDebugUtilsMessengeEXT object.
        
        // We will need another proxy function to get the address of vkDestroyDebugUtilsMessengerEXT function to be able to destroy the created object
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void extensionCheck(const char** glfwExtensionList = nullptr, uint32_t ExtCount = 0) {

        // Below code is to handle the VK_ERROR_EXTENSION_NOT_PRESENT error.
        // stores the number of extensions
        uint32_t extensionCount = 0;
        // stores the extension count to the extensionCount variable
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        // Stores the list of extensions
        std::vector<VkExtensionProperties> extensions(extensionCount);
        // we call this function twice instead of once because this function requires a pointer to a contiguous block of memory of a knwon size.
        // The first time it is called, we have the number of extensions and the second time, we will have the list stored in the vector list
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "List of available Extensions the driver supports(VK):\n ";
        for (auto &extension : extensions) {
            std::cout << "\t" << extension.extensionName << "\n";
        }

        if (ExtCount > 0) {
            std::cout << "\t#####################################\n"; 

            std::cout << "List of Required Extensions(glfw):\n ";
            for (int i = 0; i < ExtCount; i++) {
                std::cout << "\t" << glfwExtensionList[i] << "\n";
            }
        }
    }

    // This function checks if all the requested validation layers are available
    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        return true;
    }

    // optional message callback
    // Returns the list of extensions based on wheather validation layers are enabled or not
    // Returns a list of Vulkan instance extensions that your program must enable.
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount); //(start ptr, end ptr) endPtr = startPtr + size of the list ==> *(p + size)

        if (enableValidationLayers) {
            // "VK_EXT_debug_utils" enables the debugging tools
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}