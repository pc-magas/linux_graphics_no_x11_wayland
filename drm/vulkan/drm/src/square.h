#ifndef SQUARE_H
#define SQUARE_H

#include <vulkan/vulkan.h>

typedef struct {
    VkDevice device;
    VkPipeline pipeline;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
} Square;

VkResult createSquare(Square *square, VkDevice device,
                      VkRenderPass renderPass,
                      VkExtent2D extent,
                      VkShaderModule vertexShader,
                      VkShaderModule fragmentShader,
                      VkPipelineLayout pipelineLayout);

void drawSquare(VkCommandBuffer commandBuffer, const Square *square);

void destroySquare(Square *square);

#endif