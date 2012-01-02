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

#ifndef CON_CONSOLE_H
#define CON_CONSOLE_H

#include "d_event.h"
#include "r_gl.h"
#include "con_cvar.h"

#define MAX_CONSOLE_INPUT_LEN	80
extern char     ConsoleInputBuff[];
extern int      ConsoleInputLen;
extern int      ConsolePos;
extern dboolean ConsoleInitialized;

#define CONCLEARINPUT() (dmemset(ConsoleInputBuff+1, 0, MAX_CONSOLE_INPUT_LEN-1))

void CON_Init(void);
void CON_AddText(char *text);
void CON_Printf(rcolor clr, const char *s, ...);
void CON_Warnf(const char *s, ...);
void CON_Draw(void);
void CON_AddLine(char *line, int len);
void CON_Ticker(void);

dboolean CON_Responder (event_t* ev);

#endif