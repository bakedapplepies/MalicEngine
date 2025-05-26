cmake_minimum_required(VERSION 3.26.0)
project(Client CXX C)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${MALIC_HEADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    LMalicEngine
)

# Build-type configurations
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
    target_link_options(${PROJECT_NAME} PRIVATE -g)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    target_link_options(${PROJECT_NAME} PRIVATE -O3)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    message("MinSizeRel config not supported yet!")
    
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    message("RelWithDebInfo config not supported yet!")

endif()