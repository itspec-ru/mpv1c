cmake_minimum_required(VERSION 3.16)
project(mpv1c LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SOURCES
    src/mpv1c_component.cpp
)

add_library(mpv1c SHARED ${SOURCES})
target_include_directories(mpv1c PRIVATE include)

# ==============================================================================
# ПОДКЛЮЧЕНИЕ libmpv
# ==============================================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED mpv)
target_include_directories(mpv1c PRIVATE ${MPV_INCLUDE_DIRS})
target_link_libraries(mpv1c PRIVATE ${MPV_LIBRARIES})

# ==============================================================================
# НАСТРОЙКИ ПЛАТФОРМ
# ==============================================================================
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(mpv1c PROPERTIES PREFIX "")
    target_compile_definitions(mpv1c PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(mpv1c PRIVATE user32) # Нужно для поиска окон в Windows
    
elseif(UNIX AND NOT APPLE)
    set_target_properties(mpv1c PROPERTIES PREFIX "lib")
    
    # Подключаем X11 для поиска окон в Linux
    find_package(X11 REQUIRED)
    target_include_directories(mpv1c PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(mpv1c PRIVATE ${X11_LIBRARIES})
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.script")
        set_target_properties(mpv1c PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/version.script")
    endif()
endif()
