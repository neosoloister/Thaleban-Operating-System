#include "vga.h"

static volatile uint16_t* VGA = (uint16_t*)VGA_ADDRESS;
static uint8_t VGA_ATTR = 0x0F;

uint8_t cursor_row = 0;
uint8_t cursor_column = 0;

static uint8_t vga_attr(uint8_t fg, uint8_t bg) {
    return (bg << 4) | (fg & 0x0F);
}

static unsigned int vga_index(uint8_t row, uint8_t column) {
    return row * VGA_WIDTH + column;
}

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    VGA_ATTR = vga_attr(fg, bg);
}

void vga_set_cursor(uint8_t row, uint8_t column) {
    cursor_row = row;
    cursor_column = column;
}

void vga_scroll() {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        VGA[i] = VGA[i + VGA_WIDTH];
    }
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        VGA[i] = vga_entry(' ', VGA_ATTR);
    }
    cursor_row = VGA_HEIGHT - 1;
}

void vga_putch(char c) {
    if (c == '\n') {
        cursor_column = 0;
        cursor_row++;
    } 
    else {
        VGA[vga_index(cursor_row, cursor_column)] = vga_entry(c, VGA_ATTR);
        cursor_column++;
        if (cursor_column >= VGA_WIDTH) {
            cursor_column = 0;
            cursor_row++;
        }
    }
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_write(char *s) {
    while (*s != '\0') {
        vga_putch(*s);
        s++;
    }
}

void vga_clear() {
    vga_set_color(VGA_WHITE, VGA_BLACK);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA[i] = vga_entry(' ', VGA_ATTR);
    }
    vga_set_cursor(0, 0);
}

