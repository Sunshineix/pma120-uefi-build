/* GblProbe.c — minimal EFI PE probe for gbl-chainload's `oem boot-efi` channel.
   Paints the screen via the UEFI Graphics Output Protocol (GBL's own framebuffer
   mapping) so we don't guess addresses: top half red, bottom half blue.
   Covers both BGRx/RGBx byte orders: one band is red, the other blue either way.
   Stalls 60s (bands visible), then returns so GBL's fastboot UI comes back.

   Build: gcc-aarch64-linux-gnu + gnu-efi (see .github/workflows/gbl-probe.yml) */
#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = NULL;

  InitializeLib (ImageHandle, SystemTable);

  Print (L"GBLPROBE: LoadImage/StartImage ran OK\r\n");

  Status = uefi_call_wrapper (SystemTable->BootServices->LocateProtocol, 3,
                              &gEfiGraphicsOutputProtocolGuid, NULL,
                              (VOID **)&Gop);
  Print (L"GBLPROBE: LocateProtocol(GOP) = %r\r\n", Status);

  if (!EFI_ERROR (Status) && Gop != NULL && Gop->Mode != NULL &&
      Gop->Mode->Info != NULL) {
    UINTN  W = Gop->Mode->Info->HorizontalResolution;
    UINTN  H = Gop->Mode->Info->VerticalResolution;
    UINTN  Stride = Gop->Mode->Info->PixelsPerScanLine;
    UINT32 *Fb = (UINT32 *)(UINTN)Gop->Mode->FrameBufferBase;
    UINTN  x, y;

    Print (L"GBLPROBE: %dx%d stride=%d base=0x%lx\r\n",
           W, H, Stride, (UINT64)Gop->Mode->FrameBufferBase);

    for (y = 0; y < H; y++) {
      UINT32 Color = (y < H / 2) ? 0x00FF0000u : 0x000000FFu;
      for (x = 0; x < W; x++) {
        Fb[y * Stride + x] = Color;
      }
    }
    Print (L"GBLPROBE: painted red/blue bands, stalling 60s\r\n");
  } else {
    /* Fallback: fixed splash address, alternating red/blue words. */
    volatile UINT32 *Fb2 = (volatile UINT32 *)(UINTN)0xFC800000UL;
    UINTN i;
    for (i = 0; i < 4561920u; i++) {
      Fb2[i] = (i & 1) ? 0x00FF0000u : 0x000000FFu;
    }
    Print (L"GBLPROBE: GOP missing (%r); painted fixed-addr fallback, stalling\r\n",
           Status);
  }

  /* Keep the bands on screen for observation, then return to fastboot UI. */
  uefi_call_wrapper (SystemTable->BootServices->Stall, 1, 60 * 1000 * 1000);
  Print (L"GBLPROBE: returning\r\n");
  return EFI_SUCCESS;
}
