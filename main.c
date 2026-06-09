#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);

        printf("[INFO]: Found %s\n", device_properties.deviceName);
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
                printf("\tSupports graphics queue family: %d\n", j);
            }

            VkBool32 supports_present;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, surface, &supports_present);

            // Check if the queue family has presentation capabilities specifically
            // for our surface.
            if (supports_present) {
                present_queue_family = j;
                found_present_queue_family = true;
                printf("\tSupports present queue family: %d\n", j);
            }
        }

        free(queue_families);

        // Because we will be using the swapchain, we also have to make sure
        // that the swapchain extension is supported by the Vulkan device.

        bool supports_swapchains = false;

        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &extension_count, NULL);

        VkExtensionProperties *extensions = malloc(sizeof(VkExtensionProperties) * extension_count);
        vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &extension_count, extensions);

        for (uint32_t j = 0; j < extension_count; j++) {
            if (strcmp(extensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                supports_swapchains = true;
                printf("\tSupports swapchains!\n");
                break; // We have only one device extension to check for anyways so we can break out early
            }
        }

        free(extensions);

        // If the device supports both operations and also the swapchain then we can safely break.
        if (found_graphics_queue_family && found_present_queue_family && supports_swapchains) {
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

    const char *device_extensions[] = {
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

    // SWAPCHAIN CREATION
    // ========================================================================
    // ========================================================================
    // ========================================================================

    // First we select the surface format

    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, NULL);

    VkSurfaceFormatKHR *surface_formats = malloc(sizeof(VkSurfaceFormatKHR) * surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);

    // By default we just use the first surface format we see
    VkSurfaceFormatKHR surface_format = surface_formats[0];

    // But if we find the sRGB format we should use that
    for (uint32_t i = 0; i < surface_format_count; i++) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surface_format = surface_formats[i];
            break;
        }
    }

    // We need to also select the swap extent

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);

    // By default, the one specified in the surface capabilities should be fine
    // This is because apparently the driver would set it up for you
    VkExtent2D swap_extent = surface_capabilities.currentExtent;

    // But, in some situations, we might need to set it up ourselves, such as
    // in some HiDPI displays.
    if (swap_extent.width == UINT32_MAX) {
        // By obtaining it from GLFW, of course.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        swap_extent = (VkExtent2D){
            .width = width,
            .height = height,
        };

        // We also must clamp them, of course.
        if (swap_extent.width > surface_capabilities.maxImageExtent.width) {
            swap_extent.width = surface_capabilities.maxImageExtent.width;
        }
        if (swap_extent.width < surface_capabilities.minImageExtent.width) {
            swap_extent.width = surface_capabilities.minImageExtent.width;
        }
        if (swap_extent.height > surface_capabilities.maxImageExtent.height) {
            swap_extent.height = surface_capabilities.maxImageExtent.height;
        }
        if (swap_extent.height < surface_capabilities.minImageExtent.height) {
            swap_extent.height = surface_capabilities.minImageExtent.height;
        }
    }

    // Finally, we need to determine the number of images in the swapchain.
    // Ideally, it should be one more than the minimum number of images
    // This is so that we don't have to wait on the driver to complete
    // internal operations or whatever.
    uint32_t image_count = surface_capabilities.minImageCount + 1;

    // But we must also clamp it, of course.
    if (image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = swap_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, // Apparently this is always available
        .clipped = VK_TRUE,                      // This is for clipping pixels that aren't visible.
        .oldSwapchain = NULL,
    };

    uint32_t queue_families[] = {graphics_queue_family, present_queue_family};

    // If the two queue families are different, then we need to set up sharing
    if (graphics_queue_family != present_queue_family) {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_families;
    }

    VkSwapchainKHR swapchain;
    assert(vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain) == VK_SUCCESS);

    // Of course we must also get the images

    uint32_t swapchain_image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);

    VkImage *swapchain_images = malloc(sizeof(VkImage) * swapchain_image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);

    // And not to forget the image views

    // There is the same number of image views as images so we need only one counter
    VkImageView *swapchain_image_views = malloc(sizeof(VkImageView) * swapchain_image_count);
    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        VkImageViewCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surface_format.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }};

        assert(vkCreateImageView(device, &info, NULL, swapchain_image_views + i) == VK_SUCCESS);
    }

    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        vkDestroyImageView(device, swapchain_image_views[i], NULL);
    }
    free(swapchain_image_views);
    free(swapchain_images);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();
    vkDestroyInstance(instance, NULL);

    return 0;
}
