/* crt0.S — minimal EFI PE entry for aarch64.
   EFI firmware calls the PE entry with x0=ImageHandle, x1=SystemTable.
   We just call efi_main and return its status. */
    .text
    .global _start
    .type _start, %function
_start:
    stp  x29, x30, [sp, #-16]!
    bl   efi_main
    ldp  x29, x30, [sp], #16
    ret
    .size _start, . - _start
