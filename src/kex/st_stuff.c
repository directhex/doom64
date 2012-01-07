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
//  Status bar code.
//  Does the face/direction indicator animatin.
//  Does palette indicators as well (red pain/berserk, bright pickup)
//	Handles hud and chat messages
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif


#include <stdio.h>
#include "doomdef.h"
#include "g_game.h"
#include "st_stuff.h"
#include "p_local.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "sounds.h"
#include "m_misc.h"
#include "m_shift.h"
#include "con_console.h"
#include "i_system.h"
#include "am_map.h"
#include "r_texture.h"
#include "g_actions.h"
#include "z_zone.h"
#include "p_setup.h"

#ifdef _WIN32
#include "i_xinput.h"

void M_DrawXInputButton(int x, int y, int button);

#endif

//
// STATUS BAR DATA
//

typedef struct
{
    dboolean    active;
    dboolean    doDraw;
    int         delay;
    int         times;
} keyflash_t;

static keyflash_t flashCards[NUMCARDS];	/* INFO FOR FLASHING CARDS & SKULLS */

#define	FLASHDELAY      8       /* # of tics delay (1/30 sec) */
#define FLASHTIMES      6       /* # of times to flash new frag amount (EVEN!) */

#define ST_HEALTHTEXTX  29
#define ST_HEALTHTEXTY  203

#define ST_ARMORTEXTX   253
#define ST_ARMORTEXTY   203

#define ST_KEYX         78
#define ST_KEYY         216

#define ST_JMESSAGES    45

#define ST_MSGTIMEOUT   (5*TICRATE)
#define ST_MSGFADESTART (ST_MSGTIMEOUT - (1*TICRATE))
#define ST_MSGFADETIME  5
#define ST_MSGCOLOR(x)  (D_RGBA(255, 255, 255, x))


static player_t*        plyr;   // main player in game
static int              st_msgtic = 0;
static int              st_msgalpha = 0xff;
static char*            st_msg = NULL;
static vtx_t            st_vtx[32];
static int              st_vtxcount = 0;
static byte             st_flash_r;
static byte             st_flash_g;
static byte             st_flash_b;
static byte             st_flash_a;
static int              st_jmessages[ST_JMESSAGES];   // japan-specific messages
static dboolean         st_hasjmsg = false;

char* chat_macros[] =
{
    HUSTR_CHATMACRO0,
    HUSTR_CHATMACRO1,
    HUSTR_CHATMACRO2,
    HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4,
    HUSTR_CHATMACRO5,
    HUSTR_CHATMACRO6,
    HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8,
    HUSTR_CHATMACRO9
};

char player_names[MAXPLAYERS][MAXPLAYERNAME] =
{
    HUSTR_PLR1,
    HUSTR_PLR2,
    HUSTR_PLR3,
    HUSTR_PLR4
};

static const rcolor st_chatcolors[MAXPLAYERS] =
{
    D_RGBA(192, 255, 192, 255),
    D_RGBA(255, 192, 192, 255),
    D_RGBA(128, 255, 192, 255),
    D_RGBA(192, 192, 255, 255),
};

#define MAXCHATNODES    4
#define MAXCHATTIME     256
#define MAXCHATSIZE     256
#define STCHATX         32
#define STCHATY         384

typedef struct
{
    char msg[MAXCHATSIZE];
    int tics;
    rcolor color;
} stchat_t;

static stchat_t stchat[MAXCHATNODES];
static int st_chatcount = 0;
dboolean st_chatOn = false;
static char st_chatstring[MAXPLAYERS][MAXCHATSIZE];

#define STQUEUESIZE		256
static int st_chathead;
static int st_chattail;
static byte st_chatqueue[STQUEUESIZE];

#define ST_CROSSHAIRSIZE    32
int st_crosshairs = 0;

static void ST_DrawChatText(void);
static void ST_EatChatMsg(void);
static void ST_DisplayName(int playernum);

//
// DAMAGE MARKER SYSTEM
//

typedef struct damagemarker_s
{
    struct damagemarker_s*  prev;
    struct damagemarker_s*  next;
    int     tics;
    mobj_t* source;
} damagemarker_t;

static damagemarker_t dmgmarkers;

//
// ST_RunDamageMarkers
//

static void ST_RunDamageMarkers(void)
{
    damagemarker_t* dmgmarker;

    for(dmgmarker = dmgmarkers.next; dmgmarker != &dmgmarkers; dmgmarker = dmgmarker->next)
    {
        if(!dmgmarker->tics--)
        {
            P_SetTarget(&dmgmarker->source, NULL);

            dmgmarker->next->prev = dmgmarker->prev;
            dmgmarker->prev->next = dmgmarker->next;
            Z_Free(dmgmarker);
        }
    }
}

//
// ST_ClearDamageMarkers
//

void ST_ClearDamageMarkers(void)
{
    dmgmarkers.next = dmgmarkers.prev = &dmgmarkers;
}

//
// ST_AddDamageMarker
//

void ST_AddDamageMarker(mobj_t* target, mobj_t* source)
{
    damagemarker_t* dmgmarker;

    if(target->player != &players[consoleplayer])
        return;

    dmgmarker               = Z_Calloc(sizeof(*dmgmarker), PU_LEVEL, 0);
    dmgmarker->tics         = 32;
    P_SetTarget(&dmgmarker->source, source);

    dmgmarkers.prev->next   = dmgmarker;
    dmgmarker->next         = &dmgmarkers;
    dmgmarker->prev         = dmgmarkers.prev;
    dmgmarkers.prev         = dmgmarker;
}

//
// ST_DrawDamageMarkers
//

static void ST_DrawDamageMarkers(void)
{
    damagemarker_t* dmgmarker;

    for(dmgmarker = dmgmarkers.next; dmgmarker != &dmgmarkers; dmgmarker = dmgmarker->next)
    {
        static vtx_t v[3];
        player_t* p;
        float angle;
        byte alpha;

        R_GLToggleBlend(1);
        R_GLEnable2D(0);

        alpha = (dmgmarker->tics << 3);

        if(alpha < 0)
            alpha = 0;

        v[0].x = -8;
        v[0].a = alpha;
        v[1].x = 8;
        v[1].a = alpha;
        v[2].y = 4;
        v[2].r = 255;
        v[2].a = alpha;

        p = &players[consoleplayer];

        angle = (float)TRUEANGLES(p->mo->angle -
            R_PointToAngle2(dmgmarker->source->x, dmgmarker->source->y,
            p->mo->x, p->mo->y));

        dglPushMatrix();
        dglTranslatef(160, 120, 0);
        dglRotatef(angle, 0.0f, 0.0f, 1.0f);
        dglTranslatef(0, 16, 0);
        dglDisable(GL_TEXTURE_2D);
        dglSetVertex(v);
        dglTriangle(0, 1, 2);
        dglDrawGeometry(3, v);
        dglEnable(GL_TEXTURE_2D);
        dglPopMatrix();
    
        R_GLDisable2D();
        R_GLToggleBlend(0);
    }
}

//
// STATUS BAR CODE
//

//
// ST_ClearMessage
//

void ST_ClearMessage(void)
{
    st_msgtic = 0;
    st_msgalpha = 0xff;
    st_msg = NULL;
}

//
// ST_Ticker
//

void ST_Ticker (void)
{
    int ind = 0;
    
    plyr = &players[consoleplayer];
    
    //
    // keycard stuff
    //
    
    /* */
    /* Tried to open a CARD or SKULL door? */
    /* */
    for(ind = 0; ind < NUMCARDS; ind++)
    {
        /* CHECK FOR INITIALIZATION */
        if(plyr->tryopen[ind])
        {
            plyr->tryopen[ind] = false;
            flashCards[ind].active = true;
            flashCards[ind].delay = FLASHDELAY;
            flashCards[ind].times = FLASHTIMES+1;
            flashCards[ind].doDraw = false;
        }
        
        /* MIGHT AS WELL DO TICKING IN THE SAME LOOP! */
        if(flashCards[ind].active && !--flashCards[ind].delay)
        {
            flashCards[ind].delay = FLASHDELAY;
            flashCards[ind].doDraw ^= 1;
            
            if(!--flashCards[ind].times)
                flashCards[ind].active = false;
            
            if(flashCards[ind].doDraw && flashCards[ind].active)
                S_StartSound(NULL,sfx_itemup);
        }
    }
    
    //
    // messages
    //
    if(plyr->message)
    {
        CON_Printf(WHITE, "%s\n", plyr->message);

        ST_ClearMessage();
        st_msg = plyr->message;
        plyr->message = NULL;
    }
    
    if(st_msg || plyr->messagepic >= 0)
    {
        st_msgtic++;
        
        if(st_msgtic >= ST_MSGFADESTART)
            st_msgalpha = MAX((st_msgalpha -= ST_MSGFADETIME), 0);
        
        if(st_msgtic >= ST_MSGTIMEOUT)
        {
            ST_ClearMessage();
            plyr->messagepic = -1;
        }
    }

    //
    // flashes
    //
    if(plyr->cameratarget == plyr->mo)
        ST_UpdateFlash();
    
    //
    // chat stuff
    //
    for(ind = 0; ind < MAXCHATNODES; ind++)
    {
        if(stchat[ind].tics)
            stchat[ind].tics--;
    }
    
    ST_EatChatMsg();

    //
    // damage indicator
    //

    if(p_damageindicator.value)
        ST_RunDamageMarkers();
}

//
// ST_FlashingScreen
//

void ST_FlashingScreen(byte r, byte g, byte b, byte a)
{
    rcolor c = D_RGBA(r, g, b, a);

    R_GLToggleBlend(1);
    R_GLEnable2D(1);
    
    dglDisable(GL_TEXTURE_2D);
    dglColor4ubv((byte*)&c);
    dglRecti(SCREENWIDTH, SCREENHEIGHT, 0, 0);
    dglEnable(GL_TEXTURE_2D);
    
    R_GLToggleBlend(0);
}

//
// ST_DrawStatusItem
//

static void ST_DrawStatusItem(const float xy[4][2], const float uv[4][2], rcolor color)
{
    int i;

    dglTriangle(st_vtxcount + 0, st_vtxcount + 1, st_vtxcount + 2);
    dglTriangle(st_vtxcount + 0, st_vtxcount + 2, st_vtxcount + 3);

    dglSetVertexColor(st_vtx + st_vtxcount, color, 4);

    for(i = 0; i < 4; i++)
    {
        st_vtx[st_vtxcount + i].x  = xy[i][0];
        st_vtx[st_vtxcount + i].y  = xy[i][1];
        st_vtx[st_vtxcount + i].tu = uv[i][0];
        st_vtx[st_vtxcount + i].tv = uv[i][1];
    }

    st_vtxcount += 4;
}

//
// ST_DrawKey
//

static void ST_DrawKey(int key, const float uv[4][2], const float xy[4][2])
{
    float keydrawxy[4][2];

    if(plyr->cards[key] ||
        (flashCards[key].doDraw && flashCards[key].active))
    {
        dmemcpy(keydrawxy, xy, (sizeof(float)*4) * 2);

        if(st_drawhud.value >= 2)
        {
            keydrawxy[0][0] += 24;
            keydrawxy[1][0] += 24;
            keydrawxy[2][0] += 24;
            keydrawxy[3][0] += 24;

            keydrawxy[0][1] += 8;
            keydrawxy[1][1] += 8;
            keydrawxy[2][1] += 8;
            keydrawxy[3][1] += 8;
        }

        ST_DrawStatusItem(keydrawxy, uv, D_RGBA(0xff, 0xff, 0xff, 0x80));
    }
}

//
// ST_DrawStatus
//

static const float st_healthVertex[4][2] =
{
    { ST_HEALTHTEXTX, ST_HEALTHTEXTY },
    { ST_HEALTHTEXTX + 40, ST_HEALTHTEXTY },
    { ST_HEALTHTEXTX + 40, ST_HEALTHTEXTY + 6 },
    { ST_HEALTHTEXTX, ST_HEALTHTEXTY + 6 }
};

static const float st_armorVertex[4][2] =
{
    { ST_ARMORTEXTX, ST_ARMORTEXTY },
    { ST_ARMORTEXTX + 36, ST_ARMORTEXTY },
    { ST_ARMORTEXTX + 36, ST_ARMORTEXTY + 6 },
    { ST_ARMORTEXTX, ST_ARMORTEXTY + 6 }
};

static const float st_key1Vertex[4][2] =
{
    { ST_KEYX, ST_KEYY },
    { ST_KEYX + 9, ST_KEYY },
    { ST_KEYX + 9, ST_KEYY + 10 },
    { ST_KEYX, ST_KEYY + 10 }
};

static const float st_key2Vertex[4][2] =
{
    { ST_KEYX + 10, ST_KEYY },
    { ST_KEYX + 19, ST_KEYY },
    { ST_KEYX + 19, ST_KEYY + 10 },
    { ST_KEYX + 10, ST_KEYY + 10 }
};

static const float st_key3Vertex[4][2] =
{
    { ST_KEYX + 20, ST_KEYY },
    { ST_KEYX + 29, ST_KEYY },
    { ST_KEYX + 29, ST_KEYY + 10 },
    { ST_KEYX + 20, ST_KEYY + 10 }
};

static void ST_DrawStatus(void)
{
    int     lump;
    float   width;
    float   height;
    float   uv[4][2];
    const rcolor color = D_RGBA(0x68, 0x68, 0x68, 0x90);

    R_GLToggleBlend(1);
    lump = R_BindGfxTexture("STATUS", true);

    width = (float)gfxwidth[lump];
    height = (float)gfxheight[lump];

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);

    R_GLEnable2D(0);

    dglSetVertex(st_vtx);
    st_vtxcount = 0;
    
    if(st_drawhud.value == 1)
    {
        // health

        uv[0][0] = uv[3][0] = 0.0f;
        uv[0][1] = uv[1][1] = 0.0f;
        uv[1][0] = uv[2][0] = 40.0f / width;
        uv[2][1] = uv[3][1] = 6.0f / height;

        ST_DrawStatusItem(st_healthVertex, uv, color);

        // armor

        uv[0][0] = uv[3][0] = 40.0f / width;
        uv[0][1] = uv[1][1] = 0.0f;
        uv[1][0] = uv[2][0] = uv[0][0] + (36.0f / width);
        uv[2][1] = uv[3][1] = 6.0f / height;

        ST_DrawStatusItem(st_armorVertex, uv, color);
    }
    
    // cards

    uv[0][0] = uv[3][0] = 0.0f;
    uv[0][1] = uv[1][1] = 6.0f / height;
    uv[1][0] = uv[2][0] = 9.0f / width;
    uv[2][1] = uv[3][1] = 1.0f;
    
    ST_DrawKey(it_bluecard, uv, st_key1Vertex);

    uv[0][0] = uv[3][0] = 9.0f / width;
    uv[1][0] = uv[2][0] = (9.0f / width) * 2;

    ST_DrawKey(it_yellowcard, uv, st_key2Vertex);

    uv[0][0] = uv[3][0] = (9.0f / width) * 2;
    uv[1][0] = uv[2][0] = (9.0f / width) * 3;

    ST_DrawKey(it_redcard, uv, st_key3Vertex);
    
    // skulls

    uv[0][0] = uv[3][0] = (9.0f / width) * 3;
    uv[1][0] = uv[2][0] = (9.0f / width) * 4;

    ST_DrawKey(it_blueskull, uv, st_key1Vertex);

    uv[0][0] = uv[3][0] = (9.0f / width) * 4;
    uv[1][0] = uv[2][0] = (9.0f / width) * 5;

    ST_DrawKey(it_yellowskull, uv, st_key2Vertex);

    uv[0][0] = uv[3][0] = (9.0f / width) * 5;
    uv[1][0] = uv[2][0] = (9.0f / width) * 6;

    ST_DrawKey(it_redskull, uv, st_key3Vertex);
    
    dglDrawGeometry(st_vtxcount, st_vtx);

    R_GLDisable2D();
    R_GLToggleBlend(0);
}

//
// ST_DrawCrosshair
//

void ST_DrawCrosshair(int x, int y, int slot, byte scalefactor, rcolor color)
{
    float u;
    int index;
    int scale;

    if(slot <= 0)
        return;

    if(slot > st_crosshairs)
        return;

    index = slot - 1;

    R_BindGfxTexture("CRSHAIRS", true);
    R_GLToggleBlend(1);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);

    u = 1.0f / st_crosshairs;
    scale = scalefactor == 0 ? ST_CROSSHAIRSIZE : (ST_CROSSHAIRSIZE / (1 << scalefactor));

    R_GLDraw2DStrip((float)x, (float)y, scale, scale,
        u*index, u + (u*index), 0, 1, color, 0);

    R_GLToggleBlend(0);
}

//
// ST_DrawJMessage
//

static void ST_DrawJMessage(int pic)
{
    int lump = st_jmessages[pic];

    R_BindGfxTexture(lumpinfo[lump].name, true);
    R_GLToggleBlend(1);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    R_GLDraw2DStrip(
        20,
        20,
        gfxwidth[lump - g_start],
        gfxheight[lump - g_start],
        0,
        1,
        0,
        1,
        ST_MSGCOLOR(automapactive ? 0xff : st_msgalpha),
        false
        );

    R_GLToggleBlend(0);
}

//
// ST_Drawer
//

void ST_Drawer(void)
{
    dboolean checkautomap;

    //
    // flash overlay
    //

    if(st_flashoverlay.value && flashcolor)
        ST_FlashingScreen(st_flash_r, st_flash_g, st_flash_b, st_flash_a);

    if(demoplayback)
        return;

    checkautomap = (!automapactive || am_overlay.value);
    
    //
    // draw hud
    //

    if(checkautomap && st_drawhud.value)
    {
        //Status graphics
        ST_DrawStatus();

        // original hud layout
        if(st_drawhud.value == 1)
        {
            //Draw Ammo counter
            if(weaponinfo[plyr->readyweapon].ammo != am_noammo)
                M_DrawNumber(160, 215, plyr->ammo[weaponinfo[plyr->readyweapon].ammo], 0, REDALPHA(0x7f));
        
            //Draw Health
            M_DrawNumber(49, 215, plyr->health, 0, REDALPHA(0x7f));
        
            //Draw Armor
            M_DrawNumber(271, 215, plyr->armorpoints, 0, REDALPHA(0x7f));
        }
        // arranged hud layout
        else if(st_drawhud.value >= 2)
        {
            // display ammo sprite
            switch(weaponinfo[plyr->readyweapon].ammo)
            {
                case am_clip:
                    R_DrawHudSprite(SPR_CLIP, 0, 0, 248, 233, 1.0f, 0, WHITEALPHA(0xC0));
                    break;
                case am_shell:
                    R_DrawHudSprite(SPR_SHEL, 0, 0, 248, 233, 1.0f, 0, WHITEALPHA(0xC0));
                    break;
                case am_misl:
                    R_DrawHudSprite(SPR_RCKT, 0, 0, 332, 312, 0.75f, 0, WHITEALPHA(0xC0));
                    break;
                case am_cell:
                    R_DrawHudSprite(SPR_CELL, 0, 0, 330, 312, 0.75f, 0, WHITEALPHA(0xC0));
                    break;
            }

            // display artifact sprite when using laser weapon
            if(plyr->readyweapon == wp_laser)
            {
                if(plyr->artifacts & (1<<ART_TRIPLE))
                    R_DrawHudSprite(SPR_ART3, 0, 0, 496, 456, 0.5f, 0, WHITEALPHA(0xC0));
        
                if(plyr->artifacts & (1<<ART_DOUBLE))
                    R_DrawHudSprite(SPR_ART2, 0, 0, 532, 456, 0.5f, 0, WHITEALPHA(0xC0));
        
                if(plyr->artifacts & (1<<ART_FAST))
                    R_DrawHudSprite(SPR_ART1, 0, 0, 568, 456, 0.5f, 0, WHITEALPHA(0xC0));
            }

            // display medkit/armor
            R_DrawHudSprite(SPR_MEDI, 0, 0, 24, 286, 0.75f, 0, WHITEALPHA(0xC0));
            R_DrawHudSprite(SPR_ARM1, 0, 0, 32, 465, 0.5f, 0, WHITEALPHA(0xC0));

            //Draw Health
            M_DrawNumber(72, 220-18, plyr->health, 2, REDALPHA(0xC0));
            M_DrawSmbText(80, 222-18, REDALPHA(0xC0), "%");

            //Draw Armor
            M_DrawNumber(72, 220, plyr->armorpoints, 2, REDALPHA(0xC0));
            M_DrawSmbText(80, 222, REDALPHA(0xC0), "%");

            //Draw Ammo counter
            if(weaponinfo[plyr->readyweapon].ammo != am_noammo)
                M_DrawNumber(268, 220, plyr->ammo[weaponinfo[plyr->readyweapon].ammo], 1, REDALPHA(0xC0));
        }
    }
    
    //
    // draw messages
    //

    if(st_hasjmsg && st_regionmsg.value && plyr->messagepic >= 0)
    {
        ST_DrawJMessage(plyr->messagepic);
    }
    else if(st_msg && (int)m_messages.value)
    {
        M_DrawText(20, 20, ST_MSGCOLOR(automapactive ? 0xff : st_msgalpha), 1, false, st_msg);
    }
    else if(automapactive)
    {
        char str[128];
        mapdef_t* map = P_GetMapInfo(gamemap);

        if(map)
        {
            dmemset(&str, 0, 128);

            if(map->type == 2)
                sprintf(str, "%s", map->mapname);
            else
                sprintf(str, "Level %i: %s", gamemap, map->mapname);

            M_DrawText(20, 20, ST_MSGCOLOR(0xff), 1, false, str);
        }
    }
    
    //
    // draw chat text and player names
    //

    if(netgame)
    {
        ST_DrawChatText();

        if(checkautomap)
        {
            int i;

            for(i = 0; i < MAXPLAYERS; i++)
            {
                if(playeringame[i])
                    ST_DisplayName(i);
            }
        }
    }

    //
    // draw crosshairs
    //

    if(st_crosshairs && !automapactive)
    {
        int x = (SCREENWIDTH / 2) - (ST_CROSSHAIRSIZE / 8);
        int y = (SCREENHEIGHT / 2) - (ST_CROSSHAIRSIZE / 8);
        int alpha = (int)st_crosshairopacity.value;

        if(alpha > 0xff)
            alpha = 0xff;

        if(alpha < 0)
            alpha = 0;

        ST_DrawCrosshair(x, y, (int)st_crosshair.value, 2, WHITEALPHA(alpha));
    }

    //
    // use action context
    //

    if(p_usecontext.value)
    {
        if(P_UseLines(&players[consoleplayer], true))
        {
            char usestring[16];
            char contextstring[32];
            float x;

#ifdef _USE_XINPUT  // XINPUT
            if(xgamepad.connected)
            {
                M_DrawXInputButton(140, 156, XINPUT_GAMEPAD_A);
                M_DrawText(213, 214, WHITEALPHA(0xA0), 0.75, false, "Use");
            }
            else
#endif
            {
                G_GetActionBindings(usestring, "+use");
                dsprintf(contextstring, "(%s)Use", usestring);

                x = (160 / 0.75f) - ((dstrlen(contextstring) * 8) / 2);

                M_DrawText((int)x, 214, WHITEALPHA(0xA0), 0.75f, false, contextstring);
            }
        }
    }

    //
    // damage indicator
    //

    if(p_damageindicator.value)
        ST_DrawDamageMarkers();
}

//
// ST_UpdateFlash
//

#define ST_MAXDMGCOUNT  160
#define ST_MAXSTRCOUNT  32
#define ST_MAXBONCOUNT  100

void ST_UpdateFlash(void)
{
    player_t* p = &players[consoleplayer];

    flashcolor = 0;

    // invulnerability flash (white)
    if(p->powers[pw_invulnerability] > 61 || (p->powers[pw_invulnerability] & 8))
    {
        flashcolor = D_RGBA(128, 128, 128, 0xff);
        st_flash_r = 255;
        st_flash_g = 255;
        st_flash_b = 255;
        st_flash_a = 64;
    }
    // bfg flash (green)
    else if(p->bfgcount)
    {
        flashcolor = D_RGBA(0, p->bfgcount & 0xff, 0, 0xff);
        st_flash_r = 0;
        st_flash_g = 255;
        st_flash_b = 0;
        st_flash_a = p->bfgcount;
    }
    // damage and strength flash (red)
    else if(p->damagecount || (p->powers[pw_strength] > 1))
    {
        int r1 = p->damagecount;
        int r2 = p->powers[pw_strength];

        if(r1)
        {
            if(r1 > ST_MAXDMGCOUNT)
                r1 = ST_MAXDMGCOUNT;
        }

        if(r2 == 1)
            r2 = 0;
        else if(r2 > ST_MAXSTRCOUNT)
            r2 = ST_MAXSTRCOUNT;

        // take priority based on value
        if(r1 > r2)
        {
            flashcolor = D_RGBA(r1 & 0xff, 0, 0, 0xff);
            st_flash_r = 255;
            st_flash_g = 0;
            st_flash_b = 0;
            st_flash_a = r1;
        }
        else
        {
            flashcolor = D_RGBA(r2 & 0xff, 0, 0, 0xff);
            st_flash_r = 255;
            st_flash_g = 0;
            st_flash_b = 0;
            st_flash_a = r2;
        }
    }
    // suit flash (green/yellow)
    else if(p->powers[pw_ironfeet] > 61 || (p->powers[pw_ironfeet] & 8))
    {
        flashcolor = D_RGBA(0, 32, 4, 0xff);
        st_flash_r = 0;
        st_flash_g = 255;
        st_flash_b = 31;
        st_flash_a = 64;
    }
    // bonus flash (yellow)
    else if(p->bonuscount)
    {
        int c1 = (p->bonuscount + 8) >> 3;
        int c2;

        if(c1 > ST_MAXBONCOUNT)
            c1 = ST_MAXBONCOUNT;

        c2 = (((c1 << 2) + c1) << 1);

        flashcolor = D_RGBA(c2 & 0xff, c2 & 0xff, c1 & 0xff, 0xff);
        st_flash_r = 255;
        st_flash_g = 255;
        st_flash_b = 0;
        st_flash_a = (p->bonuscount + 8) << 1;
    }
}

//
// ST_Init
//

void ST_Init (void)
{
    int i = 0;
    int lump;
    
    plyr = &players[consoleplayer];
    
    // setup keycards
    
    for (i = 0; i < NUMCARDS; i++)
    {
        flashCards[i].active = false;
        players[consoleplayer].tryopen[i] = false;
    }
    
    // setup hud messages
    
    ST_ClearMessage();
    
    // setup player names
    
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i] && net_player_names[i][0])
            snprintf(player_names[i], MAXPLAYERNAME, "%s", net_player_names[i]);
    }
    
    // setup chat text
    
    for(i = 0; i < MAXCHATNODES; i++)
    {
        stchat[i].msg[0] = 0;
        stchat[i].tics = 0;
        stchat[i].color = 0;
    }
    
    dmemset(st_chatstring, 0, MAXPLAYERS * MAXCHATSIZE);
    dmemset(st_chatqueue, 0, STQUEUESIZE);

    // setup crosshairs

    st_crosshairs = 0;
    lump = W_CheckNumForName("CRSHAIRS");

    if(!(lump <= -1))
        st_crosshairs = (gfxwidth[lump - g_start] / ST_CROSSHAIRSIZE);

    dmgmarkers.next = dmgmarkers.prev = &dmgmarkers;

    // setup region-specific messages

    for(i = 0; i < ST_JMESSAGES; i++)
    {
        char name[9];

        dsprintf(name, "JPMSG%02d", i + 1);
        st_jmessages[i] = W_CheckNumForName(name);

        if(st_jmessages[i] != -1)
            st_hasjmsg = true;
    }
}

//
// ST_AddChatMsg
//

void ST_AddChatMsg(char *msg, int player)
{
    char str[MAXCHATSIZE];
    
    sprintf(str, "%s: %s", player_names[player], msg);
    dmemset(stchat[st_chatcount].msg, 0, MAXCHATSIZE);
    memcpy(stchat[st_chatcount].msg, str, dstrlen(str));
    stchat[st_chatcount].tics = MAXCHATTIME;
    stchat[st_chatcount].color = st_chatcolors[player];
    st_chatcount = (st_chatcount + 1) % MAXCHATNODES;
    
    S_StartSound(NULL, sfx_darthit);
    CON_Printf(WHITE, str);
    CON_Printf(WHITE, "\n");
}

//
// ST_Notification
// Broadcast message to all clients
//

void ST_Notification(char *msg)
{
    int i;

    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i] && i != consoleplayer)
            ST_AddChatMsg(msg, i);
    }
}

//
// ST_DrawChatText
//

static void ST_DrawChatText(void)
{
    int i;
    int y = STCHATY;
    int current = (st_chatcount - 1);
    
    if(current < 0)
        current = (MAXCHATNODES - 1);
    else if(current >= (MAXCHATNODES - 1))
        current = 0;
    
    for(i = 0; i < MAXCHATNODES; i++)
    {
        if(stchat[current].msg[0] && stchat[current].tics)
        {
            
            M_DrawText(STCHATX, y, stchat[current].color, 0.5f, false, stchat[current].msg);
            y -= 8;
        }
        
        current = (current - 1) % MAXCHATNODES;
        
        if(current < 0)
            current = (MAXCHATNODES - 1);
        else if(current >= (MAXCHATNODES - 1))
            current = 0;
    }
    
    if(st_chatOn)
    {
        char tmp[MAXCHATSIZE];
        
        sprintf(tmp, "%s_", st_chatstring[consoleplayer]);
        M_DrawText(STCHATX, STCHATY + 8, WHITE, 0.5f, false, tmp);
    }
}

//
// ST_QueueChatChar
//

static void ST_QueueChatChar(char ch)
{
    if(((st_chattail+1) & (STQUEUESIZE - 1)) == st_chathead)
        return; // the queue is full
    
    st_chatqueue[st_chattail] = ch;
    st_chattail = ((st_chattail + 1) & (STQUEUESIZE - 1));
}

//
// ST_DequeueChatChar
//

char ST_DequeueChatChar(void)
{
    byte temp;
    
    if(st_chathead == st_chattail)
        return 0; // queue is empty
    
    temp = st_chatqueue[st_chathead];
    st_chathead = ((st_chathead + 1) & (STQUEUESIZE - 1));
    return temp;
}

//
// ST_FeedChatMsg
//

static dboolean st_shiftOn = false;
static void ST_FeedChatMsg(event_t *ev)
{
    int c;
    
    if(!st_chatOn)
        return;
    
    if(!(c = ev->data1))
        return;
    
    switch(c)
    {
        int len;
        
    case KEY_ENTER:
    case KEY_KEYPADENTER:
        ST_QueueChatChar((char)c);
        st_chatOn = false;
        break;
    case KEY_BACKSPACE:
        if(ev->type != ev_keydown)
            return;
        ST_QueueChatChar((char)c);
        break;
    case KEY_ESCAPE:
        len = dstrlen(st_chatstring[consoleplayer]);
        st_chatOn = false;
        dmemset(st_chatstring[consoleplayer], 0, len);
        break;
    case KEY_CAPS:
        if(ev->type == ev_keydown)
            st_shiftOn ^= 1;
        break;
    case KEY_SHIFT:
        if(ev->type == ev_keydown)
            st_shiftOn = true;
        else if(ev->type == ev_keyup)
            st_shiftOn = false;
        break;
    case KEY_ALT:
    case KEY_PAUSE:
    case KEY_TAB:
    case KEY_RIGHTARROW:
    case KEY_LEFTARROW:
    case KEY_UPARROW:
    case KEY_DOWNARROW:
    case KEY_F1:
    case KEY_F2:
    case KEY_F3:
    case KEY_F4:
    case KEY_F5:
    case KEY_F6:
    case KEY_F7:
    case KEY_F8:
    case KEY_F9:
    case KEY_F10:
    case KEY_F11:
    case KEY_F12:
    case KEY_INSERT:
    case KEY_HOME:
    case KEY_PAGEUP:
    case KEY_PAGEDOWN:
    case KEY_DEL:
    case KEY_END:
    case KEY_SCROLLLOCK:
    case KEY_NUMLOCK:
        break; // too lazy to do anything clever here..
    default:
        if(ev->type != ev_keydown)
            return;
        
        if(st_shiftOn)
            c = shiftxform[c];
        ST_QueueChatChar((char)c);
        break;
    }
}

//
// ST_EatChatMsg
//

static void ST_EatChatMsg(void)
{
    int c;
    int len;
    int i;
    
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(!playeringame[i]) continue;
        if(!players[i].cmd.chatchar) continue;
        
        c = players[i].cmd.chatchar;
        
        len = dstrlen(st_chatstring[i]);
        
        switch(c)
        {
        case KEY_ENTER:
        case KEY_KEYPADENTER:
            ST_AddChatMsg(st_chatstring[i], i);
            dmemset(st_chatstring[i], 0, len);
            break;
        case KEY_BACKSPACE:
            st_chatstring[i][MAX(len--, 0)] = 0;
            break;
        default:
            st_chatstring[i][len] = c;
            break;
        }
    }
}

//
// ST_Responder
// Respond to keyboard input events, intercept cheats.
//

dboolean ST_Responder(event_t* ev)
{
    M_CheatProcess(plyr, ev);

    if(netgame)
    {
        ST_FeedChatMsg(ev);
        
        if(st_chatOn)
            return true;
        
        if(ev->type == ev_keydown && ev->data1 == 't')
            st_chatOn = true;
    }
    
    return false;
}

//
// ST_DisplayName
//

static void ST_DisplayName(int playernum)
{
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    fixed_t     xangle;
    fixed_t     yangle;
    fixed_t     screenx;
    fixed_t     xpitch;
    fixed_t     ypitch;
    fixed_t     screeny;
    player_t*   player;
    char        name[MAXPLAYERNAME];
    rcolor      color;
    fixed_t     distance;

    // don't display self
    if(playernum == consoleplayer)
        return;

    player = &players[playernum];    

    // get distance
    distance = P_AproxDistance(viewx - player->mo->x, viewy - player->mo->y);
    if(distance > (1280*FRACUNIT))
        return;     // too far

    x = player->mo->x - viewx;
    y = player->mo->y - viewy;
    z = player->mo->z - (players[consoleplayer].viewz - (96*FRACUNIT));
        
    // set relative viewpoint
    xangle = (FixedMul(dsin(viewangle), x) - FixedMul(dcos(viewangle), y));
    yangle = (FixedMul(dsin(viewangle), y) + FixedMul(dcos(viewangle), x));
    xpitch = (FixedMul(dsin(viewpitch), yangle) - FixedMul(dcos(viewpitch), z));
    ypitch = (FixedMul(dsin(viewpitch), z) + FixedMul(dcos(viewpitch), yangle));
    
    // check x offscreen
    if(xangle < -yangle)
        return;
    
    // check y offscreen
    if(yangle < xangle)
        return;
    
    // check if behind view
    if(yangle < 0x80001)
        return;

    if(ypitch > xpitch)
        return;
    
    // adjust if needed
    if(yangle < 0x80000)
    {
        xangle += FixedMul(FixedDiv(0x80000 - yangle,
            xangle - yangle), yangle - xangle);
        yangle = 0x80000;
    }

    // convert to screen space
    screenx = ((FixedDiv(xangle, yangle) * (SCREENWIDTH/2)) >> FRACBITS) + (SCREENWIDTH/2);
    screeny = (((FixedDiv(ypitch, xpitch) * -(SCREENWIDTH/2)) >> FRACBITS) + (SCREENWIDTH/2)) - 40;
    
    if(screenx < 0)
        screenx = 0;

    if(screenx > SCREENWIDTH)
        screenx = SCREENWIDTH;

    if(screeny < 0)
        screeny = 0;

    if(screeny > SCREENHEIGHT)
        screeny = SCREENHEIGHT;

    // change colors based on health/condition
    if(player->health < 40)
        color = RED;
    else if(player->health < 80)
        color = YELLOW;
    else
        color = WHITE;

    // fade alpha based on distance. farther will mean less alpha
    distance /= FRACUNIT;

    // reset alpha and set new value
    color ^= (((color >> 24) & 0xff) << 24);
    color |= ((255 - (int)((float)distance * 0.19921875f)) << 24);

    // display player name
    dsnprintf(name, MAXPLAYERNAME, "%s", player_names[playernum]);
    M_DrawText(screenx, screeny, color, 1.0f, 0, name);
}


