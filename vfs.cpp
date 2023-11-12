#include <cstdio>
#include "terminal.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>


enum open_mode {
    mode_none,
    mode_read,
    mode_write,
    mode_read_write
};


class vfs_entry {

public:
    // Directory interfaces
    virtual vfs_entry* resolve(const std::string& path); 
    virtual vfs_entry* create_file(const std::string& path);

    // File interfaces
    virtual bool open(open_mode mode);
    virtual bool truncate();
    virtual size_t write(void* buffer, size_t size);
    virtual bool close();

    virtual bool is_file();
};

vfs_entry* vfs_entry::resolve(const std::string& path) {
    return nullptr;
}
vfs_entry* vfs_entry::create_file(const std::string& path) {
    return nullptr;
}
bool vfs_entry::open(open_mode mode) {
    return false;
}
bool vfs_entry::is_file() {
    return false;
}
bool vfs_entry::truncate() {
    return false;
}
size_t vfs_entry::write(void* buffer, size_t size) {
    return 0;
}
bool vfs_entry::close() {
    return false;
}

class ramfs_file : public vfs_entry {
    std::string _name;
    std::vector<uint8_t> _data;
    bool _opened;
public:
    ramfs_file(std::string name) : _name(std::move(name)), _opened(false) {}

    bool open(open_mode mode) override;
    bool is_file() override;
    bool truncate() override;
    size_t write(void* buffer, size_t size) override;
    bool close() override;
};

bool ramfs_file::open(open_mode mode) {
    _opened = true;
    return true;
}
bool ramfs_file::is_file() {
    return true;
}
bool ramfs_file::truncate() {
    if (!_opened)
        return false;
    _data.clear();
    return true;
}
size_t ramfs_file::write(void* buffer, size_t size) {
    if (!_opened)
        return false;
    _data.reserve(size);
    memcpy(_data.data(), buffer, size);
    return size;
}
bool ramfs_file::close() {
    _opened = false;
    return true;
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
    
    auto entry = new ramfs_file(path);

    _content.emplace(path, entry);

    return entry;
}

ramfs_directory root("/");

open_mode flags_to_mode(int flags) {
    switch (flags & 0b11) {
        case O_RDONLY:
            return mode_read;
        case O_WRONLY:
            return mode_write;
        case O_RDWR:
            return mode_read_write;
        default:
            return mode_none;
    }
}

int allocate_file_descriptor() {
    static int current = 0;
    return current++;
}

std::unordered_map<int, vfs_entry*> opened_files;

extern "C" {
    
    int open(const char *pathname, int flags, ...) {
        int available_flags = ~(O_CREAT | O_TRUNC | O_WRONLY);
        if (flags & available_flags)
            panic("open was called with not implemented flags: %x\n", flags & available_flags);
        
        vfs_entry* entry = root.resolve(pathname);

        if (!entry) {
            if (flags & O_CREAT) // TODO: O_CREAT 
                entry = root.create_file(pathname);
            else {
                errno = ENOENT;
                return -1;
            }
        }
        else if (!entry->is_file()) {
            errno = EISDIR;
            return -1;
        }

        bool ret = entry->open(flags_to_mode(flags));
        if (!ret)
        {
            errno = EACCES;
            return -1;
        }

        if (flags & O_TRUNC)
            entry->truncate(); // TODO: check
        
        int fd = allocate_file_descriptor();
        opened_files.emplace(fd, entry);

		printk("open(%s, %x)\n", pathname, flags);
        return fd;
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
    new (&opened_files) std::unordered_map<int, vfs_entry*>();

    FILE* file = fopen("test.txt", "w");
    if (file == NULL)
        panic("Failed to open test.txt\n");
    
    fputs("Hello\n", file);

    fclose(file);
}