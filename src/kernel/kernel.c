#include "kernel.h"

void cpu_idle() {
    for (;;) __asm__ __volatile__ ("hlt");
}

void kernel_main () {
    vga_clear();
    vga_write("Thaleban Operating System.");
    cpu_idle();
}