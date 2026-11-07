## Firmware Infos

- **Device:** OPPO Find X9 Ultra (PMA120)
- **Region:** CN
- **Version:** ColorOS 17 / `PMA120_11.C.20_1200_202610051631`
- **UEFI BuildId:** `BOOT.MXF.2.5.3-00199-KAANAPALI-1.131443.27`
- **Source:** `uefi_a` partition of the phone itself (untouched by GBL; read-only dump 2026-11-07,
  sha256 of `uefi_a.img` F393CECB8D7B5450A8BCE034155C81792E3FB5AD6523AB60043805420357C7B3)

## Extraction

- Tool: [UEFIReader](https://github.com/WOA-Project/UEFIReader) (net8.0), built locally with the SDK's
  Roslyn `csc` (MSBuild/NuGet bypassed).
- Local patches (see `X9U-iOS移植/vendor-DXE提取runbook.md`):
  1. volume scan takes the first `_FVH` candidate whose magic AND volume checksum validate
     (SM8850 images contain false `_FVH` hits before the real volume);
  2. same-named modules are disambiguated with the GUID prefix instead of aborting.
- 422 files extracted: 88 QcomPkg driver directories, full ArmPkg/EmbeddedPkg/FatPkg/MdeModulePkg set,
  RawFiles with 91 panel XMLs (includes this phone's `Panel_AC347_P_3_A0019_dsc_cmd.xml`).

## Patches/Fixes

None applied yet. Candidates ported from redwood (verify each on SM8850 before applying):

- ButtonsDxe: Qcom key code 0x102 -> Enter 0xD (power button usable in UEFI)
- DALSYSDxe: enable cache coherence for UFS
- TzDxeLA: global TZ applet variable TRUE (avoid UEFI boot crash)
- UsbConfigDxe: remove USB deinit from ExitBootServices
- UsbMsdDxe: removable -> non-removable (storage as hard drives)

Binary patches are applied with byte-level edits; back up the original extraction first and record
every patch here.
