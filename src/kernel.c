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

void kernel_main() {
    vga_init();
    vga_print("Initializing...\n", VGA_WHITE);

    // Initialize heap
    kheap_init();
    vga_print("Heap initialized.\n", VGA_WHITE);

    // Initialize IDT
    idt_init();
    vga_print("IDT initialized.\n", VGA_WHITE);

    enable_interrupts();
    vga_print("Interrupts enabled.\n", VGA_WHITE);
}