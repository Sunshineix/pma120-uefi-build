/* BootstrapFdXnu.S 閳?carrier for booting our Mu-Silicium UEFI firmware with the
 * XNUP payload in RAM (for the firmware's Pma120XnuDxe to start the loader).
 *
 * Color markers (final color on screen = how far it got):
 *   ORANGE = wrapper entered
 *   BLUE   = FD copied to 0xC6900000
 *   GREEN  = payload copied to 0xC6C00000
 *   (no color change after ORANGE means the FD copy faulted;
 *    ORANGE->BLUE only means the payload write faulted)
 *
 * Then: clean caches, stop timers, disable GIC, MMU off, jump to the FD.
 */
#define FD_BASE      0xC6900000
#define FD_SIZE      0x00300000
#define PAYLOAD_BASE 0xC7800000   /* the firmware's reserved Kernel region: mapped by both GBL and the firmware */
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
    /* ORANGE: entered */
    ldr   x9,  =SPLASH
    ldr   x10, =0x00FF8000
    ldr   x11, =NPIX
    mov   x12, x10
1:
    stp   x10, x12, [x9], #16
    subs  x11, x11, #2
    b.ne  1b

    /* copy FD to FD_BASE */
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
    dsb   sy

    /* BLUE: FD copied */
    ldr   x9,  =SPLASH
    ldr   x10, =0x000000FF
    ldr   x11, =NPIX
    mov   x12, x10
3:
    stp   x10, x12, [x9], #16
    subs  x11, x11, #2
    b.ne  3b

    /* copy the XNUP payload to PAYLOAD_BASE */
    adrp  x0, PayloadImage
    add   x0, x0, :lo12:PayloadImage
    adrp  x1, PayloadEnd
    add   x1, x1, :lo12:PayloadEnd
    ldr   x2, =PAYLOAD_BASE
    mov   x4, x0
4:
    ldp   x5, x6, [x4], #16
    stp   x5, x6, [x2], #16
    cmp   x4, x1
    b.lo  4b
    dsb   sy

    /* GREEN: payload copied */
    ldr   x9,  =SPLASH
    ldr   x10, =0x0000FF00
    ldr   x11, =NPIX
    mov   x12, x10
5:
    stp   x10, x12, [x9], #16
    subs  x11, x11, #2
    b.ne  5b

    /* clean FD range to PoC */
    ldr   x9,  =FD_BASE
    ldr   x10, =FD_SIZE
    add   x10, x9, x10
    mrs   x11, ctr_el0
    ubfx  x11, x11, #16, #4
    mov   x12, #4
    lsl   x11, x12, x11
    sub   x12, x11, #1
    bic   x8,  x9,  x12
6:
    dc    cvac, x8
    add   x8,  x8,  x11
    cmp   x8,  x10
    b.lo  6b

    /* clean the PAYLOAD DESTINATION range to PoC */
    ldr   x9,  =PAYLOAD_BASE
    adrp  x10, PayloadEnd
    add   x10, x10, :lo12:PayloadEnd
    adrp  x11, PayloadImage
    add   x11, x11, :lo12:PayloadImage
    sub   x10, x10, x11          /* payload length */
    add   x10, x9, x10
    mrs   x11, ctr_el0
    ubfx  x11, x11, #16, #4
    mov   x12, #4
    lsl   x11, x12, x11
    sub   x12, x11, #1
    bic   x8,  x9,  x12
7:
    dc    cvac, x8
    add   x8,  x8,  x11
    cmp   x8,  x10
    b.lo  7b

    dsb   sy
    ic    iallu
    dsb   sy
    isb

    /* mask IRQs, stop timers, disable GIC */
    msr   daifset, #0xf
    msr   cntp_ctl_el0, xzr
    msr   cntv_ctl_el0, xzr
    isb
    ldr   x9, =0x17000000
    str   wzr, [x9]
    dsb   sy

    /* MMU + caches off, jump to the FD */
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
