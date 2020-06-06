cmake_minimum_required(VERSION 3.17)
project(OpenGL_posteffect_tutorial)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main GLEW::GLEW glfw glm)
