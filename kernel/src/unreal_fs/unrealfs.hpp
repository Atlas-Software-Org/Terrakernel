#ifndef UNREALFS_HPP
#define UNREALFS_HPP 1

#include <cstdint>
#include <cstddef>

struct statbuf {
    bool is_dir;
    size_t size;
};

#define DefineMode(ModeType, ModeName, ModeValue) constexpr auto ModeType##_MODE_##ModeName = ModeValue;

DefineMode(WRITE, DEFAULT, 0)
DefineMode(WRITE, APPEND, 1)
DefineMode(WRITE, INSERT, 2)
DefineMode(WRITE, OVERWRITE, 3)
DefineMode(WRITE, OFFSET, 4)

DefineMode(READ, NONE, 0) // no read modes for now

DefineMode(OPEN, OPEN, 0)
DefineMode(OPEN, CREATE, 1)

namespace ufs {

void initialise();

int fd_error(int fd);

int open(const char *path, int mode);
int close(int fd);

size_t read(int fd, void *buf, size_t count, int mode);
size_t write(int fd, const void *buf, size_t count, int mode);

int mkdir(const char *path);
int rmkdir(const char *path); // recursive

int rmdir(const char *path);
int rrmdir(const char *path); // recursive

int opendir(const char *path, void *outbuf);
int closedir(int dirfd);

int stat(int fd, void* outbuf, size_t bufsize);

void list_dir(int dir_fd, int indent = 0);

void load_ustar_archive(void* base, size_t size);

}

#endif