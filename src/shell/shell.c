#include "shell.h"
#include "../libc/kprintf.h"
#include "../libc/string.h"
#include "../drivers/vga.h"

#define MAX_COMMAND_LEN 256

static char command_buffer[MAX_COMMAND_LEN];
static int buffer_index = 0;

int strcmp(const char *s1, const char *s2);

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