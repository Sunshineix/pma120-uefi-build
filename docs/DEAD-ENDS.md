# 已证实的死路清单（永久排除，不再重走）

> 2026-11-09 整理。每条都有实测证据。禁止重走。

## A. 载荷/地址类

| 死路 | 证据 |
|------|------|
| 载荷放在 0xF0000000 | GBL 和固件都未映射该地址 → 写即崩（蓝色后黑屏）|
| 固件加 4GB 以上 DRAM 区（DRAM_HI 12GB/256MB）| 3 次实测必崩（绿→2行→黑）|
| "Kernel" 区域改 Conv | 1 次实测崩 + 与 v4 逐行 diff 定位 |

## B. 显示类

| 死路 | 证据 |
|------|------|
| SimpleFbDxe 运行 | 在本机崩溃（显示优先版绿→4行→黑）|
| XnuDxe 内装 GOP | 崩溃（绿→黑）|
| 依赖屏幕读内核日志 | FB serial lib 是环形缓冲，长输出会绕回顶部，不可靠 |

## C. 派发/APRIORI 类

| 死路 | 证据 |
|------|------|
| APRIORI 的 `!if/!else` 出现空块 | 破坏 APRIORI 表 → 早崩（GOP 版绿→3行→黑）|
| 把 UFSDxe 留在派发 | 06:03 实测挂死在 UFSDxe |
| 把 OplusProject 留在派发 | 06:16 实测 NULL+8 崩溃（ELR=0xC1144B50, FAR=0x8）|
| 把 TLMMExDxe 留在派发 | 06:30 实测 DxeCore 未定义指令崩溃（ELR=0xC13CA000）|
| 把 PIL/SPSS/FeatureEnabler 留在派发 | 06:42 前实测级联崩溃（存储依赖）|
| **PCD FALSE（跳过 XBL 扫描）** | EnvDxeEnhanced ASSERT_RETURN_ERROR (Not Found) 停机 |
| **扫描彻底废掉（返回 0）** | 缺 DtbExtension HOB → 同上 EnvDxe assert |
| 完整原版扫描（无边界）| PlatformPeim 逐指令搜 RET 无界 → 偶发死循环（10:43）|

## D. USB/loader 类

| 死路 | 证据 |
|------|------|
| 移除 UsbConfigDxe | loader 的 init 事件无 listener → usb_start 失败（19:05 橙色）|
| 加 UsbInitDxe | 其事件处理器覆盖 loader 描述符 → 总线残留 vendor 配置 |
| loader 不发 init 事件（nosignal）| start_ex 失败（18:42/18:30 无紫屏，48 秒回 Android）|
| 期待 loader usb 模式失败后自动重启可观测 | 会重启回 Android，电话状态丢失；已改为橙色保持存活 |

## E. 环境/理论类

| 死路 | 证据 |
|------|------|
| "电池/过热导致崩溃" | 电池 100%、温度 35-42°C 时照样崩 |
| "冷关机修复 UFS 状态" | 14:28 冷关机后同款固件仍失败 |
| "工件不是 04:28 那套" | 逐字节 diff：仅 26 字节差异（时间戳+PE 字段）= 同一套 |

## F. 观测通道（不可靠）

| 通道 | 问题 |
|------|------|
| 屏幕小字 | 分辨率高、字体小、环形缓冲 → 只能看颜色和大意 |
| QUSB_BULK 判定固件活着 | 也可能是重启窗口的 XBL（同一 VID/PID） |
| fastboot 对半死设备 | 会一直等/超时（需带超时封装） |

## G. 保留下来的可靠通道

1. **屏幕颜色标记**（跳板红/绿、loader 白/紫/橙/绿/红、XnuDxe 青/橙/蓝/黄）——唯一稳定的机内输出
2. **USB 枚举日志**（Get-PnpDevice 快照 + fastboot devices 超时封装）
3. **48 秒回 Android 的时序**（loader 回落 reboot 的指纹）
4. **QEMU 全链路本地验证**（软件层，零手机成本）
