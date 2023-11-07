#include <cstdio>
#include "terminal.h"
#include <sys/stat.h>
#include <string>
#include <unordered_map>

class vfs_entry {

    virtual vfs_entry* resolve(const std::string& path); 
    virtual vfs_entry* create_file(const std::string& path);
};

vfs_entry* vfs_entry::resolve(const std::string& path) {
    return nullptr;
}
vfs_entry* vfs_entry::create_file(const std::string& path) {
    return false;
}

// std::unordered_map<std::string, int> a;
// a["a"] = 0;
// a["b"] = 1;
// if (a[2] == 0)
class ramfs_directory : public vfs_entry {
    std::string _name;
    std::unordered_map<std::string, vfs_entry*> _content;
public:
    ramfs_directory(std::string name);

    vfs_entry* resolve(const std::string& path) override; 
    vfs_entry* create_file(const std::string& path) override;
};

ramfs_directory::ramfs_directory(std::string name) : _name(std::move(name)) {}


vfs_entry* ramfs_directory::resolve(const std::string& path) {
    auto iter = _content.find(path);
    if (iter == _content.end()) 
        return nullptr;

    return iter->second;
}

vfs_entry* ramfs_directory::create_file(const std::string& path) {
    auto iter = _content.find(path);
    if (iter != _content.end()) 
        return nullptr;

    return _content.emplace(path, new ramfs_file(path)).first->second;
}

ramfs_directory root("/");

extern "C" {
    
    int open(const char *pathname, int flags, ...) {
        int available_flags = ~(O_CREAT | O_TRUNC | O_WRONLY);
        if (flags & available_flags)
            panic("open was called with not implemented flags: %x\n", flags & available_flags);
        
        vfs_entry* entry = root.resolve(pathname);
        if (!entry) // TODO: O_CREAT 
            entry = root.create_file(pathname);
        

		printk("open(%s, %x)\n", pathname, flags);
        return 0;
	}

    int fstat(int fd, struct stat *statbuf) {
		printk("fstat()\n");

        statbuf->st_size = 0;

        return 0;
	}

    ssize_t write(int fd, const void *buf, size_t count) {
        (void)fd;
        (void)buf;
        (void)count;

		printk("write()\n");

        return count;
	}

    int close(int fd) {
        (void)fd;

		printk("close()\n");

        return 0;
	}

}


void init_vfs() {
    new (&root) ramfs_directory("/");

    FILE* file = fopen("test.txt", "w");
    if (file == NULL)
        panic("Failed to open test.txt\n");
    
    fputs("Hello\n", file);

    fclose(file);
}