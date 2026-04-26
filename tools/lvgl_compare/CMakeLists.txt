cmake_minimum_required(VERSION 3.16)

project(lvgl_text_compare C)

include(FetchContent)
find_package(SDL2 REQUIRED)

set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE PATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_SET_CONFIG_OPTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    lvgl
    URL https://codeload.github.com/lvgl/lvgl/tar.gz/refs/tags/v9.5.0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(lvgl)

add_executable(lvgl_text_compare
    main.c
)

target_link_libraries(lvgl_text_compare PRIVATE lvgl::lvgl SDL2::SDL2 m)
