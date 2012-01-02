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
//
//-----------------------------------------------------------------------------

#ifndef __I_SWAP_H__
#define __I_SWAP_H__

#include "SDL_endian.h"
#include "doomtype.h"

#define I_SwapLE16(x)   SDL_SwapLE16(x)
#define I_SwapLE32(x)   SDL_SwapLE32(x)
#define I_SwapBE16(x)   SDL_SwapBE16(x)
#define I_SwapBE32(x)   SDL_SwapBE32(x)

#define SHORT(x)        ((signed short)I_SwapLE16(x))
#define LONG(x)         ((signed long)I_SwapLE32(x))

// Defines for checking the endianness of the system.

#if SDL_BYTEORDER == SYS_LIL_ENDIAN
#define SYS_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SYS_BIG_ENDIAN
#define SYS_BIG_ENDIAN
#endif

#endif // __I_SWAP_H__