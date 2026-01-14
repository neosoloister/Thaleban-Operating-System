#include "kernel.h"
#include "../drivers/keyboard.h"
#include "../shell/shell.h"
#include "../libc/malloc.h"

#include "../fs/fat.h"

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
    
    // Initialize FAT File System
    fat_init();
    
    // Initialize Heap at 0x100000 (1MB) with 16MB size
    init_heap(0x100000, 16 * 1024 * 1024);

    for (int i = 0; i < 10; i++) {
        malloc(1024);
    }

    shell_init();
    cpu_idle();
}