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

#ifndef D3DR_AM_H
#define D3DR_AM_H

void AM_BeginDraw(angle_t view, fixed_t x, fixed_t y);
void AM_EndDraw(void);
void AM_DrawLeafs(float scale);
void AM_DrawLine(int x1, int x2, int y1, int y2, float scale, byte r, byte g, byte b);
void AM_DrawTriangle(mobj_t* mobj, float scale, dboolean solid, byte r, byte g, byte b);
void AM_DrawSprite(mobj_t* thing, float scale);

#endif