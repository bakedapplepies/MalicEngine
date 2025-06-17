#include "Engine/VulkanManager.h"

#include <unordered_map>
#include <algorithm>
#include <array>
#include <set>

#include "Engine/core/Config.h"
#include "Engine/core/Assert.h"
#include "Engine/core/Debug.h"
#include "Engine/core/Logging.h"
#include "Engine/VertexArray.h"
#include "Engine/Shader.h"

MLC_NAMESPACE_START

#ifdef NDEBUG
    const bool ENABLE_VALIDATION_LAYERS = false;
#else
    const bool ENABLE_VALIDATION_LAYERS = true;
#endif

static std::unordered_map<std::string_view, bool> s_supportedExtensions;
static std::unordered_map<std::string_view, bool> s_supportedLayers;

void VulkanManager::Init(GLFWwindow* window)
{
    // Note: Every vkCreateXXX has a mandatory vkDestroyXXX
    //       Resource allocated from pools (cmd buffers or descriptors) don't need this
    m_window = window;

    if (ENABLE_VALIDATION_LAYERS)
    {
        MLC_ASSERT(_CheckValidationLayerSupport(), "Validation Layer enabled, not available.");
    }
    _CreateInstance();
    _SetupDebugMessenger();
    _CreateSurface();
    _PickPhysicalDevice();
    _CreateLogicalDevice();
    _GetQueues();
    _CreateSwapChain();
    _CreateImageViews();
    _CreateRenderPass();
    _CreateDescriptorPool();
    _CreateFramebuffers();
    _CreateCommandPools();
    _CreateCommandBuffers();
    _CreateSyncObjects();

    MLC_INFO("Vulkan Initialization: Success");
}

void VulkanManager::ShutDown()
{
    for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
    {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], MLC_VULKAN_ALLOCATOR);
        m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
    }
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], MLC_VULKAN_ALLOCATOR);
        vkDestroyFence(m_device, m_inFlightFences[i], MLC_VULKAN_ALLOCATOR);
        m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
        m_inFlightFences[i] = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(m_device, m_swapChain, MLC_VULKAN_ALLOCATOR);
    m_swapChain = VK_NULL_HANDLE;
    vkDestroyCommandPool(m_device, m_graphicsCmdPool, MLC_VULKAN_ALLOCATOR);
    vkDestroyCommandPool(m_device, m_transferCmdPool, MLC_VULKAN_ALLOCATOR);
    m_graphicsCmdPool = VK_NULL_HANDLE;
    m_transferCmdPool = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < m_swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], MLC_VULKAN_ALLOCATOR);
        m_swapChainFramebuffers[i] = VK_NULL_HANDLE;
    }
    // TODO: Since it doesn't really mater what order resource is deleted
    // (thanks to VkDeviceWaitIdle)
    // maybe I should relocate pipeline cleanup to somewhere else
    vkDestroyDescriptorPool(m_device, m_descriptorPool, MLC_VULKAN_ALLOCATOR);
    m_descriptorPool = VK_NULL_HANDLE;
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, MLC_VULKAN_ALLOCATOR);
    m_descriptorSetLayout = VK_NULL_HANDLE;
    vkDestroyPipeline(m_device, m_graphicsPipeline, MLC_VULKAN_ALLOCATOR);
    m_graphicsPipeline = VK_NULL_HANDLE;
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, MLC_VULKAN_ALLOCATOR);
    m_pipelineLayout = VK_NULL_HANDLE;
    vkDestroyRenderPass(m_device, m_renderPass, MLC_VULKAN_ALLOCATOR);
    m_renderPass = VK_NULL_HANDLE;
    for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(m_device, m_swapChainImageViews[i], MLC_VULKAN_ALLOCATOR);
        m_swapChainImageViews[i] = VK_NULL_HANDLE;
    }
    vkDestroyDevice(m_device, MLC_VULKAN_ALLOCATOR);
    m_device = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(m_instance, m_surface, MLC_VULKAN_ALLOCATOR);
    m_surface = VK_NULL_HANDLE;
    if (ENABLE_VALIDATION_LAYERS)
    {
        _DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, MLC_VULKAN_ALLOCATOR);
        m_debugMessenger = VK_NULL_HANDLE;
    }
    vkDestroyInstance(m_instance, MLC_VULKAN_ALLOCATOR);
    m_instance = VK_NULL_HANDLE;

    MLC_INFO("Vulkan Deinitialization: Success");
}

void VulkanManager::Present()
{
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrameIndex], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device,
                                            m_swapChain,
                                            UINT64_MAX,
                                            m_imageAvailableSemaphores[m_currentFrameIndex],
                                            VK_NULL_HANDLE,
                                            &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _RecreateSwapChain();
    }
    else
    {
        MLC_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire next swap chain image.");
    }

    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrameIndex]);

    vkResetCommandBuffer(m_graphicsCmdBuffers[m_currentFrameIndex], 0);
    _RecordCommandBuffer(m_graphicsCmdBuffers[m_currentFrameIndex], imageIndex);

    std::array<VkSemaphore, 1> waitSemaphores = { m_imageAvailableSemaphores[m_currentFrameIndex] };  // waiting for next image
    std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkSemaphore, 1> signalSemaphores = { m_renderFinishedSemaphores[imageIndex] };

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = waitSemaphores.size(),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = waitStages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &m_graphicsCmdBuffers[m_currentFrameIndex],
        .signalSemaphoreCount = signalSemaphores.size(),
        .pSignalSemaphores = signalSemaphores.data()
    };

    result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrameIndex]);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer to queue.");

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = signalSemaphores.size(),
        .pWaitSemaphores = signalSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &m_swapChain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr  // Optional to check for result of every given swap chain
    };

    result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        _RecreateSwapChain();
    }
    else
    {
        MLC_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image.");
    }

    m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanManager::WaitIdle()
{
    vkDeviceWaitIdle(m_device);
}

void VulkanManager::ResizeFramebuffer()
{
    m_framebufferResized = true;
}

uint32_t VulkanManager::GetCurrentFrameInFlight() const
{
    return m_currentFrameIndex;
}

void VulkanManager::AllocateBuffer(GPUBuffer& buffer,
                                   VkDeviceSize size,
                                   VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags properties) const
{
    std::vector<uint32_t> queueFamilies {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.transferFamily.value()
    };
    VkBufferCreateInfo vertexBufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,  // buffer size is explicitly-defined
        .usage = usage,
        .sharingMode = 
            m_queueFamilyIndices.IsExclusive()
            ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount =
            m_queueFamilyIndices.IsExclusive()
            ? 0 : static_cast<uint32_t>(queueFamilies.size()),
        .pQueueFamilyIndices =
            m_queueFamilyIndices.IsExclusive()
            ? nullptr : queueFamilies.data()
    };

    VkResult result = vkCreateBuffer(m_device,
                                     &vertexBufferCreateInfo,
                                     MLC_VULKAN_ALLOCATOR,
                                     &buffer.m_handle);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create vertex buffer.");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer.m_handle, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = _FindMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    result = vkAllocateMemory(m_device, &allocInfo, MLC_VULKAN_ALLOCATOR, &buffer.m_memory);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to allocate vertex buffer memory.");

    VkBindBufferMemoryInfo bindBufferInfo;
    vkBindBufferMemory(m_device, buffer.m_handle, buffer.m_memory, 0);
    buffer.m_properties = properties;
}

void VulkanManager::DeallocateBuffer(GPUBuffer& buffer) const
{
    MLC_ASSERT(buffer.m_handle != VK_NULL_HANDLE, "Buffer handle is VK_NULL_HANDLE.");
    MLC_ASSERT(buffer.m_memory != VK_NULL_HANDLE, "Buffer memory is VK_NULL_HANDLE.");

    vkDestroyBuffer(m_device, buffer.m_handle, MLC_VULKAN_ALLOCATOR);
    buffer.m_handle = VK_NULL_HANDLE;
    vkFreeMemory(m_device, buffer.m_memory, MLC_VULKAN_ALLOCATOR);
    buffer.m_memory = VK_NULL_HANDLE;
}

void VulkanManager::UploadBuffer(const GPUBuffer& buffer, const void* data, size_t size) const
{
    void* mappedRegion;
    vkMapMemory(m_device, buffer.m_memory, 0, size, 0, &mappedRegion);
    memcpy(mappedRegion, data, size);
    vkUnmapMemory(m_device, buffer.m_memory);
}

void VulkanManager::CopyBuffer(const GPUBuffer& src, const GPUBuffer& dst, VkDeviceSize size) const
{
    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_transferCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    
    VkCommandBuffer copyCmdBuffer;
    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &copyCmdBuffer);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to allocate copy command buffer.");

    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer(copyCmdBuffer, &beginInfo);
    VkBufferCopy copyRegion {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    vkCmdCopyBuffer(copyCmdBuffer, src.m_handle, dst.m_handle, 1, &copyRegion);
    vkEndCommandBuffer(copyCmdBuffer);

    VkFence fence;
    CreateFences(&fence, 1, false);
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &copyCmdBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    vkQueueSubmit(m_transferQueue, 1, &submitInfo, fence);
    vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkFreeCommandBuffers(m_device, m_transferCmdPool, 1, &copyCmdBuffer);
    DestroyFences(&fence, 1);
}

void* VulkanManager::GetBufferMapping(const GPUBuffer& buffer,
                                      VkDeviceSize offset,
                                      VkDeviceSize size) const
{
    static constexpr VkMemoryPropertyFlags mandatoryMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    MLC_ASSERT((buffer.m_properties & mandatoryMemoryProperties) == mandatoryMemoryProperties,
               "Failed to get buffer mapping due to invalid buffer's memory properties.");

    void* mappedMemory;
    vkMapMemory(m_device, buffer.m_memory, offset, size, 0, &mappedMemory);

    return mappedMemory;
}

void VulkanManager::CreateShaderModule(VkShaderModule& shader_module, const std::vector<char>& bytecode) const
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = bytecode.size(),
        .pCode = reinterpret_cast<const uint32_t*>(bytecode.data())
    };

    VkResult result = vkCreateShaderModule(m_device,
                                           &shaderModuleCreateInfo,
                                           MLC_VULKAN_ALLOCATOR,
                                           &shader_module);
}

void VulkanManager::DestroyShaderModule(VkShaderModule& shader_module) const
{
    vkDestroyShaderModule(m_device, shader_module, MLC_VULKAN_ALLOCATOR);
    shader_module = VK_NULL_HANDLE;
}

void VulkanManager::CreateFences(VkFence* fences, uint32_t amount, bool signaled) const
{
    VkResult result;
    VkFenceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u
    };

    for (uint32_t i = 0; i < amount; i++)
    {
        result = vkCreateFence(m_device, &createInfo, MLC_VULKAN_ALLOCATOR, &fences[i]);

        MLC_ASSERT(result == VK_SUCCESS, "Failed to create fence.");
    }
}

void VulkanManager::DestroyFences(VkFence* fences, uint32_t amount) const
{
    for (uint32_t i = 0; i < amount; i++)
    {
        vkDestroyFence(m_device, fences[i], MLC_VULKAN_ALLOCATOR);
    }
}

void VulkanManager::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr  // related to sampler-related descriptors
    };
    VkDescriptorSetLayoutCreateInfo uboCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };

    VkResult result = vkCreateDescriptorSetLayout(m_device,
                                                  &uboCreateInfo,
                                                  MLC_VULKAN_ALLOCATOR,
                                                  &m_descriptorSetLayout);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set layout.");
}

void VulkanManager::CreateDescriptorSets()
{
    std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts;
    layouts.fill(m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts.data()
    };

    vkAllocateDescriptorSets(m_device, &allocateInfo, m_descriptorSets.data());
}

void VulkanManager::DescriptorSetBindUBO(const std::array<GPUBuffer, MAX_FRAMES_IN_FLIGHT>& ubo_buffers,
                                         VkDeviceSize offset,
                                         VkDeviceSize size_per_buffer) const
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo {
            .buffer = ubo_buffers[i].m_handle,
            .offset = offset,
            .range = size_per_buffer
        };
        VkWriteDescriptorSet descriptorWrite {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,  // It is possible to update multiple descriptors at once in an array
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = nullptr
        };
        
        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanManager::CreateGraphicsPipeline(GraphicsPipelineConfig& pipeline_config)
{
    // ----- Programmable stages of the pipeline -----
    
    m_pipelineConfig = pipeline_config;

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = pipeline_config.shader->m_vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr  // this can specify constants inside the shader
    };

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = pipeline_config.shader->m_fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr  // this can specify constants inside the shader
    };

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        vertShaderStageCreateInfo,
        fragShaderStageCreateInfo
    };

    // ----- Fixed stages -----

    // Dynamic states
    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Vertex input
    // TODO
    VkVertexInputBindingDescription bindingDesc =
        pipeline_config.vertexArrays->at(0).GetBindingDescription();
    std::array<VkVertexInputAttributeDescription, 2> attribDescs =
        pipeline_config.vertexArrays->at(0).GetAttribDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDesc,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size()),
        .pVertexAttributeDescriptions = attribDescs.data()
    };

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,  // VK_TRUE might be useful for shadow maps
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,  // depth bias may be useful for shadow maps
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    // Depth buffer (later, // TODO)
    // Stencil buffer (later, // TODO)

    // Color blending (// TODO)
    VkPipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,  // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }  // Optional
    };

    // ----- Pipeline Layout -----

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &m_descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, MLC_VULKAN_ALLOCATOR, &m_pipelineLayout);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout.");

    // ----- Graphics Pipeline -----

    VkGraphicsPipelineCreateInfo pipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multisamplingCreateInfo,
        .pDepthStencilState = nullptr,  // Optional
        .pColorBlendState = &colorBlendingCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };
    // See https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
    // for the last two parameters
    
    result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, MLC_VULKAN_ALLOCATOR, &m_graphicsPipeline);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create graphics pipeline.");
}

MLC_NODISCARD std::vector<const char*> VulkanManager::_GLFWGetRequiredExtensions()
{
    uint32_t glfwExtensionCount;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    MLC_ASSERT(glfwExtensions != NULL, "Failed to get required extensions.");

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (ENABLE_VALIDATION_LAYERS)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensions;
}

MLC_NODISCARD bool VulkanManager::_CheckValidationLayerSupport()
{
    for (uint32_t i = 0; i < VALIDATION_LAYERS.size(); i++)
    {
        s_supportedLayers[VALIDATION_LAYERS[i]] = true;
    }

    uint32_t supportedLayerCount;
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
    std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

    for (const char* validation_layer : VALIDATION_LAYERS)
    {
        if (!s_supportedLayers[validation_layer]) return false;
    }
    return true;
}

void VulkanManager::_CreateInstance()
{
    VkApplicationInfo vkApplicationInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Malic",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "Malic Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    std::vector<const char*> requiredExtensions = _GLFWGetRequiredExtensions();

    uint32_t supportedExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

    fmt::print("{} supported Vulkan extensions.\n", supportedExtensionCount);
    for (const VkExtensionProperties& extension : supportedExtensions)
    {
        // fmt::print("{} [{}]\n", strlen(extension.extensionName), extension.specVersion);
        s_supportedExtensions[extension.extensionName] = true;
    }
    fmt::print("Required Vulkan extensions ({}): \n", requiredExtensions.size());
    for (uint32_t i = 0; i < requiredExtensions.size(); i++)
    {
        fmt::print("    {} [{}]\n",
            requiredExtensions[i],
            s_supportedExtensions[requiredExtensions[i]] ? "SUPPORTED" : "UNSUPPORTED"
        );
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugCallback,
        .pUserData = nullptr
    };

    VkInstanceCreateInfo vkInstanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsMessengerCreateInfo,
        .flags = 0,
        .pApplicationInfo = ENABLE_VALIDATION_LAYERS ? &vkApplicationInfo : nullptr,
        .enabledLayerCount = ENABLE_VALIDATION_LAYERS ? static_cast<uint32_t>(VALIDATION_LAYERS.size()) : 0, 
        .ppEnabledLayerNames = ENABLE_VALIDATION_LAYERS ? VALIDATION_LAYERS.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    VkResult result = vkCreateInstance(&vkInstanceCreateInfo, MLC_VULKAN_ALLOCATOR, &m_instance);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create VkInstance.");
}

VkResult VulkanManager::_CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;

    return func(instance, pCreateInfo, pAllocator, pMessenger);
}

void VulkanManager::_DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
    MLC_ASSERT(func != nullptr, "Failed to delete Debug Messenger.");

    func(instance, debugMessenger, pAllocator);
}

void VulkanManager::_CreateSurface()
{
    // VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {
    //     .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    //     .pNext = nullptr,
    //     .flags = 0,
    //     .hinstance = GetModuleHandle(nullptr),
    //     .hwnd = glfwGetWin32Window(glfwGetCurrentContext())
    // };

    // VkResult result = vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, MLC_VULKAN_ALLOCATOR, &m_surface);
    // MLC_ASSERT(result == VK_SUCCESS, "Failed to create window surface.");
    
    VkResult result = glfwCreateWindowSurface(m_instance, m_window, MLC_VULKAN_ALLOCATOR, &m_surface);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create window surface.");
}

void VulkanManager::_SetupDebugMessenger()
{
    if (!ENABLE_VALIDATION_LAYERS) return;

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugCallback,
        .pUserData = nullptr
    };

    VkResult result = _CreateDebugUtilsMessengerEXT(m_instance,
                                                    &debugMessengerCreateInfo,
                                                    MLC_VULKAN_ALLOCATOR,
                                                    &m_debugMessenger);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to set up debug messenger.");
}

VulkanManager::QueueFamiliesIndices VulkanManager::_FindQueueFamilies(const VkPhysicalDevice& device)
{
    QueueFamiliesIndices familyIndices;
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    std::vector<VkQueueFlags> queueFamiliesFlags(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        queueFamiliesFlags[i] = queueFamilies[i].queueFlags;
    }

    // Find all needed queue families, might just be one family
    for (int i = 0; i < queueFamilyCount; i++)
    {
        uint32_t thisQueueCount = 0;
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && thisQueueCount < queueFamilies[i].queueCount)
        {
            familyIndices.graphicsFamily = i;
            if (presentSupport && thisQueueCount < queueFamilies[i].queueCount)  // might be the same queue family as graphics queues
            {
                familyIndices.presentFamily = i;
            }
            if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && thisQueueCount < queueFamilies[i].queueCount)
            {
                familyIndices.transferFamily = i;
            }
        }

        // Found all required queue families
        if (familyIndices.IsComplete()) break;
    }

    // Find other queue families that may be specialized
    static auto findBestQueueFamilyIndex = [](const std::vector<VkQueueFlags>& queueFamiliesFlags,
                                       VkQueueFlags targetFlags)
    {
        uint32_t index;
        VkQueueFlags closestBit = VK_QUEUE_FLAG_BITS_MAX_ENUM;
        for (uint32_t i = 0; i < queueFamiliesFlags.size(); i++)
        {
            if (queueFamiliesFlags[i] - targetFlags < closestBit && (queueFamiliesFlags[i] & targetFlags) == targetFlags)
            {
                closestBit = queueFamiliesFlags[i] - targetFlags;
                index = i;
            }
        }
        return index;
    };
    uint32_t transferQueueFamilyIndex = findBestQueueFamilyIndex(queueFamiliesFlags, VK_QUEUE_TRANSFER_BIT);
    if (transferQueueFamilyIndex < queueFamilyCount)
    {
        familyIndices.transferFamily = transferQueueFamilyIndex;
    }

    return familyIndices;
}

bool VulkanManager::_CheckDeviceExtensionSupport(const VkPhysicalDevice& physical_device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string_view> requiredDeviceExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredDeviceExtensions.erase(extension.extensionName);
    }

    return requiredDeviceExtensions.empty();
}

VulkanManager::SwapChainSupportDetails VulkanManager::_QuerySwapChainSupport(const VkPhysicalDevice& physical_device)
{
    SwapChainSupportDetails details;

    // Query capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_surface, &details.capabilities);

    // Query formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &formatCount, nullptr);
    MLC_ASSERT(formatCount != 0, "Failed to query for pixel format count.");
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &formatCount, details.formats.data());

    // Query present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &presentModeCount, nullptr);
    MLC_ASSERT(presentModeCount != 0, "Failed to query for present mode count.");
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &presentModeCount, details.presentModes.data());

    return details;
}

bool VulkanManager::_IsPhysicalDeviceSuitable(const VkPhysicalDevice& physical_device)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceProperties(physical_device, &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physical_device, &physicalDeviceFeatures);
    fmt::print("{}\n", physicalDeviceProperties.deviceName);

    // Each time _IsPhysicalDeviceSuitable is run, m_queueFamilyIndices will be
    // set to another set of queue familes.
    // When the function returns true, it's always the suitable set.
    m_queueFamilyIndices = _FindQueueFamilies(physical_device);

    bool extensionsSupported = _CheckDeviceExtensionSupport(physical_device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupportDetails = _QuerySwapChainSupport(physical_device);
        swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
    }

    return m_queueFamilyIndices.IsComplete() && extensionsSupported && swapChainAdequate;
}

void VulkanManager::_PickPhysicalDevice()
{
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    MLC_ASSERT(deviceCount > 0, "Failed to find GPU with Vulkan support.");
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

    for (VkPhysicalDevice& physical_device : physicalDevices)
    {
        if (_IsPhysicalDeviceSuitable(physical_device))
        {
            m_physicalDevice = physical_device;
            break;
        }
    }

    MLC_ASSERT(m_physicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU.");
}

void VulkanManager::_CreateLogicalDevice()
{
    // Queue create info and device features
    float queuePriority = 1.0f;

    // Repeat values are deduplicated
    const std::set<uint32_t> uniqueQueueFamilies {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value(),
        m_queueFamilyIndices.transferFamily.value()
    };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queue_family : uniqueQueueFamilies)
    {
        queueCreateInfos.push_back(VkDeviceQueueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    // Device creation here
    VkDeviceCreateInfo deviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = ENABLE_VALIDATION_LAYERS ? static_cast<uint32_t>(VALIDATION_LAYERS.size()) : 0,
        .ppEnabledLayerNames = ENABLE_VALIDATION_LAYERS ? VALIDATION_LAYERS.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
        .pEnabledFeatures = &deviceFeatures
    };

    // Create logical device
    VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, MLC_VULKAN_ALLOCATOR, &m_device);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create logical device.");

    // Note: Get queues in main Init function
}

void VulkanManager::_GetQueues()
{
    // If the graphics and present queues are the same, they'll have the same address
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.transferFamily.value(), 0, &m_transferQueue);
}

VkExtent2D VulkanManager::_ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        VkExtent2D actualExtent {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

VkSurfaceFormatKHR VulkanManager::_ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];  // could be chosen based on given assessed rank
}

VkPresentModeKHR VulkanManager::_ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& present_modes)
{
    for (const VkPresentModeKHR& present_mode : present_modes)
    {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanManager::_CreateSwapChain()
{
    VkExtent2D extent;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    uint32_t imageCount;  // how many images in swap chain
    std::array<uint32_t, 3> queueFamilyIndices {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value(),
        m_queueFamilyIndices.transferFamily.value()
    };
    bool graphicsQueueIsPresentQueue = queueFamilyIndices[0] == queueFamilyIndices[1];    

    SwapChainSupportDetails swapChainSupport = _QuerySwapChainSupport(m_physicalDevice);
    extent = _ChooseSwapChainExtent(swapChainSupport.capabilities);
    surfaceFormat = _ChooseSwapChainSurfaceFormat(swapChainSupport.formats);
    presentMode = _ChooseSwapChainPresentMode(swapChainSupport.presentModes);

    imageCount = swapChainSupport.capabilities.minImageCount + 1;  // +1 to ensure available images
                                                                   // to render to
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)  // if maxImageCount = 0
    {                                                              // there are no limits
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Swap chain creation here
    VkSwapchainCreateInfoKHR swapChainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = m_surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // TODO: VK_IMAGE_USAGE_TRANSFER_DST_BIT for post-processing
        .imageSharingMode = graphicsQueueIsPresentQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = graphicsQueueIsPresentQueue ? 0u : 3u,
        .pQueueFamilyIndices = queueFamilyIndices.data(),
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,  // WINDOW opacity, which is interesting
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    VkResult result = vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, MLC_VULKAN_ALLOCATOR, &m_swapChain);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create swap chain.");

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, nullptr);
    m_swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, m_swapChainImages.data());
}

void VulkanManager::_CreateImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_swapChainImageFormat,
            .components = VkComponentMapping {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkResult result = vkCreateImageView(m_device,
                                            &imageViewCreateInfo,
                                            MLC_VULKAN_ALLOCATOR,
                                            &m_swapChainImageViews[i]);
        MLC_ASSERT(result == VK_SUCCESS, "Failed to create swap chain image views.");
    }
}

void VulkanManager::_CreateRenderPass()
{
    VkAttachmentDescription colorAttachment {
        .flags = 0,
        .format = m_swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef {
        .attachment = 0,  // attachment index in array
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    VkSubpassDependency subpassDependency {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency
    };

    VkResult result = vkCreateRenderPass(m_device,
                                         &renderPassCreateInfo,
                                         MLC_VULKAN_ALLOCATOR,
                                         &m_renderPass);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create render pass.");                                        
}

void VulkanManager::_CreateFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
    
    for (uint32_t i = 0; i < m_swapChainImageViews.size(); i++)
    {
        VkImageView attachment = m_swapChainImageViews[i];
        VkFramebufferCreateInfo framebufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = m_renderPass,
            .attachmentCount = 1,
            .pAttachments = &attachment,
            .width = m_swapChainExtent.width,
            .height = m_swapChainExtent.height,
            .layers = 1
        };

        VkResult result = vkCreateFramebuffer(m_device,
                                              &framebufferCreateInfo,
                                              MLC_VULKAN_ALLOCATOR,
                                              &m_swapChainFramebuffers[i]);
        MLC_ASSERT(result == VK_SUCCESS, "Failed to create framebuffer.");
    }
}

void VulkanManager::_CreateCommandPools()
{
    VkCommandPoolCreateInfo graphicsCmdPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value()
    };

    VkCommandPoolCreateInfo transferCmdPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_queueFamilyIndices.transferFamily.value()
    };

    VkResult result;
    result = vkCreateCommandPool(m_device,
                                 &graphicsCmdPoolCreateInfo,
                                 MLC_VULKAN_ALLOCATOR,
                                 &m_graphicsCmdPool);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create Graphics command pool.");
    result = vkCreateCommandPool(m_device,
                                 &transferCmdPoolCreateInfo,
                                 MLC_VULKAN_ALLOCATOR,
                                 &m_transferCmdPool);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create Transfer command pool.");
}

void VulkanManager::_CreateCommandBuffers()
{
    m_graphicsCmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_transferCmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo graphicsCmdBufferAllocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_graphicsCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(m_graphicsCmdBuffers.size())
    };

    VkCommandBufferAllocateInfo transferCmdBufferAllocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_transferCmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(m_transferCmdBuffers.size())
    };

    VkResult result;
    result = vkAllocateCommandBuffers(m_device, &graphicsCmdBufferAllocInfo, m_graphicsCmdBuffers.data());
    MLC_ASSERT(result == VK_SUCCESS, "Failed to allocate Graphics command buffers.");
    result = vkAllocateCommandBuffers(m_device, &transferCmdBufferAllocInfo, m_transferCmdBuffers.data());
    MLC_ASSERT(result == VK_SUCCESS, "Failed to allocate Transfer command buffers.");
}

void VulkanManager::_RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t swch_image_index)
{
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr  // optional
    };

    VkResult result = vkBeginCommandBuffer(command_buffer, &beginInfo);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create command buffer.");

    VkClearValue clearValue {
        .color = { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    VkRenderPassBeginInfo renderPassBeginInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = m_renderPass,
        .framebuffer = m_swapChainFramebuffers[swch_image_index],
        .renderArea = {
            .offset = { 0, 0 },
            .extent = m_swapChainExtent
        },
        .clearValueCount = 1,
        .pClearValues = &clearValue
    };
    vkCmdBeginRenderPass(command_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    VkViewport viewport {
        .x = 0,
        .y = static_cast<float>(m_swapChainExtent.height),
        .width = static_cast<float>(m_swapChainExtent.width),
        .height = -static_cast<float>(m_swapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor {
        .offset = { 0, 0 },
        .extent = m_swapChainExtent
    };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    const std::vector<VertexArray>* vertexArrays = m_pipelineConfig.vertexArrays;
    std::array<VkBuffer, 1> vertexBuffers = { vertexArrays->at(0).GetVertexBuffer().m_handle };
    std::array<VkDeviceSize, 1> offsets = { 0 }; 
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertexBuffers.data(), offsets.data());
    vkCmdBindIndexBuffer(command_buffer, vertexArrays->at(0).GetIndexBuffer().m_handle, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout,
                            0,
                            1,
                            &m_descriptorSets[m_currentFrameIndex],
                            0,
                            nullptr);

    vkCmdDrawIndexed(command_buffer,
                     vertexArrays->at(0).GetIndicesCount(),
                     1,
                     0,
                     0,
                     0);  // TODO

    vkCmdEndRenderPass(command_buffer);

    result = vkEndCommandBuffer(command_buffer);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to record command buffer.");
}

void VulkanManager::_CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize {  // TODO: Maybe add more types
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    VkResult result = vkCreateDescriptorPool(m_device,
                                             &descriptorPoolCreateInfo,
                                             MLC_VULKAN_ALLOCATOR,
                                             &m_descriptorPool);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to creat descriptor pool.");
}

void VulkanManager::_CreateSyncObjects()
{
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(m_swapChainImages.size());
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0  
    };
    VkFenceCreateInfo fenceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        bool result;
        result = vkCreateSemaphore(m_device,
                                   &semaphoreCreateInfo,
                                   MLC_VULKAN_ALLOCATOR,
                                   &m_imageAvailableSemaphores[i]) == VK_SUCCESS
              && vkCreateFence(m_device,
                               &fenceCreateInfo,
                               MLC_VULKAN_ALLOCATOR,
                               &m_inFlightFences[i]) == VK_SUCCESS;
        MLC_ASSERT(result, "Failed to create sync objects.");
    }
    for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
    {
        VkResult result = vkCreateSemaphore(m_device,
                                &semaphoreCreateInfo,
                                MLC_VULKAN_ALLOCATOR,
                                &m_renderFinishedSemaphores[i]);
        MLC_ASSERT(result == VK_SUCCESS, "Failed to create sync objects.");
    }
}

void VulkanManager::_RecreateSwapChain()
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    // This function may be called whilst rendering is happening
    // so some we need to wait for everything to finish before
    // recreating the swap chain and its resources
    vkDeviceWaitIdle(m_device);

    for (uint32_t i = 0; i < m_swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], MLC_VULKAN_ALLOCATOR);
    }
    for (uint32_t i = 0; i < m_swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(m_device, m_swapChainImageViews[i], MLC_VULKAN_ALLOCATOR);
    }
    vkDestroySwapchainKHR(m_device, m_swapChain, MLC_VULKAN_ALLOCATOR);

    _CreateSwapChain();
    _CreateImageViews();
    _CreateFramebuffers();
}

uint32_t VulkanManager::_FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    MLC_ASSERT(false, "Failed to find suitable device memory type.");
    return static_cast<uint32_t>(-1);
}

MLC_NAMESPACE_END