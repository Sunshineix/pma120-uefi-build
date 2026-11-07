/* BootstrapVerify.S 鈥?FD chainloader with copy verification.
   Color report (final color on screen before jump/halt):
     RED    = wrapper entered
     YELLOW = source/code overlaps FD copy target (no copy attempted; halt)
     MAGENTA= copy checksum mismatch (data corruption caught; halt)
     GREEN  = copy verified OK, MMU off, jumping to firmware
   Checksum: 64-bit sum of source FD before copy vs dest after copy.
   Overlap pre-check covers [FdImage, FdImage+0x310000) vs [FD_BASE, FD_BASE+FD_SIZE).
   Provides efi_main for crt0.S. PC-relative addressing only. */

#define FD_BASE   0xC6900000
#define FD_SIZE   0x00300000
#define SPLASH    0xFC800000
#define PAYLOAD_BASE 0xC7800000   /* firmware's reserved Kernel region */
#define NPIX      0x459C00          /* 1440*3168 pixels */
#define SPAN      0x00310000        /* FD + code margin for overlap test */

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
    /* ---- RED: entered ---- */
    ldr   x9,  =SPLASH
    ldr   x10, =0x00FF0000
    bl    FillScreen

    /* ---- overlap pre-check: [src, src+SPAN) vs [FD_BASE, FD_BASE+FD_SIZE) ---- */
    adrp  x0, FdImage
    add   x0, x0, :lo12:FdImage
    ldr   x1, =FD_BASE
    ldr   x2, =FD_SIZE
    ldr   x3, =SPAN
    add   x3, x0, x3
    cmp   x3, x1
    b.le  1f                    /* srcEnd <= destStart: no overlap */
    add   x4, x1, x2
    cmp   x0, x4
    b.ge  1f                    /* src >= destEnd: no overlap */
    /* overlap -> YELLOW halt */
    ldr   x9,  =SPLASH
    ldr   x10, =0x00FFFF00
    bl    FillScreen
2:
    b     2b
1:
    /* ---- checksum of source ---- */
    adrp  x4, FdImage
    add   x4, x4, :lo12:FdImage
    adrp  x1, FdEnd
    add   x1, x1, :lo12:FdEnd
    mov   x6, xzr
    mov   x7, xzr
3:
    ldp   x5, x8, [x4], #16
    adds  x6, x6, x5
    adc   x7, x7, xzr
    adds  x6, x6, x8
    adc   x7, x7, xzr
    cmp   x4, x1
    b.lo  3b

    /* ---- copy FD to FD_BASE (x6/x7 hold src checksum) ---- */
    adrp  x4, FdImage
    add   x4, x4, :lo12:FdImage
    adrp  x1, FdEnd
    add   x1, x1, :lo12:FdEnd
    ldr   x2, =FD_BASE
4:
    ldp   x5, x8, [x4], #16
    stp   x5, x8, [x2], #16
    cmp   x4, x1
    b.lo  4b
    dsb   sy

    /* ---- copy the XNUP payload to PAYLOAD_BASE (after the FD copy) ---- */
    adrp  x4, PayloadImage
    add   x4, x4, :lo12:PayloadImage
    adrp  x1, PayloadEnd
    add   x1, x1, :lo12:PayloadEnd
    ldr   x2, =PAYLOAD_BASE
12:
    ldp   x5, x8, [x4], #16
    stp   x5, x8, [x2], #16
    cmp   x4, x1
    b.lo  12b
    dsb   sy

    /* ---- checksum of destination ---- */
    ldr   x4, =FD_BASE
    ldr   x2, =FD_SIZE
    add   x1, x4, x2
    mov   x9, xzr
    mov   x10, xzr
5:
    ldp   x5, x8, [x4], #16
    adds  x9, x9, x5
    adc   x10, x10, xzr
    adds  x9, x9, x8
    adc   x10, x10, xzr
    cmp   x4, x1
    b.lo  5b

    /* ---- compare ---- */
    cmp   x9, x6
    b.ne  6f
    cmp   x10, x7
    b.ne  6f

    /* ---- GREEN: verified, about to jump ---- */
    ldr   x9,  =SPLASH
    ldr   x10, =0x0000FF00
    bl    FillScreen

    /* clean FD range to PoC, invalidate I-cache */
    ldr   x9,  =FD_BASE
    ldr   x10, =FD_SIZE
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

    /* ---- mask IRQs, stop timer interrupts, disable GIC, then MMU/caches off ---- */
    msr   daifset, #0xf

    /* Stop the arch timers so no periodic IRQ fires into the firmware's
       not-yet-installed vector table (GBL's fastboot timers keep ticking). */
    msr   cntp_ctl_el0, xzr
    msr   cntv_ctl_el0, xzr
    isb

    /* Disable the GIC distributor (SM8850 GICD = 0x17000000, per KaanapaliPkg).
       All interrupt delivery stops until the firmware's ArmGicDxe re-inits. */
    ldr   x9, =0x17000000
    str   wzr, [x9]
    dsb   sy

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

    /* ---- MAGENTA: checksum mismatch ---- */
6:
    ldr   x9,  =SPLASH
    ldr   x10, =0x00FF00FF
    bl    FillScreen
8:
    b     8b

/* FillScreen: x9 = base, x10 = color (clobbers x9-x12) */
FillScreen:
    ldr   x11, =NPIX
    mov   x12, x10
9:
    stp   x10, x12, [x9], #16
    subs  x11, x11, #2
    b.ne  9b
    ret

    .size efi_main, . - efi_main
    .ltorg
