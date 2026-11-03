set(CMAKE_TOOLCHAIN_FILE "C:/bin/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")
cmake_minimum_required(VERSION 3.20)
project(OpenGLCourseWork)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Dear ImGui лежит в папке imgui-master рядом с проектом
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui-master)

add_executable(app
    main.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
)

target_include_directories(app PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)

target_link_libraries(app PRIVATE glfw glad::glad glm::glm OpenGL::GL)