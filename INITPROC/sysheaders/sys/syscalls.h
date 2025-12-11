#ifndef SYSCALLS_H
#define SYSCALLS_H 1

#include <stddef.h>

#define num_sys_read      0
#define num_sys_write     1
#define num_sys_open      2
#define num_sys_close     3
#define num_sys_stat      5
#define num_sys_lseek     8
#define num_sys_pread     17
#define num_sys_pwrite    18
#define num_sys_sync      74
#define num_sys_datasync  75
#define num_sys_truncate  77
#define num_sys_getdents  78
#define num_sys_chdir     80
#define num_sys_rename    82
#define num_sys_mkdir     83
#define num_sys_rmdir     84
#define num_sys_link      86
#define num_sys_unlink    87
#define num_sys_symlink   88
#define num_sys_readlink  89
#define num_sys_chmod     90
#define num_sys_chown     92

typedef unsigned int   mode_t;
typedef unsigned int   uid_t;
typedef unsigned int   gid_t;
typedef long           off_t;
typedef signed long long ssize_t;

static inline long syscall0(long n) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall1(long n, long a1) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall2(long n, long a1, long a2) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall3(long n, long a1, long a2, long a3) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall4(long n, long a1, long a2, long a3, long a4) {
    long ret;
    register long r10 asm("r10") = a4;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall5(long n, long a1, long a2, long a3, long a4, long a5) {
    long ret;
    register long r10 asm("r10") = a4;
    register long r8  asm("r8")  = a5;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6) {
    long ret;
    register long r10 asm("r10") = a4;
    register long r8  asm("r8")  = a5;
    register long r9  asm("r9")  = a6;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline int sys_open(const char* path, int flags, mode_t mode) {
    return syscall3(num_sys_open, (long)path, flags, mode);
}

static inline int sys_close(int fd) {
    return syscall1(num_sys_close, fd);
}

static inline ssize_t sys_read(int fd, void* buf, size_t count) {
    return syscall3(num_sys_read, fd, (long)buf, count);
}

static inline ssize_t sys_write(int fd, const void* buf, size_t count) {
    return syscall3(num_sys_write, fd, (long)buf, count);
}

static inline ssize_t sys_pread(int fd, void* buf, size_t count, off_t offset) {
    return syscall4(num_sys_pread, fd, (long)buf, count, offset);
}

static inline ssize_t sys_pwrite(int fd, const void* buf, size_t count, off_t offset) {
    return syscall4(num_sys_pwrite, fd, (long)buf, count, offset);
}

static inline off_t sys_lseek(int fd, off_t offset, int whence) {
    return syscall3(num_sys_lseek, fd, offset, whence);
}

static inline int sys_stat(int fd, void* buf) {
    return syscall2(num_sys_stat, fd, (long)buf);
}

static inline int sys_chmod(int fd, mode_t mode) {
    return syscall2(num_sys_chmod, fd, mode);
}

static inline int sys_chown(int fd, uid_t owner, gid_t group) {
    return syscall3(num_sys_chown, fd, owner, group);
}

static inline int sys_truncate(int fd, off_t length) {
    return syscall2(num_sys_truncate, fd, length);
}

static inline int sys_sync(int f) {
    return syscall0(num_sys_sync);
}

static inline int sys_datasync() {
    return syscall0(num_sys_datasync);
}

static inline int sys_mkdir(const char* path, mode_t mode) {
    return syscall2(num_sys_mkdir, (long)path, mode);
}

static inline int sys_chdir(const char* path) {
    return syscall1(num_sys_chdir, (long)path);
}

static inline int sys_link(const char* oldpath, const char* newpath) {
    return syscall2(num_sys_link, (long)oldpath, (long)newpath);
}

static inline int sys_unlink(const char* path) {
    return syscall1(num_sys_unlink, (long)path);
}

static inline int sys_rename(const char* oldpath, const char* newpath) {
    return syscall2(num_sys_rename, (long)oldpath, (long)newpath);
}

static inline int sys_symlink(const char* target, const char* linkpath) {
    return syscall2(num_sys_symlink, (long)target, (long)linkpath);
}

static inline int sys_readlink(const char* path, char* buf, size_t bufsize) {
    return syscall3(num_sys_readlink, (long)path, (long)buf, bufsize);
}

static inline int sys_rmdir(const char* path) {
    return syscall1(num_sys_rmdir, (long)path);
}

static inline int sys_getdents(int fd, void* buf, size_t bufsize) {
    return syscall3(num_sys_getdents, fd, (long)buf, bufsize);
}

#endif
