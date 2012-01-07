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
// DESCRIPTION:
//   Setup a game, startup stuff.
//
//-----------------------------------------------------------------------------


#ifndef __P_SETUP__
#define __P_SETUP__


#ifdef __GNUG__
#pragma interface
#endif


// NOT called by W_Ticker. Fixme.
void P_SetupLevel(int map, int playermask, skill_t skill);

// Called by startup code.
void P_Init (void);

//
// [kex] mapinfo
//
mapdef_t* P_GetMapInfo(int map);
clusterdef_t* P_GetCluster(int map);

//
// [kex] sky definitions
//
typedef enum
{
    SKF_CLOUD       = 0x1,
    SKF_THUNDER     = 0x2,
    SKF_FIRE        = 0x4,
    SKF_BACKGROUND  = 0x8,
    SKF_FADEBACK    = 0x10,
    SKF_VOID        = 0x20
} skyflags_e;

typedef struct
{
    char        flat[9];
    int         flags;
    char        pic[9];
    char        backdrop[9];
    rcolor      fogcolor;
    rcolor      skycolor[3];
    int         fognear;
} skydef_t;

#endif
