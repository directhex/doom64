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
//	Networking stuff.
//
//-----------------------------------------------------------------------------


#ifndef __D_NET__
#define __D_NET__

#include "d_player.h"

#include "Ext/ChocolateDoom/net_client.h"
#include "Ext/ChocolateDoom/net_io.h"
#include "Ext/ChocolateDoom/net_query.h"
#include "Ext/ChocolateDoom/net_server.h"
#include "Ext/ChocolateDoom/net_sdl.h"
#include "Ext/ChocolateDoom/net_loop.h"

#ifdef __GNUG__
#pragma interface
#endif

#define MAXNETNODES		8	// Max computers/players in a game.
#define BACKUPTICS		128	// Networking and tick handling related.


// Create any new ticcmds and broadcast to other players.
void NetUpdate (void);

// Broadcasts special packets to other players
//  to notify of game exit
void D_QuitNetGame (void);

extern dboolean ShowGun;
extern dboolean drone;
extern dboolean	net_cl_new_sync;

#endif

