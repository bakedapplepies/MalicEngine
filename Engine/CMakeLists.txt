cmake_minimum_required(VERSION 3.26.0)
project(MalicEngine CXX C)

set(VULKAN_LIBS "C:/VulkanSDK/1.4.309.0/Lib" CACHE INTERNAL "")
set(VULKAN_HEADERS "C:/VulkanSDK/1.4.309.0/Include" CACHE INTERNAL "")
set(MALIC_HEADERS
    ${VULKAN_HEADERS}
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${GLFW_INSTALL_DIR}/install/include
    ${GLM_INSTALL_DIR}/install/include
    ${FMT_INSTALL_DIR}/install/include
CACHE INTERNAL "")

add_subdirectory(vendor)
add_subdirectory(src)

set_target_properties(LMalicEngine PROPERTIES VERIFY_INTERFACE_HEADER_SETS true)

# Build-type configurations
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_link_options(LMalicEngine INTERFACE -fsanitize=address -g)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(LMalicEngine INTERFACE MLC_RELEASE)
endif()