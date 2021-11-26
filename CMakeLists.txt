cmake_minimum_required(VERSION 3.19)
project(VulkanExample)

set(CMAKE_CXX_STANDARD 20)

include("cmake/build_glsls.cmake")

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb.h")

add_executable(${PROJECT_NAME} "main.cpp" "application.cpp")
compile_shader(${PROJECT_NAME}
    ENV vulkan
    FORMAT bin
    SOURCES
        "shaders/shader.frag"
        "shaders/shader.vert"
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm Vulkan::Vulkan imgui::imgui tinyobjloader::tinyobjloader)
target_compile_definitions(${PROJECT_NAME} PRIVATE REPO_HOME="${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(${PROJECT_NAME} PRIVATE ${STB_INCLUDE_DIRS})