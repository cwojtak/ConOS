[org 0x7c00]
KERNEL_OFFSET equ 0x1000

mov [BOOT_DRIVE], dl
mov bp, 0x9000
mov sp, bp

mov bx, KERNEL_OFFSET ; load_kernel
mov dh, 54
mov dl, [BOOT_DRIVE]

call disk_load

mov ah, 0x0e
mov al, 'S'
int 0x10
mov al, 'T'
int 0x10
mov al, 'A'
int 0x10
mov al, 'R'
int 0x10
mov al, 'T'
int 0x10
mov al, 'I'
int 0x10
mov al, 'N'
int 0x10
mov al, 'G'
int 0x10

call pm_switch
jmp $

%include "boot/32bit.asm"

disk_load:
	pusha
	push dx

	mov ah, 0x02
	mov al, dh
	mov cl, 0x02
	mov ch, 0x00
	mov dh, 0x00
	int 0x13
	jc disk_error
	
	pop dx
	cmp al, dh
	jne disk_error
	popa
	ret

disk_error:
	jmp $

[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_string_pm:
	pusha
	mov edx, VIDEO_MEMORY
	jmp print_string_pm_loop

print_string_pm_loop:
	mov al, [ebx]
	mov ah, WHITE_ON_BLACK
	cmp al, 0
	je print_string_pm_done
	mov [edx], ax
	add ebx, 1
	add edx, 2

	jmp print_string_pm_loop

print_string_pm_done:
	popa
	ret

PM:
	mov ebx, MSG_PROT_MODE
	call print_string_pm
	call KERNEL_OFFSET
	jmp $

MSG_PROT_MODE db "Successfully transfered to 32-bit mode!"

BOOT_DRIVE db 0

times 510-($-$$) db 0

dw 0xaa55

