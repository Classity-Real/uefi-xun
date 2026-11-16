// Numeric configuration map of the SM6225 (bengal) stock UEFI, seeded from
// tb128fuPkg (Lenovo Xiaoxin Pad 2022, same SoC family).
//
// The purely SoC-level entries (NumCpus, MaxLogFileSize, Sdc*GpioConfig*,
// etc.) are fogPkg's generic SM6225 numbers and should hold for xun as-is.
//
// EnableUfsIOC is 0 here, matching xun's own requirement (it boots from
// eMMC via 4784000.sdhci, not UFS) -- this was already tb128fu's value, not
// a change made for xun.
//
// NOT YET VERIFIED FOR XUN: SharedIMEMBaseAddr (0x0c125000), DloadCookieAddr
// (0x003d3000) and TzAppsRegnAddr/Size (0x61800000/0x02100000) were
// confirmed against the live TB128FU device tree, not xun's. These are
// board-specific addresses (IMEM cookie location, qseecom TZ app region)
// that must be re-checked against xun's own device tree/live memory before
// Phase 1 is trusted -- see PORTING.md, "Unverified".
#include <Library/ConfigurationMapLib.h>

STATIC
EFI_CONFIGURATION_ENTRY_DESCRIPTOR
gConfigurationDescriptor[] = {
  // Configuration Map
  {"NumCpusFuseAddr", 0x5C04C},
  {"EnableShell", 0x1},
  {"SharedIMEMBaseAddr", 0x0C125000},
  {"DloadCookieAddr", 0x003D3000},
  {"DloadCookieValue", 0x10},
  {"NumCpus", 8},
  {"NumActiveCores", 8},
  {"MaxLogFileSize", 0x400000},
  {"UefiMemUseThreshold", 0x77},
  {"USBHS1_Config", 0x0},
  {"UsbFnIoRevNum", 0x00010001},
  {"PwrBtnShutdownFlag", 0x0},
  {"Sdc1GpioConfigOn", 0x1E92},
  {"Sdc2GpioConfigOn", 0x1E92},
  {"Sdc1GpioConfigOff", 0xA00},
  {"Sdc2GpioConfigOff", 0xA00},
  {"EnableSDHCSwitch", 0x1},
  {"EnableUfsIOC", 0},
  {"UfsSmmuConfigForOtherBootDev", 1},
  {"SecurityFlag", 0xC4},
  {"TzAppsRegnAddr", 0x61800000},
  {"TzAppsRegnSize", 0x02100000},
  {"TzAppsRegnSizeLowRAM", 0xB00000},
  {"EnableLogFsSyncInRetail", 0x0},
  {"ShmBridgememSize", 0xA00000},
  {"EnableMultiThreading", 1},
  {"MaxCoreCnt", 8},
  {"EarlyInitCoreCnt", 1},
  {"EnableDisplayThread", 1},
  {"EnableUefiSecAppDebugLogDump", 0x1},
  {"AllowNonPersistentVarsInRetail", 0x1},
  {"MinidumpTALoadingCfg", 0x0}
};

VOID
GetConfigurationMap (
  OUT EFI_CONFIGURATION_ENTRY_DESCRIPTOR **ConfigurationDescriptor,
  OUT UINT8                               *ConfigurationDescriptorCount)
{
  // Pass Data
  *ConfigurationDescriptor      = gConfigurationDescriptor;
  *ConfigurationDescriptorCount = ARRAY_SIZE (gConfigurationDescriptor);
}
