#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "../libc/kprintf.h"

static void keyboard_callback(registers_t *regs) {
    // PIC leaves scancode in port 0x60
    uint8_t scancode = port_byte_in(0x60);
    char *sc_ascii;

    kprintf("Keyboard scancode: %x\n", scancode);
    (void)regs;
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback); 
    
    // Read current mask
    uint8_t mask = port_byte_in(0x21);
    // Clear bit 1 (IRQ1)
    mask = mask & ~(1 << 1);
    // Write back
    port_byte_out(0x21, mask);
}
