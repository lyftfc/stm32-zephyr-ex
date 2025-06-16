cmake_minimum_required(VERSION 3.13.1)

if(NOT DEFINED BOARD)
    set(BOARD stm32f103c6_blue)
endif()

if(NOT DEFINED PROJ_NAME)
    set(PROJ_NAME usb-console-ex)
endif()

set(EXTRA_ZEPHYR_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/modules")
# set(DTC_OVERLAY_FILE "../zephyr/app.overlay;../zephyr/stm32f103c6_blue.overlay")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(${PROJ_NAME})

FILE(GLOB app_sources ../src/*.c*)
set(app_inc_dir ../inc)

target_include_directories(app PRIVATE ${app_inc_dir})
target_sources(app PRIVATE ${app_sources})
