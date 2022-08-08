set(CROSS_COMPILE riscv32-unknown-elf-)

set(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/bootrom.ld")

set(CMAKE_C_FLAGS "-march=rv32imac -mabi=ilp32 -mcmodel=medany -std=gnu99 -O2 -Wall -static -ffreestanding -nostartfiles -Wl,--nmagic -T${LINKER_SCRIPT}")

set(CMAKE_C_COMPILER "${CROSS_COMPILE}gcc")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")
set(CMAKE_ASM_COMPILER "${CROSS_COMPILE}gcc")
