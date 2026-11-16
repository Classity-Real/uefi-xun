# XUN for UniXNU

> **Planning/scaffolding stage.** Nothing in this package has been built or run
> on a Xiaomi/Redmi Pad SE yet. This is a skeleton seeded from tb128fuPkg
> (Lenovo Xiaoxin Pad 2022, same SM6225/bengal SoC family, which IS reported
> to boot to the iPadOS 17.7.11 home screen), adapted with the device
> readouts published in `KernelZero123/UniXNU-xun`'s `docs/DEVICE.md`. See
> the top-level `README.md` for the phased plan this package is Phase 0/1 of.

This adds the Xiaomi/Redmi Pad SE (xun, SM6225 bengal) to the Mu-Silicium
tree and integrates UniXNU, the same way tb128fuPkg does for the Lenovo
Xiaoxin Pad 2022.

## What's directly reused from tb128fuPkg (SoC-level, not board-level)

* SoC: SM6225/bengal, 4xA73 + 4xA53, Adreno 610 with no GMU.
* Pure-Mu boot chain assumption (no extractable vendor DXE from stock ABL) --
  **not yet confirmed for xun's own ABL**, see "Unverified" below.
* `ConfigurationMapLib`'s purely numeric SM6225 entries (NumCpus,
  MaxLogFileSize, SDHC GPIO configs, etc).
* The splash/DXE-heap/FD/removed-region address block
  (`0x5C000000`..`0x63900000`) and the FD placement at `0x5FC00000` (3 MiB)
  inside it -- confirmed to be the same shape on a real xun unit
  (see `docs/DEVICE.md`).

## What's different on xun and was NOT copied

| Item | tb128fu | xun |
|---|---|---|
| Panel | 1200x2000, hx83102p | 1200x1920 portrait native (HUAXING `m84_42_03_0c`, video-mode FHD+); two other known panel variants (`m84_35_03_0b`, `m84_36_02_0a`) not yet confirmed absent |
| Touch | Himax HX83102E over SPI | FocalTech FT8203 over SPI, IRQ GPIO 478, reset GPIO 484, firmware `focaltech_ts_fw_huaxing_ft8203.bin` -- Phase 3, not implemented here |
| Storage | UFS (NativeUfs path) | eMMC via `4784000.sdhci` (`mmcblk0`), slot A, `boot_a`/`boot_b` 128 MiB each, `userdata` on `mmcblk0p91` at LBA `0x1D16000` -- Phase 2, not implemented here |
| Boot image | header v1, single boot.img, `fastboot flash boot_a` works, `fastboot boot` buffer is too small | header **v4**, separate `boot`/`init_boot`/`vendor_boot` -- how to get a test FD booted via fastboot is **unverified**, see below |
| DRAM layout | two banks, no gap | **three banks with a real hole**: `0x40000000`+`0x3CC00000` (ends `0x7CC00000`), `0x80000000`+`0x40000000`, `0xC0000000`+`0x100000000`. ~6092 MiB visible to the kernel. Do not reuse tb128fu's `dram_size`. |
| Splash framebuffer | ABL leaves a live `cont_splash_region` for `SimpleFbDxe` to scan out | **No `cont_splash_region` found** on the real unit read in `docs/DEVICE.md` -- see "Unverified", this is the single biggest open risk in this package |

## Changes in this package

* `xun.dsc` -- same `DivarPkg` (SM6225) base as tb128fu, stack at
  `0x5FF90000`, `SimpleFbDxe` set to 1200x1920 (not tb128fu's 1200x2000),
  SMBIOS strings for Xiaomi/Redmi Pad SE, `PcdBuildXblHobs = FALSE` (assumed,
  unverified). Builds UniXNU, UniXNUDxe, `ShengBootMemoryDxe` and DarwinBoot,
  same as tb128fu.
* `xun.fdf` -- the same Mu-only DXE set as tb128fu, new `FvNameGuid` and
  `FV_IMAGE` file GUID (so this doesn't collide with tb128fu's FV if both
  trees are ever built from the same workspace).
* `Include/DXE.inc` -- same driver list as tb128fu's, `Tb128fuKeyInputDxe`
  renamed to `XunKeyInputDxe`, `SimpleFbDxe`'s comment updated to flag the
  missing `cont_splash_region`.
* `Include/RAW.inc` -- ported comment, flagged as unverified whether xun's
  ABL is also a single statically-linked loader with no raw files to pull in
  (tb128fu's is; Xiaomi's Khaje/bengal ABL has not been checked).
* `Drivers/XunKeyInputDxe` -- `Tb128fuKeyInputDxe` renamed, new FILE_GUID.
  GPIO numbering (volume up on TLMM GPIO 5 as Enter) assumed unchanged from
  tb128fu -- **not confirmed against xun's own device tree.**
* `Library/MemoryMapLib` -- rebuilt from `docs/DEVICE.md`'s live readout
  (three DRAM banks with the `0x7CC00000`..`0x80000000` hole; splash/
  ramoops/removed block kept at tb128fu's addresses since the readout found
  the same footprint; SDHCI eMMC controller region added at `0x04784000`).
* `Library/ConfigurationMapLib` -- SoC-generic entries kept as-is; board
  addresses inherited from tb128fu's tree (`SharedIMEMBaseAddr`,
  `DloadCookieAddr`, `TzAppsRegnAddr`/`Size`) are flagged unverified for xun.
* `Resources/Configs/xun.toml` -- FD base/size carried over (same free gap);
  `header_version = 4` set to match xun's real boot image format, but with
  the open risk noted inline (see "Unverified").
* `Resources/DTBs/xun.dtb` -- **not yet present.** tb128fu's equivalent is
  the merged live Khaje IDP tree with bootargs/initrd properties stripped.
  Need the same treatment for a real xun device-tree dump before Phase 1 can
  actually run.

## Build

Not yet runnable end-to-end -- this package has never been built. Once the
device repo analogue of `UniXNU-tb128fu` exists for xun (Phase 0), the build
is expected to follow the same shape as tb128fu's:

```sh
cd uefi-xun
git submodule update --init --depth 1 -- Binaries Common/Mu Common/Mu_OEM_Sample Silicium-ACPI Silicon/Silicium/OpensslPkg/Library/OpensslLib/openssl

cd ../UniXNU
export UNIXNU_UEFI=$PWD/../uefi-xun
./unixnu build firmware -D ../UniXNU-xun
```

## Unverified

These are the open questions blocking Phase 1, roughly in the order they
need to be resolved:

1. **No `cont_splash_region` on xun.** tb128fu's entire display strategy is
   "scan out whatever ABL already left running." The live readout in
   `docs/DEVICE.md` did not find that region on xun. Until this is
   re-checked against xun's actual device tree (not just the reserved
   address range, which does match), `SimpleFbDxe` may come up with no
   framebuffer to publish at all, and the UEFI menu may not be visible even
   if everything else works.
2. **Boot image header v4.** Whether `fastboot boot` accepts a v4-packed FD
   the way it accepts tb128fu's v1 image, or whether the payload needs to go
   through `vendor_boot` instead. This blocks even a first `fastboot boot`
   test of this package.
3. Whether xun's ABL is also a thin Linux loader with no extractable vendor
   DXE (as assumed in `RAW.inc` and `PcdBuildXblHobs|FALSE`), or whether
   Xiaomi's bengal ABL differs from Lenovo's enough to carry real vendor DXE
   modules worth extracting into a `Binaries/xun` directory.
4. Whether the GPIO numbering in `XunKeyInputDxe` (volume up on TLMM GPIO 5)
   holds on xun's own pinctrl, not just inherited from tb128fu.
5. Whether `SharedIMEMBaseAddr`, `DloadCookieAddr`, `TzAppsRegnAddr`/`Size`
   in `ConfigurationMapLib` (confirmed against TB128FU's device tree, not
   xun's) are safe to reuse.
6. Which of the three known xun panel variants (`m84_35_03_0b`,
   `m84_36_02_0a`, `m84_42_03_0c`) is on hand -- `docs/DEVICE.md` confirms
   `m84_42_03_0c` on the unit it read, but that may not match every xun in
   the wild.
