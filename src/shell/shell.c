#include "shell.h"
#include "../kernel/kprintf.h"
#include "../libc/string.h"
#include "../cpu/ports.h"
#include "../drivers/vga.h"
#include "../libc/malloc.h"

#define MAX_COMMAND_LEN 256

static char command_buffer[MAX_COMMAND_LEN];
static int buffer_index = 0;

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);

void shell_init() {
    kprintf("==========================================\n");
    kprintf("   Welcome to Thaleban Operating System\n");
    kprintf("        Made by Thaleban Thaokloy\n");
    kprintf("==========================================\n");
    kprintf("\n");
    kprintf("> ");
    buffer_index = 0;
}

static void cmd_help() {
    kprintf("Available commands:\n");
    kprintf("help - Show this message\n");
    kprintf("clear - Clear screen\n");
    kprintf("credit - Show credits\n");
    kprintf("poweroff - Poweroff system\n");
    kprintf("reboot - Reboot system\n");
    kprintf("echo [text] - Echo text\n");

    kprintf("mstats - Show memory statistics\n");
}

static void cmd_clear() {
    vga_clear();
}

static void cmd_credit() {
    kprintf("=========================\n");
    kprintf("Thaleban Operating System\n");
    kprintf("Made by Thaleban Thaokloy\n");
    kprintf("=========================\n");
}

static void cmd_poweroff() {
    kprintf("Shutting down...\n");
    // QEMU
    port_word_out(0x604, 0x2000);
    // VirtualBox
    port_word_out(0x4004, 0x3400);
    // Bochs
    port_word_out(0xB004, 0x2000);
}

static void cmd_reboot() {
    kprintf("Rebooting...\n");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    __asm__ __volatile__("hlt");
}

static void cmd_echo(char *input) {
    if (input[4] == ' ') {
        kprintf(input + 5);
    }
    kprintf("\n");
}





static void process_command(char *input) {
    if (strcmp(input, "help") == 0) {
        cmd_help();
    } 
    else if (strcmp(input, "clear") == 0) {
        cmd_clear();
    }
    else if (strcmp(input, "credit") == 0) {
        cmd_credit();
    }
    else if (strcmp(input, "poweroff") == 0) {
        cmd_poweroff();
    }
    else if (strcmp(input, "reboot") == 0) {
        cmd_reboot();
    }
    else if (strcmp(input, "mstats") == 0) {
        mstats();
    }

    else if (strncmp(input, "echo", 4) == 0 && (input[4] == '\0' || input[4] == ' ')) {
        cmd_echo(input);
    }
    else {
        kprintf("Unknown command: ");
        kprintf(input);
        kprintf("\n");
    }
}

static void execute_command() {
    kprintf("\n");
    
    if (buffer_index == 0) {
        kprintf("> ");
        return;
    }

    command_buffer[buffer_index] = '\0';

    process_command(command_buffer);

    // Reset buffer
    buffer_index = 0;
    kprintf("> ");
}

void shell_input(char c) {
    if (c == '\n') {
        execute_command();
    } 
    else if (c == '\b') {
        if (buffer_index > 0) {
            buffer_index--;
            kprintf("\b");
        }
    } 
    else {
        if (buffer_index < MAX_COMMAND_LEN - 1) {
            command_buffer[buffer_index++] = c;
            char str[2] = {c, '\0'};
            kprintf(str);
        }
    }
}