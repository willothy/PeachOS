#include "vga_writer.h"

vga_writer writer;

uint16_t vga_make_char(char c, VGA_COLOR color) {
    // little endian, color at 1st byte
    return (color << 8) | c;
}

void vga_put_char(int x, int y, char c, VGA_COLOR color) {
    writer.buffer[y * BUFFER_WIDTH + x] = vga_make_char(c, color);
}

void vga_write_char(char c, VGA_COLOR color) {
    if (c == '\n') {
        writer.col = 0;
        writer.row++;
        return;
    }

    writer.buffer[writer.row * BUFFER_WIDTH + writer.col] =
        vga_make_char(c, color);
    writer.col += 1;

    if (writer.col >= BUFFER_WIDTH) {
        writer.col = 0;
        writer.row += 1;
    }
}

void newline() {
    writer.col = 0;
    writer.row += 1;
}

void vga_print(char *str, VGA_COLOR color) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_write_char(str[i], color);
    }
}

void vga_print_int(uint32_t n, VGA_COLOR color) {
    if (n == 0) {
        vga_print("0", color);
        return;
    }

    char buf[11];
    int i = 10;
    buf[10] = '\0';
    while (n > 0) {
        buf[--i] = '0' + (n % 10);
        n /= 10;
    }
    vga_print(buf + i, color);
}

void vga_clear() {
    for (int x = 0; x < BUFFER_WIDTH; x++) {
        for (int y = 0; y < BUFFER_HEIGHT; y++) {
            vga_put_char(x, y, ' ', 0);
        }
    }
}

void vga_init() {
    writer.buffer = (uint16_t *)0xb8000;
    writer.col = 0;
    writer.row = 0;
    vga_clear(writer);
}