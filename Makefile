CROSS_COMPILE:=~/x-tools/i386-unknown-elf/bin/i386-unknown-elf-
AS:=$(CROSS_COMPILE)as
CC:=$(CROSS_COMPILE)gcc
CXX:=$(CROSS_COMPILE)g++
CXXFLAGS:=-g -O2 -Wall -Wextra
LDFLAGS:=-T linker.ld -ffreestanding -O2 -nostartfiles

all: myos.bin

myos.bin: boot.o kernel.o gdt.o interrupts.o terminal.o allocator.o stdlib.o assembler.o vfs.o processes.o
	$(CXX) $(LDFLAGS) -o $@ $^

.PHONY: boot gdb clean

boot: myos.bin
	qemu-system-i386 -kernel myos.bin

gdb: myos.bin
	qemu-system-i386 -kernel myos.bin -S -gdb tcp::1234

clean:
	rm -rf *.o
	rm -rf myos.bin
