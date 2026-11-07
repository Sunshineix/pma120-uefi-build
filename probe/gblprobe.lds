/* gblprobe.lds — minimal link script producing a relocatable EFI PE payload.
   Base 0; _start must be at offset 0 (PE AddressOfEntryPoint = 0).
   .rela.dyn entries are converted to PE base relocations by objcopy. */
ENTRY(_start)
SECTIONS
{
  . = 0;
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) *(.sdata*) }
  .got : { *(.got*) }
  .dynamic : { *(.dynamic) }
  .rela : { *(.rela*) }
  .reloc : { *(.reloc) }
  /DISCARD/ : { *(.note*) *(.comment) *(.eh_frame*) *(.ARM.attributes) }
}
