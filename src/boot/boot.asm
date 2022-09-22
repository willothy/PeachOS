ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

jmp short start
nop

; FAT16 Header
OEMIdentifier       db 'PEACHOS '
BytesPerSector      dw 0x200        ; 512
SectorsPerCluster   db 0x80         ; 128
ReservedSectors     dw 200          ;
FATCopies           db 0x02         ; 2
RootDirEntries      dw 0x40         ; 64
NumSectors          dw 0x00         ;
MediaType           db 0xf8         ; Fixed disk (i.e., typically a partition on a hard disk)
SectorsPerFat       dw 0x100        ; 256
SectorsPerTrack     dw 0x20         ; 32
NumberOfHeads       dw 0x40         ; 64
HiddenSectors       dd 0x00
SectorsBig          dd 0x773594     ; 7812500

; Extended BIOS Parameter Block
DriveNumber         db 0x80
WinNTBit            db 0x00
Signature           db 0x29
VolumeID            dd 0xD105
VolumeIDString      db 'PEACHOSBOOT'; MUST be 11 bytes
SystemIDString      db 'FAT16   '   ; MUST be 8 bytes

start:
    jmp 0:step2

step2:
    cli ; clear interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ; enable interrupts


.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

; GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:           ; CS should point to this
    dw 0xffff       ; segment limit first 0-15 bits
    dw 0            ; base address first 0-15 bits
    db 0            ; base address 16-23 bits
    db 0x9a         ; access byte
    db 11001111b    ; high and low 4 bit flags
    db 0            ; base address 24-31 bits

; offset 0x10
gdt_data:           ; DS, SS, ES, FS, GS should point to this
    dw 0xffff       ; segment limit first 0-15 bits
    dw 0            ; base address first 0-15 bits
    db 0            ; base address 16-23 bits
    db 0x92         ; access byte
    db 11001111b    ; high and low 4 bit flags
    db 0            ; base address 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    mov eax, 1
    mov ecx, 100
    mov edi, 0x0100000
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax ; backup the LBA
    ; send highest 8 bits of LBA to drive to hdd controller
    shr eax, 24
    or eax, 0xe0 ; select the master drive
    mov dx, 0x1f6
    out dx, al
    ; end
    ; send total sectors to read
    mov eax, ecx
    mov dx, 0x1f2
    out dx, al
    ; end
    ; send more bits of the LBA
    mov eax, ebx ; restore the LBA
    mov dx, 0x1f3
    out dx, al
    ; end

    mov dx, 0x1f4
    mov eax, ebx ; restore the LBA
    shr eax, 8
    out dx, al
    ; end

    ; Send upper 16 bits of LBA
    mov dx, 0x1f5
    mov eax, ebx ; restore the LBA
    shr eax, 16
    out dx, al
    ; end

    ; send command to read sectors
    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ; read all sectors into memory
.next_sector:
    push ecx

.try_again:
    mov dx, 0x1f7
    in al, dx
    test al, 8
    jz .try_again

    ; Read 256 words at a time
    mov ecx, 256
    mov dx, 0x1f0
    rep insw
    pop ecx
    loop .next_sector
    ; end
    ret

times 510-($-$$) db 0 ; pad to 510 bytes
dw 0xaa55
