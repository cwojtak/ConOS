[org 0x7c00]
KERNEL_OFFSET equ 0x1000

[bits 16]
mov [BOOT_DRIVE], dl
mov bp, 0x9000
mov sp, bp

mov bx, KERNEL_OFFSET ; load_kernel
mov dh, 54
mov dl, [BOOT_DRIVE]

call disk_load

call load_gdt

call BiosGetMemoryMap

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

[bits 16]

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

struc	MemoryMapEntry
	.baseAddress	resq	1	; base address of address range
	.length		resq	1	; length of address range in bytes
	.type		resd	1	; type of address range
	.acpi_null	resd	1	; reserved
endstruc

BiosGetMemoryMap:
	pushad
	mov di, 0x9004
	xor ebx, ebx
	xor	bp, bp			; number of entries stored here
	mov	edx, 0x0534D4150		; 'SMAP'
	mov	eax, 0xe820
	mov [es:di + 20], dword 1
	mov	ecx, 24			; memory map entry struct is 24 bytes
	int	0x15			; get first entry
	jc	short .error
	mov	edx, 0x0534D4150		; 'SMAP'
	cmp	eax, edx		; bios returns SMAP in eax
	jne	short .error
	test	ebx, ebx		; if ebx=0 then list is one entry long; bail out
	je	short .error
	jmp	short .start
.e8201p:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short .e820f
	mov	edx, 0x0534D4150		; 'SMAP'
.start:
	jcxz	.skip_entry		; if actual returned bytes is 0, skip entry
	cmp cl, 20
	jbe short .notext
	test byte [es:di + 20] , 1
	je short .skip_entry
.notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz .skip_entry
	inc bp
	add di, 24
.skip_entry:
	test ebx, ebx
	jne short .e8201p
.e820f:
	mov [0x9000], bp
	clc
	popad
	ret
.error:
	stc
	popad
	ret

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

