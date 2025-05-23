cmake_minimum_required(VERSION 3.26.0)
project(MalicEngine CXX C)

# Make sure binary directory is not the same as source directory
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

add_subdirectory(Engine)
add_subdirectory(Client)

# Build-type configurations
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_options(-O0 -fsanitize=address -g)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_compile_options(-O3)
    add_link_options(-O3)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    message("MinSizeRel config not supported yet!")
    
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    message("RelWithDebInfo config not supported yet!")

endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/.vscode/settings.json")
    message("Altering .vscode/settings.json")
    execute_process(
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/.vscode
        COMMAND_ECHO STDOUT
        COMMAND py compile_commands_change.py ${CMAKE_BUILD_TYPE}
    )
endif()

if (WIN32)
    execute_process(
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Engine/scripts
        COMMAND_ECHO STDOUT
        COMMAND compile_shaders.bat
    )
elseif(UNIX)
    execute_process(
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Engine/scripts
        COMMAND_ECHO STDOUT
        COMMAND ./compile_shaders.sh
    )
endif()