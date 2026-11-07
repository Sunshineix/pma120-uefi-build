/** @file Pma120XnuDxe.c
 *  pma120 XNU-loader starter.
 *  Installs a minimal GOP on the known panel framebuffer (SimpleFbDxe crashes
 *  on this hardware), then starts the hakios loader out of the XNUP payload.
 *
 *  Copyright (c) 2026, X9U iOS Port Project. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/GraphicsOutput.h>

#define PAYLOAD_BASE  0xC7800000ULL
#define PAYLOAD_MAGIC 0x50554E58u  /* "XNUP" */
#define PAYLOAD_MAX   0x04000000u  /* loader.c: PAYLOAD_MAX_SIZE */

#define FB_BASE       0xFC800000ULL
#define FB_WIDTH      1440
#define FB_HEIGHT     3168
#define FB_STRIDE     1440

typedef struct {
  UINT32 Magic;
  UINT32 Size;
} XNUP_HEADER;

typedef struct {
  CHAR8  Name[16];
  UINT32 Size;
  UINT32 Reserved;
} XNUP_ENTRY;

static EFI_GRAPHICS_OUTPUT_PROTOCOL        mGop;
static EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE   mGopMode;
static EFI_GRAPHICS_OUTPUT_MODE_INFORMATION mGopInfo;

static EFI_STATUS EFIAPI
GopQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                 ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
{
  if (ModeNumber != 0) {
    return EFI_UNSUPPORTED;
  }
  *Info = &mGopInfo;
  *SizeOfInfo = sizeof (mGopInfo);
  return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI
GopSetMode (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL  *This,
  IN UINT32                         ModeNumber
  )
{
  return (ModeNumber == 0) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

static EFI_STATUS EFIAPI
GopBlt (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL       *This,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL      *BltBuffer,
  IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
  IN UINTN                               SourceX,
  IN UINTN                               SourceY,
  IN UINTN                               DestinationX,
  IN UINTN                               DestinationY,
  IN UINTN                               Width,
  IN UINTN                               Height,
  IN UINTN                               Delta
  )
{
  return EFI_UNSUPPORTED;
}

static void
InstallGop (void)
{
  EFI_HANDLE  Handle = NULL;

  ZeroMem (&mGopInfo, sizeof (mGopInfo));
  mGopInfo.Version              = 0;
  mGopInfo.HorizontalResolution = FB_WIDTH;
  mGopInfo.VerticalResolution   = FB_HEIGHT;
  mGopInfo.PixelFormat          = PixelBlueGreenRedReserved8BitPerColor;
  mGopInfo.PixelsPerScanLine    = FB_STRIDE;

  ZeroMem (&mGopMode, sizeof (mGopMode));
  mGopMode.MaxMode         = 1;
  mGopMode.Mode            = 0;
  mGopMode.Info            = &mGopInfo;
  mGopMode.SizeOfInfo      = sizeof (mGopInfo);
  mGopMode.FrameBufferBase = (EFI_PHYSICAL_ADDRESS)FB_BASE;
  mGopMode.FrameBufferSize = (UINTN)FB_WIDTH * FB_HEIGHT * 4;

  mGop.QueryMode = GopQueryMode;
  mGop.SetMode   = GopSetMode;
  mGop.Blt       = GopBlt;
  mGop.Mode      = &mGopMode;

  gBS->InstallMultipleProtocolInterfaces (
         &Handle,
         &gEfiGraphicsOutputProtocolGuid, &mGop,
         NULL
         );
}

static BOOLEAN
StrEq16 (
  IN CONST CHAR8  *A,
  IN CONST CHAR8  *B
  )
{
  UINTN  i;
  for (i = 0; i < 16; i++) {
    if (A[i] != B[i]) {
      return FALSE;
    }
    if (A[i] == 0) {
      return TRUE;
    }
  }
  return TRUE;
}

EFI_STATUS
EFIAPI
Pma120XnuDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  XNUP_HEADER  *Hdr;
  UINT8        *P;
  UINT8        *Pend;
  UINT8        *Loader = NULL;
  UINT32        LoaderSize = 0;
  EFI_HANDLE    LoaderHandle = NULL;
  EFI_STATUS    Status;

  InstallGop ();
  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: GOP installed at 0x%lx %dx%d\n", FB_BASE, FB_WIDTH, FB_HEIGHT));

  Hdr = (XNUP_HEADER *)(UINTN)PAYLOAD_BASE;
  if (Hdr->Magic != PAYLOAD_MAGIC) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: no XNUP payload at 0x%lx (magic 0x%x)\n",
            PAYLOAD_BASE, Hdr->Magic));
    return EFI_NOT_FOUND;
  }
  if (Hdr->Size > PAYLOAD_MAX) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: payload too big (%u)\n", Hdr->Size));
    return EFI_BAD_BUFFER_SIZE;
  }

  P    = (UINT8 *)(Hdr + 1);
  Pend = P + Hdr->Size;

  while (P + sizeof (XNUP_ENTRY) <= Pend) {
    XNUP_ENTRY  *E = (XNUP_ENTRY *)P;
    UINT8       *Data;
    if (E->Name[0] == 0) {
      break;
    }
    Data = P + sizeof (XNUP_ENTRY);
    if (StrEq16 (E->Name, "loader.efi")) {
      Loader     = Data;
      LoaderSize = E->Size;
      break;
    }
    P += sizeof (XNUP_ENTRY) + ((E->Size + 15) & ~15u);
  }

  if (Loader == NULL || LoaderSize == 0) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: no loader.efi entry in the payload\n"));
    return EFI_NOT_FOUND;
  }

  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: starting loader (%u bytes)\n", LoaderSize));

  Status = gBS->LoadImage (FALSE, ImageHandle, NULL, Loader, LoaderSize, &LoaderHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: LoadImage failed: %r\n", Status));
    return Status;
  }

  Status = gBS->StartImage (LoaderHandle, NULL, NULL);
  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: loader returned: %r\n", Status));
  return Status;
}
