#include "kernel.h"

void cpu_idle() {
    for (;;) __asm__ __volatile__ ("hlt");
}

void kernel_main() {
    vga_clear();
    kprintf("Thaleban Operating System.");
    set_idt();

    char c = 'A';
    char *s = "Hello";
    int d = 12345;
    unsigned int x = 0xDEADBEEF;

    kprintf("\nChar: %c", c);
    kprintf("\nString: %s", s);
    kprintf("\nInt: %d", d);
    kprintf("\nHex: %x", x);
    
    cpu_idle();
}