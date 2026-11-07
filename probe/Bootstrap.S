/* Bootstrap.S — EFI-app wrapper that chainloads our UEFI firmware (FD).
   Executed by GBL's `oem boot-efi` (LoadImage/StartImage), MMU ON, EL1.
   Sequence:
     1. Paint the whole splash buffer red  (marker: the wrapper ran)
     2. Copy the embedded FD to FD_BASE    (0xC6900000, 3 MB)
     3. Clean/invalidate caches
     4. Mask interrupts, disable MMU + caches (EL1), invalidate TLBs
     5. Jump to FD_BASE  (never returns)
   Provides efi_main for crt0.S. All addressing is PC-relative. */

#define FD_BASE  0xC6900000
#define FD_SIZE  0x00300000

    .section .fd,"a"
    .align  12
    .global FdImage
FdImage:
    .incbin "SILICIUM_UEFI.fd"
FdEnd:
    .size FdImage, . - FdImage

    .text
    .align  12
    .global efi_main
    .type   efi_main,%function
efi_main:
    /* ---- 1. Paint splash red: the wrapper executed ---- */
    ldr   x9,  =0xFC800000
    ldr   x10, =0x00FF0000
    ldr   x11, =0x459C00            /* 1440*3168 pixels */
    mov   x12, x10
1:
    stp   x10, x12, [x9], #16
    subs  x11, x11, #2
    b.ne  1b

    /* ---- 2. Copy FD to FD_BASE ---- */
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

    /* ---- 3. Clean the FD range to PoC, invalidate I-cache ---- */
    ldr   x9,  =FD_BASE
    ldr   x10, =FD_SIZE
    add   x10, x9, x10
    mrs   x11, ctr_el0
    ubfx  x11, x11, #16, #4
    mov   x12, #4
    lsl   x11, x12, x11
    sub   x12, x11, #1
    bic   x8,  x9,  x12
3:
    dc    cvac, x8
    add   x8,  x8,  x11
    cmp   x8,  x10
    b.lo  3b
    dsb   sy
    ic    iallu
    dsb   sy
    isb

    /* ---- 4. Mask IRQs, disable MMU + caches at EL1 ---- */
    msr   daifset, #0xf

    mrs   x0, sctlr_el1
    bic   x0, x0, #(1 << 0)         /* M  */
    bic   x0, x0, #(1 << 2)         /* C  */
    bic   x0, x0, #(1 << 12)        /* I  */
    msr   sctlr_el1, x0
    isb
    tlbi  vmalle1
    dsb   sy
    isb

    /* ---- 5. Clean handoff registers and jump ---- */
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
