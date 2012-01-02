// Emacs style mode select	 -*- C++ -*-
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
// $Log: g_game.c,v $
// Revision 1.1  2008/05/18 22:28:33  svkaiser
// Initial submission
//
//
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------

#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif

#include <stdlib.h>
#include <stdarg.h>

#include "doomdef.h"
#include "doomstat.h"
#include "z_zone.h"
#include "f_finale.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_cheat.h"
#include "m_random.h"
#include "i_system.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"
#include "d_main.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "w_wad.h"
#include "p_local.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "sounds.h"
#include "tables.h"
#include "info.h"
#include "r_local.h"
#include "r_wipe.h"
#include "con_console.h"
#include "g_local.h"
#include "m_password.h"

#define DCLICK_TIME     20
#define DEMOMARKER      0x80

dboolean    G_CheckDemoStatus(void);
void        G_ReadDemoTiccmd(ticcmd_t* cmd);
void        G_WriteDemoTiccmd(ticcmd_t* cmd);
void        G_PlayerReborn(int player);
void        G_InitNew(skill_t skill, int map);
void        G_DoReborn(int playernum);
void        G_DoLoadGame(void);
void        G_SetFastParms(int fast_pending);


gameaction_t    gameaction = 0;
gamestate_t     gamestate = 0;
skill_t         gameskill = 0;
dboolean        respawnmonsters = false;
dboolean        respawnspecials = false;
int             gamemap = 0;
int             nextmap = 0;

dboolean        paused = false;
dboolean        sendpause = false;          // send a pause event next tic
dboolean        sendsave = false;           // send a save event next tic
dboolean        usergame = false;           // ok to save / end game

dboolean        timingdemo = false;         // if true, exit with report on completion
dboolean        nodrawers = false;          // for comparative timing purposes
dboolean        noblit = false;             // for comparative timing purposes
int             starttime = 0;              // for comparative timing purposes

int             deathmatch = false;         // only if started as net death
dboolean        netcheat = false;
dboolean        netkill = false;
dboolean        netgame = false;            // only true if packets are broadcast
dboolean        playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

int             consoleplayer;              // player taking events and displaying
int             displayplayer;              // view being displayed
int             gametic = 0;

static dboolean savenow = false;

// for intermission
int             totalkills, totalitems, totalsecret;

char            demoname[32];
dboolean        demorecording=false;
dboolean        demoplayback=false;
dboolean        netdemo=false;
byte*           demobuffer;
byte*           demo_p;
byte*           demoend;
dboolean        singledemo=false;           // quit after playing a demo from cmdline

dboolean        precache = true;            // if true, load all graphics at start

byte            consistancy[MAXPLAYERS][BACKUPTICS];

#define MAXPLMOVE       (forwardmove[1])
#define TURBOTHRESHOLD  0x32

fixed_t     forwardmove[2] =    { 0x1c, 0x2c };
fixed_t     sidemove[2] =       { 0x1c, 0x2c };
fixed_t     angleturn[20] =     { 0x32, 0x32, 0x53, 0x53, 0x64, 0x74, 0x85, 0x96, 0x96, 0xA6,
                                  0x85, 0x85, 0x96, 0xA6, 0xA6, 0xC8, 0xC8, 0xD8, 0xD8, 0xE9
                                };

#define SLOWTURNTICS    10

int         turnheld;                       // for accelerative turning
int         lookheld;

int         savegameslot;
char        savedescription[32];

playercontrols_t    Controls;

#define BODYQUESIZE 32

mobj_t*     bodyque[BODYQUESIZE];
int         bodyqueslot;
void*       statcopy=NULL;				// for statistics driver


//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//

void G_BuildTiccmd(ticcmd_t* cmd)
{
    int                 i;
    int                 speed;
    int                 forward;
    int                 side;
    playercontrols_t    *pc;
    
    pc = &Controls;
    dmemset(cmd, 0, sizeof(ticcmd_t));
    
    cmd->consistancy = consistancy[consoleplayer][maketic % BACKUPTICS];
    
    if(pc->key[PCKEY_RUN])
        speed = 1;
    else
        speed=0;
    
    if(p_autorun.value)
        speed = !speed;
    
    forward = side = 0;
    
    // use two stage accelerative turning
    // on the keyboard and joystick
    if(pc->key[PCKEY_LEFT] || pc->key[PCKEY_RIGHT])
        turnheld += ticdup;
    else
        turnheld = 0;
    
    if(turnheld >= SLOWTURNTICS)
        turnheld = SLOWTURNTICS-1;
    
    if(pc->key[PCKEY_LOOKUP] || pc->key[PCKEY_LOOKDOWN])
        lookheld += ticdup;
    else
        lookheld = 0;
    
    if(lookheld >= SLOWTURNTICS)
        lookheld = SLOWTURNTICS-1;
    
    if(pc->key[PCKEY_STRAFE])
    {
        if(pc->key[PCKEY_RIGHT])
        {
            side += sidemove[speed];
        }
        if(pc->key[PCKEY_LEFT])
        {
            side -= sidemove[speed];
        }
        side += sidemove[1] * pc->mousex * 2;
    }
    else
    {
        if(pc->key[PCKEY_RIGHT])
            cmd->angleturn -= angleturn[turnheld + (speed ? SLOWTURNTICS : 0)] << 2;
        
        if(pc->key[PCKEY_LEFT])
            cmd->angleturn += angleturn[turnheld + (speed ? SLOWTURNTICS : 0)] << 2;
        
        if(pc->key[PCKEY_LOOKUP])
            cmd->pitch += angleturn[lookheld + (speed ? SLOWTURNTICS : 0)] << 2;
        
        if(pc->key[PCKEY_LOOKDOWN])
            cmd->pitch -= angleturn[lookheld + (speed ? SLOWTURNTICS : 0)] << 2;
        
        cmd->angleturn -= pc->mousex * 0x8;
        
        if((int)v_mlook.value)
            cmd->pitch -= (int)v_mlookinvert.value ? pc->mousey * 0x8 : -(pc->mousey * 0x8);
    }
    
    if(pc->key[PCKEY_CENTER])
        cmd->buttons2 |= BT2_CENTER;
    
    if(pc->key[PCKEY_FORWARD])
        forward += forwardmove[speed];

    //
    // forward/side movement with joystick
    //
    if(pc->flags & PCF_GAMEPAD)
    {
        forward += pc->joyy;
        side += pc->joyx;
    }
    
    if(pc->key[PCKEY_BACK])
        forward -= forwardmove[speed];
    
    if(pc->key[PCKEY_STRAFERIGHT])
        side += sidemove[speed];
    
    if(pc->key[PCKEY_STRAFELEFT])
        side -= sidemove[speed];
    
    pc->mousex = pc->mousey = 0;
    pc->joyx = pc->joyy = 0;
    
    cmd->chatchar = ST_DequeueChatChar();
    
    if(pc->key[PCKEY_ATTACK])
        cmd->buttons |= BT_ATTACK;
    
    if(pc->key[PCKEY_USE])
    {
        cmd->buttons |= BT_USE;
        // clear double clicks if hit use button
        pc->flags &= ~(PCF_FDCLICK2|PCF_SDCLICK2);
    }
    
    if((int)p_allowjump.value)
    {
        if(pc->key[PCKEY_JUMP])
            cmd->buttons2 |= BT2_JUMP;
    }
    
    if(pc->flags & PCF_NEXTWEAPON)
    {
        cmd->buttons |= BT_CHANGE;
        cmd->buttons2 |= BT2_NEXTWEAP;
        pc->flags &= ~PCF_NEXTWEAPON;
    }
    else if(pc->flags & PCF_PREVWEAPON)
    {
        cmd->buttons |= BT_CHANGE;
        cmd->buttons2 |= BT2_PREVWEAP;
        pc->flags &= ~PCF_PREVWEAPON;
    }
    else if(pc->nextweapon != wp_nochange)
    {
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= pc->nextweapon << BT_WEAPONSHIFT;
        pc->nextweapon = wp_nochange;
    }
    
    //doubleclick use
    i=pc->flags & PCF_FDCLICK;
    if(pc->key[PCKEY_FORWARD] & PCKF_DOUBLEUSE)
        i ^= PCF_FDCLICK;
    
    if(i)
    {
        pc->flags ^= PCF_FDCLICK;
        if(pc->key[PCKEY_FORWARD] & PCKF_DOUBLEUSE)
        {
            if(pc->flags & PCF_FDCLICK2)
            {
                if(p_fdoubleclick.value)
                    cmd->buttons |= BT_USE;

                pc->flags &= ~PCF_FDCLICK2;
            }
            else
                pc->flags |= PCF_FDCLICK2;
        }
        pc->fdclicktime = 0;
    }
    else if(pc->fdclicktime >= 0)
    {
        pc->fdclicktime += ticdup;
        if(pc->fdclicktime > DCLICK_TIME)
        {
            pc->flags &= ~PCF_FDCLICK2;
            pc->fdclicktime = -1;
        }
    }
    
    i = pc->flags & PCF_SDCLICK;
    if(pc->key[PCKEY_STRAFE] & PCKF_DOUBLEUSE)
        i ^= PCF_SDCLICK;
    
    if(i)
    {
        pc->flags ^= PCF_SDCLICK;
        if(pc->key[PCKEY_STRAFE] & PCKF_DOUBLEUSE)
        {
            if(pc->flags & PCF_SDCLICK2)
            {
                if(p_sdoubleclick.value)
                    cmd->buttons |= BT_USE;

                pc->flags &= ~PCF_SDCLICK2;
            }
            else
                pc->flags |= PCF_SDCLICK2;
        }
        pc->sdclicktime = 0;
    }
    else if(pc->sdclicktime >= 0)
    {
        pc->sdclicktime += ticdup;
        if(pc->sdclicktime > DCLICK_TIME)
        {
            pc->flags &= ~PCF_SDCLICK2;
            pc->sdclicktime = -1;
        }
    }
    
    
    if(forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if(forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;

    if(side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if(side < -MAXPLMOVE)
        side = -MAXPLMOVE;
    
    cmd->forwardmove += forward;
    cmd->sidemove += side;
    
    // special buttons
    if(sendpause)
    {
        sendpause = false;
        cmd->buttons = BT_SPECIAL | BTS_PAUSE;
    }
    
    if(sendsave)
    {
        sendsave = false;
        cmd->buttons = BT_SPECIAL | BTS_SAVEGAME | (savegameslot << BTS_SAVESHIFT);
    }
}


//
// G_ClearInput
//

void G_ClearInput(void)
{
    int                 i;
    playercontrols_t    *pc;
    
    pc = &Controls;
    pc->flags = 0;
    pc->nextweapon = wp_nochange;
    for(i = 0; i < NUM_PCKEYS; i++)
        pc->key[i] = 0;
}

//
// G_DoLoadLevel
//

void G_DoLoadLevel (void)
{
    int i;
    
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;
        
        dmemset(players[i].frags, 0, sizeof(players[i].frags));
    }

    // update settings from server cvar
    gameskill   = (int)sv_skill.value;
    respawnparm = (int)sv_respawn.value;
    respawnitem = (int)sv_respawnitems.value;
    fastparm    = (int)sv_fastmonsters.value;
    nomonsters  = (int)sv_nomonsters.value;

    // This was quite messy with SPECIAL and commented parts.
    // Supposedly hacks to make the latest edition work.
    // It might not work properly.
    
    G_SetFastParms(fastparm || gameskill == sk_nightmare);  // killough 4/10/98

    if(gameskill == sk_nightmare || respawnparm )
        respawnmonsters = true;
    else
        respawnmonsters = false;
    
    if(gameskill == sk_nightmare || respawnitem)
        respawnspecials = true;
    else
        respawnspecials = false;
    
    P_SetupLevel(gamemap, 0, gameskill);
    displayplayer = consoleplayer;		// view the guy you are playing
    starttime = I_GetTime();
    gameaction = ga_nothing;
    Z_CheckHeap();
    
    // clear cmd building stuff
    G_ClearInput();
    sendpause = sendsave = paused = false;
}


//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//

dboolean G_Responder(event_t* ev)
{
    // Handle level specific ticcmds
    if(gamestate == GS_LEVEL)
    {
        // allow spy mode changes even during the demo
        if(ev->type == ev_keydown
            && ev->data1 == KEY_F12 && (singledemo || !deathmatch))
        {
            // spy mode
            do
            {
                displayplayer++;
                if(displayplayer == MAXPLAYERS)
                    displayplayer = 0;

            } while(!playeringame[displayplayer] && displayplayer != consoleplayer);

            return true;
        }

        if(demoplayback)
        {
            if(ev->type == ev_keydown ||
                ev->type == ev_gamepad)
            {
                G_CheckDemoStatus();
                gameaction = ga_warpquick;
                return true;
            }
            else
                return false;
        }
        
        if(ST_Responder(ev))
            return true;	// status window ate it

        if(AM_Responder(ev))
            return true;	// automap ate it
    }

    // Handle screen specific ticcmds
    if(gamestate == GS_SKIPPABLE)
    {
        if(gameaction == ga_nothing)
        {
            if(ev->type == ev_keydown ||
                (ev->type == ev_mouse && ev->data1) ||
                ev->type == ev_gamepad)
            {
                gameaction = ga_title;
                return true;
            }
            return false;
        }
    }
    
    if((ev->type == ev_keydown) && (ev->data1 == KEY_PAUSE))
    {
        sendpause = true;
        return true;
    }
    
    if(G_ActionResponder(ev))
        return true;
    
    return false;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker(void)
{
    int         i;
    int         buf;
    ticcmd_t*   cmd;
    
    G_ActionTicker();
    CON_Ticker();

    if(savenow)
    {
        G_DoSaveGame();
        savenow = false;
    }

    if(gameaction == ga_screenshot)
    {
        M_ScreenShot();
        gameaction = ga_nothing;
    }
        
    // get commands, check consistancy,
    // and build new consistancy check
    buf = (gametic / ticdup) % BACKUPTICS;
        
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i])
        {
            cmd = &players[i].cmd;
                
            dmemcpy(cmd, &netcmds[i][buf], sizeof(ticcmd_t));
                
            if(demoplayback)
                G_ReadDemoTiccmd(cmd);
            if(demorecording)
                G_WriteDemoTiccmd(cmd);
                
            // check for turbo cheats
            if(cmd->forwardmove > TURBOTHRESHOLD
                && !(gametic&31) && ((gametic>>5)&3) == i)
            {
                static char turbomessage[80];
                sprintf(turbomessage, "%s is turbo!",player_names[i]);
                players[consoleplayer].message = turbomessage;
            }
                
            if(netgame && !netdemo && !(gametic % ticdup))
            {
                if(gametic > BACKUPTICS
                    && consistancy[i][buf] != cmd->consistancy)
                {
                    I_Error("consistency failure (%i should be %i)",
                        cmd->consistancy, consistancy[i][buf], consoleplayer);
                }
                if(players[i].mo)
                    consistancy[i][buf] = players[i].mo->x;
                else
                    consistancy[i][buf] = rndindex;
            }
        }
    }
        
    // check for special buttons
    for(i = 0; i < MAXPLAYERS; i++)
    {
        if(playeringame[i])
        {
            if(players[i].cmd.buttons & BT_SPECIAL)
            {
            /*villsa - fixed crash when player restarts level after dying
                Changed switch statments to if statments*/
                if((players[i].cmd.buttons & BT_SPECIALMASK) == BTS_PAUSE)
                {
                    paused ^= 1;
                    if(paused)
                        S_PauseSound();
                    else
                        S_ResumeSound();
                }
                    
                if((players[i].cmd.buttons & BT_SPECIALMASK) == BTS_SAVEGAME)
                {
                    if(!savedescription[0])
                        dstrcpy (savedescription, "NET GAME");
                    savegameslot =
                        (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
                    savenow = true;
                }
            }
        }
    }
}


//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Mobj
//

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//

void G_PlayerFinishLevel(int player)
{
    player_t*   p;
    
    p = &players[player];
    
    dmemset(p->powers, 0, sizeof(p->powers));
    dmemset(p->cards, 0, sizeof(p->cards));
    p->mo->flags &= ~MF_SHADOW; 	// cancel invisibility
    p->damagecount = 0; 		// no palette changes
    p->bonuscount = 0;
    p->bfgcount = 0;
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//

void G_PlayerReborn(int player)
{
    player_t    *p;
    int         i;
    int         frags[MAXPLAYERS];
    int         killcount;
    int         itemcount;
    int         secretcount;
    dboolean    cards[NUMCARDS];
    dboolean    wpns[NUMWEAPONS];
    int         pammo[NUMAMMO];
    int         pmaxammo[NUMAMMO];
    int         artifacts;
    dboolean    backpack;
    
    dmemcpy(frags, players[player].frags, sizeof(frags));
    dmemcpy(cards, players[player].cards, sizeof(dboolean)*NUMCARDS);
    dmemcpy(wpns, players[player].weaponowned, sizeof(dboolean)*NUMWEAPONS);
    dmemcpy(pammo, players[player].ammo, sizeof(int)*NUMAMMO);
    dmemcpy(pmaxammo, players[player].maxammo, sizeof(int)*NUMAMMO);

    backpack = players[player].backpack;
    artifacts = players[player].artifacts;
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;
    
    quakeviewx = 0;
    quakeviewy = 0;
    infraredFactor = 0;
    R_RefreshBrightness();
    
    p = &players[player];
    dmemset(p, 0, sizeof(*p));
    
    dmemcpy(players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;
    
    p->usedown = p->attackdown = p->jumpdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = 50;
    p->recoilpitch = 0;
    
    for(i = 0; i < NUMAMMO; i++)
        p->maxammo[i] = maxammo[i];

    p->artifacts = artifacts;

    if(netgame)
    {
        for(i = 0; i < NUMCARDS; i++)
            players[player].cards[i] = cards[i];

        if(sv_keepitems.value)
        {
            p->backpack = backpack;

            for(i = 0; i < NUMAMMO; i++)
            {
                p->ammo[i] = pammo[i];
                p->maxammo[i] = pmaxammo[i];
            }

            for(i = 0; i < NUMWEAPONS; i++)
                p->weaponowned[i] = wpns[i];
        }
    }
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//

dboolean G_CheckSpot(int playernum, mapthing_t* mthing)
{
    fixed_t         x;
    fixed_t         y;
    subsector_t*    ss;
    angle_t         an;
    mobj_t*         mo;
    int             i;
    
    if(!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for(i = 0; i < playernum; i++)
        {
            if((players[i].mo->x == INT2F(mthing->x)) && (players[i].mo->y == INT2F(mthing->y)))
                return false;
        }
        return true;
    }
    
    x = INT2F(mthing->x);
    y = INT2F(mthing->y);
    
    if(!P_CheckPosition(players[playernum].mo, x, y) )
        return false;
    
    // flush an old corpse if needed
    if(bodyqueslot >= BODYQUESIZE)
        P_RemoveMobj(bodyque[bodyqueslot % BODYQUESIZE]);
    bodyque[bodyqueslot%BODYQUESIZE] = players[playernum].mo;
    bodyqueslot++;
    
    // spawn a teleport fog
    ss = R_PointInSubsector(x, y);
    an = (ANG45 * (mthing->angle/45)) >> ANGLETOFINESHIFT;
    
    mo = P_SpawnMobj(x + 20*finecosine[an], y + 20*finesine[an],
        ss->sector->floorheight, MT_TELEPORTFOG);
    
    if(players[playernum].viewz != 1)
        S_StartSound(mo, sfx_telept);	// don't start sound on first frame
    
    return true;
}


//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//

void G_DeathMatchSpawnPlayer(int playernum)
{
    int i, j;
    int selections;
    
    selections = deathmatch_p - deathmatchstarts;
    if(selections < 4)
        I_Error ("G_DeathMatchSpawnPlayer: Only %i deathmatch spots, 4 required", selections);
    
    for(j = 0; j < 20; j++)
    {
        i = P_Random() % selections;
        if(G_CheckSpot(playernum, &deathmatchstarts[i]) )
        {
            deathmatchstarts[i].type = playernum+1;
            P_SpawnPlayer(&deathmatchstarts[i]);
            return;
        }
    }
    
    // no good spot, so the player will probably get stuck
    P_SpawnPlayer(&playerstarts[playernum]);
}

//
// G_DoReborn
//

void G_DoReborn(int playernum)
{
    if(!netgame)
        gameaction = ga_loadlevel;      // reload the level from scratch
    else	 // respawn at the start
    {
        int i;
        
        // first dissasociate the corpse
        if(players[playernum].mo == NULL)
            I_Error("G_DoReborn: Player start #%i not found!", playernum+1);
        
        players[playernum].mo->player = NULL;
        
        // spawn at random spot if in death match
        if(deathmatch)
        {
            G_DeathMatchSpawnPlayer (playernum);
            return;
        }
        
        if(G_CheckSpot (playernum, &playerstarts[playernum]) )
        {
            P_SpawnPlayer (&playerstarts[playernum]);
            return;
        }
        
        // try to spawn at one of the other players spots
        for(i = 0; i < MAXPLAYERS; i++)
        {
            if(G_CheckSpot(playernum, &playerstarts[i]) ) 
            { 
                playerstarts[i].type = playernum+1;	// fake as other player 
                P_SpawnPlayer(&playerstarts[i]); 
                playerstarts[i].type = i+1;			// restore 
                return;
            }	    
            // he's going to be inside something.  Too bad.
        }
        P_SpawnPlayer (&playerstarts[playernum]);
    }
}

//
// G_ScreenShot
//

void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}

//
// G_CompleteLevel
//

void G_CompleteLevel(void)
{
    mapdef_t* map;
    clusterdef_t* cluster;

    map = P_GetMapInfo(gamemap);

    if(!map->nointermission)
        gameaction = ga_completed;
    else
        gameaction = ga_victory;

    cluster = P_GetCluster(nextmap);

    if(cluster && cluster->nointermission)
        gameaction = ga_victory;
}

//
// G_ExitLevel
//

void G_ExitLevel(void)
{
    line_t junk;
    mapdef_t* map;
    
    map = P_GetMapInfo(gamemap);

    dmemset(&junk, 0, sizeof(line_t));
    junk.tag = map->exitdelay;
    
    P_SpawnDelayTimer(&junk, G_CompleteLevel);
    
    nextmap = gamemap + 1;
}

//
// G_SecretExitLevel
//

void G_SecretExitLevel(int map)
{
    line_t junk;
    mapdef_t* mapdef;
    
    mapdef = P_GetMapInfo(gamemap);

    dmemset(&junk, 0, sizeof(line_t));
    junk.tag = mapdef->exitdelay;
    
    P_SpawnDelayTimer(&junk, G_CompleteLevel);
    
    nextmap = map;
}

//
// G_RunTitleMap
//

void G_RunTitleMap(void)
{
    demobuffer = Z_Calloc(0x16000, PU_STATIC, NULL);
    demo_p = demobuffer;
    demobuffer[0x16000-1] = DEMOMARKER;

    G_InitNew(sk_medium, 33);

    precache = true;
    usergame = false;
    demoplayback = true;

    G_DoLoadLevel();
    D_MiniLoop(P_Start, P_Stop, P_Drawer, P_Ticker);
}

//
// G_RunGame
// The game should already have been initialized or loaded
//

void G_RunGame(void)
{
    int next = 0;

    G_InitNew(startskill, startmap);

    while(gameaction != ga_title)
    {
        if(gameaction == ga_loadgame)
            G_DoLoadGame();
        else
            G_DoLoadLevel();

        next = D_MiniLoop(P_Start, P_Stop, P_Drawer, P_Ticker);

        if(next == ga_loadlevel)
            continue;   // restart level from scratch

        if(next == ga_warplevel || next == ga_warpquick)
            continue;   // skip intermission

        if(next == ga_title)
            return;     // exit game and return to title screen

        if(next == ga_completed)
            next = D_MiniLoop(WI_Start, WI_Stop, WI_Drawer, WI_Ticker);

        if(next == ga_victory)
        {
            next = D_MiniLoop(IN_Start, IN_Stop, IN_Drawer, IN_Ticker);

            if(next == ga_finale)
                D_MiniLoop(F_Start, F_Stop, F_Drawer, F_Ticker);
        }

        gamemap = nextmap;
    }
}

char savename[256];

//
// G_LoadGame
//

void G_LoadGame(const char* name)
{
    dstrcpy(savename, name);
    gameaction = ga_loadgame;
}

#define VERSIONSIZE 	16

//
// G_DoLoadGame
//

void G_DoLoadGame(void)
{
    if(!P_ReadSaveGame(savename))
    {
        gameaction = ga_nothing;
        players[consoleplayer].message = "couldn't load game!";
        return;
    }
}


//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//

void G_SaveGame(int slot, const char* description)
{
    savegameslot = slot;
    dstrcpy(savedescription, description);
    sendsave = true;
}

//
// G_DoSaveGame
//

void G_DoSaveGame (void)
{
    if(!P_WriteSaveGame(savedescription, savegameslot))
    {
        players[consoleplayer].message = "couldn't save game!";
        return;
    }

    savedescription[0] = 0;
    players[consoleplayer].message = GGSAVED;
}


//
// G_DeferedInitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//

void G_DeferedInitNew(skill_t skill, int map)
{
    startskill = skill;
    startmap = map;
    nextmap = map;
    gameaction = ga_newgame;
}

//
// G_Init
//

void G_Init(void)
{
    dmemset(playeringame, 0, sizeof(playeringame));
    G_ClearInput();
}

//
// G_SetFastParms
// killough 4/10/98: New function to fix bug which caused Doom
// lockups when idclev was used in conjunction with -fast.

void G_SetFastParms(int fast_pending)
{
    static int fast = 0;            // remembers fast state
    int i;
    if(fast != fast_pending) 
    {     /* only change if necessary */
        if((fast = fast_pending))
        {
            for(i = S_044; i <= S_058; i++)
            {
                if(states[i].tics != 1) // killough 4/10/98
                    states[i].tics >>= 1;  // don't change 1->0 since it causes cycles
            }
            mobjinfo[MT_PROJ_BRUISER1].speed = 20*FRACUNIT;
            mobjinfo[MT_PROJ_HEAD].speed = 30*FRACUNIT;
            mobjinfo[MT_PROJ_IMP2].speed = 35*FRACUNIT;
            mobjinfo[MT_PROJ_IMP1].speed = 20*FRACUNIT;
        }
        else
        {
            for(i = S_044; i <= S_058; i++)
                states[i].tics <<= 1;
            
            mobjinfo[MT_PROJ_BRUISER1].speed = 15*FRACUNIT;
            mobjinfo[MT_PROJ_HEAD].speed = 20*FRACUNIT;
            mobjinfo[MT_PROJ_IMP2].speed = 20*FRACUNIT;
            mobjinfo[MT_PROJ_IMP1].speed = 10*FRACUNIT;
        }
    }
}

//
// G_InitNew
//

void G_InitNew(skill_t skill, int map)
{
    int i;

    if(!netgame)
    {
        netdemo = false;
        netgame = false;
        deathmatch = false;
        playeringame[1] = playeringame[2] = playeringame[3] = 0;
        playeringame[0]=true;
        consoleplayer = 0;
    }
    
    if(paused)
        paused = false;

    if(skill > sk_nightmare)
        skill = sk_nightmare;
    
    // force players to be initialized upon first level load
    for(i = 0; i < MAXPLAYERS; i++)
        players[i].playerstate = PST_REBORN;
    
    usergame		= true;				// will be set false if a demo
    paused			= false;
    demoplayback	= false;
    automapactive	= false;
    gamemap			= map;
    gameskill		= skill;

    // [d64] For some reason this is added here
    M_ClearRandom();

    CON_CvarSetValue(sv_skill.name, (float)skill);
}


//
// DEMO RECORDING
//

//
// G_ReadDemoTiccmd
//

void G_ReadDemoTiccmd(ticcmd_t* cmd)
{
    if(*demo_p == DEMOMARKER)
    {
        // end of demo data stream
        G_CheckDemoStatus();
        return;
    }
    
    cmd->forwardmove    = ((signed char)*demo_p++);
    cmd->sidemove       = ((signed char)*demo_p++);
    cmd->angleturn      = ((unsigned char)*demo_p++)<<8;
    cmd->pitch          = ((unsigned char)*demo_p++)<<8;
    cmd->buttons        = (unsigned char)*demo_p++;
    cmd->buttons2       = (unsigned char)*demo_p++;
}


//
// G_WriteDemoTiccmd
//

void G_WriteDemoTiccmd(ticcmd_t* cmd)
{   
    *demo_p++ = cmd->forwardmove;
    *demo_p++ = cmd->sidemove;
    *demo_p++ = (cmd->angleturn+128)>>8;
    *demo_p++ = (cmd->pitch+128)>>8;
    *demo_p++ = cmd->buttons;
    *demo_p++ = cmd->buttons2;
    
    demo_p -= 6;
    
    if(demo_p > demoend - 24)
    {
        // no more space
        G_CheckDemoStatus();
        return;
    }
    
    G_ReadDemoTiccmd(cmd);	// make SURE it is exactly the same
}



//
// G_RecordDemo
//

void G_RecordDemo(const char* name)
{
    int i;
    int maxsize;

    dstrcpy(demoname, name);
    dstrcat(demoname, ".lmp");

    maxsize = 0x20000;
    i = M_CheckParm("-maxdemo");

    if(i && i<myargc-1)
        maxsize = datoi(myargv[i+1])*1024;
    
    demobuffer = Z_Malloc(maxsize, PU_STATIC, NULL);
    demoend = demobuffer + maxsize;
    demo_p = demobuffer;

    G_InitNew(startskill, startmap);
    
    *demo_p++ = 1;
    *demo_p++ = gameskill;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = respawnparm;
    *demo_p++ = respawnitem;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;
    
    for(i = 0; i < MAXPLAYERS; i++)
        *demo_p++ = playeringame[i];

    demorecording = true;
    usergame = false;

    G_DoLoadLevel();
    D_MiniLoop(P_Start, P_Stop, P_Drawer, P_Ticker);

    G_CheckDemoStatus();
}

//
// G_PlayDemo
//

void G_PlayDemo(const char* name)
{
    int i;
    int p;

    gameaction = ga_nothing;

    p = M_CheckParm ("-playdemo");
    if(p && p < myargc-1)
    {
        if(M_ReadFile(myargv[p+1], &demobuffer) == -1)
        {
            gameaction = ga_exitdemo;
            return;
        }

        demo_p = demobuffer;
    }
    else
    {
        if(W_CheckNumForName(name) == -1)
        {
            gameaction = ga_exitdemo;
            return;
        }

        demobuffer = demo_p = W_CacheLumpName(name, PU_STATIC);
    }

    demo_p++;
    
    startskill      = *demo_p++;
    startmap        = *demo_p++;
    deathmatch      = *demo_p++;
    respawnparm     = *demo_p++;
    respawnitem     = *demo_p++;
    fastparm        = *demo_p++;
    nomonsters      = *demo_p++;
    consoleplayer   = *demo_p++;
    
    for(i = 0; i < MAXPLAYERS; i++)
        playeringame[i] = *demo_p++;

    G_InitNew(startskill, startmap);

    if(playeringame[1])
    {
        netgame = true;
        netdemo = true;
    }

    precache = true;
    usergame = false;
    demoplayback = true;

    G_DoLoadLevel();
    D_MiniLoop(P_Start, P_Stop, P_Drawer, P_Ticker);
}

//
// G_CheckDemoStatus
// Called after a death or level completion to allow demos to be cleaned up
// Returns true if a new demo loop action will take place
//

dboolean G_CheckDemoStatus(void)
{
    int endtime;
    
    if(timingdemo)
    {
        endtime = I_GetTime();
        I_Error("G_CheckDemoStatus: timed %i gametics in %i realtics (%d FPS)",
            gametic, endtime-starttime, (gametic*TICRATE)/(endtime-starttime));
    }
    
    if(demoplayback)
    {
        if(singledemo)
            I_Quit();
        
        Z_Free(demobuffer);
        
        netdemo         = false;
        netgame         = false;
        deathmatch      = false;
        playeringame[1]	= playeringame[2] = playeringame[3] = 0;
        respawnparm     = false;
        respawnitem     = false;
        fastparm        = false;
        nomonsters      = false;
        consoleplayer   = 0;
        gameaction      = ga_exitdemo;
        
        return true;
    }
    
    if(demorecording)
    {
        *demo_p++ = DEMOMARKER;
        M_WriteFile(demoname, demobuffer, demo_p - demobuffer);
        Z_Free(demobuffer);
        demorecording = false;
        I_Error("Demo %s recorded", demoname);
    }
    
    return false;
}


