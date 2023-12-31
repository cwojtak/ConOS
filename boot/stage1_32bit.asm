gdt_start:
	dd 0x0
	dd 0x0

gdt_code:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0

gdt_data:
	dw 0xffff
	dw 0x0
	db 0x0
	db 10010010b
	db 11001111b
	db 0x0

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

[bits 16]
load_gdt:
	cli
	pusha
	lgdt [gdt_descriptor]
	sti
	popa
	ret

pm_switch:
        cli 
        mov eax, cr0
        or eax, 0x1
        and eax, 0x1f
        mov cr0, eax
        jmp CODE_SEG:init_pm

[bits 32]
init_pm:
        mov ax, DATA_SEG
        mov ds, ax
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov ebp, 0x90000
		mov esp, ebp

        call PM

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

