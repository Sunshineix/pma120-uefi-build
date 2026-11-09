# 下一步操作指南（给下一个对话）——2026-11-09 20:05

> 状态：**一个已编译好的载体在等测试**，直接开始，不需要先构建任何东西。

## 一、当前就绪的东西

| 项 | 位置 / 值 |
|----|----------|
| **测试载体** | `C:\temp\carrier-final\gbl-verify2\BootstrapVerify2.efi`（3,221,131 B，已字节验证：APRIORI11 固件 + 你的 1 秒延迟 loader）|
| 固件 FD（仓库已同步）| `C:\temp\final2-fd\pma120-uefi\SILICIUM_UEFI.fd`（sha256 623318d9…）|
| 延迟修复载荷（仓库已同步）| `C:\temp\usb-delay-payload.bin`（loader 含 `BS->stall(1000000)`）|
| 编译记录 | loader build 42613174921（success）；固件 build 42609565783（success）|
| 监听脚本 | `C:\temp\kernel_hello3.ps1` |
| 手机状态 | 停在 loader/复位中，需长按电源 12 秒再进 GBL |

## 二、下一步操作（逐条）

1. **确认手机在 Android 或亮屏**；若黑屏卡死：长按电源 12 秒复位。
2. **启动监听**（后台）：
   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File C:\temp\kernel_hello3.ps1
   ```
3. **请用户操作手机**：重启 → 震动时按住音量上键进 GBL → 松开 → 跑完**先发视频**。
4. **判断（按用户视频帧 + 监听日志）**：

| 观察 | 含义 | 下一步 |
|------|------|--------|
| `fastboot devices` 出现 **xnuloader**（18D1:D00D）| 🏆 USB 枚举修好 | 立刻 stage SMP 修复内核 + hello 载荷 → `oem run` → 等 hello 输出 |
| 品红 + 总线仍是 QUSB_BULK | 1 秒不够或 stop() 无效 | 加大间隔（2-3 秒）或直接操作 DWC3 寄存器（DCTL/USBCMD discon）|
| 橙色 | usb_start 失败 | 对照 04:28 的 loader 行为，检查 init event/UsbConfigDxe |
| 灰/蓝/黄 | XnuDxe 层问题 | 按颜色代码表定位 |

5. **若 xnuloader 上线**，用它送内核（重要：这是 04:28 后第一次有机会跑真机内核）：
   ```powershell
   fastboot stage C:\temp\kernel-hello-payload.bin   # 或 SMP 修复版载荷
   fastboot oem run
   fastboot get_staged log.txt
   ```
   期望：核心里程碑 → 最后 "Hello from user space on Darwin!"

## 三、颜色代码表（当前固件）

- BootstrapVerify2：红（入口）/ 黄（重叠）/ 品红（校验失败）/ **绿（校验通过，跳转）**
- loader：**白（载荷解析）** / **品红（usb_start 成功）** / **橙（usb_start 失败，保持存活）** / 绿（进内核）/ 红（重启）
- Pma120XnuDxe：**青（入口）** / 灰（载荷缺失）/ 蓝（LoadImage 失败）/ 黄（loader 返回）

## 四、四条工作规矩（用户要求，必须遵守）

1. 每步先静态分析/历史对比，再动机器。
2. 以历史最佳为基线，只修当前实测的那一个点，不删验证过的东西。
3. **每次测试先发视频**，读帧后再决定构建/测试。
4. 每轮结束报告一次进展与问题。

## 五、GitHub 备份（全部已推送）

- `Sunshineix/pma120-uefi-build`：probe/SILICIUM_UEFI.fd（APRIORI11）、probe/xnu-payload.bin（延迟 loader）、docs/（方法、死路、交接、报告、进度图、脚本）
- `Sunshineix/hakios-xnu-kernel`：xnuloader/loader.c（含用户 9aebd29b 延迟修复）

## 六、若 USB 枚举仍不行——确定的备选路线（按优先级）

1. 把 stall 从 1 秒加大到 2-3 秒（一行改动）
2. 直接写 DWC3 寄存器：`DCTL` 的 run-stop、`USBCMD` 等，强制 controller 断开→连接（loader 里加 native DWC3 访问；DWC3 base 可从 UsbfnDwc3Dxe 或 DT 取）
3. 读 vendor 开源 ABL fastboot 源码（`QcomModulePkg/Application/Fastboot`）照抄其 USB 启动序列
4. 在 GBL 的 fastboot 会话里保留 USB（`oem boot-efi` 前不 stop）——研究 GBL 参数是否支持
