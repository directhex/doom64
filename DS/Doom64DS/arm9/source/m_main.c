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
    void(*exec)(void);
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

static void M_GenericHandler(void);
static void M_DebugHandler(void);

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
    menu_lockmonsters,
    menu_returnpaused2,
    NUM_MENU_ITEMS
};

#define MENU_MAINMENU       menu_newgame
#define MENU_GAMEMENU       menu_options2
#define MENU_GAMEOPTIONS    menu_controls
#define MENU_QUITTOMAIN     menu_quitmain_yes
#define MENU_RESTARTLEVEL   menu_restart_yes
#define MENU_DEBUGMENU      menu_nolights

static menuitem_t menulist[NUM_MENU_ITEMS] =
{
    { "New Game", 82, 144, NULL },
    { "Options", 82, 162, NULL },
    { "Options", 82, 64, M_GenericHandler },
    { "Main Menu", 82, 82, M_GenericHandler },
    { "Restart Level", 82, 100, M_GenericHandler },
    { "Debug", 82, 118, M_GenericHandler },
    { "Controls", 82, 64, NULL },
    { "Volume", 82, 82, NULL },
    { "Display", 82, 100, NULL },
    { "/r Return", 82, 118, M_GenericHandler },
    { "Yes", 108, 80, NULL },
    { "No", 108, 98, M_GenericHandler },
    { "Yes", 108, 80, NULL },
    { "No", 108, 98, M_GenericHandler },
    { "No Lights", 32, 40, M_DebugHandler },
    { "God Mode", 32, 52, M_DebugHandler },
    { "No Clip", 32, 64, M_DebugHandler },
    { "All Keys", 32, 76, M_DebugHandler },
    { "All Weapons", 32, 88, M_DebugHandler },
    { "Lock Monsters", 32, 100, M_DebugHandler },
    { "Return", 32, 112, M_GenericHandler }
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
// M_DebugDrawer
//

static void M_DebugDrawer(void)
{
    int i;
    char str[8];

     ST_DrawBigFont(-1, 16, MENUFONTRED, "Debug");

    for(i = 0; i < m_menuitems; i++)
        ST_DrawMessage(menu[i].x, menu[i].y, MENUFONTWHITE, menu[i].name);

    if(nolights)
        sprintf(str, ":On");
    else
        sprintf(str, ":Off");

    ST_DrawMessage(menulist[menu_nolights].x + 160, menulist[menu_nolights].y, MENUFONTWHITE, str);

    if(players[consoleplayer].cheats & CF_GODMODE)
        sprintf(str, ":On");
    else
        sprintf(str, ":Off");

    ST_DrawMessage(menulist[menu_godmode].x + 160, menulist[menu_godmode].y, MENUFONTWHITE, str);

    if(players[consoleplayer].cheats & CF_NOCLIP)
        sprintf(str, ":On");
    else
        sprintf(str, ":Off");

    ST_DrawMessage(menulist[menu_noclip].x + 160, menulist[menu_noclip].y, MENUFONTWHITE, str);

    if(showfullitemvalue[0])
        sprintf(str, ":100%%");
    else
        sprintf(str, ":-");

    ST_DrawMessage(menulist[menu_allkeys].x + 160, menulist[menu_allkeys].y, MENUFONTWHITE, str);

    if(showfullitemvalue[1])
        sprintf(str, ":100%%");
    else
        sprintf(str, ":-");

    ST_DrawMessage(menulist[menu_allweapons].x + 160, menulist[menu_allweapons].y, MENUFONTWHITE, str);

    if(lockmonsters)
        sprintf(str, ":On");
    else
        sprintf(str, ":Off");

    ST_DrawMessage(menulist[menu_lockmonsters].x + 160, menulist[menu_lockmonsters].y, MENUFONTWHITE, str);

    ST_DrawBigFont(menu[m_itemOn].x - 12, menu[m_itemOn].y - 2, 0xFFFFF, "/l");
}

//
// M_DebugHandler
//

static void M_DebugHandler(void)
{
    int i;
    player_t* p;

    p = &players[consoleplayer];

    switch(m_currentmenu + m_itemOn)
    {
    case menu_nolights:
        nolights ^= 1;
        break;
    case menu_godmode:
        if(p->cheats & CF_GODMODE)
            p->cheats &= ~CF_GODMODE;
        else
            p->cheats |= CF_GODMODE;
        break;
    case menu_noclip:
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
    case menu_lockmonsters:
        lockmonsters ^= 1;
        break;
    default:
        break;
    }

    S_StartSound(NULL, sfx_switch2);
}

//
// M_GenericHandler
//

static void M_GenericHandler(void)
{
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
        M_SetMenu(MENU_DEBUGMENU, 7, M_DebugDrawer);
        showfullitemvalue[0] = showfullitemvalue[1] = false;
        break;
    case menu_returnpaused:
    case menu_returnpaused2:
    case menu_quitmain_no:
    case menu_restart_no:
        S_StartSound(NULL, sfx_pistol);
        M_SetMenu(MENU_GAMEMENU, 4, M_GenericDrawer);
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

                if(menu[m_itemOn].exec)
                    menu[m_itemOn].exec();
            }
        }
    }
    else if(ev->type == ev_btnup)
    {
    }

    return rc;
}