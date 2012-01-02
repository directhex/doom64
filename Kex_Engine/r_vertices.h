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

#ifndef _R_VERTICES_H_
#define _R_VERTICES_H_

#include "doomtype.h"
#include "doomdef.h"
#include "r_gl.h"
#include "r_things.h"

typedef enum
{
    DLF_GLOW        = 0x1,
    DLF_WATER1      = 0x2,
    DLF_CEILING     = 0x4,
    DLF_MIRRORS     = 0x8,
    DLF_MIRRORT     = 0x10,
    DLF_WATER2      = 0x20
} drawlistflag_e;

typedef enum
{
    DLT_WALL,
    DLT_FLAT,
    DLT_SPRITE,
    DLT_AMAP,
    NUMDRAWLISTS
} drawlisttag_e;

typedef struct
{
    void        *data;
    dboolean    (*drawfunc)(void*, vtx_t*);
    dtexture    texid;
    int         flags;
    int         glowvalue;
} vtxlist_t;

typedef struct
{
    vtxlist_t   *list;
    int         index;
    int         max;
} drawlist_t;

extern drawlist_t drawlist[NUMDRAWLISTS];

int qsort_CompareDL(const void *a, const void *b);

#define MAXDLDRAWCOUNT  0x10000
vtx_t drawVertex[MAXDLDRAWCOUNT];

void DL_PushVertex(drawlist_t *dl);
void DL_PushSprite(drawlist_t *dl, visspritelist_t *vis, int texid);
void DL_PushSeg(drawlist_t *dl, seg_t *line, int texid, int sidetype);
void DL_PushLeaf(drawlist_t *dl, subsector_t *sub, int texid);
void DL_RenderDrawList(void);
void DL_Init(drawlist_t *list);

#endif

