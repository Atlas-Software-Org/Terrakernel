#include <cstdint>
#include <cstddef>
#include <cstring>
#include <mem/vmm.hpp>
#include <mem/pmm.hpp>
#include <arch/arch.hpp>
#include <cstdio>

constexpr size_t PAGE_SIZE = 0x1000;

struct Elf64_Ehdr {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct Elf64_Phdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

struct Elf64_Dyn {
    int64_t  d_tag;
    uint64_t d_val;
};

struct Elf64_Rela {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t  r_addend;
};

constexpr uint16_t ET_EXEC = 2;
constexpr uint16_t ET_DYN  = 3;
constexpr uint32_t PT_LOAD = 1;
constexpr uint32_t PT_DYNAMIC = 2;

constexpr int64_t DT_NULL    = 0;
constexpr int64_t DT_RELA    = 7;
constexpr int64_t DT_RELASZ  = 8;
constexpr int64_t DT_RELAENT = 9;

constexpr uint64_t R_X86_64_RELATIVE = 8;

static void apply_relocations(void* load_base, Elf64_Phdr* phdrs, int phnum) {
    Elf64_Dyn* dyn = nullptr;
    for (int i = 0; i < phnum; i++) {
        if (phdrs[i].p_type == PT_DYNAMIC) {
            dyn = reinterpret_cast<Elf64_Dyn*>(load_base + phdrs[i].p_vaddr);
            break;
        }
    }
    if (!dyn) return;

    Elf64_Rela* rela = nullptr;
    size_t rela_size = 0;
    size_t rela_ent = sizeof(Elf64_Rela);

    for (Elf64_Dyn* d = dyn; d->d_tag != DT_NULL; d++) {
        switch (d->d_tag) {
        case DT_RELA:    rela = reinterpret_cast<Elf64_Rela*>(load_base + d->d_val); break;
        case DT_RELASZ:  rela_size = d->d_val; break;
        case DT_RELAENT: rela_ent = d->d_val; break;
        }
    }

    if (!rela || rela_size == 0) return;

    size_t count = rela_size / rela_ent;
    for (size_t i = 0; i < count; i++) {
        Elf64_Rela* r = &rela[i];
        uint64_t type = r->r_info & 0xFFFFFFFF;
        if (type == R_X86_64_RELATIVE) {
            *reinterpret_cast<uint64_t*>(load_base + r->r_offset) = (uint64_t)load_base + r->r_addend;
        }

        Log::infof("Applying relocation: offset=0x%lx, type=%llu, addend=0x%lx", r->r_offset, type, r->r_addend);
    }
}

void run_elf(void* base, size_t filesz) {
    auto* ehdr = reinterpret_cast<Elf64_Ehdr*>(base);

    if (!(ehdr->e_ident[0] == 0x7F && ehdr->e_ident[1] == 'E' &&
          ehdr->e_ident[2] == 'L' && ehdr->e_ident[3] == 'F')) {
        return;
    }

    void* load_base = 0;

    auto* phdr = reinterpret_cast<Elf64_Phdr*>(
        reinterpret_cast<uint8_t*>(base) + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;

        void* seg_base = (ehdr->e_type == ET_DYN) ? (void*)((uint64_t)load_base + phdr[i].p_vaddr)
                                                      : (void*)(phdr[i].p_vaddr);
        size_t memsz = phdr[i].p_memsz;
        size_t filesz = phdr[i].p_filesz;
        void* offset = (void*)(phdr[i].p_offset);

        void* map_start = (void*)((uint64_t)seg_base & ~(PAGE_SIZE - 1));
        void* map_end   = (void*)(((uint64_t)seg_base + memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
        size_t map_pages = ((uint64_t)map_end - (uint64_t)map_start) / PAGE_SIZE;

        for (size_t p = 0; p < map_pages; p++) {
            void* page_addr = map_start + p * PAGE_SIZE;
            mem::vmm::munmap(reinterpret_cast<void*>(page_addr), 1); // ensure it is unmapped
            void* page = mem::vmm::valloc(1);
            mem::vmm::mmap(page, page, 1, PAGE_PRESENT | PAGE_RW | PAGE_USER);
            uint64_t ret = mem::vmm::mmap(reinterpret_cast<void*>(page_addr),
                           page,
                           1,
                           PAGE_PRESENT | PAGE_RW | PAGE_USER);
            mem::vmm::mmap((void*)0x400000, (void*)0x400000, 1, PAGE_PRESENT | PAGE_RW | PAGE_USER);
        }

        mem::memcpy(reinterpret_cast<void*>(seg_base),
                    reinterpret_cast<uint8_t*>(base) + (uint64_t)offset,
                    filesz);
        if (memsz > filesz) {
            mem::memset(reinterpret_cast<uint8_t*>(seg_base) + filesz, 0, memsz - filesz);
        }
    }

    if (ehdr->e_type == ET_DYN) {
        apply_relocations(reinterpret_cast<void*>(load_base), phdr, ehdr->e_phnum);
    }

    void* stack_phys1 = mem::pmm::palloc(1);
    void* stack_phys2 = mem::pmm::palloc(1);

    mem::vmm::mmap(stack_phys1, stack_phys1, 1, PAGE_PRESENT | PAGE_RW | PAGE_USER);
    mem::vmm::mmap(stack_phys2, stack_phys2, 1, PAGE_PRESENT | PAGE_RW | PAGE_USER);

    void* entry_addr = (ehdr->e_type == ET_DYN)
                               ? (void*)(load_base + ehdr->e_entry)
                               : (void*)(ehdr->e_entry);

    arch::x86_64::ringctl::execute_ring3(
        reinterpret_cast<void(*)()>(entry_addr),
        reinterpret_cast<uint8_t*>(stack_phys2) + PAGE_SIZE);
}
