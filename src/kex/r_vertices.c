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
// DESCRIPTION: Vertex draw lists.
// Stores geometry info produced by R_RenderBSPNode into a list for optimal rendering
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "d_devstat.h"
#include "r_local.h"
#include "r_texture.h"
#include "r_gl.h"
#include "r_vertices.h"
#include "i_system.h"
#include "z_zone.h"
#include "p_local.h"

drawlist_t drawlist[NUMDRAWLISTS];

dboolean R_GenerateSpritePlane(visspritelist_t* vissprite, vtx_t* vertex);
dboolean R_GenerateLowerSegPlane(seg_t *line, vtx_t* v);
dboolean R_GenerateUpperSegPlane(seg_t *line, vtx_t* v);
dboolean R_GenerateMiddleSegPlane(seg_t *line, vtx_t* v);
dboolean R_GenerateSwitchPlane(seg_t *line, vtx_t *v);

//
// DL_PushVertex
//

void DL_PushVertex(drawlist_t *dl)
{
    vtxlist_t *list;

    list = &dl->list[dl->index];

    if(list == &dl->list[dl->max - 1])
    {
        // add a new list to the array
        dl->max++;
        
        // allocate array
        dl->list =
            (vtxlist_t*)Z_Realloc(dl->list,
            dl->max * sizeof(vtxlist_t), PU_LEVEL, NULL);

        dmemset(&dl->list[dl->max - 1], 0, sizeof(vtxlist_t));

        list = &dl->list[dl->index];
        
        // count size of the draw list
        if(devparm) drawListSize += (dl->max * sizeof(vtxlist_t));
    }

    list->flags = 0;
    list->texid = 0;
    list->glowvalue = 0;
}

//
// DL_PushSprite
//

void DL_PushSprite(drawlist_t *dl, visspritelist_t *vis, int texid)
{
    vtxlist_t *list;
    mobj_t* mobj;
    
    DL_PushVertex(dl);
    
    list = &dl->list[dl->index];
    list->data = (visspritelist_t*)vis;
    list->drawfunc = R_GenerateSpritePlane;

    mobj = vis->spr;
    
    if(mobj->subsector->sector->lightlevel)
    {
        // add sprite's gamma glow values as a flag
        
        list->flags |= DLF_GLOW;
        list->glowvalue = mobj->subsector->sector->lightlevel;
    }
    
    // hack to include info on palette indexes
    list->texid =
        (texid | ((mobj->player ? mobj->player->palette : mobj->info->palette) << 24)
        | (list->flags << 16));
    
    dl->index++;
}

//
// DL_PushSeg
//

void DL_PushSeg(drawlist_t *dl, seg_t *line, int texid, int sidetype)
{
    vtxlist_t *list;
    
    DL_PushVertex(dl);
    
    list = &dl->list[dl->index];
    list->data = (seg_t*)line;

    switch(sidetype)
    {
    case 0:
        list->drawfunc = R_GenerateLowerSegPlane;
        break;
    case 1:
        list->drawfunc = R_GenerateUpperSegPlane;
        break;
    case 2:
        list->drawfunc = R_GenerateMiddleSegPlane;
        break;
    case 3:
        list->drawfunc = R_GenerateSwitchPlane;
        break;
    default:
        return;
    }

    if(line->linedef->flags & ML_HMIRROR)
        list->flags |= DLF_MIRRORS;
    
    if(line->linedef->flags & ML_VMIRROR)
        list->flags |= DLF_MIRRORT;
    
    if(line->frontsector->lightlevel)
    {
        // add seg's gamma glow values
        
        list->flags |= DLF_GLOW;
        list->glowvalue = line->frontsector->lightlevel;
    }
    
    list->texid = (list->flags << 16) | texid;
    
    dl->index++;
}

//
// DL_PushLeaf
//

void DL_PushLeaf(drawlist_t *dl, subsector_t *sub, int texid)
{
    vtxlist_t *list;
    sector_t* sector;
    
    DL_PushVertex(dl);
    
    list = &dl->list[dl->index];
    list->data = (subsector_t*)sub;
    list->drawfunc = NULL;

    sector = sub->sector;
    
    if(sector->lightlevel)
    {
        // add subsector's gamma glow values
        
        list->flags |= DLF_GLOW;
        list->glowvalue = sector->lightlevel;
    }
    
    list->texid = (list->flags << 16) | texid;
    
    dl->index++;
}

//
// DL_RenderDrawList
//

static dtexture envtexture = 0;
static float envcolor[4] = { 0, 0, 0, 0 };

vtx_t drawVertex[MAXDLDRAWCOUNT];

int qsort_CompareDL(const void *a, const void *b)
{
    vtxlist_t *xa = (vtxlist_t *)a;
    vtxlist_t *xb = (vtxlist_t *)b;
    
    return xb->texid - xa->texid;
}

void DL_RenderDrawList(void)
{
    int j = 0;
    int drawcount = 0;
    int i = 0;
    vtxlist_t* head;
    vtxlist_t* tail;
    drawlist_t* dl;
    dtexture tex;
    dboolean depthtest = false;

    dglSetVertex(drawVertex);

    dglActiveTexture(GL_TEXTURE0_ARB);
    if(r_fillmode.value)
        dglEnable(GL_TEXTURE_2D);
    else
        dglDisable(GL_TEXTURE_2D);

    //
    // setup texture environment for effects
    //
    if(r_texturecombiner.value)
    {
        dglTexCombColorf(GL_TEXTURE0_ARB, envcolor, GL_ADD);

        if(!nolights)
        {
            dglActiveTexture(GL_TEXTURE1_ARB);
            dglEnable(GL_TEXTURE_2D);
            dglTexCombModulate(GL_PREVIOUS, GL_PRIMARY_COLOR);
        }

        if(st_flashoverlay.value <= 0)
        {
            dglActiveTexture(GL_TEXTURE2_ARB);
            dglEnable(GL_TEXTURE_2D);
            dglTexCombColor(GL_PREVIOUS, flashcolor, GL_ADD);
        }

        dglTexCombReplaceAlpha(GL_TEXTURE0_ARB);
        dglActiveTexture(GL_TEXTURE0_ARB);
    }
    else
        dglTexCombReplace();

    dglEnable(GL_ALPHA_TEST);

    envtexture = 0;
    
    // begin draw list loop
    
    // -------------- Draw walls (segs) ---------------

    dl = &drawlist[DLT_WALL];
    
    if(dl->max > 0)
    {
        depthtest = true;

        qsort(dl->list, dl->index, sizeof(vtxlist_t), qsort_CompareDL);

        tail = &dl->list[dl->index];
        
        for(i = 0; i < dl->index; i++)
        {
            vtxlist_t* rover;
            seg_t* seg;
            sector_t* sector;

            if(!i)
                sector = NULL;

            head = &dl->list[i];

            // break if no data found in list
            if(!head->data)
                break;

            if(drawcount >= MAXDLDRAWCOUNT)
                I_Error("DL_RenderDrawList: Seg draw overflow by %i", dl->index);
            
            seg = (seg_t*)head->data;

            if(seg->frontsector != sector)
            {
                sector = seg->frontsector;

                bspColor[LIGHT_FLOOR]	= R_GetSectorLight(0xff, sector->colors[LIGHT_FLOOR]);
                bspColor[LIGHT_CEILING] = R_GetSectorLight(0xff, sector->colors[LIGHT_CEILING]);
                bspColor[LIGHT_THING]	= R_GetSectorLight(0xff, sector->colors[LIGHT_THING]);
                bspColor[LIGHT_UPRWALL] = R_GetSectorLight(0xff, sector->colors[LIGHT_UPRWALL]);
                bspColor[LIGHT_LWRWALL] = R_GetSectorLight(0xff, sector->colors[LIGHT_LWRWALL]);
            }

            if(!head->drawfunc(seg, &drawVertex[drawcount]))
                continue;

            dglTriangle(drawcount + 0, drawcount + 1, drawcount + 2);
            dglTriangle(drawcount + 1, drawcount + 2, drawcount + 3);

            drawcount += 4;

            rover = head + 1;
            
            if(rover != tail)
            {
                if(head->texid == rover->texid && head->glowvalue == rover->glowvalue)
                    continue;
            }
            
            head->texid = (head->texid & 0xffff);
            tex = textureptr[texturetranslation[head->texid]][palettetranslation[head->texid]];
            
            // setup texture ID
            R_BindWorldTexture(head->texid, 0, 0);
            
            // non sprite textures must repeat or mirrored-repeat
            dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                head->flags & DLF_MIRRORS ? GL_MIRRORED_REPEAT : GL_REPEAT);
            dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                head->flags & DLF_MIRRORT ? GL_MIRRORED_REPEAT : GL_REPEAT);

            envcolor[0] = envcolor[1] = envcolor[2] = ((float)head->glowvalue / 255.0f);
            dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcolor);

            dglDrawGeometry(drawcount, drawVertex);
            
            // count vertex size
            if(devparm) vertCount += drawcount;

            drawcount = 0;
            head->data = NULL;
        }
    }
    
    // -------------- Draw floors/ceilings (leafs) ---------------
    
    drawcount = 0;

    dl = &drawlist[DLT_FLAT];
    R_GLToggleBlend(1);
    
    if(dl->max > 0)
    {
        dglEnable(GL_CULL_FACE);
        
        qsort(dl->list, dl->index, sizeof(vtxlist_t), qsort_CompareDL);

        tail = &dl->list[dl->index];
        
        for(i = 0; i < dl->index; i++)
        {
            fixed_t tx;
            fixed_t ty;
            leaf_t* leaf;
            subsector_t* ss;
            sector_t* sector;
            vtxlist_t* rover;

            head = &dl->list[i];

            // break if no vertex data found in list
            if(!head->data)
                break;

            if(drawcount >= MAXDLDRAWCOUNT)
                I_Error("DL_RenderDrawList: Leaf draw overflow by %i", dl->index);

            ss = (subsector_t*)head->data;
            leaf = &leafs[ss->leaf];
            sector = ss->sector;

            for(j = 0; j < ss->numleafs - 2; j++)
                dglTriangle(drawcount, drawcount + 1 + j, drawcount + 2 + j);

            // need to keep texture coords small to avoid
            // floor 'wobble' due to rounding errors on some cards
            // make relative to first vertex, not (0,0)
            // which is arbitary anyway

            tx = (leaf->vertex->x >> 6) & ~(FRACUNIT - 1);
            ty = (leaf->vertex->y >> 6) & ~(FRACUNIT - 1);
            
            for(j = 0; j < ss->numleafs; j++)
            {
                int idx;
                vtx_t *v = &drawVertex[drawcount];

                if(head->flags & DLF_CEILING)
                    leaf = &leafs[(ss->leaf + (ss->numleafs - 1)) - j];
                else
                    leaf = &leafs[ss->leaf + j];

                v->x = F2D3D(leaf->vertex->x);
                v->y = F2D3D(leaf->vertex->y);

                if(head->flags & DLF_CEILING)
                {
                    if(i_interpolateframes.value)
                        v->z = F2D3D(sector->frame_z2[1]);
                    else
                        v->z = F2D3D(sector->ceilingheight);
                }
                else
                {
                    if(i_interpolateframes.value)
                        v->z = F2D3D(sector->frame_z1[1]);
                    else
                        v->z = F2D3D(sector->floorheight);
                }
                
                v->tu = F2D3D((leaf->vertex->x >> 6) - tx);
                v->tv = -F2D3D((leaf->vertex->y >> 6) - ty);

                // set the mapping offsets for scrolling floors/ceilings
                if((!(head->flags & DLF_CEILING) && sector->flags & MS_SCROLLFLOOR) ||
                    (head->flags & DLF_CEILING && sector->flags & MS_SCROLLCEILING))
                {
                    v->tu   += F2D3D(sector->xoffset >> 6);
                    v->tv   += F2D3D(sector->yoffset >> 6);
                }

                v->a = 0xff;

                if(head->flags & DLF_CEILING)
                    idx = sector->colors[LIGHT_CEILING];
                else
                    idx = sector->colors[LIGHT_FLOOR];

                R_LightToVertex(v, idx, 1);

                //
                // water layer 1
                //
                if(head->flags & DLF_WATER1)
                {
                    v->tv -= F2D3D(scrollfrac >> 6);
                    v->a = 0xA0;
                }

                //
                // water layer 2
                //
                if(head->flags & DLF_WATER2)
                    v->tu += F2D3D(scrollfrac >> 6);
                
                drawcount++;
            }

            rover = head + 1;
            
            if(rover != tail)
            {
                if(head->texid == rover->texid && head->glowvalue == rover->glowvalue)
                    continue;
            }

            head->texid = (head->texid & 0xffff);
            tex = textureptr[texturetranslation[head->texid]][palettetranslation[head->texid]];
            
            // setup texture ID
            R_BindWorldTexture(head->texid, 0, 0);

            envcolor[0] = envcolor[1] = envcolor[2] = ((float)head->glowvalue / 255.0f);
            dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcolor);

            dglDrawGeometry(drawcount, drawVertex);
            
            // count vertex size
            if(devparm) vertCount += drawcount;
            
            drawcount = 0;
            head->data = NULL;
        }
    }
    
    // -------------- Draw things (sprites) ---------------
    
    drawcount = 0;
    
    dglDisable(GL_CULL_FACE);
    dglDepthMask(0);

    R_GLToggleBlend(1);

    dl = &drawlist[DLT_SPRITE];
    
    if(dl->max > 0)
    {
        int palette = 0;

        depthtest = true;
        
        tail = &dl->list[dl->index];
        
        for(i = 0; i < dl->index; i++)
        {
            visspritelist_t* vis;
            mobj_t* mobj;

            head = &dl->list[i];

            // break if no vertex data found in list
            if(!head->data)
                break;

            if(drawcount >= MAXDLDRAWCOUNT)
                I_Error("DL_RenderDrawList: Sprite draw overflow by %i", dl->index);

            vis = (visspritelist_t*)head->data;
            mobj = vis->spr;

            if(!mobj)
                continue;

            if(!head->drawfunc(vis, &drawVertex[drawcount]))
                continue;

            dglTriangle(drawcount + 0, drawcount + 1, drawcount + 2);
            dglTriangle(drawcount + 1, drawcount + 2, drawcount + 3);

            drawcount += 4;
            
            // textid in sprites contains hack that stores palette index data
            palette = head->texid >> 24;
            head->texid = head->texid & 0xffff;
            R_BindSpriteTexture(head->texid, palette);

            envcolor[0] = envcolor[1] = envcolor[2] = ((float)head->glowvalue / 255.0f);
            dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcolor);

            if(mobj->type == MT_PROP_POLEBASELONG ||
                mobj->type == MT_PROP_POLEBASESHORT)
            {
                if(depthtest)
                {
                    dglDepthRange(0.0f, 0.999999f);
                    depthtest = false;
                }
            }
            else
            {
                if(!depthtest)
                {
                    dglDepthRange(0.0f, 1.0f);
                    depthtest = true;
                }
            }
            
            dglDrawGeometry(drawcount, drawVertex);
            
            // count vertex size
            if(devparm) vertCount += drawcount;

            drawcount = 0;
            head->data = NULL;
        }

        if(!depthtest)
            dglDepthRange(0.0f, 1.0f);
    }
    
    // -------------- Restore states ---------------
    
    dglDisable(GL_ALPHA_TEST);
    dglDepthMask(1);

    R_GLToggleBlend(0);
    R_GLResetCombiners();
}

//
// DL_Init
// Intialize draw list
//

void DL_Init(drawlist_t *dl)
{
    dl->index   = 0;
    dl->max     = 1;
    dl->list    = Z_Calloc(sizeof(vtxlist_t) * dl->max, PU_LEVEL, 0);
}

