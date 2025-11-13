#include "unrealfs.hpp"
#include <drivers/serial/printf.h>
#include <mem/mem.hpp>

namespace unreal_fs::ustar_parser {

size_t ustar_strlen(const char* str) {
    size_t len = 0;
    while (str && *str++) len++;
    return len;
}

static char* ustar_strdup(const char* s) {
    size_t len = ustar_strlen(s);
    char* dup = static_cast<char*>(mem::heap::malloc(len + 1));
    if (!dup) return nullptr;
    for (size_t i = 0; i <= len; ++i) dup[i] = s[i];
    return dup;
}

struct ustar_header {
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
    char pad[12];
};

static size_t parse_octal(const char* str, size_t size) {
    size_t result = 0;
    for (size_t i = 0; i < size && str[i] >= '0' && str[i] <= '7'; i++) {
        result = (result << 3) + (str[i] - '0');
    }
    return result;
}

void parse_ustar_archive(uint8_t* archive, size_t archive_size,
                         struct ustar_file** out_files, uint64_t* out_count) {
    if (!archive || archive_size == 0 || !out_files || !out_count) return;

    uint64_t capacity = 16;
    uint64_t count = 0;
    ustar_file* files = static_cast<ustar_file*>(mem::heap::malloc(sizeof(ustar_file) * capacity));

    size_t offset = 0;
    while (offset + 512 <= archive_size) {
        ustar_header* header = reinterpret_cast<ustar_header*>(archive + offset);

        if (header->name[0] == '\0') break;

        char full_path[256] = {0};
        if (header->prefix[0]) {
            snprintf(full_path, sizeof(full_path), "%s/%s", header->prefix, header->name);
        } else {
            snprintf(full_path, sizeof(full_path), "%s", header->name);
        }

        int type = 0; // file
        if (header->typeflag == '5') type = 1;

        size_t size = parse_octal(header->size, sizeof(header->size));

        if (count >= capacity) {
            capacity *= 2;
            files = static_cast<ustar_file*>(mem::heap::realloc(files, sizeof(ustar_file) * capacity));
        }

        files[count].path = ustar_strdup(full_path);
        files[count].type = type;
        files[count].size = size;
        files[count].data = size > 0 ? archive + offset + 512 : nullptr;
        count++;

        size_t blocks = (size + 511) / 512; // ceil division
        offset += 512 + blocks * 512;
    }

    *out_files = files;
    *out_count = count;
}

}