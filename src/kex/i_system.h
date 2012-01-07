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
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

#include <stdio.h>

#include "d_ticcmd.h"
#include "d_event.h"
#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

// Called by DoomMain.
void I_Init (void);

#ifdef _WIN32

#define USESYSCONSOLE

void I_SpawnSysConsole(void);
void I_ShowSysConsole(dboolean show);

#endif


// Called by D_DoomLoop,
// returns current time in tics.

extern fixed_t rendertic_frac;

extern int  (*I_GetTime)(void);
void        I_InitClockRate(void);
int         I_GetTimeMS(void);
void        I_Sleep(unsigned long usecs);
dboolean    I_StartDisplay(void);
void        I_EndDisplay(void);
fixed_t     I_GetTimeFrac(void);
void        I_GetTime_SaveMS(void);


void I_FinishUpdate(void);


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic (void);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
ticcmd_t* I_BaseTiccmd (void);


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void I_Quit (void);

void I_BeginRead(void);
void I_Error (char *error, ...);
void I_Printf(char *msg, ...);
char *I_DoomExeDir(void);

extern FILE	*DebugFile;
extern dboolean	DigiJoy;

#endif
