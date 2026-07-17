/*
 * wo1wan - Play web games on Nintendo Switch
 *
 * Opens https://play.wo1wan.com in the Switch Web Applet (real browser kernel).
 * The website itself handles login (WeChat/QQ QR) and game navigation.
 *
 * Controls inside browser:
 *   Left Stick = mouse pointer
 *   Touch screen = tap/click
 *
 * Outside browser (on console):
 *   A = re-open browser    + = quit app
 *
 * Build: bash build.sh   Output: out/wo1wan.nro
 */

#include <switch.h>
#include <stdio.h>
#include <string.h>

#define SITE_URL "https://play.wo1wan.com"

static const char *g_banner =
    "\n\n"
    "  =============================================\n"
    "      wo1wan  -  Play web games on Switch\n"
    "  =============================================\n"
    "\n"
    "  Opening https://play.wo1wan.com ...\n"
    "  Scan QR code with phone to log in.\n"
    "\n"
    "  Inside browser:\n"
    "    Left Stick = mouse    Touch = tap\n"
    "  Outside browser:\n"
    "    A = re-open          + = quit\n"
    "\n";

/* ======================================================================== */
int main(int argc, char **argv)
{
    WebCommonConfig config;
    WebCommonReply  reply;
    Result          rc;
    HidNpadSystemState npad;
    u64             buttons;
    bool            running = true;

    (void)argc;
    (void)argv;

    /* ---- Initialise required services ---- */
    rc = appletInitialize();
    if (R_FAILED(rc)) { /* cannot even print without console */ return 1; }

    rc = hidInitialize();
    if (R_FAILED(rc)) { appletExit(); return 1; }
    hidInitializeNpad();

    consoleInit(NULL);
    printf("%s", g_banner);
    consoleUpdate(NULL);

    /* ---- Main loop: open browser → wait for close → ask user ---- */
    while (running && appletMainLoop())
    {
        /* --- Launch Web Applet --- */
        printf("\n>> Launching browser...\n");
        consoleUpdate(NULL);

        /* Zero-init the config struct — libnx does NOT do this for us,
           and stale stack memory can cause fatal svc errors (0x1003). */
        memset(&config, 0, sizeof(config));
        memset(&reply, 0, sizeof(reply));

        rc = webPageCreate(&config, SITE_URL);
        if (R_FAILED(rc))
        {
            printf("!! webPageCreate failed (0x%08x)\n", rc);
            consoleUpdate(NULL);
            break;
        }

        webConfigSetPointer(&config, true);
        webConfigSetLeftStickMode(&config, WebLeftStickMode_Pointer);
        webConfigSetTouchEnabledOnContents(&config, true);
        webConfigSetWebAudio(&config, true);

        /* Blocks until user closes the Web Applet (presses B or HOME). */
        rc = webConfigShow(&config, &reply);
        if (R_FAILED(rc))
        {
            printf("!! webConfigShow error (0x%08x)\n", rc);
            consoleUpdate(NULL);
            /* Don't crash — fall through to menu so user can retry or quit. */
        }
        else
        {
            printf("<< Browser closed normally.\n");
        }

        /* Web Applet takes over the display; re-init console. */
        consoleExit(NULL);
        consoleInit(NULL);

        printf("\n============================================\n");
        printf("  Press A : re-open browser\n");
        printf("  Press + : quit wo1wan\n");
        printf("============================================\n");
        consoleUpdate(NULL);

        /* ---- Wait for button ---- */
        while (appletMainLoop())
        {
            memset(&npad, 0, sizeof(npad));
            buttons = 0;

            hidGetNpadStatesSystem(HidNpadIdType_Handheld, &npad, 1);
            buttons |= npad.buttons;
            hidGetNpadStatesSystem(HidNpadIdType_No1,      &npad, 1);
            buttons |= npad.buttons;

            if (buttons & HidNpadButton_Plus)
            { running = false; break; }
            if (buttons & HidNpadButton_A)
            { break; }               /* relaunch */

            svcSleepThread(10 * 1000000ULL); /* 10 ms */
        }

        /* Re-init console before next loop (web applet dirties state). */
        consoleExit(NULL);
        consoleInit(NULL);
    }

    /* ---- Clean shutdown (single exit path) ---- */
    printf("\nShutting down. Goodbye!\n");
    consoleUpdate(NULL);
    consoleExit(NULL);
    hidExit();
    appletExit();
    return 0;
}
