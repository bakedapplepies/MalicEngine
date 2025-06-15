#pragma once

#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

constexpr std::array VALIDATION_LAYERS {
    "VK_LAYER_KHRONOS_validation"  // common validation layers bundled into one
};

constexpr std::array DEVICE_EXTENSIONS {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    // VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME
};

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
constexpr glm::vec3 VEC3_UP = glm::vec3(0.0f, 1.0f, 0.0f);

MLC_NAMESPACE_END