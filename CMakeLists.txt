cmake_minimum_required(VERSION 3.10)
project(TuringMachine)

set(CMAKE_CXX_STANDARD 17)

# Include ImGui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

# Find GLFW and OpenGL
find_package(OpenGL REQUIRED)
# In case GLFW isn't found by standard means, we won't fail hard, 
# but let's try to find it.
find_package(glfw3 QUIET)
if (NOT glfw3_FOUND)
    # macOS fallback or explicit
    set(GLFW_LIBRARIES "-lglfw")
else()
    set(GLFW_LIBRARIES glfw)
endif()

file(GLOB SOURCES 
    "*.cpp"
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_executable(turing_machine ${SOURCES})

if (APPLE)
    target_link_libraries(turing_machine "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" ${GLFW_LIBRARIES})
else()
    target_link_libraries(turing_machine OpenGL::GL ${GLFW_LIBRARIES})
endif()
