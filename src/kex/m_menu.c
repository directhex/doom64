// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Author$
// $Revision$
// $Date$
//
//
// DESCRIPTION:
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif

#ifdef _WIN32
#include <io.h>
#endif

#ifdef _MSC_VER
#include "i_opndir.h"
#else
#include <dirent.h>
#endif

#include <fcntl.h>
#include "doomdef.h"
#include "v_sdl.h"
#include "d_englsh.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "d_main.h"
#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "g_actions.h"
#include "g_game.h"
#include "s_sound.h"
#include "doomstat.h"
#include "sounds.h"
#include "m_menu.h"
#include "m_fixed.h"
#include "d_devstat.h"
#include "r_local.h"
#include "m_shift.h"
#include "m_password.h"
#include "r_wipe.h"
#include "st_stuff.h"
#include "p_saveg.h"
#include "p_setup.h"

#ifdef _WIN32
#include "i_xinput.h"
#endif

//
// definitions
//

#define MENUSAVESTRINGSIZE  16
#define SKULLXOFF           -40	//villsa: changed from -32 to -40
#define SKULLXTEXTOFF       -24
#define LINEHEIGHT          18
#define TEXTLINEHEIGHT      18
#define MENUCOLORRED	    D_RGBA(255, 0, 0, menualphacolor)
#define MENUCOLORWHITE	    D_RGBA(255, 255, 255, menualphacolor)
#define MAXBRIGHTNESS	    100

//
// defaulted values
//

char        *messageBindCommand;
int         quickSaveSlot;                      // -1 = no quicksave slot picked!
int         saveStringEnter = false;            // we are going to be entering a savegame string
int         saveSlot;                           // which slot to save in
int         saveCharIndex;                      // which char we're editing
char        saveOldString[MENUSAVESTRINGSIZE];      // old save description before edit
dboolean    allowmenu = true;                   // can menu be accessed?
dboolean	menuactive = false;
dboolean	mainmenuactive = false;
dboolean    allowclearmenu = true;              // can user hit escape to clear menu?
char        savegamestrings[10][MENUSAVESTRINGSIZE];
char        endstring[160];
dboolean    alphaprevmenu = false;
int         menualphacolor = 0xff;

//
// fade-in/out stuff
//

void M_MenuFadeIn(void);
void M_MenuFadeOut(void);
void(*menufadefunc)(void) = NULL;

//
// static variables
//

static char     MenuBindBuff[256];
static char     MenuBindMessage[256];
static dboolean MenuBindActive = false;
static dboolean showfullitemvalue[3] = { false, false, false};
static int      levelwarp = 0;
static dboolean wireframeon = false;
static dboolean lockmonstersmon = false;
static int      thermowait = 0;
static int      m_aspectRatio = 0;
static int      m_ScreenSize = 1;
static int      m_mousex = 0;
static int      m_mousey = 0;
static int      thumbnail_active = -1;

//------------------------------------------------------------------------
//
// MENU TYPEDEFS
//
//------------------------------------------------------------------------

typedef struct
{
    // -3 = disabled/hidden item, -2 = enter key ok, -1 = disabled, 0 = no cursor here,
    // 1 = ok, 2 = arrows ok, 3 = for sliders
    short status;
    
    char name[64];
    
    // choice = menu item #
    void (*routine)(int choice);
    
    // hotkey in menu
    char alphaKey;
} menuitem_t;

typedef struct
{
    cvar_t* mitem;
    float	mdefault;
} menudefault_t;

typedef struct menu_s
{
    short               numitems;           // # of menu items
    dboolean            textonly;
    struct menu_s*      prevMenu;           // previous menu
    menuitem_t*         menuitems;          // menu items
    void                (*routine)(void);   // draw routine
    char                title[64];
    short               x;
    short               y;                  // x,y of menu
    short               lastOn;             // last item user was on in menu
    dboolean            smallfont;          // draw text using small fonts
    menudefault_t       *defaultitems;      // pointer to default values for cvars
    short               numpageitems;       // number of items to display per page
    short               menupageoffset;
    float               scale;
    char**              hints;
} menu_t;

typedef struct
{
    char	*name;
    char	*action;
}menuaction_t;

short           itemOn;                 // menu item skull is on
short           skullAnimCounter;       // skull animation counter
short           whichSkull;             // which skull to draw

char    msgNames[2][4]          = {"Off","On"};

// current menudef
menu_t* currentMenu;

//------------------------------------------------------------------------
//
// PROTOTYPES
//
//------------------------------------------------------------------------

void M_SetupNextMenu(menu_t *menudef);
void M_StartControlPanel(void);
void M_ClearMenus (void);

void M_QuickSave(void);
void M_QuickLoad(void);

static int M_StringWidth(const char *string);
static int M_StringHeight(const char *string);

static void M_DrawThermo(int x, int y, int thermWidth, float thermDot);
static void M_DoDefaults(int choice);
static void M_Return(int choice);
static void M_ReturnToOptions(int choice);
static void M_SetCvar(cvar_t *cvar, float value);
static void M_DrawSmbString(const char* text, menu_t* menu, int item);
static void M_DrawSaveGameFrontend(menu_t* def);

static dboolean M_SetThumbnail(int which);

//------------------------------------------------------------------------
//
// MAIN MENU
//
//------------------------------------------------------------------------

void M_NewGame(int choice);
void M_Options(int choice);
void M_LoadGame(int choice);
void M_QuitDOOM(int choice);

enum
{
    newgame = 0,
    options,
    loadgame,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {1,"New Game",M_NewGame,'n'},
    {1,"Options",M_Options,'o'},
    {1,"Load Game",M_LoadGame,'l'},
    {1,"Quit Game",M_QuitDOOM,'q'},
};

menu_t  MainDef =
{
    main_end,
    false,
    NULL,
    MainMenu,
    NULL,
    "",
    112,150,
    0,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

//------------------------------------------------------------------------
//
// IN GAME MENU
//
//------------------------------------------------------------------------

void M_SaveGame(int choice);
void M_Features(int choice);
void M_QuitMainMenu(int choice);
void M_ConfirmRestart(int choice);

enum
{
    pause_options = 0,
    pause_mainmenu,
    pause_restartlevel,
    pause_features,
    pause_loadgame,
    pause_savegame,
    pause_quitdoom,
    pause_end
} pause_e;

menuitem_t PauseMenu[]=
{
    {1,"Options",M_Options,'o'},
    {1,"Main Menu",M_QuitMainMenu,'m'},
    {1,"Restart Level",M_ConfirmRestart,'r'},
    {-3,"Features",M_Features,'f'},
    {1,"Load Game",M_LoadGame,'l'},
    {1,"Save Game",M_SaveGame,'s'},
    {1,"Quit Game",M_QuitDOOM,'q'},
};

menu_t  PauseDef =
{
    pause_end,
    false,
    NULL,
    PauseMenu,
    NULL,
    "Pause",
    112,80,
    0,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

//------------------------------------------------------------------------
//
// QUIT GAME PROMPT
//
//------------------------------------------------------------------------

void M_QuitGame(int choice);
void M_QuitGameBack(int choice);

enum
{
    quityes = 0,
    quitno,
    quitend
}; quitprompt_e;

menuitem_t QuitGameMenu[]=
{
    {1,"Yes",M_QuitGame,'y'},
    {1,"No",M_QuitGameBack,'n'},
};

menu_t  QuitDef =
{
    quitend,
    false,
    &PauseDef,
    QuitGameMenu,
    NULL,
    "Quit DOOM?",
    144,112,
    quitno,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_QuitDOOM(int choice)
{
    menu_t* menu = currentMenu;

    M_SetupNextMenu(&QuitDef);
    currentMenu->prevMenu = menu;
}

void M_QuitGame(int choice)
{
    I_Quit();
}

void M_QuitGameBack(int choice)
{
    M_SetupNextMenu(currentMenu->prevMenu);
}

//------------------------------------------------------------------------
//
// EXIT TO MAIN MENU PROMPT
//
//------------------------------------------------------------------------

void M_EndGame(int choice);

enum
{
    PMainYes = 0,
    PMainNo,
    PMain_end
} prompt_e;

menuitem_t PromptMain[]=
{
    {1,"Yes",M_EndGame,'y'},
    {1,"No",M_ReturnToOptions,'n'},
};

menu_t  PromptMainDef =
{
    PMain_end,
    false,
    &PauseDef,
    PromptMain,
    NULL,
    "Quit To Main Menu?",
    144,112,
    PMainNo,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_QuitMainMenu(int choice)
{
    M_SetupNextMenu(&PromptMainDef);
}

void M_EndGame(int choice)
{
    if(choice)
    {
        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            menufadefunc = M_MenuFadeOut;
            alphaprevmenu = true;
        }
    }
    else
    {
        currentMenu->lastOn = itemOn;
        M_ClearMenus();
        gameaction = ga_title;
        currentMenu = &MainDef;
    }
}

//------------------------------------------------------------------------
//
// RESTART LEVEL PROMPT
//
//------------------------------------------------------------------------

void M_RestartLevel(int choice);

enum
{
    RMainYes = 0,
    RMainNo,
    RMain_end
}; rlprompt_e;

menuitem_t RestartConfirmMain[]=
{
    {1,"Yes",M_RestartLevel,'y'},
    {1,"No",M_ReturnToOptions,'n'},
};

menu_t  RestartDef =
{
    RMain_end,
    false,
    &PauseDef,
    RestartConfirmMain,
    NULL,
    "Quit Current Game?",
    144,112,
    RMainNo,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_ConfirmRestart(int choice)
{
    M_SetupNextMenu(&RestartDef);
}

void M_RestartLevel(int choice)
{
    if(!netgame)
    {
        gameaction = ga_loadlevel;
        nextmap = gamemap;
        players[consoleplayer].playerstate = PST_REBORN;
    }

    currentMenu->lastOn = itemOn;
    M_ClearMenus();
}

//------------------------------------------------------------------------
//
// START NEW IN NETGAME NOTIFY
//
//------------------------------------------------------------------------

void M_DrawStartNewNotify(void);
void M_NewGameNotifyResponse(int choice);

enum
{
    SNN_Ok = 0,
    SNN_End
}; startnewnotify_e;

menuitem_t StartNewNotify[]=
{
    {1,"Ok",M_NewGameNotifyResponse,'o'}
};

menu_t  StartNewNotifyDef =
{
    SNN_End,
    false,
    &PauseDef,
    StartNewNotify,
    M_DrawStartNewNotify,
    " ",
    144,112,
    SNN_Ok,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_NewGameNotifyResponse(int choice)
{
    M_SetupNextMenu(&MainDef);
}

void M_DrawStartNewNotify(void)
{
    M_DrawSmbText(-1, 16, MENUCOLORRED , "You Cannot Start");
    M_DrawSmbText(-1, 32, MENUCOLORRED , "A New Game On A Network");
}

//------------------------------------------------------------------------
//
// NEW GAME MENU
//
//------------------------------------------------------------------------

void M_ChooseSkill(int choice);

enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {1,"Be Gentle!",M_ChooseSkill, 'b'},
    {1,"Bring It On!",M_ChooseSkill, 'r'},
    {1,"I Own Doom!",M_ChooseSkill, 'i'},
    {1,"Watch Me Die!",M_ChooseSkill, 'w'},
    {-3,"Hardcore!",M_ChooseSkill, 'h'},
};

menu_t  NewDef =
{
    newg_end,
    false,
    &MainDef,
    NewGameMenu,
    NULL,
    "Choose Your Skill...",
    112,80,
    toorough,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_NewGame(int choice)
{
    if(netgame && !demoplayback)
    {
        M_StartControlPanel();
        M_SetupNextMenu(&StartNewNotifyDef);
        return;
    }

    M_SetupNextMenu(&NewDef);
}

void M_ChooseSkill(int choice)
{
    G_DeferedInitNew(choice,1);
    M_ClearMenus();
    dmemset(passwordData, 0xff, 16);
}


//------------------------------------------------------------------------
//
// OPTIONS MENU
//
//------------------------------------------------------------------------

void M_DrawOptions(void);
void M_Controls(int choice);
void M_Mouse(int choice);
void M_Sound(int choice);
void M_Display(int choice);
void M_Video(int choice);
void M_Misc(int choice);
void M_Password(int choice);
void M_Region(int choice);

enum
{
    options_controls,
    options_mouse,
    options_soundvol,
    options_display,
    options_video,
    options_misc,
    options_password,
    options_region,
    options_return,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"Controls",M_Controls, 'c'},
    {1,"Mouse",M_Mouse,'m'},
    {1,"Sound",M_Sound,'s'},
    {1,"Display",M_Display, 'd'},
    {1,"Video",M_Video, 'v'},
    {1,"Misc",M_Misc, 'i'},
    {1,"Password",M_Password, 'p'},
    {1,"Region",M_Region, 'f'},
    {1,"/r Return",M_Return, 0x20}
};

char* OptionHints[opt_end]=
{
    "change keyboard bindings",
    "configure mouse functionality",
    "adjust sound volume",
    "change how the game is displayed on screen",
    "configure video-specific options",
    "various options for gameplay and other features",
    "enter a password to access a level",
    "configure localization settings",
    NULL
};

menu_t  OptionsDef =
{
    opt_end,
    false,
    &PauseDef,
    OptionsMenu,
    M_DrawOptions,
    "Options",
    170,85,
    0,
    false,
    NULL,
    -1,
    0,
    0.75f,
    OptionHints
};

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

void M_DrawOptions(void)
{
    if(OptionsDef.hints[itemOn] != NULL)
    {
        R_GLSetOrthoScale(0.5f);
        M_DrawSmbText(-1, 410, MENUCOLORWHITE, OptionsDef.hints[itemOn]);
        R_GLSetOrthoScale(OptionsDef.scale);
    }
}

//------------------------------------------------------------------------
//
// REGIONAL MENU
//
//------------------------------------------------------------------------

void M_RegionChoice(int choice);
void M_DrawRegion(void);

enum
{
    region_mode,
    region_lang,
    region_blood,
    region_default,
    region_return,
    region_end
} region_e;

menuitem_t RegionMenu[]=
{
    {2,"Region Mode:", M_RegionChoice, 'r'},
    {2,"Language:",M_RegionChoice, 'l'},
    {2,"Blood Color:",M_RegionChoice, 'b'},
    {-2,"Default",M_DoDefaults,'d'},
    {1,"/r Return",M_Return, 0x20}
};

menudefault_t RegionDefault[] =
{
    { &p_regionmode, 0 },
    { &st_regionmsg, 0 },
    { &m_regionblood, 0 },
    { NULL, -1 }
};

char* RegionHints[region_end]=
{
    "affects the legal screen",
    "change language for hud messages",
    "changes color for blood",
    NULL,
    NULL
};

menu_t  RegionDef =
{
    region_end,
    false,
    &OptionsDef,
    RegionMenu,
    M_DrawRegion,
    "Region",
    130,80,
    0,
    false,
    RegionDefault,
    -1,
    0,
    0.8f,
    RegionHints
};

void M_Region(int choice)
{
    M_SetupNextMenu(&RegionDef);
}

void M_RegionChoice(int choice)
{
    int lump;

    switch(itemOn)
    {
    case region_mode:
        if(choice)
        {
            if(p_regionmode.value < 2)
                M_SetCvar(&p_regionmode, p_regionmode.value + 1);
            else
                CON_CvarSetValue(p_regionmode.name, 2);
        }
        else
        {
            if(p_regionmode.value > 0)
                M_SetCvar(&p_regionmode, p_regionmode.value - 1);
            else
                CON_CvarSetValue(p_regionmode.name, 0);
        }
        break;
    case region_lang:
        if(choice)
            M_SetCvar(&st_regionmsg, 1);
        else
            M_SetCvar(&st_regionmsg, 0);
        break;
    case region_blood:
        if(choice)
            M_SetCvar(&m_regionblood, 1);
        else
            M_SetCvar(&m_regionblood, 0);

        lump = (int)m_regionblood.value;

        if(lump > 1) lump = 1;
        if(lump < 0) lump = 0;

        states[S_494].sprite = SPR_RBLD + lump;
        states[S_495].sprite = SPR_RBLD + lump;
        states[S_496].sprite = SPR_RBLD + lump;
        states[S_497].sprite = SPR_RBLD + lump;

        break;
    }
}

void M_DrawRegion(void)
{
    static const char* bloodcolor[2]    = { "Red", "Green" };
    static const char* language[2]      = { "English", "Japanese" };
    static const char* regionmode[3]    = { "NTSC", "PAL", "NTSC-Japan" };
    rcolor color;

    if(RegionMenu[region_mode].status == 1)
        color = MENUCOLORWHITE;
    else
        color = MENUCOLORRED;

    M_DrawSmbText(RegionDef.x + 136, RegionDef.y + LINEHEIGHT * region_mode, color,
        regionmode[(int)p_regionmode.value]);

    if(RegionMenu[region_lang].status == 1)
        color = MENUCOLORWHITE;
    else
        color = MENUCOLORRED;

    M_DrawSmbText(RegionDef.x + 136, RegionDef.y + LINEHEIGHT * region_lang, color,
        language[(int)st_regionmsg.value]);

    if(RegionMenu[region_blood].status == 1)
        color = MENUCOLORWHITE;
    else
        color = MENUCOLORRED;

    M_DrawSmbText(RegionDef.x + 136, RegionDef.y + LINEHEIGHT * region_blood, color,
        bloodcolor[(int)m_regionblood.value]);

    if(RegionDef.hints[itemOn] != NULL)
    {
        R_GLSetOrthoScale(0.5f);
        M_DrawSmbText(-1, 410, MENUCOLORWHITE, RegionDef.hints[itemOn]);
        R_GLSetOrthoScale(RegionDef.scale);
    }
}

//------------------------------------------------------------------------
//
// MISC MENU
//
//------------------------------------------------------------------------

void M_MiscChoice(int choice);
void M_DrawMisc(void);

enum
{
    misc_header1,
    misc_menufade,
    misc_empty1,
#if 0
    misc_menumouse,
#endif
    misc_header2,
    misc_aim,
    misc_jump,
    misc_context,
    misc_header3,
    misc_wipe,
    misc_texresize,
    misc_frame,
    misc_sprites,
    misc_skylook,
    misc_header4,
    misc_showkey,
    misc_showlocks,
    misc_amobjects,
    misc_amoverlay,
    misc_header5,
    misc_comp_collision,
    misc_default,
    misc_return,
    misc_end
} misc_e;

menuitem_t MiscMenu[]=
{
    {-1,"Menu Options",0 },
    {3,"Menu Fade Speed",M_MiscChoice, 'm' },
    {-1,"",0 },
#if 0
    {2,"Menu Cursor:",M_MiscChoice, 'h'},
#endif
    {-1,"Gameplay",0 },
    {2,"Auto Aim:",M_MiscChoice, 'a'},
    {2,"Jumping:",M_MiscChoice, 'j'},
    {2,"Use Context:",M_MiscChoice, 'u'},
    {-1,"Rendering",0 },
    {2,"Screen Melt:",M_MiscChoice, 's' },
    {2,"Texture Fit:",M_MiscChoice,'t' },
    {2,"Framerate:",M_MiscChoice, 'f' },
    {2,"Sprite Pitch:",M_MiscChoice,'p'},
    {2,"Sky Pitch:",M_MiscChoice,'k'},
    {-1,"Automap",0 },
    {2,"Key Pickups:",M_MiscChoice },
    {2,"Locked Doors:",M_MiscChoice },
    {2,"Draw Objects:",M_MiscChoice },
    {2,"Overlay:",M_MiscChoice },
    {-1,"N64 Compatibility",0 },
    {2,"Collision:",M_MiscChoice,'c' },
    {-2,"Default",M_DoDefaults,'d'},
    {1,"/r Return",M_Return, 0x20}
};

char* MiscHints[misc_end]=
{
    NULL,
    "change transition speeds between switching menus",
    NULL,
#if 0
    NULL,
#endif
    NULL,
    "toggle classic style auto-aiming",
    "toggle the ability to jump",
    "if enabled interactive objects will highlight when near",
    NULL,
    "enable the melt effect when completing a level",
    "set how texture dimentions are stretched",
    "interpolate between frames to achieve smooth framerate",
    "toggles billboard sprite rendering",
    "sky backgrounds will adjust accordingly to player view",
    NULL,
    "display key pickups in automap",
    "colorize locked doors accordingly to the key in automap",
    "set how objects are rendered in automap",
    "render the automap into the player hud",
    NULL,
    "surrounding blockmaps are not checked for an object",
    NULL,
    NULL
};

menudefault_t MiscDefault[] =
{
    { &m_menufadetime, 20 },
#if 0
    { &m_menumouse, 0 },
#endif
    { &p_autoaim, 1 },
    { &p_allowjump, 0 },
    { &p_usecontext, 0 },
    { &r_wipe, 1 },
    { &r_texnonpowresize, 0 },
    { &i_interpolateframes, 0 },
    { &r_rendersprites, 1 },
    { &r_looksky, 0 },
    { &am_showkeymarkers, 0 },
    { &am_showkeycolors, 0 },
    { &am_drawobjects, 0 },
    { &am_overlay, 0 },
    { &compat_collision, 1 },
    { NULL, -1 }
};

menu_t  MiscDef =
{
    misc_end,
    false,
    &OptionsDef,
    MiscMenu,
    M_DrawMisc,
    "Miscellaneous",
    216,108,
    0,
    false,
    MiscDefault,
    14,
    0,
    0.5f,
    MiscHints
};

void M_Misc(int choice)
{
    M_SetupNextMenu(&MiscDef);
}

void M_MiscChoice(int choice)
{
    float slope = 80.0f / 100.0f;

    switch(itemOn)
    {
    case misc_menufade:
        if(choice)
        {
            if(m_menufadetime.value < 100.0f)
                M_SetCvar(&m_menufadetime, m_menufadetime.value + slope);
            else
                CON_CvarSetValue(m_menufadetime.name, 100);
        }
        else
        {
            if(m_menufadetime.value > 20.0f)
                M_SetCvar(&m_menufadetime, m_menufadetime.value - slope);
            else
                CON_CvarSetValue(m_menufadetime.name, 20);
        }
        break;

#if 0
    case misc_menumouse:
        M_SetCvar(&m_menumouse, (float)choice);
        break;
#endif

    case misc_aim:
        M_SetCvar(&p_autoaim, (float)choice);
        break;

    case misc_jump:
        M_SetCvar(&p_allowjump, (float)choice);
        break;

    case misc_context:
        M_SetCvar(&p_usecontext, (float)choice);
        break;

    case misc_wipe:
        M_SetCvar(&r_wipe, (float)choice);
        break;

    case misc_texresize:
        if(choice)
        {
            if(r_texnonpowresize.value < 2)
            {
                M_SetCvar(&r_texnonpowresize, r_texnonpowresize.value + 1);
                R_DumpTextures();
            }
            else
                CON_CvarSetValue(r_texnonpowresize.name, 2);
        }
        else
        {
            if(r_texnonpowresize.value > 0)
            {
                M_SetCvar(&r_texnonpowresize, r_texnonpowresize.value - 1);
                R_DumpTextures();
            }
            else
                CON_CvarSetValue(r_texnonpowresize.name, 0);
        }
        break;

    case misc_frame:
        M_SetCvar(&i_interpolateframes, (float)choice);
        break;

    case misc_sprites:
        if(choice)
            M_SetCvar(&r_rendersprites, 2);
        else
            M_SetCvar(&r_rendersprites, 1);
        break;

    case misc_skylook:
        M_SetCvar(&r_looksky, (float)choice);
        break;

    case misc_showkey:
        M_SetCvar(&am_showkeymarkers, (float)choice);
        break;

    case misc_showlocks:
        M_SetCvar(&am_showkeycolors, (float)choice);
        break;

    case misc_amobjects:
        if(choice)
        {
            if(am_drawobjects.value < 2)
                M_SetCvar(&am_drawobjects, am_drawobjects.value + 1);
            else
                CON_CvarSetValue(am_drawobjects.name, 2);
        }
        else
        {
            if(am_drawobjects.value > 0)
                M_SetCvar(&am_drawobjects, am_drawobjects.value - 1);
            else
                CON_CvarSetValue(am_drawobjects.name, 0);
        }
        break;

    case misc_amoverlay:
        M_SetCvar(&am_overlay, (float)choice);
        break;

    case misc_comp_collision:
        M_SetCvar(&compat_collision, (float)choice);
        break;
    }
}

void M_DrawMisc(void)
{
    static const char* frametype[2] = { "Capped", "Smooth" };
    static const char* mapdisplaytype[2] = { "Hide", "Show" };
    static const char* objectdrawtype[3] = { "Arrows", "Sprites", "Both" };
    static const char* texresizetype[3] = { "Auto", "Padded", "Scaled" };
    int y;

    if(currentMenu->menupageoffset <= misc_menufade+1 &&
        (misc_menufade+1) - currentMenu->menupageoffset < currentMenu->numpageitems)
    {
        y = misc_menufade - currentMenu->menupageoffset;
        M_DrawThermo(MiscDef.x,MiscDef.y+LINEHEIGHT*(y+1), 80, m_menufadetime.value - 20);
    }

#define DRAWMISCITEM(a, b, c) \
    if(currentMenu->menupageoffset <= a && \
        a - currentMenu->menupageoffset < currentMenu->numpageitems) \
    { \
        y = a - currentMenu->menupageoffset; \
        M_DrawSmbText(MiscDef.x + 176, MiscDef.y+LINEHEIGHT*y, MENUCOLORRED, \
            c[(int)b]); \
    }

#if 0
    DRAWMISCITEM(misc_menumouse, m_menumouse.value, msgNames);
#endif

    DRAWMISCITEM(misc_aim, p_autoaim.value, msgNames);
    DRAWMISCITEM(misc_jump, p_allowjump.value, msgNames);
    DRAWMISCITEM(misc_context, p_usecontext.value, mapdisplaytype);
    DRAWMISCITEM(misc_wipe, r_wipe.value, msgNames);
    DRAWMISCITEM(misc_texresize, r_texnonpowresize.value, texresizetype);
    DRAWMISCITEM(misc_frame, i_interpolateframes.value, frametype);
    DRAWMISCITEM(misc_sprites, r_rendersprites.value - 1, msgNames);
    DRAWMISCITEM(misc_skylook, r_looksky.value, msgNames);
    DRAWMISCITEM(misc_showkey, am_showkeymarkers.value, mapdisplaytype);
    DRAWMISCITEM(misc_showlocks, am_showkeycolors.value, mapdisplaytype);
    DRAWMISCITEM(misc_amobjects, am_drawobjects.value, objectdrawtype);
    DRAWMISCITEM(misc_amoverlay, am_overlay.value, msgNames);
    DRAWMISCITEM(misc_comp_collision, compat_collision.value, msgNames);

    if(MiscDef.hints[itemOn] != NULL)
    {
        R_GLSetOrthoScale(0.5f);
        M_DrawSmbText(-1, 410, MENUCOLORWHITE, MiscDef.hints[itemOn]);
        R_GLSetOrthoScale(MiscDef.scale);
    }
}

//------------------------------------------------------------------------
//
// MOUSE MENU
//
//------------------------------------------------------------------------

void M_ChangeSensitivity(int choice);
void M_ChangeMouseAccel(int choice);
void M_ChangeMouseLook(int choice);
void M_ChangeMouseInvert(int choice);
void M_DrawMouse(void);

enum
{
    mouse_sensx,
    mouse_empty1,
    mouse_sensy,
    mouse_empty2,
    mouse_accel,
    mouse_empty3,
    mouse_look,
    mouse_invert,
    mouse_default,
    mouse_return,
    mouse_end
} mouse_e;

menuitem_t MouseMenu[]=
{
    {3,"Mouse Sensitivity X",M_ChangeSensitivity, 'x'},
    {-1,"",0},
    {3,"Mouse Sensitivity Y",M_ChangeSensitivity, 'y'},
    {-1,"",0},
    {3, "Mouse Acceleration",M_ChangeMouseAccel, 'a'},
    {-1, "",0},
    {2,"Mouse Look:",M_ChangeMouseLook,'l'},
    {2,"Invert Look:",M_ChangeMouseInvert, 'i'},
    {-2,"Default",M_DoDefaults,'d'},
    {1,"/r Return",M_Return, 0x20}
};

menudefault_t MouseDefault[] =
{
    { &v_msensitivityx, 5 },
    { &v_msensitivityy, 5 },
    { &v_macceleration, 0 },
    { &v_mlook, 0 },
    { &v_mlookinvert, 0 },
    { NULL, -1 }
};

menu_t  MouseDef =
{
    mouse_end,
    false,
    &OptionsDef,
    MouseMenu,
    M_DrawMouse,
    "Mouse",
    96,48,
    0,
    false,
    MouseDefault,
    -1,
    0,
    1.0f,
    NULL
};

void M_Mouse(int choice)
{
    M_SetupNextMenu(&MouseDef);
}

void M_DrawMouse(void)
{
    M_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_sensx+1),MAXSENSITIVITY, v_msensitivityx.value);
    M_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_sensy+1),MAXSENSITIVITY, v_msensitivityy.value);
    
    M_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_accel+1),20, v_macceleration.value);
    
    M_DrawSmbText(MouseDef.x + 144, MouseDef.y+LINEHEIGHT*mouse_look, MENUCOLORRED,
        msgNames[(int)v_mlook.value]);
    M_DrawSmbText(MouseDef.x + 144, MouseDef.y+LINEHEIGHT*mouse_invert, MENUCOLORRED,
        msgNames[(int)v_mlookinvert.value]);
}

void M_ChangeSensitivity(int choice)
{
    float slope = (float)MAXSENSITIVITY / 100.0f;
    switch(choice)
    {
    case 0:
        switch(itemOn)
        {
        case mouse_sensx:
            if(v_msensitivityx.value > 0.0f)
                M_SetCvar(&v_msensitivityx, v_msensitivityx.value - slope);
            else
                CON_CvarSetValue(v_msensitivityx.name, 0);
            break;
        case mouse_sensy:
            if(v_msensitivityy.value > 0.0f)
                M_SetCvar(&v_msensitivityy, v_msensitivityy.value - slope);
            else
                CON_CvarSetValue(v_msensitivityy.name, 0);
            break;
        }
        break;
        case 1:
            switch(itemOn)
            {
            case mouse_sensx:
                if(v_msensitivityx.value < (float)MAXSENSITIVITY)
                    M_SetCvar(&v_msensitivityx, v_msensitivityx.value + slope);
                else
                    CON_CvarSetValue(v_msensitivityx.name, (float)MAXSENSITIVITY);
                break;
            case mouse_sensy:
                if(v_msensitivityy.value < (float)MAXSENSITIVITY)
                    M_SetCvar(&v_msensitivityy, v_msensitivityy.value + slope);
                else
                    CON_CvarSetValue(v_msensitivityy.name, (float)MAXSENSITIVITY);
                break;
            }
            break;
    }
}

void M_ChangeMouseAccel(int choice)
{
    float slope = 20.0f / 100.0f;
    switch(choice)
    {
    case 0:
        if(v_macceleration.value > 0.0f)
            M_SetCvar(&v_macceleration, v_macceleration.value - slope);
        else CON_CvarSetValue(v_macceleration.name, 0);
        break;
    case 1:
        if(v_macceleration.value < 20.0f)
            M_SetCvar(&v_macceleration, v_macceleration.value + slope);
        else CON_CvarSetValue(v_macceleration.name, 20);
        break;
    }
    V_MouseAccelChange();
}

void M_ChangeMouseLook(int choice)
{
    M_SetCvar(&v_mlook, (float)choice);
}

void M_ChangeMouseInvert(int choice)
{
    M_SetCvar(&v_mlookinvert, (float)choice);
}

//------------------------------------------------------------------------
//
// DISPLAY MENU
//
//------------------------------------------------------------------------

void M_ChangeBrightness(int choice);
void M_ChangeMessages(int choice);
void M_ToggleHudDraw(int choice);
void M_ToggleFlashOverlay(int choice);
void M_ToggleDamageHud(int choice);
void M_ChangeCrosshair(int choice);
void M_ChangeOpacity(int choice);
void M_DrawDisplay(void);

enum
{
    dbrightness,
    display_empty1,
    messages,
    statusbar,
    display_flash,
    display_damage,
    display_crosshair,
    display_opacity,
    display_empty2,
    e_default,
    display_return,
    display_end
} display_e;

menuitem_t DisplayMenu[]=
{
    {3,"Brightness",M_ChangeBrightness, 'b'},
    {-1,"",0},
    {2,"Messages:",M_ChangeMessages, 'm'},
    {2,"Status Bar:",M_ToggleHudDraw, 's'},
    {2,"Hud Flash:",M_ToggleFlashOverlay, 'f'},
    {2,"Damage Hud:",M_ToggleDamageHud, 'd'},
    {2,"Crosshair:",M_ChangeCrosshair, 'c'},
    {3,"Crosshair Opacity",M_ChangeOpacity, 'o'},
    {-1,"",0},
    {-2,"Default",M_DoDefaults, 'd'},
    {1,"/r Return",M_Return, 0x20}
};

char* DisplayHints[display_end]=
{
    "change light color intensity",
    NULL,
    "toggle messages displaying on hud",
    "change look and style for hud",
    "change how flashes are rendered",
    "toggle hud indicators when taking damage",
    "toggle crosshair",
    "change opacity for crosshairs",
    NULL,
    NULL,
    NULL
};

menudefault_t DisplayDefault[] =
{
    { &i_brightness, 0 },
    { &m_messages, 1 },
    { &st_drawhud, 1 },
    { &p_damageindicator, 0 },
    { &st_crosshair, 0 },
    { &st_crosshairopacity, 80 },
    { NULL, -1 }
};

menu_t  DisplayDef =
{
    display_end,
    false,
    &OptionsDef,
    DisplayMenu,
    M_DrawDisplay,
    "Display",
    140,60,
    0,
    false,
    DisplayDefault,
    -1,
    0,
    0.8f,
    DisplayHints
};

void M_Display(int choice)
{
    M_SetupNextMenu(&DisplayDef);
}

void M_DrawDisplay(void)
{
    static const char* hudtype[3] = { "Off", "Classic", "Absolution" };
    static const char* flashtype[2] = { "Blending", "Overlay" };
    
    M_DrawThermo(DisplayDef.x, DisplayDef.y+LINEHEIGHT*(dbrightness+1), MAXBRIGHTNESS, i_brightness.value);
    M_DrawSmbText(DisplayDef.x + 136, DisplayDef.y+LINEHEIGHT*messages, MENUCOLORRED,
        msgNames[(int)m_messages.value]);
    M_DrawSmbText(DisplayDef.x + 136, DisplayDef.y+LINEHEIGHT*statusbar, MENUCOLORRED,
        hudtype[(int)st_drawhud.value]);
    M_DrawSmbText(DisplayDef.x + 136, DisplayDef.y+LINEHEIGHT*display_flash, MENUCOLORRED,
        flashtype[(int)st_flashoverlay.value]);
    M_DrawSmbText(DisplayDef.x + 136, DisplayDef.y+LINEHEIGHT*display_damage, MENUCOLORRED,
        msgNames[(int)p_damageindicator.value]);

    if(st_crosshair.value <= 0)
    {
        M_DrawSmbText(DisplayDef.x + 136, DisplayDef.y+LINEHEIGHT*display_crosshair, MENUCOLORRED,
        msgNames[0]);
    }
    else
    {
        ST_DrawCrosshair(DisplayDef.x + 136, DisplayDef.y+LINEHEIGHT*display_crosshair,
            (int)st_crosshair.value, 1, MENUCOLORWHITE);
    }

    M_DrawThermo(DisplayDef.x, DisplayDef.y+LINEHEIGHT*(display_opacity+1),
        255, st_crosshairopacity.value);

    if(DisplayDef.hints[itemOn] != NULL)
    {
        R_GLSetOrthoScale(0.5f);
        M_DrawSmbText(-1, 415, MENUCOLORWHITE, DisplayDef.hints[itemOn]);
        R_GLSetOrthoScale(DisplayDef.scale);
    }
}

void M_ChangeBrightness(int choice)
{
    switch(choice)
    {
    case 0:
        if(i_brightness.value > 0.0f)
            M_SetCvar(&i_brightness, i_brightness.value - 1);
        else i_brightness.value = 0;
        break;
    case 1:
        if(i_brightness.value < (int)MAXBRIGHTNESS)
            M_SetCvar(&i_brightness, i_brightness.value + 1);
        else i_brightness.value = (int)MAXBRIGHTNESS;
        break;
    }
    
    R_RefreshBrightness();
}

void M_ChangeMessages(int choice)
{
    M_SetCvar(&m_messages, (float)choice);

    if(choice)
        players[consoleplayer].message = MSGON;
    else
        players[consoleplayer].message = MSGOFF;
}

void M_ToggleHudDraw(int choice)
{
    if(choice)
    {
        if(st_drawhud.value == 1)
            M_SetCvar(&st_drawhud, 2);
        else if(st_drawhud.value <= 0)
            M_SetCvar(&st_drawhud, 1);
    }
    else
    {
        if(st_drawhud.value == 1)
            M_SetCvar(&st_drawhud, 0);
        else if(st_drawhud.value >= 2)
            M_SetCvar(&st_drawhud, 1);
    }
}

void M_ToggleDamageHud(int choice)
{
    M_SetCvar(&p_damageindicator, (float)choice);
}

void M_ToggleFlashOverlay(int choice)
{
    M_SetCvar(&st_flashoverlay, (float)choice);
}

void M_ChangeCrosshair(int choice)
{
    int i = (int)st_crosshair.value;

    if(choice)
    {
        i++;
        if(i <= st_crosshairs)
            M_SetCvar(&st_crosshair, (float)i);
    }
    else
    {
        i--;
        if(i >= 0)
            M_SetCvar(&st_crosshair, (float)i);
    }
}

void M_ChangeOpacity(int choice)
{
    float slope = 255.0f / 100.0f;

    if(choice)
    {
        if(st_crosshairopacity.value < 255.0f)
            M_SetCvar(&st_crosshairopacity, st_crosshairopacity.value + slope);
        else
            CON_CvarSetValue(st_crosshairopacity.name, 255);
    }
    else
    {
        if(st_crosshairopacity.value > 0.0f)
            M_SetCvar(&st_crosshairopacity, st_crosshairopacity.value - slope);
        else
            CON_CvarSetValue(st_crosshairopacity.name, 0);
    }
}

//------------------------------------------------------------------------
//
// VIDEO MENU
//
//------------------------------------------------------------------------

void M_ChangeGammaLevel(int choice);
void M_ChangeFilter(int choice);
void M_ChangeWindowed(int choice);
void M_ChangeRatio(int choice);
void M_ChangeResolution(int choice);
void M_ChangeVSync(int choice);
void M_ChangeDepthSize(int choice);
void M_ChangeBufferSize(int choice);
void M_DrawVideo(void);

enum
{
    video_dgamma,
    video_empty1,
    filter,
    windowed,
    vsync,
    depth,
    buffer,
    ratio,
    resolution,
    v_default,
    video_return,
    video_end
} video_e;

menuitem_t VideoMenu[]=
{
    {3,"Gamma Correction",M_ChangeGammaLevel, 'g'},
    {-1,"",0},
    {2,"Filter:",M_ChangeFilter, 'f'},
    {2,"Windowed:",M_ChangeWindowed,'w'},
    {2,"Vsync:",M_ChangeVSync, 'v'},
    {2,"Depth Size:",M_ChangeDepthSize, 'd'},
    {2,"Buffer Size:",M_ChangeBufferSize, 'b'},
    {2,"Aspect Ratio:",M_ChangeRatio,'a'},
    {2,"Resolution:",M_ChangeResolution, 'r'},
    {-2,"Default",M_DoDefaults,'e'},
    {1,"/r Return",M_Return, 0x20}
};

menudefault_t VideoDefault[] =
{
    { &i_gamma, 0 },
    { &r_filter, 0 },
    { &v_windowed, 1 },
    { &v_vsync, 1 },
    { &v_depthsize, 24 },
    { &v_buffersize, 32 },
    { NULL, -1 },
    { NULL, -1 }
};

menu_t  VideoDef =
{
    video_end,
    false,
    &OptionsDef,
    VideoMenu,
    M_DrawVideo,
    "Video",
    136,80,
    0,
    false,
    VideoDefault,
    12,
    0,
    0.65f,
    NULL
};

#define MAX_RES4_3  9
static const int Resolution4_3[MAX_RES4_3][2] =
{
    {   320,    240     },
    {   640,    480     },
    {   768,    576     },
    {   800,    600     },
    {   1024,   768     },
    {   1152,   864     },
    {   1280,   960     },
    {   1400,   1050    },
    {   1600,   1200    }
};

#define MAX_RES16_9  4
static const int Resolution16_9[MAX_RES16_9][2] =
{
    {   854,    480     },
    {   1280,   720     },
    {   1366,   768     },
    {   1920,   1080    }
};

#define MAX_RES16_10  5
static const int Resolution16_10[MAX_RES16_10][2] =
{
    {   320,    200     },
    {   1280,   800     },
    {   1440,   900     },
    {   1680,   1050    },
    {   1920,   1200    }
};

static const float ratioVal[3] =
{
    4.0f / 3.0f,
    16.0f / 9.0f,
    16.0f / 10.0f
};

static char gammamsg[21][28] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4,
    GAMMALVL5,
    GAMMALVL6,
    GAMMALVL7,
    GAMMALVL8,
    GAMMALVL9,
    GAMMALVL10,
    GAMMALVL11,
    GAMMALVL12,
    GAMMALVL13,
    GAMMALVL14,
    GAMMALVL15,
    GAMMALVL16,
    GAMMALVL17,
    GAMMALVL18,
    GAMMALVL19,
    GAMMALVL20
};

void M_Video(int choice)
{
    float checkratio;
    int i;

    M_SetupNextMenu(&VideoDef);

    checkratio = v_width.value / v_height.value;

    if(dfcmp(checkratio, ratioVal[2]))
        m_aspectRatio = 2;
    else if(dfcmp(checkratio, ratioVal[1]))
        m_aspectRatio = 1;
    else
        m_aspectRatio = 0;

    switch(m_aspectRatio)
    {
    case 0:
        for(i = 0; i < MAX_RES4_3; i++)
        {
            if((int)v_width.value == Resolution4_3[i][0])
            {
                m_ScreenSize = i;
                return;
            }
        }
        break;
    case 1:
        for(i = 0; i < MAX_RES16_9; i++)
        {
            if((int)v_width.value == Resolution16_9[i][0])
            {
                m_ScreenSize = i;
                return;
            }
        }
        break;
    case 2:
        for(i = 0; i < MAX_RES16_10; i++)
        {
            if((int)v_width.value == Resolution16_10[i][0])
            {
                m_ScreenSize = i;
                return;
            }
        }
        break;
    }

    m_ScreenSize = 1;
}

void M_DrawVideo(void)
{
    static const char* filterType[2] = { "Linear", "Nearest" };
    static const char* ratioName[3] = { "4 : 3", "16 : 9", "16 : 10" };
    static char bitValue[8];
    char res[16];
    int y;

    if(currentMenu->menupageoffset <= video_dgamma + 1 &&
        (video_dgamma+1) - currentMenu->menupageoffset < currentMenu->numpageitems)
    {
        y = video_dgamma - currentMenu->menupageoffset;
        M_DrawThermo(VideoDef.x, VideoDef.y + LINEHEIGHT*(y + 1), 20, i_gamma.value);
    }

#define DRAWVIDEOITEM(a, b) \
    if(currentMenu->menupageoffset <= a && \
        a - currentMenu->menupageoffset < currentMenu->numpageitems) \
    { \
        y = a - currentMenu->menupageoffset; \
        M_DrawSmbText(VideoDef.x + 176, VideoDef.y+LINEHEIGHT*y, MENUCOLORRED, b); \
    }

#define DRAWVIDEOITEM2(a, b, c) DRAWVIDEOITEM(a, c[(int)b])

    DRAWVIDEOITEM2(filter, r_filter.value, filterType);
    DRAWVIDEOITEM2(windowed, v_windowed.value, msgNames);
    DRAWVIDEOITEM2(ratio, m_aspectRatio, ratioName);

    sprintf(res, "%ix%i", (int)v_width.value, (int)v_height.value);
    DRAWVIDEOITEM(resolution, res);

    DRAWVIDEOITEM2(vsync, v_vsync.value, msgNames);

    if(currentMenu->menupageoffset <= depth &&
        depth - currentMenu->menupageoffset < currentMenu->numpageitems)
    {
        if(v_depthsize.value == 8)
            dsnprintf(bitValue, 1, "8");
        else if(v_depthsize.value == 16)
            dsnprintf(bitValue, 2, "16");
        else if(v_depthsize.value == 24)
            dsnprintf(bitValue, 2, "24");
        else
            dsnprintf(bitValue, 8, "Invalid");

        y = depth - currentMenu->menupageoffset;
        M_DrawSmbText(VideoDef.x + 176, VideoDef.y+LINEHEIGHT*y, MENUCOLORRED, bitValue);
    }

    if(currentMenu->menupageoffset <= buffer &&
        buffer - currentMenu->menupageoffset < currentMenu->numpageitems)
    {
        if(v_buffersize.value == 8)
            dsnprintf(bitValue, 1, "8");
        else if(v_buffersize.value == 16)
            dsnprintf(bitValue, 2, "16");
        else if(v_buffersize.value == 24)
            dsnprintf(bitValue, 2, "24");
        else if(v_buffersize.value == 32)
            dsnprintf(bitValue, 2, "32");
        else
            dsnprintf(bitValue, 8, "Invalid");

        y = buffer - currentMenu->menupageoffset;
        M_DrawSmbText(VideoDef.x + 176, VideoDef.y+LINEHEIGHT*y, MENUCOLORRED, bitValue);
    }
    
    M_DrawText(145, 308, MENUCOLORWHITE, VideoDef.scale, false,
        "Changes will take effect\nafter restarting the game..");

    R_GLSetOrthoScale(VideoDef.scale);
}

void M_ChangeGammaLevel(int choice)
{
    float slope = 20.0f / 100.0f;
    switch(choice)
    {
    case 0:
        if(i_gamma.value > 0.0f)
            M_SetCvar(&i_gamma, i_gamma.value - slope);
        else
            CON_CvarSetValue(i_gamma.name, 0);
        break;
    case 1:
        if(i_gamma.value < 20.0f)
            M_SetCvar(&i_gamma, i_gamma.value + slope);
        else
            CON_CvarSetValue(i_gamma.name, 20);
        break;
    case 2:
        (int)i_gamma.value++;
        if (i_gamma.value > 20)
            i_gamma.value = 0;
        players[consoleplayer].message=gammamsg[(int)i_gamma.value];
        break;
    }
    R_DumpTextures();
}

void M_ChangeFilter(int choice)
{
    M_SetCvar(&r_filter, (float)choice);
    
    R_DumpTextures();
    R_GLSetFilter();
}

void M_ChangeWindowed(int choice)
{
    if(choice)
        M_SetCvar(&v_windowed, 1);
    else
        M_SetCvar(&v_windowed, 0);
}

static void M_SetResolution(void)
{
    int width = SCREENWIDTH;
    int height = SCREENHEIGHT;

    switch(m_aspectRatio)
    {
    case 0:
        width = Resolution4_3[m_ScreenSize][0];
        height = Resolution4_3[m_ScreenSize][1];
        break;
    case 1:
        width = Resolution16_9[m_ScreenSize][0];
        height = Resolution16_9[m_ScreenSize][1];
        break;
    case 2:
        width = Resolution16_10[m_ScreenSize][0];
        height = Resolution16_10[m_ScreenSize][1];
        break;
    }

    M_SetCvar(&v_width, (float)width);
    M_SetCvar(&v_height, (float)height);
}

void M_ChangeRatio(int choice)
{
    int max = 0;

    if(choice)
        m_aspectRatio = MIN(m_aspectRatio++, 2);
    else
        m_aspectRatio = MAX(m_aspectRatio--, 0);

    switch(m_aspectRatio)
    {
    case 0:
        max = MAX_RES4_3;
        break;
    case 1:
        max = MAX_RES16_9;
        break;
    case 2:
        max = MAX_RES16_10;
        break;
    }

    m_ScreenSize = MIN(m_ScreenSize, max - 1);

    M_SetResolution();
}

void M_ChangeResolution(int choice)
{
    int max = 0;

    switch(m_aspectRatio)
    {
    case 0:
        max = MAX_RES4_3;
        break;
    case 1:
        max = MAX_RES16_9;
        break;
    case 2:
        max = MAX_RES16_10;
        break;
    }

    if(choice)
        m_ScreenSize = MIN(m_ScreenSize++, max - 1);
    else
        m_ScreenSize = MAX(m_ScreenSize--, 0);

    M_SetResolution();
}

void M_ChangeVSync(int choice)
{
    if(choice)
        M_SetCvar(&v_vsync, 1);
    else
        M_SetCvar(&v_vsync, 0);
}

void M_ChangeDepthSize(int choice)
{
    if(choice)
    {
        if(v_depthsize.value == 8)
        {
            M_SetCvar(&v_depthsize, 16);
            return;
        }
        else if(v_depthsize.value == 16)
        {
            M_SetCvar(&v_depthsize, 24);
            return;
        }
        else
            return;
    }
    else
    {
        if(v_depthsize.value == 24)
        {
            M_SetCvar(&v_depthsize, 16);
            return;
        }
        else if(v_depthsize.value == 16)
        {
            M_SetCvar(&v_depthsize, 8);
            return;
        }
        else
            return;
    }
}

void M_ChangeBufferSize(int choice)
{
    if(choice)
    {
        if(v_buffersize.value == 8)
        {
            M_SetCvar(&v_buffersize, 16);
            return;
        }
        else if(v_buffersize.value == 16)
        {
            M_SetCvar(&v_buffersize, 24);
            return;
        }
        else if(v_buffersize.value == 24)
        {
            M_SetCvar(&v_buffersize, 32);
            return;
        }
        else
            return;
    }
    else
    {
        if(v_buffersize.value == 32)
        {
            M_SetCvar(&v_buffersize, 24);
            return;
        }
        else if(v_buffersize.value == 24)
        {
            M_SetCvar(&v_buffersize, 16);
            return;
        }
        else if(v_buffersize.value == 16)
        {
            M_SetCvar(&v_buffersize, 8);
            return;
        }
        else
            return;
    }
}

//------------------------------------------------------------------------
//
// PASSWORD MENU
//
//------------------------------------------------------------------------

void M_DrawPassword(void);

menuitem_t PasswordMenu[32];

menu_t PasswordDef =
{
    32,
    false,
    &OptionsDef,
    PasswordMenu,
    M_DrawPassword,
    "Password",
    92,60,
    0,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

static dboolean passInvalid = false;
static int		curPasswordSlot = 0;
static int		passInvalidTic = 0;

void M_Password(int choice)
{
    M_SetupNextMenu(&PasswordDef);
    passInvalid = false;
    passInvalidTic = 0;
    
    for(curPasswordSlot = 0; curPasswordSlot < 15; curPasswordSlot++)
    {
        if(passwordData[curPasswordSlot] == 0xff)
            break;
    }
}

void M_DrawPassword(void)
{
    char password[2];
    byte *passData;
    int i = 0;
    
#ifdef _USE_XINPUT  // XINPUT
    if(!xgamepad.connected)
#endif
    {
        M_DrawSmbText(-1, 240 - 48, MENUCOLORWHITE , "Press Backspace To Change");
        M_DrawSmbText(-1, 240 - 32, MENUCOLORWHITE , "Press Escape To Return");
    }
    
    if(passInvalid)
    {
        if(!passInvalidTic--)
        {
            passInvalidTic = 0;
            passInvalid = false;
        }
        
        if(passInvalidTic & 16)
        {
            M_DrawSmbText(-1, 240 - 80, MENUCOLORWHITE, "Invalid Password");
            return;
        }
    }
    
    dmemset(password, 0, 2);
    passData = passwordData;
    
    for(i = 0; i < 19; i++)
    {
        if(i == 4 || i == 9 || i == 14)
            password[0] = 0x20;
        else
        {
            if(*passData == 0xff)
                password[0] = '.';
            else
                password[0] = passwordChar[*passData];
            
            passData++;
        }
        
        M_DrawSmbText((currentMenu->x + (i * 12)) - 48, 240 - 80, MENUCOLORRED, password);
    }
}

static void M_PasswordSelect(void)
{
    S_StartSound(NULL, sfx_switch2);
    passwordData[curPasswordSlot++] = (byte)itemOn;
    if(curPasswordSlot > 15)
    {
        static const char* hecticdemo = "rvnh3ct1cd3m0???";
        int i;

        for(i = 0; i < 16; i++)
        {
            if(passwordChar[passwordData[i]] != hecticdemo[i])
                break;
        }

        if(i >= 16)
        {
            rundemo4 = true;
            M_Return(0);

            return;
        }

        if(!M_DecodePassword(1))
        {
            passInvalid = true;
            passInvalidTic = 64;
        }
        else
        {
            M_DecodePassword(0);
            G_DeferedInitNew(gameskill, gamemap);
            doPassword = true;
            currentMenu->lastOn = itemOn;
            M_ClearMenus();
            
            return;
        }

        curPasswordSlot = 15;
    }
}

static void M_PasswordDeSelect(void)
{
    S_StartSound(NULL, sfx_switch2);
    if(passwordData[curPasswordSlot] == 0xff)
        curPasswordSlot--;
    
    if(curPasswordSlot < 0)
        curPasswordSlot = 0;
    
    passwordData[curPasswordSlot] = 0xff;
}

//------------------------------------------------------------------------
//
// SOUND MENU
//
//------------------------------------------------------------------------

void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_DrawSound(void);

enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_default,
    sound_return,
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {3,"Sound Volume",M_SfxVol,'s'},
    {-1,"",0},
    {3,"Music Volume",M_MusicVol,'m'},
    {-1,"",0},
    {-2,"Default",M_DoDefaults,'d'},
    {1,"/r Return",M_Return, 0x20}
};

menudefault_t SoundDefault[] =
{
    { &s_sfxvol, 80 },
    { &s_musvol, 80 },
    { NULL, -1 }
};

menu_t  SoundDef =
{
    sound_end,
    false,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    "Sound",
    96,60,
    0,
    false,
    SoundDefault,
    -1,
    0,
    1.0f,
    NULL
};

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_DrawSound(void)
{
    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1), 100, s_sfxvol.value);
    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1), 100, s_musvol.value);
}

void M_SfxVol(int choice)
{
    float slope = 1.0f;
    switch(choice)
    {
    case 0:
        if(s_sfxvol.value > 0.0f)
            M_SetCvar(&s_sfxvol, s_sfxvol.value - slope);
        else
            CON_CvarSetValue(s_sfxvol.name, 0);
        break;
    case 1:
        if(s_sfxvol.value < 100.0f)
            M_SetCvar(&s_sfxvol, s_sfxvol.value + slope);
        else
            CON_CvarSetValue(s_sfxvol.name, 100);
        break;
    }

    S_SetSoundVolume(s_sfxvol.value);
}

void M_MusicVol(int choice)
{
    float slope = 1.0f;
    switch(choice)
    {
    case 0:
        if(s_musvol.value > 0.0f)
            M_SetCvar(&s_musvol, s_musvol.value - slope);
        else
            CON_CvarSetValue(s_musvol.name, 0);
        break;
    case 1:
        if(s_musvol.value < 100.0f)
            M_SetCvar(&s_musvol, s_musvol.value + slope);
        else
            CON_CvarSetValue(s_musvol.name, 100);
        break;
    }

    S_SetMusicVolume(s_musvol.value);
}

//------------------------------------------------------------------------
//
// FEATURES MENU
//
//------------------------------------------------------------------------

void M_DoFeature(int choice);
void M_DrawFeaturesMenu(void);

enum
{
    features_levels = 0,
    features_invulnerable,
    features_healthboost,
    features_weapons,
    features_mapeverything,
    features_securitykeys,
    features_lockmonsters,
    features_noclip,
    features_wireframe,
    features_end
} features_e;

#define FEATURESWARPLEVEL	"Warp To Level:"
#define FEATURESWARPFUN		"Warp To Fun:"
#define FEATURESWARPSINGLE  "Warp To Pwad:"

menuitem_t FeaturesMenu[]=
{
    {2,FEATURESWARPLEVEL,M_DoFeature,'l'},
    {2,"Invulnerable:",M_DoFeature,'i'},
    {2,"Health Boost:",M_DoFeature,'h'},
    {2,"Weapons:",M_DoFeature,'w'},
    {2,"Map Everything:",M_DoFeature,'m'},
    {2,"Security Keys:",M_DoFeature,'k'},
    {2,"Lock Monsters:",M_DoFeature,'o'},
    {2,"Wall Blocking:",M_DoFeature,'w'},
    {2,"Wireframe Mode:",M_DoFeature,'r'},
};

menu_t  featuresDef =
{
    features_end,
    false,
    &PauseDef,
    FeaturesMenu,
    M_DrawFeaturesMenu,
    "Features",
    56,56,
    0,
    true,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_Features(int choice)
{
    M_SetupNextMenu(&featuresDef);
    
    showfullitemvalue[0]=showfullitemvalue[1]=showfullitemvalue[2]=false;
}

void M_DrawFeaturesMenu(void)
{
    mapdef_t* map = P_GetMapInfo(levelwarp + 1);

    /*Warp To Level*/
    M_DrawSmbString(map->mapname, &featuresDef, features_levels);
    
    /*Lock Monsters Mode*/
    M_DrawSmbString(msgNames[(int)sv_lockmonsters.value], &featuresDef, features_lockmonsters);
    
    /*Wireframe Mode*/
    M_DrawSmbString(msgNames[wireframeon], &featuresDef, features_wireframe);
    
    /*Invulnerable*/
    M_DrawSmbString(msgNames[players[consoleplayer].cheats & CF_GODMODE ? 1 : 0],
        &featuresDef, features_invulnerable);

    /*No Clip*/
    M_DrawSmbString(msgNames[players[consoleplayer].cheats & CF_NOCLIP ? 1 : 0],
        &featuresDef, features_noclip);
    
    /*Map Everything*/
    M_DrawSmbString(msgNames[amCheating==2 ? 1 : 0], &featuresDef, features_mapeverything);
    
    /*Full Health*/
    M_DrawSmbString(showfullitemvalue[0] ? "100%%" : "-", &featuresDef, features_healthboost);
    
    /*Full Weapons*/
    M_DrawSmbString(showfullitemvalue[1] ? "100%%" : "-", &featuresDef, features_weapons);
    
    /*Full Keys*/
    M_DrawSmbString(showfullitemvalue[2] ? "100%%" : "-", &featuresDef, features_securitykeys);

    switch(map->type)
    {
    case 0:
        sprintf(featuresDef.menuitems[features_levels].name, FEATURESWARPLEVEL);
        break;
    case 1:
        sprintf(featuresDef.menuitems[features_levels].name, FEATURESWARPFUN);
        break;
    case 2:
        sprintf(featuresDef.menuitems[features_levels].name, FEATURESWARPSINGLE);
        break;
    }
}

void M_DoFeature(int choice)
{
    int i = 0;
    
    switch(itemOn)
    {
    case features_levels:
        if(choice)
        {
            levelwarp++;
            if(levelwarp >= 31)
                levelwarp = 31;
        }
        else
        {
            levelwarp--;
            if(levelwarp <= 0)
                levelwarp = 0;
        }
        break;
        
    case features_invulnerable:
        if(choice)
            players[consoleplayer].cheats |= CF_GODMODE;
        else
            players[consoleplayer].cheats &= ~CF_GODMODE;
        break;

    case features_noclip:
        if(choice)
            players[consoleplayer].cheats |= CF_NOCLIP;
        else
            players[consoleplayer].cheats &= ~CF_NOCLIP;
        break;
        
    case features_healthboost:
        showfullitemvalue[0] = true;
        players[consoleplayer].health = 100;
        players[consoleplayer].mo->health = 100;
        break;
        
    case features_weapons:
        showfullitemvalue[1] = true;
        
        for (i = 0; i < NUMWEAPONS; i++)
            players[consoleplayer].weaponowned[i] = true;
        
        if (!players[consoleplayer].backpack)
        {
            players[consoleplayer].backpack = true;
            for (i = 0; i < NUMAMMO; i++)
                players[consoleplayer].maxammo[i] *= 2;
        }
        
        for (i = 0; i < NUMAMMO; i++)
            players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];
        
        break;
        
    case features_mapeverything:
        amCheating = choice ? 2 : 0;
        break;
        
    case features_securitykeys:
        showfullitemvalue[2] = true;
        
        for (i = 0; i < NUMCARDS; i++)
            players[consoleplayer].cards[i] = true;
        
        break;
        
    case features_lockmonsters:
        if(choice)
            CON_CvarSetValue(sv_lockmonsters.name, 1);
        else
            CON_CvarSetValue(sv_lockmonsters.name, 0);
        break;
        
    case features_wireframe:
        R_DrawWireframe(choice);
        wireframeon = choice;
        break;
    }

    S_StartSound(NULL, sfx_switch2);
}

#ifdef _USE_XINPUT  // XINPUT

#include "g_controls.h"

void M_DrawXInputButton(int x, int y, int button);

//------------------------------------------------------------------------
//
// XBOX 360 CONTROLLER MENU
//
//------------------------------------------------------------------------

void M_XGamePadChoice(int choice);
void M_DrawXGamePad(void);

#if 0
void M_XCtrlSchemeChoice(int choice);
void M_DrawXCtrlScheme(void);
#endif

enum
{
    xgp_sensitivity,
    xgp_empty1,
    xgp_threshold,
    xgp_empty2,
    xgp_layout,
    xgp_look,
    xgp_invert,
    xgp_default,
    xgp_return,
    xgp_end
} xgp_e;

#if 0
enum
{
    xcs_1,
    xcs_2,
    xcs_3,
    xcs_4,
    xcs_5,
    xcs_6,
    xcs_7,
    xcs_8,
    xcs_9,
    xcs_10,
    xcs_12,
    xcs_13,
    xcs_14,
    xcs_default,
    xcs_return,
    xcs_end
} xcs_e;
#endif

menuitem_t XGamePadMenu[]=
{
    {3,"Stick Sensitivity",M_XGamePadChoice,'s'},
    {-1,"",0},
    {3,"Turn Threshold",M_XGamePadChoice,'t'},
    {-1,"",0},
    {2,"Layout:",M_XGamePadChoice,'b'},
    {2,"Y Axis Look:",M_ChangeMouseLook,'l'},
    {2,"Invert Look:",M_ChangeMouseInvert, 'i'},
    {-2,"Default",M_DoDefaults,'d'},
    {1,"/r Return",M_Return, 0x20}
};

#if 0
menuitem_t XCtrlSchemeMenu[]=
{
    {1,"   Attack", NULL, 0},
    {1,"   Use", NULL, 0},
    {1,"   Forward", NULL, 0},
    {1,"   Back", NULL, 0},
    {1,"   Left", NULL, 0},
    {1,"   Right", NULL, 0},
    {1,"   Strafe Left", NULL, 0},
    {1,"   Strafe Right", NULL, 0},
    {1,"   Run", NULL, 0},
    {1,"   Jump", NULL, 0},
    {1,"   Look Up", NULL, 0},
    {1,"   Look Down", NULL, 0},
    {1,"   Center View", NULL, 0},
    {-2,"Default",M_DoDefaults,'d'},
    {1,"/r Return",M_Return, 0x20}
};
#endif

menudefault_t XGamePadDefault[] =
{
    { &i_rsticksensitivity, 0.0080f },
    { &i_rstickthreshold, 20 },
    { &i_xinputscheme, 0 },
    { &v_mlook, 0 },
    { &v_mlookinvert, 0 },
    { NULL, -1 }
};

menu_t XGamePadDef =
{
    xgp_end,
    false,
    &OptionsDef,
    XGamePadMenu,
    M_DrawXGamePad,
    "XBOX 360 Gamepad",
    88,48,
    0,
    false,
    XGamePadDefault,
    -1,
    0,
    1.0f,
    NULL
};

#if 0
menu_t XCtrlSchemeDef =
{
    xcs_end,
    false,
    &XGamePadDef,
    XCtrlSchemeMenu,
    M_DrawXCtrlScheme,
    "Config Buttons",
    96,56,
    0,
    false,
    NULL,
    8,
    0,
    1.0f,
    NULL
};
#endif

void M_XGamePadChoice(int choice)
{
    float slope1 = 0.0125f / 100.0f;
    float slope2 = 100.0f / 50.0f;

    switch(itemOn)
    {
    case xgp_sensitivity:
        if(choice)
        {
            if(i_rsticksensitivity.value < 0.0125f)
                M_SetCvar(&i_rsticksensitivity, i_rsticksensitivity.value + slope1);
            else
                CON_CvarSetValue(i_rsticksensitivity.name, 0.0125f);
        }
        else
        {
            if(i_rsticksensitivity.value > 0.001f)
                M_SetCvar(&i_rsticksensitivity, i_rsticksensitivity.value - slope1);
            else
                CON_CvarSetValue(i_rsticksensitivity.name, 0.001f);
        }
        break;

    case xgp_threshold:
        if(choice)
        {
            if(i_rstickthreshold.value < 100)
                M_SetCvar(&i_rstickthreshold, i_rstickthreshold.value + slope2);
            else
                CON_CvarSetValue(i_rstickthreshold.name, 100);
        }
        else
        {
            if(i_rstickthreshold.value > 1)
                M_SetCvar(&i_rstickthreshold, i_rstickthreshold.value - slope2);
            else
                CON_CvarSetValue(i_rstickthreshold.name, 1);
        }
        break;

    case xgp_layout:
        M_SetCvar(&i_xinputscheme, (float)choice);
        break;
    }
}

void M_DrawXGamePad(void)
{
    static const char* schemeType[] = { "Modern", "Classic" };

    M_DrawThermo(XGamePadDef.x, XGamePadDef.y + LINEHEIGHT*(xgp_sensitivity+1),
        100, i_rsticksensitivity.value * 8000.0f);

    M_DrawThermo(XGamePadDef.x, XGamePadDef.y + LINEHEIGHT*(xgp_threshold+1),
        50, i_rstickthreshold.value * 0.5f);

     M_DrawSmbText(XGamePadDef.x + 128, XGamePadDef.y + LINEHEIGHT * xgp_layout, MENUCOLORRED,
        schemeType[(int)i_xinputscheme.value]);

     M_DrawSmbText(XGamePadDef.x + 128, XGamePadDef.y + LINEHEIGHT * xgp_look, MENUCOLORRED,
        msgNames[(int)v_mlook.value]);

    M_DrawSmbText(XGamePadDef.x + 128, XGamePadDef.y + LINEHEIGHT * xgp_invert, MENUCOLORRED,
        msgNames[(int)v_mlookinvert.value]);
}

#if 0
void M_XCtrlSchemeChoice(int choice)
{
}

void M_DrawXCtrlScheme(void)
{
    int i;
    int j;
    int keys[NUM_PCKEYS];

    dmemset(keys, 0, sizeof(int)*NUM_PCKEYS);

    for(i = 0; i < NUM_PCKEYS; i++)
    {
        for(j = 0; j < XINPUT_BUTTONS + 2; j++)
        {
            if(xbtnlayout[0][j][0] == i)
            {
                keys[i] = xbtnlayout[0][j][1];
                continue;
            }
        }
    }

    for(i = xcs_1; i < xcs_14 + 1; i++)
    {
        if(currentMenu->menupageoffset <= i &&
            i - currentMenu->menupageoffset < currentMenu->numpageitems)
        {
            M_DrawXInputButton(XCtrlSchemeDef.x,
                XCtrlSchemeDef.y + LINEHEIGHT * (i - currentMenu->menupageoffset),
                keys[i - 1]);
        }
    }
}
#endif

#endif  // XINPUT

//------------------------------------------------------------------------
//
// CONTROLS MENU
//
//------------------------------------------------------------------------

void M_ChangeKeyBinding(int choice);
void M_BuildControlMenu(void);
void M_DrawControls(void);

#define NUM_NONBINDABLE_ITEMS   17
#define	NUM_CONTROL_ITEMS		34 + NUM_NONBINDABLE_ITEMS

menuaction_t*   PlayerActions;
menu_t          ControlsDef;
menuitem_t      ControlsItem[NUM_CONTROL_ITEMS];

menuaction_t mPlayerActionsDef[]=
{
    {"Movement", NULL},
    {"Forward", "+forward"},
    {"Back", "+back"},
    {"Left", "+left"},
    {"Right", "+right"},
    {"Strafe", "+strafe"},
    {"Strafe Left", "+strafeleft"},
    {"Strafe Right", "+straferight"},
    {"Action", NULL},
    {"Fire", "+fire"},
    {"Use", "+use"},
    {"Run", "+run"},
    {"Jump", "+jump"},
    {"Autorun", "autorun"},
    {"Look Up", "+lookup"},
    {"Look Down", "+lookdown"},
    {"Center View", "+center"},
    {"Weapons", NULL},
    {"Next Weapon", "nextweap"},
    {"Previous Weapon", "prevweap"},
    {"Fist", "weapon 2"},
    {"Pistol", "weapon 3"},
    {"Shotgun", "weapon 4"},
    {"Chaingun", "weapon 6"},
    {"Rocket Launcher", "weapon 7"},
    {"Plasma Rifle", "weapon 8"},
    {"BFG 9000", "weapon 9"},
    {"Chainsaw", "weapon 1"},
    {"Super Shotgun", "weapon 5"},
    {"Laser Artifact", "weapon 10"},
    {"Other", NULL},
    {"Detatch Camera", "setcamerastatic"},
    {"Chasecam", "setcamerachase"},
    {NULL, NULL},
};

void M_Controls(int choice)
{
#ifdef _USE_XINPUT  // XINPUT
    if(xgamepad.connected)
        M_SetupNextMenu(&XGamePadDef);
    else
#endif
        M_BuildControlMenu();
}

void M_BuildControlMenu(void)
{
    menu_t		*menu;
    int			actions;
    int			item;
    int			i;
    
    PlayerActions = mPlayerActionsDef;
    
    actions = 0;
    while(PlayerActions[actions].name)
        actions++;
    
    menu = &ControlsDef;
    // add extra menu items for non-bindable items (display only)
    menu->numitems = actions + NUM_NONBINDABLE_ITEMS;
    menu->textonly = false;
    menu->numpageitems = 16;
    menu->prevMenu = &OptionsDef;
    menu->menuitems = ControlsItem;
    menu->routine = M_DrawControls;
    menu->x = 120;
    menu->y = 80;
    menu->smallfont = true;
    menu->menupageoffset = 0;
    menu->scale = 0.75f;
    sprintf(menu->title, "Controls");
    menu->lastOn = itemOn;

    for(item = 0; item < actions; item++)
    {   
        dstrcpy(menu->menuitems[item].name, PlayerActions[item].name);
        if(PlayerActions[item].action)
        {
            for(i = dstrlen(PlayerActions[item].name); i < 15; i++)
                menu->menuitems[item].name[i] = ' ';

            menu->menuitems[item].name[15] = ':';
            menu->menuitems[item].status = 1;
            menu->menuitems[item].routine = M_ChangeKeyBinding;

            G_GetActionBindings(&menu->menuitems[item].name[16], PlayerActions[item].action);
        }
        else
        {
            menu->menuitems[item].status = -1;
            menu->menuitems[item].routine = NULL;
        }

        menu->menuitems[item].alphaKey = 0;
    }

#define ADD_NONBINDABLE_ITEM(i, str, s)                 \
    dstrcpy(menu->menuitems[actions + i].name, str);    \
    menu->menuitems[actions + i].status = s;            \
    menu->menuitems[actions + i].routine = NULL        

    ADD_NONBINDABLE_ITEM(0, "Non-Bindable Keys", -1);
    ADD_NONBINDABLE_ITEM(1, "Save Game      : F2", 1);
    ADD_NONBINDABLE_ITEM(2, "Load Game      : F3", 1);
    ADD_NONBINDABLE_ITEM(3, "Screenshot     : F5", 1);
    ADD_NONBINDABLE_ITEM(4, "Quicksave      : F6", 1);
    ADD_NONBINDABLE_ITEM(5, "Quickload      : F7", 1);
    ADD_NONBINDABLE_ITEM(6, "Change Gamma   : F11", 1);
    ADD_NONBINDABLE_ITEM(7, "Toggle Automap : Tab", 1);
    ADD_NONBINDABLE_ITEM(8, "Zoom In Map    : KeyPad+", 1);
    ADD_NONBINDABLE_ITEM(9, "Zoom Out Map   : KeyPad-", 1);
    ADD_NONBINDABLE_ITEM(10,"Follow Mode    : f", 1);
    ADD_NONBINDABLE_ITEM(11,"Map Pan Left   : Left", 1);
    ADD_NONBINDABLE_ITEM(12,"Map Pan Right  : Right", 1);
    ADD_NONBINDABLE_ITEM(13,"Map Pan Up     : Up", 1);
    ADD_NONBINDABLE_ITEM(14,"Map Pan Down   : Down", 1);
    ADD_NONBINDABLE_ITEM(15,"Map Pan Drag   : Space", 1);
    ADD_NONBINDABLE_ITEM(16,"Chat           : t", 1);

    M_SetupNextMenu(menu);
}

void M_ChangeKeyBinding(int choice)
{
    char action[128];
    sprintf(action, "%s %d", PlayerActions[choice].action, 1);
    dstrcpy(MenuBindBuff, action);
    messageBindCommand=MenuBindBuff;
    sprintf(MenuBindMessage, "%s", PlayerActions[choice].name);
    MenuBindActive = true;
}

void M_DrawControls(void)
{
    M_DrawSmbText(-1, 264, MENUCOLORWHITE , "Press Escape To Return");
    M_DrawSmbText(-1, 280, MENUCOLORWHITE , "Press Delete To Unbind");
}

//------------------------------------------------------------------------
//
// QUICKSAVE CONFIRMATION
//
//------------------------------------------------------------------------

void M_DrawQuickSaveConfirm(void);

enum
{
    QS_Ok = 0,
    QS_End
}; qsconfirm_e;

menuitem_t QuickSaveConfirm[]=
{
    {1,"Ok",M_ReturnToOptions,'o'}
};

menu_t  QuickSaveConfirmDef =
{
    QS_End,
    false,
    &PauseDef,
    QuickSaveConfirm,
    M_DrawQuickSaveConfirm,
    " ",
    144,112,
    QS_Ok,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_DrawQuickSaveConfirm(void)
{
    M_DrawSmbText(-1, 16, MENUCOLORRED , "You Need To Pick");
    M_DrawSmbText(-1, 32, MENUCOLORRED , "A Quicksave Slot!");
}

//------------------------------------------------------------------------
//
// LOAD IN NETGAME NOTIFY
//
//------------------------------------------------------------------------

void M_DrawNetLoadNotify(void);

enum
{
    NLN_Ok = 0,
    NLN_End
}; netloadnotify_e;

menuitem_t NetLoadNotify[]=
{
    {1,"Ok",M_ReturnToOptions,'o'}
};

menu_t  NetLoadNotifyDef =
{
    NLN_End,
    false,
    &PauseDef,
    NetLoadNotify,
    M_DrawNetLoadNotify,
    " ",
    144,112,
    NLN_Ok,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_DrawNetLoadNotify(void)
{
    M_DrawSmbText(-1, 16, MENUCOLORRED , "You Cannot Load While");
    M_DrawSmbText(-1, 32, MENUCOLORRED , "In A Net Game!");
}

//------------------------------------------------------------------------
//
// SAVEDEAD NOTIFY
//
//------------------------------------------------------------------------

void M_DrawSaveDeadNotify(void);

enum
{
    SDN_Ok = 0,
    SDN_End
}; savedeadnotify_e;

menuitem_t SaveDeadNotify[]=
{
    {1,"Ok",M_ReturnToOptions,'o'}
};

menu_t  SaveDeadDef =
{
    SDN_End,
    false,
    &PauseDef,
    SaveDeadNotify,
    M_DrawSaveDeadNotify,
    " ",
    144,112,
    SDN_Ok,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_DrawSaveDeadNotify(void)
{
    M_DrawSmbText(-1, 16, MENUCOLORRED , "You Cannot Save");
    M_DrawSmbText(-1, 32, MENUCOLORRED , "While Not In Game");
}

//------------------------------------------------------------------------
//
// SAVE GAME MENU
//
//------------------------------------------------------------------------

void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_DrawSave(void);

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end
} load_e;

menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'},
    {1,"", M_SaveSelect,'7'},
    {1,"", M_SaveSelect,'8'},
};

menu_t  SaveDef =
{
    load_end,
    false,
    &PauseDef,
    SaveMenu,
    M_DrawSave,
    "Save Game",
    112,144,
    0,
    false,
    NULL,
    -1,
    0,
    0.5f,
    NULL
};

//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int i;

    M_DrawSaveGameFrontend(&SaveDef);
    
    for(i = 0; i < load_end; i++)
        M_DrawSmbText(SaveDef.x, SaveDef.y + LINEHEIGHT * i, MENUCOLORRED, savegamestrings[i]);
    
    if(saveStringEnter)
    {
        i = ((int)(160.0f / SaveDef.scale) - M_CenterSmbText(savegamestrings[saveSlot])) * 2;
        M_DrawSmbText(SaveDef.x + i, (SaveDef.y + LINEHEIGHT * saveSlot) - 2, MENUCOLORWHITE, "/r");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame(slot,savegamestrings[slot]);
    M_ClearMenus();

    thumbnail_active = -1;
    
    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
        quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;
    
    saveSlot = choice;
    dstrcpy(saveOldString,savegamestrings[choice]);

    if(!dstrcmp(savegamestrings[choice],EMPTYSTRING))
        savegamestrings[choice][0] = 0;

    saveCharIndex = dstrlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame(int choice)
{
    if(!usergame)
    {
        M_StartControlPanel();
        M_SetupNextMenu(&SaveDeadDef);
        return;
    }
    
    if(gamestate != GS_LEVEL)
        return;
    
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}

//------------------------------------------------------------------------
//
// LOAD GAME MENU
//
//------------------------------------------------------------------------

void M_LoadSelect(int choice);
void M_DrawLoad(void);

menuitem_t DoomLoadMenu[]=//LoadMenu conflicts with Win32 API
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'},
    {1,"", M_LoadSelect,'7'},
    {1,"", M_LoadSelect,'8'}
};

menu_t  LoadMainDef =
{
    load_end,
    false,
    &MainDef,
    DoomLoadMenu,
    M_DrawLoad,
    "Load Game",
    112,144,
    0,
    false,
    NULL,
    -1,
    0,
    0.5f,
    NULL
};

menu_t  LoadDef =
{
    load_end,
    false,
    &PauseDef,
    DoomLoadMenu,
    M_DrawLoad,
    "Load Game",
    112,144,
    0,
    false,
    NULL,
    -1,
    0,
    0.5f,
    NULL
};

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int i;

    M_DrawSaveGameFrontend(&LoadDef);

    for(i = 0; i < load_end; i++)
        M_DrawSmbText(LoadDef.x, LoadDef.y + LINEHEIGHT * i,
        MENUCOLORRED, savegamestrings[i]);
}


//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char name[256];
    
    dsprintf(name, SAVEGAMENAME"%d.dsg", choice);
    G_LoadGame(name);
    M_ClearMenus();
}

//
// Selected from DOOM menu
//
void M_LoadGame(int choice)
{
    if(netgame)
    {
        M_StartControlPanel();
        M_SetupNextMenu(&NetLoadNotifyDef);
        return;
    }
    
    if(currentMenu == &MainDef)
        M_SetupNextMenu(&LoadMainDef);
    else
        M_SetupNextMenu(&LoadDef);

    M_ReadSaveStrings();
}

//
// M_ReadSaveStrings
//
// Read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    int     handle;
    int     i;
    char    name[256];
    
    for (i = 0; i < load_end; i++)
    {
        dsprintf(name, SAVEGAMENAME"%d.dsg", i);
        
        handle = open(name, O_RDONLY | 0, 0666);
        if(handle == -1)
        {
            dstrcpy(&savegamestrings[i][0],EMPTYSTRING);
            DoomLoadMenu[i].status = 0;
            continue;
        }
        read(handle, &savegamestrings[i], MENUSAVESTRINGSIZE);
        close(handle);
        DoomLoadMenu[i].status = 1;
    }
}

//------------------------------------------------------------------------
//
// QUICKSAVE PROMPT
//
//------------------------------------------------------------------------

void M_QuickSaveResponse(int ch);

enum
{
    QSP_Yes = 0,
    QSP_No,
    QSP_End
}; quicksaveprompt_e;

menuitem_t QuickSavePrompt[]=
{
    {1,"Yes",M_QuickSaveResponse,'y'},
    {1,"No",M_ReturnToOptions,'n'}
};

menu_t  QuickSavePromptDef =
{
    QSP_End,
    false,
    &PauseDef,
    QuickSavePrompt,
    NULL,
    "Overwrite QuickSave?",
    144,112,
    QSP_Yes,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_QuickSaveResponse(int ch)
{
    M_DoSave(quickSaveSlot);
}

//------------------------------------------------------------------------
//
// QUICKLOAD PROMPT
//
//------------------------------------------------------------------------

void M_QuickLoadResponse(int ch);

enum
{
    QLP_Yes = 0,
    QLP_No,
    QLP_End
}; quickloadprompt_e;

menuitem_t QuickLoadPrompt[]=
{
    {1,"Yes",M_QuickLoadResponse,'y'},
    {1,"No",M_ReturnToOptions,'n'}
};

menu_t  QuickLoadPromptDef =
{
    QLP_End,
    false,
    &PauseDef,
    QuickLoadPrompt,
    NULL,
    "Load QuickSave?",
    144,112,
    QLP_Yes,
    false,
    NULL,
    -1,
    0,
    1.0f,
    NULL
};

void M_QuickLoadResponse(int ch)
{
    M_LoadSelect(quickSaveSlot);
}

//------------------------------------------------------------------------
//
// COMMON MENU FUNCTIONS
//
//------------------------------------------------------------------------

//
// M_SetCvar
//

static int prevtic = 0; // hack - check for overlapping sounds
static void M_SetCvar(cvar_t *cvar, float value)
{
    if(cvar->value == value)
        return;

    if(prevtic != gametic)
    {
        S_StartSound(NULL,
            currentMenu->menuitems[itemOn].status == 3 ? sfx_secmove : sfx_switch2);

        prevtic = gametic;
    }

    CON_CvarSetValue(cvar->name, value);
}

//
// M_DoDefaults
//

static void M_DoDefaults(int choice)
{
    int i = 0;
    
    for(i = 0; currentMenu->defaultitems[i].mitem != NULL; i++)
        CON_CvarSetValue(currentMenu->defaultitems[i].mitem->name, currentMenu->defaultitems[i].mdefault);
    
    if(currentMenu == &DisplayDef)
        R_RefreshBrightness();

    if(currentMenu == &SoundDef)
    {
        S_SetSoundVolume(s_sfxvol.value);
        S_SetMusicVolume(s_musvol.value);
    }
    
    if(currentMenu == &VideoDef)
    {
        CON_CvarSetValue(v_width.name, 640);
        CON_CvarSetValue(v_height.name, 480);
        
        R_DumpTextures();
        R_GLSetFilter();
    }

    S_StartSound(NULL, sfx_switch2);
}

//
// M_ReturnToOptions
//

void M_ReturnToOptions(int choice)
{
    M_SetupNextMenu(&PauseDef);
}

//
// M_Return
//

static void M_Return(int choice)
{
    currentMenu->lastOn = itemOn;
    if(currentMenu->prevMenu)
    {
        menufadefunc = M_MenuFadeOut;
        alphaprevmenu = true;
        S_StartSound(NULL, sfx_pistol);
    }
}

//
// M_ReturnInstant
//

static void M_ReturnInstant(void)
{
    if(currentMenu->prevMenu)
    {
        currentMenu = currentMenu->prevMenu;
        itemOn = currentMenu->lastOn;

        S_StartSound(NULL, sfx_switch2);
    }
    else
        M_ClearMenus();
}

//
// M_DrawSmbString
//

static void M_DrawSmbString(const char* text, menu_t* menu, int item)
{
    int x;
    int y;
    
    x = menu->x + 128;
    y = menu->y + (ST_FONTWHSIZE + 1) * item;
    M_DrawText(x, y, MENUCOLORWHITE, 1.0f, false, text);
}

//
// Find string width from hu_font chars
//

static int M_StringWidth(const char* string)
{
    int             i;
    int             w = 0;
    int             c;
    
    for (i = 0;i < (int)dstrlen(string);i++)
    {
        c = toupper(string[i]) - ST_FONTSTART;
        if (c < 0 || c >= ST_FONTSIZE)
            w += 4;
        else
            w += ST_FONTWHSIZE;
    }
    
    return w;
}



//
// Find string height from hu_font chars
//

static int M_StringHeight(const char* string)
{
    int             i;
    int             h;
    int             height = ST_FONTWHSIZE;
    
    h = height;
    for (i = 0;i < (int)dstrlen(string);i++)
        if (string[i] == '\n')
            h += height;
        
        return h;
}

//
// M_QuickSave
//

//
// M_QuickSave
//

void M_QuickSave(void)
{
    if(!usergame)
    {
        S_StartSound(NULL,sfx_oof);
        return;
    }
    
    if(gamestate != GS_LEVEL)
        return;
    
    if(quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;     // means to pick a slot now
        return;
    }

    M_StartControlPanel();
    M_SetupNextMenu(&QuickSavePromptDef);
}


void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartControlPanel();
        M_SetupNextMenu(&NetLoadNotifyDef);
        return;
    }
    
    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_SetupNextMenu(&QuickSaveConfirmDef);
        return;
    }

    M_StartControlPanel();
    M_SetupNextMenu(&QuickLoadPromptDef);
}


//
// Menu Functions
//

static void M_DrawThermo(int x, int y, int thermWidth, float thermDot)
{
    float slope = 100.0f / (float)thermWidth;
    
    M_DrawSmbText(x, y, MENUCOLORWHITE, "/t");
    M_DrawSmbText(x + (int)(thermDot * slope) * (symboldata[SM_THERMO].w / 100), y, MENUCOLORWHITE, "/s");
}

//
// M_SetThumbnail
//

static dtexture thumbnail = 0;
static char thumbnail_date[32];
static int thumbnail_skill = -1;
static int thumbnail_map = -1;

static dboolean M_SetThumbnail(int which)
{
    byte* data;
    char name[256];

    //
    // still selected on current thumbnail?
    //
    if(thumbnail_active == which)
    {
        dglBindTexture(GL_TEXTURE_2D, thumbnail);
        return 1;
    }

    thumbnail_active = which;

    //
    // delete old thumbnail texture
    //
    if(thumbnail)
    {
        dglDeleteTextures(1, &thumbnail);
        thumbnail = 0;
    }

    dsprintf(name, SAVEGAMENAME"%d.dsg", which);

    data = Z_Malloc((128 * 128) * 3, PU_STATIC, 0);

    //
    // poke into savegame file and fetch
    // thumbnail, date and stats
    //
    if(!P_QuickReadSaveHeader(name, thumbnail_date, (int*)data,
        &thumbnail_skill, &thumbnail_map))
    {
        Z_Free(data);
        thumbnail_active = -1;
        return 0;
    }

    //
    // make a new thumbnail texture
    //
    dglGenTextures(1, &thumbnail);
    dglBindTexture(GL_TEXTURE_2D, thumbnail);

    R_GLSetFilter();

    dglTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        128,
        128,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data
        );

    Z_Free(data);

    return 1;
}

//
// M_DrawSaveGameFrontend
//

static void M_DrawSaveGameFrontend(menu_t* def)
{
    R_GLToggleBlend(1);
    R_GLEnable2D(0);

    dglDisable(GL_TEXTURE_2D);

    //
    // draw back panels
    //
    dglColor4ub(32, 32, 32, menualphacolor);
    //
    // save game panel
    //
    dglRecti(
        def->x - 48,
        def->y - 12,
        def->x + 256,
        def->y + 156
        );
    //
    // thumbnail panel
    //
    dglRecti(
        def->x + 272,
        def->y - 12,
        def->x + 464,
        def->y + 116
        );
    //
    // stats panel
    //
    dglRecti(
        def->x + 272,
        def->y + 124,
        def->x + 464,
        def->y + 176
        );

    //
    // draw outline for panels
    //
    dglColor4ub(192, 192, 192, menualphacolor);
    dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //
    // save game panel
    //
    dglRecti(
        def->x - 48,
        def->y - 12,
        def->x + 256,
        def->y + 156
        );
    //
    // thumbnail panel
    //
    dglRecti(
        def->x + 272,
        def->y - 12,
        def->x + 464,
        def->y + 116
        );
    //
    // stats panel
    //
    dglRecti(
        def->x + 272,
        def->y + 124,
        def->x + 464,
        def->y + 176
        );
    dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    dglEnable(GL_TEXTURE_2D);

    //
    // draw thumbnail texture and stats
    //
    if(M_SetThumbnail(itemOn))
    {
        char string[128];

        dglBegin(GL_POLYGON);
        dglColor4ub(0xff, 0xff, 0xff, menualphacolor);
        dglTexCoord2f(0, 0);
        dglVertex2i(def->x + 288, def->y + -8);
        dglTexCoord2f(1, 0);
        dglVertex2i(def->x + 448, def->y + -8);
        dglTexCoord2f(1, 1);
        dglVertex2i(def->x + 448, def->y + 112);
        dglTexCoord2f(0, 1);
        dglVertex2i(def->x + 288, def->y + 112);
        dglEnd();

        curgfx = -1;

        R_GLSetOrthoScale(0.35f);

        M_DrawSmbText(def->x + 444, def->y + 244, MENUCOLORWHITE, thumbnail_date);

        dsprintf(string, "Skill: %s", NewGameMenu[thumbnail_skill].name);
        M_DrawSmbText(def->x + 444, def->y + 268, MENUCOLORWHITE, string);

        dsprintf(string, "Map: %s", P_GetMapInfo(thumbnail_map)->mapname);
        M_DrawSmbText(def->x + 444, def->y + 292, MENUCOLORWHITE, string);

        R_GLSetOrthoScale(def->scale);
    }

    R_GLToggleBlend(0);
}



//------------------------------------------------------------------------
//
// CONTROL PANEL
//
//------------------------------------------------------------------------

#ifdef _USE_XINPUT  // XINPUT

const symboldata_t xinputbutons[12] =
{
    { 0, 0, 15, 16 },   // B
    { 15, 0, 15, 16 },  // A
    { 30, 0, 15, 16 },  // Y
    { 45, 0, 15, 16 },  // X
    { 60, 0, 19, 16 },  // LB
    { 79, 0, 19, 16 },  // RB
    { 98, 0, 15, 16 },  // LEFT
    { 113, 0, 15, 16 }, // RIGHT
    { 128, 0, 15, 16 }, // UP
    { 143, 0, 15, 16 }, // DOWN
    { 158, 0, 12, 16 }, // START
    { 170, 0, 12, 16 }  // SELECT
};

//
// M_DrawXInputButton
//

void M_DrawXInputButton(int x, int y, int button)
{
    int index = 0;
    float vx1 = 0.0f;
    float vy1 = 0.0f;
    float tx1 = 0.0f;
    float tx2 = 0.0f;
    float ty1 = 0.0f;
    float ty2 = 0.0f;
    float width;
    float height;
    int pic;
    vtx_t vtx[4];
    const rcolor color = MENUCOLORWHITE;
    
    switch(button)
    {
    case XINPUT_GAMEPAD_B:
        index = 0;
        break;
    case XINPUT_GAMEPAD_A:
        index = 1;
        break;
    case XINPUT_GAMEPAD_Y:
        index = 2;
        break;
    case XINPUT_GAMEPAD_X:
        index = 3;
        break;
    case XINPUT_GAMEPAD_LEFT_SHOULDER:
        index = 4;
        break;
    case XINPUT_GAMEPAD_RIGHT_SHOULDER:
        index = 5;
        break;
    case XINPUT_GAMEPAD_DPAD_LEFT:
        index = 6;
        break;
    case XINPUT_GAMEPAD_DPAD_RIGHT:
        index = 7;
        break;
    case XINPUT_GAMEPAD_DPAD_UP:
        index = 8;
        break;
    case XINPUT_GAMEPAD_DPAD_DOWN:
        index = 9;
        break;
    case XINPUT_GAMEPAD_START:
        index = 10;
        break;
    case XINPUT_GAMEPAD_BACK:
        index = 11;
        break;
        //
        // [kex] TODO: finish adding remaining buttons?
        //
    default:
        return;
    }

    pic = R_BindGfxTexture("BUTTONS", true);

    width = (float)gfxwidth[pic];
    height = (float)gfxheight[pic];

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    
    dglEnable(GL_BLEND);
    dglSetVertex(vtx);

    R_GLEnable2D(0);
    
    vx1 = (float)x;
    vy1 = (float)y;
    
    tx1 = ((float)xinputbutons[index].x / width) + 0.001f;
    tx2 = (tx1 + (float)xinputbutons[index].w / width) - 0.001f;
    ty1 = ((float)xinputbutons[index].y / height) + 0.005f;
    ty2 = (ty1 + (((float)xinputbutons[index].h / height))) - 0.008f;

    R_GLSetupVertex(
        vtx,
        vx1,
        vy1,
        xinputbutons[index].w,
        xinputbutons[index].h,
        tx1,
        tx2,
        ty1,
        ty2,
        color
        );

    dglTriangle(0, 1, 2);
    dglTriangle(1, 2, 3);
    dglDrawGeometry(4, vtx);
    
    R_GLDisable2D();
    dglDisable(GL_BLEND);
}

static const int xbtnmenutable[8][3] =
{
    { XINPUT_GAMEPAD_DPAD_UP        , KEY_UPARROW,      5000  },
    { XINPUT_GAMEPAD_DPAD_DOWN      , KEY_DOWNARROW,    5000  },
    { XINPUT_GAMEPAD_DPAD_LEFT      , KEY_LEFTARROW,    8  },
    { XINPUT_GAMEPAD_DPAD_RIGHT     , KEY_RIGHTARROW,   8  },
    { XINPUT_GAMEPAD_START          , KEY_ESCAPE,       5000  },
    { XINPUT_GAMEPAD_BACK           , KEY_BACKSPACE,    5000  },
    { XINPUT_GAMEPAD_A              , KEY_ENTER,        5000 },
    { XINPUT_GAMEPAD_B              , KEY_BACKSPACE,    5000 }
};

//
// M_GetXInputMenuKey
// Interprete xinput buttons to PC keys...
// pretty much a huge hack to avoid adding
// any more code to this already huge system
//

static int M_GetXInputMenuKey(event_t *ev)
{
    int i;
    int tic;

    if(ev->data3)
        return -1;

    for(i = 0; i < 8; i++)
    {
        tic = xbtnmenutable[i][2];

        //
        // hack for thermo bar sliders...
        // set tic to 0 so user can hold down button to slide
        // the thermo bar
        //
        if(currentMenu->menuitems[itemOn].status == 3 &&
            (xbtnmenutable[i][1] == KEY_LEFTARROW || xbtnmenutable[i][1] == KEY_RIGHTARROW))
        {
            tic = 0;
        }

        if(I_XInputTicButtonPress(ev->data1, xbtnmenutable[i][0], tic))
            return xbtnmenutable[i][1];
    }

    return -1;
}

#endif

//
// M_Responder
//

dboolean M_Responder(event_t* ev)
{
    int ch;
    int i;
    int mousewait = 0;
    
    ch = -1;
    
    if(menufadefunc || !allowmenu)
        return false;
    
    
    if(MenuBindActive == true)//key Bindings
    {
        if(ev->data1 == KEY_ESCAPE)
        {
            MenuBindActive = false;
            M_BuildControlMenu();
        }
        else if(G_BindActionByEvent(ev, messageBindCommand))
        {
            MenuBindActive = false;
            M_BuildControlMenu();
        }
        return true;
    }

#ifdef _USE_XINPUT  // XINPUT
    if(ev->type == ev_gamepad && !ev->data3)
    {
        //
        // only check for start button if menu is not active
        //
        if(!menuactive)
        {
            if(I_XInputTicButtonPress(ev->data1, XINPUT_GAMEPAD_START, 5000))
                ch = KEY_ESCAPE;
        }
        else
        {
            ch = M_GetXInputMenuKey(ev);

            //
            // hack for thermo bar sliders
            // call menu item routine immediately
            //
            if(currentMenu->menuitems[itemOn].status == 3)
            {
                if(ch == KEY_LEFTARROW)
                {
                    currentMenu->menuitems[itemOn].routine(0);
                    return true;
                }
                else if(ch == KEY_RIGHTARROW)
                {
                    currentMenu->menuitems[itemOn].routine(1);
                    return true;
                }
            }
        }
    }
    else
#endif
    {
        if(ev->type == ev_mouse)
        {
            if(mousewait < I_GetTime())
            {
                if(ev->data1 & 1)
                {
                    ch = KEY_ENTER;
                    mousewait = I_GetTime() + 15;
                }
            
                if(ev->data1 & 2)
                {
                    ch = KEY_BACKSPACE;
                    mousewait = I_GetTime() + 15;
                }
            }

            if(ev->data2 || ev->data3)
                SDL_GetMouseState(&m_mousex, &m_mousey);
        }
        else if(ev->type == ev_keydown)
        {
            ch = ev->data1;
        }
        else if(ev->type == ev_keyup)
            thermowait = 0;
    }
    
    if(ch == -1)
        return false;
    
    
    // Save Game string input
    if (saveStringEnter)
    {
        switch(ch)
        {
        case KEY_BACKSPACE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[saveSlot][saveCharIndex] = 0;
            }
            break;
            
        case KEY_ESCAPE:
            saveStringEnter = 0;
            dstrcpy(&savegamestrings[saveSlot][0],saveOldString);
            break;
            
        case KEY_ENTER:
            saveStringEnter = 0;
            if (savegamestrings[saveSlot][0])
                M_DoSave(saveSlot);
            break;
            
        default:
            ch = toupper(ch);
            if (ch != 32)
                if (ch-ST_FONTSTART < 0 || ch-ST_FONTSTART >= ST_FONTSIZE)
                    break;
                if (ch >= 32 && ch <= 127 &&
                    saveCharIndex < MENUSAVESTRINGSIZE-1 &&
                    M_StringWidth(savegamestrings[saveSlot]) <
                    (MENUSAVESTRINGSIZE-2)*8)
                {
                    savegamestrings[saveSlot][saveCharIndex++] = ch;
                    savegamestrings[saveSlot][saveCharIndex] = 0;
                }
                break;
        }
        return true;
    }
    
    
    // F-Keys
    if(!menuactive)
        switch(ch)
        {
          case KEY_F2:            // Save
              M_StartControlPanel();
              M_SaveGame(0);
              return true;
              
          case KEY_F3:            // Load
              M_StartControlPanel();
              M_LoadGame(0);
              return true;
              
          case KEY_F5:
              G_ScreenShot();
              return true;
              
          case KEY_F6:            // Quicksave
              M_QuickSave();
              return true;
              
          case KEY_F7:            // Quickload
              M_QuickLoad();
              return true;
              
          case KEY_F11:           // gamma toggle
              M_ChangeGammaLevel(2);
              return true;
        }
        
        
        // Pop-up menu?
        if (!menuactive)
        {
            if (ch == KEY_ESCAPE && gamemap != 33 && !st_chatOn)
            {
                M_StartControlPanel ();
                return true;
            }
            return false;
        }
        
        
        // Keys usable within menu
        switch (ch)
        {
        case KEY_DOWNARROW:
            S_StartSound(NULL,sfx_switch1);
            if(currentMenu == &PasswordDef)
            {
                itemOn = ((itemOn + 8) & 31);
                return true;
            }
            else
            {
                do
                {
                    if(itemOn+1 > currentMenu->numitems-1)
                        itemOn = 0;
                    else itemOn++;
                } while(currentMenu->menuitems[itemOn].status==-1 ||
                    currentMenu->menuitems[itemOn].status==-3);
                return true;
            }
            
        case KEY_UPARROW:
            S_StartSound(NULL,sfx_switch1);
            if(currentMenu == &PasswordDef)
            {
                itemOn = ((itemOn - 8) & 31);
                return true;
            }
            else
            {
                do
                {
                    if(!itemOn)
                        itemOn = currentMenu->numitems-1;
                    else itemOn--;
                } while(currentMenu->menuitems[itemOn].status==-1 ||
                    currentMenu->menuitems[itemOn].status==-3);
                return true;
            }
            
        case KEY_LEFTARROW:
            if(currentMenu == &PasswordDef)
            {
                S_StartSound(NULL,sfx_switch1);
                do
                {
                    if(!itemOn)
                        itemOn = currentMenu->numitems-1;
                    else itemOn--;
                } while(currentMenu->menuitems[itemOn].status==-1);
                return true;
            }
            else
            {
                if (currentMenu->menuitems[itemOn].routine &&
                    currentMenu->menuitems[itemOn].status >= 2)
                {
                    currentMenu->menuitems[itemOn].routine(0);
                    
                    if(currentMenu->menuitems[itemOn].status == 3)
                        thermowait = 1;
                }
                return true;
            }
            
        case KEY_RIGHTARROW:
            if(currentMenu == &PasswordDef)
            {
                S_StartSound(NULL,sfx_switch1);
                do
                {
                    if(itemOn+1 > currentMenu->numitems-1)
                        itemOn = 0;
                    else itemOn++;
                } while(currentMenu->menuitems[itemOn].status==-1);
                return true;
            }
            else
            {
                if (currentMenu->menuitems[itemOn].routine &&
                    currentMenu->menuitems[itemOn].status >= 2)
                {
                    currentMenu->menuitems[itemOn].routine(1);
                    
                    if(currentMenu->menuitems[itemOn].status == 3)
                        thermowait = -1;
                }
                return true;
            }
            
        case KEY_ENTER:
            if(currentMenu == &PasswordDef)
            {
                M_PasswordSelect();
                return true;
            }
            else
            {
                if(currentMenu->menuitems[itemOn].routine &&
                    currentMenu->menuitems[itemOn].status)
                {
                    if(currentMenu->menuitems[itemOn].routine == M_Return)
                    {
                        M_Return(0);
                        return true;
                    }
                    
                    currentMenu->lastOn = itemOn;
                    if(currentMenu == &featuresDef)
                    {
                        if(currentMenu->menuitems[itemOn].routine == M_DoFeature && 
                            itemOn == features_levels)
                        {
                            gameaction = ga_warplevel;
                            gamemap = nextmap = levelwarp + 1;
                            M_ClearMenus();
                            dmemset(passwordData, 0xff, 16);
                            return true;
                        }
                    }
                    else if(currentMenu->menuitems[itemOn].status >= 2 ||
                        currentMenu->menuitems[itemOn].status == -2)
                    {
                        currentMenu->menuitems[itemOn].routine(1);      // right arrow
                    }
                    else
                    {
                        if(currentMenu == &ControlsDef)
                        {
                            // don't do the fade effect and jump straight to the next screen
                            M_ChangeKeyBinding(itemOn);
                        }
                        else
                        {
                            menufadefunc = M_MenuFadeOut;
                            alphaprevmenu = false;
                        }
                        
                        S_StartSound(NULL, sfx_pistol);
                    }
                }
                return true;
            }
            
        case KEY_ESCAPE:
            //villsa
            if(gamestate == GS_SKIPPABLE || demoplayback)
                return false;

            M_ReturnInstant();
            return true;
            
        case KEY_DEL:
            if(currentMenu == &ControlsDef)
            {
                if(currentMenu->menuitems[itemOn].routine)
                {
                    G_UnbindAction(PlayerActions[itemOn].action);
                    M_BuildControlMenu();
                }
            }
            return true;
            
        case KEY_BACKSPACE:
            if(currentMenu == &PasswordDef)
                M_PasswordDeSelect();
            else
                M_Return(0);
            return true;
            
        default:
            for (i = itemOn+1;i < currentMenu->numitems;i++)
                if (currentMenu->menuitems[i].status != -1 
                    && currentMenu->menuitems[i].alphaKey == ch)
                {
                    itemOn = i;
                    S_StartSound(NULL, sfx_switch1);
                    return true;
                }
                for (i = 0;i <= itemOn;i++)
                    if (currentMenu->menuitems[i].status != -1 
                        && currentMenu->menuitems[i].alphaKey == ch)
                    {
                        itemOn = i;
                        S_StartSound(NULL, sfx_switch1);
                        return true;
                    }
                    break;
                    
    }
    
    return false;
}

//
// M_StartControlPanel
//

void M_StartControlPanel(void)
{
    if(!allowmenu)
        return;

    if(demoplayback)
        return;
    
    // intro might call this repeatedly
    if(menuactive)
        return;
    
    menuactive = 1;
    menufadefunc = NULL;
    currentMenu = !usergame ? &MainDef : &PauseDef;
    itemOn = currentMenu->lastOn;

    S_PauseSound();
}


#if 0

//
// M_CursorHighlightItem
//
// Highlight effects when positioning mouse cursor
// over menu item
//
// Big text just highlights to a brighter color and small text
// displays a highlight box over them
//

static dboolean M_CursorHighlightItem(int x, int y, menu_t* menu, int item)
{
    float scrnx;
    float scrny;
    int centerwidth;
    int width;
    int height;

    //
    // don't highlight static items
    //
    if(menu->menuitems[item].status == -1)
        return false;

    //
    // determine size of highlight box
    //
    centerwidth = menu->smallfont ?
        160 - M_StringWidth(menu->menuitems[item].name) / 2 :
    M_CenterSmbText(menu->menuitems[item].name);

    scrnx = (float)video_width / (float)SCREENWIDTH;
    scrny = (float)video_height / (float)SCREENHEIGHT;
    width = menu == &ControlsDef ? SCREENWIDTH : (160 - centerwidth) * 2;
    height = menu->smallfont ? 8 : 14;

    if(m_mousex >= ((float)x * scrnx) && m_mousex <= ((float)(x + width) * scrnx))
    {
        if(m_mousey >= ((float)y * scrny) && m_mousey <= (((float)y + height) * scrny))
        {
            vtx_t vertex[4];

            if(menu == &PasswordDef || !menu->smallfont)
                return true;

            R_GLToggleBlend(1);
            R_GLEnable2D(0);

            R_GLSetupVertex(vertex, (float)x - 1, (float)y - 1,
                width + 2, height + 2, 0, 1, 0, 1, 0);

            //
            // draw highlight box for small fonts
            //
            dglDisable(GL_TEXTURE_2D);
            dglBegin(GL_POLYGON);
            dglColor4ub(192, 224, 255, (96 * menualphacolor) / 0xff);
            dglVertex2f(vertex[0].x, vertex[0].y);
            dglVertex2f(vertex[1].x, vertex[1].y);
            dglColor4ub(32, 48, 64, (96 * menualphacolor) / 0xff);
            dglVertex2f(vertex[3].x, vertex[3].y);
            dglVertex2f(vertex[2].x, vertex[2].y);
            dglEnd();
            dglColor4ub(255, 255, 255, (0xff * menualphacolor) / 0xff);
            dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            dglBegin(GL_POLYGON);
            dglVertex2f(vertex[0].x, vertex[0].y);
            dglVertex2f(vertex[1].x, vertex[1].y);
            dglVertex2f(vertex[3].x, vertex[3].y);
            dglVertex2f(vertex[2].x, vertex[2].y);
            dglEnd();
            dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            dglEnable(GL_TEXTURE_2D);

            R_GLDisable2D();
            R_GLToggleBlend(0);

            return true;
        }
    }

    return false;
}

#endif

//
// M_DrawMenuSkull
//
// Draws skull icon from the symbols lump
// Pretty straightforward stuff..
//

static void M_DrawMenuSkull(int x, int y)
{
    int index = 0;
    float vx1 = 0.0f;
    float vy1 = 0.0f;
    float tx1 = 0.0f;
    float tx2 = 0.0f;
    float ty1 = 0.0f;
    float ty2 = 0.0f;
    float smbwidth;
    float smbheight;
    int pic;
    vtx_t vtx[4];
    const rcolor color = MENUCOLORWHITE;
    
    pic = R_BindGfxTexture("SYMBOLS", true);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    
    dglEnable(GL_BLEND);
    dglSetVertex(vtx);

    R_GLEnable2D(0);
    
    index = (whichSkull & 7) + SM_SKULLS;
    
    vx1 = (float)x;
    vy1 = (float)y;

    smbwidth = (float)gfxwidth[pic];
    smbheight = (float)gfxheight[pic];
    
    tx1 = ((float)symboldata[index].x / smbwidth) + 0.001f;
    tx2 = (tx1 + (float)symboldata[index].w / smbwidth) - 0.001f;
    ty1 = ((float)symboldata[index].y / smbheight) + 0.005f;
    ty2 = (ty1 + (((float)symboldata[index].h / smbheight))) - 0.008f;

    R_GLSetupVertex(
        vtx,
        vx1,
        vy1,
        symboldata[index].w,
        symboldata[index].h,
        tx1,
        tx2,
        ty1,
        ty2,
        color
        );

    dglTriangle(0, 1, 2);
    dglTriangle(1, 2, 3);
    dglDrawGeometry(4, vtx);
    
    R_GLDisable2D();
    dglDisable(GL_BLEND);
}

//
// M_Drawer
//
// Called after the view has been rendered,
// but before it has been blitted.
//

void M_Drawer (void)
{
    short x;
    short y;
    short i;
    short max;
    int start;
    int height;
    dboolean highlighted;

    if(currentMenu != &MainDef)
        ST_FlashingScreen(0, 0, 0, 96);
    
    if(MenuBindActive)
    {
        M_DrawSmbText(-1, 64, MENUCOLORWHITE, "Press New Key For");
        M_DrawSmbText(-1, 80, MENUCOLORRED, MenuBindMessage);
        return;
    }
    
    if(!menuactive)
        return;
    
    M_DrawSmbText(-1, 16, MENUCOLORRED, currentMenu->title);

    if(currentMenu->scale != 1)
        R_GLSetOrthoScale(currentMenu->scale);

    if(currentMenu->routine)
        currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    
    start = currentMenu->menupageoffset;
    max = (currentMenu->numpageitems == -1) ? currentMenu->numitems : currentMenu->numpageitems;

    if(currentMenu->textonly)
        height = TEXTLINEHEIGHT;
    else
        height = LINEHEIGHT;
    
    if(currentMenu->smallfont)
        height /= 2;
    
    //
    // begin drawing all menu items
    //
    for(i = start; i < max+start; i++)
    {
        //
        // skip hidden items
        //
        if(currentMenu->menuitems[i].status == -3)
            continue;

        highlighted = false;

        if(currentMenu == &PasswordDef)
        {
            if(i > 0)
            {
                if(!(i & 7))
                {
                    y += height;
                    x = currentMenu->x;
                }
                else
                    x += TEXTLINEHEIGHT;
            }
        }

#if 0
        //
        // draw highlights
        //
        if(m_menumouse.value)
        {
            highlighted = M_CursorHighlightItem(x, y, currentMenu, i);

            if(highlighted)
                itemOn = i;
        }
#endif
        
        if(currentMenu->menuitems[i].status != -1)
        {
            //
            // blinking letter for password menu
            //
            if(currentMenu == &PasswordDef && gametic & 4 && i == itemOn)
                continue;
            
            if(!currentMenu->smallfont)
            {
                rcolor fontcolor = MENUCOLORRED;

                if(highlighted)
                    fontcolor += D_RGBA(0, 128, 8, 0);

                M_DrawSmbText(x, y, fontcolor, currentMenu->menuitems[i].name);
            }
            else
            {
                rcolor color = MENUCOLORWHITE;

                //
                // tint the non-bindable key items to a shade of red
                //
                if(currentMenu == &ControlsDef)
                {
                    if(i >= (NUM_CONTROL_ITEMS - NUM_NONBINDABLE_ITEMS))
                        color = D_RGBA(255, 192, 192, menualphacolor);
                }

                M_DrawText(
                x,
                y,
                color,
                currentMenu->scale,
                false,
                currentMenu->menuitems[i].name
                );

                //
                // nasty hack to re-set the scale after a drawtext call
                //
                if(currentMenu->scale != 1)
                    R_GLSetOrthoScale(currentMenu->scale);
            }
        }
        //
        // if menu item is static but has text, then display it as gray text
        // used for subcategories
        //
        else if(currentMenu->menuitems[i].name != "")
        {
            if(!currentMenu->smallfont)
            {
                M_DrawSmbText(
                    -1,
                    y,
                    MENUCOLORWHITE,
                    currentMenu->menuitems[i].name
                    );
            }
            else
            {
                int strwidth = M_StringWidth(currentMenu->menuitems[i].name);

                M_DrawText(
                    ((int)(160.0f / currentMenu->scale) - (strwidth / 2)),
                    y,
                    D_RGBA(255, 0, 0, menualphacolor),
                    currentMenu->scale,
                    false,
                    currentMenu->menuitems[i].name
                    );

                //
                // nasty hack to re-set the scale after a drawtext call
                //
                if(currentMenu->scale != 1)
                    R_GLSetOrthoScale(currentMenu->scale);
            }
        }
        
        if(currentMenu != &PasswordDef)
            y += height;
    }
    
    //
    // display indicators that the user can scroll farther up or down
    //
    if(currentMenu->numpageitems != -1)
    {
        if(currentMenu->menupageoffset)
        {
            //up arrow
            M_DrawSmbText(currentMenu->x, currentMenu->y - 24, MENUCOLORWHITE, "/u More...");
        }
        
        if(currentMenu->menupageoffset + currentMenu->numpageitems < currentMenu->numitems)
        {
            //down arrow
            M_DrawSmbText(currentMenu->x, (currentMenu->y - 2 + (currentMenu->numpageitems-1) * height) + 24,
                MENUCOLORWHITE, "/d More...");
        }
    }
    
    //
    // draw password cursor
    //
    if(currentMenu == &PasswordDef)
    {
        M_DrawSmbText((currentMenu->x + ((itemOn & 7) * height)) - 4,
            currentMenu->y + ((int)(itemOn / 8) * height) + 3, MENUCOLORWHITE, "/b");
    }
    else
    {
        // DRAW SKULL
        if(!currentMenu->smallfont)
        {
            int offset = 0;

            if(currentMenu->textonly)
                x += SKULLXTEXTOFF;
            else
                x += SKULLXOFF;

            if(itemOn)
            {
                for(i = itemOn; i > 0; i--)
                {
                    if(currentMenu->menuitems[i].status == -3)
                        offset++;
                }
            }
            
            M_DrawMenuSkull(x, currentMenu->y - 5 + ((itemOn - currentMenu->menupageoffset) - offset) * height);
        }
        //
        // draw arrow cursor
        //
        else
        {
            M_DrawSmbText(x - 12, 
                currentMenu->y - 4 + (itemOn - currentMenu->menupageoffset) * height,
                MENUCOLORWHITE, "/l");
        }
    }

    if(currentMenu->scale != 1)
        R_GLSetOrthoScale(1.0f);

#ifdef _USE_XINPUT  // XINPUT
    if(xgamepad.connected && currentMenu != &MainDef)
    {
        if(currentMenu == &PasswordDef)
        {
            M_DrawXInputButton(4, 184, XINPUT_GAMEPAD_B);
            M_DrawText(22, 188, MENUCOLORWHITE, 1.0f, false, "Change");
        }

        M_DrawXInputButton(4, 200, XINPUT_GAMEPAD_A);
        M_DrawText(22, 204, MENUCOLORWHITE, 1.0f, false, "Select");

        if(currentMenu != &PauseDef)
        {
            M_DrawXInputButton(5, 216, XINPUT_GAMEPAD_START);
            M_DrawText(22, 220, MENUCOLORWHITE, 1.0f, false, "Return");
        }
    }
#endif
}


//
// M_ClearMenus
//

void M_ClearMenus (void)
{
    if(!allowclearmenu)
        return;

    menufadefunc = NULL;
    menualphacolor = 0xff;
    menuactive = 0;

    S_ResumeSound();
}


//
// M_SetupNextMenu
//

void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_MenuFadeIn
//

void M_MenuFadeIn(void)
{
    if((menualphacolor + (int)m_menufadetime.value) < 0xff)
        menualphacolor += (int)m_menufadetime.value;
    else 
    {
        menualphacolor = 0xff;
        alphaprevmenu = false;
        menufadefunc = NULL;
    }
}


//
// M_MenuFadeOut
//

void M_MenuFadeOut(void)
{
    if(menualphacolor > (int)m_menufadetime.value)
        menualphacolor -= (int)m_menufadetime.value;
    else
    {
        menualphacolor = 0;
        
        if(alphaprevmenu == false)
            currentMenu->menuitems[itemOn].routine(itemOn);
        else 
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
        }
        
        menufadefunc = M_MenuFadeIn;
    }
}


//
// M_Ticker
//

void M_Ticker (void)
{
    mainmenuactive = (currentMenu == &MainDef) ? true : false;
    
    if((currentMenu == &MainDef || 
        currentMenu == &PauseDef) && usergame && demoplayback)
    {
        menuactive = 0;
        return;
    }
    if(!usergame)
    {
        OptionsDef.prevMenu = &MainDef;
        LoadDef.prevMenu = &MainDef;
        SaveDef.prevMenu = &MainDef;
    }
    else 
    {
        OptionsDef.prevMenu = &PauseDef;
        LoadDef.prevMenu = &PauseDef;
        SaveDef.prevMenu = &PauseDef;
    }

    //
    // hidden features menu
    //
    if(currentMenu == &PauseDef)
        currentMenu->menuitems[pause_features].status = p_features.value ? 1 : -3;

    //
    // hidden hardcore difficulty option
    //
    if(currentMenu == &NewDef)
        currentMenu->menuitems[nightmare].status = p_features.value ? 1 : -3;

#ifdef _USE_XINPUT  // XINPUT
    //
    // hide mouse menu if xbox 360 controller is plugged in
    //
    if(currentMenu == &OptionsDef)
        currentMenu->menuitems[options_mouse].status = xgamepad.connected ? -3 : 1;
#endif

    // auto-adjust itemOn and page offset if the first menu item is being used as a header
    if(currentMenu->menuitems[0].status == -1 &&
        currentMenu->menuitems[0].name != "")
    {
        // bump page offset up
        if(itemOn == 1)
            currentMenu->menupageoffset = 0;
        
        // bump the cursor down
        if(itemOn <= 0)
            itemOn = 1;
    }

    //
    // clamp menu fade cvar values
    //
    if(m_menufadetime.value < 0)
        CON_CvarSetValue(m_menufadetime.name, 0);
    if(m_menufadetime.value > 256)
        CON_CvarSetValue(m_menufadetime.name, 256);
    
    if(menufadefunc)
        menufadefunc();

    // auto adjust page offset for long menu items
    if(currentMenu->numpageitems != -1)
    {
        if(itemOn >= (currentMenu->numpageitems + currentMenu->menupageoffset))
        {
            currentMenu->menupageoffset = (itemOn + 1) - currentMenu->numpageitems;

            if(currentMenu->menupageoffset >= currentMenu->numitems)
                currentMenu->menupageoffset = currentMenu->numitems;
        }
        else if(itemOn < currentMenu->menupageoffset)
        {
            currentMenu->menupageoffset = itemOn;

            if(currentMenu->menupageoffset < 0)
                currentMenu->menupageoffset = 0;
        }
    }
    
    if (--skullAnimCounter <= 0)
    {
        whichSkull++;
        skullAnimCounter = 4;
    }
    
    if(thermowait != 0 && currentMenu->menuitems[itemOn].status == 3 &&
        currentMenu->menuitems[itemOn].routine)
    {
        currentMenu->menuitems[itemOn].routine(thermowait == -1 ? 1 : 0);
    }
}


//
// M_Init
//

void M_Init(void)
{
    int i = 0;
    
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 4;
    quickSaveSlot = -1;
    menufadefunc = NULL;
    
    for(i = 0; i < NUM_CONTROL_ITEMS; i++)
    {
        ControlsItem[i].alphaKey = 0;
        dmemset(ControlsItem[i].name, 0, 64);
        ControlsItem[i].routine = NULL;
        ControlsItem[i].status = 1;
    }
    
    // setup password menu
    
    for(i = 0; i < 32; i++)
    {
        PasswordMenu[i].status = 1;
        PasswordMenu[i].name[0] = passwordChar[i];
        PasswordMenu[i].routine = NULL;
        PasswordMenu[i].alphaKey = (char)passwordChar[i];
    }
    
    dmemset(passwordData, 0xff, 16);
    
    MainDef.y += 8;
    NewDef.prevMenu = &MainDef;

    // setup region menu

    if(W_CheckNumForName("BLUDA0") != -1)
    {
        CON_CvarSetValue(m_regionblood.name, 0);
        RegionMenu[region_blood].status = 1;
    }

    if(W_CheckNumForName("JPMSG01") == -1)
    {
        CON_CvarSetValue(st_regionmsg.name, 0);
        RegionMenu[region_lang].status = 1;
    }

    if(W_CheckNumForName("PLLEGAL") == -1 &&
        W_CheckNumForName("JPLEGAL") == -1)
    {
        CON_CvarSetValue(p_regionmode.name, 0);
        RegionMenu[region_mode].status = 1;
    }
    
    M_InitShiftXForm();
}




