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
//
//
//-----------------------------------------------------------------------------


#ifndef __M_MISC__
#define __M_MISC__


#include "doomtype.h"
#include "m_fixed.h"
#include "r_local.h"

//
// MISC
//

#ifndef O_BINARY
#define O_BINARY 0
#endif

extern  int	myargc;
extern  char**	myargv;

// Returns the position of the given parameter
// in the arg list (0 if not found).
int M_CheckParm (char* check);

// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};	// bbox coordinates

// Bounding box functions.
void M_ClearBox (fixed_t*	box);

void
M_AddToBox
( fixed_t*	box,
 fixed_t	x,
 fixed_t	y );

dboolean M_WriteFile(char const* name, void* source, int length);
int M_ReadFile(char const* name, byte** buffer);
void M_NormalizeSlashes(char *str);
int M_FileExists(char *filename);
long M_FileLength(FILE *handle);
dboolean M_WriteTextFile(char const* name, char* source, int length);

void M_ScreenShot(void);
int M_CacheThumbNail(byte** data);

#define SM_FONT1		16
#define SM_FONT2		42
#define SM_MISCFONT		10
#define SM_NUMBERS		0
#define SM_SKULLS		70
#define SM_THERMO		68
#define SM_MICONS		78

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} symboldata_t;

extern const symboldata_t symboldata[];

int M_DrawText(int x, int y, rcolor color, float scale, dboolean wrap, const char* string, ...);
int M_CenterSmbText(const char* string);
int M_DrawSmbText(int x, int y, rcolor color, const char* string);
void M_DrawNumber(int x, int y, int num, int type, rcolor c);
void M_LoadDefaults(void);
void M_SaveDefaults(void);

//
// DEFAULTS
//
extern int		DualMouse;

extern int      viewwidth;
extern int      viewheight;

extern char*    chat_macros[];

//extern dboolean HighSound;

#endif
