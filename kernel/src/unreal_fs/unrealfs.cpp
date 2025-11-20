#include "unrealfs.hpp"
#include <drivers/serial/print.hpp>
#include <mem/mem.hpp>

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

static size_t u_strlen(const char* s) {
    size_t n = 0;
    if (!s) return 0;
    while (s[n]) n++;
    return n;
}

static char* u_strcpy(char* dst, const char* src) {
    char* r = dst;
    while ((*dst++ = *src++)) {}
    return r;
}

static char* u_strncpy(char* dst, const char* src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = 0;
    return dst;
}

static int u_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

static char* u_strtok_r(char* str, const char* delim, char** saveptr) {
    char* token;
    if (str) *saveptr = str;
    if (!*saveptr) return nullptr;

    char* s = *saveptr;

    while (*s) {
        bool is_delim = false;
        for (size_t i = 0; delim[i]; i++) {
            if (*s == delim[i]) { is_delim = true; break; }
        }
        if (!is_delim) break;
        s++;
    }

    if (*s == '\0') { *saveptr = nullptr; return nullptr; }

    token = s;

    while (*s) {
        for (size_t i = 0; delim[i]; i++) {
            if (*s == delim[i]) {
                *s = '\0';
                *saveptr = s + 1;
                return token;
            }
        }
        s++;
    }

    *saveptr = nullptr;
    return token;
}

static char* u_strcat(const char* s1, const char* s2) {
    if (!s1) s1 = "";
    if (!s2) s2 = "";

    size_t len1 = u_strlen(s1);
    size_t len2 = u_strlen(s2);

    char* result = (char*)mem::heap::malloc(len1 + len2 + 1);
    if (!result) return NULL;

    mem::memcpy(result, s1, len1);
    mem::memcpy(result + len1, s2, len2);

    result[len1 + len2] = '\0';

    return result;
}

static int next_fd_id = 1;

namespace ufs {

unreal_fd* root_fd = nullptr;

static unreal_fd* alloc_fd(const char* name, bool is_dir, unreal_fd* parent) {
    unreal_fd* f = (unreal_fd*)mem::heap::calloc(1, sizeof(unreal_fd));
    f->fd_id = next_fd_id++;
    f->is_dir = is_dir;
    f->is_root = (parent == nullptr);
    f->parent = parent;

    size_t len = u_strlen(name) + 1;
    f->name = (char*)mem::heap::malloc(len);
    u_strcpy(f->name, name);

    if (is_dir) {
        f->children.fd_capacity = 8;
        f->children.child_fds = (unreal_fd**)mem::heap::malloc(
            8 * sizeof(unreal_fd*)
        );
    }

    return f;
}

static void add_child(unreal_fd* parent, unreal_fd* child) {
    if (parent->children.num_fds >= parent->children.fd_capacity) {
        size_t newcap = parent->children.fd_capacity * 2;
        parent->children.child_fds = (unreal_fd**)mem::heap::realloc(
            parent->children.child_fds,
            newcap * sizeof(unreal_fd*)
        );
        parent->children.fd_capacity = newcap;
    }

    parent->children.child_fds[parent->children.num_fds++] = child;
}

static unreal_fd* walk(unreal_fd* dir, const char* name) {
    if (!dir || !dir->is_dir) return nullptr;
    for (size_t i = 0; i < dir->children.num_fds; i++) {
        unreal_fd* c = dir->children.child_fds[i];
        if (!c) continue;
        if (u_strcmp(c->name, name) == 0)
            return c;
    }
    return nullptr;
}

static unreal_fd* resolve_path(const char* path, bool create_missing, bool final_is_dir) {
    if (!root_fd || !path || path[0] != '/') return nullptr;

    char buf[512];
    u_strncpy(buf, path, sizeof(buf));
    buf[sizeof(buf)-1] = 0;

    char* saveptr;
    char* token = u_strtok_r(buf, "/", &saveptr);

    unreal_fd* curr = root_fd;
    int depth = 0;

    while (token) {
        depth++;
        if (depth > 1024) { Log::errf("resolve_path: too deep"); return nullptr; }

        unreal_fd* next = walk(curr, token);

        if (!next) {
            if (!create_missing) return nullptr;

            char* lookahead = u_strtok_r(nullptr, "/", &saveptr);
            bool make_dir = (lookahead != nullptr) || final_is_dir;

            next = alloc_fd(token, make_dir, curr);
            add_child(curr, next);

            token = lookahead;
            curr = next;
            continue;
        }

        curr = next;
        token = u_strtok_r(nullptr, "/", &saveptr);
    }

    return curr;
}

void initialise() {
    root_fd = alloc_fd("/", true, nullptr);
}

int fd_error(int fd) {
    return 0;
}

static unreal_fd* find_fd(unreal_fd* node, int target_fd) {
    if (!node) return nullptr;
    if (node->fd_id == target_fd) return node;
    if (!node->is_dir) return nullptr;

    for (size_t i = 0; i < node->children.num_fds; i++) {
        unreal_fd* res = find_fd(node->children.child_fds[i], target_fd);
        if (res) return res;
    }
    return nullptr;
}

int open(const char* path, int mode) {
    if (mode == 0) {
        unreal_fd* f = resolve_path(path, false, false);
        if (!f || f->is_dir) return -1;
        return f->fd_id;
    }

    unreal_fd* f = resolve_path(path, true, false);
    if (!f) return -1;

    if (!f->data_base) {
        f->data_base = mem::heap::malloc(1);
        f->data_length = 0;
    }

    return f->fd_id;
}

int close(int fd) { return 0; }

size_t read(int fd, void* buf, size_t count, int mode) {
    (void)mode;
    unreal_fd* f = find_fd(root_fd, fd);
    if (!f || f->is_dir) return 0;

    if (count > f->data_length) count = f->data_length;
    mem::memcpy(buf, f->data_base, count);
    return count;
}

size_t write(int fd, const void* buf, size_t count, int mode) {
    unreal_fd* f = find_fd(root_fd, fd);
    if (!f || f->is_dir || !buf || count == 0) return 0;

    size_t pos = f->offset;
    if (mode == WRITE_MODE_APPEND) {
        pos = f->data_length;
    } else if (mode == WRITE_MODE_DEFAULT) {
        pos = f->data_length;
    } else if (mode == WRITE_MODE_OVERWRITE) {
        if (pos > f->data_length) pos = f->data_length;
    } else if (mode == WRITE_MODE_INSERT) {
        if (pos > f->data_length) pos = f->data_length;
    } else if (mode == WRITE_MODE_OFFSET) {
    }

    size_t new_length = (mode == WRITE_MODE_INSERT) ? f->data_length + count : (pos + count);
    if (new_length > f->capacity) {
        size_t new_cap = (new_length + 1023) & ~1023;
        void* new_base = mem::heap::realloc(f->data_base, new_cap);
        if (!new_base) return 0;
        f->data_base = new_base;
        f->capacity = new_cap;
    }

    char* base = (char*)f->data_base;

    if (mode == WRITE_MODE_INSERT) {
        mem::memmove(base + pos + count, base + pos, f->data_length - pos);
        mem::memcpy(base + pos, buf, count);
        f->data_length += count;
    } else {
        mem::memcpy(base + pos, buf, count);
        if (pos + count > f->data_length)
            f->data_length = pos + count;
    }

    if (mode != WRITE_MODE_OFFSET)
        f->offset = pos + count;

    return count;
}

int mkdir(const char* path) {
    unreal_fd* f = resolve_path(path, true, true);
    return f ? 0 : -1;
}

int rmkdir(const char* path) {
    unreal_fd* f = resolve_path(path, false, true);
    if (!f || !f->is_dir || f->children.num_fds != 0) return -1;

    unreal_fd* p = f->parent;
    if (!p) return -1;

    for (size_t i = 0; i < p->children.num_fds; i++) {
        if (p->children.child_fds[i] == f) {
            p->children.child_fds[i] =
                p->children.child_fds[p->children.num_fds - 1];
            p->children.num_fds--;
            break;
        }
    }

    mem::heap::free(f->name);
    mem::heap::free(f->data_base);
    mem::heap::free(f);
    return 0;
}

int rmdir(const char* path) { return rmkdir(path); }

static void rr_free(unreal_fd* f) {
    for (size_t i = 0; i < f->children.num_fds; i++)
        rr_free(f->children.child_fds[i]);

    mem::heap::free(f->name);
    mem::heap::free(f->data_base);
    mem::heap::free(f->children.child_fds);
    mem::heap::free(f);
}

int rrmdir(const char* path) {
    unreal_fd* f = resolve_path(path, false, true);
    if (!f || f == root_fd) return -1;

    unreal_fd* p = f->parent;
    if (!p) return -1;

    for (size_t i = 0; i < p->children.num_fds; i++) {
        if (p->children.child_fds[i] == f) {
            p->children.child_fds[i] =
                p->children.child_fds[p->children.num_fds - 1];
            p->children.num_fds--;
            break;
        }
    }

    rr_free(f);
    return 0;
}

int opendir(const char* path, void* outbuf) {
    unreal_fd* f = resolve_path(path, false, true);
    return (f && f->is_dir) ? f->fd_id : -1;
}


int closedir(int fd) { return 0; }

int stat(int fd, void* outbuf, size_t bufsize) {
    struct S { bool is_dir; size_t size; } sbuf;

    unreal_fd* f = find_fd(root_fd, fd);
    if (!f) return -1;

    sbuf.is_dir = f->is_dir;
    sbuf.size   = f->data_length;

    if (bufsize < sizeof(sbuf)) return -1;

    mem::memcpy(outbuf, &sbuf, sizeof(sbuf));
    return 0;
}

void list_dir(int dir_fd, int indent) {
    unreal_fd* dir = find_fd(root_fd, dir_fd);
    if (!dir || !dir->is_dir) {
        Log::errf("list_dir: invalid fd %d", dir_fd);
        return;
    }

    for (size_t i = 0; i < dir->children.num_fds; i++) {
        unreal_fd* child = dir->children.child_fds[i];

        for (int j = 0; j < indent; j++) Log::info("  ");

        printf("%s%s\n", child->name, child->is_dir ? "/" : "");

        if (child->is_dir)
            list_dir(child->fd_id, indent + 1);
    }
}

void mount_dir(char* filename, char* path, char* path_prefix, char* thisdir, char* parentdir, void* database, size_t filelen) {
    if (path[0] == '.') path++;
    if (path[0] == '/') path++;

    char* full_path = u_strcat(path_prefix, path);
    if (!full_path) return;

    int dfd = mkdir(full_path);
    if (!(dfd >= 0)) Log::errf("Failed to open dir");

    printf("MOUNT: Mounted directory %s to path %s...\n", filename, full_path);

    mem::heap::free(full_path);
}

void mount_file(char* filename, char* path, char* path_prefix, char* thisdir, char* parentdir, void* database, size_t filelen) {
    if (path[0] == '.') path++;
    if (path[0] == '/') path++;

    char* full_path = u_strcat(path_prefix, path);
    if (!full_path) return;

    int fd = open(full_path, OPEN_MODE_CREATE);
    if (fd >= 0) {
        write(fd, database, filelen, WRITE_MODE_OVERWRITE);
        close(fd);
    } else {
        Log::errf("Failed to open file");
    }

    printf("MOUNT: Mounted file %s to path %s...\n", filename, full_path);

    mem::heap::free(full_path);
}

struct ustar_inode {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
} __attribute__((packed));

static size_t parse_octal(const char* str, size_t n) {
    size_t val = 0;
    for (size_t i = 0; i < n && str[i] >= '0' && str[i] <= '7'; i++) {
        val = (val << 3) + (str[i] - '0');
    }
    return val;
}

void load_ustar_archive(void* base, size_t size) {
    uint8_t* ptr = (uint8_t*)base;
    uint8_t* end = ptr + size;

    while (ptr + sizeof(struct ustar_inode) <= end) {
        struct ustar_inode* header = (struct ustar_inode*)ptr;

        int is_empty = 1;
        for (int i = 0; i < 512; i++) {
            if (ptr[i] != 0) { is_empty = 0; break; }
        }
        if (is_empty) break;

        size_t filesize = parse_octal(header->size, sizeof(header->size));

        char fullpath[256] = {0};
        if (header->prefix[0]) {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", header->prefix, header->name);
        } else {
            snprintf(fullpath, sizeof(fullpath), "%s", header->name);
        }

        char* path_prefix = "/initrd/";

        if (header->name[u_strlen(header->name)] == '/' || header->name[u_strlen(header->name)-1] == '/') {
            mount_dir(header->name, fullpath, path_prefix, ".", "..", ptr + 512, filesize);
        } else {
            mount_file(header->name, fullpath, path_prefix, ".", "..", ptr + 512, filesize);
        }

        size_t total_size = 512 + ((filesize + 511) & ~511);
        ptr += total_size;
    }
}

}
