# 部署到 GitHub（自动构建 wo1wan.nro）

本目录已经是一个初始化好的 git 仓库（分支 `main`，首次提交已完成），
推送到 GitHub 后，GitHub Actions 会自动用 devkitPro 编译出 `wo1wan.nro`，
在 Actions 页面的 **Artifacts** 里下载即可。无需本地安装任何工具链。

> 仓库路径（你机器上）：`C:\Users\Administrator\Desktop\wiliwili-NintendoSwitch\wo1wan`

---

## 方法一：GitHub 网页 + git 命令（最稳，推荐）

### 1. 在 GitHub 新建一个空仓库
- 打开 https://github.com/new
- Repository name 填 `wo1wan`（或你喜欢的名字）
- 选 **Public** 或 **Private** 都行（Actions 免费额度足够）
- **不要**勾选 "Add a README" / ".gitignore" / "License"（保持空仓库，避免冲突）
- 点 **Create repository**

### 2. 在本机推送（在 wo1wan 目录下打开 Git Bash / PowerShell）
把下面 `<你的GitHub用户名>` 换成你的用户名：

```bash
cd "C:\Users\Administrator\Desktop\wiliwili-NintendoSwitch\wo1wan"

git remote add origin https://github.com/<你的GitHub用户名>/wo1wan.git
git branch -M main
git push -u origin main
```

> **认证说明**：GitHub 已不支持密码登录。推送时会要求输入用户名和密码——
> 「密码」处请填你的 **Personal Access Token (PAT)**，不是账号密码。
> 没有 PAT 的话：GitHub → Settings → Developer settings → Personal access tokens →
> 生成一个有 `repo` 权限的 token，推送时用它当密码。
> （或用 GitHub Desktop / GitHub CLI 登录更省事）

### 3. 下载构建产物
- 推送后打开仓库页面 → 顶部 **Actions** 标签 → 看 `Build wo1wan.nro` 这个工作流运行
- 等它变绿（约 1–2 分钟）→ 点进该次运行 → 右侧 **Artifacts** 区下载 `wo1wan-nro`
- 解压得到 `wo1wan.nro`

### 4. 装到 Switch
把 `wo1wan.nro` 放到内存卡 `switch/wo1wan.nro`，hbmenu 里打开即可。

---

## 方法二：用 GitHub CLI（若已装 `gh`）

```bash
cd "C:\Users\Administrator\Desktop\wiliwili-NintendoSwitch\wo1wan"

gh auth login                      # 按提示浏览器登录
gh repo create wo1wan --public --source=. --remote=origin --push
```

推送后同样去 **Actions → Artifacts** 下载 `wo1wan-nro`。

---

## 以后改代码再出包
只要把改动 `git add` + `git commit` + `git push`，Actions 就会自动重新构建，
去 Artifacts 下最新产物即可。例如改了打开的网址（`source/main.c` 里的 `GAME_URL`）：

```bash
git add source/main.c
git commit -m "换游戏入口"
git push
```

---

## 排错
- **Actions 红了对构建**：点进失败的运行看日志。常见是图标不是 256×256 JPEG
  （本仓库已确认是 256×256，正常不会触发）。
- **push 被拒 (non-fast-forward)**：说明远程已有提交，先 `git pull --rebase origin main` 再 push。
- **没看到 Artifacts**：确认工作流是绿色完成状态；制品在运行详情页右侧，不在仓库文件里。
