OUTPUT_ARCH ("riscv")
ENTRY (_entry)

MEMORY
{
    BOOTROM (rx)      : ORIGIN = 0x100000, LENGTH = 1M
    RAM    (xrw)      : ORIGIN = 0x80000000, LENGTH = 32K
}

SECTIONS
{
    . = 0x10000;

    .text : {
        *(.text._entry)
        *(.text .text.*)
    } >BOOTROM

    .rodata : {
        *(.rodata .rodata.*)
    } >BOOTROM

    .srodata : {
        *(.srodata .srodata.*)
    } >BOOTROM

    .data : ALIGN (4) {
        *(.data)
        *(.data*)
    } >RAM AT> BOOTROM

    .sdata : ALIGN (4) {
        PROVIDE (__global_pointer$ = . + 0x800);
        *(.sdata .sdata.*)
        PROVIDE (__data_end = .);
    } >RAM AT> BOOTROM

    PROVIDE (__data_start = ADDR(.data));
    PROVIDE (__data_load_start = LOADADDR(.data));

    .bss (NOLOAD) : ALIGN (4) {
        PROVIDE (__bss_start = .);
        *(.sbss .sbss.*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        PROVIDE (__bss_end = .);
    } >RAM

    .__heap : ALIGN (16) {
        PROVIDE (__heap_start = .);
    } > RAM

    PROVIDE(__stack_top = ORIGIN(RAM) + LENGTH(RAM));
}
