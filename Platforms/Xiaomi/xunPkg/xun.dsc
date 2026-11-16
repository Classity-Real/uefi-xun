##
#  Copyright (c) 2011 - 2022, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2020, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
#  Copyright (c) Microsoft Corporation.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = xun
  PLATFORM_GUID                  = 8F2E6C3D-9A4B-4E7F-B1C2-3D4E5F6A7B8C
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/xunPkg
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = RELEASE|DEBUG
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = xunPkg/xun.fdf
  USE_CUSTOM_DISPLAY_DRIVER      = 0

  #
  # 0 = SM6225
  # 1 = SM6225-AD
  #
  SOC_TYPE                       = 0

!include DivarPkg/DivarPkg.dsc.inc

[PcdsFixedAtBuild]
  #
  # DDR Memory
  #
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x40000000

  #
  # UEFI Stack
  #
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x5FF90000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x40000

  #
  # SMBIOS
  #
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemManufacturer|"Xiaomi"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Redmi Pad SE"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"xun"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"XUN"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemBoardModel|"XUN"

  #
  # Simple Frame Buffer
  #
  # UNVERIFIED (see PORTING.md): unlike tb128fu, xun's ABL does not appear
  # to leave a cont_splash_region, so there may be nothing live for
  # SimpleFbDxe to scan out at this address yet. The panel (HUAXING
  # m84_42_03_0c) is native 1200x1920 portrait -- matches the FT8203 touch
  # resolution reported in docs/DEVICE.md. Do NOT reuse tb128fu's 1200x2000.
  #
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferWidth|1200
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferHeight|1920
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferColorDepth|32
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferFontScale|2
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferDelay|20000

  #
  # Platform PEI
  #
  gQcomPkgTokenSpaceGuid.PcdPlatformType|"LA"
  # Carried over from tb128fu: assumes the xun ABL is also a thin loader
  # with no stock DXE scheduler HOBs for PlatformPeim to consume. Not yet
  # independently confirmed for xun's ABL -- see PORTING.md, "Unverified".
  gQcomPkgTokenSpaceGuid.PcdBuildXblHobs|FALSE

[LibraryClasses]
  #
  # Memory Libraries
  #
  MemoryMapLib|xunPkg/Library/MemoryMapLib/MemoryMapLib.inf

  #
  # QCOM Libraries
  #
  ConfigurationMapLib|xunPkg/Library/ConfigurationMapLib/ConfigurationMapLib.inf

[Components]
  #
  # UniXNU: the boot manager, its registrar, the ramdisk block device and the XNU loader
  #
  UniXNUPkg/Application/UniXNU/UniXNU.inf
  UniXNUPkg/Driver/UniXNUDxe/UniXNUDxe.inf
  shengPkg/Drivers/ShengBootMemoryDxe/ShengBootMemoryDxe.inf
  DarwinBootPkg/Driver/DarwinBoot.inf
  xunPkg/Drivers/XunKeyInputDxe/XunKeyInputDxe.inf

[LibraryClasses]
  UniXNULib|UniXNUPkg/Library/UniXNULib/UniXNULib.inf
  FdtLib|MdePkg/Library/BaseFdtLib/BaseFdtLib.inf
