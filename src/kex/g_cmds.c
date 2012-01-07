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
//      Doom3D console command system
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "";
#endif

#include "g_local.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "i_system.h"
#include "m_cheat.h"
#include "m_fixed.h"
#include "tables.h"
#include "p_local.h"
#include "info.h"
#include "m_misc.h"
#include "v_sdl.h"
#include "con_console.h"
#include "r_local.h"

#include <stdlib.h>

void G_CmdButton(int data, char **param);
void G_CmdWeapon(int data, char **param);
void G_CmdNextWeapon(int data, char **param);
void G_CmdPrevWeapon(int data, char **param);
void G_CmdAutorun(int data, char **param);
void G_CmdBind(int data, char **param);
void G_CmdSeta(int data, char **param);
void G_CmdQuit(int data, char **param);
void G_CmdExec(int data, char **param);
void G_CmdList(int data, char **param);
void G_CmdCheat(int data, char **param);
void G_CmdPause(int data, char **param);
void G_CmdWireframe(int data, char** param);
void G_CmdSpawnThing(int data, char** param);
void G_CmdLockMonsters(int data, char **param);
void G_CmdExitLevel(int data, char **param);
void G_CmdTriggerSpecial(int data, char** param);
void G_CmdPrintGLExt(int data, char** param);
void G_CmdPlayerCamera(int data, char** param);

struct
{
    char            *name;
    actionproc_t    proc;
    int             data;
    dboolean        allownet;
} Cmds[]=
{
    {"+fire",           G_CmdButton,        PCKEY_ATTACK,               true},
    {"-fire",           G_CmdButton,        PCKEY_ATTACK|PCKF_UP,       true},
    {"+strafe",         G_CmdButton,        PCKEY_STRAFE,               true},
    {"-strafe",         G_CmdButton,        PCKEY_STRAFE|PCKF_UP,       true},
    {"+use",            G_CmdButton,        PCKEY_USE,                  true},
    {"-use",            G_CmdButton,        PCKEY_USE|PCKF_UP,          true},
    {"+run",            G_CmdButton,        PCKEY_RUN,                  true},
    {"-run",            G_CmdButton,        PCKEY_RUN|PCKF_UP,          true},
    {"+jump",           G_CmdButton,        PCKEY_JUMP,                 true},
    {"-jump",           G_CmdButton,        PCKEY_JUMP|PCKF_UP,         true},
    {"weapon",          G_CmdWeapon,        0,                          true},
    {"nextweap",        G_CmdNextWeapon,    0,                          true},
    {"prevweap",        G_CmdPrevWeapon,    0,                          true},
    {"+forward",        G_CmdButton,        PCKEY_FORWARD,              true},
    {"-forward",        G_CmdButton,        PCKEY_FORWARD|PCKF_UP,      true},
    {"+back",           G_CmdButton,        PCKEY_BACK,                 true},
    {"-back",           G_CmdButton,        PCKEY_BACK|PCKF_UP,         true},
    {"+left",           G_CmdButton,        PCKEY_LEFT,                 true},
    {"-left",           G_CmdButton,        PCKEY_LEFT|PCKF_UP,         true},
    {"+right",          G_CmdButton,        PCKEY_RIGHT,                true},
    {"-right",          G_CmdButton,        PCKEY_RIGHT|PCKF_UP,        true},
    {"+lookup",         G_CmdButton,        PCKEY_LOOKUP,               true},
    {"-lookup",         G_CmdButton,        PCKEY_LOOKUP|PCKF_UP,       true},
    {"+lookdown",       G_CmdButton,        PCKEY_LOOKDOWN,             true},
    {"-lookdown",       G_CmdButton,        PCKEY_LOOKDOWN|PCKF_UP,     true},
    {"+center",         G_CmdButton,        PCKEY_CENTER,               true},
    {"-center",         G_CmdButton,        PCKEY_CENTER|PCKF_UP,       true},
    {"autorun",         G_CmdAutorun,       0,                          true},//convert to cvar
    {"+strafeleft",     G_CmdButton,        PCKEY_STRAFELEFT,           true},
    {"-strafeleft",     G_CmdButton,        PCKEY_STRAFELEFT|PCKF_UP,   true},
    {"+straferight",    G_CmdButton,        PCKEY_STRAFERIGHT,          true},
    {"-straferight",    G_CmdButton,        PCKEY_STRAFERIGHT|PCKF_UP,  true},
    {"bind",            G_CmdBind,          0,                          true},
    {"seta",            G_CmdSeta,          0,                          true},
    {"quit",            G_CmdQuit,          0,                          true},
    {"exec",            G_CmdExec,          0,                          false},
    {"alias",           G_CmdAlias,         0,                          false},
    {"cmdlist",         G_CmdList,          0,                          true},
    {"unbind",          G_CmdUnbind,        0,                          true},
    {"unbindall",       G_CmdUnbindAll,     0,                          true},
    {"god",             G_CmdCheat,         0,                          false},
    {"noclip",          G_CmdCheat,         1,                          false},
    {"pause",           G_CmdPause,         0,                          false},
    {"wireframe",       G_CmdWireframe,     0,                          true},
    {"spawnthing",      G_CmdSpawnThing,    0,                          false},
    {"exitlevel",       G_CmdExitLevel,     0,                          false},
    {"trigger",         G_CmdTriggerSpecial,0,                          false},
    {"printglext",      G_CmdPrintGLExt,    0,                          true},
    {"setcamerastatic", G_CmdPlayerCamera,  0,                          true},
    {"setcamerachase",  G_CmdPlayerCamera,  1,                          true},
    {NULL, NULL}
};

//
// G_InitCmds
//

void G_InitCmds(void)
{
    int i;

    for(i = 0; Cmds[i].name; i++)
    {
        G_RegisterAction(Cmds[i].name, Cmds[i].proc, Cmds[i].data, Cmds[i].allownet);
    }
}

//
// G_CmdButton
//

void G_CmdButton(int data, char **param)
{
    playercontrols_t    *pc;
    int                 key;
    
    pc = &Controls;
    
    key = data & PCKF_COUNTMASK;
    
    if (data & PCKF_UP)
    {
        if((pc->key[key] & PCKF_COUNTMASK) > 0)
            pc->key[key]--;

        if(ButtonAction)
            pc->key[key] &= ~PCKF_DOUBLEUSE;
    }
    else
    {
        pc->key[key]++;

        if(ButtonAction)
            pc->key[key] |= PCKF_DOUBLEUSE;
    }
}

//
// G_DoCmdMouseMove
//

void G_DoCmdMouseMove(int x, int y)
{
    playercontrols_t *pc;
    
    pc = &Controls;
    pc->mousex += ((V_MouseAccel(x) * (int)v_msensitivityx.value) / 128);
    pc->mousey += ((V_MouseAccel(y) * (int)v_msensitivityy.value) / 128);
}

//
// G_CmdNextWeapon
//

void G_CmdNextWeapon(int data, char **param)
{
    playercontrols_t *pc;
    
    pc = &Controls;
    pc->flags |= PCF_NEXTWEAPON;
}

//
// G_CmdPrevWeapon
//

void G_CmdPrevWeapon(int data, char **param)
{
    playercontrols_t *pc;
    
    pc = &Controls;
    pc->flags |= PCF_PREVWEAPON;
}

//
// G_CmdWeapon
//

void G_CmdWeapon(int data, char **param)
{
    playercontrols_t    *pc;
    int                 id;
    
    if(!(param[0]))
        return;
    
    id = datoi(param[0]);
    
    if((id > NUMWEAPONS) || (id < 1))
        return;
    
    pc = &Controls;
    pc->nextweapon = id - 1;
}

//
// G_CmdBind
//

void G_CmdBind(int data, char **param)
{
    if(!param[0])
        return;
    
    if(!param[1])
    {
        G_ShowBinding(param[0]);
        return;
    }

    G_BindActionByName(param[0], param[1]);
}

//
// G_CmdSeta
//

void G_CmdSeta(int data, char **param)
{
    if(!param[0] || !param[1])
        return;

    CON_CvarSet(param[0], param[1]);

    if(netgame)
    {
        if(playeringame[0] && consoleplayer == 0)
            NET_SV_UpdateCvars(CON_CvarGet(param[0]));
    }
}

//
// G_CmdAutorun
//

void G_CmdAutorun(int data, char **param)
{
    if(gamestate != GS_LEVEL)
        return;

    if(p_autorun.value == 0)
    {
        CON_CvarSetValue(p_autorun.name, 1);
        players[consoleplayer].message = GGAUTORUNON;
    }
    else
    {
        CON_CvarSetValue(p_autorun.name, 0);
        players[consoleplayer].message = GGAUTORUNOFF;
    }
}

//
// G_CmdQuit
//

void G_CmdQuit(int data, char **param)
{
    I_Quit();
}

//
// G_CmdExec
//

void G_CmdExec(int data, char **param)
{
    G_ExecuteFile(param[0]);
}

//
// G_CmdList
//

void G_CmdList(int data, char **param)
{
    int cmds;
    
    I_Printf("Available commands:\n");
    cmds=G_ListCommands();
    I_Printf("(%d commands)\n", cmds);
}

//
// G_CmdCheat
//

void G_CmdCheat(int data, char **param)
{
    player_t *player;

    if(gamestate != GS_LEVEL)
        return;
    
    player = &players[consoleplayer];
    switch(data)
    {
    case 0:
        M_CheatGod(player, NULL);
        break;
    case 1:
        M_CheatClip(player, NULL);
        break;
    }
}

//
// G_CmdPause
//

void G_CmdPause(int data, char **param)
{
    sendpause = true;
}

//
// G_CmdWireframe
//

void G_CmdWireframe(int data, char** param)
{
    dboolean b;
    
    if(!param[0])
        return;
    
    b = datoi(param[0]) & 1;
    R_DrawWireframe(b);
}

//
// G_CmdSpawnThing
//

void G_CmdSpawnThing(int data, char** param)
{
    int id = 0;
    player_t *player;
    mobj_t *thing;
    fixed_t x, y, z;

    if(gamestate != GS_LEVEL)
        return;
    
    if(!param[0])
        return;
    
    if(netgame)
        return;
    
    id = datoi(param[0]);
    if(id >= NUMMOBJTYPES || id < 0)
        return;
    
    player = &players[consoleplayer];
    x = player->mo->x + FixedMul(INT2F(64) + mobjinfo[id].radius, dcos(player->mo->angle));
    y = player->mo->y + FixedMul(INT2F(64) + mobjinfo[id].radius, dsin(player->mo->angle));
    z = player->mo->z;
    
    thing = P_SpawnMobj(x, y, z, id);

    if(thing->info->spawnstate == S_000)
    {
        P_RemoveMobj(thing);
        return;
    }

    thing->angle = player->mo->angle;
}

//
// G_CmdExitLevel
//

void G_CmdExitLevel(int data, char **param)
{
    if(gamestate != GS_LEVEL)
        return;

    if(demoplayback)
        return;
    
    if(!param[0])
        G_ExitLevel();
    else
        G_SecretExitLevel(datoi(param[0]));
}

//
// G_CmdTriggerSpecial
//

void G_CmdTriggerSpecial(int data, char** param)
{
    line_t junk;

    if(gamestate != GS_LEVEL)
        return;
    
    if(!param[0])
        return;
    
    dmemset(&junk, 0, sizeof(line_t));
    junk.special = datoi(param[0]);
    junk.tag = datoi(param[1]);
    
    P_DoSpecialLine(players[consoleplayer].mo, &junk, 0);
}

//
// G_CmdPrintGLExt
//

void G_CmdPrintGLExt(int data, char** param)
{
    char *string;
    int i = 0;
    int len = 0;
    
    string = (char*)dglGetString(GL_EXTENSIONS);
    len = dstrlen(string);
    
    for(i = 0; i < len; i++)
    {
        if(string[i] == 0x20)
            string[i] = '\n';
    }
    
    M_WriteTextFile("GL_EXTENSIONS.TXT", string, len);
    CON_Printf(WHITE, "Written GL_EXTENSIONS.TXT\n");
}

//
// G_CmdPlayerCamera
//

void G_CmdPlayerCamera(int data, char** param)
{
    player_t *player;

    if(gamestate != GS_LEVEL)
        return;
    
    player = &players[consoleplayer];

    switch(data)
    {
    case 0:
        P_SetStaticCamera(player);
        break;

    case 1:
        P_SetFollowCamera(player);
        break;
    }
}


