# wo1wan

模仿 [wiliwili](https://github.com/xfangfang/wiliwili) 的做法，把网页游戏平台
**畅玩空间**（https://play.wo1wan.com ）变成任天堂 Switch 的自制程序（`.nro`），
让你在 Switch 上**直接在线玩网页小游戏**。

它本质上是一个 Switch 自制启动器：打开后用机器内置的网页小程序（Web Applet，
真正的浏览器内核）加载目标网站，所以 HTML5 / Canvas / WebGL 小游戏都能直接跑。

## 特性

- 启动后先打开「畅玩空间」主页/登录页，扫码登录后再进入游戏大厅。
- 用 Switch 内置的 Web Applet 加载网页，HTML5 / Canvas / WebGL 小游戏直接运行。
- **左摇杆 = 鼠标指针**，**触摸屏 = 直接触摸**，方便操作网页游戏。
- 开启网页音频，游戏有声音。
- 退出网页后可选「重新进入」或「退出程序」，像一个真正的 App。
- 加强的资源清理和退出检测，避免异常退出导致系统不稳定。

## 构建

### 方式一：本地用 devkitPro 编译 `.nro`（网页壳本体）

1. 安装 devkitPro，并确保装好 Switch 组件：
   ```bash
   dkp-pacman -S switch-dev
   ```
2. 在项目目录执行（任选其一）：
   ```bash
   make             # 用 Makefile
   bash build.sh    # 等价脚本，CI 用的就是这个
   ```
3. 产物：`out/wo1wan.nro`

> Windows 上若 `DEVKITPRO` 没自动设置：`make DEVKITPRO=/c/devkitpro`

### 方式二：用 GitHub Actions 自动出包（无需本地工具链）

把本仓库推送到 GitHub，Actions 会自动编译并在 **Release（latest）** 里给出 `wo1wan.nro`：

- nro: `https://github.com/Zheng-liu-94/wo1wan/releases/download/latest/wo1wan.nro`

（见 `.github/workflows/build.yml`）

> 关于 `.nsp` 桌面图标：需要用户自己的 `prod.keys` 才能用 hacbrewpack 打包成合法可安装 NSP
> （见下方「桌面图标」一节）。CI 无法提供密钥，因此不再产出 `.nsp`。

## 在 Switch 上使用

⚠️ **关键：不要从「相册」打开 hbmenu，必须「按住 R 键启动一个游戏」来打开 hbmenu。**

原因：Switch 的网页浏览器（Web Applet）只能在 **Application 上下文** 里启动。

| 打开 hbmenu 的方式 | 上下文 | 能开浏览器吗 |
| --- | --- | --- |
| 点「相册」进 | applet 模式（LibraryApplet） | ❌ 系统禁止，白屏「无法使用此功能」 |
| **按住 R 键**再点游戏进 | 标题接管 title override（Application） | ✅ 可以 |

官方教程（switch.hacks.guide）明确：用相册打开 hbmenu「无法启动完整网页浏览器」，
建议按住 R 启动游戏。这是纯系统限制，与本软件无关。

正确步骤（免安装、免密钥）：
1. 下载 `wo1wan.nro`，放到内存卡 `switch/wo1wan.nro`。
2. Switch 桌面「按住 R 键」不放 → 点开任意已安装游戏 → 继续按住 R 直到 hbmenu 出现。
   （游戏要选账号时：先点游戏 → 等选账号画面 → 这时再按住 R 选账号，一直按住到 hbmenu）
3. 确认右上角**没有**红字 `Applet Mode`（有红字说明还在相册模式，重来）。
4. 在 hbmenu 里点 wo1wan，浏览器自动加载「畅玩空间」，扫码登录后即可玩。

### 桌面图标（可选，需要你自己的 prod.keys）

想像 wiliwili 那样从桌面图标直接打开（不用每次按 R），需要一个 Application 类型的
可安装 NSP。打包它必须用从**你自己主机 dump** 出来的 `prod.keys`（受版权保护，仓库/CI
无法提供）。步骤：

1. 用 Lockpick_RCM dump 出 `prod.keys`。
2. 本地装 devkitPro + hacbrewpack，参考 wiliwili 的 `scripts/switch-forwarder/pack.sh`：
   ```bash
   # forwarder/ 目录已备好 exefs(main/main.npdm)、control(nacp/icon)、logo
   hacbrewpack -k prod.keys --titleid 010ff000ffff0002 --titlename wo1wan --noromfs
   ```
3. 得到的 NSP 用 DBI/Tinfoil 安装，桌面会出现图标。

> 注意：旧版 `forwarder/build_forwarder.sh` / `make_nsp.py` 生成的是**裸 ExeFS PFS0**，
> 不含 NCA/CNMT，安装器会报「未找到内容元数据」而失败——这不是合法可安装 NSP，
> 仅作历史保留。请用上面的 hacbrewpack 方式，或直接用「按住 R」免安装方法。

## 自定义打开的网址

编辑 `source/main.c` 顶部（第 26 行附近）的宏：

```c
#define SITE_URL "https://play.wo1wan.com"
```

改成任意网页后重新 `make`（或 `bash build.sh`）即可。例如改成网站的移动版登录页，可能会获得更好的扫码体验。

## 替换图标

根目录 `icon.jpg` 用于 `.nro`（在 hbmenu 里显示）；
`forwarder/icon.jpg` 用于 `.nsp`（桌面图标）。
换成你自己的 **256×256 JPEG** 再重新构建对应目标即可。

## 已知限制

- **登录（微信 / QQ 扫码）可用，但界面和 wiliwili 不同**：
  - wiliwili 是自己用 Switch 原生 GUI 绘制二维码；
  - 本启动器使用 Web Applet（Switch 内置浏览器）打开网页，二维码由 **网页本身** 显示，因此扫码界面看起来就是网页版的登录页，而不是 wiliwili 风格的 App 内页面。
  - 打开手机扫码，登录成功后按 **A 键** 即可进入游戏大厅。
  - 每次重新打开 Web Applet 是干净的浏览器会话，登录态通常不跨次保存，重新进入后可能需要再扫一次码，属正常现象。
- Switch 内置浏览器内核较旧（基于老版本 WebKit），个别依赖最新前端框架的页面可能显示/运行不完整。若有问题，可尝试在浏览器内先登录后再按 A 进入游戏大厅。
- 左摇杆模拟鼠标，更偏「点击类」网页游戏；需要键盘输入的游戏在掌机上体验有限。

## 致谢

- 灵感与结构参考开源项目 [wiliwili](https://github.com/xfangfang/wiliwili)。
- 基于 [libnx](https://github.com/switchbrew/libnx) 与 devkitPro 工具链。
