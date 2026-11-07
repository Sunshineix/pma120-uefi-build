/* GblProbe.c v2 — EFI PE probe for gbl-chainload's `oem boot-efi`.
   Design rules learned from v1 (black screen): NO ConOut, NO Print, paint FIRST.
   - Locate GOP via BootServices (LocateProtocol at offset 296, spec-layout).
   - Paint top half red / bottom half blue directly into the GOP framebuffer
     (covers both BGRx/RGBx orders: one band is red, the other blue either way).
   - Stall 60s so the bands stay visible, then RETURN to GBL's fastboot UI.
     If GBL's UI comes back, our PE ran cleanly end-to-end. */
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

/* EFI_SYSTEM_TABLE: 24-byte header, then ConIn(24), ConOut(32), StdErr(40),
   RuntimeServices(48), BootServices(56). */
typedef struct {
  UINT64 Hdr[3];
  VOID  *ConIn;
  VOID  *ConOut;
  VOID  *StdErr;
  VOID  *RuntimeServices;
  VOID  *BootServices;
} EFI_SYSTEM_TABLE;

/* EFI_BOOT_SERVICES: LocateProtocol at offset 296, Stall at offset 224. */
typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(EFI_GUID *, VOID *, VOID **);
typedef VOID       (*EFI_STALL)(UINTN);

/* GOP: QueryMode(0) SetMode(8) Blt(16) Mode(24) */
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

/* 9042A9DE-23DC-4A38-96FB-7ADED080516A */
static const EFI_GUID GopGuid = {
  0x9042A9DE, 0x23DC, 0x4A38,
  { 0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A }
};

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
  (void)ImageHandle;
  EFI_LOCATE_PROTOCOL Locate =
      *(EFI_LOCATE_PROTOCOL *)((UINT8 *)ST->BootServices + 296);
  EFI_STALL Stall = *(EFI_STALL *)((UINT8 *)ST->BootServices + 224);
  GOP_PROTO *Gop = 0;
  EFI_STATUS st = Locate((EFI_GUID *)&GopGuid, 0, (VOID **)&Gop);

  /* Paint FIRST, before anything else can go wrong. */
  if (st == 0 && Gop != 0 && Gop->Mode != 0 && Gop->Mode->Info != 0) {
    UINTN   W = Gop->Mode->Info->HorizontalResolution;
    UINTN   H = Gop->Mode->Info->VerticalResolution;
    UINTN   S = Gop->Mode->Info->PixelsPerScanLine;
    UINT32 *Fb = (UINT32 *)(UINTN)Gop->Mode->FrameBufferBase;
    UINTN   x, y;
    for (y = 0; y < H; y++) {
      UINT32   c = (y < H / 2) ? 0x00FF0000u : 0x000000FFu;
      UINT32  *row = Fb + y * S;
      for (x = 0; x < W; x++) row[x] = c;
    }
  }

  /* Keep bands visible, then return so GBL resumes its fastboot UI. */
  Stall(60 * 1000 * 1000);
  return 0;
}
