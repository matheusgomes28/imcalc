cmake_minimum_required(VERSION 3.16)
project(ImCalc LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

find_package(imgui REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(ImCalc
  main.cpp
  imgui_impl_glfw.cpp
  imgui_impl_opengl3.cpp)

target_link_libraries(ImCalc PRIVATE imgui::imgui glfw::glfw GLEW::GLEW)
