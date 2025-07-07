cmake_minimum_required(VERSION 3.26.0)

add_executable(${PROJECT_NAME}
    main.cpp
    Core.cpp
    Application.cpp
    Input.cpp
    Camera.cpp
    Model.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${MALIC_HEADERS}
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    LMalicEngine
)

# target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_LEFT_HANDED)

# Build-type configurations
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g -fsanitize=address)
    target_link_options(${PROJECT_NAME} PRIVATE -g -fsanitize=address)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    target_link_options(${PROJECT_NAME} PRIVATE -O3)

elseif(${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel")
    message("MinSizeRel config not supported yet!")
    
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    message("RelWithDebInfo config not supported yet!")

endif()