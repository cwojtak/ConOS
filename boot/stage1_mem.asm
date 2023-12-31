[bits 16]

struc MemoryMapEntry
	.baseAddress	resq	1	; base address of address range
	.length		resq	1	; length of address range in bytes
	.type		resd	1	; type of address range
	.acpi_null	resd	1	; reserved
endstruc

get_memory_map:
	pushad
	mov di, 0x9c08
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
	test byte [es:di + 20], 1
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
	mov [0x9c04], bp
	clc
	popad
	ret
.error:
	stc
	popad
	ret
