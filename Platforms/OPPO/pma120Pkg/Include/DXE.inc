  INF MdeModulePkg/Core/Dxe/DxeMain.inf
  INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

  INF Binaries/pma120/QcomPkg/Drivers/EnvDxe/EnvDxeEnhanced.inf

  INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf

  INF Binaries/pma120/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
  INF Binaries/pma120/QcomPkg/Drivers/TzDxe/ScmDxeCompat.inf
  INF Binaries/pma120/QcomPkg/Drivers/TzDxe/TzDxeLA.inf

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

  INF Binaries/pma120/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf 
  INF Binaries/pma120/QcomPkg/Drivers/HWIODxe/HWIODxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/QupDxe/QupDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/GpiDxe/GpiDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/I2CDxe/I2CDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/I3CMasterDxe/I3CMasterDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/SPIDxe/SPIDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/PmicDxe/PmicDxeLa.inf

  INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf

  INF Binaries/pma120/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  INF Binaries/pma120/MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf

  INF FatPkg/EnhancedFatDxe/Fat.inf
  INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  INF Binaries/pma120/QcomPkg/Drivers/UFSDxe/UFSDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/PwrUtilsDxe/PwrUtilsDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/MailboxDxe/MailboxDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/QcomScmiDxe/QcomScmiDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/VcsDxe/VcsDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/ClockDxe/ClockDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/ICBDxe/ICBDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
  INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

  INF Binaries/pma120/QcomPkg/Drivers/RscDxe/RscDxe.inf

  INF EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
  INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

!if $(USE_CUSTOM_DISPLAY_DRIVER) == 1
  INF Binaries/pma120/QcomPkg/Drivers/CPRDxe/CPRDxe.inf
!endif
  INF Binaries/pma120/QcomPkg/Drivers/SdccDxe/SdccDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/FeatureEnablerDxe/FeatureEnablerDxe.inf

!if $(USE_CUSTOM_DISPLAY_DRIVER) == 1
  INF Binaries/pma120/QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf
!else
  INF SiliciumPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf
!endif

  INF Binaries/pma120/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/OcdtDxe/OplusProject.inf
  INF Binaries/pma120/QcomPkg/Drivers/OplusExGpio/TLMMExDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/IPCCDxe/IPCCDxe.inf
  #INF Binaries/pma120/QcomPkg/Drivers/GLinkDxe/GLinkDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/PmicGlinkDxe/PmicGlinkSoccpDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLAGio.inf
  INF Binaries/pma120/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/TsensDxe/TsensDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/XhciDxe/XhciDxe.inf

  INF MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  INF MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  INF MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf

  INF Binaries/pma120/QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/UCDxe/UCDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/RNGDxe/RngDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/PILDxe/PILDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
  INF Binaries/pma120/QcomPkg/Drivers/SPSSDxe/SPSSDxe.inf
