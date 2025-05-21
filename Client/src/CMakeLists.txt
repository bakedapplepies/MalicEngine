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

# foreach (element ${MALIC_HEADERS})
#     target_include_directories(${PROJECT_NAME} PRIVATE ${element})
# endforeach()