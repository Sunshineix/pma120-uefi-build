# -*- coding: utf-8 -*-
"""Build the PMA120 iOS port progress report."""
from docx import Document
from docx.shared import Pt, RGBColor, Cm
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

OUT = r"C:\Users\1\Documents\deepseek-harness\default-workspace\X9U-iOS移植\PMA120-iOS移植进度报告-20261109.docx"

doc = Document()

# Base font
style = doc.styles["Normal"]
style.font.name = "Microsoft YaHei"
style.font.size = Pt(10.5)
style.element.rPr.rFonts.set(qn("w:eastAsia"), "Microsoft YaHei")


def set_cn_font(run, name="Microsoft YaHei", size=None, bold=None, color=None):
    run.font.name = name
    run._element.rPr.rFonts.set(qn("w:eastAsia"), name)
    if size is not None:
        run.font.size = size
    if bold is not None:
        run.font.bold = bold
    if color is not None:
        run.font.color.rgb = color


def shade(cell, hexcolor):
    tcPr = cell._tc.get_or_add_tcPr()
    sh = OxmlElement("w:shd")
    sh.set(qn("w:val"), "clear")
    sh.set(qn("w:color"), "auto")
    sh.set(qn("w:fill"), hexcolor)
    tcPr.append(sh)


def add_table(headers, rows, widths=None):
    t = doc.add_table(rows=1, cols=len(headers))
    t.style = "Table Grid"
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    for i, h in enumerate(headers):
        c = t.rows[0].cells[i]
        c.text = ""
        p = c.paragraphs[0]
        r = p.add_run(h)
        set_cn_font(r, size=Pt(9.5), bold=True, color=RGBColor(0xFF, 0xFF, 0xFF))
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        shade(c, "2F5597")
    for row in rows:
        cells = t.add_row().cells
        for i, v in enumerate(row):
            cells[i].text = ""
            p = cells[i].paragraphs[0]
            r = p.add_run(v)
            set_cn_font(r, size=Pt(9))
    if widths:
        for i, w in enumerate(widths):
            for row in t.rows:
                row.cells[i].width = Cm(w)
    return t


TITLE_COLOR = RGBColor(0x1F, 0x3B, 0x73)
GREEN = RGBColor(0x1E, 0x7B, 0x34)
ORANGE = RGBColor(0xB4, 0x5F, 0x06)
GRAY = RGBColor(0x60, 0x60, 0x60)

# ---------------- Title ----------------
p = doc.add_paragraph()
p.alignment = WD_ALIGN_PARAGRAPH.CENTER
r = p.add_run("OPPO Find X9 Ultra（PMA120）iOS 移植进度报告")
set_cn_font(r, size=Pt(18), bold=True, color=TITLE_COLOR)

p = doc.add_paragraph()
p.alignment = WD_ALIGN_PARAGRAPH.CENTER
r = p.add_run("2026-11-09 • 固件（Silicium UEFI）+ 转场服务（xnuloader）+ 内核（XNU 22.5）路线")
set_cn_font(r, size=Pt(10), color=GRAY)

# ---------------- 0. 结论摘要 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("一、结论摘要")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)

summary = [
    ("适配层（UEFI 固件 + DXE 派发 + 自定义 XnuDxe）", "100% 打通", GREEN),
    ("转场服务（xnuloader）", "95% —— 仅剩 USB 主机枚举身份", ORANGE),
    ("内核（XNU 22.5）", "60% —— 真机曾完整 BSD 初始化（04:28），SMP 修复待验证", ORANGE),
    ("用户态（mockfs/hello）", "50% —— QEMU 已跑通，真机待阶段 3 打通", ORANGE),
    ("iOS 系统层（IPSW/UFS 根盘驱动）", "5% —— IPSW 已下载，未解包", GRAY),
    ("开机（SpringBoard 桌面）", "未开始", GRAY),
]
for name, status, _ in summary:
    p = doc.add_paragraph(style="List Bullet")
    r = p.add_run(name + "：")
    set_cn_font(r, bold=True)
    r = p.add_run(status)
    set_cn_font(r, color=_, bold=True)

p = doc.add_paragraph()
r = p.add_run("一句话：")
set_cn_font(r, bold=True)
r = p.add_run(
    "固件链路已全部打通并验证；唯一的阻塞点是 loader 启动 USB 后，主机仍以固件的 vendor 身份"
    "（QUSB_BULK 05C6:900E）枚举，而不是 loader 自己的 18D1:D00D “xnuloader”。"
    "打通后即可把 SMP 修复内核送入手机跑完整启动。"
)
set_cn_font(r)

# ---------------- 1. 验证体系 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("二、当前验证体系（本报告所有结论的依据）")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)

add_table(
    ["通道", "作用", "可靠性"],
    [
        ["屏幕颜色标记", "载体/loader/XnuDxe 各阶段上色（红/绿/白/品红/青/灰/蓝/黄）", "★ 唯一稳定的机内输出"],
        ["视频逐帧读取", "用户每次录音，先发视频再决策", "★ 主判据"],
        ["USB 枚举日志", "Get-PnpDevice 快照 + fastboot 探测", "★ 主机侧实证"],
        ["QEMU 本地虚拟机", "loader+内核+hello 全链路离线验证", "★ 零手机成本"],
        ["48 秒回 Android 时序", "loader 回落 reboot 的指纹", "辅助"],
    ],
    widths=[3.2, 7.5, 4.5],
)

# ---------------- 2. 阶段进度 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("三、分阶段进度（适配 → 内核 → 开机）")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)

# 阶段 1
p = doc.add_paragraph()
p.style = "Heading 2"
r = p.add_run("阶段 1｜引导链（GBL → 载体）：✅ 100%")
set_cn_font(r, size=Pt(12.5), bold=True, color=GREEN)
add_table(
    ["步骤", "状态", "证据"],
    [
        ["进入 GBL fastboot（serial 565844a2，product canoe）", "✅", "每次测试正常"],
        ["fastboot stage 载体 + oem boot-efi", "✅", "日志 OKAY 0.01s"],
        ["BootstrapVerify2：FD→0xC6900000、载荷→0xC7800000、校验、跳转", "✅", "屏幕绿色 + 校验通过"],
    ],
    widths=[8.5, 1.6, 5.2],
)

# 阶段 2
p = doc.add_paragraph()
p.style = "Heading 2"
r = p.add_run("阶段 2｜固件适配（Silicium UEFI）：✅ 100%")
set_cn_font(r, size=Pt(12.5), bold=True, color=GREEN)
add_table(
    ["步骤", "状态", "证据 / 备注"],
    [
        ["SEC：平台初始化（SMMU 跳过，保屏幕可见）", "✅", "屏幕全程可见"],
        ["SEC：串口/帧缓冲、内存、MMU、FV 解压、加载 DxeCore", "✅", "MMU: enabled / FV decompressed (Success)"],
        ["PEI：MemoryPeim + PlatformPeim（有界扫描建 InfoBlk/DtbExtension HOB）", "✅", "EnvDxeEnhanced assert 已消除"],
        ["DXE 核心启动", "✅", "Loading DxeCore"],
        ["DXE 派发（APRIORI11，约 90 驱动）", "✅", "文字滚过全部到 USB 区（最深 18:42 至 UsbDeviceDxe=93）"],
        ["存储类驱动（UFSDxe/OplusProject/TLMMEx/FeatureEnabler/PIL/SPSS）", "✅ 已排除", "在本环境必崩，已移出派发"],
        ["Pma120XnuDxe（加载 payload 内 loader.efi）", "✅", "青色（入口）→ LoadImage 成功"],
    ],
    widths=[8.5, 1.6, 5.2],
)

# 阶段 3
p = doc.add_paragraph()
p.style = "Heading 2"
r = p.add_run("阶段 3｜转场服务（xnuloader）：🟡 95%（当前唯一卡点）")
set_cn_font(r, size=Pt(12.5), bold=True, color=ORANGE)
add_table(
    ["步骤", "状态", "证据 / 备注"],
    [
        ["loader 启动、载荷解析", "✅", "白色"],
        ["屏幕/FB 服务", "✅", "颜色标记正常"],
        ["usb_start（含 stop()+start_ex 换描述符）", "✅", "品红（返回成功）"],
        ["USB 主机枚举为 18D1:D00D “xnuloader”", "❌ 卡点", "主机只见 loader 命令循环存活 20+ 分钟不重启"],
        ["fastboot 命令通道（stage/get_staged/oem run）", "❌", "依赖上一条"],
        ["看门狗 / 存活", "✅", "loader 命令循环存活 20+ 分钟不重启"],
    ],
    widths=[8.5, 1.6, 5.2],
)
p = doc.add_paragraph()
r = p.add_run("卡点详情：")
set_cn_font(r, bold=True, color=ORANGE)
r = p.add_run(
    "loader 的 usb_start 返回成功（品红），但总线上主机仍看到固件的 vendor 身份 "
    "QUSB_BULK（VID 05C6 PID 900E，序列号 565844A2）、设备状态 Error。"
    "已尝试：不发 init 事件（失败）、发 init 事件（旧方案）、start_ex 前调用 stop()（本轮，仍不重枚举）。"
)
set_cn_font(r)

# 阶段 4
p = doc.add_paragraph()
p.style = "Heading 2"
r = p.add_run("阶段 4｜内核（XNU 22.5.0 sm8850）：🟡 60%")
set_cn_font(r, size=Pt(12.5), bold=True, color=ORANGE)
add_table(
    ["步骤", "状态", "证据 / 备注"],
    [
        ["内核裁剪与构建", "✅", "CI 通过（kernel sha d240b6ab）"],
        ["内核启动（真机）", "🟡 04:28 成功过一次", "跑到完整 BSD 初始化：pmap/VM/zones/GIC v3"],
        ["SMP 多核", "🟡 修复已构建待验证", "根因：GICR stride 0x40000（内核写死 0x20000）→ cpu1 GIC 崩；修复 run 42416035130"],
        ["内核+hello 载荷（49.8MB，窗口 128MB）", "✅ 打包就绪", "C:\\temp\\kernel-hello-payload.bin"],
        ["内核链路本地验证", "✅ QEMU 完整跑通", "打印 “Hello from user space on Darwin!”"],
    ],
    widths=[8.5, 2.6, 4.2],
)

# 阶段 5
p = doc.add_paragraph()
p.style = "Heading 2"
r = p.add_run("阶段 5｜用户态：🟡 50%")
set_cn_font(r, size=Pt(12.5), bold=True, color=ORANGE)
add_table(
    ["步骤", "状态", "证据"],
    [
        ["mockfs 挂载 hello 为 /sbin/launchd", "✅ QEMU 验证", "BSD root: md0"],
        ["用户态 hello 进程运行", "✅ QEMU / ❌ 真机", "QEMU 打印横幅；真机依赖阶段 3/4"],
    ],
    widths=[8.5, 2.6, 4.2],
)

# 阶段 6/7
p = doc.add_paragraph()
p.style = "Heading 2"
r = p.add_run("阶段 6｜iOS 系统层：⏸ 5%（未开始）")
set_cn_font(r, size=Pt(12.5), bold=True, color=GRAY)
add_table(
    ["步骤", "状态", "备注"],
    [
        ["iOS 16.5 IPSW 下载（6.16GB）", "✅", "iPhone15,2_16.5_20F66"],
        ["IPSW 解包 / 研究（dyld/内核缓存/根盘）", "⏸", "未开始"],
        ["UFS 根文件系统驱动（内核读分区）", "⏸", "内核目前无存储驱动"],
        ["128MB 载荷窗口 vs 完整内核+根盘", "⏸", "路线已记录未解决"],
    ],
    widths=[8.5, 1.6, 5.2],
)
p = doc.add_paragraph()
r = p.add_run("阶段 7｜开机（SpringBoard 桌面）：⏸ 未开始")
set_cn_font(r, size=Pt(11), bold=True, color=GRAY)

# ---------------- 4. 本轮工作记录 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("四、2026-11-09 关键工作与证据（按时间）")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)
add_table(
    ["时间", "工作", "结果"],
    [
        ["14:40", "QEMU 本地虚拟机部署并完整验证软件链", "✅ “Hello from user space on Darwin!”"],
        ["18:16", "有界扫描修复 EnvDxeEnhanced assert", "✅ 派发较之前更远"],
        ["18:42", "APRIORI 精简（移除存储依赖/优化派发）", "✅ 派发到达 UsbDeviceDxe（93），历史最深"],
        ["19:05", "loader 失败路径改为橙色保持存活（不再回落重启）", "✅ 观察到橙色=usb_start 失败"],
        ["19:15", "颜色编码修正（灰=载荷缺失 / 品红=usb 成功）", "✅ 消除混淆"],
        ["19:26", "恢复 UsbConfigDxe + 信号 loader", "✅ loader 进入命令循环存活"],
        ["19:37", "恢复全部 7 个 USB/充电驱动 + loader stop()", "✅ 品红（usb_start 成功）；❌ 主机仍为 QUSB_BULK"],
    ],
    widths=[2.2, 9.0, 4.2],
)

# ---------------- 5. 下一步 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("五、下一步计划（按优先级）")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)
steps = [
    ("P0", "对照 vendor 开源 ABL fastboot 源码，按 vendor 原生顺序改写 loader 的 USB 启动（start() vs start_ex()、DWC3 reset 时机）"),
    ("P0", "若协议 API 无法强制重枚举，直接操作 DWC3 控制器寄存器（DCTL run-stop / USBCMD）触发 disconnect→connect"),
    ("P1", "打通后立即用 loader 命令通道（oem run）送入 SMP 修复内核 + hello 载荷，真机验证 BSD 初始化与 hello 输出"),
    ("P1", "SMP 修复（GICR stride）真机确认后，进入内核崩溃诊断循环"),
    ("P2", "iOS 16.5 IPSW 解包，研究根文件系统与内核缓存，规划 UFS 驱动与 128MB 窗口方案"),
]
t = add_table(["优先级", "任务"], [[a, b] for a, b in steps], widths=[2.0, 13.4])

# ---------------- 6. 工件清单 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("六、工件清单（可直接复用）")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)
add_table(
    ["工件", "路径 / 说明"],
    [
        ["主线载体（当前测试对象）", "C:\\temp\\carrier-final\\gbl-verify2\\BootstrapVerify2.efi（3,221,131 B，三段字节验证）"],
        ["当前固件 FD", "C:\\temp\\final2-fd\\pma120-uefi\\SILICIUM_UEFI.fd"],
        ["当前载荷（stop loader）", "C:\\temp\\usb-stop-payload.bin"],
        ["04:28 原始载荷（loader 9e0f8921）", "C:\\temp\\usb-payload-final.bin"],
        ["监听脚本", "C:\\temp\\kernel_hello3.ps1（检测 GBL→stage→boot-efi→监控 USB→自动 hello）"],
        ["QEMU 验证环境", "C:\\temp\\qemu\\（qemu-boot.log 含完整 hello 输出）"],
        ["方法与死路", "X9U-iOS移植\\WORK-METHOD.md、DEAD-ENDS.md"],
        ["新会话交接", "X9U-iOS移植\\HANDOFF-20261109.md"],
    ],
    widths=[5.4, 10.0],
)

# ---------------- 7. 固定规矩 ----------------
doc.add_paragraph()
p = doc.add_paragraph()
p.style = "Heading 1"
r = p.add_run("七、协作规则（固定）")
set_cn_font(r, size=Pt(14), bold=True, color=TITLE_COLOR)
rules = [
    "每步先静态分析 / 历史对比，再动机器；",
    "以历史最佳为基线，只修当前实测的那一个点，不删除验证过的东西；",
    "每次测试先发视频，读帧后再决定构建 / 测试；",
    "每轮结束报告一次进展与问题。",
]
for x in rules:
    p = doc.add_paragraph(style="List Number")
    r = p.add_run(x)
    set_cn_font(r)

doc.save(OUT)
print("saved:", OUT)
