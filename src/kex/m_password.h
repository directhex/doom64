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


#ifndef __M_PASSWORD_H__
#define __M_PASSWORD_H__

#include "doomtype.h"

extern byte passwordData[16];
extern const char *passwordChar;
extern dboolean doPassword;

void M_EncodePassword(void);
dboolean M_DecodePassword(dboolean checkOnly);

#endif  // __M_PASSWORD_H__