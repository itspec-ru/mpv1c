cmake_minimum_required(VERSION 3.16)
project(mpv1c LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Скрываем все символы по умолчанию
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(mpv1c SHARED
    src/addin_exports.cpp
    src/mpv1c_component.cpp
)

target_include_directories(mpv1c PRIVATE include)

# Платформенные настройки
if(WIN32)
    # На Windows префикс "" чтобы получить mpv1c.dll
    set_target_properties(mpv1c PROPERTIES PREFIX "")
    target_compile_definitions(mpv1c PRIVATE _WIN32_WINNT=0x0601)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Кросскомпиляция из Linux в Windows
    set_target_properties(mpv1c PROPERTIES PREFIX "")
    target_compile_definitions(mpv1c PRIVATE _WIN32_WINNT=0x0601)
elseif(UNIX AND NOT APPLE)
    # Нативная сборка под Linux
    set_target_properties(mpv1c PROPERTIES PREFIX "lib")
    target_compile_definitions(mpv1c PRIVATE __linux__)
elseif(APPLE)
    set_target_properties(mpv1c PROPERTIES PREFIX "lib")
    target_compile_definitions(mpv1c PRIVATE __APPLE__)
endif()

# Для отладки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(mpv1c PRIVATE DEBUG)
endif()
