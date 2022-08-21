cmake_minimum_required(VERSION 3.7)

project(openrigil-usb C ASM)

#set(CMAKE_C_FLAGS "-g")

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/riscv32.cmake")

add_executable(usbbootrom
    "entry.S"
    "main.c"
    "bn.c"
    "mmm.c"
    "mmm_test.c"
    "25519.c"
    "25519_test.c"
    "device.c"
    "uart.c"
    "flash.c"
    "usbd_conf.c"
    "interrupt.c"
    "dcd_zenithal.c"
    "littlefs_api.c"
)

# config of the accelerator
add_definitions(-DMMM_BLOCK=8)

add_definitions(-DMBEDTLS_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/mbedtls-config.h")
add_definitions(-DDEVICE_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/device-config.h")

add_definitions(-DDEBUG_OUTPUT)
add_definitions(-DLFS_NO_DEBUG)
add_definitions(-DLFS_NO_WARN)
add_definitions(-DLFS_NO_ERROR)

set(USE_MBEDCRYPTO ON CACHE BOOL "" FORCE)
set(ENABLE_DEBUG_OUTPUT ON CACHE BOOL "" FORCE)
set(USBD_PRODUCT_STRING "OpenRigil")
add_subdirectory(canokey-core)

set(gitrev_in git-rev.h.in)
set(gitrev git-rev.h)
add_custom_target(gitrev
        ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev_in} ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        COMMAND git describe --always --tags --long --abbrev=8 --dirty >> ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        )

target_link_libraries(usbbootrom PRIVATE canokey-core c m)
add_dependencies(usbbootrom gitrev)
