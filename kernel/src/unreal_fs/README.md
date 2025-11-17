# UnrealFS - In-Memory unreal filesystem

## Basic features
- Creating directories
- Recursive directory operations
- File write/open modes
- Parsing /unix/style/paths
- Quick file inflating/shrinking with realloc
- Stat-ing files

## `struct unreal_fd`
`struct unreal_fd` represents either a directory or a file

```c
struct unreal_fd {
    char *name;
    int fd_id;
    bool is_dir;
    bool is_root;
    unreal_fd* parent;

    struct {
        unreal_fd** child_fds;
        size_t num_fds;
        size_t fd_capacity;
    } children;

    uint64_t offset;
    uint64_t capacity;

    void* data_base;
    size_t data_length;
};
```

Also the nested `children` struct can be moved outside `unreal_fd` struct as a struct by itself

## Initialisation and operations
To initialise UFS you must call `ufs::initialise();`, then create the necesseray directories...

Such as `/dev`, `/proc`, and `/initrd`

`ufs::initialise();` basically creates the `/` directory (root)

### Operations

### Opening a file
Example:
```c
int fd_opened = ufs::open("/path/to/file.txt", OPEN_MODE_OPEN);
int fd_created = ufs::open("/path/to/another/file.c", OPEN_MODE_CREATE);
```

### Closing a file
```c
ufs::close(fd);
```

### Reading from a file
```c
char buf[128];
int read = ufs::read(fd, buf, sizeof(buf), 0);
// read = amount of bytes read
```

### Writing to a file
```c
const char* msg = "Hello!\n\r";
ufs::write(fd, msg, strlen(msg), WRITE_MODE_DEFAULT);
```

Available write modes:
```
WRITE_MODE_DEFAULT
WRITE_MODE_APPEND
WRITE_MODE_INSERT
WRITE_MODE_OVERWRITE
WRITE_MODE_OFFSET
```

### Creation of a directory
```c
ufs::mkdir("/etc");
ufs::mkdir("/etc/config");
ufs::mkdir("/recursive/test");
```

### Removing a directory recursively and non-recursively
```c
ufs::rmdir("/etc/config");
ufs::rrmdir("/recursive/test");
// NOTE!: rmdir = rrmdir
```

### Opening a directory
```c
int dir_fd = ufs::opendir("/etc");
```

### Listing directory's contents
```c
ufs::list_dir(dfd, 0);
```

### Stat-ing a file
```c
/*
struct {
	bool is_dir;
	size_t size;
} statbuf;
*/
statbuf *sb = (statbuf*)mem::heap::malloc(sizeof(statbuf));
ufs::stat(fd, sb, sizeof(sb));
```

## License
Same as Terrakernel license
