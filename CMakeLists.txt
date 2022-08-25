cmake_minimum_required(VERSION 3.7)

project(openrigil-firmware C ASM)

# enable Test when necessary
#file(GLOB_RECURSE SRCS "test/*.c")

file(GLOB_RECURSE SRCS "src/*.c" "src/*.S" "crypto/*.c")

include_directories(include)

add_executable(openrigil ${SRCS})

# config of the accelerator
add_definitions(-DMMM_BLOCK=64)

add_definitions(-DMBEDTLS_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/include/mbedtls-config.h")
add_definitions(-DDEVICE_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/include/device-config.h")

add_definitions(-DDEBUG_OUTPUT)
add_definitions(-DLFS_NO_DEBUG)
add_definitions(-DLFS_NO_WARN)
add_definitions(-DLFS_NO_ERROR)

set(USE_MBEDCRYPTO ON CACHE BOOL "" FORCE)
set(ENABLE_DEBUG_OUTPUT ON CACHE BOOL "" FORCE)
set(USBD_PRODUCT_STRING "OpenRigil")
add_subdirectory(canokey-core)

set(gitrev_in include/git-rev.h.in)
set(gitrev include/git-rev.h)
add_custom_target(gitrev
        ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev_in} ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        COMMAND git describe --always --tags --long --abbrev=8 --dirty >> ${CMAKE_CURRENT_SOURCE_DIR}/${gitrev}
        )

target_link_libraries(openrigil PRIVATE canokey-core c m)
add_dependencies(openrigil gitrev)

add_custom_target(openrigil-hex ALL
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_BINARY_DIR}/openrigil ${CMAKE_BINARY_DIR}/openrigil.bin
        COMMAND od -t x4 -An -w4 -v ${CMAKE_BINARY_DIR}/openrigil.bin > ${CMAKE_BINARY_DIR}/openrigil.hex
        )
add_dependencies(openrigil-hex openrigil)
