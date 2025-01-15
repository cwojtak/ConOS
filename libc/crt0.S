.section .text

.global _start
_start:
    /* Set up the end of the stack frame */
    movl $0, %ebp
    pushl %ebp
    pushl %ebp
    movl %esp, %ebp

    /* Save arguments for main */
    pushl %esi
    pushl %edi

    /* Set up C runtime */
    /* call initialize_standard_library */

    /* Call the global constructors */
    call _init

    /* Restore arguments for main */
    popl %edi
    popl %esi
    
    /* Run main */
    call main

    /* Terminate the process */
    movl %eax, %edi
    call _fini //call exit
.size _start, . - _start
