#define VK_USE_PLATFORM_MACOS_MVK
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_MACOS
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <chrono>
#include <array>
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
#include "Includes/GameFunctions.hpp"
#include "Includes/LoadFile.hpp"
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() {
        return presentFamily.has_value() && graphicsFamily.has_value();
    }
};
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
#include "Includes/SupportCheck.hpp"
#include "Includes/FrameBuffer.hpp"
#include "Includes/SwapchainHelpers.hpp"
#include "Includes/DebugHelpers.hpp"
#include "Includes/Instance.hpp"
#include "Includes/DeviceCreation.hpp"
#include "Includes/Memory.hpp"
#include "Includes/Vertex.hpp"
#include "Includes/CommandBuffer.hpp"
#include "Includes/Descriptors.hpp"
#include "Includes/Sync.hpp"
#include "Includes/Shaders.hpp"
#include "Includes/GraphicsPipeline.hpp"
#include "Includes/Rendering.hpp"
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
    
private:
    const int MAX_FRAMES_IN_FLIGHT = 2;
    GLFWwindow* window;
    
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkCommandBuffer> commandBuffers;
    VkPipeline graphicsPipeline;
    
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    bool framebufferResized = false;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
    uint32_t currentFrame = 0;
    
    void initWindow() {
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    void initVulkan() {
        createInstance(instance, enableValidationLayers);
        setupDebugMessenger(debugMessenger, enableValidationLayers, instance);
        createSurface();
        pickPhysicalDevice(physicalDevice, surface, instance);
        createLogicalDevice(physicalDevice, device, surface, enableValidationLayers, graphicsQueue, presentQueue);
        createSwapChain(swapChain, swapChainImageFormat, swapChainExtent, swapChainImages, device, physicalDevice, surface, window);
        createImageViews(swapChainImageViews, swapChainImages, swapChainImageFormat, device);
        createRenderPass(renderPass, device, swapChainImageFormat);
        createDescriptorSetLayout(device, descriptorSetLayout);
        createGraphicsPipeline(graphicsPipeline, pipelineLayout, descriptorSetLayout, renderPass, device);
        createFramebuffers(swapChainFramebuffers, swapChainExtent, renderPass, swapChainImageViews, device);
        createCommandPool(commandPool, physicalDevice, device, surface);
        createVertexBuffer(vertexBuffer, vertexBufferMemory, device, physicalDevice, commandPool, graphicsQueue);
        createIndexBuffer(indexBuffer, indexBufferMemory, device, physicalDevice, commandPool, graphicsQueue);
        createUniformBuffers(device, physicalDevice, uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, MAX_FRAMES_IN_FLIGHT);
        createDescriptorPool(device, descriptorPool, MAX_FRAMES_IN_FLIGHT);
        createDescriptorSets(device, descriptorSets, descriptorPool, descriptorSetLayout, uniformBuffers, MAX_FRAMES_IN_FLIGHT);
        createCommandBuffer(commandPool, commandBuffers, vertexBuffer, device, MAX_FRAMES_IN_FLIGHT);
        createSyncObjects(imageAvailableSemaphores, inFlightFences, renderFinishedSemaphores, MAX_FRAMES_IN_FLIGHT, device);
    }
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
    
    void mainLoop() {
        const int FPSDEBUG = 1;
        int it = 0;
        auto startTime = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(window)) {
            if(framebufferResized){
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
            }
            if(FPSDEBUG){
                auto endTime = std::chrono::high_resolution_clock::now();
                if(getFPS(it, startTime, endTime)){
                    startTime = std::chrono::high_resolution_clock::now();
                }
            }
            glfwPollEvents();
//            drawFrame(device, graphicsPipeline, swapChain, swapChainFramebuffers, swapChainExtent, renderPass, imageAvailableSemaphores, renderFinishedSemaphores, commandBuffers, inFlightFences, graphicsQueue, presentQueue, MAX_FRAMES_IN_FLIGHT, currentFrame);
            drawFrame(device, graphicsPipeline, swapChain, swapChainFramebuffers, swapChainExtent, renderPass, imageAvailableSemaphores, renderFinishedSemaphores, commandBuffers, vertexBuffer, indexBuffer, uniformBuffersMapped, inFlightFences, graphicsQueue, presentQueue, swapChainImageFormat, swapChainImages, swapChainImageViews, physicalDevice, surface, window, framebufferResized, descriptorSets, pipelineLayout, MAX_FRAMES_IN_FLIGHT, currentFrame);
        }
    }
    
    void cleanup() {
        vkDeviceWaitIdle(device);//IF BROKEN: usleep(.01 * 1000000); wait for commands to stop running so shutdown is safe
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }
        
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        
        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);
        
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);
        
        vkDestroyCommandPool(device, commandPool, nullptr);
        
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        cleanupSwapChain(device, swapChain, swapChainImageViews, swapChainFramebuffers);
        
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
