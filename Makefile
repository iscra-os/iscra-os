CROSS_COMPILE:=~/x-tools/i386-unknown-elf/bin/i386-unknown-elf-
AS:=$(CROSS_COMPILE)as
CC:=$(CROSS_COMPILE)gcc

all:
	$(AS) boot.s -o boot.o
	$(CC) -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

.PHONY: boot
boot:
	qemu-system-i386 -kernel myos.bin
