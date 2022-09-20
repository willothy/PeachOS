#include "kernel.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "vga_writer.h"

static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main() {
    vga_init();
    vga_print("Initializing...\n", VGA_WHITE);

    // Initialize heap
    kheap_init();
    vga_print("Heap initialized.\n", VGA_WHITE);

    // Initialize IDT
    idt_init();
    vga_print("IDT initialized.\n", VGA_WHITE);

    // Setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT |
                                  PAGING_ACCESS_FROM_ALL);

    // Switch to kernel paging
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    // Enable paging
    enable_paging();
    vga_print("Paging enabled.\n", VGA_WHITE);

    // Enable interrupts
    enable_interrupts();
    vga_print("Interrupts enabled.\n", VGA_WHITE);
}