#include "pmm.h"

#define PAGE_SIZE 0x1000
#define PMM_UNSAFE_LIMIT 0x100000

typedef struct {
    uint64_t base;
    uint64_t length;
    uint8_t idx:2;
    uint8_t dirty:1;
    uint8_t is_bitmap:1;
    uint8_t is_user:1;
    uint8_t unsafe:1;
    uint8_t reserved:2;
} PmmSegment;

typedef struct {
    PmmSegment segment0;
    PmmSegment segment1;
    PmmSegment segment2;
    PmmSegment segment3;
} Pmm;

typedef struct {
    uint64_t page_base;
    uint8_t  allocated;
} PageEntry;

static PageEntry* page_table[4];
static uint64_t page_table_size[4];

Pmm pmm_ins;
Pmm *pmm;

void init_segment(PmmSegment* seg, uint64_t base, uint64_t length, uint8_t idx) {
    seg->base = PA2VAu64(base);
    seg->length = length;
    seg->idx = idx;
    seg->dirty = 0;
    seg->is_bitmap = 0;
    seg->is_user = 0;

    seg->unsafe = (base < PMM_UNSAFE_LIMIT) ? 1 : 0;
    seg->reserved = 0;

    printk("PMM Segment %d: base=0x%lx, len=0x%lx, unsafe=%d\n", idx, seg->base, seg->length, seg->unsafe);
}

int pmm_init(uint64_t largest1sz, uint64_t largest1base,
             uint64_t largest2sz, uint64_t largest2base,
             uint64_t largest3sz, uint64_t largest3base,
             uint64_t largest4sz, uint64_t largest4base) {
    pmm = &pmm_ins;

    init_segment(&pmm->segment0, largest1base, largest1sz, 0);
    init_segment(&pmm->segment1, largest2base, largest2sz, 1);
    init_segment(&pmm->segment2, largest3base, largest3sz, 2);
    init_segment(&pmm->segment3, largest4base, largest4sz, 3);

    return 0;
}

static uint64_t page_count(PmmSegment* seg) {
    return seg->length / PAGE_SIZE;
}

void pmm_setup_page_tables() {
    for (int i = 0; i < 4; i++) {
        PmmSegment* seg = &pmm->segment0 + i;
        if (!seg->length || seg->unsafe) continue;

        uint64_t count = page_count(seg);
        page_table[i] = (PageEntry*)seg->base;
        page_table_size[i] = count;

        for (uint64_t j = 0; j < count; j++) {
            page_table[i][j].page_base = seg->base + j * PAGE_SIZE;
            page_table[i][j].allocated = 0;
        }

        seg->is_bitmap = 1;
    }
}

void pmm_defrag() {
    for (int i = 0; i < 4; i++) {
        if (!page_table[i]) continue;

        uint64_t next_free_slot = 0;
        for (uint64_t j = 0; j < page_table_size[i]; j++) {
            if (page_table[i][j].allocated) {
                if (j != next_free_slot) {
                    page_table[i][next_free_slot].page_base = page_table[i][j].page_base;
                    page_table[i][next_free_slot].allocated = 1;
                    page_table[i][j].allocated = 0;
                }
                next_free_slot++;
            }
        }
        for (uint64_t j = next_free_slot; j < page_table_size[i]; j++) {
            page_table[i][j].allocated = 0;
        }
    }
}

void* palloc(uint64_t npages) {
    for (int i = 0; i < 4; i++) {
        if (!page_table[i]) continue;

        uint64_t contiguous = 0;
        uint64_t start_idx = 0;

        for (uint64_t j = 0; j < page_table_size[i]; j++) {
            if (!page_table[i][j].allocated) {
                if (contiguous == 0) start_idx = j;
                contiguous++;
                if (contiguous == npages) {
                    for (uint64_t k = 0; k < npages; k++)
                        page_table[i][start_idx + k].allocated = 1;
                    return (void*)page_table[i][start_idx].page_base;
                }
            } else {
                contiguous = 0;
            }
        }

        pmm_defrag();

        contiguous = 0;
        start_idx = 0;
        for (uint64_t j = 0; j < page_table_size[i]; j++) {
            if (!page_table[i][j].allocated) {
                if (contiguous == 0) start_idx = j;
                contiguous++;
                if (contiguous == npages) {
                    for (uint64_t k = 0; k < npages; k++)
                        page_table[i][start_idx + k].allocated = 1;
                    return (void*)page_table[i][start_idx].page_base;
                }
            } else {
                contiguous = 0;
            }
        }
    }
    return NULL;
}

void pfree(void* ptr) {
    uint64_t addr = (uint64_t)ptr;
    for (int i = 0; i < 4; i++) {
        if (!page_table[i]) continue;

        for (uint64_t j = 0; j < page_table_size[i]; j++) {
            if (page_table[i][j].page_base == addr) {
                while (j < page_table_size[i] && page_table[i][j].allocated) {
                    page_table[i][j].allocated = 0;
                    j++;
                }
                return;
            }
        }
    }
    pmm_defrag();
}
