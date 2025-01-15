.section .init
    /* gcc puts crtend.o's init section here */
    popl %ebp
    ret

.section .fini
    /* gcc puts crtend.o's fini section here */
    popl %ebp
    ret
