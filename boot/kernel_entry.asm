global _start;
[bits 32]

_start:
	[extern kernel_main]
	jmp kernel_main
	jmp $
