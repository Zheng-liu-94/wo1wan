/*
 * wo1wan - a Switch homebrew that mimics wiliwili
 *
 * Turns the "Chang Wan Kong Jian" web game lobby (https://play.wo1wan.com)
 * into a Switch app by opening it inside the Switch Web Applet (a real
 * browser kernel), so HTML5 / Canvas / WebGL mini-games run directly.
 *
 * Controls:
 *   - Left Stick = mouse pointer, Touch screen = tap
 *   - Press A to re-open the lobby, Press + to quit
 *
 * To open a different site, change GAME_URL below and rebuild.
 *
 * Build: run `bash build.sh` in a devkitPro environment.
 *        Output: out/wo1wan.nro
 *
 * NOTE: all on-screen text is English on purpose - the Switch console font
 * has no CJK glyphs, so Chinese would render as garbage boxes.
 */

#include <switch.h>
#include <stdio.h>
#include <string.h>

/* Default site: the "play online" PC version of the lobby. Change freely. */
#define GAME_URL "https://play.wo1wan.com/nextgame/pc/#/"

static const char *g_banner =
    "\n"
    "  ============================================\n"
    "           wo1wan  -  Play web games on Switch\n"
    "  ============================================\n"
    "\n"
    "  Launching the web game lobby...\n"
    "  Left Stick = mouse    Touch screen = tap\n"
    "  Press  A  to re-open   Press  +  to quit\n"
    "\n";

int main(int argc, char **argv)
{
    WebCommonConfig    config;
    WebCommonReply     reply;
    Result             rc;
    HidNpadSystemState npad;
    u64                buttons;
    bool               running = true;

    /* Required service initialization. Without appletInitialize() the Web
       Applet launch (and appletMainLoop) crashes on a real Switch. */
    appletInitialize();
    hidInitialize();
    hidInitializeNpad();
    consoleInit(NULL);

    printf("%s", g_banner);
    consoleUpdate(NULL);

    while (running)
    {
        printf("\n>> Launching game lobby...\n");
        consoleUpdate(NULL);

        rc = webPageCreate(&config, GAME_URL);
        if (R_FAILED(rc))
        {
            printf("!! Failed to create web config (0x%08x)\n", rc);
            consoleUpdate(NULL);
            break;
        }

        /* Left stick acts as the mouse pointer for web games. */
        webConfigSetPointer(&config, true);
        webConfigSetLeftStickMode(&config, WebLeftStickMode_Pointer);
        /* Forward touch input to the page (good for touch games). */
        webConfigSetTouchEnabledOnContents(&config, true);
        /* Enable web audio (game sound / music). */
        webConfigSetWebAudio(&config, true);

        /* Blocks until the user closes the web applet. */
        rc = webConfigShow(&config, &reply);
        if (R_FAILED(rc))
        {
            printf("!! Failed to launch web (0x%08x)\n", rc);
            consoleUpdate(NULL);
            break;
        }

        /* Web applet returned; the display was taken over, so re-init console. */
        consoleExit(NULL);
        consoleInit(NULL);

        printf("\n============================================\n");
        printf("  Left the game lobby.\n");
        printf("  Press  A : re-open lobby\n");
        printf("  Press  + : quit wo1wan\n");
        printf("============================================\n");
        consoleUpdate(NULL);

        /* Wait for A (relaunch) or + (quit). */
        while (1)
        {
            memset(&npad, 0, sizeof(npad));
            buttons = 0;
            hidGetNpadStatesSystem(HidNpadIdType_Handheld, &npad, 1);
            buttons |= npad.buttons;
            hidGetNpadStatesSystem(HidNpadIdType_No1, &npad, 1);
            buttons |= npad.buttons;

            if (buttons & HidNpadButton_A)
                break;                 /* relaunch lobby */
            if (buttons & HidNpadButton_Plus)
            {
                running = false;
                break;                 /* quit */
            }
            svcSleepThread(10000000);  /* ~10ms, avoid busy loop */
        }
    }

    /* Fallback: if web launch failed above, just wait for + to quit. */
    printf("\nPress  +  to quit...\n");
    consoleUpdate(NULL);
    while (running && appletMainLoop())
    {
        memset(&npad, 0, sizeof(npad));
        buttons = 0;
        hidGetNpadStatesSystem(HidNpadIdType_Handheld, &npad, 1);
        buttons |= npad.buttons;
        hidGetNpadStatesSystem(HidNpadIdType_No1, &npad, 1);
        buttons |= npad.buttons;
        if (buttons & HidNpadButton_Plus)
            break;
        svcSleepThread(10000000);
    }

    consoleExit(NULL);
    hidExit();
    appletExit();
    return 0;
}
