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

### 方式一：本地用 devkitPro 编译（推荐，几秒出包）

1. 安装 devkitPro，并确保装好 Switch 组件：
   ```bash
   # 在 devkitPro 的 MSYS2 / 终端里执行
   dkp-pacman -S switch-dev
   ```
2. 在项目目录执行：
   ```bash
   make
   ```
3. 产物：`out/wo1wan.nro`

> Windows 上若 `DEVKITPRO` 没自动设置，可手动指定：
> `make DEVKITPRO=/c/devkitpro`

### 方式二：用 GitHub Actions 自动出包（无需本地工具链）

把本仓库推送到 GitHub，Actions 会自动编译并在 **Artifacts** 里给出 `wo1wan.nro`。
（见 `.github/workflows/build.yml`）

## 安装到 Switch

⚠️ **必须从桌面图标启动，不能从 hbmenu 直接打开 `.nro`。**

原因：Switch 的网页浏览器（Web Applet）只能在「应用程序（Application）」上下文里启动，
而 hbmenu 是小程序（LibraryApplet）上下文，从它打开的 `.nro` 无法启动网页，
会显示「无法使用此功能 / Unable to use this feature」。wiliwili 能开网页，
也是因为它从桌面图标启动——它本身就是一个 Application 类型的 NSP 启动器。

正确步骤：
1. 下载 `wo1wan.nsp`，用 **DBI / Tesla 菜单 / Tinfoil** 安装到桌面（像装普通游戏一样）。
2. 下载 `wo1wan.nro`，放到内存卡 `switch/wo1wan.nro`（被桌面图标自动加载，无需手动开）。
3. 从桌面点击 wo1wan 图标打开，浏览器自动加载「畅玩空间」，扫码登录后即可玩。

## 自定义打开的网址

编辑 `source/main.c` 顶部的两个宏：

```c
#define HOME_URL  "https://play.wo1wan.com"                 /* 登录页 */
#define GAME_URL  "https://play.wo1wan.com/nextgame/pc/#/"  /* 游戏大厅 */
```

改成任意网页后重新 `make` 即可。例如把 `HOME_URL` 改成网站的移动版登录页，可能会获得更好的扫码体验。

## 替换图标

`icon.jpg` 是显示在 hbmenu 里的图标（当前为占位图）。
换成你自己的 **256×256 JPEG** 再 `make` 即可。

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
