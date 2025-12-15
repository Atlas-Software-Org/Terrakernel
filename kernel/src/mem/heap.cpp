#include <mem/mem.hpp>
#include <mem/heap.hpp>
#include <cstdio>

void* heap_base = nullptr;
size_t heap_size = 0;

struct heap_block {
    void* base;
    size_t length;
    bool is_free;
    heap_block* prev;
    heap_block* next;
};

namespace mem::heap {

void initialise() {
    const size_t initial_size = 0x100000000;
    const size_t min_heap_size = 0x100000;
    size_t divisor = 1;
    heap_base = nullptr;

    while (heap_base == nullptr && initial_size / divisor >= min_heap_size) {
        heap_size = initial_size / divisor;
        size_t num_pages = heap_size / 0x1000;

        heap_base = mem::pmm::reserve_heap(num_pages);

        if (heap_base != nullptr) {
        } else {
        }

        divisor *= 2;
    }

    if (heap_base == nullptr) {
    } else {
    }

    heap_base = (void*)mem::vmm::pa_to_va((uint64_t)heap_base);

    heap_block* first_block = (heap_block*)heap_base;
    first_block->base = (void*)((uint8_t*)first_block + sizeof(heap_block));
    first_block->length = heap_size - sizeof(heap_block);
    first_block->is_free = true;
    first_block->prev = nullptr;
    first_block->next = nullptr;
}

void defragment() {
    if (heap_base == nullptr) {
        Log::errf("Heap not initialized, cannot defragment");
        return;
    }

    heap_block* current = (heap_block*)heap_base;
    while (current != nullptr) {
        if (current->is_free && current->next != nullptr && current->next->is_free) {
            current->length += current->next->length + sizeof(heap_block);
            current->next = current->next->next;

            if (current->next != nullptr) {
                current->next->prev = current;
            }
        } else {
            current = current->next;
        }
    }
}

void* malloc(size_t n) {
    heap_block* current = (heap_block*)heap_base;
    heap_block* best_fit = nullptr;

    while (current != nullptr) {
        if (current->is_free && current->length >= n) {
            if (best_fit == nullptr || current->length < best_fit->length) {
                best_fit = current;
            }
        }
        current = current->next;
    }

    if (best_fit == nullptr) {
        Log::errf("Malloc: No suitable free block found for %zu bytes", n);
        return nullptr;
    }

    if (best_fit->length > n + sizeof(heap_block)) {
        heap_block* new_block = (heap_block*)((uint8_t*)best_fit + sizeof(heap_block) + n);
        new_block->base = (void*)((uint8_t*)new_block + sizeof(heap_block));
        new_block->length = best_fit->length - n - sizeof(heap_block);
        new_block->is_free = true;
        new_block->next = best_fit->next;
        if (best_fit->next) {
            best_fit->next->prev = new_block;
        }
        best_fit->next = new_block;
        new_block->prev = best_fit;
        best_fit->length = n;
    }

    best_fit->is_free = false;
    return best_fit->base;
}

void* malloc_aligned(size_t n, size_t alignment) {
    if ((alignment & (alignment - 1)) != 0) return nullptr;

    heap_block* current = (heap_block*)heap_base;
    heap_block* best_fit = nullptr;

    while (current != nullptr) {
        if (current->is_free && current->length >= n + alignment) {
            if (best_fit == nullptr || current->length < best_fit->length) {
                best_fit = current;
            }
        }
        current = current->next;
    }

    if (best_fit == nullptr) {
        Log::errf("malloc_aligned: No suitable free block found for %zu bytes", n);
        return nullptr;
    }

    uintptr_t raw_addr = (uintptr_t)best_fit->base;
    uintptr_t aligned_addr = (raw_addr + alignment - 1) & ~(alignment - 1);
    size_t padding = aligned_addr - raw_addr;

    if (padding > 0) {
        if (best_fit->length <= padding + sizeof(heap_block)) {
            return nullptr;
        }

        heap_block* pad_block = (heap_block*)((uint8_t*)best_fit + sizeof(heap_block) + padding);
        pad_block->base = (void*)((uint8_t*)pad_block + sizeof(heap_block));
        pad_block->length = best_fit->length - padding - sizeof(heap_block);
        pad_block->is_free = true;
        pad_block->next = best_fit->next;
        if (best_fit->next) best_fit->next->prev = pad_block;
        pad_block->prev = best_fit;

        best_fit->length = padding;
        best_fit->next = pad_block;
        best_fit = pad_block;
    }

    if (best_fit->length > n + sizeof(heap_block)) {
        heap_block* new_block = (heap_block*)((uint8_t*)best_fit + sizeof(heap_block) + n);
        new_block->base = (void*)((uint8_t*)new_block + sizeof(heap_block));
        new_block->length = best_fit->length - n - sizeof(heap_block);
        new_block->is_free = true;
        new_block->next = best_fit->next;
        if (best_fit->next) new_block->next->prev = new_block;
        new_block->prev = best_fit;
        best_fit->next = new_block;
        best_fit->length = n;
    }

    best_fit->is_free = false;
    return best_fit->base;
}

void* realloc(void* ptr, size_t n) {
    if (ptr == nullptr) {
        return malloc(n);
    }

    if (n == 0) {
        free(ptr);
        return nullptr;
    }

    heap_block* current = (heap_block*)heap_base;
    while (current != nullptr) {
        if (current->base == ptr) {
            if (current->length >= n) {
                current->length = n;
                return current->base;
            }

            void* new_ptr = malloc(n);
            if (new_ptr) {
                memcpy(new_ptr, ptr, current->length);
                free(ptr);
                return new_ptr;
            }

            Log::errf("Realloc: Failed to allocate %zu bytes", n);
            return nullptr;
        }
        current = current->next;
    }

    Log::errf("Realloc: Failed to find memory block for %p", ptr);
    return nullptr;
}

void* calloc(size_t n, size_t size) {
    size_t total_size = n * size;

    void* ptr = malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

void free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }

    heap_block* current = (heap_block*)heap_base;
    while (current != nullptr) {
        if (current->base == ptr) {
            current->is_free = true;
            if (current->next && current->next->is_free) {
                current->length += current->next->length + sizeof(heap_block);
                current->next = current->next->next;
                if (current->next) {
                    current->next->prev = current;
                }
            }

            if (current->prev && current->prev->is_free) {
                current->prev->length += current->length + sizeof(heap_block);
                current->prev->next = current->next;
                if (current->next) {
                    current->next->prev = current->prev;
                }
            }

            return;
        }
        current = current->next;
    }

    Log::errf("Free: Attempted to free a block that wasn't allocated: %p", ptr);
}

}
