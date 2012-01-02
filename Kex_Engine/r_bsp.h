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

#ifndef D3DR_BSP_H
#define D3DR_BSP_H

void R_RenderBSPNode(int bspnum);
void R_CountSubsectorVerts(void);

extern vtx_t	*SSectorVertices;

#endif