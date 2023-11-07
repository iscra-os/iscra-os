CROSS_COMPILE:=~/x-tools/i386-unknown-elf/bin/i386-unknown-elf-
AS:=$(CROSS_COMPILE)as
CC:=$(CROSS_COMPILE)gcc
CXX:=$(CROSS_COMPILE)g++

all:
	$(AS) boot.s -o boot.o
	$(CXX) -c kernel.cpp -o kernel.o -O2 -Wall -Wextra
	$(CXX) -c gdt.cpp -o gdt.o -O2 -Wall -Wextra
	$(CXX) -c interrupts.cpp -o interrupts.o -O2 -Wall -Wextra
	$(CXX) -c terminal.cpp -o terminal.o -O2 -Wall -Wextra
	$(CXX) -c allocator.cpp -o allocator.o -O2 -Wall -Wextra
	$(CXX) -c stdlib.cpp -o stdlib.o -O2 -Wall -Wextra
	$(CXX) -c assembler.cpp -o assembler.o -O2 -Wall -Wextra
	$(CXX) -c vfs.cpp -o vfs.o -O2 -Wall -Wextra
	$(CXX) -T linker.ld -o myos.bin -ffreestanding -O2 boot.o kernel.o gdt.o interrupts.o terminal.o allocator.o stdlib.o assembler.o vfs.o -nostartfiles

.PHONY: boot
boot:
	qemu-system-i386 -kernel myos.bin
