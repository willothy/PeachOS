#include "memory/heap/kheap.h"
#include "config.h"
#include "memory/heap/heap.h"
#include "memory/memory.h"
#include "status.h"
#include "vga_writer.h"
struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init() {
    int total_table_entries = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries =
        (HEAP_BLOCK_TABLE_ENTRY*)PEACHOS_HEAP_TABLE_ADDRESS;
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*)PEACHOS_HEAP_ADDRESS, end,
                          &kernel_heap_table);
    if (ISERR(res)) {
        vga_print("Failed to create heap\n", VGA_RED);
    }
}

void* kmalloc(int size) { return heap_malloc(&kernel_heap, size); }
void* kzalloc(int size) {
    void* ptr = kmalloc(size);
    if (!ptr) {
        vga_print("Failed to allocate memory\n", VGA_RED);
        return 0;
    }
    memset(ptr, 0, size);
    return ptr;
}
void kfree(void* ptr) { heap_free(&kernel_heap, ptr); }