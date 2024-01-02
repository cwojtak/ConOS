; BEGIN STAGE 1

[org 0x7c00]
STAGE2_OFFSET equ 0x7e00
STAGE3_OFFSET equ 0x8100
KERNEL_OFFSET equ 0x1000

; Entry
[bits 16]

jmp main
nop

; BIOS Parameter Block

bpbOEM:                 db "ConOS   "
bpbBytesPerSector:      dw 512
bpbSectorsPerCluster:   db 1
bpbReservedSectors:     dw 3
bpbNumberOfFATs:        db 1
bpbRootEntries:         dw 224
bpbTotalSectors:        dw 4086
bpbMedia:               db 0xf0
bpbSectorsPerFAT:       dw 0x0c ; could be 0x09?
bpbSectorsPerTrack:     dw 18
bpbHeadsPerCylinder:    dw 2
bpbHiddenSectors:       dd 0
bpbTotalSectorsBig:     dd 0
bsDriveNumber:          db 0
bsUnused:               db 0
bsExtBootSignature:     db 0x29
bsSerialNumber:         dd 0xa0a1a2a3
bsVolumeLabel:          db "ConOS Disk "
bsFileSystem:           db "FAT12   "

main:
    mov [BOOT_DRIVE], dl
    mov bp, 0x9c00
    mov sp, bp
    xor ax, ax
    mov  dl, [BOOT_DRIVE]

    ; Load stages 2 and 3 into memory at 0x7e00 and 0x8100

    mov bx, STAGE2_OFFSET
    mov dh, 2
    mov cl, 2
    mov es, ax

    call disk_load

    call load_gdt

    call stage2

    call stage3

    call pm_switch

    cli
    hlt

[bits 32]
PM:
	mov ebx, MSG_PROT_MODE
	call print_string_pm

	mov eax, 0x2BADB002
	mov ebx, 0
	mov word [boot_info+multiboot_info.mmap_addr], 0x9c08
	mov word dx, [0x9c04]
	mov word [boot_info+multiboot_info.mmap_length], dx

	push dword boot_info
	call KERNEL_OFFSET
	jmp $

MSG_PROT_MODE db "Successfully switched to 32-bit mode!"
dd 0x0

%include "boot/stage1_disk.asm"
%include "boot/stage1_32bit.asm"
%include "boot/stage1_mem.asm"

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55

; END STAGE 1
; BEGIN STAGE 2

stage2:
    call stage2_mem
    ret

%include "boot/stage2_multiboot.asm"
%include "boot/stage2_mem.asm"

times 1024-($-$$) db 0

; END STAGE 2
; BEGIN STAGE 3

FAT_SEG equ 0x830
FAT_OFFSET equ 0x8300
ROOT_DIRECTORY_SEG equ 0x100
ROOT_DIRECTORY_OFFSET equ 0x1000

DATA_PACKET:
            db 0x10
            db 0x0
blockcount: dw 0x1
db_add:     dw 0x0
            dw 0x0
d_lba:      dd 0x1
            dd 0x0

datasector  dw 0x0000
cluster     dw 0x0000

absoluteSector db 0x00
absoluteHead   db 0x00
absoluteTrack  db 0x00

KERN_IMAGE_NAME db "KERNEL  BIN"
KERN_IMAGE_SIZE dw 0x0

[bits 16]

stage3:
    call load_root

    mov ebx, 0
    mov ebp, KERNEL_OFFSET
    mov esi, KERN_IMAGE_NAME
    call load_kernel
    mov dword [KERN_IMAGE_SIZE], ecx
    cmp ax, 0
    jne stage3_error
    ret
stage3_error:
    cli
    hlt

cluster_lba:
    sub ax, 0x0002
    xor cx, cx
    mov cl, BYTE [bpbSectorsPerCluster]
    mul cx

read_disk_lba:
    push ax
    push bx
    push cx

    mov si, DATA_PACKET
    mov ah, 0x42
    mov dl, [BOOT_DRIVE]
    int 0x13

    jnc read_disk_lba_success
    pop cx
    pop bx
    pop ax
    ;int 0x18
    ret
read_disk_lba_success:
    pop cx
    pop bx
    pop ax
    ret
    
load_root:
    pusha
    xor cx, cx
    xor dx, dx
    mov ax, 32
    mul WORD [bpbRootEntries]
    div WORD [bpbBytesPerSector]
    xchg ax, cx

    mov al, byte [bpbNumberOfFATs]
    mul word [bpbSectorsPerFAT]
    add ax, word [bpbReservedSectors]
    mov word [blockcount], cx
    mov word [db_add], ROOT_DIRECTORY_OFFSET
    mov word [d_lba], ax
    mov word [datasector], ax
    add word [datasector], cx

    mov bx, ROOT_DIRECTORY_OFFSET
    call read_disk_lba
    popa
    ret

load_fat:
    pusha
    push es
    xor ax, ax
    mov al, BYTE [bpbNumberOfFATs]
    mul word [bpbSectorsPerFAT]
    mov cx, ax
    mov word [blockcount], cx
    mov word [db_add], FAT_OFFSET
    mov ax, word [bpbReservedSectors]
    mov word [d_lba], ax
    call read_disk_lba
    pop es
    popa
    ret

find_file:
    push cx
    push dx
    push bx
    mov bx, si
    mov cx, word [bpbRootEntries]
    mov di, ROOT_DIRECTORY_OFFSET
    cld
find_file_loop:
    push cx
    mov cx, 11
    mov si, bx
    push di
    rep cmpsb
    pop di
    je find_file_found
    pop cx
    add di, 32
    loop find_file_loop
find_file_not_found:
    pop bx
    pop dx
    pop cx
    mov ax, -1
    ret
find_file_found:
    pop ax
    pop bx
    pop dx
    pop cx
    ret

load_kernel:
    xor ecx, ecx
    push ecx
load_kernel_find:
    push bx
    push bp
    
    call find_file
    
    cmp ax, -1
    jne load_kernel_fat
    pop bp
    pop bx
    pop ecx
    mov ax, -1
    cli
    hlt
load_kernel_fat:
    sub edi, ROOT_DIRECTORY_OFFSET
    sub eax, ROOT_DIRECTORY_OFFSET
     
    push word ROOT_DIRECTORY_SEG
    pop es
    mov dx, word [es:di + 0x001A]
    mov word [cluster], dx

    pop bx
    pop es
    push bx
    push es
    
    call load_fat
load_kernel_image:
    mov ax, WORD [cluster]
    pop es
    pop bx

    call cluster_lba

    xor cx, cx
    mov cl, [bpbSectorsPerCluster]
    mov word [blockcount], cx
    mov word [db_add], bx
    add ax, word [datasector]
    mov [d_lba], ax

    push dx
    mov dx, ax
    call print_hex
    pop dx

    call read_disk_lba

    mov ax, word [bpbSectorsPerCluster]
    mov cx, word [bpbBytesPerSector]
    mul cx
    add bx, ax

    pop ecx
    inc ecx
    cmp ecx, 54
    jge load_kernel_done
    push ecx
    
    push bx
    push es
    
    mov ax, FAT_SEG
    mov es, ax
    xor bx, bx

    mov ax, WORD [cluster]
    mov cx, ax
    mov dx, ax
    shr dx, 0x0001
    add cx, dx
    
    mov bx, 0
    add bx, cx
    mov dx, WORD [es:bx]
    test ax, 0x0001
    jnz load_kernel_odd_cluster
load_kernel_even_cluster:
    and dx, 0000111111111111b
    jmp load_kernel_check_done
load_kernel_odd_cluster:
    shr dx, 0x0004
load_kernel_check_done:
    mov WORD [cluster], dx
    cmp dx, 0x0FF0
    jb load_kernel_image
load_kernel_done:
    pop es
    pop bx
    pop ecx
    xor ax, ax
    ret

; receiving the data in 'dx'
; For the examples we'll assume that we're called with dx=0x1234
print_hex:
    pusha

    mov cx, 0 ; our index variable

; Strategy: get the last char of 'dx', then convert to ASCII
; Numeric ASCII values: '0' (ASCII 0x30) to '9' (0x39), so just add 0x30 to byte N.
; For alphabetic characters A-F: 'A' (ASCII 0x41) to 'F' (0x46) we'll add 0x40
; Then, move the ASCII byte to the correct position on the resulting string
hex_loop:
    cmp cx, 4 ; loop 4 times
    je end
    
    ; 1. convert last char of 'dx' to ascii
    mov ax, dx ; we will use 'ax' as our working register
    and ax, 0x000f ; 0x1234 -> 0x0004 by masking first three to zeros
    add al, 0x30 ; add 0x30 to N to convert it to ASCII "N"
    cmp al, 0x39 ; if > 9, add extra 8 to represent 'A' to 'F'
    jle step2
    add al, 7 ; 'A' is ASCII 65 instead of 58, so 65-58=7

step2:
    ; 2. get the correct position of the string to place our ASCII char
    ; bx <- base address + string length - index of char
    mov bx, HEX_OUT + 5 ; base + length
    sub bx, cx  ; our index variable
    mov [bx], al ; copy the ASCII char on 'al' to the position pointed by 'bx'
    ror dx, 4 ; 0x1234 -> 0x4123 -> 0x3412 -> 0x2341 -> 0x1234

    ; increment index and loop
    add cx, 1
    jmp hex_loop

end:
    ; prepare the parameter and call the function
    ; remember that print receives parameters in 'bx'
    mov bx, HEX_OUT
    call print

    popa
    ret

HEX_OUT:
    db '0x0000',0 ; reserve memory for our new string

print:
    pusha

; keep this in mind:
; while (string[i] != 0) { print string[i]; i++ }

; the comparison for string end (null byte)
start:
    mov al, [bx] ; 'bx' is the base address for the string
    cmp al, 0 
    je done

    ; the part where we print with the BIOS help
    mov ah, 0x0e
    int 0x10 ; 'al' already contains the char

    ; increment pointer and do next loop
    add bx, 1
    jmp start

done:
    popa
    ret

times 1536-($-$$) db 0

; END STAGE 3
