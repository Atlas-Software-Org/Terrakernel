#ifndef MODULES_HPP
#define MODULES_HPP 1

#include <cstdint>
#include <cstddef>

typedef int vfd_t;
typedef void* unreal_ptr;

namespace unreal_fs {

enum class unreal_type {
    FILE,
    DIRECTORY,
};

struct unreal_node {
    const char* name;
    unreal_type type;
    unreal_node* parent;
    unreal_node** children;
    size_t children_count;
    char* buffer;
    size_t size;
    bool open;
};

struct unreal_module {
    char* name = nullptr;
    uint64_t start_vlba = 0;
    uint64_t end_vlba = 0;
    unreal_node* root_node = nullptr;
    unreal_module* next = nullptr;
};

struct file_handle {
    bool used;
    unreal_node* node;
};

void initialise();
unreal_node* resolve_path(const char* path);
unreal_node* create_file(const char* name, char* path);
unreal_node* create_directory(const char* full_path);
unreal_node* delete_node(const char* name, char* path);
vfd_t open(const char* path);
void close(vfd_t vfd);
int64_t read(vfd_t vfd, void* buffer, size_t size);
int64_t write(vfd_t vfd, void* buffer, size_t offset, size_t size);
int64_t stat_size(vfd_t vfd);
void mount_virtual_disk(void* module_base, size_t module_size, const char* target_mount_path);

}

namespace unreal_fs::ustar_parser {
    struct ustar_file {
        char* path;
        int type;
        size_t size;
        uint8_t* data;
    };

    void parse_ustar_archive(
        uint8_t* archive, size_t archive_size,
        struct ustar_file** out_files, uint64_t* out_count
    );
}

namespace unreal_fs::modules {

struct unreal_module {
    uint64_t address;
    size_t length;
};

unreal_module* get_first_module();

}

#endif /* MODULES_HPP */
