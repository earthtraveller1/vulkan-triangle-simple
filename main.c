#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main(void) {
    assert(glfwInit());

    // Find the instance extensions that are needed by GLFW
    uint32_t glfw_extension_count;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    // First, we create the instance

    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = NULL,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = glfw_extension_count,
        .ppEnabledExtensionNames = glfw_extensions,
    };

    VkInstance instance;
    assert(vkCreateInstance(&instance_info, NULL, &instance) == VK_SUCCESS);

    // Then, we need to create the window.

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Resizing is a bit complicated so let's not worry about that lol
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Neng Li", NULL, NULL);
    assert(window);

    // From the window, we create a Vulkan surface.
    VkSurfaceKHR surface;
    assert(glfwCreateWindowSurface(instance, window, NULL, &surface) == VK_SUCCESS);

    // Next, we get the physical devices

    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL);
    assert(physical_device_count > 0);

    VkPhysicalDevice *physical_devices = malloc(physical_device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

    // We select the physical device - basically, pick the one that has the
    // queue families that we need. This is necessary because a lot of systems
    // have both an iGPU and a dGPU, and some systems have llvmpipe installed.

    uint32_t graphics_queue_family, present_queue_family;
    VkPhysicalDevice physical_device = NULL;

    for (uint32_t i = 0; i < physical_device_count; i++) {
        // Obviously allocating memory every iteration is not optimal
        // but I mean who cares?

        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, NULL);

        VkQueueFamilyProperties *queue_families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_count, NULL);

        bool found_graphics_queue_family = false, found_present_queue_family = false;

        for (uint32_t j = 0; j < queue_family_count; j++) {
            // Check if the queue family has graphics capabilities
            if (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_queue_family = j;
                found_graphics_queue_family = true;
            }

            VkBool32 supports_present;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, surface, &supports_present);

            // Check if the queue family has presentation capabilities specifically
            // for our surface.
            if (supports_present) {
                present_queue_family = j;
                found_present_queue_family = true;
            }
        }

        free(queue_families);

        // If the device supports both operations then we can safely break.
        if (found_graphics_queue_family && found_present_queue_family) {
            physical_device = physical_devices[i];
            break;
        }
    }

    // Clearly if we have yet to find a physical device your system probably 
    // doesn't support Vulkan
    assert(physical_device != VK_NULL_HANDLE);
    free(physical_devices);

    // Now that we've selected the physical device, now we create the logical
    // device.

    const char* device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    // There will at most be two queues, but fortunately we will only
    // create one if both queue families are the same, as we could
    // just use one queue for everything.
    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_infos[2] = {
        (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = graphics_queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        },
        (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = present_queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        },
    };

    // If the two queue families are the same, then there is only one
    // queue family, so we just create one.
    uint32_t queue_create_info_count;
    if (graphics_queue_family == present_queue_family) {
        queue_create_info_count = 1;
    } else {
        queue_create_info_count = 2;
    }

    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = queue_create_info_count,
        .pQueueCreateInfos = queue_infos,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_extensions,
        .pEnabledFeatures = NULL,
    };

    VkDevice device;
    assert(vkCreateDevice(physical_device, &device_info, NULL, &device) == VK_SUCCESS);

    VkQueue graphics_queue, present_queue;
    vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, present_queue_family, 0, &present_queue);



    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();
    vkDestroyInstance(instance, NULL);

    return 0;
}
