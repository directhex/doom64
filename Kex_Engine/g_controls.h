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
//---------------------------------------------------------------------

#ifndef G_CONTROLS_H
#define G_CONTROLS_H

#include "doomdef.h"
#include "r_gl.h"

#define NUMKEYS		SDLK_LAST

#define PCKF_DOUBLEUSE	0x4000
#define PCKF_UP			0x8000
#define PCKF_COUNTMASK	0x00ff

typedef enum
{
	PCKEY_ATTACK,
	PCKEY_USE,
	PCKEY_STRAFE,
	PCKEY_FORWARD,
	PCKEY_BACK,
	PCKEY_LEFT,
	PCKEY_RIGHT,
	PCKEY_STRAFELEFT,
	PCKEY_STRAFERIGHT,
	PCKEY_RUN,
	PCKEY_JUMP,
	PCKEY_LOOKUP,
	PCKEY_LOOKDOWN,
	PCKEY_CENTER,
	NUM_PCKEYS
}pckeys_t;

typedef struct
{
	int			mousex;
	int			mousey;
	int			joyx;
	int			joyy;
	int			key[NUM_PCKEYS];
	int			nextweapon;
	int			sdclicktime;
	int			fdclicktime;
	int			flags;
}playercontrols_t;

#define PCF_NEXTWEAPON	0x01
#define PCF_FDCLICK		0x02
#define PCF_FDCLICK2	0x04
#define PCF_SDCLICK		0x08
#define PCF_SDCLICK2	0x10
#define PCF_PREVWEAPON	0x20
#define PCF_GAMEPAD     0x40

extern playercontrols_t	Controls;
extern char *ConfigFileName;

#endif
