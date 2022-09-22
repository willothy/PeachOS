#ifndef PAGING_H_
#define PAGING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGING_CACHE_DISABLED 0x10 // 0b00010000
#define PAGING_WRITE_THROUGH 0x8   // 0b00001000
#define PAGING_ACCESS_FROM_ALL 0x4 // 0b00000100
#define PAGING_IS_WRITEABLE 0x2    // 0b00000010
#define PAGING_IS_PRESENT 0x1      // 0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

struct paging_4gb_chunk {
    uint32_t* directory_entry;
};

// Assembly functions
void paging_load_directory(uint32_t* directory);

void enable_paging();

// C functions
struct paging_4gb_chunk* paging_new_4gb(uint8_t flags);

void paging_switch(uint32_t* directory);

int paging_set(uint32_t* directory, void* virt, uint32_t val);

uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk);

int paging_get_indices(void* virtual_addr, uint32_t* dir_index_out,
                       uint32_t* table_index_out);

bool paging_is_aligned(void* addr);

#endif /* PAGING_H_ */
