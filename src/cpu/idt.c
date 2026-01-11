#include "idt.h"
#include "ports.h"

idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idt_reg;

void set_idt_gate(int n, uint32_t handler) {
    idt[n].low_offset = (uint16_t)((handler) & 0xFFFF);
    idt[n].sel = 0x08;
    idt[n].always0 = 0;
    idt[n].flags = 0x8E; 
    idt[n].high_offset = (uint16_t)(((handler) >> 16) & 0xFFFF);
}

void set_idt() {
    idt_reg.base = (uint32_t) &idt;
    idt_reg.limit = IDT_ENTRIES * sizeof(idt_entry_t) - 1;
    // Load IDT pointers
    __asm__ __volatile__("lidt (%0)" : : "r" (&idt_reg));
}

// PIC Ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// ICW1
#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01

// ICW4
#define ICW4_8086    0x01

void init_pic() {
    // Start initialization
    port_byte_out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    port_byte_out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    // Set vector offsets
    port_byte_out(PIC1_DATA, 0x20); // Master PIC vector offset 32
    port_byte_out(PIC2_DATA, 0x28); // Slave PIC vector offset 40

    // Tell Master about Slave
    port_byte_out(PIC1_DATA, 4);
    port_byte_out(PIC2_DATA, 2);

    // 8086 Mode
    port_byte_out(PIC1_DATA, ICW4_8086);
    port_byte_out(PIC2_DATA, ICW4_8086);

    // Mask all interrupts
    port_byte_out(PIC1_DATA, 0xFF);
    port_byte_out(PIC2_DATA, 0xFF);
}