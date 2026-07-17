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
    "  Opening browser, please wait...\n"
    "\n"
    "  Controls:\n"
    "    A = re-open     + = quit\n"
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

/* ======================================================================== */
int main(int argc, char **argv)
{
    WebCommonConfig config;
    Result          rc;
    HidNpadSystemState npad;
    u64             buttons;
    bool            running = true;

    (void)argc;
    (void)argv;

    /* Initialise services (copy wiliwili pattern) */
    appletInitialize();
    hidInitialize();
    hidInitializeNpad();
    wo1wanInitServices();

    consoleInit(NULL);
    printf("%s", g_banner);
    consoleUpdate(NULL);

    while (running && appletMainLoop())
    {
        printf("\n>> Opening %s ...\n", SITE_URL);
        consoleUpdate(NULL);

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
            printf("!! webPageCreate failed (0x%08x)\n", rc);
            consoleUpdate(NULL);
            break;
        }

        webConfigSetWhitelist(&config, "^http*");

        /* Blocks until user closes the Web Applet (B button or HOME). */
        rc = webConfigShow(&config, NULL);
        if (R_FAILED(rc))
        {
            printf("!! webConfigShow error (0x%08x)\n", rc);
            consoleUpdate(NULL);
            /* Fall through to menu so user can retry or quit. */
        }
        else
        {
            printf("<< Browser closed.\n");
        }

        /* Web Applet takes over display; re-init console. */
        consoleExit(NULL);
        consoleInit(NULL);

        printf("\n============================================\n");
        printf("  A = re-open browser\n");
        printf("  + = quit wo1wan\n");
        printf("============================================\n");
        consoleUpdate(NULL);

        /* Wait for A or + */
        while (appletMainLoop())
        {
            buttons = 0;
            hidGetNpadStatesSystem(HidNpadIdType_Handheld, &npad, 1);
            buttons |= npad.buttons;
            hidGetNpadStatesSystem(HidNpadIdType_No1,      &npad, 1);
            buttons |= npad.buttons;

            if (buttons & HidNpadButton_Plus)
            { running = false; break; }
            if (buttons & HidNpadButton_A)
            { break; }

            svcSleepThread(10 * 1000000ULL);
        }

        consoleExit(NULL);
        consoleInit(NULL);
    }

    printf("\nGoodbye!\n");
    consoleUpdate(NULL);
    consoleExit(NULL);
    hidExit();
    wo1wanExitServices();
    appletExit();
    return 0;
}
