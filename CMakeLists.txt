cmake_minimum_required(VERSION 3.19)
project(VulkanExample)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm Vulkan::Vulkan)
target_compile_definitions(${PROJECT_NAME} PRIVATE REPO_HOME="${CMAKE_CURRENT_SOURCE_DIR}/")