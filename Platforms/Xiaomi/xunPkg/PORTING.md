# XUN for UniXNU

> **Planning/scaffolding stage.** Nothing in this package has been built or run
> on a Xiaomi/Redmi Pad SE yet. This is a skeleton seeded from tb128fuPkg
> (Lenovo Xiaoxin Pad 2022, same SM6225/bengal SoC family, which IS reported
> to boot to the iPadOS 17.7.11 home screen), originally adapted with the
> device readouts published in `KernelZero123/UniXNU-xun`'s `docs/DEVICE.md`,
> then corrected and partly confirmed 2026-11-17 against a direct
> `/proc/iomem` + boot image readout off the user's own xun unit (details
> throughout this file and in `MemoryMapLib.c`). See the top-level
> `README.md` for the phased plan this package is Phase 0/1 of.

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
| Panel | 1200x2000, hx83102p | 1200x1920 portrait native (HUAXING `m84_42_03_0c`, video-mode FHD+) -- **confirmed directly from a live kernel cmdline** (`qcom,mdss_dsi_panel_m84_42_03_0c_fhdp_video`) on the user's own unit, 2026-11-17, matching KernelZero123's independent reading; two other known panel variants (`m84_35_03_0b`, `m84_36_02_0a`) exist on other units |
| Touch | Himax HX83102E over SPI | FocalTech FT8203 over SPI -- driver name **confirmed** from the same cmdline (`focaltech_ts_spi.tpsensor=focaltech_ts_ab`); IRQ GPIO 478 / reset GPIO 484 / firmware `focaltech_ts_fw_huaxing_ft8203.bin` still inherited, not independently confirmed -- Phase 3, not implemented here |
| Storage | UFS (NativeUfs path) | eMMC via `4784000.sdhci` (`mmcblk0`), slot A, `boot_a`/`boot_b` 128 MiB each, `userdata` on `mmcblk0p91` -- confirmed by position/size in a live `/proc/partitions` + GPT read, 2026-11-17 -- Phase 2, not implemented here |
| Boot image | header v1, single boot.img, `fastboot flash boot_a` works, `fastboot boot` buffer is too small | header **v4, confirmed** by parsing the real `boot_a`/`vendor_boot_a` image headers, 2026-11-17 -- kernel/ramdisk/tags offsets also confirmed matching. Whether `fastboot boot` works the same way against a v4 image is still **unverified**, see below |
| DRAM layout | two banks, no gap | **two regions with one real hole, confirmed from `/proc/iomem`, 2026-11-17** -- corrected from an earlier 3-bank estimate: `0x40000000`+`0x3BB00000` (ends `0x7BB00000`), then `0x80000000`+`0x1C0000000` (7 GiB, single contiguous span, no second gap at `0xC0000000` as previously assumed). Consistent with an 8 GB RAM xun variant; re-check on a different RAM SKU. |
| Splash framebuffer | ABL leaves a live `cont_splash_region` for `SimpleFbDxe` to scan out | **Still unconfirmed either way.** `/proc/iomem` confirms the expected address range is reserved, but a device-tree pull meant to settle this definitively failed in the 2026-11-17 session (see "Unverified", item 1) -- still the single biggest open risk in this package |

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
* `Library/MemoryMapLib` -- rebuilt twice: first from `docs/DEVICE.md`'s
  readout (a 3-bank estimate), then corrected 2026-11-17 against a live
  `/proc/iomem` + boot image header read off a real unit -- the actual DRAM
  layout is two regions with a hole at `0x7BB00000`..`0x80000000`, not
  three banks with a second gap at `0xC0000000`. Splash/ramoops/removed
  block address range confirmed unchanged; SDHCI eMMC controller region at
  `0x04784000`; Boot Image/Ramdisk placement confirmed from the real
  `vendor_boot_a` header (kernel 0x8000, ramdisk 0x1000000, tags 0x100,
  relative to base 0x80000000).
* `Library/ConfigurationMapLib` -- SoC-generic entries kept as-is; board
  addresses inherited from tb128fu's tree (`SharedIMEMBaseAddr`,
  `DloadCookieAddr`, `TzAppsRegnAddr`/`Size`) are still flagged unverified
  for xun -- this dump did not touch them.
* `Resources/Configs/xun.toml` -- FD base/size carried over (same free gap,
  confirmed); `header_version = 4` and the kernel/ramdisk/tags offsets are
  now **confirmed**, not inherited placeholders (see "Unverified" for what's
  still open about boot image handling).
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
need to be resolved. Items marked **RESOLVED** were settled 2026-11-17
against a real xun unit's own `/proc/iomem`, `/proc/partitions`, GPT, and
`boot_a`/`vendor_boot_a` image headers (see the readout summary at the top
of `Library/MemoryMapLib/MemoryMapLib.c`).

1. **No `cont_splash_region` confirmation on xun -- STILL OPEN, now the
   single clearest blocker.** tb128fu's entire display strategy is "scan out
   whatever ABL already left running." `/proc/iomem` confirms the expected
   address range (`0x5C000000`..`0x5D200000`) is reserved, which is
   necessary but not sufficient to prove a live framebuffer is actually
   there. The direct check -- grepping the real device tree for a
   `reserved-memory` node under that name -- was attempted but **failed**:
   the dump ran `tar -cf ... /proc/device-tree`, which only archived the
   symlink itself (`/proc/device-tree -> /sys/firmware/devicetree/base`),
   not its contents. **To actually resolve this**, re-run with
   dereferencing, e.g. (as root, in Termux or adb shell):
   ```sh
   cp -rL /sys/firmware/devicetree/base /sdcard/xun_dump/dt_dump
   # or: tar -chf /sdcard/xun_dump/device-tree.tar -C / proc/device-tree
   find /sdcard/xun_dump/dt_dump -iname "*splash*" -o -iname "*cont*"
   ```
   and look for a node under `reserved-memory` with `display-reserved`,
   `cont-splash-mem`, or similar in its name/compatible string. If nothing
   turns up, the display strategy inherited from tb128fu (Phase 1's whole
   premise) needs to be rethought before anything else in this package is
   worth trusting.
2. **RESOLVED -- boot image header v4.** Both `boot_a` and `vendor_boot_a`
   confirmed `header_version = 4` by directly parsing their Android boot
   image headers; `vendor_boot_a`'s header also confirmed the
   kernel/ramdisk/tags load offsets already in `xun.toml`/`MemoryMapLib.c`.
   **Still open:** whether `fastboot boot` accepts a v4-packed FD the same
   way it accepts tb128fu's v1 image, or whether the payload needs to go
   through `vendor_boot`'s ramdisk-table mechanism instead. This blocks even
   a first `fastboot boot` test of this package, and needs an actual attempt
   on hardware to answer, not a header read.
3. Whether xun's ABL is also a thin Linux loader with no extractable vendor
   DXE (as assumed in `RAW.inc` and `PcdBuildXblHobs|FALSE`), or whether
   Xiaomi's bengal ABL differs from Lenovo's enough to carry real vendor DXE
   modules worth extracting into a `Binaries/xun` directory.
4. Whether the GPIO numbering in `XunKeyInputDxe` (volume up on TLMM GPIO 5)
   holds on xun's own pinctrl, not just inherited from tb128fu.
5. Whether `SharedIMEMBaseAddr`, `DloadCookieAddr`, `TzAppsRegnAddr`/`Size`
   in `ConfigurationMapLib` (confirmed against TB128FU's device tree, not
   xun's) are safe to reuse. Not touched by the 2026-11-17 dump.
6. **Narrowed, not fully resolved -- panel variant.** `m84_42_03_0c` is now
   confirmed directly from a live cmdline on the user's own unit (not just
   inherited from `docs/DEVICE.md`), so this specific unit is settled. The
   other two known variants (`m84_35_03_0b`, `m84_36_02_0a`) may still exist
   on other xun units in the wild and would need their own panel ID check.
