# -*- coding: utf-8 -*-
"""Simple one-page progress card image for Coolapk."""
from PIL import Image, ImageDraw, ImageFont

W, H = 1080, 1500
BG = (18, 20, 26)
CARD = (28, 31, 40)
ACCENT = (0x4C, 0x8B, 0xF5)   # blue
GREEN = (0x2E, 0xC4, 0x7B)
ORANGE = (0xF5, 0xA6, 0x23)
RED = (0xE8, 0x54, 0x63)
GRAY = (0x8A, 0x91, 0xA3)
WHITE = (0xF2, 0xF4, 0xF8)

FONT = r"C:\Windows\Fonts\msyh.ttc"
FB = r"C:\Windows\Fonts\msyhbd.ttc"

def f(size, bold=False):
    return ImageFont.truetype(FB if bold else FONT, size)

img = Image.new("RGB", (W, H), BG)
d = ImageDraw.Draw(img)

# header
d.text((60, 60), "OPPO Find X9 Ultra", font=f(52, True), fill=WHITE)
d.text((60, 130), "PMA120 · 骁龙 8 Elite Gen 5 · iOS 移植进度", font=f(30), fill=GRAY)
d.line([(60, 200), (W - 60, 200)], fill=(50, 54, 66), width=2)
d.text((60, 225), "2026-11-09", font=f(26), fill=GRAY)

# stage rows
rows = [
    ("引导链（GBL → 载体 → 固件跳转）", 100, GREEN),
    ("固件适配（Silicium UEFI 全派发）", 100, GREEN),
    ("转场服务（xnuloader）", 95, ORANGE),
    ("内核（XNU 22.5 真机启动）", 60, ORANGE),
    ("用户态（mockfs / hello）", 50, ORANGE),
    ("iOS 系统层（UFS 根盘）", 5, GRAY),
]

y = 300
bar_x, bar_w = 60, 700
for name, pct, color in rows:
    d.text((bar_x, y), name, font=f(30), fill=WHITE)
    d.text((W - 170, y), f"{pct}%", font=f(32, True), fill=color)
    # bar
    d.rounded_rectangle([bar_x, y + 52, bar_x + bar_w, y + 76], radius=12, fill=(45, 49, 60))
    if pct > 0:
        d.rounded_rectangle([bar_x, y + 52, bar_x + int(bar_w * pct / 100), y + 76], radius=12, fill=color)
    y += 118

# box: current blocker
bx, by, bw, bh = 60, y + 10, W - 120, 175
d.rounded_rectangle([bx, by, bx + bw, by + bh], radius=24, fill=CARD, outline=(70, 76, 92), width=2)
d.text((bx + 36, by + 24), "当前唯一卡点", font=f(32, True), fill=RED)
d.text((bx + 36, by + 80), "loader 的 USB 身份不被主机采纳：", font=f(28), fill=WHITE)
d.text((bx + 36, by + 126), "固件侧 QUSB_BULK (05C6:900E) ≠ loader 的 18D1:D00D", font=f(26), fill=GRAY)

# footer
d.text((60, H - 110), "已打通：GBL → 载体 → UEFI 固件全派发 → 自定义驱动 → loader 运行", font=f(26), fill=GREEN)
d.text((60, H - 62), "内核链路已在 QEMU 完整验证（打印 Hello from user space on Darwin!）", font=f(24), fill=GRAY)

img.save(r"C:\Users\1\Documents\deepseek-harness\default-workspace\X9U-iOS移植\PMA120-进度图.png")
print("saved")
