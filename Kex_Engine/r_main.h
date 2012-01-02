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

#ifndef D3DR_MAIN_H
#define D3DR_MAIN_H

#include "t_bsp.h"
#include "d_player.h"
#include "w_wad.h"
#include "r_gl.h"
#include "con_cvar.h"

extern fixed_t      viewx;
extern fixed_t      viewy;
extern fixed_t      viewz;
extern float        fviewx;
extern float        fviewy;
extern float        fviewz;
extern angle_t      viewangle;
extern angle_t      viewpitch;
extern fixed_t      quakeviewx;
extern fixed_t      quakeviewy;
extern angle_t      viewangleoffset;
extern rcolor       flashcolor;

extern float        viewsin[2];
extern float        viewcos[2];
extern float        viewoffset;
extern int          skytexture;
extern player_t     *renderplayer;
extern int          logoAlpha;
extern fixed_t      scrollfrac;
extern int          vertCount;

extern unsigned int renderTic;
extern unsigned int spriteRenderTic;
extern unsigned int glBindCalls;
extern unsigned int drawListSize;

extern dboolean     bRenderSky;

void R_Init(void);
void R_RenderPlayerView(player_t *player);
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
angle_t R_PointToAngle(fixed_t x, fixed_t y);//note difference from sw version
angle_t R_PointToPitch(fixed_t z1, fixed_t z2, fixed_t dist);
void R_PrecacheLevel(void);
int R_PointOnSide(fixed_t x, fixed_t y, node_t *node);
fixed_t R_Interpolate(fixed_t ticframe, fixed_t updateframe, dboolean enable);
void R_SetupLevel(void);
void R_SetViewAngleOffset(angle_t angle);
void R_SetViewOffset(int offset);
void R_DrawGfx(int x, int y, const char* name, rcolor color, dboolean alpha);
void R_DrawHudSprite(int type, int rot, int frame, int x, int y, float scale, int pal, rcolor c);
void R_DrawWireframe(dboolean enable);	//villsa

#endif