cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(Pico-ResTouch-X_X_code)
pico_sdk_init()

add_subdirectory(lib/config)
add_subdirectory(lib/lcd)
add_subdirectory(lib/font)
add_subdirectory(system1)
add_subdirectory(lib/fatfs)
add_subdirectory(lib/sdcard)

include_directories(system1)
include_directories(./lib/config)
include_directories(./lib/lcd)
include_directories(./lib/font)
include_directories(./lib/fatfs)
include_directories(./lib/sdcard)

add_executable(main
main.c
)
target_link_libraries(main hardware_adc system1 lcd font config pico_stdlib hardware_spi pico_multicore)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)

