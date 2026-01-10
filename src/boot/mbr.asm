[bits 16]
[org 0x7c00]

start:
    ; Zeroing
    xor ax, ax
    mov ds, ax
    mov es, ax
    ; Setting Stack
    mov ss, ax
    mov sp, 0x7c00

    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x7e00
    int 0x13
    
    jc error

    mov si, mbr_msg
    call print_string
    jmp 0x0000:0x7e00

error:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
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

mbr_msg: db "MBR LOADED", 13, 10, 0

times 510-($-$$) db 0
dw 0xaa55