[bits 16]
[org 0x7e00]

start:
    mov si, stage2_msg
    call print_string
    jmp $

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

stage2_msg: db "STAGE2 LOADED", 13, 10, 0
