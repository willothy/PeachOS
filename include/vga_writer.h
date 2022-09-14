#ifndef VGA_WRITER_H_
#define VGA_WRITER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t *buffer;
    uint8_t col;
    uint8_t row;
} vga_writer;

typedef enum {
    VGA_BLACK = 0x0,
    VGA_BLUE = 0x1,
    VGA_GREEN = 0x2,
    VGA_CYAN = 0x3,
    VGA_RED = 0x4,
    VGA_MAGENTA = 0x5,
    VGA_BROWN = 0x6,
    VGA_LIGHT_GREY = 0x7,
    VGA_DARK_GREY = 0x8,
    VGA_LIGHT_BLUE = 0x9,
    VGA_LIGHT_GREEN = 0xA,
    VGA_LIGHT_CYAN = 0xB,
    VGA_LIGHT_RED = 0xC,
    VGA_PINK = 0xD,
    VGA_YELLOW = 0xE,
    VGA_WHITE = 0xF,
} VGA_COLOR;

void vga_put_char(int x, int y, char c, VGA_COLOR color);
void vga_write_char(char c, VGA_COLOR color);
void vga_print(char *str, VGA_COLOR color);
void vga_print_int(uint32_t n, VGA_COLOR color);
void newline();
void vga_clear();
void vga_init();

#define BUFFER_HEIGHT 25
#define BUFFER_WIDTH 80

#endif