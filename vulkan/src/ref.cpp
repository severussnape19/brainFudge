#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// ---- Validation layers ----
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

// ---- Required device extensions ----
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// =====================================================
// Simple file loader
// =====================================================
static std::vector<char> readFile(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary | std::ios::ate);
    if (!f.is_open()) throw std::runtime_error("Failed to open file: " + filename);
    size_t size = (size_t)f.tellg();
    std::vector<char> buffer(size);
    f.seekg(0);
    f.read(buffer.data(), size);
    return buffer;
}

struct QueueFamilies {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    bool complete() const { return graphics && present; }
};

struct SwapSupport {
    VkSurfaceCapabilitiesKHR caps;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> modes;
};

// =====================================================
// Validation Callback
// =====================================================
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* userData
) {
    std::cerr << "[VALIDATION] " << data->pMessage << "\n";
    return VK_FALSE;
}

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* ci,
    const VkAllocationCallbacks* alloc,
    VkDebugUtilsMessengerEXT* messenger
) {
    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (!fn) return VK_ERROR_EXTENSION_NOT_PRESENT;
    return fn(instance, ci, alloc, messenger);
}

void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT messenger,
    const VkAllocationCallbacks* alloc
) {
    auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fn) fn(instance, messenger, alloc);
}

// =====================================================
// Global Vulkan Variables (non-OOP like you wanted)
// =====================================================
GLFWwindow* window;
VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;

VkSurfaceKHR surface;

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkDevice device;

VkQueue graphicsQueue;
VkQueue presentQueue;

VkSwapchainKHR swapchain;
std::vector<VkImage> swapImages;
VkFormat swapFormat;
VkExtent2D swapExtent;
std::vector<VkImageView> swapImageViews;

VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;

// The rest (framebuffers, commands, sync) come in PART 2

// =====================================================
// Window
// =====================================================
void initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Triangle (Single File)", nullptr, nullptr);
}

// =====================================================
// Instance
// =====================================================
bool checkValidationSupport() {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());

    for (auto layerName : validationLayers) {
        bool found = false;
        for (auto& prop : layers) {
            if (!strcmp(layerName, prop.layerName)) {
                found = true; break;
            }
        }
        if (!found) return false;
    }
    return true;
}

std::vector<const char*> getRequiredExtensions() {
    uint32_t count;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> exts(glfwExt, glfwExt + count);
    if (enableValidationLayers) exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return exts;
}

void populateDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& info) {
    info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
}

void createInstance() {
    if (enableValidationLayers && !checkValidationSupport())
        throw std::runtime_error("Requested validation layers not available");

    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "Triangle";
    app.apiVersion = VK_API_VERSION_1_3;

    auto exts = getRequiredExtensions();

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = exts.size();
    ci.ppEnabledExtensionNames = exts.data();

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    if (enableValidationLayers) {
        populateDebugInfo(debugInfo);
        ci.enabledLayerCount = validationLayers.size();
        ci.ppEnabledLayerNames = validationLayers.data();
        ci.pNext = &debugInfo;
    }

    if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
}

// =====================================================
// Debug Messenger
// =====================================================
void setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT info;
    populateDebugInfo(info);

    if (createDebugUtilsMessengerEXT(instance, &info, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Failed to create debug messenger");
}

// =====================================================
// Surface
// =====================================================
void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Surface creation failed");
}

// =====================================================
// Physical Device
// =====================================================
QueueFamilies findQueueFamilies(VkPhysicalDevice dev) {
    QueueFamilies q;

    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, props.data());

    for (uint32_t i = 0; i < props.size(); i++) {
        if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            q.graphics = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &presentSupport);
        if (presentSupport)
            q.present = i;

        if (q.complete()) break;
    }

    return q;
}

SwapSupport querySwapSupport(VkPhysicalDevice dev) {
    SwapSupport s;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &s.caps);

    uint32_t fcount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &fcount, nullptr);
    s.formats.resize(fcount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &fcount, s.formats.data());

    uint32_t mcount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &mcount, nullptr);
    s.modes.resize(mcount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &mcount, s.modes.data());

    return s;
}

bool isDeviceSuitable(VkPhysicalDevice dev) {
    QueueFamilies q = findQueueFamilies(dev);
    if (!q.complete()) return false;

    SwapSupport s = querySwapSupport(dev);
    bool ok = !s.formats.empty() && !s.modes.empty();
    return ok;
}

void pickPhysicalDevice() {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0) throw std::runtime_error("No GPUs with Vulkan support");

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    for (auto dev : devices) {
        if (isDeviceSuitable(dev)) {
            physicalDevice = dev;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("No suitable GPU found");
      
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);

    std::cout << "GPU: " << props.deviceName << "\n";
    std::cout << "Vulkan Version: " << VK_VERSION_MAJOR(props.apiVersion) << "."
              << VK_VERSION_MINOR(props.apiVersion) << "."
              << VK_VERSION_PATCH(props.apiVersion) << "\n";

}

// =====================================================
// Logical Device  (FIXED: includes swapchain extension)
// =====================================================
void createLogicalDevice() {
    QueueFamilies q = findQueueFamilies(physicalDevice);
    std::set<uint32_t> unique = { q.graphics.value(), q.present.value() };

    float priority = 1.f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;

    for (uint32_t family : unique) {
        VkDeviceQueueCreateInfo qi{};
        qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = family;
        qi.queueCount = 1;
        qi.pQueuePriorities = &priority;
        queueInfos.push_back(qi);
    }

    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = queueInfos.size();
    ci.pQueueCreateInfos = queueInfos.data();

    // 🔥 REQUIRED FOR SWAPCHAIN
    ci.enabledExtensionCount = deviceExtensions.size();
    ci.ppEnabledExtensionNames = deviceExtensions.data();

    // validation layers inside device (optional)
    if (enableValidationLayers) {
        ci.enabledLayerCount = validationLayers.size();
        ci.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateDevice(physicalDevice, &ci, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

    vkGetDeviceQueue(device, q.graphics.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, q.present.value(), 0, &presentQueue);
}

// =====================================================
// Swapchain
// =====================================================
VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            return f;
    }
    return formats[0];
}

VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
    for (auto& m : modes) {
        if (m == VK_PRESENT_MODE_MAILBOX_KHR)
            return m;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& caps) {
    if (caps.currentExtent.width != UINT32_MAX)
        return caps.currentExtent;
    return { WIDTH, HEIGHT };
}

void createSwapchain() {
    SwapSupport s = querySwapSupport(physicalDevice);

    auto format = chooseFormat(s.formats);
    auto mode = choosePresentMode(s.modes);
    auto extent = chooseExtent(s.caps);

    uint32_t imageCount = s.caps.minImageCount + 1;
    if (s.caps.maxImageCount > 0 && imageCount > s.caps.maxImageCount)
        imageCount = s.caps.maxImageCount;

    QueueFamilies q = findQueueFamilies(physicalDevice);
    uint32_t families[] = { q.graphics.value(), q.present.value() };

    VkSwapchainCreateInfoKHR ci{};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = surface;
    ci.minImageCount = imageCount;
    ci.imageFormat = format.format;
    ci.imageColorSpace = format.colorSpace;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (q.graphics != q.present) {
        ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        ci.queueFamilyIndexCount = 2;
        ci.pQueueFamilyIndices = families;
    } else {
        ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    ci.preTransform = s.caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = mode;
    ci.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain");

    uint32_t count;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    swapImages.resize(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapImages.data());

    swapFormat = format.format;
    swapExtent = extent;
}

void createImageViews() {
    swapImageViews.resize(swapImages.size());

    for (size_t i = 0; i < swapImages.size(); i++) {
        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = swapImages[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = swapFormat;
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &ci, nullptr, &swapImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view");
    }
}

// =====================================================
// Render Pass
// =====================================================
void createRenderPass() {
    VkAttachmentDescription color{};
    color.format = swapFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref{};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub{};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &ref;

    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = 1;
    ci.pAttachments = &color;
    ci.subpassCount = 1;
    ci.pSubpasses = &sub;

    if (vkCreateRenderPass(device, &ci, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass");
}

// =====================================================
// Shader + Pipeline (FULLY FIXED)
// =====================================================
VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size();
    ci.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule mod;
    if (vkCreateShaderModule(device, &ci, nullptr, &mod) != VK_SUCCESS)
        throw std::runtime_error("Shader module creation failed");
    return mod;
}

void createGraphicsPipeline() {
    auto vertCode = readFile("../src/Shaders/simple_shader.vert.spv");
    auto fragCode = readFile("../src/Shaders/simple_shader.frag.spv");



    VkShaderModule vert = createShaderModule(vertCode);
    VkShaderModule frag = createShaderModule(fragCode);

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vert;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = frag;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = { vertStage, fragStage };

    VkPipelineVertexInputStateCreateInfo vertex{};
    vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo assembly{};
    assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.width = (float)swapExtent.width;
    viewport.height = (float)swapExtent.height;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent = swapExtent;

    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.pViewports = &viewport;
    vp.scissorCount = 1;
    vp.pScissors = &scissor;

    // ---- FIXED RASTERIZATION ----
    VkPipelineRasterizationStateCreateInfo raster{};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode = VK_CULL_MODE_BACK_BIT;
    raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster.lineWidth = 1.0f;  // REQUIRED

    // ---- FIXED MULTISAMPLING ----
    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // REQUIRED

    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo blendState{};
    blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendState.attachmentCount = 1;
    blendState.pAttachments = &blend;

    VkPipelineLayoutCreateInfo layout{};
    layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(device, &layout, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.stageCount = 2;
    ci.pStages = stages;
    ci.pVertexInputState = &vertex;
    ci.pInputAssemblyState = &assembly;
    ci.pViewportState = &vp;
    ci.pRasterizationState = &raster;
    ci.pMultisampleState = &ms;
    ci.pColorBlendState = &blendState;
    ci.layout = pipelineLayout;
    ci.renderPass = renderPass;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &ci, nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");

    vkDestroyShaderModule(device, vert, nullptr);
    vkDestroyShaderModule(device, frag, nullptr);
}

// =====================================================
// FRAMEBUFFERS
// =====================================================
std::vector<VkFramebuffer> framebuffers;

void createFramebuffers() {
    framebuffers.resize(swapImageViews.size());

    for (size_t i = 0; i < swapImageViews.size(); i++) {
        VkImageView attachments[] = { swapImageViews[i] };

        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = renderPass;
        ci.attachmentCount = 1;
        ci.pAttachments = attachments;
        ci.width = swapExtent.width;
        ci.height = swapExtent.height;
        ci.layers = 1;

        if (vkCreateFramebuffer(device, &ci, nullptr, &framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer");
    }
}

// =====================================================
// COMMAND POOL + COMMAND BUFFERS
// =====================================================
VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;

void createCommandPool() {
    QueueFamilies q = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.queueFamilyIndex = q.graphics.value();

    if (vkCreateCommandPool(device, &ci, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");
}

void createCommandBuffers() {
    commandBuffers.resize(framebuffers.size());

    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = commandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &ai, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(commandBuffers[i], &bi);

        VkRenderPassBeginInfo rp{};
        rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp.renderPass = renderPass;
        rp.framebuffer = framebuffers[i];
        rp.renderArea.offset = { 0, 0 };
        rp.renderArea.extent = swapExtent;

        VkClearValue clear = { 0.1f, 0.1f, 0.2f, 1.0f };
        rp.clearValueCount = 1;
        rp.pClearValues = &clear;

        vkCmdBeginRenderPass(commandBuffers[i], &rp, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to record command buffer");
    }
}

// =====================================================
// SYNC OBJECTS
// =====================================================
VkSemaphore imageAvailable;
VkSemaphore renderFinished;
VkFence inFlight;

void createSyncObjects() {
    VkSemaphoreCreateInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(device, &si, nullptr, &imageAvailable) != VK_SUCCESS ||
        vkCreateSemaphore(device, &si, nullptr, &renderFinished) != VK_SUCCESS)
        throw std::runtime_error("Failed to create semaphores");

    VkFenceCreateInfo fi{};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(device, &fi, nullptr, &inFlight) != VK_SUCCESS)
        throw std::runtime_error("Failed to create fence");
}

// =====================================================
// DRAW FRAME
// =====================================================
void drawFrame() {
    vkWaitForFences(device, 1, &inFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlight);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex);

    VkSemaphore waitSemaphores[] = { imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = waitSemaphores;
    si.pWaitDstStageMask = waitStages;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { renderFinished };
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &si, inFlight) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer");

    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = signalSemaphores;
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain;
    pi.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &pi);
}

// =====================================================
// CLEANUP
// =====================================================
void cleanup() {
    vkDestroySemaphore(device, renderFinished, nullptr);
    vkDestroySemaphore(device, imageAvailable, nullptr);
    vkDestroyFence(device, inFlight, nullptr);

    for (auto fb : framebuffers)
        vkDestroyFramebuffer(device, fb, nullptr);

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto iv : swapImageViews)
        vkDestroyImageView(device, iv, nullptr);

    vkDestroySwapchainKHR(device, swapchain, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

// =====================================================
// MAIN
// =====================================================
int main() {
    try {
        initWindow();
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
        cleanup();

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


