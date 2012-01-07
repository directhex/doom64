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
//	The not so system specific sound interface.
//
//-----------------------------------------------------------------------------


#ifndef __S_SOUND__
#define __S_SOUND__


#ifdef __GNUG__
#pragma interface
#endif

#include "p_mobj.h"
#include "sounds.h"

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer
//
void S_Init(void);

void S_SetSoundVolume(float volume);
void S_SetMusicVolume(float volume);

void S_ResetSound(void);
void S_PauseSound(void);
void S_ResumeSound(void);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(mobj_t* origin, int sound_id);

void S_UpdateSounds(void);
void S_RemoveOrigin(mobj_t* origin);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(mobj_t* origin, int sound_id, int volume);

// Stop sound for thing at <origin>
void S_StopSound(mobj_t* origin, int sfx_id);

int S_GetActiveSounds(void);


// Start music using <music_id> from sounds.h
void S_StartMusic(int mnum);
void S_StopMusic(void);


#endif
