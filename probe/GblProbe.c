/* GblProbe.c — freestanding EFI PE probe for gbl-chainload's `oem boot-efi`.
   NO gnu-efi headers needed: all UEFI types are hand-declared from the spec
   (stable layouts). Paints the screen via GOP (GBL's own framebuffer mapping):
   top half red, bottom half blue — works for both BGRx/RGBx orders.
   Then spins forever so the bands stay visible; user reboots with power key.
   Link envelope: gnu-efi's crt0-efi-aarch64.o + elf_aarch64_efi.lds (no libs). */

typedef unsigned long long UINT64;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;
typedef UINT64             UINTN;
typedef void               VOID;

typedef struct {
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT8  Data4[8];
} EFI_GUID;

typedef UINTN EFI_STATUS;
typedef VOID *EFI_HANDLE;

/* ---- EFI_SYSTEM_TABLE (spec 2.10) ---- */
typedef struct {
  UINT64 Hdr[3];           /* 24-byte EFI_TABLE_HEADER (opaque here) */
  VOID  *ConIn;            /* offset 24 */
  VOID  *ConOut;           /* offset 32 */
  VOID  *StdErr;           /* offset 40 */
  VOID  *RuntimeServices;  /* offset 48 */
  VOID  *BootServices;     /* offset 56 */
} EFI_SYSTEM_TABLE;

/* ---- EFI_BOOT_SERVICES: LocateProtocol at offset 296 (spec order) ---- */
typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(EFI_GUID *, VOID *, VOID **);

/* ---- GOP protocol (spec 12.9) ---- */
typedef struct {
  UINT32 Version;
  UINT32 HorizontalResolution;   /* offset 4  */
  UINT32 VerticalResolution;     /* offset 8  */
  UINT32 PixelFormat;            /* offset 12 */
  UINT32 PixelInformation[4];    /* offset 16 */
  UINT32 PixelsPerScanLine;      /* offset 32 */
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
  UINT32 MaxMode;                          /* 0 */
  UINT32 Mode;                             /* 4 */
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info; /* 8 */
  UINT32 SizeOfInfo;                       /* 16 */
  UINT64 FrameBufferBase;                  /* 24 */
} EFI_GRAPHICS_OUTPUT_MODE;

typedef struct {
  VOID *QueryMode;      /* 0  */
  VOID *SetMode;        /* 8  */
  VOID *Blt;            /* 16 */
  EFI_GRAPHICS_OUTPUT_MODE *Mode; /* 24 */
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

/* GOP GUID 9042A9DE-23DC-4A38-96FB-7ADED080516A */
static const EFI_GUID gEfiGopGuid = {
  0x9042A9DE, 0x23DC, 0x4A38,
  { 0x96, 0xFB, 0x7A, 0xDE, 0xD0, 0x80, 0x51, 0x6A }
};

/* ConOut print via SIMPLE_TEXT_OUTPUT.OutputString (offset 8). */
typedef UINTN (*EFI_TEXT_STRING)(VOID *, const UINT16 *);
static void puts16(VOID *ConOut, const UINT16 *s)
{
  EFI_TEXT_STRING OutputString = *(EFI_TEXT_STRING *)((UINT8 *)ConOut + 8);
  if (OutputString) OutputString(ConOut, s);
}
static void puts_ascii(VOID *ConOut, const char *s)
{
  UINT16 buf[128]; UINTN i = 0;
  while (*s && i < 126) { buf[i++] = (UINT16)*s++; }
  buf[i] = 0;
  puts16(ConOut, buf);
}

static UINTN hex(UINT64 v, char *out)
{
  static const char d[] = "0123456789ABCDEF";
  char tmp[20]; UINTN n = 0;
  if (v == 0) { tmp[n++] = '0'; }
  while (v && n < 16) { tmp[n++] = d[v & 15]; v >>= 4; }
  for (UINTN i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
  return n;
}

/* libc stubs in case crt0 needs them */
void *memset(void *d, int c, UINTN n) { UINT8 *p = d; while (n--) *p++ = (UINT8)c; return d; }
void *memcpy(void *d, const void *s, UINTN n) { UINT8 *p = d; const UINT8 *q = s; while (n--) *p++ = *q++; return d; }
void *memmove(void *d, const void *s, UINTN n) { return memcpy(d, s, n); }

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
  (void)ImageHandle;
  VOID *ConOut = ST->ConOut;
  puts_ascii(ConOut, "GBLPROBE: LoadImage/StartImage OK\r\n");

  EFI_LOCATE_PROTOCOL LocateProtocol =
      *(EFI_LOCATE_PROTOCOL *)((UINT8 *)ST->BootServices + 296);
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop = 0;
  EFI_STATUS st = LocateProtocol((EFI_GUID *)&gEfiGopGuid, 0, (VOID **)&Gop);

  char line[64]; UINTN n;
  puts_ascii(ConOut, "GBLPROBE: GOP status=0x");
  n = hex((UINT64)st, line); line[n] = '\r'; line[n+1] = '\n'; line[n+2] = 0;
  puts_ascii(ConOut, line);

  if (st == 0 && Gop && Gop->Mode && Gop->Mode->Info) {
    UINTN W = Gop->Mode->Info->HorizontalResolution;
    UINTN H = Gop->Mode->Info->VerticalResolution;
    UINTN Stride = Gop->Mode->Info->PixelsPerScanLine;
    UINT32 *Fb = (UINT32 *)(UINTN)Gop->Mode->FrameBufferBase;
    UINT32 *p = Fb;
    for (UINTN y = 0; y < H; y++) {
      UINT32 c = (y < H / 2) ? 0x00FF0000u : 0x000000FFu;
      UINT32 *row = Fb + y * Stride;
      for (UINTN x = 0; x < W; x++) row[x] = c;
      p = row;
    }
    puts_ascii(ConOut, "GBLPROBE: painted red/blue bands, spinning forever\r\n");
  } else {
    volatile UINT32 *Fb2 = (volatile UINT32 *)(UINTN)0xFC800000UL;
    for (UINTN i = 0; i < 4561920u; i++)
      Fb2[i] = (i & 1) ? 0x00FF0000u : 0x000000FFu;
    puts_ascii(ConOut, "GBLPROBE: GOP failed; fixed-addr fallback painted\r\n");
  }

  for (;;) { __asm__ volatile ("wfe"); }
  return 0;
}
