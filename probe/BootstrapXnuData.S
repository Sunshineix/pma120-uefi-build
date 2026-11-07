/* BootstrapXnuData.S — embedded XNUP payload + hakios loader + cache helper.
   Sections .payload and .loader are placed by gblprobe.lds and copied by
   objcopy into the PE. */

    .section .payload,"a"
    .align  12
    .global PayloadImage
PayloadImage:
    .incbin "xnu-payload.bin"
PayloadEnd:
    .size PayloadImage, . - PayloadImage

    .section .loader,"a"
    .align  12
    .global LoaderImage
LoaderImage:
    .incbin "loader.efi"
LoaderEnd:
    .size LoaderImage, . - LoaderImage

    .text
    .align 4
    .global CacheFlushRange
    .type CacheFlushRange,%function
CacheFlushRange:
    /* x0 = address, x1 = length; clean+invalidate by VA to point of coherency */
    mrs   x2, ctr_el0
    ubfx  x2, x2, #16, #4
    mov   x3, #4
    lsl   x2, x3, x2
    sub   x3, x2, #1
    bic   x0, x0, x3
    add   x1, x1, x3
    add   x4, x0, x1
1:
    dc    civac, x0
    add   x0, x0, x2
    cmp   x0, x4
    b.lo  1b
    dsb   sy
    ret
    .size CacheFlushRange, . - CacheFlushRange
