[bits 16]
[org 0x7e00]

start:
    mov si, stage2_msg
    call print_string

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    mov si, a20_msg
    call print_string

    ; Load GDT
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    ; Print "P" to video memory (0xb8000) to confirm PM
    mov byte [0xb8000], '3'
    mov byte [0xb8001], 0x0f ; White on black
    mov byte [0xb8002], '2'
    mov byte [0xb8003], 0x0f ; White on black
    mov byte [0xb8004], 'B'
    mov byte [0xb8005], 0x0f ; White on black
    mov byte [0xb8006], 'I'
    mov byte [0xb8007], 0x0f ; White on black
    mov byte [0xb8008], 'T'
    mov byte [0xb8009], 0x0f ; White on black

    jmp $

[bits 16]
print_string:
    push ax
    push si
    mov ah, 0x0e
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    pop si
    pop ax
    ret

; GDT
gdt_start:

gdt_null: ; 8 bytes of zeros
    dd 0x0
    dd 0x0

gdt_code: ; Base=0, Limit=0xfffff, Access=0x9a, Flags=0xc
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data: ; Base=0, Limit=0xfffff, Access=0x92, Flags=0xc
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

stage2_msg: db "STAGE2 LOADED", 13, 10, 0
a20_msg: db "A20 ENABLED", 13, 10, 0
