/*
 * wo1wan - Play web games on Nintendo Switch
 *
 * Opens https://play.wo1wan.com in the Switch Web Applet (real browser kernel).
 * The website handles login (WeChat/QQ QR) and game navigation.
 *
 * Web Applet usage follows wiliwili's proven pattern:
 *   webPageCreate -> webConfigSetWhitelist -> webConfigShow(NULL)
 *   NO memset on WebCommonConfig -- libnx manages internal state itself.
 *
 * Service initialization copied from wiliwili/borealis switch_wrapper.c:
 *   socketInitialize, nifmInitialize, plInitialize, setsys, set, psm, lbl.
 *
 * Display handling: we exit the console BEFORE launching the Web Applet, then
 * re-init it after the browser returns. This avoids the console and library
 * applet fighting over the display, which can cause "Unable to use this feature".
 *
 * Build: bash build.sh   Output: out/wo1wan.nro
 */

#include <switch.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SITE_URL "https://play.wo1wan.com"

static const char *g_banner =
    "\n\n"
    "  =============================================\n"
    "      wo1wan  -  Play web games on Switch\n"
    "  =============================================\n"
    "\n"
    "  The browser will open now.\n"
    "\n";

/* ------------------------------------------------------------------------ */
/* Initialise all services required by a real Switch homebrew app.            */
/* This mirrors wiliwili/borealis switch_wrapper.c exactly.                 */
/* ------------------------------------------------------------------------ */
static void wo1wanInitServices(void)
{
    SocketInitConfig cfg = *(socketGetDefaultInitConfig());
    AppletType at        = appletGetAppletType();

    appletLockExit();

    if (at == AppletType_Application || at == AppletType_SystemApplication)
    {
        cfg.num_bsd_sessions = 12;
        cfg.sb_efficiency    = 8;
    }
    else
    {
        cfg.num_bsd_sessions = 2;
        cfg.sb_efficiency    = 1;
    }
    socketInitialize(&cfg);

    romfsInit();
    plInitialize(PlServiceType_User);
    setsysInitialize();
    setInitialize();
    psmInitialize();
    nifmInitialize(NifmServiceType_User);
    lblInitialize();
}

/* ------------------------------------------------------------------------ */
/* Exit services in reverse order.                                          */
/* ------------------------------------------------------------------------ */
static void wo1wanExitServices(void)
{
    lblExit();
    nifmExit();
    psmExit();
    setExit();
    setsysExit();
    plExit();
    romfsExit();
    socketExit();
    appletUnlockExit();
}

/* ------------------------------------------------------------------------ */
/* Read handheld and/or first Joy-Con controller buttons.                   */
/* ------------------------------------------------------------------------ */
static u64 wo1wanReadButtons(void)
{
    HidNpadSystemState npad;
    u64 buttons = 0;

    hidGetNpadStatesSystem(HidNpadIdType_Handheld, &npad, 1);
    buttons |= npad.buttons;
    hidGetNpadStatesSystem(HidNpadIdType_No1,      &npad, 1);
    buttons |= npad.buttons;

    return buttons;
}

/* ------------------------------------------------------------------------ */
/* Wait for a button press, return it.                                      */
/* ------------------------------------------------------------------------ */
static u64 wo1wanWaitForButton(u64 mask)
{
    u64 buttons;

    while (appletMainLoop())
    {
        buttons = wo1wanReadButtons();
        if (buttons & mask)
            return buttons;
        svcSleepThread(10 * 1000000ULL);
    }
    return 0;
}

/* ======================================================================== */
int main(int argc, char **argv)
{
    WebCommonConfig config;
    Result          rc;
    u64             buttons;
    bool            running = true;

    (void)argc;
    (void)argv;

    /* Initialise services (copy wiliwili pattern) */
    appletInitialize();
    hidInitialize();
    hidInitializeNpad();

    /* ------------------------------------------------------------------
     * CONTEXT GUARD (critical)
     *
     * The Switch Web Applet can ONLY be launched from an Application
     * context. If this .nro is started from hbmenu that itself runs in
     * LibraryApplet context (e.g. opened via the Album / "相册"),
     * webConfigShow() triggers a kernel fatal (0x1003) and the system
     * shows "由于发生错误，软件已关闭".
     *
     * Detect the context up front and show clear guidance instead of
     * crashing. The user must launch via title override: hold R on a
     * game to enter hbmenu, then run this .nro (hbmenu then runs in
     * Application context).
     * ------------------------------------------------------------------ */
    {
        AppletType t = appletGetAppletType();
        if (t != AppletType_Application && t != AppletType_SystemApplication)
        {
            consoleInit(NULL);
            printf("\n!! 运行环境不正确 (AppletType=%d)\n\n", (int)t);
            printf("  Web Applet 只能在「应用程序」上下文启动。\n");
            printf("  你当前是从 hbmenu(LibraryApplet) 启动的，\n");
            printf("  系统会拒绝打开浏览器并导致崩溃。\n\n");
            printf("  正确做法：\n");
            printf("   1. 回到桌面，按住 R 键不放\n");
            printf("   2. 点开任意一个已安装的游戏\n");
            printf("   3. 保持按住 R 直到出现 hbmenu\n");
            printf("   4. 在 hbmenu 里点 wo1wan\n");
            printf("   （右上角没有红字 Applet Mode 即成功）\n\n");
            printf("  按 + 退出。\n");
            consoleUpdate(NULL);
            wo1wanWaitForButton(HidNpadButton_Plus);
            consoleExit(NULL);
            hidExit();
            appletExit();
            return 0;
        }
    }

    wo1wanInitServices();

    /* Show a brief banner, then release the console before Web Applet. */
    consoleInit(NULL);
    printf("%s", g_banner);
    consoleUpdate(NULL);
    svcSleepThread(1 * 1000 * 1000000ULL); /* 1 second so user can see it */
    consoleExit(NULL);

    while (running && appletMainLoop())
    {
        /*
         * Web Applet launch -- exact same pattern as wiliwili/borealis:
         *
         * 1. Declare config on stack, do NOT memset/zero it.
         *    libnx's webPageCreate fills in internal fields; pre-zeroing
         *    corrupts those fields and causes kernel panic (0x1003).
         *
         * 2. Only set whitelist (same as wiliwili). No pointer/stick/audio
         *    extras -- keep it minimal to avoid triggering Atmosphere bugs.
         *
         * 3. Pass NULL for reply (same as wiliwili).
         */
        rc = webPageCreate(&config, SITE_URL);
        if (R_FAILED(rc))
        {
            consoleInit(NULL);
            printf("\n!! webPageCreate failed (0x%08x)\n", rc);
            printf("Press + to quit.\n");
            consoleUpdate(NULL);
            wo1wanWaitForButton(HidNpadButton_Plus);
            break;
        }

        webConfigSetWhitelist(&config, "^http*");

        /* Blocks until user closes the Web Applet (B button or HOME). */
        rc = webConfigShow(&config, NULL);

        /* Re-take the console for the post-browser menu. */
        consoleInit(NULL);

        if (R_FAILED(rc))
        {
            printf("\n!! webConfigShow error (0x%08x)\n", rc);
            printf("\n============================================\n");
            printf("  A = retry\n");
            printf("  + = quit wo1wan\n");
            printf("============================================\n");
            consoleUpdate(NULL);

            buttons = wo1wanWaitForButton(HidNpadButton_A | HidNpadButton_Plus);
            if (buttons & HidNpadButton_Plus)
                running = false;

            consoleExit(NULL);
            continue;
        }

        printf("\n<< Browser closed.\n");
        printf("\n============================================\n");
        printf("  A = re-open browser\n");
        printf("  + = quit wo1wan\n");
        printf("============================================\n");
        consoleUpdate(NULL);

        buttons = wo1wanWaitForButton(HidNpadButton_A | HidNpadButton_Plus);
        if (buttons & HidNpadButton_Plus)
            running = false;

        consoleExit(NULL);
    }

    consoleInit(NULL);
    printf("\nGoodbye!\n");
    consoleUpdate(NULL);
    consoleExit(NULL);
    hidExit();
    wo1wanExitServices();
    appletExit();
    return 0;
}
