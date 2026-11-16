// Memory map of the Xiaomi/Redmi Pad SE (xun, SM6225 bengal, Khaje IDP).
//
// Unlike tb128fuPkg (whose memory map this file was seeded from), these
// addresses come from a live readout of a real xun unit on 2026-10-31
// (see docs/DEVICE.md), not from a device tree dump. Two load-bearing facts
// from that readout:
//
//   * DRAM is three banks with a hole between them, not tb128fu's two banks:
//       Bank 0: 0x40000000, 0x3CC00000 (ends 0x7CC00000)
//       Bank 1: 0x80000000, 0x40000000 (ends 0xC0000000)
//       Bank 2: 0xC0000000, 0x100000000 (ends 0x1C0000000)
//     total visible to the kernel is ~6092 MiB. Do NOT reuse tb128fu's
//     dram_size/bank layout.
//
//   * The splash/dfps/ramoops/removed reservations occupy 0x5C000000 to
//     0x63900000 -- the SAME footprint tb128fu uses for its display/DXE-heap/
//     FD/removed block. The free gap in the middle, 0x5D200000..0x60000000,
//     is the same shape as tb128fu's, so the FD placement at
//     0x5FC00000 (3 MiB) is carried over unchanged. This part of the map
//     really is SoC/ABL-common and safe to copy.
//
// What is NOT safe to copy, and is the biggest open risk in this file:
// xun's ABL does NOT appear to leave a cont_splash_region the way tb128fu's
// does. tb128fuPkg's SimpleFbDxe strategy (scan out whatever ABL left
// running in the splash buffer) has nothing to attach to here. Until this is
// re-checked against the device tree xun actually boots with, treat the
// "Display Reserved" / "DFPS Data" regions below as reserved address space
// only -- NOT as a confirmed live framebuffer. See PORTING.md, "Unverified".
#include <Library/MemoryMapLib.h>

STATIC
EFI_MEMORY_REGION_DESCRIPTOR
gMemoryDescriptor[] = {
  // Name, Address, Length, HobOption, ResourceType, ResourceAttribute, MemoryType, ArmAttribute

  // DDR Bank 0: 0x40000000 .. 0x7CC00000
  {"Kernel",             0x40000000, 0x05700000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"HYP",                0x45700000, 0x00600000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"Boot Info",          0x45D00000, 0x00020000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"XBL AOP",            0x45E00000, 0x00140000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"Sec Apps",           0x45FFF000, 0x00001000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"SMEM",               0x46000000, 0x00200000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"CDSP Secure",        0x46200000, 0x01E00000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"PIL Reserved",       0x4AB00000, 0x00AB7000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"DRAM 0x48000000",    0x48000000, 0x02B00000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  {"DRAM 0x55617000",    0x55617000, 0x069E9000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},

  // Splash / ramoops / removed block, carried over from tb128fu -- verified
  // present at this address range on the real xun unit, but NOT confirmed
  // to be a live cont_splash_region. See file header and PORTING.md.
  {"Splash Reserved",    0x5C000000, 0x00F00000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_THROUGH_XN},
  {"Ramoops",            0x5D000000, 0x00200000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_THROUGH_XN},
  {"DXE Heap",           0x5D200000, 0x02A00000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},
  {"UEFI FD",            0x5FC00000, 0x00300000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
  {"SEC Heap",           0x5FF00000, 0x0008C000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"MMU PageTables",     0x5FF8D000, 0x00003000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"UEFI Stack",         0x5FF90000, 0x00040000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"Log Buffer",         0x5FFF7000, 0x00008000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
  {"Info Blk",           0x5FFFF000, 0x00001000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
  {"Removed Region",     0x60000000, 0x03900000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"DRAM 0x63900000",    0x63900000, 0x19300000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},

  // End of Bank 0 at 0x7CC00000. There is a real hole here on xun (no RAM
  // 0x7CC00000..0x80000000) -- unlike tb128fu, which has no gap at this
  // address. Do not add a region spanning across it.

  // DDR Bank 1: 0x80000000 .. 0xC0000000. Reserve the boot image load
  // window the way tb128fu does (kernel+DTB, then ramdisk) until the actual
  // xun boot header v4 load addresses are confirmed -- see PORTING.md,
  // "Unverified". These offsets are inherited placeholders, not measured.
  {"Boot Image",         0x80000000, 0x01000000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"Ramdisk",            0x81000000, 0x03800000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"DRAM 0x84800000",    0x84800000, 0x3B800000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,   WRITE_BACK_XN},

  // DDR Bank 2: 0xC0000000 .. 0x1C0000000, all conventional.
  {"DRAM 0xC0000000",    0xC0000000, 0x100000000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv,  WRITE_BACK_XN},

  // Other Memory Regions
  {"IMEM Base",          0x0C100000, 0x00026000, NoHob,  MMAP_IO, INITIALIZED, Conv, NS_DEVICE},
  {"IMEM Cookie Base",   0x0C125000, 0x00001000, AddDev, MMAP_IO, INITIALIZED, Conv, NS_DEVICE},
  {"RPM_SS_MSG_RAM",     0x045F0000, 0x00007000, NoHob,  MMAP_IO, INITIALIZED, Conv, NS_DEVICE},

  // Register Regions -- SM6225/bengal is SoC-fixed, carried over from
  // tb128fu unchanged. These are not board-specific.
  {"TCSR_TCSR_REGS",     0x003C0000, 0x00040000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_NORTH",         0x00400000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_WEST",          0x00500000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_SOUTH",         0x00900000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_EAST",          0x00D00000, 0x00300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"GCC CLK CTL",        0x01400000, 0x00200000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"USB2",               0x01610000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"MMCX_CPR3",          0x01648000, 0x00008000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"CRYPTO0 CRYPTO",     0x01B00000, 0x00040000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"SECURITY CONTROL",   0x01B40000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"PRNG_CFG_PRNG",      0x01B50000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"PMIC ARB SPMI",      0x01C00000, 0x02800000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"SLP_CNTR",           0x04403000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"PSHOLD",             0x0440B000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TSENS0",             0x04410000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TSENS0_TM",          0x04411000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"MCCC_MCCC_MSTR",     0x0447D000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  // xun boots from eMMC (4784000.sdhci), NOT UFS. The UFS register window is
  // kept mapped (it does no harm) but no UFS driver is wired up -- Phase 2
  // needs an eMMC/SDHCI block driver here instead of tb128fu's NativeUfs path.
  {"UFS UFS REGS",       0x04800000, 0x00020000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"SDHCI eMMC",         0x04784000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"QUPV3_0_GSI",        0x04A00000, 0x000D0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  // Touch is FocalTech FT8203 over SPI. QUPV3_0_GSI above covers the QUP SPI
  // block; the specific SPI chip-select for touch (board docs call it both
  // "spi0.0" and "4a80000.spi" in different sources -- needs to be confirmed
  // from the live device tree, not assumed) is a Phase 3 item, not mapped here.
  {"USB30_PRIM",         0x04E00000, 0x00200000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"MDSS",               0x05E00000, 0x00200000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"SMMU",               0x0C600000, 0x00080000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"APSS_WDT_TMR1",      0x0F017000, 0x00001000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"QTIMER",             0x0F020000, 0x00110000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"APSS_GIC500_GICD",   0x0F200000, 0x00010000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"APSS_GIC500_GICR",   0x0F300000, 0x00100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"APSS_ACTPM_WRAP",    0x0F500000, 0x000B0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE}
};

VOID
GetMemoryMap (
  OUT EFI_MEMORY_REGION_DESCRIPTOR **MemoryDescriptor,
  OUT UINT8                         *MemoryDescriptorCount)
{
  // Pass Data
  *MemoryDescriptor      = gMemoryDescriptor;
  *MemoryDescriptorCount = ARRAY_SIZE (gMemoryDescriptor);
}
