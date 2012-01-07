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

#ifndef __I_PNG_H__
#define __I_PNG_H__

#include "png.h"
#include "doomtype.h"

byte* I_PNGReadData(int lump, dboolean palette, dboolean nopack, dboolean alpha,
					int* w, int* h, int* offset, int palindex);

byte* I_PNGCreate(int width, int height, byte* data, int* size);

#endif // __I_PNG_H__