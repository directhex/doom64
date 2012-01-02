// Emacs style mode select	 -*- C++ -*-
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
// $Author$
// $Revision$
// $Date$
//
// DESCRIPTION: OpenGL exclusive functions. All OGL initializations are also handled here
//
//-----------------------------------------------------------------------------

#ifndef __R_GL_H__
#define __R_GL_H__

#include "SDL_opengl.h"

#include "r_glExt.h"

typedef GLuint		dtexture;
typedef GLfloat		rfloat;
typedef GLuint		rcolor;
typedef GLuint		rbuffer;
typedef GLhandleARB	rhandle;

extern rcolor TextureClearColor;

typedef struct
{
    rfloat	x;
    rfloat	y;
    rfloat	z;
    rfloat	tu;
    rfloat	tv;
    byte r;
    byte g;
    byte b;
    byte a;
} vtx_t;

#define MAXSPRPALSETS       4

typedef struct
{
    byte r;
    byte g;
    byte b;
    byte a;
} dPalette_t;

extern int ViewWidth;
extern int ViewHeight;
extern int ViewWindowX;
extern int ViewWindowY;

#define FIELDOFVIEW         2048            // Fineangles in the video_width wide window.
#define ALPHACLEARGLOBAL    0.01f
#define ALPHACLEARTEXTURE   0.8f

#define TESTALPHA(x)        ((byte)((x >> 24) & 0xff) < 0xff)

extern int glFilter;
extern int DGL_CLAMP;

extern dboolean usingGL;

void R_GLInitialize(void);
void R_GLClearFrame(rcolor clearcolor);
dboolean R_GLGet(int x);
void R_GLCheckFillMode(void);
void R_GLToggleBlend(dboolean enable);
void R_GLFinish(void);
byte* R_GLGetScreen(int width, int height);
void R_GLSetFilter(void);
void R_GLEnable2D(dboolean filladjust);
void R_GLDisable2D(void);
void R_GLSetOrthoScale(float scale);
float R_GLGetOrthoScale(void);
void R_GLResetCombiners(void);
void R_GLSetupVertex(vtx_t *v, float x, float y, int width, int height,
                     float u1, float u2, float v1, float v2, rcolor c);
void R_GLRenderVertex(vtx_t *v, dboolean filladjust);
void R_GLDraw2DStrip(float x, float y, int width, int height,
                     float u1, float u2, float v1, float v2, rcolor c, dboolean filladjust);

#endif