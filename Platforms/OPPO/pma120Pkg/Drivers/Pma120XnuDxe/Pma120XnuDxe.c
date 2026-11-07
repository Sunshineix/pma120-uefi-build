/** @file Pma120XnuDxe.c
 *  pma120 XNU-loader starter.
 *  Finds the XNUP payload at 0xF0000000 (placed there by the GBL wrapper), takes
 *  the "loader.efi" entry out of it, LoadImage+StartImage it. The hakios loader
 *  then builds boot_args and jumps into XNU (or runs its USB command loop).
 *
 *  This mirrors the hook in the reference Silicium UEFI that starts the loader.
 *  Copyright (c) 2026, X9U iOS Port Project. All rights reserved.
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>

#define PAYLOAD_BASE  0xF0000000ULL
#define PAYLOAD_MAGIC 0x50554E58u  /* "XNUP" */

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
  XNUP_HEADER  *Hdr = (XNUP_HEADER *)(UINTN)PAYLOAD_BASE;
  UINT8        *P;
  UINT8        *Pend;
  UINT8        *Loader = NULL;
  UINT32        LoaderSize = 0;
  EFI_HANDLE    LoaderHandle = NULL;
  EFI_STATUS    Status;

  if (Hdr->Magic != PAYLOAD_MAGIC) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: no XNUP payload at 0x%lx (magic 0x%x)\n",
            PAYLOAD_BASE, Hdr->Magic));
    return EFI_NOT_FOUND;
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

  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: starting loader (%u bytes) from 0x%lx\n",
          LoaderSize, (UINT64)(UINTN)Loader));

  Status = gBS->LoadImage (FALSE, ImageHandle, NULL, Loader, LoaderSize, &LoaderHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Pma120XnuDxe: LoadImage failed: %r\n", Status));
    return Status;
  }

  Status = gBS->StartImage (LoaderHandle, NULL, NULL);
  DEBUG ((DEBUG_WARN, "Pma120XnuDxe: loader returned: %r\n", Status));
  return Status;
}
