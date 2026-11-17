  INF MdeModulePkg/Core/Dxe/DxeMain.inf
  INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

  INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf

  INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  INF MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  INF ArmPkg/Drivers/ArmGicDxe/ArmGicDxe.inf
  INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf

  INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf

  # The boot image's ramdisk (the UniXNU payload volume) as a read-only block
  # device. Every range comes from the device tree the previous stage hands
  # over, so the shengPkg driver serves this board unchanged.
  INF shengPkg/Drivers/ShengBootMemoryDxe/ShengBootMemoryDxe.inf
  # The XNU loader is part of the firmware: a driver loaded from a volume is
  # refused by the image verification of this build.
  INF DarwinBootPkg/Driver/DarwinBoot.inf

  INF MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  INF FatPkg/EnhancedFatDxe/Fat.inf
  INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  # UNVERIFIED for xun (see PORTING.md): on tb128fu, ABL leaves the hx83102p
  # panel scanning out cont_splash_region and this driver just publishes that
  # buffer as the GOP. A direct device-tree read off a real xun unit,
  # 2026-11-17, found a reserved-memory node at this exact address, 15 MiB,
  # labeled "cont_splash_region" -- the standard Qualcomm name for exactly
  # this purpose, and nothing Linux-side needs to reference it for ABL to
  # still paint live pixels there before handoff (Linux has no consumer of
  # it either on tb128fu, where this driver is known to work). This reverses
  # an earlier reading that found no such region -- see PORTING.md. Still
  # not proven live without an actual fastboot boot test.
  INF SiliciumPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf

  # Volume up as Enter for the UniXNU menu. NOT YET WORKING for xun: ported
  # from tb128fuPkg's Tb128fuKeyInputDxe, which reads TLMM GPIO 5 with a
  # flat MMIO access. xun's own device tree instead puts volume up on the
  # PM6125 PMIC's own GPIO 5, reached over the SPMI PMIC arbiter, not TLMM
  # at all -- confirmed 2026-11-17. See XunKeys.h for the fix needed
  # (Phase 5, not a Phase 1 blocker: the menu still renders without it).
  INF xunPkg/Drivers/XunKeyInputDxe/XunKeyInputDxe.inf

  INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

  INF EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
  INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
