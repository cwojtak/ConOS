; BEGIN STAGE 1

[org 0x7c00]
STAGE2_OFFSET equ 0x7e00
STAGE3_OFFSET equ 0x8100
KERNEL_OFFSET equ 0x1000
KERNEL_SEG equ 0x100

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

    ; Load the kernel into memory at 0x1000

    mov bx, KERNEL_OFFSET
    mov dh, 54
    mov cl, 4
    mov dl, [BOOT_DRIVE]

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
	;call KERNEL_OFFSET
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

%include "boot/stage2_multiboot.asm"
%include "boot/stage2_mem.asm"

times 1024-($-$$) db 0

; END STAGE 2
; BEGIN STAGE 3

FAT_SEG equ 0x830
ROOT_DIRECTORY_SEG equ 0x850
ROOT_DIRECTORY_OFFSET equ 0x8500

datasector  dw 0x0000
cluster     dw 0x0000

absoluteSector db 0x00
absoluteHead   db 0x00
absoluteTrack  db 0x00

KERN_IMAGE_NAME db "kernel  bin"
KERN_IMAGE_SIZE dw 0x0

[bits 16]

stage3:
    call load_root

    ;mov ebx, 0
    ;mov ebp, KERNEL_SEG
    ;mov esi, KERN_IMAGE_NAME
    ;call load_kernel
    ;mov dword [KERN_IMAGE_SIZE], ecx
    ;cmp ax, 0
    ;jne stage3_error
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

    xor dx, dx
    div WORD [bpbSectorsPerTrack]
    inc dl
    mov BYTE [absoluteSector], dl
    xor dx, dx
    div WORD [bpbHeadsPerCylinder]
    mov BYTE [absoluteHead], dl
    mov BYTE [absoluteTrack], al
    mov ah, 0x02
    mov al, 0x01
    mov ch, BYTE [absoluteTrack]
    mov cl, BYTE [absoluteSector]
    mov dh, BYTE [absoluteHead]
    mov dl, BYTE [BOOT_DRIVE]
    int 0x13

    jnc read_disk_lba_success
    xor ax, ax
    int 0x13
    dec di
    pop cx
    pop bx
    pop ax
    int 0x18
read_disk_lba_success:
    pop cx
    pop bx
    pop ax
    add bx, WORD [bpbBytesPerSector]
    inc ax
    loop read_disk_lba
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
    push word FAT_SEG
    pop es
    xor bx, bx
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
    mov cl, BYTE [bpbSectorsPerCluster]
    
    call read_disk_lba
    
    pop ecx
    inc ecx
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

times 1536-($-$$) db 0

; END STAGE 3
