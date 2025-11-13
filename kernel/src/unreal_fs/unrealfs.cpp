#include "unrealfs.hpp"
#include <mem/mem.hpp>
#include <drivers/serial/print.hpp>

namespace unreal_fs {

unreal_node** nodes = nullptr;
uint64_t node_count = 0;
const char* ROOT_DIR_NAME = "/";
static file_handle handles[512];

size_t u_strlen(const char* str) {
    size_t len = 0;
    while (str && *str++) len++;
    return len;
}

int u_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

char* u_strcpy(char* dest, const char* src) {
    char* p = dest;
    while ((*p++ = *src++));
    return dest;
}

const char* u_strchr(const char* str, char c) {
    while (str && *str) {
        if (*str == c) return str;
        str++;
    }
    return nullptr;
}

static char* u_strdup(const char* s) {
    size_t len = u_strlen(s);
    char* dup = static_cast<char*>(mem::heap::malloc(len + 1));
    if (!dup) return nullptr;
    for (size_t i = 0; i <= len; ++i) dup[i] = s[i];
    return dup;
}

char* u_strtok(char* str, const char* delim) {
    char* *saveptr = nullptr;
    if (!str && !*saveptr) return nullptr;

    char* s = str ? str : *saveptr;
    if (!s) return nullptr;

    while (*s && u_strchr(delim, *s)) s++;

    if (!*s) {
        *saveptr = nullptr;
        return nullptr;
    }

    char* token_start = s;

    while (*s && !u_strchr(delim, *s)) s++;

    if (*s) {
        *s = '\0';
        *saveptr = s + 1;
    } else {
        *saveptr = nullptr;
    }

    return token_start;
}

char* u_strncpy(char* dest, const char* src, size_t n) {
    if (!dest || !src || n == 0) return dest;

    size_t i = 0;
    for (; i < n - 1 && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }

    dest[i] = '\0';

    return dest;
}

char* u_strcat(char* dest, const char* src) {
    if (!dest || !src) return dest;

    char* d = dest;
    while (*d) d++;

    while (*src) {
        *d++ = *src++;
    }

    *d = '\0';
    return dest;
}

static unreal_node* find_child(unreal_node* parent, const char* name) {
    if (!parent || !name || parent->type != unreal_type::DIRECTORY) return nullptr;
    for (size_t i = 0; i < parent->children_count; ++i) {
        unreal_node* c = parent->children[i];
        if (c && u_strcmp(c->name, name) == 0) return c;
    }
    return nullptr;
}

unreal_node* resolve_path(const char* path) {
    if (!path || !*path || path[0] != '/') return nullptr;
    if (node_count == 0 || !nodes || !nodes[0]) return nullptr;
    unreal_node* current = nodes[0];
    const char* p = path + 1;
    const char* segment_start = p;
    char segment[256];
    while (*p) {
        if (*p == '/') {
            size_t len = static_cast<size_t>(p - segment_start);
            if (len == 0) { p++; segment_start = p; continue; }
            if (len >= sizeof(segment)) return nullptr;
            mem::memcpy(segment, segment_start, len);
            segment[len] = '\0';
            current = find_child(current, segment);
            if (!current) return nullptr;
            segment_start = p + 1;
        }
        p++;
    }
    if (*segment_start) {
        size_t len = static_cast<size_t>(p - segment_start);
        if (len >= sizeof(segment)) return nullptr;
        mem::memcpy(segment, segment_start, len);
        segment[len] = '\0';
        current = find_child(current, segment);
    }
    return current;
}

unreal_node* create_file(const char* name, char* path) {
    Log::infof("Creating file: %s in path: %s\n\r", name, path);
    unreal_node* parent = resolve_path(path);
    if (!parent || parent->type != unreal_type::DIRECTORY) return nullptr;
    unreal_node* node = static_cast<unreal_node*>(mem::heap::malloc(sizeof(unreal_node)));
    if (!node) return nullptr;
    node->name = u_strdup(name);
    node->type = unreal_type::FILE;
    node->parent = parent;
    node->children = nullptr;
    node->children_count = 0;
    node->buffer = nullptr;
    node->size = 0;
    node->open = false;
    parent->children = static_cast<unreal_node**>(mem::heap::realloc(parent->children, sizeof(unreal_node*) * (parent->children_count + 1)));
    parent->children[parent->children_count++] = node;
    nodes = static_cast<unreal_node**>(mem::heap::realloc(nodes, sizeof(unreal_node*) * (node_count + 1)));
    nodes[node_count++] = node;
    return node;
}

unreal_node* create_directory(const char* full_path) {
    if (!full_path || u_strlen(full_path) == 0) return nullptr;
    if (!nodes[0]) return nullptr;

    unreal_node* parent = nodes[0];
    char path_copy[512];
    u_strncpy(path_copy, full_path, sizeof(path_copy));
    path_copy[sizeof(path_copy)-1] = 0;

    char* token = u_strtok(path_copy, "/");
    while (token) {
        unreal_node* child = nullptr;

        for (size_t i = 0; i < parent->children_count; ++i) {
            if (u_strcmp(parent->children[i]->name, token) == 0 &&
                parent->children[i]->type == unreal_type::DIRECTORY) {
                child = parent->children[i];
                break;
            }
        }

        if (!child) {
            child = static_cast<unreal_node*>(mem::heap::malloc(sizeof(unreal_node)));
            if (!child) return nullptr;

            child->name = u_strdup(token);
            child->type = unreal_type::DIRECTORY;
            child->parent = parent;
            child->children = nullptr;
            child->children_count = 0;
            child->buffer = nullptr;
            child->size = 0;
            child->open = false;

            parent->children = static_cast<unreal_node**>(
                mem::heap::realloc(parent->children, sizeof(unreal_node*) * (parent->children_count + 1))
            );
            parent->children[parent->children_count++] = child;

            nodes = static_cast<unreal_node**>(
                mem::heap::realloc(nodes, sizeof(unreal_node*) * (node_count + 1))
            );
            nodes[node_count++] = child;
        }

        parent = child;
        token = u_strtok(nullptr, "/");
    }

    return parent;
}

unreal_node* delete_node(const char* name, char* path) {
    (void)path;
    for (uint64_t i = 0; i < node_count; ++i) {
        if (nodes[i] && u_strcmp(nodes[i]->name, name) == 0) {
            unreal_node* removed = nodes[i];
            mem::heap::free((void*)removed->name);
            mem::heap::free(removed->buffer);
            mem::heap::free(removed);
            nodes[i] = nullptr;
            return removed;
        }
    }
    return nullptr;
}

vfd_t open(const char* path) {
    unreal_node* n = resolve_path(path);
    if (!n || n->type != unreal_type::FILE) return -1;
    for (int i = 0; i < 512; ++i) {
        if (!handles[i].used) {
            handles[i].used = true;
            handles[i].node = n;
            n->open = true;
            return i;
        }
    }
    return -1;
}

void close(vfd_t vfd) {
    if (vfd < 0 || vfd >= 512) return;
    if (handles[vfd].used) {
        handles[vfd].node->open = false;
        handles[vfd].used = false;
        handles[vfd].node = nullptr;
    }
}

int64_t read(vfd_t vfd, void* buffer, size_t size) {
    if (vfd < 0 || vfd >= 512) return 0;
    file_handle* h = &handles[vfd];
    if (!h->used || !h->node || !h->node->buffer) return 0;
    size_t to_read = (size < h->node->size) ? size : h->node->size;
    mem::memcpy(buffer, h->node->buffer, to_read);
    return static_cast<int64_t>(to_read);
}

int64_t write(vfd_t vfd, void* buffer, size_t offset, size_t size) {
    if (vfd < 0 || vfd >= 512) return 0;

    file_handle* h = &handles[vfd];
    if (!h->used || !h->node || h->node->type != unreal_type::FILE) return 0;

    size_t new_size = offset + size;
    if (!h->node->buffer) {
        h->node->buffer = reinterpret_cast<char*>(mem::heap::malloc(new_size));
    } else {
        h->node->buffer = reinterpret_cast<char*>(mem::heap::realloc(h->node->buffer, new_size));
    }

    if (!h->node->buffer) return 0;

    mem::memcpy(h->node->buffer + offset, buffer, size);

    if (new_size > h->node->size) {
        h->node->size = new_size;
    }

    return static_cast<int64_t>(size);
}

int64_t stat_size(vfd_t vfd) {
    if (vfd < 0 || vfd >= 512) return -1;
    file_handle* h = &handles[vfd];
    if (!h->used || !h->node) return -1;
    if (h->node->type != unreal_type::FILE) return -1;
    return h->node->size;
}

void initialise() {
    node_count = 1;
    for (int i = 0; i < 512; ++i) {
        handles[i].used = false;
        handles[i].node = nullptr;
    }
    nodes = static_cast<unreal_node**>(mem::heap::malloc(sizeof(unreal_node*)));
    unreal_node* root = static_cast<unreal_node*>(mem::heap::malloc(sizeof(unreal_node)));
    root->name = const_cast<char*>(ROOT_DIR_NAME);
    root->type = unreal_type::DIRECTORY;
    root->parent = nullptr;
    root->children = nullptr;
    root->children_count = 0;
    root->buffer = nullptr;
    root->size = 0;
    root->open = false;
    nodes[0] = root;
}

void mount_virtual_disk(void* module_base, size_t module_size, const char* target_mount_path) {
    using namespace unreal_fs;

    Log::infof("Mounting virtual disk at %s\n\rModule base: %p, size: %u bytes",
               target_mount_path, module_base, static_cast<unsigned int>(module_size));

    ustar_parser::ustar_file* ustar_files = nullptr;
    uint64_t archived_file_count = 0;

    ustar_parser::parse_ustar_archive(
        reinterpret_cast<uint8_t*>(module_base),
        module_size,
        &ustar_files,
        &archived_file_count
    );

    for (uint64_t i = 0; i < archived_file_count; ++i) {
        auto* entry = &ustar_files[i];
        if (!entry || !entry->path) continue;

        const char* last_slash = nullptr;
        for (const char* p = entry->path; *p; ++p) {
            if (*p == '/') last_slash = p;
        }

        unreal_node* parent = nullptr;

        if (last_slash) {
            size_t dir_len = static_cast<size_t>(last_slash - entry->path);
            char dir_path[512];
            u_strncpy(dir_path, entry->path, dir_len);
            dir_path[dir_len] = '\0';

            char full_path[1024];
            u_strcpy(full_path, target_mount_path);
            u_strcat(full_path, "/");
            u_strcat(full_path, dir_path);

            parent = create_directory(full_path);
        } else {
            parent = resolve_path(target_mount_path);
            if (!parent) parent = resolve_path("/");
        }

        if (entry->type == 0) {
            const char* file_name = last_slash ? last_slash + 1 : entry->path;
            unreal_node* file_node = create_file(file_name, (char*)parent->name);
            if (file_node && entry->data && entry->size > 0) {
                vfd_t fd = open(file_node->name);
                if (fd >= 0) {
                    write(fd, entry->data, 0, entry->size);
                    close(fd);
                }
            }
        }
    }

    for (uint64_t i = 0; i < archived_file_count; ++i) {
        mem::heap::free(ustar_files[i].path);
    }
    mem::heap::free(ustar_files);
}

}
