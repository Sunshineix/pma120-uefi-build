/** @file Pma120Diag.c
 *  pma120 diagnostic DXE driver.
 *  Paints the GOP framebuffer with red / white / blue bands and prints a
 *  banner so on-device bring-up progress is visible on the panel.
 *  Depex: gEfiGraphicsOutputProtocolGuid (loads after the display stack).
 *  Copyright (c) 2026, X9U iOS Port Project. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Protocol/GraphicsOutput.h>

EFI_STATUS
EFIAPI
Pma120DiagEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                         Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL       *Gop = NULL;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINT32                             *Fb;
  UINTN                              W, H, S, x, y;

  Status = gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL,
                                (VOID **)&Gop);
  if (EFI_ERROR (Status) || Gop == NULL || Gop->Mode == NULL ||
      Gop->Mode->Info == NULL) {
    DEBUG ((DEBUG_ERROR, "Pma120Diag: GOP unavailable (%r)\n", Status));
    return Status;
  }

  Info = Gop->Mode->Info;
  W    = Info->HorizontalResolution;
  H    = Info->VerticalResolution;
  S    = Info->PixelsPerScanLine;
  Fb   = (UINT32 *)(UINTN)Gop->Mode->FrameBufferBase;

  for (y = 0; y < H; y++) {
    UINT32 Color;
    if (y < H / 2) {
      Color = 0x00FF0000;                 /* red   */
    } else if (y < (H / 2) + (H / 10)) {
      Color = 0xFFFFFFFF;                 /* white stripe */
    } else {
      Color = 0x000000FF;                 /* blue  */
    }
    for (x = 0; x < W; x++) {
      Fb[y * S + x] = Color;
    }
  }

  Print (L"PMA120 DXE ALIVE: GOP %dx%d stride=%d base=0x%lx\n",
         W, H, S, Gop->Mode->FrameBufferBase);
  DEBUG ((DEBUG_INFO, "Pma120Diag: painted %dx%d\n", W, H));

  return EFI_SUCCESS;
}
