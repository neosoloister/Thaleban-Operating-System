[bits 16]
[org 0x7e00]
KERNEL_OFFSET equ 0x1000

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7e00

    mov si, stage2_msg
    call print_string

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    mov si, a20_msg
    call print_string

    ; Reset Disk
    mov ah, 0
    mov dl, 0x80
    int 0x13
    jc error

    ; Load Kernel
    mov bx, KERNEL_OFFSET
    mov dh, 0
    mov dl, 0x80
    mov ch, 0
    mov cl, 3
    mov al, 50
    mov ah, 0x02
    int 0x13
    
    jc error

    mov si, kernel_load_msg
    call print_string

    ; Load GDT
    lgdt [gdt_descriptor]

    mov si, gdt_msg
    call print_string

    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm

error:
    mov si, disk_error_msg
    call print_string
    jmp $

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

    call KERNEL_OFFSET

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
gdt_msg: db "GDT LOADED", 13, 10, 0
kernel_load_msg: db "KERNEL LOADED", 13, 10, 0
disk_error_msg: db "DISK ERROR", 13, 10, 0

times 512-($-$$) db 0
