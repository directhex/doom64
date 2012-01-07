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
// DESCRIPTION:
//	Fixed point implementation.
//
//-----------------------------------------------------------------------------

#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif

#include "stdlib.h"

#include "doomtype.h"
#include "doomdef.h"
#include "i_system.h"

#ifdef __GNUG__
#pragma implementation "m_fixed.h"
#endif
#include "m_fixed.h"




// Fixme. __USE_C_FIXED__ or something.

fixed_t
FixedMul
( fixed_t	a,
  fixed_t	b )
{
#ifdef USE_ASM
    fixed_t	c;
    _asm
    {
	mov eax, [a]
	mov ecx, [b]
	imul ecx
	shr eax, 16
	shl edx, 16
	or eax, edx
	mov [c], eax
    }
    return(c);
#else
    return (fixed_t)(((int64) a * (int64) b) >> FRACBITS);
#endif
}



//
// FixedDiv, C version.
//

fixed_t
FixedDiv
( fixed_t	a,
  fixed_t	b )
{
    if ( (D_abs(a)>>14) >= D_abs(b))
	return (a^b)<0 ? MININT : MAXINT;
    return FixedDiv2 (a,b);

}



fixed_t
FixedDiv2
( fixed_t	a,
  fixed_t	b )
{
    return (fixed_t)((((int64)a)<<FRACBITS)/b);
}


