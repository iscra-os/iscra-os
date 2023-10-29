#include "terminal.h"
#include <sys/types.h>
#include <cstdlib>


extern "C" {
	int close(int fd) {
        (void)fd;
		terminal_writestring("close()\n");
		while (1);
	}

	off_t lseek(int fd, off_t offset, int whence) {
        (void)fd;
        (void)offset;
        (void)whence;
		terminal_writestring("lseek()\n");
		while (1);
	}
	ssize_t read(int fd, void *buf, size_t count) {
        (void)fd;
        (void)buf;
        (void)count;
		terminal_writestring("read()\n");
		while (1);
	}

	ssize_t write(int fd, const void *buf, size_t count) {
        (void)fd;
        (void)buf;
        (void)count;
		terminal_writestring("write()\n");
		while (1);
	}
}