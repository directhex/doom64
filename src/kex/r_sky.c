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
// DESCRIPTION: Sky rendering code
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include <stdlib.h>

#include "doomstat.h"
#include "r_lights.h"
#include "r_sky.h"
#include "w_wad.h"
#include "m_random.h"
#include "sounds.h"
#include "s_sound.h"
#include "p_local.h"
#include "z_zone.h"
#define close _close
#include "i_png.h"
#undef _close
#include "r_texture.h"

skydef_t*   sky;
int         skypicnum = -1;
int         skybackdropnum = -1;
int         skyflatnum = -1;
int         thunderCounter = 0;
int         lightningCounter = 0;
int         thundertic = 1;
dboolean    skyfadeback = false;
byte*       fireBuffer;
dPalette_t  firePal16[256];
int         fireLump = -1;

static word CloudOffsetY = 0;
static word CloudOffsetX = 0;

#define FIRESKYSIZE 64

//
// R_GetSkyViewPos
//

#define SKYVIEWPOS(angle, amount, x) x = -(angle / (float)ANG90 * amount); while(x < 1.0f) x += 1.0f

//
// R_CloudThunder
// Loosely based on subroutine at 0x80026418
//

static void R_CloudThunder(void)
{
    if(!(gametic & ((thunderCounter & 1) ? 1 : 3)))
        return;
    
    if((thunderCounter - thundertic) > 0)
    {
        thunderCounter = (thunderCounter - thundertic);
        return;
    }
    
    if(lightningCounter == 0)
    {
        S_StartSound(NULL, sfx_thndrlow + (M_Random() & 1));
        thundertic = (1 + (M_Random() & 1));
    }
    
    if(!(lightningCounter < 6))	// Reset loop after 6 lightning flickers
    {
        int rand = (M_Random() & 7);
        thunderCounter = (((rand << 4) - rand) << 2) + 60;
        lightningCounter = 0;
        return;
    }
    
    if((lightningCounter & 1) == 0)
    {
        sky->skycolor[0] += 0x001111;
        sky->skycolor[1] += 0x001111;
    }
    else
    {
        sky->skycolor[0] -= 0x001111;
        sky->skycolor[1] -= 0x001111;
    }
    
    thunderCounter = (M_Random() & 7) + 1;	// Do short delay loops for lightning flickers
    lightningCounter++;
}

//
// R_CloudTicker
//

static void R_CloudTicker(void)
{
    CloudOffsetX -= (dcos(viewangle) >> 10);
    CloudOffsetY += (dsin(viewangle) >> 9);
    
    if(sky->flags & SKF_THUNDER)
        R_CloudThunder();
}

//
// R_TitleSkyTicker
//

static void R_TitleSkyTicker(void)
{
    if(skyfadeback == true)
    {
        logoAlpha += 8;
        if(logoAlpha > 0xff)
        {
            logoAlpha = 0xff;
            skyfadeback = false;
        }
    }
}

//
// R_DrawSkyBackdrop
//

static void R_DrawSkyBackdrop(void)
{
    float pos1;
    float pos2;
    int gfxLmp;
    int width;
    int height;
    dboolean looksky = (int)r_looksky.value;
    
    gfxLmp = R_BindGfxTexture(lumpinfo[skybackdropnum].name, true);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    width = gfxwidth[gfxLmp];
    height = gfxheight[gfxLmp];
    
    SKYVIEWPOS(viewangle, 1, pos1);
    pos2 = looksky ? ((float)dcos(viewpitch) / 512) : 0.0f;
    
    R_GLToggleBlend(1);
    
    R_GLDraw2DStrip(0, (float)(170-height) - pos2, SCREENWIDTH, height,
        pos1, 1.25f + pos1, 0.006f, 1.0f, WHITE, 1);
    
    R_GLToggleBlend(0);
}

//
// R_DrawSimpleSky
//

static void R_DrawSimpleSky(int offset)
{
    float pos1;
    float pos2;
    float width;
    int height;
    int gfxLmp;
    dboolean looksky = (int)r_looksky.value;
    
    gfxLmp = R_BindGfxTexture(lumpinfo[skypicnum].name, false);
    height = gfxheight[gfxLmp];

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    SKYVIEWPOS(viewangle, 1, pos1);
    if(looksky)
    {
        SKYVIEWPOS(viewpitch, 1, pos2);
    }
    else
        pos2 = 0.0f;

    width = (float)SCREENWIDTH / (float)gfxwidth[gfxLmp];
    
    R_GLDraw2DStrip(0, (float)offset, SCREENWIDTH, -height,
        pos1, width + pos1, -pos2, -1.0f - pos2, WHITE, 1);
}

//
// R_DrawVoidSky
//

static void R_DrawVoidSky(void)
{
    R_GLEnable2D(1);
    
    dglDisable(GL_TEXTURE_2D);
    dglColor4ubv((byte*)&sky->skycolor[2]);
    dglRecti(SCREENWIDTH, SCREENHEIGHT, 0, 0);
    dglEnable(GL_TEXTURE_2D);
    
    R_GLDisable2D();
}

//
// R_DrawTitleSky
//

static void R_DrawTitleSky(void)
{
    R_DrawSimpleSky(240);
    R_DrawGfx(63, 25, sky->backdrop, D_RGBA(255, 255, 255, logoAlpha), false);
}

//
// R_DrawClouds
//

static void R_DrawClouds(void)
{
    rfloat pos = 0.0f;
    vtx_t v[4];

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    dglFrustum(SCREENWIDTH, SCREENHEIGHT, 45.0f, 0.1f);

    R_GLSetOrthoScale(1.0f); // force ortho mode to be set
    R_BindGfxTexture(lumpinfo[skypicnum].name, false);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    pos = (TRUEANGLES(viewangle) / 360.0f) * 2.0f;

    v[3].x = v[1].x = 1.1025f;
    v[0].x = v[2].x = -1.1025f;
    v[2].y = v[3].y = 0;
    v[0].y = v[1].y = 0.4315f;
    v[0].z = v[1].z = 0;
    v[2].z = v[3].z = -1.0f;
    v[0].tu = v[2].tu = F2D3D(CloudOffsetX) - pos;
    v[1].tu = v[3].tu = (F2D3D(CloudOffsetX) + 1.5f) - pos;
    v[0].tv = v[1].tv = F2D3D(CloudOffsetY);
    v[2].tv = v[3].tv = F2D3D(CloudOffsetY) + 2.0f;

    dglSetVertexColor(&v[0], sky->skycolor[0], 2);
    dglSetVertexColor(&v[2], sky->skycolor[1], 2);

    dglSetVertex(v);

    // pass 1: texture * skycolor
    dglTexCombReplaceAlpha(GL_TEXTURE0_ARB);
    dglActiveTexture(GL_TEXTURE0_ARB);
    dglTexCombColor(GL_TEXTURE, sky->skycolor[2], GL_MODULATE);

    // pass 2: result * const (though the original game uses the texture's alpha)
    dglActiveTexture(GL_TEXTURE1_ARB);
    dglEnable(GL_TEXTURE_2D);
    dglTexCombColor(GL_PREVIOUS, 0xFF909090, GL_MODULATE);

    // pass 3: result + fragment color
    dglActiveTexture(GL_TEXTURE2_ARB);
    dglEnable(GL_TEXTURE_2D);
    dglTexCombAdd(GL_PREVIOUS, GL_PRIMARY_COLOR);

    dglAlphaFunc(GL_ALWAYS, 0);
    dglEnable(GL_BLEND);
    dglMatrixMode(GL_MODELVIEW);
    dglPushMatrix();
    dglTranslated(0.0f, 0.0f, -1.0f);
    dglTriangle(0, 1, 2);
    dglTriangle(1, 2, 3);
    dglDrawGeometry(4, v);
    dglPopMatrix();
    dglDisable(GL_BLEND);
    dglAlphaFunc(GL_GEQUAL, ALPHACLEARGLOBAL);
    
    R_GLResetCombiners();

    if(skybackdropnum >= 0)
        R_DrawSkyBackdrop();
}

//
// R_SpreadFire
//

static void R_SpreadFire(byte* src1, byte* src2, int pixel, int counter, int* rand, int width)
{
    int randIdx = 0;
    byte *tmpSrc;
    
    if(pixel != 0)
    {
        randIdx = rndtable[*rand];
        *rand = ((*rand+2) & 0xff);
        
        tmpSrc = (src1 + (((counter - (randIdx & 3)) + 1) & (width-1)));
        *(byte*)(tmpSrc - width) = (pixel - ((randIdx & 1) << 4));
    }
    else
        *(byte*)(src2 - width) = 0;
}

//
// R_Fire
//

static void R_Fire(byte *buffer)
{
    int counter = 0;
    int rand = 0;
    int step = 0;
    int pixel = 0;
    int i = 0;
    const int width = FIRESKYSIZE;
    byte *src;
    byte *srcoffset;
    
    for(i = 0; i < (width*width); i++)
        buffer[i] <<= 4;
    
    rand = (M_Random() & 0xff);
    src = buffer;
    counter = 0;
    src += width;
    
    do	// height
    {
        srcoffset = (src + counter);
        pixel = *(byte*)srcoffset;
        
        step = 2;
        
        R_SpreadFire(src, srcoffset, pixel, counter, &rand, width);
        
        src += width;
        srcoffset += width;
        
        do	// width
        {
            pixel = *(byte*)srcoffset;
            step += 2;
            
            R_SpreadFire(src, srcoffset, pixel, counter, &rand, width);
            
            pixel = *(byte*)(srcoffset + width);
            src += width;
            srcoffset += width;
            
            R_SpreadFire(src, srcoffset, pixel, counter, &rand, width);
            
            src += width;
            srcoffset += width;
            
        } while(step != width);
        
        counter++;
        src -= ((width*width)-width);
        
    } while(counter != width);
    
    for(i = 0; i < (width*width); i++)
        buffer[i] >>= 4;
}

//
// R_InitFire
//

static rcolor firetexture[FIRESKYSIZE * FIRESKYSIZE];

void R_InitFire(void)
{
    int i;

    fireLump = W_GetNumForName("FIRE") - g_start;
    dmemset(&firePal16, 0, sizeof(dPalette_t)*256);
    for(i = 0; i < 16; i++)
    {
        firePal16[i].r = 16 * i;
        firePal16[i].g = 16 * i;
        firePal16[i].b = 16 * i;
        firePal16[i].a = 0xff;
    }
    
    fireBuffer = I_PNGReadData(g_start + fireLump,
        true, true, false, 0, 0, 0, 0);
    
    for(i = 0; i < 4096; i++)
        fireBuffer[i] >>= 4;
}

//
// R_FireTicker
//

static void R_FireTicker(void)
{
    if(leveltime & 1)
        R_Fire(fireBuffer);
}

//
// R_DrawFire
//

static void R_DrawFire(void)
{
    dboolean looksky = (int)r_looksky.value;
    float pos1;
    float pos2;
    vtx_t v[4];
    dtexture t = gfxptr[fireLump];
    int i;

    //
    // copy fire pixel data to texture data array
    //
    dmemset(firetexture, 0, sizeof(int) * FIRESKYSIZE * FIRESKYSIZE);
    for(i = 0; i < FIRESKYSIZE * FIRESKYSIZE; i++)
    {
        byte rgb[3];
            
        rgb[0] = firePal16[fireBuffer[i]].r;
        rgb[1] = firePal16[fireBuffer[i]].g;
        rgb[2] = firePal16[fireBuffer[i]].b;
                
        firetexture[i] = D_RGBA(rgb[2], rgb[1], rgb[0], 0xff);
    }
    
    if(!t)
        dglGenTextures(1, &gfxptr[fireLump]);

    dglBindTexture(GL_TEXTURE_2D, gfxptr[fireLump]);
    R_GLCheckFillMode();
    R_GLSetFilter();

    if(devparm)
        glBindCalls++;

    
    if(!t)
    {
        //
        // copy data if it didn't exist before
        //
        dglTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            FIRESKYSIZE,
            FIRESKYSIZE,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            firetexture
            );
    }
    else
    {
        //
        // update texture data
        //
        dglTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            FIRESKYSIZE,
            FIRESKYSIZE,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            firetexture
            );
    }
    
    SKYVIEWPOS(viewangle, 4, pos1);
    pos2 = looksky ? ((float)dcos(viewpitch) / 512) : 0.0f;
    
    //
    // adjust UV by 0.0035f units due to the fire sky showing a 
    // strip of color going along the top portion of the texture
    //
    R_GLSetupVertex(v, 0, -pos2, SCREENWIDTH, 120,
        pos1, 5.0f + pos1, 0.0035f + (looksky ? 0.007f : 0.0f), 1.0f, 0);

    dglSetVertexColor(&v[0], sky->skycolor[0], 2);
    dglSetVertexColor(&v[2], sky->skycolor[1], 2);
    
    R_GLRenderVertex(v, 1);
}

//
// R_DrawSky
//

void R_DrawSky(void)
{
    if(!sky)
        return;

    if(sky->flags & SKF_VOID)
    {
        R_DrawVoidSky();
    }
    else if(skypicnum >= 0)
    {
        if(sky->flags & SKF_CLOUD)
        {
            R_DrawClouds();
        }
        else
        {
            R_DrawSimpleSky(128);
        }
    }

    if(sky->flags & SKF_FIRE)
    {
        R_DrawFire();
    }

    if(skybackdropnum >= 0)
    {
        if(sky->flags & SKF_FADEBACK)
        {
            R_DrawTitleSky();
        }
        else if(sky->flags & SKF_BACKGROUND)
        {
            R_DrawSkyBackdrop();
        }
    }
}

//
// R_SkyTicker
//

void R_SkyTicker(void)
{
    if(menuactive)
        return;

    if(!sky)
        return;

    if(sky->flags & SKF_CLOUD)
    {
        R_CloudTicker();
    }

    if(sky->flags & SKF_FIRE)
    {
        R_FireTicker();
    }

    if(sky->flags & SKF_FADEBACK)
    {
        R_TitleSkyTicker();
    }
}


