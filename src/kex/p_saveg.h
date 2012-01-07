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
//	Savegame I/O, archiving, persistence.
//
//-----------------------------------------------------------------------------


#ifndef __P_SAVEG__
#define __P_SAVEG__


#ifdef __GNUG__
#pragma interface
#endif

#define SAVEGAMESIZE    0x60000
#define SAVESTRINGSIZE  16

dboolean P_WriteSaveGame(char* description, int slot);
dboolean P_ReadSaveGame(char* name);
dboolean P_QuickReadSaveHeader(char* name, char* date, int* thumbnail, int* skill, int* map);

// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayers(void);
void P_UnArchivePlayers(void);
void P_ArchiveWorld(void);
void P_UnArchiveWorld(void);
void P_ArchiveMobjs(void);
void P_UnArchiveMobjs(void);
void P_ArchiveSpecials(void);
void P_UnArchiveSpecials(void);
void P_ArchiveMacros(void);
void P_UnArchiveMacros(void);

#endif
