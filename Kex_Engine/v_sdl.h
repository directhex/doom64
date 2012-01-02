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
// DESCRIPTION: Global SDL stuff
//
//-----------------------------------------------------------------------------


#ifndef __V_SDL_H__
#define __V_SDL_H__

#include "SDL.h"

#define SDL_BPP		32

////////////Video///////////////

extern SDL_Surface *screen;

void V_Init(void);
void V_InitGL(void);
void V_NetWaitScreen(void);
void V_Shutdown(void);
void V_ShutdownWait(void);
void V_StartTic (void);
void V_FinishUpdate(void);

////////////Input//////////////

extern int	UseMouse[2];
extern int	UseJoystick;

void V_InitInputs(void);
int V_MouseAccel(int val);
void V_MouseAccelChange(void);

#endif