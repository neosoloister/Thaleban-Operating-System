#include "shell.h"
#include "../kernel/kprintf.h"
#include "../libc/string.h"
#include "../cpu/ports.h"
#include "../drivers/vga.h"
#include "../libc/malloc.h"
#include "../fs/fat.h"

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
    kprintf("ls - List files\n");
    kprintf("cd [dir] - Change directory\n");
    kprintf("cat [file] - Read file\n");
    kprintf("touch [file] - Create file\n");
    kprintf("mkdir [dir] - Create directory\n");
    kprintf("cp [src] [dst] - Copy file\n");
    kprintf("mv [src] [dst] - Move/Rename file\n");
    kprintf("write \"text\" [file] - Write to file\n");
    kprintf("rm [file] - Remove file\n");
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

static void cmd_ls() {
    fat_list_current_dir();
}

static void cmd_cat(char *input) {
    if (input[3] != ' ') {
        kprintf("Usage: cat <filename>\n");
        return;
    }
    
    char *filename = input + 4;
    // Trip spaces? fat_open handles basic parsing but simplistic.
    // The current fat_open parser expects simplistic uppercase input or conversion.
    // The shell passes "cat filename" -> filename = "filename" (starts at index 4)
    
    int size = fat_open(filename);
    if (size < 0) {
        kprintf("File not found: ");
        kprintf(filename);
        kprintf("\n");
        return;
    }
    
    char *buf = (char*)malloc(size + 1);
    if (!buf) {
        kprintf("Memory allocation failed\n");
        return;
    }
    
    fat_read(buf, size);
    buf[size] = 0; // Null terminate
    
    kprintf(buf);
    kprintf("\n"); // Newline after file content
    
    free(buf);
}

static void cmd_cd(char *input) {
    if (input[2] != ' ') {
        kprintf("Usage: cd <directory>\n");
        return;
    }
    
    char *dirname = input + 3;
    if (fat_change_dir(dirname) == 0) {
        kprintf("Directory changed to ");
        kprintf(dirname);
        kprintf("\n");
    } else {
        kprintf("Directory not found: ");
        kprintf(dirname);
        kprintf("\n");
    }
}

static void cmd_touch(char *input) {
    if (input[5] != ' ') { kprintf("Usage: touch <file>\n"); return; }
    if (fat_touch(input + 6) == 0) kprintf("File created.\n");
    else kprintf("Error creating file.\n");
}

static void cmd_mkdir(char *input) {
    if (input[5] != ' ') { kprintf("Usage: mkdir <dir>\n"); return; }
    if (fat_mkdir(input + 6) == 0) kprintf("Directory created.\n");
    else kprintf("Error creating directory.\n");
}

static void cmd_cp(char *input) {
    char *p = input + 3;
    if (*p == 0) { kprintf("Usage: cp <src> <dest>\n"); return; }
    while(*p == ' ') p++;
    char *src = p;
    while(*p != ' ' && *p != 0) p++;
    if (*p == 0) { kprintf("Usage: cp <src> <dest>\n"); return; }
    *p = 0; 
    p++;
    while(*p == ' ') p++;
    char *dest = p;
    
    if (fat_cp(src, dest) == 0) kprintf("File copied.\n");
    else kprintf("Error copying file.\n");
}

static void cmd_mv(char *input) {
    char *p = input + 3;
    if (*p == 0) { kprintf("Usage: mv <src> <dest>\n"); return; }
    while(*p == ' ') p++;
    char *src = p;
    while(*p != ' ' && *p != 0) p++;
    if (*p == 0) { kprintf("Usage: mv <src> <dest>\n"); return; }
    *p = 0;
    p++;
    while(*p == ' ') p++;
    char *dest = p;
    
    if (fat_mv(src, dest) == 0) kprintf("File moved/renamed.\n");
    else kprintf("Error moving file.\n");
}

static void cmd_write(char *input) {
    // format: write "content" filename
    char *p = input + 5;
    while (*p == ' ') p++;
    
    if (*p != '"') {
        kprintf("Usage: write \"string\" <filename>\n");
        return;
    }
    p++; // skip opening quote
    
    char *start = p;
    while (*p != '"' && *p != 0) p++;
    
    if (*p == 0) {
        kprintf("Usage: write \"string\" <filename>\n");
        return;
    }
    
    *p = 0; // Terminate string
    char *content = start;
    
    p++; // skip closing quote
    while (*p == ' ') p++;
    
    if (*p == 0) {
        kprintf("Usage: write \"string\" <filename>\n");
        return;
    }
    char *filename = p;
    
    if (fat_write_file(filename, content, strlen(content)) == 0) {
        kprintf("File written.\n");
    } else {
        kprintf("Error writing file.\n");
    }
}

static void cmd_rm(char *input) {
    if (input[2] != ' ') { kprintf("Usage: rm <file>\n"); return; }
    char *filename = input + 3;
    while(*filename == ' ') filename++;
    
    if (fat_delete_file(filename) == 0) {
        kprintf("File deleted.\n");
    } else {
        kprintf("Error deleting file.\n");
    }
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
    else if (strcmp(input, "ls") == 0) {
        cmd_ls();
    }
    else if (strncmp(input, "cd", 2) == 0 && (input[2] == '\0' || input[2] == ' ')) {
        cmd_cd(input);
    }
    else if (strncmp(input, "cat", 3) == 0 && (input[3] == '\0' || input[3] == ' ')) {
        cmd_cat(input);
    }
    else if (strncmp(input, "touch", 5) == 0) cmd_touch(input);
    else if (strncmp(input, "mkdir", 5) == 0) cmd_mkdir(input);
    else if (strncmp(input, "cp", 2) == 0) cmd_cp(input);
    else if (strncmp(input, "mv", 2) == 0) cmd_mv(input);
    else if (strncmp(input, "write", 5) == 0) cmd_write(input);
    else if (strncmp(input, "rm", 2) == 0) cmd_rm(input);
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