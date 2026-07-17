# wo1wan

模仿 [wiliwili](https://github.com/xfangfang/wiliwili) 的做法，把网页游戏平台
**畅玩空间**（https://play.wo1wan.com ）变成任天堂 Switch 的自制程序（`.nro`），
让你在 Switch 上**直接在线玩网页小游戏**。

它本质上是一个 Switch 自制启动器：打开后用机器内置的网页小程序（Web Applet，
真正的浏览器内核）加载目标网站，所以 HTML5 / Canvas / WebGL 小游戏都能直接跑。

## 特性

- 启动即打开「畅玩空间」在线玩大厅，免电脑、免手机，掌机/主机直接玩。
- **左摇杆 = 鼠标指针**，**触摸屏 = 直接触摸**，方便操作网页游戏。
- 开启网页音频，游戏有声音。
- 退出网页后可选「重新进入」或「退出程序」，像一个真正的 App。

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

1. 把编译出的 `wo1wan.nro` 放到内存卡 `switch/wo1wan.nro`。
2. 在 Switch 主页**按住 R 键**打开任意游戏进入 hbmenu，
   在列表里选择 `wo1wan` 点击打开即可。
3. 首次打开会进入网页游戏大厅，等待加载即可开玩。

## 自定义打开的网址

编辑 `source/main.c` 顶部的：

```c
#define GAME_URL "https://play.wo1wan.com/nextgame/pc/#/"
```

改成任意网页后重新 `make` 即可。比如想试更偏触摸的版本，可改成
`https://play.wo1wan.com/` 再观察效果。

## 替换图标

`icon.jpg` 是显示在 hbmenu 里的图标（当前为占位图）。
换成你自己的 **256×256 JPEG** 再 `make` 即可。

## 已知限制

- **登录（微信 / QQ 扫码）完全可用**：Web Applet 就是 Switch 内置的真浏览器，
  二维码会直接显示在屏幕上，拿出手机扫码即可——和 wiliwili 扫 bilibili 登录码是同一套机制。
  需要登录才能玩的内容（如个人收藏、部分专区）都能正常进入。
  > 提示：每次打开 Web Applet 是干净的浏览器会话，登录态通常不跨次保存，
  > 所以重新进入后可能要再扫一次码，属正常现象。
- Switch 内置浏览器内核较旧（基于老版本 WebKit），个别依赖最新前端框架的页面
  可能显示/运行不完整。若有问题，可尝试把 `GAME_URL` 换成网站的「简洁/旧版/移动」入口。
- 左摇杆模拟鼠标，更偏「点击类」网页游戏；需要键盘输入的游戏在掌机上体验有限。

## 致谢

- 灵感与结构参考开源项目 [wiliwili](https://github.com/xfangfang/wiliwili)。
- 基于 [libnx](https://github.com/switchbrew/libnx) 与 devkitPro 工具链。
