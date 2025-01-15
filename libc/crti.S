.section .init
.global _init
_init:
    push %ebp
    movl %esp, %ebp
    /* gcc puts crtbegin.o's .init section here */

.section .fini
.global _fini
_fini:
    push %ebp
    movl %esp, %ebp
    /* gcc puts crtbegin.o's .fini section here */
