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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "r_gl.h"
#include "p_mobj.h"

#include "Ext/ChocolateDoom/net_client.h"

//
// STATUS BAR
//

#define ST_FONTWHSIZE	8
#define ST_FONTNUMSET	32	//# of fonts per row in font pic
#define ST_FONTSTART	'!'	// the first font characters
#define ST_FONTEND		'_'	// the last font characters

#define ST_FONTSIZE		(ST_FONTEND - ST_FONTSTART + 1) // Calculate # of glyphs in font.

// Called by main loop.
dboolean ST_Responder (event_t* ev);

// Called by main loop.
void ST_Ticker (void);
void ST_ClearMessage(void);

// Called when the console player is spawned on each level.
void ST_Start (void);

// Called by startup code.
void ST_Init (void);
void ST_AddChatMsg(char *msg, int player);
void ST_Notification(char *msg);
void ST_Drawer(void);
void ST_FlashingScreen(byte r, byte g, byte b, byte a);
char ST_DequeueChatChar(void);
void ST_DrawCrosshair(int x, int y, int slot, byte scalefactor, rcolor color);
void ST_UpdateFlash(void);
void ST_AddDamageMarker(mobj_t* target, mobj_t* source);
void ST_ClearDamageMarkers(void);

extern char player_names[MAXPLAYERS][MAXPLAYERNAME];
extern dboolean st_chatOn;
extern int st_crosshairs;


#endif
