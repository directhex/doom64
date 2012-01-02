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
// DESCRIPTION: Endlevel wipe FX.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "doomdef.h"
#include "r_wipe.h"
#include "r_local.h"
#include "st_stuff.h"
#include "m_fixed.h"
#include "z_zone.h"
#include "i_system.h"
#include "m_random.h"
#include "r_texture.h"
#include "doomstat.h"

void M_ClearMenus(void);    // from m_menu.c

static dtexture wipeMeltTexture = 0;
static int wipeFadeAlpha        = 0;

//
// WIPE_DisplayScreen
//

static void WIPE_DisplayScreen(vtx_t* v, rcolor argb)
{
    int starttime = I_GetTime();
    int tics = 0;

    dglSetVertexColor(v, argb, 4);
    R_GLRenderVertex(v, 1);

    do
    {
        tics = I_GetTime() - starttime;
        //
        // don't bash the CPU
        //
        I_Sleep(1);
    }
    while(!tics);

    //
    // make sure to refresh buffers
    //
    R_GLFinish();
}

//
// WIPE_FadeScreen
//

void WIPE_FadeScreen(int fadetics)
{
    int padw, padh;
    vtx_t v[4];
    float left, right, top, bottom;

    allowmenu = false;
    wipeFadeAlpha = 0xff;
    wipeMeltTexture = R_ScreenToTexture();

    padw = R_PadTextureDims(video_width);
    padh = R_PadTextureDims(video_height);
    
    R_GLToggleBlend(1);
    dglEnable(GL_TEXTURE_2D);

    //
    // setup vertex coordinates for plane
    //
    left = (float)(ViewWindowX * ViewWidth / video_width);
    right = left + (SCREENWIDTH * ViewWidth / video_width);
    top = (float)(ViewWindowY * ViewHeight / video_height);
    bottom = top + (SCREENHEIGHT * ViewHeight / video_height);
    
    v[0].x = v[2].x = left;
    v[1].x = v[3].x = right;
    v[0].y = v[1].y = top;
    v[2].y = v[3].y = bottom;

    v[0].z = v[1].z = v[2].z = v[3].z = 0.0f;
    
    v[0].tu = v[2].tu = 0.0f;
    v[1].tu = v[3].tu = (float)video_width / (float)padw;
    v[0].tv = v[1].tv = (float)video_height / (float)padh;
    v[2].tv = v[3].tv = 0.0f;

    dglBindTexture(GL_TEXTURE_2D, wipeMeltTexture);

    //
    // begin fade out
    //
    while(wipeFadeAlpha > 0)
    {
        //
        // clear frame
        //
        R_GLClearFrame(0xFF000000);

        if(wipeFadeAlpha < 0)
            wipeFadeAlpha = 0;

        //
        // display screen overlay
        //
        WIPE_DisplayScreen(v,
            D_RGBA(
            wipeFadeAlpha,
            wipeFadeAlpha,
            wipeFadeAlpha,
            0xff)
            );

        wipeFadeAlpha -= fadetics;
    }

    R_GLToggleBlend(0);
    R_UnloadTexture(&wipeMeltTexture);

    allowmenu = true;
}

//
// WIPE_MeltScreen
//

void WIPE_MeltScreen(void)
{
    int padw, padh;
    vtx_t v[4];
    float left, right, top, bottom;
    float blend[4];
    int i = 0;
    int refreshtic;

    M_ClearMenus();
    allowmenu = false;
    wipeMeltTexture = R_ScreenToTexture();

    //
    // setup texture color environment (abgr)
    //
    blend[0] = 0.25f;
    blend[1] = blend[2] = 0.0f;
    blend[3] = 255.0f;

    padw = R_PadTextureDims(video_width);
    padh = R_PadTextureDims(video_height);

    //
    // don't clear the frame, otherwise the effect won't be good
    //
    
    R_GLToggleBlend(1);
    dglEnable(GL_TEXTURE_2D);

    //
    // setup vertex coordinates for plane
    //
    left = (float)(ViewWindowX * ViewWidth / video_width);
    right = left + (SCREENWIDTH * ViewWidth / video_width);
    top = (float)(ViewWindowY * ViewHeight / video_height);
    bottom = top + (SCREENHEIGHT * ViewHeight / video_height);
    
    v[0].x = v[2].x = left;
    v[1].x = v[3].x = right;
    v[0].y = v[1].y = top;
    v[2].y = v[3].y = bottom;

    v[0].z = v[1].z = v[2].z = v[3].z = 0.0f;
    
    v[0].tu = v[2].tu = 0.0f;
    v[1].tu = v[3].tu = (float)video_width / (float)padw;
    v[0].tv = v[1].tv = (float)video_height / (float)padh;
    v[2].tv = v[3].tv = 0.0f;

    dglBindTexture(GL_TEXTURE_2D, wipeMeltTexture);

    //
    // nasty hack for fullscreen mode due to issues with refresh rate...
    // pause for a bit by displaying screen for half of a tic
    // then continue
    //
    refreshtic = I_GetTime();

    for(i = refreshtic; i < refreshtic + (TICRATE / 2); i++)
        WIPE_DisplayScreen(v, D_RGBA(0xff, 0xff, 0xff, 0xff));

    //
    // setup texture environment
    //
    dglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
    dglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0_ARB);
    dglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    dglTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
    dglTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

    for(i = 0; i < 160; i += 2)
    {
        int j;
        //
        // update environment color
        //
        dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blend);
        blend[0] += 0.0005f;

        //
        // setup color and draw plane
        //
        WIPE_DisplayScreen(v, D_RGBA(0xff, 0xff, 0xff, 8));

        //
        // move screen down. without clearing the frame, we should
        // get a nice melt effect using the HOM effect
        //
        for(j = 0; j < 4; j++)
            v[j].y += 0.5f;
    }

    //
    // reset combiners and blending
    //
    R_GLResetCombiners();
    R_GLToggleBlend(0);
    R_UnloadTexture(&wipeMeltTexture);

    //
    // fade screen out
    //
    WIPE_FadeScreen(6);
}

