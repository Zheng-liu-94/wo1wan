/*
 * wo1wan - a Switch homebrew that mimics wiliwili
 *
 * Turns "Chang Wan Kong Jian" (https://play.wo1wan.com) into a Switch app.
 * Uses the Switch Web Applet (real browser kernel) so HTML5/Canvas/WebGL games
 * run directly on the console.
 *
 * Login flow (mimics wiliwili's QR pattern):
 *   Step 1 - Open wo1wan HOME page  -> WeChat/QQ QR code appears -> user scans
 *   Step 2 - After login, press A    -> Open GAME LOBBY to play
 *   Press + at any time to quit.
 *
 * Controls inside Web Applet:
 *   Left Stick = mouse pointer
 *   Touch screen = tap/click
 *   B button     = back
 *
 * Build: bash build.sh   Output: out/wo1wan.nro
 *
 * NOTE: All on-screen text is English because the Switch console font has no
 * CJK glyphs -- Chinese would render as garbage boxes.
 */

#include <switch.h>
#include <stdio.h>
#include <string.h>

/* ---- URLs --------------------------------------------------------------- */
/* Home / login page -- shows WeChat or QQ QR code for scanning. */
#define HOME_URL  "https://play.wo1wan.com"
/* Game lobby deep link -- shown after successful login. */
#define GAME_URL  "https://play.wo1wan.com/nextgame/pc/#/"

/* ---- UI text (English only – CJK = mojibake on Switch console) ---------- */
static const char *g_banner =
    "\n\n"
    "  ==============================================\n"
    "      wo1wan  -  Play web games on Switch\n"
    "  ==============================================\n"
    "\n"
    "  STEP 1: Scan QR code to log in (WeChat / QQ)\n"
    "  STEP 2: After login, press A to enter game lobby\n"
    "\n"
    "  Inside browser:\n"
    "    Left Stick = mouse    Touch = tap\n"
    "    B = back              +  = quit app\n"
    "\n";

static const char *g_post_login =
    "\n  =============================================\n"
    "  Returned from browser.\n"
    "  A = re-open    + = quit wo1wan\n"
    "  =============================================\n";

/* ------------------------------------------------------------------------ */
/* Launch the Web Applet with the given URL and standard settings.
   Returns true if the user exited normally, false on error. */
static bool launch_web(const char *url, const char *label)
{
    WebCommonConfig config;
    WebCommonReply  reply;
    Result          rc;

    printf(">> Opening %s ...\n", label);
    printf("   URL: %s\n", url);
    consoleUpdate(NULL);

    rc = webPageCreate(&config, url);
    if (R_FAILED(rc))
    {
        printf("!! webPageCreate failed (0x%08x)\n", rc);
        consoleUpdate(NULL);
        return false;
    }

    /* Configure for game-friendly browsing. */
    webConfigSetPointer(&config, true);
    webConfigSetLeftStickMode(&config, WebLeftStickMode_Pointer);
    webConfigSetTouchEnabledOnContents(&config, true);
    webConfigSetWebAudio(&config, true);

    /* Block here until the user closes the Web Applet. */
    rc = webConfigShow(&config, &reply);
    if (R_FAILED(rc))
    {
        printf("!! webConfigShow failed (0x%08x)\n", rc);
        consoleUpdate(NULL);
        return false;
    }

    printf("<< Browser closed.\n");
    return true;
}

/* ------------------------------------------------------------------------ */
/* Wait for A (continue) or + (quit). Returns true if user wants to continue.
   Also returns false if the system requests exit (appletMainLoop). */
static bool wait_for_a_or_plus(void)
{
    HidNpadSystemState npad_handheld;
    HidNpadSystemState npad_1;
    u64                buttons;

    while (appletMainLoop())
    {
        memset(&npad_handheld, 0, sizeof(npad_handheld));
        memset(&npad_1,       0, sizeof(npad_1));
        buttons = 0;

        hidGetNpadStatesSystem(HidNpadIdType_Handheld, &npad_handheld, 1);
        buttons |= npad_handheld.buttons;
        hidGetNpadStatesSystem(HidNpadIdType_No1,      &npad_1,       1);
        buttons |= npad_1.buttons;

        if (buttons & HidNpadButton_Plus)
            return false;               /* user wants to quit */
        if (buttons & HidNpadButton_A)
            return true;                /* user wants to continue */

        svcSleepThread(10 * 1000000ULL); /* 10 ms */
    }

    /* System requested exit (e.g., home button → close). */
    return false;
}

/* ======================================================================== */
int main(int argc, char **argv)
{
    bool  logged_in = false;
    bool  running   = true;

    (void)argc;
    (void)argv;

    /* ---- Initialise services ------------------------------------------- */
    appletInitialize();
    hidInitialize();
    hidInitializeNpad();
    consoleInit(NULL);

    printf("%s", g_banner);
    consoleUpdate(NULL);

    /* ---- Main loop ---------------------------------------------------- */
    while (running && appletMainLoop())
    {
        /* Phase 1: Login page (shows QR code for WeChat / QQ scan). */
        if (!logged_in)
        {
            printf("\n-- PHASE 1: Login (scan QR with phone) -----------\n");
            consoleUpdate(NULL);

            if (!launch_web(HOME_URL, "Login / Home page"))
            {
                printf("!! Could not open login page.\n");
                break;
            }

            /* User returned from browser -- assume they logged in. */
            logged_in = true;
            printf("\n-- Login phase done. Press A for game lobby. --\n");
            consoleUpdate(NULL);
        }

        /* Phase 2: Game lobby (the actual games). */
        if (logged_in)
        {
            printf("\n-- PHASE 2: Game Lobby --------------------------\n");
            consoleUpdate(NULL);

            if (!launch_web(GAME_URL, "Game Lobby"))
            {
                printf("!! Could not open game lobby.\n");
                /* Don't crash -- let user retry or quit. */
            }
        }

        /* Returned from browser. Show menu. */
        consoleExit(NULL);
        consoleInit(NULL);
        printf("%s", g_post_login);
        consoleUpdate(NULL);

        running = wait_for_a_or_plus();

        /* Re-init console for next iteration (web applet takes over display). */
        consoleExit(NULL);
        consoleInit(NULL);
    }

    /* ---- Clean shutdown ----------------------------------------------- */
    printf("\nShutting down wo1wan. Goodbye!\n");
    consoleUpdate(NULL);

    consoleExit(NULL);
    hidExit();
    appletExit();
    return 0;
}
