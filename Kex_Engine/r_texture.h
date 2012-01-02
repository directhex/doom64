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

#ifndef _R_TEXTURE_H_
#define _R_TEXTURE_H_

#include "r_gl.h"

extern int                  curtexture;
extern int                  cursprite;
extern int	                curtrans;
extern int                  curgfx;

extern word*                texturewidth;
extern word*                textureheight;
extern dtexture**           textureptr;
extern int                  t_start;
extern int                  t_end;
extern int                  swx_start;
extern int                  numtextures;
extern word*                texturetranslation;
extern word*                palettetranslation;

extern int                  MinTextureSize;
extern int                  MaxTextureSize;

extern int                  g_start;
extern int                  g_end;
extern int                  numgfx;
extern dtexture*            gfxptr;
extern word*                gfxwidth;
extern word*                gfxheight;

extern int                  s_start;
extern int                  s_end;
extern dtexture**           spriteptr;
extern int                  numsprtex;
extern word*                spritewidth;
extern float*               spriteoffset;
extern float*               spritetopoffset;
extern word*                spriteheight;

void        R_BindDummyTexture(void);
void        R_InitTextures(void);
void        R_UnloadTexture(dtexture* texture);
void        R_CheckTextureSizeLimits(void);
void        R_BindWorldTexture(int texnum, int *width, int *height);
void        R_BindSpriteTexture(int spritenum, int pal);
int         R_BindGfxTexture(const char* name, dboolean alpha);
int         R_PadTextureDims(int size);
void        R_SetNewPalette(int id, byte palID);
void        R_DumpTextures(void);
dtexture    R_ScreenToTexture(void);

#endif