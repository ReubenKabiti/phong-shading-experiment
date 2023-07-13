file(GLOB SRC_FILES "src/*.cpp")
add_library(imgui SHARED ${SRC_FILES})
target_include_directories(imgui PUBLIC include)
