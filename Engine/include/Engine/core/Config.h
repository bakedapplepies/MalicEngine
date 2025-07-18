#pragma once

#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

const std::array VALIDATION_LAYERS {
    "VK_LAYER_KHRONOS_validation"  // common validation layers bundled into one
};

const std::array DEVICE_EXTENSIONS {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME
    // VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME
};

const size_t MAX_PUSH_CONSTANTS_SIZE = 128;

const uint32_t VERTEX_ATTRIB_INDEX_POSITION = 0;
const uint32_t VERTEX_ATTRIB_INDEX_COLOR = 1;
const uint32_t VERTEX_ATTRIB_INDEX_UV = 2;

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t MAX_DESCRIPTOR_SETS = MAX_FRAMES_IN_FLIGHT;
const uint32_t MAX_DESCRIPTOR_PER_SET_UNIFORM_BUFFER = 1;
const uint32_t MAX_DESCRIPTOR_PER_SET_COMBINED_SAMPLER = 1;
const glm::vec3 VEC3_UP = glm::vec3(0.0f, 1.0f, 0.0f);

MLC_NAMESPACE_END