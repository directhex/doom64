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

#ifndef CON_CVAR_H
#define CON_CVAR_H

#include "doomtype.h"

typedef struct cvar_s
{
    char*           name;
    char*           string;
    dboolean        nonclient;
    float           value;
    char*           defvalue;
    struct cvar_s*  next;
} cvar_t;

extern cvar_t*  cvarcap;

// AM CVARS

extern cvar_t   am_lines;
extern cvar_t   am_nodes;
extern cvar_t   am_ssect;
extern cvar_t   am_fulldraw;
extern cvar_t   am_showkeycolors;
extern cvar_t   am_showkeymarkers;
extern cvar_t   am_drawobjects;
extern cvar_t   am_overlay;

// R CVARS

extern cvar_t   r_fov;
extern cvar_t   r_fillmode;
extern cvar_t   r_uniformtime;
extern cvar_t   r_fog;
extern cvar_t   r_filter;
extern cvar_t   r_looksky;
extern cvar_t   r_wipe;
extern cvar_t   r_drawtris;
extern cvar_t   r_drawmobjbox;
extern cvar_t   r_drawblockmap;
extern cvar_t   r_drawtrace;
extern cvar_t   r_texturecombiner;
extern cvar_t   r_rendersprites;
extern cvar_t   r_texnonpowresize;

// V CVARS

extern cvar_t   v_msensitivityx;
extern cvar_t   v_msensitivityy;
extern cvar_t   v_macceleration;
extern cvar_t   v_mlook;
extern cvar_t   v_mlookinvert;
extern cvar_t   v_width;
extern cvar_t   v_height;
extern cvar_t   v_windowed;
extern cvar_t   v_vsync;
extern cvar_t   v_depthsize;
extern cvar_t   v_buffersize;

// I CVARS

extern cvar_t   i_gamma;
extern cvar_t   i_affinitymask;
extern cvar_t   i_brightness;
extern cvar_t   i_cpupriority;
extern cvar_t   i_interpolateframes;
extern cvar_t   i_rsticksensitivity;
extern cvar_t   i_rstickthreshold;
extern cvar_t   i_xinputscheme;

// M CVARS

extern cvar_t   m_messages;
extern cvar_t   m_menufadetime;
extern cvar_t   m_regionblood;

#if 0
extern cvar_t   m_menumouse;
#endif

extern cvar_t   m_playername;
extern cvar_t   m_chatmacro0;
extern cvar_t   m_chatmacro1;
extern cvar_t   m_chatmacro2;
extern cvar_t   m_chatmacro3;
extern cvar_t   m_chatmacro4;
extern cvar_t   m_chatmacro5;
extern cvar_t   m_chatmacro6;
extern cvar_t   m_chatmacro7;
extern cvar_t   m_chatmacro8;
extern cvar_t   m_chatmacro9;

// P CVARS

extern cvar_t   p_allowjump;
extern cvar_t   p_autoaim;
extern cvar_t   p_features;
extern cvar_t   p_autorun;
extern cvar_t   p_fdoubleclick;
extern cvar_t   p_sdoubleclick;
extern cvar_t   p_usecontext;
extern cvar_t   p_damageindicator;
extern cvar_t   p_regionmode;

// ST CVARS

extern cvar_t   st_drawhud;
extern cvar_t   st_crosshair;
extern cvar_t   st_crosshairopacity;
extern cvar_t   st_flashoverlay;
extern cvar_t   st_regionmsg;

// S CVARS

extern cvar_t   s_sfxvol;
extern cvar_t   s_musvol;

// SERVER CVARS

extern cvar_t   sv_nomonsters;
extern cvar_t   sv_fastmonsters;
extern cvar_t   sv_respawnitems;
extern cvar_t   sv_lockmonsters;
extern cvar_t   sv_respawn;
extern cvar_t   sv_skill;
extern cvar_t   sv_damagescale;
extern cvar_t   sv_healthscale;
extern cvar_t   sv_allowcheats;
extern cvar_t   sv_allowcmds;
extern cvar_t   sv_friendlyfire;
extern cvar_t   sv_keepitems;

// COMPATIBILITY CVARS

extern cvar_t   compat_collision;

void CON_CvarInit(void);
void CON_CvarRegister(cvar_t *variable);
void CON_CvarSet(char *var_name, char *value);
void CON_CvarSetValue(char *var_name, float value);
void CON_CvarAutoComplete(char *partial);
cvar_t *CON_CvarGet(char *name);

#endif

