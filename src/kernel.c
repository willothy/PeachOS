#include "kernel.h"
#include "idt.h"
#include "io.h"
#include "vga_writer.h"

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void kernel_main() {
    vga_init();
    vga_print("Initializing...\n", VGA_WHITE);

    // Initialize IDT
    idt_init();
    vga_print("IDT initialized.\n", VGA_WHITE);
}