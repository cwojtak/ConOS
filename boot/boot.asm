[org 0x7c00]
STAGE2_OFFSET equ 0x7e00
KERNEL_OFFSET equ 0x1000

[bits 16]
mov [BOOT_DRIVE], dl
mov bp, 0x9000
mov sp, bp

mov bx, STAGE2_OFFSET
mov dh, 1
mov cl, 2
mov dl, [BOOT_DRIVE]

call disk_load

mov bx, KERNEL_OFFSET
mov dh, 54
mov cl, 3
mov dl, [BOOT_DRIVE]

call disk_load

call load_gdt
call get_memory_map
call get_memory_size_legacy
mov [0x9000], ax
call enable_a20

call pm_switch
jmp $

%include "boot/disk.asm"
%include "boot/print.asm"
%include "boot/multiboot.asm"
%include "boot/32bit.asm"
%include "boot/mem.asm"

[bits 32]

PM:
	mov ebx, MSG_PROT_MODE
	call print_string_pm

	mov eax, 0x2BADB002
	mov ebx, 0
	mov word [boot_info+multiboot_info.mmap_addr], 0x9008
	mov word dx, [0x9004]
	mov word [boot_info+multiboot_info.mmap_length], dx

	push dword boot_info

	call KERNEL_OFFSET
	jmp $

MSG_PROT_MODE db "Successfully switched to 32-bit mode!"

BOOT_DRIVE db 0

times 510-($-$$) db 0

dw 0xaa55

%include "boot/mem_stage2.asm"

times 1024-($-$$) db 0
