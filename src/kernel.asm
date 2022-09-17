[BITS 32]
global _start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    in al, 0x92
    or al, 2
    out 0x92, al

    ; Remap master PIC
    mov al, 00010001b
    out 0x20, al ; Tell master PIC

    mov al, 0x20 ; Interrupt 0x20 is where master ISR should start
    out 0x21, al ; Tell master PIC

    mov al, 00000001b ; Tell master PIC to enter x86 mode
    out 0x21, al
    ; End PIC remap

    

    call kernel_main

    ; Infinite loop
    jmp $

times 512 - ($ - $$) db 0