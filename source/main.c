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
