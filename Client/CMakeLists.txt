cmake_minimum_required(VERSION 3.26.0)
project(Client CXX C)

add_subdirectory(src)

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