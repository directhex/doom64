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
#include "m_math.h"

static float envcolor[4] = { 0, 0, 0, 0 };

vtx_t drawVertex[MAXDLDRAWCOUNT];
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
// DL_ProcessWalls
//

static sector_t* prevsector = NULL;
dboolean DL_ProcessWalls(vtxlist_t* vl, int* drawcount)
{
    seg_t* seg = (seg_t*)vl->data;

    if(seg->frontsector != prevsector)
    {
        prevsector = seg->frontsector;

        bspColor[LIGHT_FLOOR]	= R_GetSectorLight(0xff, prevsector->colors[LIGHT_FLOOR]);
        bspColor[LIGHT_CEILING] = R_GetSectorLight(0xff, prevsector->colors[LIGHT_CEILING]);
        bspColor[LIGHT_THING]	= R_GetSectorLight(0xff, prevsector->colors[LIGHT_THING]);
        bspColor[LIGHT_UPRWALL] = R_GetSectorLight(0xff, prevsector->colors[LIGHT_UPRWALL]);
        bspColor[LIGHT_LWRWALL] = R_GetSectorLight(0xff, prevsector->colors[LIGHT_LWRWALL]);
    }

    if(!vl->drawfunc(seg, &drawVertex[*drawcount]))
        return false;

    dglTriangle(*drawcount + 0, *drawcount + 1, *drawcount + 2);
    dglTriangle(*drawcount + 1, *drawcount + 2, *drawcount + 3);

    *drawcount += 4;

    return true;
}

//
// DL_ProcessLeafs
//

dboolean DL_ProcessLeafs(vtxlist_t* vl, int* drawcount)
{
    int j;
    fixed_t tx;
    fixed_t ty;
    leaf_t* leaf;
    subsector_t* ss;
    sector_t* sector;
    int count;

    ss      = (subsector_t*)vl->data;
    leaf    = &leafs[ss->leaf];
    sector  = ss->sector;
    count   = *drawcount;

    for(j = 0; j < ss->numleafs - 2; j++)
        dglTriangle(count, count + 1 + j, count + 2 + j);

    // need to keep texture coords small to avoid
    // floor 'wobble' due to rounding errors on some cards
    // make relative to first vertex, not (0,0)
    // which is arbitary anyway

    tx = (leaf->vertex->x >> 6) & ~(FRACUNIT - 1);
    ty = (leaf->vertex->y >> 6) & ~(FRACUNIT - 1);
    
    for(j = 0; j < ss->numleafs; j++)
    {
        int idx;
        vtx_t *v = &drawVertex[count];

        if(vl->flags & DLF_CEILING)
            leaf = &leafs[(ss->leaf + (ss->numleafs - 1)) - j];
        else
            leaf = &leafs[ss->leaf + j];

        v->x = F2D3D(leaf->vertex->x);
        v->y = F2D3D(leaf->vertex->y);

        if(vl->flags & DLF_CEILING)
        {
            /*if(i_interpolateframes.value)
                v->z = F2D3D(sector->frame_z2[1]);
            else*/
                v->z = F2D3D(M_PointToZ(&sector->ceilingplane,
                leaf->vertex->x, leaf->vertex->y));
        }
        else
        {
            /*if(i_interpolateframes.value)
                v->z = F2D3D(sector->frame_z1[1]);
            else*/
                v->z = F2D3D(M_PointToZ(&sector->floorplane,
                leaf->vertex->x, leaf->vertex->y));
        }
        
        v->tu = F2D3D((leaf->vertex->x >> 6) - tx);
        v->tv = -F2D3D((leaf->vertex->y >> 6) - ty);

        // set the mapping offsets for scrolling floors/ceilings
        if((!(vl->flags & DLF_CEILING) && sector->flags & MS_SCROLLFLOOR) ||
            (vl->flags & DLF_CEILING && sector->flags & MS_SCROLLCEILING))
        {
            v->tu   += F2D3D(sector->xoffset >> 6);
            v->tv   += F2D3D(sector->yoffset >> 6);
        }

        v->a = 0xff;

        if(vl->flags & DLF_CEILING)
            idx = sector->colors[LIGHT_CEILING];
        else
            idx = sector->colors[LIGHT_FLOOR];

        R_LightToVertex(v, idx, 1);

        //
        // water layer 1
        //
        if(vl->flags & DLF_WATER1)
        {
            v->tv -= F2D3D(scrollfrac >> 6);
            v->a = 0xA0;
        }

        //
        // water layer 2
        //
        if(vl->flags & DLF_WATER2)
            v->tu += F2D3D(scrollfrac >> 6);
        
        count++;
    }

    *drawcount = count;

    return true;
}

//
// DL_ProcessSprites
//

dboolean DL_ProcessSprites(vtxlist_t* vl, int* drawcount)
{
    visspritelist_t* vis;
    mobj_t* mobj;

    vis = (visspritelist_t*)vl->data;
    mobj = vis->spr;

    if(!mobj)
        return false;

    if(!vl->drawfunc(vis, &drawVertex[*drawcount]))
        return false;

    dglTriangle(*drawcount + 0, *drawcount + 1, *drawcount + 2);
    dglTriangle(*drawcount + 1, *drawcount + 2, *drawcount + 3);

    *drawcount += 4;

    return true;
}

//
// DL_ProcessDrawList
//

int qsort_CompareDL(const void *a, const void *b)
{
    vtxlist_t *xa = (vtxlist_t *)a;
    vtxlist_t *xb = (vtxlist_t *)b;
    
    return xb->texid - xa->texid;
}

void DL_ProcessDrawList(int tag, dboolean (*procfunc)(vtxlist_t*, int*))
{
    drawlist_t* dl;
    int i;
    int drawcount = 0;
    vtxlist_t* head;
    vtxlist_t* tail;

    if(tag < 0 && tag >= NUMDRAWLISTS)
        return;

    dl = &drawlist[tag];

    if(dl->max > 0)
    {
        int palette = 0;

        // horrible hack...
        if(tag == DLT_WALL || tag == DLT_TWALL)
            prevsector = NULL;

        if(tag != DLT_SPRITE)
            qsort(dl->list, dl->index, sizeof(vtxlist_t), qsort_CompareDL);

        tail = &dl->list[dl->index];
        
        for(i = 0; i < dl->index; i++)
        {
            vtxlist_t* rover;

            head = &dl->list[i];

            // break if no data found in list
            if(!head->data)
                break;

            if(drawcount >= MAXDLDRAWCOUNT)
                I_Error("DL_ProcessDrawList: Draw overflow by %i, tag=%i", dl->index, tag);

            if(procfunc)
            {
                if(!procfunc(head, &drawcount))
                    continue;
            }

            rover = head + 1;
            
            if(tag != DLT_SPRITE)
            {
                if(rover != tail)
                {
                    if(head->texid == rover->texid && head->glowvalue == rover->glowvalue)
                        continue;
                }
            }
            
            // setup texture ID
            if(tag == DLT_SPRITE)
            {
                // textid in sprites contains hack that stores palette index data
                palette = head->texid >> 24;
                head->texid = head->texid & 0xffff;
                R_BindSpriteTexture(head->texid, palette);
            }
            else
            {
                head->texid = (head->texid & 0xffff);
                R_BindWorldTexture(head->texid, 0, 0);
            }
            
            // non sprite textures must repeat or mirrored-repeat
            if(tag == DLT_WALL)
            {
                dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    head->flags & DLF_MIRRORS ? GL_MIRRORED_REPEAT : GL_REPEAT);
                dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    head->flags & DLF_MIRRORT ? GL_MIRRORED_REPEAT : GL_REPEAT);
            }

            envcolor[0] = envcolor[1] = envcolor[2] = ((float)head->glowvalue / 255.0f);
            dglTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envcolor);

            dglDrawGeometry(drawcount, drawVertex);
            
            // count vertex size
            if(devparm) vertCount += drawcount;

            drawcount = 0;
            head->data = NULL;
        }
    }
}

//
// DL_BeginDrawList
//

void DL_BeginDrawList(dboolean t, dboolean a)
{
    dglSetVertex(drawVertex);

    dglActiveTexture(GL_TEXTURE0_ARB);
    if(t)
        dglEnable(GL_TEXTURE_2D);
    else
        dglDisable(GL_TEXTURE_2D);

    if(a)
        dglTexCombColorf(GL_TEXTURE0_ARB, envcolor, GL_ADD);
}

//
// DL_Init
// Intialize draw lists
//

void DL_Init(void)
{
    drawlist_t *dl;
    int i;

    for(i = 0; i < NUMDRAWLISTS; i++)
    {
        dl = &drawlist[i];

        dl->index   = 0;
        dl->max     = 1;
        dl->list    = Z_Calloc(sizeof(vtxlist_t) * dl->max, PU_LEVEL, 0);
    }
}

