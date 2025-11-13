#include "unrealfs.hpp"
#include <limine.h>
#include <mem/mem.hpp>

__attribute__((section(".limine_requests")))
volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0,
};

namespace unreal_fs::modules {

unreal_module* get_first_module() {
    if (module_request.response == nullptr || module_request.response->module_count < 1) {
        return nullptr;
    }

    static struct unreal_module* first;
    if (!first) {
         first = static_cast<unreal_module*>(mem::heap::malloc(sizeof(unreal_module)));
    } else return first;
    if (!first) return nullptr;

    first->address = (uint64_t)module_request.response->modules[0]->address;
    first->length = module_request.response->modules[0]->size;

    return first;
}

}