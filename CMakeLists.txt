cmake_minimum_required(VERSION 3.0)
set(TARGET_NAME "phong-shading")

project(${TARGET_NAME})

add_executable(${TARGET_NAME} src/main.cpp)
add_subdirectory(imgui)

target_link_libraries(${TARGET_NAME} PUBLIC imgui -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread)
target_include_directories(${TARGET_NAME} PUBLIC imgui/include)
