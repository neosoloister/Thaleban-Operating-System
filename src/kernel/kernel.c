#include "kernel.h"

void cpu_idle() {
    for (;;) __asm__ __volatile__ ("hlt");
}

void kernel_main() {
    vga_clear();
    kprintf("Thaleban Operating System.");
    isr_install();
    set_idt();
    init_pic();
    irq_install();
    __asm__ __volatile__("sti");
    init_keyboard();

    char c = 'A';
    char *s = "Hello";
    int d = 12345;
    unsigned int x = 0xDEADBEEF;

    kprintf("Char: %c\n", c);
    kprintf("String: %s\n", s);
    kprintf("Int: %d\n", d);
    kprintf("Hex: %x\n", x);
    
    cpu_idle();
}