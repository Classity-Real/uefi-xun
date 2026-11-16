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
