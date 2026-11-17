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
| Touch | Himax HX83102E over SPI | FocalTech over SPI at `spi@4a80000` (CONFIRMED from the live device tree, 2026-11-17, resolving the `spi0.0`/`4a80000.spi` ambiguity); IRQ **TLMM GPIO 80**, reset **TLMM GPIO 86** (CONFIRMED, corrects an earlier 478/484 reading); display-coords confirm 1200x1920 at 10x oversampling -- Phase 3, not implemented here |
| Storage | UFS (NativeUfs path) | eMMC via `4784000.sdhci` (`mmcblk0`), slot A, `boot_a`/`boot_b` 128 MiB each, `userdata` on `mmcblk0p91` -- confirmed by position/size in a live `/proc/partitions` + GPT read, 2026-11-17 -- Phase 2, not implemented here |
| Boot image | header v1, single boot.img, `fastboot flash boot_a` works, `fastboot boot` buffer is too small | header **v4, confirmed** by parsing the real `boot_a`/`vendor_boot_a` image headers, 2026-11-17 -- kernel/ramdisk/tags offsets also confirmed matching. Whether `fastboot boot` works the same way against a v4 image is still **unverified**, see below |
| DRAM layout | two banks, no gap | **two regions with one real hole, confirmed from `/proc/iomem`, 2026-11-17** -- corrected from an earlier 3-bank estimate: `0x40000000`+`0x3BB00000` (ends `0x7BB00000`), then `0x80000000`+`0x1C0000000` (7 GiB, single contiguous span, no second gap at `0xC0000000` as previously assumed). Consistent with an 8 GB RAM xun variant; re-check on a different RAM SKU. |
| Splash framebuffer | ABL leaves a live `cont_splash_region` for `SimpleFbDxe` to scan out | `cont_splash_region` **CONFIRMED present** in the live device tree, 2026-11-17: a reserved-memory node at `0x5C000000`, 15 MiB, literally labeled `"cont_splash_region"`. This **reverses** an earlier "not found" reading (from a non-rooted readout). No Linux driver references it by phandle, which is expected (ABL/Linux handoff, not a Linux-consumed binding) -- see "Unverified", item 1 for what's still genuinely open |
| Volume-up key | TLMM GPIO 5, flat MMIO read | **CONFIRMED different peripheral, 2026-11-17:** PM6125 PMIC's own GPIO 5, over the SPMI PMIC arbiter -- `XunKeyInputDxe` as inherited reads the wrong hardware (TLMM, not SPMI) and does not work yet. See "Unverified", item 4 |

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
  renamed to `XunKeyInputDxe`. `SimpleFbDxe`'s comment updated 2026-11-17:
  `cont_splash_region` is now confirmed present (see table above), reversing
  an earlier "missing" note.
* `Include/RAW.inc` -- ported comment, flagged as unverified whether xun's
  ABL is also a single statically-linked loader with no raw files to pull in
  (tb128fu's is; Xiaomi's Khaje/bengal ABL has not been checked).
* `Drivers/XunKeyInputDxe` -- `Tb128fuKeyInputDxe` renamed, new FILE_GUID.
  **Confirmed broken for xun, 2026-11-17:** the inherited implementation
  reads TLMM GPIO 5 via flat MMIO; xun's volume-up key is actually on the
  PM6125 PMIC's SPMI-based GPIO 5, a completely different access path. See
  `XunKeys.h`'s file header for what needs replacing -- Phase 5, not a
  Phase 1 blocker (the menu still renders without a working Enter key).
* `Library/MemoryMapLib` -- rebuilt in stages: first from `docs/DEVICE.md`'s
  readout (a 3-bank estimate), then corrected 2026-11-17 against a live
  `/proc/iomem` + boot image header read (DRAM is two regions with a hole
  at `0x7BB00000`..`0x80000000`, not three banks; splash/ramoops/removed
  range confirmed unchanged; Boot Image/Ramdisk placement confirmed from
  `vendor_boot_a`'s header), then again the same day against the real
  device tree (confirmed `cont_splash_region` exists; corrected touch
  IRQ/reset GPIOs; SDHCI eMMC controller region at `0x04784000`).
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

Two dumps were taken off the user's own xun unit on 2026-11-17: a root
`/proc/iomem`/`/proc/partitions`/GPT/boot-image read, then (after fixing a
symlink-dereference bug in the first attempt) a full live device-tree pull.
Items marked **RESOLVED** were settled by one of those two dumps; see the
readout summary at the top of `Library/MemoryMapLib/MemoryMapLib.c` for
detail. Remaining order of what still blocks Phase 1:

1. **RESOLVED -- `cont_splash_region` exists.** The live device tree has a
   `reserved-memory` node at `0x5C000000`, 15 MiB, literally labeled
   `cont_splash_region`. This reverses the earlier "not found" reading
   (which came from a non-rooted dump). No Linux driver references this
   node by phandle, but that's expected, not a red flag -- ABL paints the
   splash pixels there before Linux ever boots, so Linux has no reason to
   reference it any more than tb128fu's own equivalent region does. **Still
   not fully proven**: the only way to be certain ABL actually leaves live
   pixels there (rather than the carve-out just existing, unused, for some
   other reason) is to try `SimpleFbDxe` against it with a real
   `fastboot boot` and see what's on screen. This is now the right next
   step to attempt, not a reason to hold off.
2. **RESOLVED -- boot image header v4.** Both `boot_a` and `vendor_boot_a`
   confirmed `header_version = 4` by directly parsing their Android boot
   image headers; `vendor_boot_a`'s header also confirmed the
   kernel/ramdisk/tags load offsets already in `xun.toml`/`MemoryMapLib.c`.
   **Still open:** whether `fastboot boot` accepts a v4-packed FD the same
   way it accepts tb128fu's v1 image, or whether the payload needs to go
   through `vendor_boot`'s ramdisk-table mechanism instead. Needs an actual
   attempt on hardware to answer, not a header read -- same next step as
   item 1 above, both are answered by the same first boot attempt.
3. Whether xun's ABL is also a thin Linux loader with no extractable vendor
   DXE (as assumed in `RAW.inc` and `PcdBuildXblHobs|FALSE`), or whether
   Xiaomi's bengal ABL differs from Lenovo's enough to carry real vendor DXE
   modules worth extracting into a `Binaries/xun` directory. Not touched by
   either 2026-11-17 dump.
4. **RESOLVED (and the inherited driver is confirmed BROKEN) -- volume-up
   key.** The live device tree's `gpio_keys/vol_up` node is on
   `qcom,pm6125@0/pinctrl@c000` -- the PM6125 PMIC's own SPMI-based GPIO
   block, not TLMM. `XunKeyInputDxe`/`XunKeys.h` as inherited from tb128fu
   does a flat MMIO read of a TLMM register and will never see this key
   press on xun. Both devices happen to use GPIO number "5", which is
   coincidence, not equivalence. Fixing this needs an SPMI PMIC arbiter
   transaction against PM6125 GPIO 5, not implemented -- Phase 5, not a
   Phase 1 blocker (the menu still renders without a working Enter key).
5. Whether `SharedIMEMBaseAddr`, `DloadCookieAddr`, `TzAppsRegnAddr`/`Size`
   in `ConfigurationMapLib` (confirmed against TB128FU's device tree, not
   xun's) are safe to reuse. Not touched by either 2026-11-17 dump.
6. **RESOLVED for this unit -- panel variant.** `m84_42_03_0c` confirmed
   both from the live cmdline and from the device tree's active panel node
   (two other variant nodes, `m84_35_03_0b` and `m84_36_02_0a`, are also
   compiled into the same tree but inactive on this unit). A different xun
   unit could still boot with a different variant selected.
7. **RESOLVED -- touch bus and GPIOs.** The live device tree's `touch@0`
   node sits on `qupv3_0_geni_se@4ac0000/spi@4a80000` (confirming
   `4a80000.spi`, not `spi0.0`), with `focaltech,irq-gpio` = TLMM GPIO 80
   and `focaltech,reset-gpio` = TLMM GPIO 86 -- corrects an earlier 478/484
   reading from the non-rooted dump. `focaltech,display-coords` confirms
   1200x1920 at 10x oversampling, matching the already-assumed touch
   resolution. The same node also carries Novatek-compatible properties for
   a different panel SKU; FocalTech is confirmed active on this unit via
   the kernel cmdline.
