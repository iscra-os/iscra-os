CROSS_COMPILE:=~/x-tools/i386-unknown-elf/bin/i386-unknown-elf-
AS:=$(CROSS_COMPILE)as
CC:=$(CROSS_COMPILE)gcc
CXX:=$(CROSS_COMPILE)g++

all:
	$(AS) boot.s -o boot.o
	$(CXX) -c -g kernel.cpp -o kernel.o -O2 -Wall -Wextra
	$(CXX) -c -g gdt.cpp -o gdt.o -O2 -Wall -Wextra
	$(CXX) -c -g interrupts.cpp -o interrupts.o -O2 -Wall -Wextra
	$(CXX) -c -g terminal.cpp -o terminal.o -O2 -Wall -Wextra
	$(CXX) -c -g allocator.cpp -o allocator.o -O2 -Wall -Wextra
	$(CXX) -c -g stdlib.cpp -o stdlib.o -O2 -Wall -Wextra
	$(CXX) -c -g assembler.cpp -o assembler.o -O2 -Wall -Wextra
	$(CXX) -c -g vfs.cpp -o vfs.o -O2 -Wall -Wextra
	$(CXX) -T linker.ld -o myos.bin -ffreestanding -O2 boot.o kernel.o gdt.o interrupts.o terminal.o allocator.o stdlib.o assembler.o vfs.o -nostartfiles

.PHONY: boot gdb
boot:
	qemu-system-i386 -kernel myos.bin

gdb: all
	qemu-system-i386 -kernel myos.bin -S -gdb tcp::1234
