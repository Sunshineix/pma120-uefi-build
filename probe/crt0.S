/* crt0.S — minimal EFI PE entry for aarch64 (adapted from gnu-efi's crt0).
   EFI firmware calls the PE entry with x0=ImageHandle, x1=SystemTable.
   We call efi_main directly; the image is fully PC-relative (-Bsymbolic build)
   so no self-relocation is needed.
   The hand-crafted dummy .reloc section mirrors gnu-efi's upstream trick so the
   PE carries a valid base-relocation directory. */
    .text
    .align  12
    .globl _start
    .type  _start,%function
_start:
    stp   x29, x30, [sp, #-32]!
    mov   x29, sp
    bl    efi_main
    ldp   x29, x30, [sp], #32
    ret
    .size _start, . - _start

    .data
dummy0: .4byte 0
dummy1: .4byte 0

    .section .reloc, "a", %progbits
    .4byte dummy1 - dummy0
    .4byte 12
    .2byte 0
    .2byte 0
