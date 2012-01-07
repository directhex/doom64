// Emacs style mode select	 -*- C++ -*-
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
// DESCRIPTION: Console cvar functionality (from Quake)
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "doomstat.h"
#include "con_console.h"
#include "z_zone.h"
#include "st_stuff.h"
#include "g_actions.h"
#include "m_shift.h"
#include "i_system.h"
#include "con_console.h"
#include "con_cvar.h"
#include "d_englsh.h"

#ifdef _WIN32
#include "i_xinput.h"
#endif

cvar_t  *cvarcap;

// AM CVARS

cvar_t  am_lines            = { "am_lines", "1", 0 };
cvar_t  am_nodes            = { "am_nodes", "0", 0 };
cvar_t  am_ssect            = { "am_ssect", "0", 0 };
cvar_t  am_fulldraw         = { "am_fulldraw", "0", 0 };
cvar_t  am_showkeycolors    = { "am_showkeycolors", "0", 0 };
cvar_t  am_showkeymarkers   = { "am_showkeymarkers", "0", 0 };
cvar_t  am_drawobjects      = { "am_drawobjects", "0", 0 };
cvar_t  am_overlay          = { "am_overlay", "0", 0 };

// R CVARS

cvar_t  r_fov               = { "r_fov", "74.0", 0 };
cvar_t  r_fillmode          = { "r_fillmode", "1", 0 };
cvar_t  r_uniformtime       = { "r_uniformtime", "0.03", 0 };
cvar_t  r_fog               = { "r_fog", "1", 0 };
cvar_t  r_filter            = { "r_filter", "0", 0 };
cvar_t  r_looksky           = { "r_looksky", "0", 0 };
cvar_t  r_wipe              = { "r_wipe", "1", 0 };
cvar_t  r_drawtris          = { "r_drawtris", "0", 0 };
cvar_t  r_drawmobjbox       = { "r_drawmobjbox", "0", 0 };
cvar_t  r_drawblockmap      = { "r_drawblockmap", "0", 0 };
cvar_t  r_drawtrace         = { "r_drawtrace", "0", 0 };
cvar_t  r_texturecombiner   = { "r_texturecombiner", "1", 0 };
cvar_t  r_rendersprites     = { "r_rendersprites", "1", 0 };
cvar_t  r_texnonpowresize   = { "r_texnonpowresize", "0", 0 };

// V CVARS

cvar_t  v_msensitivityx     = { "v_msensitivityx", "5", 0 };
cvar_t  v_msensitivityy     = { "v_msensitivityy", "5", 0 };
cvar_t  v_macceleration     = { "v_macceleration", "0", 0 };
cvar_t  v_mlook             = { "v_mlook", "0", 0 };
cvar_t  v_mlookinvert       = { "v_mlookinvert", "0", 0 };
cvar_t  v_width             = { "v_width", "640", 0 };
cvar_t  v_height            = { "v_height", "480", 0 };
cvar_t  v_windowed          = { "v_windowed", "1", 0 };
cvar_t  v_vsync             = { "v_vsync", "1", 0 };
cvar_t  v_depthsize         = { "v_depthsize", "24", 0 };
cvar_t  v_buffersize        = { "v_buffersize", "32", 0 };

// I CVARS

cvar_t  i_gamma             = { "i_gamma", "0", 0 };
cvar_t  i_affinitymask      = { "i_affinitymask", "0", 0 };
cvar_t  i_brightness        = { "i_brightness", "100", 0 };
cvar_t  i_cpupriority       = { "i_cpupriority", "0", 0 };
cvar_t  i_interpolateframes = { "i_interpolateframes", "0", 0 };

#ifdef _USE_XINPUT  // XINPUT
cvar_t  i_rsticksensitivity = { "i_rsticksensitivity", "0.0080", 0 };
cvar_t  i_rstickthreshold   = { "i_rstickthreshold", "20.0", 0 };
cvar_t  i_xinputscheme      = { "i_xinputscheme", "0", 0 };
#endif

// M CVARS

cvar_t  m_messages          = { "m_messages", "1", 0 };
cvar_t  m_menufadetime      = { "m_menufadetime", "20", 0 };
cvar_t  m_regionblood       = { "m_regionblood", "0", 0 };

#if 0
cvar_t  m_menumouse         = { "m_menumouse", "0", 0 };
#endif

cvar_t  m_playername        = { "m_playername", "Player", 0 };
cvar_t  m_chatmacro0        = { "m_chatmacro0", HUSTR_CHATMACRO0 };
cvar_t  m_chatmacro1        = { "m_chatmacro1", HUSTR_CHATMACRO1 };
cvar_t  m_chatmacro2        = { "m_chatmacro2", HUSTR_CHATMACRO2 };
cvar_t  m_chatmacro3        = { "m_chatmacro3", HUSTR_CHATMACRO3 };
cvar_t  m_chatmacro4        = { "m_chatmacro4", HUSTR_CHATMACRO4 };
cvar_t  m_chatmacro5        = { "m_chatmacro5", HUSTR_CHATMACRO5 };
cvar_t  m_chatmacro6        = { "m_chatmacro6", HUSTR_CHATMACRO6 };
cvar_t  m_chatmacro7        = { "m_chatmacro7", HUSTR_CHATMACRO7 };
cvar_t  m_chatmacro8        = { "m_chatmacro8", HUSTR_CHATMACRO8 };
cvar_t  m_chatmacro9        = { "m_chatmacro9", HUSTR_CHATMACRO9 };

// P CVARS

cvar_t  p_allowjump         = { "p_allowjump", "0", 0 };
cvar_t  p_autoaim           = { "p_autoaim", "1", 0 };
cvar_t  p_features          = { "p_features", "0", 0 };
cvar_t  p_autorun           = { "p_autorun", "0", 0 };
cvar_t  p_fdoubleclick      = { "p_fdoubleclick", "0", 0 };
cvar_t  p_sdoubleclick      = { "p_dsoubleclick", "0", 0 };
cvar_t  p_usecontext        = { "p_usecontext", "0", 0 };
cvar_t  p_damageindicator   = { "p_damageindicator", "0", 0 };
cvar_t  p_regionmode        = { "p_regionmode", "0", 0 };

// ST CVARS

cvar_t  st_drawhud          = { "st_drawhud", "1", 0 };
cvar_t  st_crosshair        = { "st_crosshair", "0", 0 };
cvar_t  st_crosshairopacity = { "st_crosshairopacity", "80", 0 };
cvar_t  st_flashoverlay     = { "st_flashoverlay", "0", 0 };
cvar_t  st_regionmsg        = { "st_regionmsg", "0", 0 };

// S CVARS

cvar_t  s_sfxvol            = { "s_sfxvol", "80", 0 };
cvar_t  s_musvol            = { "s_musvol", "80", 0 };

// SERVER CVARS

cvar_t  sv_nomonsters       = { "sv_nomonsters", "0", 1 };
cvar_t  sv_fastmonsters     = { "sv_fastmonsters", "0", 1 };
cvar_t  sv_respawnitems     = { "sv_respawnitems", "0", 1 };
cvar_t  sv_lockmonsters     = { "sv_lockmonsters", "0", 1 };
cvar_t  sv_respawn          = { "sv_respawn", "0", 1 };
cvar_t  sv_skill            = { "sv_skill", "2", 1 };
cvar_t  sv_damagescale      = { "sv_damagescale", "1", 1 };
cvar_t  sv_healthscale      = { "sv_healthscale", "1", 1 };
cvar_t  sv_allowcheats      = { "sv_allowcheats", "0", 1 };
cvar_t  sv_allowcmds        = { "sv_allowcmds", "0", 1 };
cvar_t  sv_friendlyfire     = { "sv_friendlyfire", "0", 1 };
cvar_t  sv_keepitems        = { "sv_keepitems", "0", 1 };

// COMPATIBILITY CVARS

cvar_t  compat_collision    = { "compat_collision", "1", 0 };

//
// CON_CvarGet
//

cvar_t *CON_CvarGet(char *name)
{
    cvar_t	*var;
    
    for(var = cvarcap; var; var = var->next)
        if(!dstrcmp(name, var->name))
            return var;
        
        return NULL;
}

//
// CON_CvarValue
//

float CON_CvarValue(char *name)
{
    cvar_t	*var;
    
    var = CON_CvarGet(name);
    if(!var)
        return 0;
    
    return datof(var->string);
}

//
// CON_CvarString
//

char *CON_CvarString(char *name)
{
    cvar_t *var;
    
    var = CON_CvarGet(name);
    if(!var)
        return "";
    
    return var->string;
}

//
// CON_CvarAutoComplete
//

void CON_CvarAutoComplete(char *partial)
{
    cvar_t*     cvar;
    int         len;
    char*       name = NULL;
    int         spacinglength;
    dboolean    match = false;
    char*       spacing = NULL;
    
    dstrlwr(partial);
    
    len = dstrlen(partial);
    
    if(!len)
        return;
    
    // check functions
    for(cvar = cvarcap; cvar; cvar = cvar->next)
    {
        if(!dstrncmp(partial, cvar->name, len))
        {
            if(!match)
            {
                match = true;
                CON_Printf(0, "\n");
            }

            name = cvar->name;

            // setup spacing
            spacinglength = 24 - dstrlen(cvar->name);
            spacing = Z_Malloc(spacinglength + 1, PU_STATIC, NULL);
            dmemset(spacing, 0x20, spacinglength);
            spacing[spacinglength] = 0;

            // print all matching cvars
            CON_Printf(AQUA, "%s%s= %s (%s)\n", name, spacing, cvar->string, cvar->defvalue);

            Z_Free(spacing);

            CONCLEARINPUT();
            sprintf(ConsoleInputBuff+1, "%s ", name);
            ConsoleInputLen = dstrlen(ConsoleInputBuff);
        }
    }
}

//
// CON_CvarSet
//

void CON_CvarSet(char *var_name, char *value)
{
    cvar_t	*var;
    dboolean changed;
    
    var = CON_CvarGet(var_name);
    if(!var)
    {	// there is an error in C code if this happens
        CON_Printf(WHITE, "CON_CvarSet: variable %s not found\n", var_name);
        return;
    }
    
    changed = dstrcmp(var->string, value);
    
    Z_Free(var->string);	// free the old value string
    
    var->string = Z_Malloc(dstrlen(value)+1, PU_STATIC, 0);
    dstrcpy(var->string, value);
    var->value = datof(var->string);
}

//
// CON_CvarSetValue
//

void CON_CvarSetValue(char *var_name, float value)
{
    char val[32];
    
    sprintf(val, "%f",value);
    CON_CvarSet(var_name, val);
}

//
// CON_CvarRegister
//

void CON_CvarRegister(cvar_t *variable)
{
    char *oldstr;
    
    // first check to see if it has allready been defined
    if(CON_CvarGet(variable->name))
    {
        CON_Printf(WHITE, "CON_CvarRegister: Can't register variable %s, already defined\n", variable->name);
        return;
    }
    
    // copy the value off, because future sets will Z_Free it
    oldstr = variable->string;
    variable->string = Z_Malloc(dstrlen(variable->string)+1, PU_STATIC, 0);	
    dstrcpy(variable->string, oldstr);
    variable->value = datof(variable->string);
    variable->defvalue = Z_Malloc(dstrlen(variable->string)+1, PU_STATIC, 0);
    dstrcpy(variable->defvalue, variable->string);
    
    // link the variable in
    variable->next = cvarcap;
    cvarcap = variable;
}

void CON_CvarInit(void)
{
    // AM CVARS
    
    CON_CvarRegister(&am_lines);
    CON_CvarRegister(&am_nodes);
    CON_CvarRegister(&am_ssect);
    CON_CvarRegister(&am_fulldraw);
    CON_CvarRegister(&am_showkeycolors);
    CON_CvarRegister(&am_showkeymarkers);
    CON_CvarRegister(&am_drawobjects);
    CON_CvarRegister(&am_overlay);
    
    // R CVARS
    
    CON_CvarRegister(&r_fov);
    CON_CvarRegister(&r_fillmode);
    CON_CvarRegister(&r_uniformtime);
    CON_CvarRegister(&r_fog);
    CON_CvarRegister(&r_filter);
    CON_CvarRegister(&r_looksky);
    CON_CvarRegister(&r_wipe);
    CON_CvarRegister(&r_drawtris);
    CON_CvarRegister(&r_drawmobjbox);
    CON_CvarRegister(&r_drawblockmap);
    CON_CvarRegister(&r_drawtrace);
    CON_CvarRegister(&r_texturecombiner);
    CON_CvarRegister(&r_rendersprites);
    CON_CvarRegister(&r_texnonpowresize);
    
    // V CVARS
    
    CON_CvarRegister(&v_msensitivityx);
    CON_CvarRegister(&v_msensitivityy);
    CON_CvarRegister(&v_macceleration);
    CON_CvarRegister(&v_mlook);
    CON_CvarRegister(&v_mlookinvert);
    CON_CvarRegister(&v_width);
    CON_CvarRegister(&v_height);
    CON_CvarRegister(&v_windowed);
    CON_CvarRegister(&v_vsync);
    CON_CvarRegister(&v_depthsize);
    CON_CvarRegister(&v_buffersize);
    
    // I CVARS
    
    CON_CvarRegister(&i_gamma);
    CON_CvarRegister(&i_affinitymask);
    CON_CvarRegister(&i_brightness);
    CON_CvarRegister(&i_cpupriority);
    CON_CvarRegister(&i_interpolateframes);

#ifdef _USE_XINPUT  // XINPUT
    CON_CvarRegister(&i_rsticksensitivity);
    CON_CvarRegister(&i_rstickthreshold);
    CON_CvarRegister(&i_xinputscheme);
#endif
    
    // M CVARS
    
    CON_CvarRegister(&m_messages);
    CON_CvarRegister(&m_regionblood);
    CON_CvarRegister(&m_menufadetime);

#if 0
    CON_CvarRegister(&m_menumouse);
#endif

    CON_CvarRegister(&m_playername);
    CON_CvarRegister(&m_chatmacro0);
    CON_CvarRegister(&m_chatmacro1);
    CON_CvarRegister(&m_chatmacro2);
    CON_CvarRegister(&m_chatmacro3);
    CON_CvarRegister(&m_chatmacro4);
    CON_CvarRegister(&m_chatmacro5);
    CON_CvarRegister(&m_chatmacro6);
    CON_CvarRegister(&m_chatmacro7);
    CON_CvarRegister(&m_chatmacro8);
    CON_CvarRegister(&m_chatmacro9);
    
    // P CVARS
    
    CON_CvarRegister(&p_allowjump);
    CON_CvarRegister(&p_autoaim);
    CON_CvarRegister(&p_features);
    CON_CvarRegister(&p_autorun);
    CON_CvarRegister(&p_fdoubleclick);
    CON_CvarRegister(&p_sdoubleclick);
    CON_CvarRegister(&p_usecontext);
    CON_CvarRegister(&p_damageindicator);
    CON_CvarRegister(&p_regionmode);
    
    // ST CVARS
    
    CON_CvarRegister(&st_drawhud);
    CON_CvarRegister(&st_crosshair);
    CON_CvarRegister(&st_crosshairopacity);
    CON_CvarRegister(&st_flashoverlay);
    CON_CvarRegister(&st_regionmsg);
    
    // S CVARS
    
    CON_CvarRegister(&s_sfxvol);
    CON_CvarRegister(&s_musvol);

    // SERVER CVARS

    CON_CvarRegister(&sv_nomonsters);
    CON_CvarRegister(&sv_fastmonsters);
    CON_CvarRegister(&sv_respawnitems);
    CON_CvarRegister(&sv_lockmonsters);
    CON_CvarRegister(&sv_respawn);
    CON_CvarRegister(&sv_skill);
    CON_CvarRegister(&sv_damagescale);
    CON_CvarRegister(&sv_healthscale);
    CON_CvarRegister(&sv_allowcheats);
    CON_CvarRegister(&sv_allowcmds);
    CON_CvarRegister(&sv_friendlyfire);
    CON_CvarRegister(&sv_keepitems);

    // COMPATIBILITY CVARS

    CON_CvarRegister(&compat_collision);
}

