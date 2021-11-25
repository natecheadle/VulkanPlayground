cmake_minimum_required(VERSION 3.19)
project(VulkanExample)

set(CMAKE_CXX_STANDARD 20)

include("cmake/build_glsls.cmake")

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME} "main.cpp")
compile_shader(${PROJECT_NAME}
    ENV vulkan
    FORMAT bin
    SOURCES
        "shaders/vertex.vert"
        "shaders/fragment.frag"
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm Vulkan::Vulkan)
target_compile_definitions(${PROJECT_NAME} PRIVATE REPO_HOME="${CMAKE_CURRENT_SOURCE_DIR}/")