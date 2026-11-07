/* BootstrapFdXnu.S — carrier for booting our Mu-Silicium UEFI firmware with the
 * XNUP payload in RAM, so the firmware's Pma120XnuDxe can start the hakios loader
 * from it (a clean EFI environment with a free USB controller, like the
 * reference Silicium UEFI flow).
 *
 * Steps:
 *   1. paint the splash orange
 *   2. copy the embedded FD to FD_BASE (0xC6900000)
 *   3. copy the embedded XNUP payload to PAYLOAD_BASE (0xF0000000)
 *   4. clean caches; stop arch timers; disable the GIC distributor
 *   5. disable MMU + caches, jump to the FD (SEC entry)
 */
#define FD_BASE      0xC6900000
#define FD_SIZE      0x00300000
#define PAYLOAD_BASE 0xF0000000
#define SPLASH       0xFC800000
#define NPIX         0x459C00

    .section .fd,"a"
    .align  12
    .global FdImage
FdImage:
    .incbin "SILICIUM_UEFI.fd"
FdEnd:
    .size FdImage, . - FdImage

    .section .payload,"a"
    .align  12
    .global PayloadImage
PayloadImage:
    .incbin "xnu-payload.bin"
PayloadEnd:
    .size PayloadImage, . - PayloadImage

    .text
    .align  12
    .global efi_main
    .type   efi_main,%function
efi_main:
    /* 1. orange */
    ldr   x9,  =SPLASH
    ldr   x10, =0x00FF8000
    ldr   x11, =NPIX
    mov   x12, x10
1:
    stp   x10, x12, [x9], #16
    subs  x11, x11, #2
    b.ne  1b

    /* 2. copy FD to FD_BASE */
    adrp  x0, FdImage
    add   x0, x0, :lo12:FdImage
    adrp  x1, FdEnd
    add   x1, x1, :lo12:FdEnd
    ldr   x2, =FD_BASE
    mov   x4, x0
2:
    ldp   x5, x6, [x4], #16
    stp   x5, x6, [x2], #16
    cmp   x4, x1
    b.lo  2b

    /* 3. copy the XNUP payload to PAYLOAD_BASE */
    adrp  x0, PayloadImage
    add   x0, x0, :lo12:PayloadImage
    adrp  x1, PayloadEnd
    add   x1, x1, :lo12:PayloadEnd
    ldr   x2, =PAYLOAD_BASE
    mov   x4, x0
3:
    ldp   x5, x6, [x4], #16
    stp   x5, x6, [x2], #16
    cmp   x4, x1
    b.lo  3b

    dsb   sy

    /* 4a. clean the FD range to PoC */
    ldr   x9,  =FD_BASE
    ldr   x10, =FD_SIZE
    add   x10, x9, x10
    mrs   x11, ctr_el0
    ubfx  x11, x11, #16, #4
    mov   x12, #4
    lsl   x11, x12, x11
    sub   x12, x11, #1
    bic   x8,  x9,  x12
4:
    dc    cvac, x8
    add   x8,  x8,  x11
    cmp   x8,  x10
    b.lo  4b

    /* 4b. clean the payload range to PoC (its length is small: 66 KB) */
    adrp  x9, PayloadImage
    add   x9, x9, :lo12:PayloadImage
    adrp  x10, PayloadEnd
    add   x10, x10, :lo12:PayloadEnd
    bic   x8, x9, x12
5:
    dc    cvac, x8
    add   x8,  x8,  x11
    cmp   x8,  x10
    b.lo  5b

    dsb   sy
    ic    iallu
    dsb   sy
    isb

    /* 4c. stop timers + GIC, mask IRQs */
    msr   daifset, #0xf
    msr   cntp_ctl_el0, xzr
    msr   cntv_ctl_el0, xzr
    isb
    ldr   x9, =0x17000000
    str   wzr, [x9]
    dsb   sy

    /* 5. MMU + caches off, jump to the FD */
    mrs   x0, sctlr_el1
    bic   x0, x0, #(1 << 0)
    bic   x0, x0, #(1 << 2)
    bic   x0, x0, #(1 << 12)
    msr   sctlr_el1, x0
    isb
    tlbi  vmalle1
    dsb   sy
    isb

    mov   x0, xzr
    mov   x1, xzr
    mov   x2, xzr
    mov   x3, xzr
    ldr   x5, =FD_BASE
    br    x5

.Ldead:
    b     .Ldead
    .size efi_main, . - efi_main
    .ltorg
