# uefi-xun

UEFI (Mu-Silicium/EDK2) platform package for the **Xiaomi/Redmi Pad SE**
(codename **xun**, Qualcomm SM6225 "bengal"), aimed at the
[UniXNU](https://unixnu.com) boot chain -- the same approach that has
already gotten a different SM6225 tablet, the Lenovo Xiaoxin Pad 2022
(**tb128fu**), to the iPadOS 17.7.11 home screen.

> **Status: planning / scaffolding.** Nothing in this repository has booted
> on a real xun unit yet. See `Platforms/Xiaomi/xunPkg/PORTING.md` for the
> full list of what's verified, what's inherited-but-unverified, and the
> open risks that need resolving before Phase 1 below can even be tried.

## What this is (and isn't)

This is an **overlay**, not a standalone buildable tree. The actual
Mu-Silicium codebase -- EDK2 core, the Qualcomm/Silicium silicon packages,
UniXNU itself -- lives in
[`xVanTuring/uefi-tb128fu`](https://github.com/xVanTuring/uefi-tb128fu) (a
large multi-vendor, multi-device monorepo). This repo holds only the parts
specific to xun:

```
Platforms/Xiaomi/xunPkg/   -- the platform package (DSC, FDF, drivers, libs)
Resources/Configs/xun.toml -- FD/boot-image config for build_uefi.py
Resources/DTBs/            -- device tree (not yet present, see below)
docs/DEVICE.md             -- reference notes on the real-device readout this was built from
```

To build it, copy (or let the CI workflow copy) `Platforms/Xiaomi/xunPkg`
and `Resources/Configs/xun.toml` into a checkout of `uefi-tb128fu`, then run
its `build_uefi.py -d xun`. This mirrors how
[`KernelZero123/uefi-xun`](https://github.com/KernelZero123/uefi-xun) (an
earlier, unfinished attempt at the same thing) is structured, and how
`UniXNU-tb128fu`/`uefi-tb128fu` relate to each other upstream.

## Why xun, and what's reused vs. rebuilt

xun is SM6225 "bengal" -- the same SoC family as tb128fu -- so the SoC-level
boot chain, Adreno 610 GPU, and generic configuration values transfer
directly. What does **not** transfer is anything board-specific: the panel,
the touch controller, and critically, the storage medium (xun is eMMC,
tb128fu is UFS) and the boot image format (xun uses header v4 with a split
boot/init_boot/vendor_boot, tb128fu uses header v1). Full comparison table
in `PORTING.md`.

## Phased plan

Each phase is meant to be independently testable via `fastboot boot` before
moving to the next.

1. **Menu + splash only.** Get the UEFI menu to appear on screen at all,
   with no storage driver. **Primary open risk:** xun's ABL does not appear
   to leave a live `cont_splash_region` the way tb128fu's does -- see
   `PORTING.md`, "Unverified", item 1. Secondary open risk: whether
   `fastboot boot` even works the same way against xun's header v4 boot
   image -- item 2.
2. **eMMC block I/O.** Replace tb128fu's UFS path with one that talks to
   the `4784000.sdhci` controller (`mmcblk0`), using the real partition
   layout already read off a device (`docs/DEVICE.md`).
3. **Correct framebuffer geometry.** 1200x1920 portrait native (not
   tb128fu's 1200x2000), and confirm which of the three known panel
   variants is actually on hand.
4. **Touch.** Port a FocalTech FT8203 (SPI) driver -- tb128fu's Himax
   HX83102E driver does not apply.
5. **GPU.** Bring up Adreno 610, structurally similar to tb128fu, but with
   xun's own SMMU windows and zap firmware.
6. **Polish / OS bring-up.** Buttons, fuel gauge, sensors, then attempt the
   actual iPadOS boot chain once 1-5 are stable.

This repository currently covers the groundwork for Phase 0/1: the
`xunPkg` platform package skeleton, adapted memory map, and build scaffolding.
No phase has been validated against real hardware yet.

## Building (once the open risks above are resolved)

```sh
git clone https://github.com/xVanTuring/uefi-tb128fu.git
cd uefi-tb128fu
git submodule update --init --depth 1 -- Binaries Common/Mu Common/Mu_OEM_Sample Silicium-ACPI Silicon/Silicium/OpensslPkg/Library/OpensslLib/openssl

cp -r ../uefi-xun/Platforms/Xiaomi/xunPkg Platforms/Xiaomi/xunPkg
cp ../uefi-xun/Resources/Configs/xun.toml Resources/Configs/xun.toml
# Resources/DTBs/xun.dtb is not yet present -- a real device-tree dump is
# needed here before this will produce anything that boots.

./setup_env.sh --package-manager apt
./build_uefi.py -d xun -r DEBUG
```

The GitHub Actions workflow in `.github/workflows/build.yml` does the same
thing (manual trigger only, `workflow_dispatch`) and uploads whatever `.fd`
and build logs come out, successful or not -- useful for seeing exactly
where the build currently fails.

## Credit

- [`xVanTuring/UniXNU-tb128fu`](https://github.com/xVanTuring/UniXNU-tb128fu) /
  [`xVanTuring/uefi-tb128fu`](https://github.com/xVanTuring/uefi-tb128fu) --
  the working reference this was seeded from.
- [`KernelZero123/UniXNU-xun`](https://github.com/KernelZero123/UniXNU-xun) /
  [`KernelZero123/uefi-xun`](https://github.com/KernelZero123/uefi-xun) --
  the first attempt at xun specifically, and the source of the real-device
  readout this repo's memory map and config are built from.
- [UniXNU](https://unixnu.com) and [Project Silicium](https://github.com/Project-Silicium) --
  the underlying boot-manager and Mu-Silicium projects.
