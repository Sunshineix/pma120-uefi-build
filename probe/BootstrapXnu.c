/* BootstrapXnu.c — EFI-app wrapper that boots the hakios XNU loader.
 *
 * Runs in GBL's EFI environment via `fastboot stage` + `fastboot oem boot-efi`.
 * Steps:
 *   1. paint the splash orange (wrapper entered marker)
 *   2. copy the embedded XNUP payload to PAYLOAD_BASE (0xF0000000, per loader.c)
 *   3. flush caches so the loader sees the payload at physical memory
 *   4. LoadImage + StartImage the embedded hakios loader.efi
 *   5. if the loader ever returns: paint yellow + spin
 *
 * No EDK2 headers: minimal UEFI structures from the spec (stable layouts). */
typedef unsigned long long UINT64;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;
typedef UINT64             UINTN;
typedef void               VOID;
typedef UINTN              EFI_STATUS;
typedef VOID              *EFI_HANDLE;

typedef struct {
  UINT64 Hdr[3];
  VOID  *ConIn;
  VOID  *ConOut;
  VOID  *StdErr;
  VOID  *RuntimeServices;
  VOID  *BootServices;   /* offset 56 */
} EFI_SYSTEM_TABLE;

/* EFI_BOOT_SERVICES function pointers used here (spec offsets) */
typedef struct {
  UINT32 Data1;
  UINT16 Data2;
  UINT16 Data3;
  UINT8  Data4[8];
} EFI_GUID;

typedef EFI_STATUS (*EFI_IMAGE_LOAD)(UINT8 BootPolicy, EFI_HANDLE ParentImageHandle,
                                     VOID *DevicePath, VOID *SourceBuffer,
                                     UINTN SourceSize, EFI_HANDLE *ImageHandle);
typedef EFI_STATUS (*EFI_IMAGE_START)(EFI_HANDLE ImageHandle, UINTN *ExitDataSize,
                                      UINT16 **ExitData);
typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(EFI_GUID *, VOID *, VOID **);

/* QCOM_USB_DEVICE_GUID: d9d9ce48-44b8-4f49-8e3e-2a3b927dc6c1 */
static const EFI_GUID UsbDeviceGuid = {
  0xd9d9ce48, 0x44b8, 0x4f49,
  { 0x8e, 0x3e, 0x2a, 0x3b, 0x92, 0x7d, 0xc6, 0xc1 }
};

/* EFI_USB_DEVICE_PROTOCOL (QcomPkg EFIUsbDevice.h): polled peripheral-mode API.
   stop at offset 48: deconfigures the controller that GBL's fastboot left started. */
typedef struct {
  UINT64      Revision;
  VOID       *Start;
  VOID       *Send;
  VOID       *HandleEvent;
  VOID       *AllocateTransferBuffer;
  VOID       *FreeTransferBuffer;
  EFI_STATUS (*Stop)(VOID);
} USB_DEVICE_PROTOCOL;

static void UsbRelease(EFI_SYSTEM_TABLE *ST)
{
  EFI_LOCATE_PROTOCOL Locate = *(EFI_LOCATE_PROTOCOL *)((UINT8 *)ST->BootServices + 296);
  USB_DEVICE_PROTOCOL *Usb = 0;
  if (Locate((EFI_GUID *)&UsbDeviceGuid, 0, (VOID **)&Usb) == 0 && Usb != 0 && Usb->Stop != 0) {
    Usb->Stop();
  }
}

#define SPLASH       0xFC800000ULL
#define SCREEN_W     1440
#define SCREEN_H     3168
#define PAYLOAD_BASE 0xF0000000ULL

extern UINT8 PayloadImage[];
extern UINT8 PayloadEnd[];
extern UINT8 LoaderImage[];
extern UINT8 LoaderEnd[];
extern void  CacheFlushRange(VOID *p, UINTN len);

static void FillScreen(UINT32 Color)
{
  UINT32 *Fb = (UINT32 *)(UINTN)SPLASH;
  UINTN   i;
  for (i = 0; i < (UINTN)SCREEN_W * SCREEN_H; i++) {
    Fb[i] = Color;
  }
  CacheFlushRange((VOID *)(UINTN)SPLASH, (UINTN)SCREEN_W * SCREEN_H * 4);
}

static void Halt(void)
{
  for (;;) {
    __asm__ volatile ("wfe");
  }
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
  UINT8 *src, *dst, *end;
  UINTN  len;
  EFI_IMAGE_LOAD  Load;
  EFI_IMAGE_START Start;
  EFI_HANDLE      LoaderHandle = 0;
  EFI_STATUS      Status;

  /* 1. orange: wrapper entered */
  FillScreen(0x00FF8000u);

  /* 2. copy the XNUP payload to PAYLOAD_BASE (8-byte chunks) */
  src = PayloadImage;
  end = PayloadEnd;
  dst = (UINT8 *)(UINTN)PAYLOAD_BASE;
  while (src < end) {
    *(UINT64 *)(UINTN)dst = *(UINT64 *)(UINTN)src;
    src += 8;
    dst += 8;
  }
  len = (UINTN)(PayloadEnd - PayloadImage);
  CacheFlushRange((VOID *)(UINTN)PAYLOAD_BASE, len);

  /* 3. release the USB controller GBL's fastboot left started, so the
        loader's own start_ex() can configure it. */
  UsbRelease(ST);

  /* 4. LoadImage + StartImage the loader */
  Load  = *(EFI_IMAGE_LOAD *)((UINT8 *)ST->BootServices + 176);
  Start = *(EFI_IMAGE_START *)((UINT8 *)ST->BootServices + 184);

  Status = Load(0, ImageHandle, 0, (VOID *)LoaderImage,
                (UINTN)(LoaderEnd - LoaderImage), &LoaderHandle);
  if (Status != 0) {
    /* magenta: LoadImage failed */
    FillScreen(0x00FF00FFu);
    Halt();
  }

  Status = Start(LoaderHandle, 0, 0);

  /* yellow: the loader returned without handing off to the kernel */
  FillScreen(0x00FFFF00u);
  Halt();
  return Status;
}
