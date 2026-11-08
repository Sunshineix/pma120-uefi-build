/** @file Pma120XnuDxe.c
 *  pma120 XNU-loader starter.
 *  Finds the XNUP payload at PAYLOAD_BASE (0xC7800000, the firmware's reserved
 *  Kernel region, mapped by both GBL and this firmware; 0xF0000000 is mapped by
 *  neither), takes the "loader.efi" entry out of it, LoadImage+StartImage it.
 *  The hakios loader reads the same payload from 0xC7800000.
 *
 *  Copyright (c) 2026, X9U iOS Port Project. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>

#define PAYLOAD_BASE  0xC7800000ULL
#define PAYLOAD_MAGIC 0x50554E58u  /* "XNUP" */
#define PAYLOAD_MAX   0x04000000u  /* loader.c: PAYLOAD_MAX_SIZE */

/* Splash framebuffer (panel scanout): every paint must flush the cache. */
#define MARK_BASE  0xFC800000ULL
#define MARK_W     1440ULL
#define MARK_H     3168ULL

static
VOID
Paint (
  IN UINT32  Color
  )
{
  volatile UINT32  *Fb = (volatile UINT32 *)(UINTN)MARK_BASE;
  UINTN             Total = MARK_W * MARK_H;
  UINTN             i;

  for (i = 0; i < Total; i++) {
    Fb[i] = Color;
  }
  WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)MARK_BASE, Total * 4);
}

typedef struct {
  UINT32 Magic;
  UINT32 Size;
} XNUP_HEADER;

typedef struct {
  CHAR8  Name[16];
  UINT32 Size;
  UINT32 Reserved;
} XNUP_ENTRY;

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

  Paint (0x00FFFF00u);  /* cyan: XnuDxe entered */

  Hdr = (XNUP_HEADER *)(UINTN)PAYLOAD_BASE;
  if (Hdr->Magic != PAYLOAD_MAGIC) {
    Paint (0x00FF00FFu);  /* magenta: no payload at PAYLOAD_BASE */
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: no XNUP payload at 0x%lx (magic 0x%x)\n",
            PAYLOAD_BASE, Hdr->Magic));
    return EFI_NOT_FOUND;
  }
  if (Hdr->Size > PAYLOAD_MAX) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: payload too big (%u)\n", Hdr->Size));
    return EFI_BAD_BUFFER_SIZE;
  }
  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: payload %u bytes at 0x%lx\n", Hdr->Size, PAYLOAD_BASE));

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
    Paint (0x00FF7F00u);  /* orange: LoadImage failed */
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: LoadImage failed: %r\n", Status));
    return Status;
  }

  Paint (0x000000FFu);  /* blue: LoadImage ok, starting */
  Status = gBS->StartImage (LoaderHandle, NULL, NULL);
  Paint (0x0000FFFFu);  /* yellow: the loader returned */
  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: loader returned: %r\n", Status));
  return Status;
}
