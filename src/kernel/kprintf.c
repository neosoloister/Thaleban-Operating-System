#include "kprintf.h"
#include "../drivers/vga.h"
#include <stdarg.h>
#include <stdbool.h>

void itoa(int n, char *buffer, int base) {
    int i = 0;
    bool is_negative = false;

    if (n == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    if (n < 0 && base == 10) {
        is_negative = true;
        n = -n;
    }

    while (n != 0) {
        int rem = n % base;
        buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        n = n / base;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

void htos(unsigned int n, char *buffer) {
    char hex[] = "0123456789ABCDEF";
    int i = 0;
    
    buffer[i++] = '0';
    buffer[i++] = 'x';

    if (n == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }

    int start = i;
    while (n > 0) {
        buffer[i++] = hex[n % 16];
        n /= 16;
    }
    buffer[i] = '\0';

    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

void kprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == 'c') {
                char c = (char)va_arg(args, int);
                vga_putch(c);
            } else if (*format == 's') {
                char *s = va_arg(args, char *);
                vga_write(s);
            } else if (*format == 'd') {
                int d = va_arg(args, int);
                char buffer[32];
                itoa(d, buffer, 10);
                vga_write(buffer);
            } else if (*format == 'x') {
                unsigned int x = va_arg(args, unsigned int);
                char buffer[32];
                htos(x, buffer);
                vga_write(buffer);
            } else if (*format == '%') {
                vga_putch('%');
            }
        } else {
            vga_putch(*format);
        }
        format++;
    }

    va_end(args);
}