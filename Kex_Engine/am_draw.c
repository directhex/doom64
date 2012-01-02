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
// DESCRIPTION: Automap rendering code
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "r_lights.h"
#include "m_fixed.h"
#include "tables.h"
#include "doomstat.h"
#include "z_zone.h"
#include "r_gl.h"
#include "r_texture.h"
#include "am_draw.h"
#include "m_cheat.h"
#include "r_sky.h"
#include "p_local.h"
#include "r_vertices.h"

extern fixed_t automappanx;
extern fixed_t automappany;
extern byte amModeCycle;

static angle_t am_viewangle;

//
// AM_BeginDraw
//

void AM_BeginDraw(angle_t view, fixed_t x, fixed_t y)
{
    curtexture = cursprite = curgfx = -1;
    am_viewangle = view;

    if(am_overlay.value)
    {
        R_GLToggleBlend(1);

        //
        // increase the rgb scale so the automap can look good while transparent (overlay mode)
        //
        dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        dglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 4);
    }

    dglDepthRange(0.0f, 0.0f);
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    dglFrustum(video_width, video_height, 45.0f, 0.1f);
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    dglPushMatrix();
    dglTranslatef(-F2D3D(automappanx), -F2D3D(automappany), 0);
    dglRotatef(-(float)TRUEANGLES(am_viewangle), 0.0f, 0.0f, 1.0f);
    dglTranslatef(-F2D3D(x), -F2D3D(y), 0);

    drawlist[DLT_AMAP].index = 0;
}

//
// AM_EndDraw
//

void AM_EndDraw(void)
{
    dglPopMatrix();
    dglDepthRange(0.0f, 1.0f);

    if(am_overlay.value)
    {
        R_GLToggleBlend(0);
        dglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        dglTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
    }

    R_GLResetCombiners();
}

//
// AM_DrawLeafs
//

void AM_DrawLeafs(float scale)
{
    subsector_t* sub;
    drawlist_t* am_drawlist;
    int i;

    am_drawlist = &drawlist[DLT_AMAP];

    for(i = 0; i < numsubsectors; i++)
    {
        sub = &subsectors[i];

        //
        // don't add sky flats
        //
        if(sub->sector->floorpic == skyflatnum)
            continue;

        //
        // must be mapped
        //
        if(segs[sub->firstline].linedef->flags & ML_MAPPED || amCheating)
        {
            //
            // add to draw list if visible
            //
            if(!(sub->sector->flags & MS_HIDESSECTOR) || am_fulldraw.value)
            {
                vtxlist_t *list;

                DL_PushVertex(am_drawlist);

                list            = &am_drawlist->list[am_drawlist->index++];
                list->data      = (subsector_t*)sub;
                list->drawfunc  = NULL;
                list->texid     = sub->sector->floorpic;
            }
        }
    }

    //
    // set vertex pointer
    //
    dglSetVertex(drawVertex);

    //
    // setup texture environments
    //
    dglActiveTexture(GL_TEXTURE0_ARB);

    if(am_ssect.value || !r_fillmode.value)
        dglDisable(GL_TEXTURE_2D);

    if(!nolights)
        dglTexCombModulate(GL_TEXTURE0_ARB, GL_PRIMARY_COLOR);
    else
        dglTexCombReplace();

    //
    // draw everything in list
    //
    if(am_drawlist->max > 0)
    {
        vtxlist_t* head;
        vtxlist_t* tail;
        int drawcount = 0;

        qsort(am_drawlist->list, am_drawlist->index, sizeof(vtxlist_t), qsort_CompareDL);

        tail = &am_drawlist->list[am_drawlist->index];

        for(i = 0; i < am_drawlist->index; i++)
        {
            vtxlist_t* rover;
            leaf_t* leaf;
            rcolor color;
            fixed_t	tx;
            fixed_t	ty;
            vtx_t *v;
            int j;

            head = &am_drawlist->list[i];

            if(!head->data)
                break;

            if(drawcount >= MAXDLDRAWCOUNT)
                I_Error("AM_DrawLeafs: Leaf draw overflow by %i", am_drawlist->index);

            sub = (subsector_t*)head->data;
            leaf = &leafs[sub->leaf];

            for(j = 0; j < sub->numleafs - 2; j++)
                dglTriangle(drawcount, drawcount + 1 + j, drawcount + 2 + j);

            tx = (leaf->vertex->x >> 6) & ~(FRACUNIT - 1);
            ty = (leaf->vertex->y >> 6) & ~(FRACUNIT - 1);

            //
            // setup RGB data
            //
            if(am_ssect.value)
            {
                int num = sub - subsectors;
                color = D_RGBA(
                    (num * 0x3f) & 0xff,
                    (num * 0xf) & 0xff,
                    (num * 0x7) & 0xff,
                    0xff
                    );

                if(color == 0xff000000)
                    color = D_RGBA(0x80, 0x80, 0x80, 0xff);
            }
            else
            {
                if(nolights)
                    color = WHITE;
                else
                {
                    light_t* light;

                    light = &lights[sub->sector->colors[LIGHT_FLOOR]];
                    color = D_RGBA(
                        light->active_r,
                        light->active_g,
                        light->active_b,
                        0xff
                        );
                }
            }

            if(am_overlay.value)
                color -= D_RGBA(0, 0, 0, 0xBF);

            v = &drawVertex[drawcount];

            dglSetVertexColor(v, color, sub->numleafs);

            //
            // setup vertex data
            //
            for(j = 0; j < sub->numleafs; j++)
            {
                vertex_t *vertex;

                vertex = leafs[sub->leaf + j].vertex;
        
                v[j].x = F2D3D(vertex->x);
                v[j].y = F2D3D(vertex->y);
                v[j].z = -(scale*2);
        
                v[j].tu = F2D3D((vertex->x >> 6) - tx);
                v[j].tv = -F2D3D((vertex->y >> 6) - ty);

                drawcount++;
            }

            rover = head + 1;

            if(rover != tail)
            {
                if(head->texid == rover->texid)
                    continue;
            }

            R_BindWorldTexture(head->texid, 0, 0);

            dglDrawGeometry(drawcount, drawVertex);
            
            // count vertex size
            if(devparm) vertCount += drawcount;
            
            drawcount = 0;
            head->data = NULL;
        }
    }
}

//
// AM_DrawLine
//

void AM_DrawLine(int x1, int x2, int y1, int y2, float scale, byte r, byte g, byte b)
{
    vtx_t v[2];
    
    v[0].x = F2D3D(x1);
    v[0].z = F2D3D(y1);
    v[1].x = F2D3D(x2);
    v[1].z = F2D3D(y2);

    v[0].r = v[1].r = r;
    v[0].g = v[1].g = g;
    v[0].b = v[1].b = b;
    v[0].a = v[1].a = 0xff;

    v[0].y = v[1].y = (scale*2);

    dglDisable(GL_TEXTURE_2D);
    dglColor4ub(r, g, b, 0xff);
    dglBegin(GL_LINES);
    dglVertex3f(v[0].x, v[0].z, -v[0].y);
    dglVertex3f(v[1].x, v[1].z, -v[1].y);
    dglEnd();
    dglEnable(GL_TEXTURE_2D);
}

//
// AM_DrawTriangle
//

void AM_DrawTriangle(mobj_t* mobj, float scale, dboolean solid, byte r, byte g, byte b)
{
    vtx_t tri[3];
    fixed_t x;
    fixed_t y;
    angle_t angle;
    
    if(mobj->flags & (MF_NOSECTOR|MF_RENDERLASER))
        return;
    else if(mobj->state == (state_t *)S_000)
        return;
    
    if(r_fillmode.value)
        dglPolygonMode(GL_FRONT_AND_BACK, (solid == 1) ? GL_LINE : GL_FILL);

    x = mobj->x;
    y = mobj->y;
    angle = mobj->angle;

    dglSetVertex(tri);
    dglTriangle(0, 1, 2);

    tri[0].z = tri[1].z = tri[2].z = -(scale*2);
    tri[0].tu = tri[1].tu = tri[2].tu = 0.0f;
    tri[0].tv = tri[1].tv = tri[2].tv = 0.0f;

    tri[0].x = F2D3D((dcos(angle) << 5) + x);
    tri[0].y = F2D3D((dsin(angle) << 5) + y);

    tri[1].x = F2D3D((dcos(angle + 0xA0000000) << 5) + x);
    tri[1].y = F2D3D((dsin(angle + 0xA0000000) << 5) + y);

    tri[2].x = F2D3D((dcos(angle + 0x60000000) << 5) + x);
    tri[2].y = F2D3D((dsin(angle + 0x60000000) << 5) + y);

    tri[0].r = tri[1].r = tri[2].r = r;
    tri[0].g = tri[1].g = tri[2].g = g;
    tri[0].b = tri[1].b = tri[2].b = b;
    tri[0].a = tri[1].a = tri[2].a = 0xff;
    
    dglDisable(GL_TEXTURE_2D);
    dglDrawGeometry(3, tri);
    dglEnable(GL_TEXTURE_2D);
    
    if(r_fillmode.value)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//
// AM_DrawSprite
//

void AM_DrawSprite(mobj_t* thing, float scale)
{
    spritedef_t	*sprdef;
    spriteframe_t *sprframe;
    fixed_t x;
    fixed_t y;
    float flip = 0.0f;
    float width;
    float height;
    int rot = 0;
    rcolor c;
    byte alpha;
    float scalefactor;
    float fz;
    float tx;
    float ty;
    float dx1;
    float dx2;
    float dy1;
    float dy2;
    float cos;
    float sin;
    vtx_t vtx[4];

    if(thing->flags & (MF_NOSECTOR|MF_RENDERLASER))
        return;
    else if(thing->state == (state_t *)S_000)
        return;
    
    if(!thing->sprite)
        return;
    else
    {
        //
        // setup sprite data
        //
        sprdef = &spriteinfo[thing->sprite];
        sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

        if(sprframe->rotate)
            rot = ((am_viewangle - thing->angle) + ANG90 + (unsigned)(ANG45 / 2) * 9) >> 29;

        //
        // keys and artifacts are scaled when zooming out
        //
        if(thing->type >= MT_ITEM_BLUECARDKEY && thing->type <= MT_ITEM_ARTIFACT3)
        {
            scalefactor = scale / 200.0f;

            if(scalefactor >= 5.0f)
                scalefactor = 5.0f;
        }
        else
            scalefactor = 1.0f;
        
        width = ((float)spritewidth[sprframe->lump[rot]] * scalefactor);
        height = ((float)spriteheight[sprframe->lump[rot]] * scalefactor);
        
        if(sprframe->flip[rot])
            flip = 1.0f;
        else
            flip = 0.0f;
    }

    x = thing->x;
    y = thing->y;

    //
    // get vertex data and rotate the plane
    //
    tx  = F2D3D(x);
    ty  = F2D3D(y);
    fz  = -(scale*2);
    dx1 = -(width / 2.0f);
    dx2 = dx1 + width;
    dy1 = -(height / 2.0f);
    dy2 = dy1 + height;
    cos = F2D3D(dcos(am_viewangle + ANG90));
    sin = F2D3D(dsin(am_viewangle + ANG90));

    dglSetVertex(vtx);

    vtx[0].x    = tx - ((dx2 * sin) + (dy1 * cos));
    vtx[0].y    = ty + ((dx2 * cos) - (dy1 * sin));
    vtx[0].z    = fz;
    vtx[0].tu   = flip;
    vtx[0].tv   = 0.0f;
    vtx[1].x    = tx - ((dx2 * sin) + (dy2 * cos));
    vtx[1].y    = ty + ((dx2 * cos) - (dy2 * sin));
    vtx[1].z    = fz;
    vtx[1].tu   = flip;
    vtx[1].tv   = 1.0f;
    vtx[2].x    = tx - ((dx1 * sin) + (dy2 * cos));
    vtx[2].y    = ty + ((dx1 * cos) - (dy2 * sin));
    vtx[2].z    = fz;
    vtx[2].tu   = 1 - flip;
    vtx[2].tv   = 1.0f;
    vtx[3].x    = tx - ((dx1 * sin) + (dy1 * cos));
    vtx[3].y    = ty + ((dx1 * cos) - (dy1 * sin));
    vtx[3].z    = fz;
    vtx[3].tu   = 1 - flip;
    vtx[3].tv   = 0.0f;
    
    R_BindSpriteTexture(sprframe->lump[rot], thing->info->palette);
    R_GLToggleBlend(1);

    alpha = (thing->alpha * (am_overlay.value ? 96 : 0xff)) / 0xff;

    //
    // show as full white in non-textured mode or if the mobj is an item
    //
    if((thing->frame & FF_FULLBRIGHT) || nolights || thing->flags & MF_SPECIAL || amModeCycle)
        c = D_RGBA(255, 255, 255, alpha);
    else
        c = R_GetSectorLight(alpha, thing->subsector->sector->colors[LIGHT_THING]);

    dglSetVertexColor(vtx, c, 4);

    //
    // do the drawing
    //
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    dglTriangle(0, 1, 2);
    dglTriangle(0, 2, 3);
    dglDrawGeometry(4, vtx);

    R_GLToggleBlend(0);
}

