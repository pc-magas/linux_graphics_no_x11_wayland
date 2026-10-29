#include <stdio.h>
#include <vulkan/vulkan.h>

int main() {
    VkInstance instance;
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        printf("Failed to create Vulkan instance!\n");
        return -1;
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties extensions[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    printf("Available extensions:\n");
    for (uint32_t i = 0; i < extensionCount; i++) {
        printf("\t%s\n", extensions[i].extensionName);
    }

    vkDestroyInstance(instance, NULL);
    return 0;
}