CROSS_COMPILE:=~/x-tools/i386-unknown-elf/bin/i386-unknown-elf-
AS:=$(CROSS_COMPILE)as
CC:=$(CROSS_COMPILE)gcc
CXX:=$(CROSS_COMPILE)g++

all:
	$(AS) boot.s -o boot.o
	$(CC) -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	$(CXX) -c gdt.cpp -o gdt.o -ffreestanding -O2 -Wall -Wextra
	$(CXX) -c interrupts.cpp -o interrupts.o -ffreestanding -O2 -Wall -Wextra
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 boot.o kernel.o gdt.o interrupts.o -nostartfiles

.PHONY: boot
boot:
	qemu-system-i386 -kernel myos.bin
