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
// $Author$
// $Revision$
// $Date$
//
//
// DESCRIPTION:  Linedef Macro handling/logic
//
//-----------------------------------------------------------------------------

#ifndef __P_MACROS__
#define __P_MACROS__


#ifdef __GNUG__
#pragma interface
#endif

#define MAXQUEUELIST    16

extern int taglist[MAXQUEUELIST];
extern int taglistidx;

void P_QueueSpecial(mobj_t* mobj);

extern thinker_t    *macrothinker;
extern macrodef_t   *macro;
extern macrodata_t  *nextmacro;
extern mobj_t       *mobjmacro;
extern short        macrocounter;
extern short        macroid;

void P_InitMacroVars(void);
void P_ToggleMacros(int tag, dboolean toggleon);
void P_MacroDetachThinker(thinker_t *thinker);
void P_RunMacros(void);

int P_StartMacro(mobj_t *thing, line_t *line);
int P_SuspendMacro(int tag);
int P_InitMacroCounter(int counts);

#endif