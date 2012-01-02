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
// DESCRIPTION: Texture handling
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "doomstat.h"
#include "r_local.h"
#include "i_png.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_texture.h"
#include "r_gl.h"
#include "p_spec.h"

int         curtexture;
int         cursprite;
int         curtrans;
int         curgfx;

// world textures

int         t_start;
int         t_end;
int         swx_start;
int         numtextures;
dtexture**  textureptr;
word*       texturewidth;
word*       textureheight;
word*       texturetranslation;
word*       palettetranslation;

int         MinTextureSize;
int         MaxTextureSize;

// gfx textures

int         g_start;
int         g_end;
int         numgfx;
dtexture*   gfxptr;
word*       gfxwidth;
word*       gfxheight;

// sprite textures

int         s_start;
int         s_end;
dtexture**  spriteptr;
int         numsprtex;
word*       spritewidth;
float*      spriteoffset;
float*      spritetopoffset;
word*       spriteheight;
word*       spritecount;

//
// R_InitWorldTextures
//

void R_InitWorldTextures(void)
{
    int i = 0;
    
    t_start             = W_GetNumForName("T_START") + 1;
    t_end               = W_GetNumForName("T_END") - 1;
    swx_start           = -1;
    numtextures         = (t_end - t_start) + 1;
    textureptr          = (dtexture**)Z_Calloc(sizeof(dtexture*) * numtextures, PU_STATIC, NULL);
    texturetranslation  = Z_Calloc(numtextures * sizeof(word), PU_STATIC, NULL);
    palettetranslation  = Z_Calloc(numtextures * sizeof(word), PU_STATIC, NULL);
    texturewidth        = Z_Calloc(numtextures * sizeof(word), PU_STATIC, NULL);
    textureheight       = Z_Calloc(numtextures * sizeof(word), PU_STATIC, NULL);
    
    for(i = 0; i < numtextures; i++)
    {
        byte* png;
        int w;
        int h;
        
        // allocate at least one slot for each texture pointer
        textureptr[i] = (dtexture*)Z_Malloc(1 * sizeof(dtexture), PU_STATIC, 0);
        
        // get starting index for switch textures
        if(!dstrnicmp(lumpinfo[t_start + i].name, "SWX", 3) && swx_start == -1)
            swx_start = i;
        
        texturetranslation[i] = i;
        palettetranslation[i] = 0;
        
        // read PNG and setup global width and heights
        png = I_PNGReadData(t_start + i, true, true, false, &w, &h, NULL, 0);
        
        textureptr[i][0] = 0;
        texturewidth[i] = w;
        textureheight[i] = h;
        
        Z_Free(png);
    }
}

//
// R_BindWorldTexture
//

void R_BindWorldTexture(int texnum, int *width, int *height)
{
    byte *png;
    int w;
    int h;
    
    // get translation index
    texnum = texturetranslation[texnum];
    
    if(width)
        *width = texturewidth[texnum];
    if(height)
        *height = textureheight[texnum];
    
    if(curtexture == texnum)
        return;
    
    curtexture = texnum;
    
    // if texture is already in video ram
    if(textureptr[texnum][palettetranslation[texnum]])
    {
        dglBindTexture(GL_TEXTURE_2D, textureptr[texnum][palettetranslation[texnum]]);
        dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if(devparm) glBindCalls++;
        return;
    }
    
    // create a new texture
    png = I_PNGReadData(t_start + texnum, false, true, true,
        &w, &h, NULL, palettetranslation[texnum]);
    
    dglGenTextures(1, &textureptr[texnum][palettetranslation[texnum]]);
    dglBindTexture(GL_TEXTURE_2D, textureptr[texnum][palettetranslation[texnum]]);
    dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, png);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    R_GLCheckFillMode();
    R_GLSetFilter();
    
    // update global width and heights
    texturewidth[texnum] = w;
    textureheight[texnum] = h;
    
    if(width)
        *width = texturewidth[texnum];
    if(height)
        *height = textureheight[texnum];
    
    Z_Free(png);
    
    if(devparm) glBindCalls++;
}

//
// R_SetNewPalette
//

void R_SetNewPalette(int id, byte palID)
{
    palettetranslation[id] = palID;
    /*if(textureptr[id])
    {
    dglDeleteTextures(1, &textureptr[id]);
    textureptr[id] = 0;
}*/
}

//
// R_InitGfxTextures
//

void R_InitGfxTextures(void)
{
    int i = 0;
    
    g_start     = W_GetNumForName("G_START") + 1;
    g_end       = W_GetNumForName("G_END") - 1;
    numgfx      = (g_end - g_start) + 1;
    gfxptr      = Z_Calloc(numgfx * sizeof(dtexture), PU_STATIC, NULL);
    gfxwidth    = Z_Calloc(numgfx * sizeof(short), PU_STATIC, NULL);
    gfxheight   = Z_Calloc(numgfx * sizeof(short), PU_STATIC, NULL);
    
    for(i = 0; i < numgfx; i++)
    {
        byte* png;
        int w;
        int h;
        
        png = I_PNGReadData(g_start + i, true, true, false, &w, &h, NULL, 0);
        
        gfxptr[i] = 0;
        gfxwidth[i] = w;
        gfxheight[i] = h;
        
        Z_Free(png);
    }
}

//
// R_BindGfxTexture
//

int R_BindGfxTexture(const char* name, dboolean alpha)
{
    byte* png;
    byte* pngp;
    dboolean npot;
    int lump;
    int width;
    int height;
    int wp;
    int hp;
    int format;
    int type;
    int gfxid;
    
    lump = W_GetNumForName(name);
    gfxid = (lump - g_start);
    
    if(gfxid == curgfx)
        return gfxid;
    
    curgfx = gfxid;
    
    // if texture is already in video ram
    if(gfxptr[gfxid])
    {
        dglBindTexture(GL_TEXTURE_2D, gfxptr[gfxid]);
        if(devparm) glBindCalls++;
        return gfxid;
    }
    
    png = I_PNGReadData(lump, false, true, alpha, &width, &height, NULL, 0);
    
    // check for non-power of two textures
    npot = R_GLCheckExt("GL_ARB_texture_non_power_of_two");

    if(!npot && r_texnonpowresize.value <= 0)
        CON_CvarSetValue(r_texnonpowresize.name, 1.0f);
    
    if(r_texnonpowresize.value > 0)
    {
        const byte bits = (alpha ? 4 : 3);

        npot = false;

        // pad the width and heights
        wp = R_PadTextureDims(width);
        hp = R_PadTextureDims(height);
        
        pngp = Z_Calloc(wp * hp * bits, PU_STATIC, 0);

        if(r_texnonpowresize.value >= 2)
        {
            // this will probably look like crap
            gluScaleImage(alpha ? GL_RGBA : GL_RGB, width, height,
                GL_UNSIGNED_BYTE, png, wp, hp, GL_UNSIGNED_BYTE, pngp);
        }
        else
        {
            int y;

            for(y = 0; y < height; y++)          
            {
                dmemcpy(pngp + y * wp * bits,
                    ((byte*)png) + y * width * bits, width * bits);
            }

            width = wp;
            height = hp;
        }
    }
    
    dglGenTextures(1, &gfxptr[gfxid]);
    dglBindTexture(GL_TEXTURE_2D, gfxptr[gfxid]);
    
    // if alpha is specified, setup the format for only RGBA pixels (4 bytes) per pixel
    format = alpha ? GL_RGBA8 : GL_RGB8;
    type = alpha ? GL_RGBA : GL_RGB;
    
    if(!npot)
        dglTexImage2D(GL_TEXTURE_2D, 0, format, wp, hp, 0, type, GL_UNSIGNED_BYTE, pngp);
    else
        dglTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, png);
    
    R_GLCheckFillMode();
    R_GLSetFilter();
    
    gfxwidth[gfxid] = width;
    gfxheight[gfxid] = height;
    
    Z_Free(png);
    if(!npot)
        Z_Free(pngp);
    
    if(devparm) glBindCalls++;
    
    return gfxid;
}

//
// R_InitSpriteTextures
//

void R_InitSpriteTextures(void)
{
    int i = 0;
    int j = 0;
    int p = 0;
    int offset[2];
    
    s_start             = W_GetNumForName("S_START") + 1;
    s_end               = W_GetNumForName("S_END") - 1;
    numsprtex           = (s_end - s_start) + 1;
    spritewidth         = (word*)Z_Malloc(numsprtex * sizeof(word), PU_STATIC, 0);
    spriteoffset        = (float*)Z_Malloc(numsprtex * sizeof(float), PU_STATIC, 0);
    spritetopoffset     = (float*)Z_Malloc(numsprtex * sizeof(float), PU_STATIC, 0);
    spriteheight        = (word*)Z_Malloc(numsprtex * sizeof(word), PU_STATIC, 0);
    spriteptr           = (dtexture**)Z_Malloc(sizeof(dtexture*) * numsprtex, PU_STATIC, 0);
    spritecount         = (word*)Z_Calloc(numsprtex * sizeof(word), PU_STATIC, 0);
    
    // gather # of sprites per texture pointer
    for(i = 0; i < numsprtex; i++)
    {
        spritecount[i]++;
        
        for(j = 0; j < NUMSPRITES; j++)
        {
            // start looking for external palette lumps
            if(!dstrncmp(lumpinfo[s_start + i].name, sprnames[j], 4))
            {
                char palname[9];
                
                // increase the count if a palette lump is found
                for(p = 1; p < 10; p++)
                {
                    sprintf(palname, "PAL%s%i", sprnames[j], p);
                    if(W_CheckNumForName(palname) != -1)
                        spritecount[i]++;
                    else
                        break;
                }
                break;
            }
        }
    }
    
    for(i = 0; i < numsprtex; i++)
    {
        byte* png;
        int w;
        int h;
        size_t x;
        
        // allocate # of sprites per pointer
        spriteptr[i] = (dtexture*)Z_Malloc(spritecount[i] * sizeof(dtexture), PU_STATIC, 0);
        
        // reset references
        for(x = 0; x < spritecount[i]; x++)
            spriteptr[i][x] = 0;
        
        // read data and setup globals
        png = I_PNGReadData(s_start + i, true, true, false, &w, &h, offset, 0);
        
        spritewidth[i]      = w;
        spriteheight[i]     = h;
        spriteoffset[i]     = (float)offset[0];
        spritetopoffset[i]  = (float)offset[1];
        
        Z_Free(png);
    }
}

//
// R_BindSpriteTexture
//

void R_BindSpriteTexture(int spritenum, int pal)
{
    byte* png;
    byte* pngp;
    dboolean npot;
    int w;
    int h;
    int wp;
    int hp;
    
    if((spritenum == cursprite) && (pal == curtrans))
        return;

    // switch to default palette if pal is invalid
    if(pal && pal >= spritecount[spritenum])
        pal = 0;
    
    cursprite = spritenum;
    curtrans = pal;
    
    // if texture is already in video ram
    if(spriteptr[spritenum][pal])
    {
        dglBindTexture(GL_TEXTURE_2D, spriteptr[spritenum][pal]);
        dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
        dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
        if(devparm) glBindCalls++;
        return;
    }
    
    png = I_PNGReadData(s_start + spritenum, false, true, true, &w, &h, NULL, pal);
    
    // check for non-power of two textures
    npot = R_GLCheckExt("GL_ARB_texture_non_power_of_two");

    if(!npot && r_texnonpowresize.value <= 0)
        CON_CvarSetValue(r_texnonpowresize.name, 1.0f);
    
    if(r_texnonpowresize.value > 0)
    {
        npot = false;

        // pad the width and heights
        wp = R_PadTextureDims(w);
        hp = R_PadTextureDims(h);
        
        pngp = Z_Calloc(wp * hp * 4, PU_STATIC, 0);

        if(r_texnonpowresize.value >= 2)
        {
            // this will probably look like crap
            gluScaleImage(GL_RGBA, w, h, GL_UNSIGNED_BYTE, png, wp, hp, GL_UNSIGNED_BYTE, pngp);
        }
        else
        {
            int y;

            for(y = 0; y < h; y++)          
            {
                dmemcpy(pngp + y * wp * 4,
                    ((byte*)png) + y * w * 4, w * 4);
            }

            w = wp;
            h = hp;
        }
    }

    spritewidth[spritenum] = w;
    spriteheight[spritenum] = h;
    
    dglGenTextures(1, &spriteptr[spritenum][pal]);
    dglBindTexture(GL_TEXTURE_2D, spriteptr[spritenum][pal]);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    
    if(!npot)
        dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, wp, hp, 0, GL_RGBA, GL_UNSIGNED_BYTE, pngp);
    else
        dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, png);
    
    R_GLCheckFillMode();
    R_GLSetFilter();
    
    Z_Free(png);
    if(!npot)
        Z_Free(pngp);
    
    if(devparm) glBindCalls++;
}

//
// R_CaptureScreen
//

dtexture R_ScreenToTexture(void)
{
    dtexture id;
    int width;
    int height;
    
    dglEnable(GL_TEXTURE_2D);
    
    dglGenTextures(1, &id);
    dglBindTexture(GL_TEXTURE_2D, id);
    
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    width = R_PadTextureDims(video_width);
    height = R_PadTextureDims(video_height);

    dglTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        0
        );

    dglCopyTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        0,
        0,
        width,
        height
        );
    
    return id;
}

//
// R_BindDummyTexture
//

static dtexture dummytexture = 0;

void R_BindDummyTexture(void)
{
    if(!dummytexture)
    {
        //
        // build dummy texture
        //

        byte rgb[12];   // 2x2 RGB texture

        dmemset(rgb, 0xff, 12);

        dglGenTextures(1, &dummytexture);
        dglBindTexture(GL_TEXTURE_2D, dummytexture);
        dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
        dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
        R_GLCheckFillMode();
        R_GLSetFilter();
    }
    else
        dglBindTexture(GL_TEXTURE_2D, dummytexture);
}

//
// R_UnloadTexture
//

void R_UnloadTexture(dtexture* texture)
{
    if(*texture != 0)
    {
        dglDeleteTextures(1, texture);
        *texture = 0;
    }
}

//
// R_InitTextures
//

void R_InitTextures(void)
{
    R_InitWorldTextures();
    R_InitGfxTextures();
    R_InitSpriteTextures();
}

//
// R_PadTextureDims
//

#define MAXTEXSIZE	2048
#define MINTEXSIZE	1

int R_PadTextureDims(int n)
{
    int mask = 1;
    
    while(mask < 0x40000000)
    {
        if(n == mask || (n & (mask-1)) == n)
            return mask;
        
        mask <<= 1;
    }
    return n;
}

//
// R_CheckTextureSizeLimits
//

void R_CheckTextureSizeLimits(void)
{
    MinTextureSize = R_PadTextureDims(MinTextureSize);
    MaxTextureSize = R_PadTextureDims(MaxTextureSize);
}

//
// R_DumpTextures
// Unbinds all textures from memory
//

void R_DumpTextures(void)
{
    int	i;
    int j;
    int	p;
    
    for(i = 0; i < numtextures; i++)
    {
        R_UnloadTexture(&textureptr[i][0]);
        
        for(p = 0; p < numanimdef; p++)
        {
            int lump = W_GetNumForName(animdefs[p].name) - t_start;
            
            if(lump != i)
                continue;
            
            if(animdefs[p].palette)
            {
                for(j = 1; j < animdefs[p].frames; j++)
                    R_UnloadTexture(&textureptr[i][j]);
            }
        }
    }
    
    for(i = 0; i < numsprtex; i++)
    {
        for(p = 0; p < spritecount[i]; p++)
            R_UnloadTexture(&spriteptr[i][p]);
    }

    for(i = 0; i < numgfx; i++)
        R_UnloadTexture(&gfxptr[i]);
}

//
// R_PrecacheLevel
// Loads and binds all world textures before level startup
//

void R_PrecacheLevel(void)
{
    char *texturepresent;
    int	i;	
    
    R_DumpTextures();
    
    texturepresent = Z_Alloca(numtextures);
    dmemset(texturepresent,0, numtextures);
    
    for(i = 0; i < numsides; i++)
    {
        texturepresent[sides[i].toptexture] = 1;
        texturepresent[sides[i].midtexture] = 1;
        texturepresent[sides[i].bottomtexture] = 1;
    }
    
    for(i = 0; i < numsectors; i++)
    {
        texturepresent[sectors[i].ceilingpic] = 1;
        texturepresent[sectors[i].floorpic] = 1;
    }
    
    for(i = 0; i < numtextures; i++)
    {
        if(texturepresent[i])
        {
            R_BindWorldTexture(i, 0, 0);

            // texture isn't exactly loaded into memory
            // until something is drawn, so draw something..
            dglBegin(GL_TRIANGLES);
                dglVertex2f(0, 0);
                dglVertex2f(0, 0);
                dglVertex2f(0, 0);
            dglEnd();
        }
    }

    dglActiveTexture(GL_TEXTURE1_ARB);
    R_BindDummyTexture();

    dglActiveTexture(GL_TEXTURE2_ARB);
    R_BindDummyTexture();

    dglActiveTexture(GL_TEXTURE3_ARB);
    R_BindDummyTexture();

    dglActiveTexture(GL_TEXTURE4_ARB);
    R_BindDummyTexture();

    R_GLResetCombiners();
}
