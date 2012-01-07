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

#ifndef G_ACTIONS_H
#define G_ACTIONS_H

#define MAX_ACTIONPARAM		2

typedef void (*actionproc_t)(int data, char **param);

void        G_InitActions(void);
dboolean    G_ActionResponder(event_t *ev);
void        G_RegisterAction(char *name, actionproc_t proc, int data, dboolean allownet);
void        G_ActionTicker(void);
void        G_ExecuteCommand(char *action);
void        G_BindActionByName(char *key, char *action);
dboolean    G_BindActionByEvent(event_t *ev, char *action);
void        G_ShowBinding(char *key);
void        G_GetActionBindings(char *buff, char *action);
void        G_UnbindAction(char *action);
int         G_ListCommands(void);

void        G_CmdAlias(int data, char **param);
void        G_OutputBindings(FILE *fh);
void        G_CmdUnbind(int data, char **param);
void        G_CmdUnbindAll(int data, char **param);
void        G_DoCmdMouseMove(int x, int y);

extern dboolean	ButtonAction;

#endif