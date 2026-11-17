# xun device readout (reference)

Summary of a real-device readout published upstream in
[`KernelZero123/UniXNU-xun`](https://github.com/KernelZero123/UniXNU-xun)'s
`docs/DEVICE.md`, dated 2026-10-31, taken from a Xiaomi/Redmi Pad SE without
root. This is paraphrased here as a working reference for this package's
`Library/MemoryMapLib` and `Resources/Configs/xun.toml` -- go to the upstream
file for the authoritative original.

**Boot and storage**
- Device boots from slot A.
- `boot_a` / `boot_b` are 128 MiB each.
- `userdata` is ~104 GiB on `mmcblk0p91`, starting at LBA `0x1D16000`.
- Block device is `mmcblk0` (eMMC) -- the system is NOT on UFS, unlike
  tb128fu's `uefi-tb128fu` docs, which describe a UFS HS-G3 link for that
  device. Do not reuse tb128fu's `NativeUfs`/`native-ufs-root=1` path.

**Memory**
- Kernel-visible RAM is ~6092 MiB across three DRAM banks with a hole
  between `0x7CC00000` and `0x80000000`.
- Do not reuse tb128fu's `dram_size` value or its two-bank layout.

**Reserved regions**
- No `cont_splash_region` was found on this unit -- this is the single
  biggest open risk for this package's display bring-up (see
  `Platforms/Xiaomi/xunPkg/PORTING.md`, "Unverified").
- The splash, DFPS, ramoops and removed regions occupy
  `0x5C000000`..`0x63900000`, the same footprint tb128fu uses.
- The gap from `0x5D200000` to `0x60000000` is free, and the tb128fu
  firmware-volume placement at `0x5FC00000` (3 MiB) fits inside it
  unchanged.

**Display and touch**
- Panel: HUAXING `m84_42_03_0c`, video-mode FHD+ -- one of (at least) three
  known xun panel variants (`m84_35_03_0b`, `m84_36_02_0a`, `m84_42_03_0c`).
- Touch: FocalTech FT8203 on SPI, NOT the Novatek or Himax (HX83102E)
  drivers tb128fu uses.
  - Touch resolution: 1200x1920.
  - IRQ: GPIO 478. Reset: GPIO 484.
  - Firmware file: `focaltech_ts_fw_huaxing_ft8203.bin`.

## Still needed (per upstream UniXNU-xun, as of its last update)

- A `xunPkg` Mu platform package did not exist upstream as of the readout
  above -- this repository is the attempt to build one.
- Partition layout, splash region length, and the `userdata` starting LBA
  were already read (see above); the eMMC block driver, the actual boot
  header v4 payload format, and a confirmed live framebuffer strategy were
  not.

## Direct readout, 2026-11-17 (this repo, user's own unit, rooted)

A root dump (`/proc/iomem`, `/proc/partitions`, GPT, `boot_a`/`vendor_boot_a`
image headers, kernel cmdline) was taken directly off the user's own xun
tablet and used to correct and confirm parts of the upstream readout above.
Full detail is in `Platforms/Xiaomi/xunPkg/PORTING.md` and the header
comment of `Library/MemoryMapLib/MemoryMapLib.c`; summary:

- **Corrected:** actual DRAM layout is two regions, not three banks --
  `0x40000000`+`0x3BB00000` (ends `0x7BB00000`, not `0x7CC00000` as the
  upstream estimate had it), then a single contiguous 7 GiB region at
  `0x80000000`..`0x240000000` with no second gap at `0xC0000000`. Consistent
  with an 8 GB RAM xun variant.
- **Confirmed:** panel `m84_42_03_0c` and FocalTech SPI touch, directly from
  a live kernel cmdline (`qcom,mdss_dsi_panel_m84_42_03_0c_fhdp_video`,
  `focaltech_ts_spi.tpsensor=focaltech_ts_ab`) -- independent confirmation of
  the upstream reading, not just a repeat of it.
- **Confirmed:** `boot_a` and `vendor_boot_a` both report Android boot
  header_version 4; `vendor_boot_a`'s own header gives kernel/ramdisk/tags
  load offsets matching what was already inherited into this repo's
  `xun.toml`.
- **Confirmed:** splash/ramoops/removed reserved address range
  (`0x5C000000`..`0x63900000`) matches the upstream reading exactly.
- **Still NOT resolved:** whether a live `cont_splash_region` actually
  exists. The attempt to check this directly (pulling the real device tree)
  failed in this session because the dump command ran `tar` on
  `/proc/device-tree` without dereferencing the symlink it actually is --
  see `PORTING.md`, "Unverified", item 1, for the corrected command to
  re-try this.
- Touch IRQ/reset GPIO numbers (478/484) and the FT8203 firmware filename
  were not independently re-checked by this dump -- still inherited from
  the upstream reading.

## Direct readout, 2026-11-17 part two (this repo, user's own unit, live device tree)

The symlink bug above was fixed (`cp -rL /sys/firmware/devicetree/base ...`
instead of a plain `tar` on the symlink) and a second dump
(`xun_dump2.tar`, 15 MB, 13,598 entries) was taken off the same unit,
this time with the real, complete device tree. This resolves most of what
the first dump above left open:

- **Resolved, reversing the earlier finding: `cont_splash_region` exists.**
  A `reserved-memory` node is present at `0x5C000000`, size `0xF00000`
  (15 MiB), with `label = "cont_splash_region"` and phandle `0x347` --
  exactly the address and size already assumed in this package's
  `Include/DXE.inc` and `MemoryMapLib.c`. No Linux node references this
  phandle, which is normal: ABL paints the splash framebuffer there before
  Linux ever runs, so Linux has no reason to consume it. This had been
  flagged as the single biggest Phase 1 risk; it no longer is. The only
  remaining question is whether ABL actually leaves live pixels there on
  this specific device, which only an actual `fastboot boot` test can show.
- **Corrected:** touch IRQ/reset GPIOs are TLMM GPIO 80 (IRQ) and TLMM
  GPIO 86 (reset) -- not 478/484 as the non-rooted upstream reading had it.
  Read directly from the `touch@0` node's `focaltech,irq-gpio` and
  `focaltech,reset-gpio` properties. The node also confirmed the touch bus
  is `qupv3_0_geni_se@4ac0000/spi@4a80000` (i.e. `4a80000.spi`), and
  `focaltech,display-coords` = `[0, 0, 11999, 19199]`, consistent with the
  1200x1920 panel at 10x oversampling.
- **New finding, not covered by either upstream or the first dump:
  volume-up key is on the PM6125 PMIC's SPMI GPIO block, not TLMM.** The
  `gpio_keys/vol_up` node sits under
  `qcom,spmi@1c40000/qcom,pm6125@0/pinctrl@c000`, GPIO 5 -- a completely
  different access mechanism (SPMI PMIC arbiter transactions) from the flat
  TLMM MMIO read this package's `XunKeyInputDxe`/`XunKeys.h` (ported from
  tb128fu, which does use TLMM GPIO 5 for the same key) currently performs.
  The matching GPIO number on both devices is coincidental. This means the
  volume-up-as-Enter key does not currently work on xun; see
  `Platforms/Xiaomi/xunPkg/PORTING.md` item 4. Deferred to Phase 5 --
  doesn't block Phase 1 (menu rendering).
- Panel `m84_42_03_0c` additionally confirmed directly from the device
  tree's active panel node (the other two known variants,
  `m84_35_03_0b` and `m84_36_02_0a`, are present as inactive nodes in the
  same tree, consistent with one shared kernel build across panel SKUs).
