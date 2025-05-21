cmake_minimum_required(VERSION 3.26.0)
project(MalicEngine CXX C)

find_package(Vulkan REQUIRED)
set(VULKAN_LIBS "C:/VulkanSDK/1.4.309.0/Lib" CACHE INTERNAL "")
set(VULKAN_HEADERS "C:/VulkanSDK/1.4.309.0/Include" CACHE INTERNAL "")
message("Vulkan Version " ${Vulkan_VERSION})

set(MALIC_HEADERS
    ${VULKAN_HEADERS}
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${GLFW_INSTALL_DIR}/install/include
    ${GLM_INSTALL_DIR}/install/include
    ${FMT_INSTALL_DIR}/install/include
CACHE INTERNAL "")

add_subdirectory(vendor)
add_subdirectory(src)

set_target_properties(LMalicEngine PROPERTIES VERIFY_INTERFACE_HEADER_SETS true)

target_compile_definitions(LMalicEngine PRIVATE
    MLC_ROOT_DIR="${CMAKE_SOURCE_DIR}"
    GLM_ENABLE_EXPERIMENTAL    
)

# Build-type configurations
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_link_options(LMalicEngine PRIVATE -fsanitize=address -g)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(LMalicEngine PRIVATE MLC_RELEASE)
endif()