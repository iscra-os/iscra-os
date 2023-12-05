CROSS_COMPILE:=~/x-tools/i386-unknown-elf/bin/i386-unknown-elf-
AS:=$(CROSS_COMPILE)as
CC:=$(CROSS_COMPILE)gcc
CXX:=$(CROSS_COMPILE)g++
CXXFLAGS:=-g -O2 -Wall -Wextra
LDFLAGS:=-T linker.ld -ffreestanding -O2 -nostartfiles

all: myos.iso

myos.iso: myos.bin
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

font.o:
	objcopy -O elf32-i386 -B i386 -I binary zap-ext-light24.psf font.o

myos.bin: boot.o kernel.o gdt.o interrupts.o terminal.o allocator.o stdlib.o assembler.o vfs.o processes.o font.o
	$(CXX) $(LDFLAGS) -o $@ $^

.PHONY: boot gdb clean

boot: myos.iso
	qemu-system-i386 -cdrom myos.iso

gdb: myos.iso
	qemu-system-i386 -cdrom myos.iso -S -gdb tcp::1234

clean:
	rm -rf *.o
	rm -rf myos.bin
