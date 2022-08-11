cmake_minimum_required(VERSION 3.7)

project(openrigil-usb C ASM)

#set(CMAKE_C_FLAGS "-g")

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/riscv32.cmake")

add_executable(usbbootrom
    "entry.S"
    "main.c"
    "uart.c"
    "flash.c"
    "usbd_conf.c"
    "interrupt.c"
    "dcd_zenithal.c"
    "littlefs_api.c"
)

#add_definitions(-DMBEDTLS_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-config.h")
add_definitions(-DDEVICE_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/device-config.h")

#add_definitions(-DDEBUG_OUTPUT)
add_definitions(-DLFS_NO_DEBUG)
add_definitions(-DLFS_NO_WARN)
add_definitions(-DLFS_NO_ERROR)

set(USE_MBEDCRYPTO OFF CACHE BOOL "" FORCE)
set(ENABLE_DEBUG_OUTPUT OFF CACHE BOOL "" FORCE)
add_subdirectory(canokey-core)

#target_link_libraries(usbbootrom PRIVATE c m gloss)
target_link_libraries(usbbootrom PRIVATE canokey-core c m gloss)
#target_link_options(usbbootrom PRIVATE "-Wl,--unresolved-symbols=ignore-in-object-files")
