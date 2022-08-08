    .section .text._entry
    .global _entry
_entry:
    # Initialize global pointer
.option push
.option norelax
1:  auipc gp, %pcrel_hi(__global_pointer$)
    addi  gp, gp, %pcrel_lo(1b)
    .option pop

    la sp, __stack_top

    # Clear the bss segment
    la   a0, __bss_start
    la   a2, __bss_end
    sub  a2, a2, a0
    li   a1, 0
    call memset

    # Copy the data segment from ROM to RAM
    la   a0, __data_start
    la   a2, __data_end
    sub  a2, a2, a0
    la   a1, __data_load_start
    call memcpy

    j main
