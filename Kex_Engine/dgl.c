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
// DESCRIPTION: Utility functions for rendering
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "SDL_opengl.h"
#include "doomdef.h"
#include "doomstat.h"
#include "r_gl.h"
#include "r_glExt.h"
#include "con_console.h"
#include "i_system.h"

#define MAXINDICES  0x10000

word statindice = 0;

static word indicecnt = 0;
static word drawIndices[MAXINDICES];

//
// dglSetVertex
//

d_inline void dglSetVertex(vtx_t *vtx)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglSetVertex(vtx=0x%p)\n", vtx);
#endif
    dglTexCoordPointer(2, GL_FLOAT, sizeof(vtx_t), &vtx->tu);
    dglVertexPointer(3, GL_FLOAT, sizeof(vtx_t), vtx);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vtx_t), &vtx->r);
}

//
// dglTriangle
//

d_inline void dglTriangle(int v0, int v1, int v2)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTriangle(v0=%i, v1=%i, v2=%i)\n", v0, v1, v2);
#endif
    if(indicecnt + 3 >= MAXINDICES)
        I_Error("Triangle indice overflow");

    drawIndices[indicecnt++] = v0;
    drawIndices[indicecnt++] = v1;
    drawIndices[indicecnt++] = v2;
}

//
// dglDrawGeometry
//

d_inline void dglDrawGeometry(dword count, vtx_t *vtx)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglDrawGeometry(count=0x%x, vtx=0x%p)\n", count, vtx);
#endif
    dglLockArrays(0, count);
    dglDrawElements(GL_TRIANGLES, indicecnt, GL_UNSIGNED_SHORT, drawIndices);
    dglUnlockArrays();

    if(r_drawtris.value)
    {
        dword j = 0;
        byte b;

        for(j = 0; j < count; j++)
        {
            vtx[j].r = 0xff;
            vtx[j].g = 0xff;
            vtx[j].b = 0xff;
            vtx[j].a = 0xff;
        }

        dglGetBooleanv(GL_FOG, &b);

        if(b) dglDisable(GL_FOG);
        dglDisableClientState(GL_TEXTURE_COORD_ARRAY);
        dglDisable(GL_TEXTURE_2D);
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        dglDepthRange(0.0f, 0.0f);
        dglLockArrays(0, count);
        dglDrawElements(GL_TRIANGLES, indicecnt, GL_UNSIGNED_SHORT, drawIndices);
        dglUnlockArrays();
        dglDepthRange(0.0f, 1.0f);
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
        dglEnable(GL_TEXTURE_2D);
        if(b) dglEnable(GL_FOG);
    }

    if(devparm)
        statindice += indicecnt;

    indicecnt = 0;
}

//
// dglFrustum
//

d_inline void dglFrustum(int width, int height, rfloat fovy, rfloat znear)
{
    rfloat left;
    rfloat right;
    rfloat bottom;
    rfloat top;
    rfloat aspect;
    rfloat m[16];
    
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglFrustum(width=%i, height=%i, fovy=%f, znear=%f)\n", width, height, fovy, znear);
#endif
    
    aspect = (rfloat)width / (rfloat)height;
    top = znear * (rfloat)tan((double)fovy * M_PI / 360.0f);
    bottom = -top;
    left = bottom * aspect;
    right = top * aspect;
    
    m[ 0] = (2 * znear) / (right - left);
    m[ 4] = 0;
    m[ 8] = (right + left) / (right - left);
    m[12] = 0;
    
    m[ 1] = 0;
    m[ 5] = (2 * znear) / (top - bottom);
    m[ 9] = (top + bottom) / (top - bottom);
    m[13] = 0;
    
    m[ 2] = 0;
    m[ 6] = 0;
    m[10] = -1;
    m[14] = -2 * znear;
    
    m[ 3] = 0;
    m[ 7] = 0;
    m[11] = -1;
    m[15] = 0;
    
    dglMultMatrixf(m);
}

//
// dglSetVertexColor
//

d_inline void dglSetVertexColor(vtx_t *v, rcolor c, word count)
{
    int i = 0;
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglSetVertexColor(v=0x%p, c=0x%x, count=0x%x)\n", v, c, count);
#endif
    for(i = 0; i < count; i++)
        *(rcolor*)&v[i].r = c;
}

//
// dglGetColorf
//

d_inline void dglGetColorf(rcolor color, float* argb)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglGetColorf(color=0x%x, argb=0x%p)\n", color, argb);
#endif
    argb[3] = (float)((color >> 24) & 0xff) / 255.0f;
    argb[2] = (float)((color >> 16) & 0xff) / 255.0f;
    argb[1] = (float)((color >> 8) & 0xff) / 255.0f;
    argb[0] = (float)(color & 0xff) / 255.0f;
}

//
// dglTexCombReplace
//

d_inline void dglTexCombReplace(void)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombReplace\n");
#endif
    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
}

//
// dglTexCombColor
//

d_inline void dglTexCombColor(int t, rcolor c, int func)
{
    float f[4];
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombColor(t=0x%x, c=0x%x)\n", t, c);
#endif
    dglGetColorf(c, f);
    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, f);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, func);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, t);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
}

//
// dglTexCombColorf
//

d_inline void dglTexCombColorf(int t, float* f, int func)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombColorf(t=0x%x, f=%p)\n", t, f);
#endif
    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, f);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, func);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, t);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
}

//
// dglTexCombModulate
//

d_inline void dglTexCombModulate(int t, int s)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombFinalize(t=0x%x)\n", t);
#endif
    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, t);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, s);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
}

//
// dglTexCombAdd
//

d_inline void dglTexCombAdd(int t, int s)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombFinalize(t=0x%x)\n", t);
#endif
    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, t);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, s);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
}

//
// dglTexCombInterpolate
//

d_inline void dglTexCombInterpolate(int t, float a)
{
    float f[4];
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombInterpolate(t=0x%x, a=%f)\n", t, a);
#endif
    f[0] = f[1] = f[2] = 0.0f;
    f[3] = a;

    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
    dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, f);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, t);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
}

//
// dglTexCombReplaceAlpha
//

d_inline void dglTexCombReplaceAlpha(int t)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("dglTexCombReplaceAlpha(t=0x%x)\n", t);
#endif
    dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    dglTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, t);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    dglTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
    dglTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
}

