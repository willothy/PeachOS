#include "kernel.h"
#include "idt.h"
#include "io.h"
#include "kheap.h"
#include "paging.h"
#include "vga_writer.h"

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void strcpy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static struct paging_4gb_chunk *kernel_chunk = 0;

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