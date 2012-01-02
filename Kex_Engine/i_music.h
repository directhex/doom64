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

#ifndef __I_MUSIC__
#define __I_MUSIC__

void I_InitMusic(int id);
void I_ShutdownMusic(void);

#ifdef USE_USF
void I_LoadUSF(void);
void I_RunMusic(byte* data);
int I_MusicPlaying(void);

typedef struct
{
	int u1;
	int u2;
	int id;
	int u3;
	int u4;
} musinfo_t;
#endif

typedef enum
{
	MUSTYPE_USF,
	MUSTYPE_EXTERNAL,
	MUSTYPE_NONE,

	NUMMUSTYPE
} mustype_e;

#endif
