# pma120-uefi-build

UEFI firmware package for **OPPO Find X9 Ultra (pma120, SM8850 "canoe")**, built on
Project Mu + Project Silicium. It produces the firmware volume used by the UniXNU
iPadOS 27 bring-up for this device.

## What is in here

| Path | Contents |
|---|---|
| `Platforms/OPPO/pma120Pkg` | Device package: `pma120.dsc` / `pma120.fdf`, DXE include lists, `Pma120XnuDxe`, `Pma120Diag`, FDT blob |
| `Binaries/pma120` | Vendor DXE modules extracted from the stock firmware (research use only) |
| `Silicon/`, `Common/`, `Silicium-ACPI/`, `Mu_Basecore/` | Mu-Silicium base (git submodules) |
| `Resources/DTBs` | Reference DTBs for other platforms (upstream) |
| `probe/` | Legacy EFI staging loaders (HakiOS/XNUP era). The current bring-up uses a standalone carrier image instead |
| `docs/` | Handoff notes and dead ends. Start with `docs/HANDOFF-20261110.md` |
| `build_uefi.py` | Build driver |

## Build (WSL)

```bash
python3 -m venv .venv && . .venv/bin/activate
pip install -r pip-requirements.txt
python3 build_uefi.py -d pma120 -r DEBUG
```

Output: `Build/pma120Pkg/DEBUG_CLANGPDB/FV/SILICIUM_UEFI.fd`.
See [Building.md](Building.md) for prerequisites and options.

## Bring-up status (2026-11-10)

- Kernel `kernel.macho` (iPadOS 27.0.1 / 24A446, prelinked, 95 MB) builds from the device repo.
- Firmware boots on hardware to the UniXNU menu; DarwinBoot hands off and the XNU entry code
  executes (entry-probe blue band). It currently stops before the MMU is enabled (no magenta
  band) — suspected bootstrap page-table exhaustion with the former 8 GiB DarwinBoot window;
  a 6 GiB window test is pending.
- Everything is RAM-only: `fastboot stage` + `fastboot oem boot-efi`. No partition is written.

## Related repositories

- Device port (`board.toml`, DTS, kernel patches): https://github.com/Sunshineix/UniXNU-pma120
- Upstream UniXNU: https://github.com/UniXNU/UniXNU

## License

BSD-2-Clause-Patent; see [LICENSE](LICENSE). Modules under `Binaries/` remain the property of
their vendors and are included for interoperability research only.
