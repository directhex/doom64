#include "doomdef.h"
#include "d_main.h"
#include "p_local.h"
#include "sounds.h"
#include "s_sound.h"
#include "st_main.h"
#include "r_local.h"

#define MENUFONTRED ARGB16(31, 31, 0, 0)
#define MENUFONTWHITE ARGB16(31, 31, 31, 31)

typedef struct
{
    char* name;
    int x;
    int y;
} menuitem_t;

static void(*menudrawfunc)(void) = NULL;
static int m_menuitems = 0;
static int m_currentmenu = 0;
static int m_itemOn = 0;
static menuitem_t* menu;
static int menuskulldelay = 0;
static dboolean showfullitemvalue[2] = { false, false };

dboolean menuactive = false;
int menuskullcounter = 0;

enum
{
    menu_newgame,
    menu_options1,
    menu_options2,
    menu_mainmenu,
    menu_restartlevel,
    menu_debug,
    menu_controls,
    menu_volume,
    menu_display,
    menu_returnpaused,
    menu_quitmain_yes,
    menu_quitmain_no,
    menu_restart_yes,
    menu_restart_no,
    menu_nolights,
    menu_godmode,
    menu_noclip,
    menu_allkeys,
    menu_allweapons,
    menu_mapall,
    menu_lockmonsters,
    menu_returnpaused2,
    menu_brightness,
    menu_swapscreens,
    menu_messages,
    menu_statusbar,
    menu_displaydefault,
    menu_returndisplay,
    NUM_MENU_ITEMS
};

#define MENU_MAINMENU       menu_newgame
#define MENU_GAMEMENU       menu_options2
#define MENU_GAMEOPTIONS    menu_controls
#define MENU_QUITTOMAIN     menu_quitmain_yes
#define MENU_RESTARTLEVEL   menu_restart_yes
#define MENU_DEBUGMENU      menu_nolights
#define MENU_DISPLAY        menu_brightness

static menuitem_t menulist[NUM_MENU_ITEMS] =
{
    { "New Game", 82, 144 },
    { "Options", 82, 162 },
    { "Options", 82, 64 },
    { "Main Menu", 82, 82 },
    { "Restart Level", 82, 100 },
    { "Debug", 82, 118 },
    { "Controls", 82, 64 },
    { "Volume", 82, 82 },
    { "Display", 82, 100 },
    { "/r Return", 82, 118 },
    { "Yes", 108, 80 },
    { "No", 108, 98 },
    { "Yes", 108, 80 },
    { "No", 108, 98 },
    { "No Lights", 32, 40 },
    { "God Mode", 32, 52 },
    { "No Clip", 32, 64 },
    { "All Keys", 32, 76 },
    { "All Weapons", 32, 88 },
    { "Map Everything", 32, 100 },
    { "Lock Monsters", 32, 112 },
    { "Return", 32, 124 },
    { "Brightness", 40, 48 },
    { "Swap Screen:", 40, 84 },
    { "Messages:", 40, 102 },
    { "Status Bar:", 40, 120 },
    { "Default", 40, 138 },
    { "/r Return", 40, 156 }
};

//
// M_SetMenu
//

void M_SetMenu(int item, int numitems, void(*drawer)(void))
{
    m_menuitems = numitems;
    menu = menulist + item;
    m_currentmenu = item;
    menudrawfunc = drawer;
    m_itemOn = 0;
}

//
// M_Ticker
//

void M_Ticker(void)
{
    if(!(menuskulldelay++ & 3))
        menuskullcounter = ((menuskullcounter + 1) & 7);
}

//
// M_GenericDrawer
//

static void M_GenericDrawer(void)
{
    int i;

    switch(m_currentmenu)
    {
    case MENU_GAMEMENU:
        ST_DrawBigFont(-1, 16, MENUFONTRED, "Paused");
        ST_DrawBigFont(-1, 160, MENUFONTWHITE, "Press Start To Exit");
        break;
    case MENU_GAMEOPTIONS:
        ST_DrawBigFont(-1, 16, MENUFONTRED, "Options");
        break;
    case MENU_QUITTOMAIN:
    case MENU_RESTARTLEVEL:
        ST_DrawBigFont(-1, 16, MENUFONTRED, "Quit Game?");
        break;
    default:
        break;
    }

    for(i = 0; i < m_menuitems; i++)
        ST_DrawBigFont(menu[i].x, menu[i].y, MENUFONTRED, menu[i].name);

    ST_DrawBigFont(menu[m_itemOn].x - 34, menu[m_itemOn].y - 8, 0xFFFFF, "*");
}

//
// M_DisplayDrawer
//

static void M_DisplayDrawer(void)
{
    int i;

    ST_DrawBigFont(-1, 16, MENUFONTRED, "Display");
    ST_DrawBigFont(menu[0].x, menu[0].y, MENUFONTRED, menu[0].name);
    ST_DrawBigFont(menu[0].x, menu[0].y + 18, MENUFONTWHITE, "/t");
    ST_DrawBigFont(menu[0].x, menu[0].y + 18, MENUFONTWHITE, "/s");

    for(i = 1; i < m_menuitems; i++)
        ST_DrawBigFont(menu[i].x, menu[i].y, MENUFONTRED, menu[i].name);

    ST_DrawBigFont(menulist[menu_swapscreens].x + 160, menulist[menu_swapscreens].y,
        MENUFONTRED, "Off");
    ST_DrawBigFont(menulist[menu_messages].x + 160, menulist[menu_messages].y,
        MENUFONTRED, "On");
    ST_DrawBigFont(menulist[menu_statusbar].x + 160, menulist[menu_statusbar].y,
        MENUFONTRED, "On");

    ST_DrawBigFont(menu[m_itemOn].x - 34, menu[m_itemOn].y - 8, 0xFFFFF, "*");
}

//
// M_DebugDrawer
//

static void M_DebugDrawer(void)
{
    int i;

     ST_DrawBigFont(-1, 16, MENUFONTRED, "Debug");

    for(i = 0; i < m_menuitems; i++)
        ST_DrawMessage(menu[i].x, menu[i].y, MENUFONTWHITE, menu[i].name);

    ST_DrawMessage(menulist[menu_nolights].x + 160, menulist[menu_nolights].y,
        MENUFONTWHITE, nolights ? ":On" : ":Off");

    ST_DrawMessage(menulist[menu_godmode].x + 160, menulist[menu_godmode].y,
        MENUFONTWHITE, players[consoleplayer].cheats & CF_GODMODE ? ":On" : ":Off");

    ST_DrawMessage(menulist[menu_noclip].x + 160, menulist[menu_noclip].y,
        MENUFONTWHITE, players[consoleplayer].cheats & CF_NOCLIP ? ":On" : ":Off");

    ST_DrawMessage(menulist[menu_allkeys].x + 160, menulist[menu_allkeys].y,
        MENUFONTWHITE, showfullitemvalue[0] ? ":100%%" : ":-");

    ST_DrawMessage(menulist[menu_allweapons].x + 160, menulist[menu_allweapons].y,
        MENUFONTWHITE, showfullitemvalue[1] ? ":100%%" : ":-");

    ST_DrawMessage(menulist[menu_mapall].x + 160, menulist[menu_mapall].y,
        MENUFONTWHITE, amCheating ? ":On" : ":Off");

    ST_DrawMessage(menulist[menu_lockmonsters].x + 160, menulist[menu_lockmonsters].y,
        MENUFONTWHITE, lockmonsters ? ":On" : ":Off");

    ST_DrawBigFont(menu[m_itemOn].x - 12, menu[m_itemOn].y - 2, 0xFFFFF, "/l");
}

//
// M_MenuChoice
//

static void M_MenuChoice(void)
{
    int i;
    player_t* p;

    p = &players[consoleplayer];

    switch(m_currentmenu + m_itemOn)
    {
    case menu_options2:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_GAMEOPTIONS, 4, M_GenericDrawer);
        break;
    case menu_mainmenu:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_QUITTOMAIN, 2, M_GenericDrawer);
        break;
    case menu_restartlevel:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_RESTARTLEVEL, 2, M_GenericDrawer);
        break;
    case menu_debug:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_DEBUGMENU, 8, M_DebugDrawer);
        showfullitemvalue[0] = showfullitemvalue[1] = false;
        break;
    case menu_display:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_DISPLAY, 6, M_DisplayDrawer);
        break;
    case menu_returnpaused:
    case menu_returnpaused2:
    case menu_quitmain_no:
    case menu_restart_no:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_GAMEMENU, 4, M_GenericDrawer);
        break;
    case menu_returndisplay:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_GAMEOPTIONS, 4, M_GenericDrawer);
        break;
    case menu_nolights:
        S_StartSound(NULL, sfx_switch2);
        nolights ^= 1;
        break;
    case menu_godmode:
        S_StartSound(NULL, sfx_switch2);
        if(p->cheats & CF_GODMODE)
            p->cheats &= ~CF_GODMODE;
        else
            p->cheats |= CF_GODMODE;
        break;
    case menu_noclip:
        S_StartSound(NULL, sfx_switch2);
        if(p->cheats & CF_NOCLIP)
            p->cheats &= ~CF_NOCLIP;
        else
            p->cheats |= CF_NOCLIP;
        break;
    case menu_allkeys:
        for(i = 0; i < NUMCARDS; i++)
            p->cards[i] = true;

        showfullitemvalue[0] = true;
        break;
    case menu_allweapons:
        S_StartSound(NULL, sfx_switch2);
        for(i = 0; i < NUMWEAPONS; i++)
            p->weaponowned[i] = true;

        if(!p->backpack)
        {
            p->backpack = true;

            for (i = 0; i < NUMAMMO; i++)
                p->maxammo[i] *= 2;
        }

        for (i = 0; i < NUMAMMO; i++)
            p->ammo[i] = p->maxammo[i];

        showfullitemvalue[1] = true;
        break;
    case menu_mapall:
        S_StartSound(NULL, sfx_switch2);
        amCheating ^= 2;
        break;
    case menu_lockmonsters:
        S_StartSound(NULL, sfx_switch2);
        lockmonsters ^= 1;
        break;
    default:
        break;
    }
}

//
// M_Drawer
//

void M_Drawer(void)
{
    I_CheckGFX();

    GFX_ORTHO();

    GFX_POLY_FORMAT =
        POLY_ALPHA(16)              |
        POLY_ID(63)                 |
        POLY_CULL_NONE              |
        POLY_MODULATION;

        GFX_TEX_FORMAT  = 0;
        GFX_PAL_FORMAT  = 0;
        GFX_COLOR       = 0;
        GFX_SCREENRECT();

    if(menudrawfunc)
        menudrawfunc();
}

//
// M_Responder
//

dboolean M_Responder(event_t* ev)
{
    int rc = false;

    if(ev->type == ev_btndown)
    {
        if(ev->data & KEY_START)
        {
            menuactive ^= 1;

            if(menuactive && gamestate == GS_LEVEL)
                M_SetMenu(MENU_GAMEMENU, 4, M_GenericDrawer);

            rc = true;
        }

        if(menuactive)
        {
            if(ev->data & KEY_DOWN)
            {
                rc = true;

                S_StartSound(NULL, sfx_switch1);
                m_itemOn++;
                if(m_itemOn >= m_menuitems)
                    m_itemOn = 0;
            }
            if(ev->data & KEY_UP)
            {
                rc = true;

                S_StartSound(NULL, sfx_switch1);
                m_itemOn--;
                if(m_itemOn < 0)
                    m_itemOn = (m_menuitems - 1);
            }
            if(ev->data & KEY_A)
            {
                rc = true;

                M_MenuChoice();
            }
        }
    }
    else if(ev->type == ev_btnup)
    {
    }

    return rc;
}