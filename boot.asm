ORG 0
BITS 16

_start:
    jmp short start
    nop

times 33 db 0

start:
    jmp 0x7c0:step2

handle_zero:
    mov si, int_zero_msg
    call println
    iret

handle_one:
    mov si, int_one_msg
    call println
    iret

step2:
    cli ; clear interrupts
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00,
    mov ss, ax
    mov sp, 0x7c00
    sti ; enable interrupts

    mov word[ss:0x00], handle_zero
    mov word[ss:0x02], 0x7c0
    mov word[ss:0x04], handle_one
    mov word[ss:0x06], 0x7c0
    mov si, message ; move address of message into si
    call println

    call disk_read
    mov si, buffer
    call println
    jmp $ ; infinite loop

disk_read:
; Read sector from disk to memory
; AH = 02h
; AL = number of sectors to read ( must be nonzero )
; CH = low eight bits of cylinder number
; CL sector number 1-63 ( bits 0-5 )
; high two bits of cylinder ( bits 6-7 , hard disk only )
; DH = head number
; DL = drive number ( bit 7 set for hard disk )
; ES : BX- > data buffer

; Return:
; CF set on error
; if AH = 11h ( corrected ECC error ) , AL = burst length
; CF clear if successful
; AH = status ( see # 00234 )
; AL = number of sectors transferred ( only valid if CF set for some
; BIOSes )
    mov ah, 2
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov bx, buffer
    int 0x13
    jc disk_read_error
    ret

disk_read_error:
    mov si, disk_read_error_msg
    call println
    ret

println:
    call print
    mov al, 10
    call print_char
    mov al, 13
    call print_char
    ret

print:
    mov bx, 0 ; set bx to 0 to prepare for int 0x10
.loop:
    lodsb ; load byte si is pointing to into al, increment si
    or al, al ; check if null terminator is reached
    jz .done ; if so, jump to .done
    call print_char ; otherwise, print the character
    jmp .loop ; and loop back
.done:
    ret

print_char:
    mov ah, 0eh ; set ah to 0eh to prepare for int 0x10
    int 0x10 ; print character in al
    ret

message: db 'Hello world!', 0
int_zero_msg: db 'Interrupt 0', 0
int_one_msg: db 'Interrupt 1', 0
disk_read_error_msg: db 'Failed to load sector', 0
newline: db 10, 13
times 510-($-$$) db 0 ; pad to 510 bytes
dw 0xaa55

buffer: