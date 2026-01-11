#include "kernel.h"
#include "../drivers/keyboard.h"
#include "../shell/shell.h"

void cpu_idle() {
    for (;;) __asm__ __volatile__ ("hlt");
}

void kernel_main() {
    vga_clear();
    isr_install();
    set_idt();
    init_pic();
    irq_install();
    __asm__ __volatile__("sti");
    init_keyboard();
    shell_init();
    
    cpu_idle();
}