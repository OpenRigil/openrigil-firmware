all: usbbootrom.bin

usbbootrom: entry.S usbbootrom.c bootrom.ld
	riscv32-unknown-elf-gcc -o $@ entry.S usbbootrom.c -std=gnu99 -O2 -fno-common -fno-builtin-printf -Wall -static -nostdlib -fno-builtin -mcmodel=medany -T./bootrom.ld -ffreestanding

usbbootrom.bin: usbbootrom
	riscv32-unknown-elf-objcopy -O binary usbbootrom usbbootrom.bin
