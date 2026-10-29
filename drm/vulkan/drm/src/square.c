
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "square.h"

static const float vertices[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
    -0.5f,  0.5f,
     0.5f,  0.5f
};

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1u << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags
             & properties) == properties) {
            return i;
        }
    }

    return UINT32_MAX;
}

VkResult createSquare(Square *square,
                      VkPhysicalDevice physicalDevice,
                      VkDevice device,
                      VkRenderPass renderPass,
                      VkExtent2D extent,
                      VkShaderModule vertexShader,
                      VkShaderModule fragmentShader,
                      VkPipelineLayout pipelineLayout)
{
    if (!square || !physicalDevice || !device ||
        !renderPass || !vertexShader || !fragmentShader ||
        !pipelineLayout || extent.width == 0 ||
        extent.height == 0) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    memset(square, 0, sizeof(*square));
    square->device = device;

    /* Create the vertex buffer. */
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertices),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult result = vkCreateBuffer(device, &bufferInfo, NULL, &square->vertexBuffer);

    if (result != VK_SUCCESS){
        destroySquare(square);
        return result;
    }

    /* Find a suitable memory type. */
    VkMemoryRequirements memRequirements;

    vkGetBufferMemoryRequirements(device, square->vertexBuffer, &memRequirements);

    uint32_t memoryType = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (memoryType == UINT32_MAX) {
        result = VK_ERROR_FEATURE_NOT_PRESENT;
        destroySquare(square);
        return result;
    }

    /* Allocate memory for the vertex buffer. */
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryType
    };

    result = vkAllocateMemory(
        device, &allocInfo, NULL, &square->vertexMemory);

    if (result != VK_SUCCESS)
        destroySquare(square);
        return result;

    result = vkBindBufferMemory(
        device, square->vertexBuffer,
        square->vertexMemory, 0);

    if (result != VK_SUCCESS){
        destroySquare(square);
        return result;
    }
        

    /* Upload the vertex data. */
    void *mapped = NULL;

    result = vkMapMemory(
        device, square->vertexMemory, 0,
        sizeof(vertices), 0, &mapped);

    if (result != VK_SUCCESS){
        destroySquare(square);
        return result;
    }


    memcpy(mapped, vertices, sizeof(vertices));
    vkUnmapMemory(device, square->vertexMemory);

    /* Vertex input: vec2 position. */
    VkVertexInputBindingDescription binding = {
        .binding = 0,
        .stride = 2 * sizeof(float),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attribute = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 0
    };

    VkPipelineVertexInputStateCreateInfo vertexInput = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = &attribute
    };

    /* Four vertices forming a triangle strip. */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = extent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = VK_FALSE,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main"
        },
        {
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName = "main"
        }
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType =
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineIndex = -1
    };

    result = vkCreateGraphicsPipelines(
        device, VK_NULL_HANDLE, 1, &pipelineInfo,
        NULL, &square->pipeline);

    if (result != VK_SUCCESS){
        destroySquare(square);
        return result;
    }

    return VK_SUCCESS;

}

void drawSquare(VkCommandBuffer commandBuffer, const Square *square)
{
    VkDeviceSize offset = 0;

    vkCmdBindPipeline(commandBuffer,
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      square->pipeline);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1,
                           &square->vertexBuffer, &offset);

    vkCmdDraw(commandBuffer, 4, 1, 0, 0);
}

void destroySquare(Square *square)
{
    if (!square || !square->device)
        return;

    if (square->pipeline)
        vkDestroyPipeline(square->device, square->pipeline, NULL);

    if (square->vertexBuffer)
        vkDestroyBuffer(square->device, square->vertexBuffer, NULL);

    if (square->vertexMemory)
        vkFreeMemory(square->device, square->vertexMemory, NULL);

    memset(square, 0, sizeof(*square));
}
