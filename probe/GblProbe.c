/* GblProbe.c v3 — paint the VISIBLE buffer first.
   GBL's fastboot menu is drawn into the splash buffer (dtb reserved-memory
   splash_region @ 0xFC800000) — that is the scanout during bootloader phase.
   v2 painted the GOP framebuffer which is NOT scanned out, hence invisible.
   v3: 1) paint 0xFC800000 (red top / white middle / blue bottom),
       2) then paint GOP framebuffer with the same pattern (belt & braces),
       3) stall 30s, 4) return so GBL resumes (fastboot dies per Finished=TRUE,
          user recovers via long power press). */
typedef unsigned long long UINT64;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;
typedef UINT64             UINTN;
typedef void               VOID;
typedef UINTN              EFI_STATUS;
typedef VOID              *EFI_HANDLE;

typedef struct {
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT8  Data4[8];
} EFI_GUID;

typedef struct {
  UINT64 Hdr[3];
  VOID  *ConIn;
  VOID  *ConOut;
  VOID  *StdErr;
  VOID  *RuntimeServices;
  VOID  *BootServices;
} EFI_SYSTEM_TABLE;

typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(EFI_GUID *, VOID *, VOID **);
typedef VOID       (*EFI_STALL)(UINTN);

typedef struct {
  UINT32 Version;
  UINT32 HorizontalResolution;
  UINT32 VerticalResolution;
  UINT32 PixelFormat;
  UINT32 PixelInformation[4];
  UINT32 PixelsPerScanLine;
} GOP_INFO;

typedef struct {
  UINT32    MaxMode;
  UINT32    Mode;
  GOP_INFO *Info;
  UINT32    SizeOfInfo;
  UINT64    FrameBufferBase;
} GOP_MODE;

typedef struct {
  VOID     *QueryMode;
  VOID     *SetMode;
  VOID     *Blt;
  GOP_MODE *Mode;
} GOP_PROTO;

static const EFI_GUID GopGuid = {
  0x9042A9DE, 0x23DC, 0x4A38,
  { 0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A }
};

/* Paint red/white/blue bands at Base with resolution WxH and stride S. */
static void Paint(UINT32 *Base, UINTN W, UINTN H, UINTN S)
{
  UINTN x, y;
  for (y = 0; y < H; y++) {
    UINT32  c;
    UINT32 *row = Base + y * S;
    if (y < H / 2)         c = 0x00FF0000u;      /* red   */
    else if (y < H / 2 + H / 10) c = 0xFFFFFFFFu; /* white stripe */
    else                   c = 0x000000FFu;      /* blue  */
    for (x = 0; x < W; x++) row[x] = c;
  }
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
  (void)ImageHandle;
  EFI_LOCATE_PROTOCOL Locate =
      *(EFI_LOCATE_PROTOCOL *)((UINT8 *)ST->BootServices + 296);
  EFI_STALL Stall = *(EFI_STALL *)((UINT8 *)ST->BootServices + 224);
  GOP_PROTO *Gop = 0;

  /* 1) The visible buffer: splash_region from our dtb. 1440x3168. */
  Paint((UINT32 *)(UINTN)0xFC800000UL, 1440, 3168, 1440);

  /* 2) GOP framebuffer, same pattern. */
  {
    EFI_STATUS st = Locate((EFI_GUID *)&GopGuid, 0, (VOID **)&Gop);
    if (st == 0 && Gop != 0 && Gop->Mode != 0 && Gop->Mode->Info != 0) {
      Paint((UINT32 *)(UINTN)Gop->Mode->FrameBufferBase,
            Gop->Mode->Info->HorizontalResolution,
            Gop->Mode->Info->VerticalResolution,
            Gop->Mode->Info->PixelsPerScanLine);
    }
  }

  Stall(30 * 1000 * 1000);
  return 0;
}
