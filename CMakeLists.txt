# Подключаем vcpkg до project(): сначала из VCPKG_ROOT, затем из C:/bin/vcpkg
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    elseif(EXISTS "C:/bin/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "C:/bin/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
endif()
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")
endif()

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

# После сборки копируем папку molecules рядом с exe, чтобы программа находила .xyz файлы
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/molecules $<TARGET_FILE_DIR:app>/molecules
)
