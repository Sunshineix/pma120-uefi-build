# 工作方法（2026-11-09 确立，用户要求）

## 一、铁律

1. **以历史最佳状态为基线**——每次修改都从"跑得最远/最成功"的那个配置出发，不推翻已验证模块。
2. **只修当前实测到的下一个失败点**——一次一个变量，改前记录基线，改后对比。
3. **不删除验证过的东西**——要移除必须有两个条件：① 它确实是当前失败点；② 移除后派发距离变长（有屏幕/USB 证据）。
4. **保留全部成功历史数据**——每个版本的 FD/载荷/实测结果都留档（下表）。

## 二、实测历史（派发距离 = 唯一客观指标）

| 时间 | 固件基线 | 实测派发到达 | 结果 |
|------|---------|-------------|------|
| 04:28 | 原始 v4 | **全程 → loader → xnuloader ✓** | 🥇 金标准（唯一成功）|
| 06:42 | APRIORI4 | **全程 → 粉色（loader usb 成功）** | 🥈 |
| 18:42 | APRIORI8 | **UsbDeviceDxe(93)**（历史最深派发）| 🥉 |
| 19:05 | APRIORI9+信号loader | **橙色=loader 运行，usb_start 失败** | 里程碑：loader 能跑 |
| 18:16 | 边界扫描 | PmicDxe→DiskIoDxe→Sdcc 死 | 边界扫描修复 EnvDxe ✓ |
| 18:30 | APRIORI4 回退 | UsbPwrCtrlDxe(81) 死 | — |
| 14:05 | 无扫描 | EnvDxe assert | 需 DtbExtension HOB |
| 10:43 | 原扫描 | PlatformPeim 死循环 | 需边界 |

## 三、当前基线（合并最优，构建 42606251199）

- **APRIORI8 的移除集**（唯一把派发推到 93 的）：UFSDxe/OplusProject/TLMMEx/FeatureEnabler/PILProxy/PIL/SPSS/GLink/QcomCharger/ChargerEx/Xhci×2/UsbBus/UsbKb/UsbMassStorage 移除；**UsbConfigDxe 保留**（04:28 USB 链的一环）
- **边界扫描**（EnvDxe 修复，18:16 验证）
- **信号 loader**（04:28 验证 + 失败橙色保持存活）
- **Pma120Diag 移除**（挡现场）
- XnuDxe 颜色标记（青/橙/蓝/黄）

## 四、下一个未知（只测这一个）

**loader 的 usb_start 是否成功**：紫色=成功→查 xnuloader；橙色=失败→查 loader 日志（logdump 分区）或 USB 描述符。

## 五、工件留档位置

| 工件 | 路径 |
|------|------|
| 各版本 FD | C:\temp\{smmu,noufs,nooplus,mark,syn,noxbl,usbfix,ap8,ap9,ap10}-fd |
| 载荷 | C:\temp\{usb-payload-final,usb-nosignal-payload,usb-final-payload}.bin |
| 载体 | C:\temp\carrier-final\gbl-verify2\BootstrapVerify2.efi（当前基线）|
| QEMU 验证 | C:\temp\qemu\qemu-boot.log（软件链全通）|
