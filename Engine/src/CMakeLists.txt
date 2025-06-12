cmake_minimum_required(VERSION 3.26.0)
project(MalicEngine CXX C)

add_library(LMalicEngine
    core/Debug.cpp
    core/Defines.cpp
    core/Logging.cpp
    GPUBuffer.cpp
    VulkanManager.cpp
    VertexArray.cpp
    Shader.cpp
    Malic.cpp
)

add_library(LMalicEngineDeps INTERFACE)

add_dependencies(LMalicEngineDeps GLFW_EXTERN)
add_dependencies(LMalicEngineDeps GLM_EXTERN)
add_dependencies(LMalicEngineDeps FMT_EXTERN)
add_dependencies(LMalicEngineDeps ASSIMP_EXTERN)

if (WIN32)
    target_link_directories(LMalicEngineDeps INTERFACE
        ${VULKAN_LIBS}
        ${GLFW_INSTALL_DIR}/install/lib
        ${GLM_INSTALL_DIR}/install/lib
        ${FMT_INSTALL_DIR}/install/lib
    )
    target_link_libraries(LMalicEngineDeps INTERFACE
        glfw3
        glm
        $<IF:$<CONFIG:Debug>, fmtd, fmt>
        Vulkan::Vulkan
    )
endif()
        
if (UNIX)
    target_link_directories(LMalicEngineDeps INTERFACE
        ${VULKAN_LIBS}
        ${GLFW_INSTALL_DIR}/install/lib64
        ${GLM_INSTALL_DIR}/install/lib64
        ${FMT_INSTALL_DIR}/install/lib64
    )
    target_link_libraries(LMalicEngineDeps INTERFACE
        Vulkan::Vulkan
        glfw3
        $<IF:$<CONFIG:Debug>, fmtd, fmt>
        # dl
        # pthread
        # Xrandr
        # Xi
    )
endif()

target_include_directories(LMalicEngine PRIVATE
    ${MALIC_HEADERS}
)

# target_link_directories(LMalicEngine PRIVATE
#     ${VULKAN_LIBS}
#     ${GLFW_INSTALL_DIR}/install/lib
#     ${GLM_INSTALL_DIR}/install/lib
#     ${FMT_INSTALL_DIR}/install/lib
# )

target_link_libraries(LMalicEngine PRIVATE
    LMalicEngineDeps
)