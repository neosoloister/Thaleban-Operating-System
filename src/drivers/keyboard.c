#include "keyboard.h"
#include "vga.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "../libc/kprintf.h"

#include <stdbool.h>

static bool shift_pressed = false;
static bool caps_lock = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;

// Scancode to Keycode Mapping
const keycode_t scancode_to_keycode[] = {
    KEY_UNKNOWN, KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE,
    KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACKET, KEY_RIGHTBRACKET, KEY_ENTER,
    KEY_CTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, KEY_BACKTICK,
    KEY_LSHIFT, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RSHIFT,
    KEY_KP_STAR, KEY_ALT, KEY_SPACE, KEY_CAPSLOCK,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_NUMLOCK, KEY_SCROLLLOCK,
    KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_MINUS_NUM,
    KEY_LEFT, KEY_CENTER, KEY_RIGHT, KEY_PLUS_NUM,
    KEY_END, KEY_DOWN, KEY_PAGEDOWN, KEY_INSERT, KEY_DELETE,
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_F11, KEY_F12
};

keycode_t keycode_to_mkeycode(keycode_t key, uint8_t modifiers) {
    (void)modifiers; 
    return key;
}

char mkeycode_to_utf8(keycode_t key) {
    char c = 0;
    
    // Handle Ctrl combinations
    if (ctrl_pressed) {
        switch (key) {
            case KEY_A: return 1;
            case KEY_B: return 2;
            case KEY_C: return 3;
            case KEY_D: return 4;
            case KEY_E: return 5;
            case KEY_F: return 6;
            case KEY_G: return 7;
            case KEY_H: return 8;
            case KEY_I: return 9;
            case KEY_J: return 10;
            case KEY_K: return 11;
            case KEY_L: return 12;
            case KEY_M: return 13;
            case KEY_N: return 14;
            case KEY_O: return 15;
            case KEY_P: return 16;
            case KEY_Q: return 17;
            case KEY_R: return 18;
            case KEY_S: return 19;
            case KEY_T: return 20;
            case KEY_U: return 21;
            case KEY_V: return 22;
            case KEY_W: return 23;
            case KEY_X: return 24;
            case KEY_Y: return 25;
            case KEY_Z: return 26;
            default: break;
        }
    }

    if (key >= KEY_1 && key <= KEY_9) {
        c = '1' + (key - KEY_1);
         if (shift_pressed) {
            const char syms[] = "!@#$%^&*(";
            c = syms[key - KEY_1];
        }
    } else if (key == KEY_0) {
        c = shift_pressed ? ')' : '0';
    } else {
         switch(key) {
            case KEY_A: c = (shift_pressed || caps_lock) ? 'A' : 'a'; break;
            case KEY_B: c = (shift_pressed || caps_lock) ? 'B' : 'b'; break;
            case KEY_C: c = (shift_pressed || caps_lock) ? 'C' : 'c'; break;
            case KEY_D: c = (shift_pressed || caps_lock) ? 'D' : 'd'; break;
            case KEY_E: c = (shift_pressed || caps_lock) ? 'E' : 'e'; break;
            case KEY_F: c = (shift_pressed || caps_lock) ? 'F' : 'f'; break;
            case KEY_G: c = (shift_pressed || caps_lock) ? 'G' : 'g'; break;
            case KEY_H: c = (shift_pressed || caps_lock) ? 'H' : 'h'; break;
            case KEY_I: c = (shift_pressed || caps_lock) ? 'I' : 'i'; break;
            case KEY_J: c = (shift_pressed || caps_lock) ? 'J' : 'j'; break;
            case KEY_K: c = (shift_pressed || caps_lock) ? 'K' : 'k'; break;
            case KEY_L: c = (shift_pressed || caps_lock) ? 'L' : 'l'; break;
            case KEY_M: c = (shift_pressed || caps_lock) ? 'M' : 'm'; break;
            case KEY_N: c = (shift_pressed || caps_lock) ? 'N' : 'n'; break;
            case KEY_O: c = (shift_pressed || caps_lock) ? 'O' : 'o'; break;
            case KEY_P: c = (shift_pressed || caps_lock) ? 'P' : 'p'; break;
            case KEY_Q: c = (shift_pressed || caps_lock) ? 'Q' : 'q'; break;
            case KEY_R: c = (shift_pressed || caps_lock) ? 'R' : 'r'; break;
            case KEY_S: c = (shift_pressed || caps_lock) ? 'S' : 's'; break;
            case KEY_T: c = (shift_pressed || caps_lock) ? 'T' : 't'; break;
            case KEY_U: c = (shift_pressed || caps_lock) ? 'U' : 'u'; break;
            case KEY_V: c = (shift_pressed || caps_lock) ? 'V' : 'v'; break;
            case KEY_W: c = (shift_pressed || caps_lock) ? 'W' : 'w'; break;
            case KEY_X: c = (shift_pressed || caps_lock) ? 'X' : 'x'; break;
            case KEY_Y: c = (shift_pressed || caps_lock) ? 'Y' : 'y'; break;
            case KEY_Z: c = (shift_pressed || caps_lock) ? 'Z' : 'z'; break;
            
            case KEY_SPACE: c = ' '; break;
            case KEY_BACKSPACE: c = '\b'; break;
            case KEY_ENTER: c = '\n'; break;
            case KEY_TAB: c = '\t'; break;
            case KEY_MINUS: c = shift_pressed ? '_' : '-'; break;
            case KEY_EQUAL: c = shift_pressed ? '+' : '='; break;
            case KEY_LEFTBRACKET: c = shift_pressed ? '{' : '['; break;
            case KEY_RIGHTBRACKET: c = shift_pressed ? '}' : ']'; break;
            case KEY_SEMICOLON: c = shift_pressed ? ':' : ';'; break;
            case KEY_QUOTE: c = shift_pressed ? '"' : '\''; break;
            case KEY_BACKTICK: c = shift_pressed ? '~' : '`'; break;
            case KEY_BACKSLASH: c = shift_pressed ? '|' : '\\'; break;
            case KEY_COMMA: c = shift_pressed ? '<' : ','; break;
            case KEY_DOT: c = shift_pressed ? '>' : '.'; break;
            case KEY_SLASH: c = shift_pressed ? '?' : '/'; break;
            default: c = 0; break;
        }
    }
    return c;
}

static void keyboard_callback(registers_t *regs) {
    // PIC leaves scancode in port 0x60
    uint8_t scancode = port_byte_in(0x60);
    
    // Check for Key Release (Break Code, usually +0x80)
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
         if (released < sizeof(scancode_to_keycode)/sizeof(keycode_t)) {
             keycode_t key = scancode_to_keycode[released];
             if (key == KEY_LSHIFT || key == KEY_RSHIFT) shift_pressed = false;
             if (key == KEY_CTRL) ctrl_pressed = false;
             if (key == KEY_ALT) alt_pressed = false;
         }
        return;
    }

    if (scancode >= sizeof(scancode_to_keycode)/sizeof(keycode_t)) return;

    keycode_t key = scancode_to_keycode[scancode];
    
    // Update modifiers
    if (key == KEY_LSHIFT || key == KEY_RSHIFT) shift_pressed = true;
    if (key == KEY_CTRL) ctrl_pressed = true;
    if (key == KEY_ALT) alt_pressed = true;
    if (key == KEY_CAPSLOCK) caps_lock = !caps_lock;

    // Convert to Modified Keycode (Pass-through for now, logic handled in UTF8 step)
    keycode_t mkey = keycode_to_mkeycode(key, 0);

    // Convert to UTF-8
    char c = mkeycode_to_utf8(mkey);
    
    if (c) {
        // For visualization of control codes (temporary)
        if (c < 32 && c > 0 && c != '\n' && c != '\t' && c != '\b') {
             kprintf("^"); 
             char k = c + '@'; // 1 ('\x01') + 64 = 65 ('A')
             char str[2] = {k, '\0'};
             kprintf(str);
        } else {
            char str[2] = {c, '\0'};
            kprintf(str);
        }
    }
    
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
