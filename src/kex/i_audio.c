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
// DESCRIPTION: Low-level audio API. Incorporates a sequencer system to
//              handle all sounds and music. All code related to the sequencer
//              is kept in it's own module and seperated from the rest of the
//              game code.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif


#include <stdlib.h>
#include <stdio.h>

#include "SDL.h"
#include "fluidsynth.h"

#include "doomtype.h"
#include "doomdef.h"
#include "i_system.h"
#include "i_audio.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_swap.h"

//
// I don't fully understand the use for these
// so I may be introducing some issues here...
// 
static SDL_mutex *mutex = NULL;

//
// DEFINES
//

#define MIDI_CHANNELS   64
#define MIDI_MESSAGE    0x07
#define MIDI_END        0x2f
#define MIDI_SET_TEMPO  0x51
#define MIDI_SEQUENCER  0x7f

//
// MIDI DATA DEFINITIONS
//
// These data should not be modified outside the
// audio thread unless they're being initialized
//

typedef struct
{
    char        header[4];
    int         length;
    byte*       data;
    byte        channel;
} track_t;

typedef struct
{
    char        header[4];
    int         chunksize;
    short       type;
    word        ntracks;
    word        delta;
    byte*       data;
    dword       length;
    track_t*    tracks;
    dword       tempo;
    double      timediv;
} song_t;

//
// SEQUENCER CHANNEL
//
// Active channels play sound or whatever
// is being fed from the midi reader. This
// is where communication between the audio
// thread and the game could get dangerous
// as they both need to access and modify
// data. In order to avoid this, certain
// properties have been divided up for
// both the audio thread and game code only
//

typedef enum
{
    CHAN_STATE_READY    = 0,
    CHAN_STATE_PAUSED   = 1,
    CHAN_STATE_ENDED    = 2,
    MAXSTATETYPES
} chanstate_e;

typedef struct
{
    // these should never be modified unless
    // they're initialized
    song_t*     song;
    track_t*    track;

    // channel id for identifying an active channel
    // used primarily by normal sounds
    byte        id;

    // these are both accessed by the
    // audio thread and the game code
    // and should lock and unlock
    // the mutex whenever these need
    // to be modified..
    float       volume;
    byte        pan;
    sndsrc_t*   origin;
    int         depth;

    // accessed by the audio thread only
    byte        key;
    byte        velocity;
    byte*       pos;
    byte*       jump;
    dword       tics;
    dword       nexttic;
    dword       lasttic;
    chanstate_e state;
    dboolean    paused;

    // read by audio thread but only
    // modified by game code
    dboolean    stop;
    float       basevol;
} channel_t;

static channel_t playlist[MIDI_CHANNELS];   // channels active in sequencer

//
// DOOM SEQUENCER
//
// the backbone of the sequencer system. handles
// global volume and panning for all sounds/tracks
// and holds the allocated list of midi songs.
// all data is modified through the game and read
// by the audio thread.
//

typedef enum
{
    SEQ_SIGNAL_IDLE     = 0,    // idles. does nothing
    SEQ_SIGNAL_SHUTDOWN,        // signal the sequencer to shutdown, cleaning up anything in the process
    SEQ_SIGNAL_READY,           // sequencer will read and play any midi track fed to it
    SEQ_SIGNAL_RESET,
    SEQ_SIGNAL_PAUSE,
    SEQ_SIGNAL_RESUME,
    SEQ_SIGNAL_STOPALL,
    MAXSIGNALTYPES
} seqsignal_e;

typedef union
{
    sndsrc_t*   valsrc;
    int         valint;
    float       valfloat;
} seqmessage_t;

typedef struct
{
    // library specific stuff. should never
    // be modified after initialization
    fluid_settings_t*       settings;
    fluid_synth_t*          synth;
    fluid_audio_driver_t*   driver;
    dword                   sfont_id;
    SDL_Thread*             thread;

    dword                   voices;

    // tweakable settings for the sequencer
    float                   musicvolume;
    float                   soundvolume;

    // keep track of midi songs
    song_t*                 songs;
    int                     nsongs;

    seqmessage_t            message[3];

    // game code signals the sequencer to do stuff. game will
    // wait (while loop) until the audio thread signals itself
    // to be ready again
    seqsignal_e             signal;
} doomseq_t;

static doomseq_t doomseq;   // doom sequencer

typedef void(*eventhandler)(doomseq_t*, channel_t*);
typedef int(*signalhandler)(doomseq_t*);

//
// I_LockMutex
//
// Theoretically, locking the mutex should prevent
// the audio thread and game code from accessing
// things at once but......
//

static void I_LockMutex(void)
{
    SDL_LockMutex(mutex);
}

//
// I_UnlockMutex
//

static void I_UnlockMutex(void)
{
    SDL_UnlockMutex(mutex);
}

//
// I_SetSynthGain
//
// Set the 'master' volume for the sequencer. Affects
// all sounds that are played
//

static void I_SetSynthGain(doomseq_t* seq, float gain)
{
    fluid_synth_set_gain(seq->synth, gain);
}

//
// I_SetReverb
//

static void I_SetReverb(doomseq_t* seq,
                        float size,
                        float damp,
                        float width,
                        float level)
{
    fluid_synth_set_reverb(seq->synth, size, damp, width, level);
    fluid_synth_set_reverb_on(seq->synth, 1);
}

//
// I_ConfigSequencer
//

static void I_ConfigSequencer(doomseq_t* seq, char* setting, int value)
{
    fluid_settings_setint(seq->settings, setting, value);
}

//
// I_GetTimeDivision
//

static double I_GetTimeDivision(song_t* song)
{
    return (double)song->tempo / (double)song->delta / 1000.0;
}

//
// I_SetSeqStatus
//

static void I_SetSeqStatus(doomseq_t* seq, int status)
{
    seq->signal = status;
}

//
// I_WaitOnSignal
//

static void I_WaitOnSignal(doomseq_t* seq)
{
    while(1)
    {
        if(seq->signal == SEQ_SIGNAL_READY)
            break;
    }
}

//
// I_SetChannelMusicVolume
//
// Should be set by the audio thread
//

static void I_SetChannelMusicVolume(doomseq_t* seq, channel_t* chan)
{
    int vol;

    vol = (int)((chan->volume * seq->musicvolume) / 127.0f);

    fluid_synth_cc(seq->synth, chan->track->channel, 0x07, vol);
}

//
// I_SetChannelSoundVolume
//
// Should be set by the audio thread
//

static void I_SetChannelSoundVolume(doomseq_t* seq, channel_t* chan)
{
    int vol;
    int pan;

    vol = (int)((chan->volume * seq->soundvolume) / 127.0f);
    pan = chan->pan;

    fluid_synth_cc(seq->synth, chan->id, 0x07, vol);
    fluid_synth_cc(seq->synth, chan->id, 0x0A, pan);
}

//
// I_ReadTrackByte
//
// Gets the next byte in a midi track
//

static byte I_ReadTrackByte(channel_t* chan)
{
    if((dword)(chan->pos - chan->song->data) >= chan->song->length)
        I_Error("I_ReadTrackByte: Unexpected end of track");

    return *chan->pos++;
}

//
// I_CheckEndOfTrack
//
// Checks if the midi reader has reached the end
//

static dboolean I_CheckEndOfTrack(channel_t* chan)
{
    return ((dword)(chan->pos - chan->song->data) >= chan->song->length);
}

//
// I_GetNextTrackTick
//
// Read the midi track to get the next delta time
//

static dword I_GetNextTrackTick(channel_t* chan)
{
    dword tic;
    int i;

    tic = I_ReadTrackByte(chan);
    if(tic & 0x80)
    {
        byte mb;

        tic = tic & 0x7f;

        //
        // the N64 version loops infinitely but since the
        // delta time can only be four bytes long, just loop
        // for the remaining three bytes..
        //
        for(i = 0; i < 3; i++)
        {
            mb = I_ReadTrackByte(chan);
            tic = (mb & 0x7f) + (tic << 7);

            if(!(mb & 0x80))
                break;
        }
    }

    return (chan->tics + (dword)((double)tic * chan->song->timediv));
}

//
// I_StopTrack
//
// Stops a specific channel and any played sounds
//

static void I_StopTrack(doomseq_t* seq, channel_t* chan)
{
    int c;

    if(chan->song->type >= 1)
        c = chan->track->channel;
    else
        c = chan->id;

    fluid_synth_cc(seq->synth, c, 0x78, 0);
}

//
// I_ClearPlaylist
//

static void I_ClearPlaylist(void)
{
    int i;

    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        dmemset(&playlist[i], 0, sizeof(song_t));

        playlist[i].id      = i;
        playlist[i].state   = CHAN_STATE_READY;
    }
}

//
// I_RemoveTrackFromPlaylist
//

static dboolean I_RemoveTrackFromPlaylist(doomseq_t* seq, channel_t* chan)
{
    if(!chan->song || !chan->track)
        return false;

    I_StopTrack(seq, chan);

    chan->song      = NULL;
    chan->track     = NULL;
    chan->jump      = NULL;
    chan->tics      = 0;
    chan->nexttic   = 0;
    chan->lasttic   = 0;
    chan->pos       = 0;
    chan->key       = 0;
    chan->velocity  = 0;
    chan->depth     = 0;
    chan->state     = CHAN_STATE_ENDED;
    chan->paused    = false;
    chan->stop      = false;
    chan->volume    = 0.0f;
    chan->basevol   = 0.0f;
    chan->pan       = 0;
    chan->origin    = NULL;

    seq->voices--;

    return true;
}

//
// I_AddTrackToPlayList
//
// Add a song to the playlist for the sequencer to play.
// Sets any default values to the channel in the process
//

static channel_t* I_AddTrackToPlayList(doomseq_t* seq, song_t* song, track_t* track)
{
    int i;

    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        if(playlist[i].song == NULL)
        {
            playlist[i].song        = song;
            playlist[i].track       = track;
            playlist[i].tics        = 0;
            playlist[i].lasttic     = 0;
            playlist[i].pos         = track->data;
            playlist[i].jump        = NULL;
            playlist[i].state       = CHAN_STATE_READY;
            playlist[i].paused      = false;
            playlist[i].stop        = false;
            playlist[i].key         = 0;
            playlist[i].velocity    = 0;

            // channels 0 through 15 are reserved for music only
            // channel ids should only be accessed by non-music sounds
            playlist[i].id          = 0x0f + i;

            playlist[i].volume      = 127.0f;
            playlist[i].basevol     = 127.0f;
            playlist[i].pan         = 64;
            playlist[i].origin      = NULL;
            playlist[i].depth       = 0;

            // immediately start reading the midi track
            playlist[i].nexttic     = I_GetNextTrackTick(&playlist[i]);

            seq->voices++;

            return &playlist[i];
        }
    }

    return NULL;
}

//
// I_EventNoteOff
//

static void I_EventNoteOff(doomseq_t* seq, channel_t* chan)
{
    chan->key       = I_ReadTrackByte(chan);
    chan->velocity  = 0;

    fluid_synth_noteoff(seq->synth, chan->track->channel, chan->key);
}

//
// I_EventNoteOn
//

static void I_EventNoteOn(doomseq_t* seq, channel_t* chan)
{
    chan->key       = I_ReadTrackByte(chan);
    chan->velocity  = I_ReadTrackByte(chan);

    fluid_synth_cc(seq->synth, chan->id, 0x5B, chan->depth);
    fluid_synth_noteon(seq->synth, chan->track->channel, chan->key, chan->velocity);
}

//
// I_EventControlChange
//

static void I_EventControlChange(doomseq_t* seq, channel_t* chan)
{
    int ctrl;
    int val;

    ctrl = I_ReadTrackByte(chan);
    val = I_ReadTrackByte(chan);

    if(ctrl == 0x07)    // update volume
    {
        if(chan->song->type == 1)
        {
            chan->volume = ((float)val * seq->musicvolume) / 127.0f;
            I_SetChannelMusicVolume(seq, chan);
        }
        else
        {
            chan->volume = ((float)val * chan->volume) / 127.0f;
            I_SetChannelSoundVolume(seq, chan);
        }
    }
    else
        fluid_synth_cc(seq->synth, chan->track->channel, ctrl, val);
}

//
// I_EventProgramChange
//

static void I_EventProgramChange(doomseq_t* seq, channel_t* chan)
{
    int program;

    program = I_ReadTrackByte(chan);

    fluid_synth_program_change(seq->synth, chan->track->channel, program);
}

//
// I_EventChannelPressure
//

static void I_EventChannelPressure(doomseq_t* seq, channel_t* chan)
{
    int val;

    val = I_ReadTrackByte(chan);

    fluid_synth_channel_pressure(seq->synth, chan->track->channel, val);
}

//
// I_EventPitchBend
//

static void I_EventPitchBend(doomseq_t* seq, channel_t* chan)
{
    int b1;
    int b2;

    b1 = I_ReadTrackByte(chan);
    b2 = I_ReadTrackByte(chan);

    fluid_synth_pitch_bend(seq->synth, chan->track->channel, ((b2 << 8) | b1) >> 1);
}

//
// I_EventMeta
//

static void I_EventMeta(doomseq_t* seq, channel_t* chan)
{
    int meta;
    int b;
    int i;
    char string[256];

    meta = I_ReadTrackByte(chan);

    switch(meta)
    {
        // mostly for debugging/logging
    case MIDI_MESSAGE:
        b = I_ReadTrackByte(chan);
        dmemset(string, 0, 256);

        for(i = 0; i < b; i++)
            string[i] = I_ReadTrackByte(chan);

        string[b + 1] = '\n';
        break;
        
    case MIDI_END:
        b = I_ReadTrackByte(chan);
        I_RemoveTrackFromPlaylist(seq, chan);
        break;

    case MIDI_SET_TEMPO:
        b = I_ReadTrackByte(chan);   // length

        if(b != 3)
            return;

        chan->song->tempo =
            (I_ReadTrackByte(chan) << 16) |
            (I_ReadTrackByte(chan) << 8)  |
            (I_ReadTrackByte(chan) & 0xff);

        chan->song->timediv = I_GetTimeDivision(chan->song);
        break;

        // game-specific midi event
    case MIDI_SEQUENCER:
        b = I_ReadTrackByte(chan);   // length
        b = I_ReadTrackByte(chan);   // manufacturer (should be 0)
        if(!b)
        {
            b = I_ReadTrackByte(chan);
            if(b == 0x23)
            {
                // set jump position
                chan->jump = chan->pos;
            }
            else if(b == 0x20)
            {
                b = I_ReadTrackByte(chan);
                b = I_ReadTrackByte(chan);

                // goto jump position
                if(chan->jump)
                    chan->pos = chan->jump;
            }
        }
        break;

    default:
        break;
    }
}

static const eventhandler seqeventlist[7] =
{
    I_EventNoteOff,
    I_EventNoteOn,
    NULL,
    I_EventControlChange,
    I_EventProgramChange,
    I_EventChannelPressure,
    I_EventPitchBend
};

//
// I_SignalIdle
//

static int I_SignalIdle(doomseq_t* seq)
{
    return 0;
}

//
// I_SignalShutdown
//

static int I_SignalShutdown(doomseq_t* seq)
{
    return -1;
}

//
// I_SignalStopAll
//

static int I_SignalStopAll(doomseq_t* seq)
{
    channel_t* c;
    int i;

    I_LockMutex();

    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        c = &playlist[i];

        if(c->song)
            I_RemoveTrackFromPlaylist(seq, c);
    }

    I_UnlockMutex();

    I_SetSeqStatus(seq, SEQ_SIGNAL_READY);
    return 1;
}

//
// I_SignalReset
//

static int I_SignalReset(doomseq_t* seq)
{
    fluid_synth_system_reset(seq->synth);

    I_SetSeqStatus(seq, SEQ_SIGNAL_READY);
    return 1;
}

//
// I_SignalPause
//
// Pause all currently playing songs
//

static int I_SignalPause(doomseq_t* seq)
{
    int i;
    channel_t* c;

    I_LockMutex();

    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        c = &playlist[i];

        if(c->song && !c->paused)
        {
            c->paused = true;
            I_StopTrack(seq, c);
        }
    }

    I_UnlockMutex();

    I_SetSeqStatus(seq, SEQ_SIGNAL_READY);
    return 1;
}

//
// I_SignalResume
//
// Resume all songs that were paused
//

static int I_SignalResume(doomseq_t* seq)
{
    int i;
    channel_t* c;

    I_LockMutex();

    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        c = &playlist[i];

        if(c->song && c->paused)
        {
            c->paused = false;
            fluid_synth_noteon(seq->synth, c->track->channel, c->key, c->velocity);
        }
    }

    I_UnlockMutex();

    I_SetSeqStatus(seq, SEQ_SIGNAL_READY);
    return 1;
}

static const signalhandler seqsignallist[MAXSIGNALTYPES] =
{
    I_SignalIdle,
    I_SignalShutdown,
    NULL,
    I_SignalReset,
    I_SignalPause,
    I_SignalResume,
    I_SignalStopAll,
};

//
// I_CheckState
//

static dboolean I_CheckState(doomseq_t* seq, channel_t* chan)
{
    if(chan->state == CHAN_STATE_ENDED)
        return true;
    else if(chan->state == CHAN_STATE_READY && chan->paused)
    {
        chan->state = CHAN_STATE_PAUSED;
        chan->lasttic = chan->nexttic - chan->tics;
        return true;
    }
    else if(chan->state == CHAN_STATE_PAUSED)
    {
        if(!chan->paused)
        {
            chan->nexttic = chan->tics + chan->lasttic;
            chan->state = CHAN_STATE_READY;
        }
        else
            return true;
    }

    return false;
}

//
// I_RunSong
//
// Main midi parsing routine
//

static void I_RunSong(doomseq_t* seq, channel_t* chan, dword msecs)
{
    byte event;
    byte c;
    song_t* song;
    byte channel;
    track_t* track;

    song = chan->song;
    track = chan->track;

    //
    // get next tic
    //
    chan->tics += (msecs - chan->tics);

    if(I_CheckState(seq, chan))
        return;
    
    //
    // keep parsing through midi track until
    // the end is reached or until it reaches next
    // delta time
    //
    while(chan->state != CHAN_STATE_ENDED)
    {
        if(chan->song->type == 0)
        {
            chan->volume = chan->basevol;
            I_SetChannelSoundVolume(seq, chan);
        }
        else
            I_SetChannelMusicVolume(seq, chan);

        //
        // not ready to execute events yet
        //
        if(chan->tics < chan->nexttic)
            return;

        c = I_ReadTrackByte(chan);

        if(c == 0xff)
            I_EventMeta(seq, chan);
        else
        {
            eventhandler eventhandle;

            event = (c >> 4) - 0x08;
            channel = c & 0x0f;

            if(event >= 0 && event < 7)
            {
                //
                // for music, use the generic midi channel
                // but for sounds, use the assigned id
                //
                if(song->type >= 1)
                    track->channel = channel;
                else
                    track->channel = chan->id;
            
                eventhandle = seqeventlist[event];

                if(eventhandle != NULL)
                    eventhandle(seq, chan);
            }
        }

        //
        // check for end of the track, otherwise get
        // the next delta time
        //
        if(chan->state != CHAN_STATE_ENDED)
        {
            if(I_CheckEndOfTrack(chan))
                chan->state = CHAN_STATE_ENDED;
            else
                chan->nexttic = I_GetNextTrackTick(chan);
        }
    }
}

//
// I_RunSequencer
//

static void I_RunSequencer(doomseq_t* seq, dword msecs)
{
    int i;
    channel_t* chan;

    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        chan = &playlist[i];

        if(!chan->song)
            continue;

        if(chan->stop)
            I_RemoveTrackFromPlaylist(seq, chan);

        I_RunSong(seq, chan, msecs);
    }
}

//
// I_RegisterTracks
//
// Allocate data for all tracks for a midi song
//

static dboolean I_RegisterTracks(song_t* song)
{
    int i;
    byte* data;

    song->tracks = (track_t*)Z_Calloc(sizeof(track_t) * song->ntracks, PU_STATIC, 0);
    data = song->data + 0x0e;

    for(i = 0; i < song->ntracks; i++)
    {
        track_t* track = &song->tracks[i];

        dmemcpy(track, data, 8);
        if(dstrncmp(track->header, "MTrk", 4))
            return false;

        data = data + 8;

        track->length   = I_SwapBE32(track->length);
        track->data     = data;

        data = data + track->length;
    }

    return true;
}

//
// I_RegisterSongs
//
// Allocate data for all midi songs
//

static dboolean I_RegisterSongs(doomseq_t* seq)
{
    int i;
    int start;
    int end;

    seq->nsongs = 0;
    i = 0;

    start = W_GetNumForName("DS_START") + 1;
    end = W_GetNumForName("DS_END") - 1;

    seq->nsongs = (end - start) + 1;

    //
    // no midi songs found in iwad?
    //
    if(seq->nsongs <= 0)
        return false;

    seq->songs = (song_t*)Z_Calloc(seq->nsongs * sizeof(song_t), PU_STATIC, 0);

    for(i = 0; i < seq->nsongs; i++)
    {
        song_t* song;

        song = &seq->songs[i];
        song->data = W_CacheLumpNum(start + i, PU_STATIC);
        song->length = W_LumpLength(start + i);

        if(!song->length)
            continue;

        dmemcpy(song, song->data, 0x0e);
        if(dstrncmp(song->header, "MThd", 4))
            return false;

        song->chunksize = I_SwapBE32(song->chunksize);
        song->ntracks   = I_SwapBE16(song->ntracks);
        song->delta     = I_SwapBE16(song->delta);
        song->type      = I_SwapBE16(song->type);
        song->timediv   = I_GetTimeDivision(song);
        song->tempo     = 480000;

        if(!I_RegisterTracks(song))
            return false; // bad midi lump?
    }

    return true;
}

//
// I_ShutdownSequencer
//

static void I_ShutdownSequencer(doomseq_t* seq)
{
    //
    // signal the sequencer to shut down
    //
    I_SetSeqStatus(seq, SEQ_SIGNAL_SHUTDOWN);

    //
    // wait until the audio thread is finished
    //
    SDL_WaitThread(seq->thread, NULL);

    //
    // fluidsynth cleanup stuff
    //
    delete_fluid_audio_driver(seq->driver);
    delete_fluid_synth(seq->synth);
    delete_fluid_settings(seq->settings);

    seq->synth = NULL;
    seq->driver = NULL;
    seq->settings = NULL;
}

//
// I_PlayerHandler
//
// Main routine of the audio thread
//

static int SDLCALL I_PlayerHandler(void *param)
{
    doomseq_t* seq = (doomseq_t*)param;
    long start = SDL_GetTicks();
    long delay = 0;
    int status;
    dword count = 0;
    signalhandler signal;

    while(1)
    {
        //
        // check status of the sequencer
        //
        signal = seqsignallist[seq->signal];

        if(signal)
        {
            status = signal(seq);

            if(status == 0)
                continue;

            if(status == -1)
                return 1;
        }

        //
        // play some songs
        //
        I_RunSequencer(seq, SDL_GetTicks() - start);
        count++;

        // try to avoid incremental time de-syncs
        delay = count - (SDL_GetTicks() - start);

        if(delay > 0)
            I_Sleep(delay);
    }

    return 0;
}

//
// I_InitSequencer
//

void I_InitSequencer(void)
{
    //
    // init mutex
    //

    mutex = SDL_CreateMutex();

    if(mutex == NULL)
    {
        I_Printf("I_InitSequencer: failed to create mutex");
        return;
    }

    dmemset(&doomseq, 0, sizeof(doomseq_t));

    //
    // init sequencer thread
    //

    doomseq.thread = SDL_CreateThread(I_PlayerHandler, &doomseq);

    if(doomseq.thread == NULL)
    {
        I_Printf("I_InitSequencer: failed to create audio thread");
        return;
    }

    //
    // init settings and synth
    //

    doomseq.settings    = new_fluid_settings();

    I_ConfigSequencer(&doomseq, "synth.midi-channels", 0x10 + MIDI_CHANNELS);
    I_ConfigSequencer(&doomseq, "synth.polyphony", 256);

    doomseq.synth = new_fluid_synth(doomseq.settings);

    if(doomseq.synth == NULL)
    {
        I_Printf("I_InitSequencer: failed to create synthesizer");
        return;
    }

    //
    // init audio driver
    //

    doomseq.driver = new_fluid_audio_driver(
        doomseq.settings, doomseq.synth);

    if(doomseq.driver == NULL)
    {
        I_Printf("I_InitSequencer: failed to create audio driver");
        return;
    }

    //
    // load soundfont
    //

    doomseq.sfont_id = fluid_synth_sfload(
        doomseq.synth, "DOOMSND.SF2", 1);

    //
    // set state
    //

    I_SetSeqStatus(&doomseq, SEQ_SIGNAL_READY);
    I_SetSynthGain(&doomseq, 1.0f);
    I_SetReverb(&doomseq, 0.65f, 0.0f, 2.0f, 1.0f);

    //
    // if something went terribly wrong, then shutdown everything
    //
    if(!I_RegisterSongs(&doomseq) || doomseq.sfont_id == -1)
    {
        I_ShutdownSequencer(&doomseq);
        return;
    }

    I_ClearPlaylist();
}

//
// I_GetMaxChannels
//

int I_GetMaxChannels(void)
{
    return MIDI_CHANNELS;
}

//
// I_GetVoiceCount
//

int I_GetVoiceCount(void)
{
    return doomseq.voices;
}

//
// I_GetSoundSource
//

sndsrc_t* I_GetSoundSource(int c)
{
    if(playlist[c].song == NULL)
        return NULL;

    return playlist[c].origin;
}

//
// I_RemoveSoundSource
//

void I_RemoveSoundSource(int c)
{
    playlist[c].origin = NULL;
}

//
// I_UpdateChannel
//

void I_UpdateChannel(int c, int volume, int pan)
{
    channel_t* chan;

    chan            = &playlist[c];
    chan->basevol   = (float)volume;
    chan->pan       = (byte)(pan >> 1);
}

//
// I_ShutdownSound
//

void I_ShutdownSound(void)
{
    if(doomseq.synth)
        I_ShutdownSequencer(&doomseq);
}

//
// I_SetMusicVolume
//

void I_SetMusicVolume(float volume)
{
    float vol = volume;
    float v = vol;

    vol *= 9; vol -= v;
    vol *= 8; vol -= v;
    vol *= 2;

    doomseq.musicvolume = ((vol / 100.0f));
}

//
// I_SetSoundVolume
//

void I_SetSoundVolume(float volume)
{
    float vol = volume;
    float v = vol;

    vol *= 7; vol += v;
    vol *= 7; vol += v;
    vol *= 2;

    doomseq.soundvolume = ((vol / 100.0f));
}

//
// I_ResetSound
//

void I_ResetSound(void)
{
    I_SetSeqStatus(&doomseq, SEQ_SIGNAL_RESET);
    I_WaitOnSignal(&doomseq);
}

//
// I_PauseSound
//

void I_PauseSound(void)
{
    I_SetSeqStatus(&doomseq, SEQ_SIGNAL_PAUSE);
    I_WaitOnSignal(&doomseq);
}

//
// I_ResumeSound
//

void I_ResumeSound(void)
{
    I_SetSeqStatus(&doomseq, SEQ_SIGNAL_RESUME);
    I_WaitOnSignal(&doomseq);
}

//
// I_StartMusic
//

void I_StartMusic(int mus_id)
{
    song_t* song;
    channel_t* chan;
    int i;

    I_LockMutex();

    song = &doomseq.songs[mus_id];
    for(i = 0; i < song->ntracks; i++)
    {
        chan = I_AddTrackToPlayList(&doomseq, song, &song->tracks[i]);

        if(chan == NULL)
            break;

        chan->volume = doomseq.musicvolume;
    }

    I_UnlockMutex();
}

//
// I_StopSound
//

void I_StopSound(sndsrc_t* origin, int sfx_id)
{
    song_t* song;
    channel_t* c;
    int i;

    I_LockMutex();

    song = &doomseq.songs[sfx_id];
    for(i = 0; i < MIDI_CHANNELS; i++)
    {
        c = &playlist[i];

        if(song == c->song || (origin && c->origin == origin))
            c->stop = true;
    }

    I_UnlockMutex();
}

//
// I_StartSound
//

void I_StartSound(int sfx_id, sndsrc_t* origin, int volume, int pan, int reverb)
{
    song_t* song;
    channel_t* chan;
    int i;

    if(doomseq.nsongs <= 0)
        return;

    I_LockMutex();

    song = &doomseq.songs[sfx_id];
    for(i = 0; i < song->ntracks; i++)
    {
        chan = I_AddTrackToPlayList(&doomseq, song, &song->tracks[i]);

        if(chan == NULL)
            break;

        chan->volume = (float)volume;
        chan->pan = (byte)(pan >> 1);
        chan->origin = origin;
        chan->depth = reverb;
    }

    I_UnlockMutex();
}

