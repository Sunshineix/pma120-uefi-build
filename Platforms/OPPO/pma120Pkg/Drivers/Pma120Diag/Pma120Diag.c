/** @file Pma120Diag.c
 *  pma120 diagnostic DXE driver v2.
 *  - Depex TRUE: runs at the very start of DXE dispatch (no GOP dependency).
 *  - Paints the splash framebuffer (0xFC800000, the panel's scanout) DIRECTLY,
 *    bypassing the whole display stack.
 *  - Installs a periodic 2s timer that repaints with a rotating pattern
 *    (red -> blue -> tricolor), producing a visible animation while the
 *    firmware is alive. If the animation stops or never starts, we know
 *    exactly where the firmware died.
 *  Copyright (c) 2026, X9U iOS Port Project. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/CacheMaintenanceLib.h>

#define SPLASH_BASE    0xFC800000ULL
#define SCREEN_W       1440
#define SCREEN_H       3168

STATIC UINTN  mPattern = 0;

STATIC
VOID
PaintSplash (
  IN UINT32 Pattern
  )
{
  UINT32 *Fb = (UINT32 *)(UINTN)SPLASH_BASE;
  UINTN   x, y;

  for (y = 0; y < SCREEN_H; y++) {
    UINT32 Color;
    if (Pattern == 0) {
      Color = 0x00FF0000;                       /* solid red  */
    } else if (Pattern == 1) {
      Color = 0x000000FF;                       /* solid blue */
    } else {
      if (y < SCREEN_H / 2) {
        Color = 0x00FF0000;                     /* top red    */
      } else if (y < (SCREEN_H / 2) + (SCREEN_H / 10)) {
        Color = 0xFFFFFFFF;                     /* white band */
      } else {
        Color = 0x000000FF;                     /* bottom blue*/
      }
    }
    for (x = 0; x < SCREEN_W; x++) {
      Fb[y * SCREEN_W + x] = Color;
    }
  }

  /* CRITICAL: flush so the display controller actually sees the writes.
     Without this the paint sits in the CPU cache and never reaches DRAM. */
  WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)SPLASH_BASE,
                                     SCREEN_H * SCREEN_W * 4);
}

STATIC
VOID
EFIAPI
PaintTick (
  IN EFI_EVENT  Event,
  IN VOID      *Context
  )
{
  PaintSplash ((UINT32)mPattern);
  mPattern = (mPattern + 1) % 3;
  DEBUG ((DEBUG_INFO, "Pma120Diag: heartbeat pattern=%u\n", mPattern));
}

EFI_STATUS
EFIAPI
Pma120DiagEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   TimerEvent = NULL;

  DEBUG ((EFI_D_WARN, "Pma120Diag: entry, painting splash\n"));

  /* Paint immediately. */
  PaintSplash (0);

  /* Install 2s periodic repaint timer. */
  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
                             PaintTick, NULL, &TimerEvent);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Pma120Diag: CreateEvent failed %r\n", Status));
    return Status;
  }

  Status = gBS->SetTimer (TimerEvent, TimerPeriodic, 2 * 1000 * 10000);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Pma120Diag: SetTimer failed %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_WARN, "Pma120Diag: heartbeat timer installed\n"));
  return EFI_SUCCESS;
}
