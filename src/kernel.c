#include "kernel.h"

const int BUFFER_HEIGHT = 25;
const int BUFFER_WIDTH = 80;

void vga_write_char(char *vmem, char c, char color, int *col) {
    vmem[*col] = c;
    vmem[*col + 1] = color;
    *col += 2;
}

void vga_write_str(char *vmem, char *str, char color, int *col) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_write_char(vmem, str[i], color, col);
    }
}

void vga_clear(char *vmem) {
    for (int i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; i++) {
        vmem[i] = 0;
    }
}

void kernel_main() {
    int col = 0;

    char *vmem = (char *)(0xb8000);
    vga_clear(vmem);
    vga_write_str(vmem, "Hello, world!", 0x0f, &col);
}