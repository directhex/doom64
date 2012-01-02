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
// DESCRIPTION: In-game Sound behavior
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "sounds.h"
#include "s_sound.h"
#include "z_zone.h"
#include "m_fixed.h"
#include "m_random.h"
#include "w_wad.h"
#include "doomdef.h"
#include "p_local.h"
#include "doomstat.h"
#include "tables.h"
#include "r_local.h"
#include "m_misc.h"
#include "p_setup.h"
#include "i_audio.h"

// Adjustable by menu.
#define NORM_VOLUME     127
#define NORM_SEP        128

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1700<<FRACBITS)

// Distance to origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).

#define S_MAX_DIST (NORM_VOLUME * (S_CLIPPING_DIST >> FRACBITS))    // [d64]
#define S_CLOSE_DIST (200 << FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

// percent attenuation from front to back
#define S_IFRACVOL              30

static dboolean nosound = false;
static dboolean nomusic = false;
static int lastmusic = 0;

//
// Internals.
//
int S_AdjustSoundParams(fixed_t x, fixed_t y, int* vol, int* sep);

//
// S_Init
//
// Initializes sound stuff, including volume
// and the sequencer
//

void S_Init(void)
{
    if(M_CheckParm("-nosound"))
        nosound = true;

    if(M_CheckParm("-nomusic"))
        nomusic = true;

	if(nosound && nomusic)
		return;

    I_InitSequencer();

    S_SetMusicVolume(s_musvol.value);
    S_SetSoundVolume(s_sfxvol.value);
}

//
// S_SetSoundVolume
//

void S_SetSoundVolume(float volume)
{
    I_SetSoundVolume(volume);
}

//
// S_SetMusicVolume
//

void S_SetMusicVolume(float volume)
{
    I_SetMusicVolume(volume);
}

//
// S_StartMusic
//

void S_StartMusic(int mnum)
{
    if(nomusic)
        return;

    if(mnum <= -1)
        return;

    I_StartMusic(mnum);
    lastmusic = mnum;
}

//
// S_StopMusic
//

void S_StopMusic(void)
{
    I_StopSound(NULL, lastmusic);
    lastmusic = 0;
}

//
// S_ResetSound
//

void S_ResetSound(void)
{
    if(nosound && nomusic)
		return;

    I_ResetSound();
}

//
// S_PauseSound
//

void S_PauseSound(void)
{
    if(nosound && nomusic)
		return;

    I_PauseSound();
}

//
// S_ResumeSound
//

void S_ResumeSound(void)
{
    if(nosound && nomusic)
		return;

    I_ResumeSound();
}

//
// S_StopSound
//

void S_StopSound(mobj_t* origin, int sfx_id)
{
    I_StopSound((sndsrc_t*)origin, sfx_id);
}

//
// S_GetActiveSounds
//

int S_GetActiveSounds(void)
{
    return I_GetVoiceCount();
}

//
// S_RemoveOrigin
//

void S_RemoveOrigin(mobj_t* origin)
{
    int     channels;
    mobj_t* source;
    int     i;

    channels = I_GetMaxChannels();

    for(i = 0; i < channels; i++)
    {
        source = (mobj_t*)I_GetSoundSource(i);
        if(origin == source)
            I_RemoveSoundSource(i);
    }
}

//
// S_UpdateSounds
//

void S_UpdateSounds(void)
{
    int     i;
    int     audible;
    int     volume;
    int     sep;
    mobj_t* source;
    int     channels;

    channels = I_GetMaxChannels();

    for(i = 0; i < channels; i++)
    {
        source = (mobj_t*)I_GetSoundSource(i);

        if(source == NULL)
            continue;

        // initialize parameters
        volume = NORM_VOLUME;
        sep = NORM_SEP;

        // check non-local sounds for distance clipping
        // or modify their params
         if(source == players[consoleplayer].mo &&
             players[consoleplayer].cameratarget == players[consoleplayer].mo)
         {
             audible = 1;
             sep = NORM_SEP;
         }
         else
             audible = S_AdjustSoundParams(source->x, source->y, &volume, &sep);

         if(audible)
             I_UpdateChannel(i, volume, sep);
    }
}

//
// S_StartSound
//

void S_StartSound(mobj_t* origin, int sfx_id)
{
    int volume;
    int sep;
    int reverb;

    if(nosound)
        return;

    if(origin && origin != players[consoleplayer].cameratarget)
    {
        if(!S_AdjustSoundParams(origin->x, origin->y, &volume, &sep))
            return;
    }
    else
    {
        sep = NORM_SEP;
        volume = NORM_VOLUME;
    }

    reverb = 0;
    
    if(origin)
    {
        subsector_t* subsector;
        
        subsector = R_PointInSubsector(origin->x, origin->y);

        if(subsector->sector->flags & MS_REVERB)
            reverb = 16;
        else if(subsector->sector->flags & MS_REVERBHEAVY)
            reverb = 32;
    }

    // Assigns the handle to one of the channels in the mix/output buffer.
    I_StartSound(sfx_id, (sndsrc_t*)origin, volume, sep, reverb);
}

//
// S_AdjustSoundParams
//
// Changes volume, stereo-separation, and pitch variables
// from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//

int S_AdjustSoundParams(fixed_t x, fixed_t y, int* vol, int* sep)
{
    fixed_t     approx_dist;
    angle_t     angle;
    mobj_t*		listener;
    player_t*	player;
    
    player = &players[consoleplayer];
    
    listener = player->cameratarget;
    
    // calculate the distance to sound origin
    //  and clip it if necessary
    
    // From _GG1_ p.428. Appox. eucledian distance fast.
    approx_dist = P_AproxDistance(listener->x - x, listener->y - y);
    
    if(approx_dist > S_CLIPPING_DIST)
        return 0;

    if(listener->x != x || listener->y != y)
    {
        // angle of source to listener
        angle = R_PointToAngle2(listener->x, listener->y, x, y);
    
        if(angle <= listener->angle)
            angle += 0xffffffff;
        angle -= listener->angle;
    
        // stereo separation
        *sep = (NORM_VOLUME + 1) - (FixedMul(S_STEREO_SWING, dsin(angle)) >> FRACBITS);
    }
    else
        *sep = NORM_SEP;
    
    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
        *vol = NORM_VOLUME;
    else
    {
        // distance effect
        approx_dist >>= FRACBITS;
        *vol = (((-approx_dist << 7) + (approx_dist)) + S_MAX_DIST) / S_ATTENUATOR;
    }
    
    return (*vol > 0);
}






