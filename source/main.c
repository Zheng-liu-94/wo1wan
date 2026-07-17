/*
 * wo1wan —— 模仿 wiliwili 的 Switch 自制程序
 * 把「畅玩空间」网页版游戏大厅 (https://play.wo1wan.com) 变成 Switch 上的应用，
 * 通过 Switch 内置的网页小程序(Web Applet) 直接在线玩游戏。
 *
 * 技术说明：
 *   - 用 libnx 的 Web Applet 接口（webPageCreate / webConfigShow）在 Switch 上
 *     打开一个真正的浏览器内核，渲染目标网站，因此 HTML5 / Canvas / WebGL 小游戏
 *     都能直接运行。
 *   - 左摇杆 = 鼠标指针，触摸屏 = 直接触摸，方便操作网页游戏。
 *   - 想打开别的网站？改下面 GAME_URL 即可。
 *
 * 构建：在装有 devkitPro 的环境里执行 `make`，产物为 out/wo1wan.nro
 */

#include <switch.h>
#include <hid.h>
#include <keycodes.h>
#include <stdio.h>
#include <string.h>

/* 默认打开的网址：畅玩空间「在线玩」PC 版。可改成任意网页。 */
#define GAME_URL "https://play.wo1wan.com/nextgame/pc/#/"

/* 如果想用更偏触摸/竖屏的版本，把上面改成例如 "https://play.wo1wan.com/" 再试。 */

static const char *g_banner =
    "\n"
    "  ============================================\n"
    "        畅玩空间  wo1wan  ·  Switch 在线玩\n"
    "  ============================================\n"
    "\n"
    "  正在打开网页游戏大厅……\n"
    "  左摇杆 = 鼠标指针      触摸屏 = 直接触摸\n"
    "  按  A  重新进入大厅     按  +  退出程序\n"
    "\n";

int main(int argc, char **argv)
{
    WebCommonConfig config;
    WebCommonReply  reply;
    Result rc;

    consoleInit(NULL);
    printf("%s", g_banner);
    consoleUpdate(NULL);

    hidInitialize();

    while (1)
    {
        printf("\n>> 正在启动游戏大厅……\n");
        consoleUpdate(NULL);

        /* 创建 Web Applet 配置，指向目标网址 */
        rc = webPageCreate(&config, GAME_URL);
        if (R_FAILED(rc))
        {
            printf("!! 无法创建网页配置 (0x%08x)\n", rc);
            consoleUpdate(NULL);
            break;
        }

        /* 让左摇杆直接当作鼠标指针移动，方便操作网页游戏 */
        webConfigSetPointer(&config, true);
        webConfigSetLeftStickMode(&config, WebLeftStickMode_Pointer);
        /* 触摸屏幕的输入直接传递给网页（适合触屏小游戏） */
        webConfigSetTouchEnabledOnContents(&config, true);
        /* 开启网页音频（游戏音效 / 音乐） */
        webConfigSetWebAudio(&config, true);

        /* 启动浏览器小程序，这里会一直阻塞到用户关闭网页 */
        rc = webConfigShow(&config, &reply);
        if (R_FAILED(rc))
        {
            printf("!! 网页启动失败 (0x%08x)\n", rc);
            consoleUpdate(NULL);
            break;
        }

        /* 用户从网页返回后，重置控制台并显示菜单 */
        consoleExit(NULL);
        consoleInit(NULL);

        printf("\n============================================\n");
        printf("  已退出游戏大厅。\n");
        printf("  按  A ：重新进入游戏大厅\n");
        printf("  按  + ：退出 wo1wan\n");
        printf("============================================\n");
        consoleUpdate(NULL);

        /* 等待用户选择 */
        while (1)
        {
            hidScanInput();
            u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
            if (kDown & KEY_A)
                break;                 /* 重新进入大厅 */
            if (kDown & KEY_PLUS)
            {
                consoleExit(NULL);
                return 0;              /* 退出程序 */
            }
            svcSleepThread(10000000);  /* 睡眠 10ms，避免空转 */
        }
    }

    printf("\n按  +  退出……\n");
    consoleUpdate(NULL);
    while (appletMainLoop())
    {
        hidScanInput();
        if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS)
            break;
        svcSleepThread(10000000);
    }

    consoleExit(NULL);
    return 0;
}
