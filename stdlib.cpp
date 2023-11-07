#include "terminal.h"
#include <sys/types.h>
#include <cstdlib>


extern "C" {

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

	int isatty(int fd) {
		panic("isatty()\n");
	}

	int getentropy(void *buffer, size_t length) {
		panic("getentropy()\n");
	}

	int kill(pid_t pid, int sig) {
		panic("kill()\n");
	}

	void abort(void) {
		panic("abort()\n");
	}

	pid_t getpid(void) {
		panic("getpid()\n");
	}

	void* __dso_handle;
}