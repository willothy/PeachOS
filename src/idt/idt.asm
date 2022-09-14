section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load
global no_interrupt

idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8] ; move val from addr ebp+8 to ebx
    lidt [ebx] ; load idt

    pop ebp
    ret

int21h:
    cli
    pushad ; Save GP register states

    call int21h_handler

    popad ; Restore GP register states
    sti
    iret

no_interrupt:
    cli
    pushad ; Save GP register states

    call no_interrupt_handler

    popad ; Restore GP register states
    sti
    iret