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
// DESCRIPTION: Main rendering code.
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include <math.h>

#include "doomdef.h"
#include "doomstat.h"
#include "v_sdl.h"
#include "d_devstat.h"
#include "r_local.h"
#include "r_sky.h"
#include "r_clipper.h"
#include "r_texture.h"
#include "r_gl.h"
#include "m_fixed.h"
#include "tables.h"
#include "i_system.h"
#include "m_random.h"
#include "d_net.h"
#include "p_local.h"
#include "z_zone.h"
#include "con_console.h"
#include "r_vertices.h"
#include "m_misc.h"

lumpinfo_t      *lumpinfo;
int             skytexture;

fixed_t         viewx=0;
fixed_t         viewy=0;
fixed_t         viewz=0;
float           fviewx=0;
float           fviewy=0;
float           fviewz=0;
angle_t         viewangle=0;
angle_t         viewpitch=0;
fixed_t         quakeviewx = 0;
fixed_t         quakeviewy = 0;
rcolor          flashcolor = 0;
angle_t         viewangleoffset=0;
float           viewoffset=0;
float           viewsin[2];
float           viewcos[2];
player_t        *renderplayer;

// [d64] for the interpolated water flats
fixed_t         scrollfrac;

int             logoAlpha = 0;

int             vertCount = 0;
unsigned int    renderTic = 0;
unsigned int    spriteRenderTic = 0;
unsigned int    glBindCalls = 0;
unsigned int    drawListSize = 0;

dboolean        bRenderSky = false;

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
// the coordinates are flipped until they are in
// the first octant of the coordinate system, then
// the y (<=x) is scaled and divided by x to get a
// tangent (slope) value which is looked up in the
// tantoangle[] table.
//Note not same as software version, which gets angle from (viewx, viewy) rather than (0, 0)

angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
    if((!x) && (!y))
        return 0;
    
    if(x >= 0)
    {
        // x >=0
        if(y>= 0)
        {
            // y>= 0
            
            if(x > y)
            {
                // octant 0
                return tantoangle[SlopeDiv(y, x)];
            }
            else
            {
                // octant 1
                return ANG90-1-tantoangle[SlopeDiv(x, y)];
            }
        }
        else
        {
            // y<0
            y = -y;
            
            if(x > y)
            {
                // octant 8
                return 0-tantoangle[SlopeDiv(y,x)];
            }
            else
            {
                // octant 7
                return ANG270+tantoangle[SlopeDiv(x,y)];
            }
        }
    }
    else
    {
        // x<0
        x = -x;
        
        if(y >= 0)
        {
            // y>= 0
            if(x > y)
            {
                // octant 3
                return ANG180-1-tantoangle[SlopeDiv(y,x)];
            }
            else
            {
                // octant 2
                return ANG90+ tantoangle[SlopeDiv(x,y)];
            }
        }
        else
        {
            // y<0
            y = -y;
            
            if(x > y)
            {
                // octant 4
                return ANG180+tantoangle[SlopeDiv(y,x)];
            }
            else
            {
                // octant 5
                return ANG270-1-tantoangle[SlopeDiv(x,y)];
            }
        }
    }
}

//
// R_PointOnSide
// Traverse BSP (sub) tree,
// check point against partition plane.
// Returns side 0 (front) or 1 (back).
//

int R_PointOnSide(fixed_t x, fixed_t y, node_t* node)
{
    fixed_t	dx;
    fixed_t	dy;
    fixed_t	left;
    fixed_t	right;
    
    if (!node->dx)
    {
        if (x <= node->x)
            return node->dy > 0;
        
        return node->dy < 0;
    }
    if (!node->dy)
    {
        if (y <= node->y)
            return node->dx < 0;
        
        return node->dx > 0;
    }
    
    dx = (x - node->x);
    dy = (y - node->y);
    
    // Try to quickly decide by looking at sign bits.
    if ( (node->dy ^ node->dx ^ dx ^ dy)&0x80000000 )
    {
        if  ( (node->dy ^ dx) & 0x80000000 )
        {
            // (left is negative)
            return 1;
        }
        return 0;
    }
    
    left = FixedMul ( F2INT(node->dy), dx );
    right = FixedMul ( dy , F2INT(node->dx));
    
    if (right < left)
    {
        // front side
        return 0;
    }
    // back side
    return 1;
}

//
// R_PointToAngle2
//

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    return R_PointToAngle (x2-x1, y2-y1);
}

//
// R_PointToPitch
//

angle_t R_PointToPitch(fixed_t z1, fixed_t z2, fixed_t dist)
{
    return R_PointToAngle2(0, z1, dist, z2);
}

//
// R_Init
//

void R_Init(void)
{
    int i = 0;
    int a = 0;
    double an;

    //
    // [d64] build finesine table
    //
    for(i = 0; i < (5 * FINEANGLES / 4); i++)
    {
        an = a * M_PI / (double)FINEANGLES;
        finesine[i] = (fixed_t)(sin(an) * (double)FRACUNIT);
        a += 2;
    }

    R_InitTextures();

    curtexture      = -1;
    curgfx          = -1;
    cursprite       = -1;
}

//
// R_PointInSubsector
//

subsector_t* R_PointInSubsector(fixed_t x, fixed_t y)
{
    node_t*	node;
    int		side;
    int		nodenum;
    
    // single subsector is a special case
    if (!numnodes)
        return subsectors;
    
    nodenum = numnodes-1;
    
    while (! (nodenum & NF_SUBSECTOR) )
    {
        node = &nodes[nodenum];
        side = R_PointOnSide (x, y, node);
        nodenum = node->children[side];
    }
    
    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_SetViewAngleOffset
//

void R_SetViewAngleOffset(angle_t angle)
{
    viewangleoffset = angle;
}

//
// R_SetViewOffset
//

void R_SetViewOffset(int offset)
{
    viewoffset=((float)offset)/10.0f;
}

//
// R_SetupLevel
//

void R_SetupLevel(void)
{
    R_CountSubsectorVerts();
    R_RefreshBrightness();

    DL_Init(&drawlist[DLT_WALL]);
    DL_Init(&drawlist[DLT_FLAT]);
    DL_Init(&drawlist[DLT_SPRITE]);
    DL_Init(&drawlist[DLT_AMAP]);
    
    drawListSize = 0;
    bRenderSky = true;
}

//
// R_SetupFrame
//

void R_SetupFrame(player_t *player)
{
    angle_t pitch;
    angle_t angle;
    fixed_t cam_z;
    mobj_t* viewcamera;

    renderplayer = player;

    //
    // reset active textures
    //
    curtexture = cursprite = curgfx = -1;

    //
    // setup view rotation/position
    //
    viewcamera = player->cameratarget;
    angle = (viewcamera->angle + quakeviewx) + viewangleoffset;
    pitch = viewcamera->pitch + ANG90;
    cam_z = (viewcamera == player->mo ? player->viewz : viewcamera->z) + quakeviewy;

    if(viewcamera == player->mo)
        pitch += player->recoilpitch;

    viewangle   = R_Interpolate(angle, frame_angle, (int)i_interpolateframes.value);
    viewpitch   = R_Interpolate(pitch, frame_pitch, (int)i_interpolateframes.value);
    viewx       = R_Interpolate(viewcamera->x, frame_viewx, (int)i_interpolateframes.value);
    viewy       = R_Interpolate(viewcamera->y, frame_viewy, (int)i_interpolateframes.value);
    viewz       = R_Interpolate(cam_z, frame_viewz, (int)i_interpolateframes.value);
    
    fviewx      = F2D3D(viewx);
    fviewy      = F2D3D(viewy);
    fviewz      = F2D3D(viewz);

    viewsin[0]  = F2D3D(dsin(viewangle));
    viewsin[1]  = F2D3D(dsin(viewpitch - ANG90));

    viewcos[0]  = F2D3D(dcos(viewangle));
    viewcos[1]  = F2D3D(dcos(viewpitch - ANG90));
    
    D_IncValidCount();

    //
    // setup fog
    //
    dglFogi(GL_FOG_MODE, GL_LINEAR);

    if(r_fillmode.value)
    {
        if(!skyflatnum)
            dglDisable(GL_FOG);
        else if(r_fog.value)
        {
            rfloat color[4] = { 0, 0, 0, 0 };
            rcolor fogcolor = 0;
            int fognear = 0;
            int fogfactor;

            fognear = sky ? sky->fognear : 985;
            fogfactor = (1000 - fognear);

            if(fogfactor <= 0)
                fogfactor = 1;

            dglEnable(GL_FOG);

            if(sky && (sky->fogcolor & 0xFFFFFF) != 0)
            {
                int min;
                int max;

                max = 128000 / fogfactor;
                min = ((fognear - 500) * 256) / fogfactor;

                fogcolor = sky->fogcolor;
                dglFogi(GL_FOG_MODE, GL_EXP);
                dglFogf(GL_FOG_DENSITY, 14.0f / (max + min));
            }
            else
            {
                float min;
                float max;
                float position;

                position = ((float)fogfactor / 1000.0f);

                if(position <= 0.0f)
                    position = 0.00001f;

                min = 5.0f / position;
                max = 30.0f / position;

                dglFogf(GL_FOG_START, min);
                dglFogf(GL_FOG_END, max);
            }

            dglGetColorf(fogcolor, color);
            dglFogfv(GL_FOG_COLOR, color);
        }
    }
}

//
// R_DrawWireframe
//

void R_DrawWireframe(dboolean enable)
{
    if (enable == true)
        CON_CvarSetValue(r_fillmode.name, 0);
    else	//Turn off wireframe and set device back to the way it was
    {
        CON_CvarSetValue(r_fillmode.name, 1);
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

//
// R_Interpolate
//

fixed_t R_Interpolate(fixed_t ticframe, fixed_t updateframe, dboolean enable)
{
    return !enable ? ticframe : updateframe + FixedMul(rendertic_frac, ticframe - updateframe);
}

//
// R_InterpolateSectors
//

static void R_InterpolateSectors(void)
{
    int i;

    for(i = 0; i < numsectors; i++)
    {
        sector_t* s = &sectors[i];

        s->frame_z1[1] = R_Interpolate(s->floorheight, s->frame_z1[0], 1);
        s->frame_z2[1] = R_Interpolate(s->ceilingheight, s->frame_z2[0], 1);
    }
}

//
// R_DrawGfx
//

void R_DrawGfx(int x, int y, const char* name, rcolor color, dboolean alpha)
{
    int gfxIdx = R_BindGfxTexture(name, alpha);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);

    R_GLToggleBlend(1);

    R_GLDraw2DStrip((float)x, (float)y, 
        gfxwidth[gfxIdx], gfxheight[gfxIdx], 0, 1.0f, 0, 1.0f, color, 0);

    R_GLToggleBlend(0);
}

//
// R_DrawHudSprite
//

void R_DrawHudSprite(int type, int rot, int frame, int x, int y, float scale, int pal, rcolor c)
{
    spritedef_t	*sprdef;
    spriteframe_t *sprframe;
    float flip = 0.0f;
    int w;
    int h;
    int offsetx = 0;
    int offsety = 0;
    
    R_GLToggleBlend(1);
    
    sprdef=&spriteinfo[type];
    sprframe = &sprdef->spriteframes[frame];

    R_BindSpriteTexture(sprframe->lump[rot], pal);

    w = spritewidth[sprframe->lump[rot]];
    h = spriteheight[sprframe->lump[rot]];
        
    if(scale <= 1.0f)
    {
        if(sprframe->flip[rot])
            flip = 1.0f;
        else
            flip = 0.0f;
            
        offsetx = (int)spriteoffset[sprframe->lump[rot]];
        offsety = (int)spritetopoffset[sprframe->lump[rot]];
    }

    R_GLSetOrthoScale(scale);

    R_GLDraw2DStrip(flip ? (float)(x + offsetx) - w : (float)x - offsetx, (float)y - offsety, w, h,
        flip, 1.0f - flip, 0, 1.0f, c, 0);

   R_GLSetOrthoScale(1.0f);
    
    cursprite = -1;
    curgfx = -1;
    
    R_GLToggleBlend(0);
}

//
// R_DrawReadDisk
//

static void R_DrawReadDisk(void)
{
    if(!BusyDisk)
        return;
    
    M_DrawText(296, 8, WHITE, 1, 0, "**");
    
    BusyDisk=true;
}

//
// R_DrawBlockMap
//

static void R_DrawBlockMap(void)
{
    float   fx;
    float   fy;
    float   fz;
    int     x;
    int     y;
    mobj_t* mo;

    dglDisable(GL_FOG);
    dglDisable(GL_TEXTURE_2D);
    dglDepthRange(0.0f, 0.0f);
    dglColor4ub(0, 128, 255, 255);
    dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    mo = players[displayplayer].mo;
    fz = F2D3D(mo->floorz);

    for(x = bmaporgx; x < ((bmapwidth << MAPBLOCKSHIFT) + bmaporgx); x += INT2F(MAPBLOCKUNITS))
    {
        for(y = bmaporgy; y < ((bmapheight << MAPBLOCKSHIFT) + bmaporgy); y += INT2F(MAPBLOCKUNITS))
        {
            fx = F2D3D(x);
            fy = F2D3D(y);

            dglBegin(GL_POLYGON);
            dglVertex3f(fx, fy, fz);
            dglVertex3f(fx + MAPBLOCKUNITS, fy, fz);
            dglVertex3f(fx + MAPBLOCKUNITS, fy + MAPBLOCKUNITS, fz);
            dglVertex3f(fx, fy + MAPBLOCKUNITS, fz);
            dglEnd();
        }
    }

    dglDepthRange(0.0f, 1.0f);
    dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    dglEnable(GL_FOG);
    dglEnable(GL_TEXTURE_2D);
}

//
// R_DrawRayTrace
//

static void R_DrawRayTrace(void)
{
    thinker_t* thinker;
    tracedrawer_t* tdrawer;

    for(thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
    {
        if(thinker->function.acp1 == (actionf_p1)T_TraceDrawer)
        {
            rcolor c = WHITE;

            tdrawer = ((tracedrawer_t*)thinker);

            if(tdrawer->flags == PT_ADDLINES)
            {
                c = D_RGBA(0, 0xff, 0, 0xff);
            }
            else if(tdrawer->flags == PT_ADDTHINGS)
            {
                c = D_RGBA(0, 0, 0xff, 0xff);
            }
            else if(tdrawer->flags == PT_EARLYOUT)
            {
                c = D_RGBA(0xff, 0, 0, 0xff);
            }
            else if(tdrawer->flags == (PT_ADDLINES | PT_ADDTHINGS))
            {
                c = D_RGBA(0, 0xff, 0xff, 0xff);
            }

            dglDepthRange(0.0f, 0.0f);
            dglDisable(GL_TEXTURE_2D);
            dglColor4ubv((byte*)&c);
            dglBegin(GL_LINES);
            dglVertex3f(F2D3D(tdrawer->x1), F2D3D(tdrawer->y1), F2D3D(tdrawer->z) - 8);
            dglVertex3f(F2D3D(tdrawer->x2), F2D3D(tdrawer->y2), F2D3D(tdrawer->z) - 8);
            dglEnd();
            dglEnable(GL_TEXTURE_2D);
            dglDepthRange(0.0f, 1.0f);
        }
    }
}

//
// R_DrawContextWall
// Displays an hightlight over the useable linedef
//

extern line_t* contextline; // from p_map.c
dboolean R_GenerateSwitchPlane(seg_t *line, vtx_t *v); // from r_bsp.c

static vertex_t* TraverseVertex(vertex_t* vertex, line_t* line)
{
    int i;
    line_t** l;

    for(i = 0, l = line->frontsector->lines; i < line->frontsector->linecount; i++)
    {
        if(l[i] == line)
            continue;

        if(l[i]->v1 == vertex)
        {
            if(l[i]->angle != line->angle)
                return vertex;

            if(l[i]->special != line->special)
                return vertex;

            // keep searching
            return TraverseVertex(l[i]->v2, l[i]);
        }
        else if(l[i]->v2 == vertex)
        {
            if(l[i]->angle != line->angle)
                return vertex;

            if(l[i]->special != line->special)
                return vertex;

            // keep searching
            return TraverseVertex(l[i]->v1, l[i]);
        }
    }

    // stop here
    return vertex;
}

static void R_DrawContextWall(line_t* line)
{
    vtx_t vtx[4];

    if(!line) return;

    if(!SWITCHMASK(line->flags))
    {
        vertex_t *v1;
        vertex_t *v2;

        //
        // try to merge all parallel lines by
        // finding the farthest left and right vertex
        //
        v1 = TraverseVertex(line->v1, line);
        v2 = TraverseVertex(line->v2, line);

        vtx[0].x = F2D3D(v1->x);
        vtx[1].x = F2D3D(v2->x);
        vtx[2].x = F2D3D(v2->x);
        vtx[3].x = F2D3D(v1->x);
        vtx[0].y = F2D3D(v1->y);
        vtx[1].y = F2D3D(v2->y);
        vtx[2].y = F2D3D(v2->y);
        vtx[3].y = F2D3D(v1->y);
        vtx[0].z = F2D3D(line->frontsector->floorheight);
        vtx[1].z = F2D3D(line->frontsector->floorheight);
        vtx[2].z = F2D3D(line->frontsector->ceilingheight);
        vtx[3].z = F2D3D(line->frontsector->ceilingheight);
    }
    else
    {
        int i;
        vtx_t v[4];
        seg_t* seg;

        for(i = 0; i < numsegs; i++)
        {
            if(segs[i].linedef == line)
            {
                seg = &segs[i];
                break;
            }
        }

        R_GenerateSwitchPlane(seg, v);

        vtx[0].x = v[0].x;
        vtx[1].x = v[1].x;
        vtx[2].x = v[3].x;
        vtx[3].x = v[2].x;
        vtx[0].y = v[0].y;
        vtx[1].y = v[1].y;
        vtx[2].y = v[3].y;
        vtx[3].y = v[2].y;
        vtx[0].z = v[0].z;
        vtx[1].z = v[1].z;
        vtx[2].z = v[3].z;
        vtx[3].z = v[2].z;
    }

    //
    // do the actual drawing
    //
    R_GLToggleBlend(1);

    dglDisable(GL_FOG);
    dglDepthRange(0.0f, 0.0f);
    dglDisable(GL_TEXTURE_2D);
    dglColor4ub(128, 128, 128, 64);
    dglBegin(GL_POLYGON);
    dglVertex3f(vtx[0].x, vtx[0].y, vtx[0].z);
    dglVertex3f(vtx[1].x, vtx[1].y, vtx[1].z);
    dglVertex3f(vtx[2].x, vtx[2].y, vtx[2].z);
    dglVertex3f(vtx[3].x, vtx[3].y, vtx[3].z);
    dglEnd();
    dglColor4ub(255, 255, 255, 255);
    dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    dglBegin(GL_POLYGON);
    dglVertex3f(vtx[0].x, vtx[0].y, vtx[0].z);
    dglVertex3f(vtx[1].x, vtx[1].y, vtx[1].z);
    dglVertex3f(vtx[2].x, vtx[2].y, vtx[2].z);
    dglVertex3f(vtx[3].x, vtx[3].y, vtx[3].z);
    dglEnd();
    dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    dglEnable(GL_TEXTURE_2D);
    dglDepthRange(0.0f, 1.0f);
    dglEnable(GL_FOG);

    R_GLToggleBlend(0);
}

//
// R_RenderPlayerView
//

void R_RenderPlayerView(player_t *player)
{
    angle_t a1;

    if(!r_fillmode.value)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    if(devparm)
        renderTic = I_GetTimeMS();
    
    //
    // reset list indexes
    //
    drawlist[DLT_WALL].index = 0;
    drawlist[DLT_FLAT].index = 0;
    drawlist[DLT_SPRITE].index = 0;
    
    R_ClearSprites();
    
    //
    // setup draw frame
    //
    R_SetupFrame(player);

    //
    // draw sky
    //
    if(bRenderSky)
        R_DrawSky();
    
    bRenderSky = false;

    //
    // setup view matrix
    //
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    dglFrustum(video_width, video_height, r_fov.value, 0.1f);
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    dglRotatef(-TRUEANGLES(viewpitch), 1.0f, 0.0f, 0.0f);
    dglRotatef(-TRUEANGLES(viewangle) + 90.0f, 0.0f, 0.0f, 1.0f);
    dglTranslatef(-fviewx, -fviewy, -fviewz);

    //
    // check for new console commands
    //
    NetUpdate();
    
    //
    // setup clipping
    //
    a1 = R_FrustumAngle();
    R_Clipper_Clear();
    R_Clipper_SafeAddClipRange(viewangle + a1, viewangle - a1);
    R_FrustrumSetup();

    if(i_interpolateframes.value)
        R_InterpolateSectors();

    //
    // traverse BSP for rendering
    //
    R_RenderBSPNode(numnodes-1);
    
    if(devparm)
        spriteRenderTic = I_GetTimeMS();
    
    //
    // setup sprites for rendering
    //
    if(r_rendersprites.value)
        R_SetupSprites();

    //
    // check for new console commands
    //
    NetUpdate();

    dglEnable(GL_DEPTH_TEST);

    //
    // render world
    //
    DL_RenderDrawList();

    if(r_drawblockmap.value)
        R_DrawBlockMap();

    if(r_drawmobjbox.value)
        R_DrawThingBBox();

    if(r_drawtrace.value)
        R_DrawRayTrace();

    if(p_usecontext.value)
        R_DrawContextWall(contextline);

    dglDisable(GL_FOG);
    dglDisable(GL_DEPTH_TEST);
    R_GLSetOrthoScale(1.0f);
    
    //
    // render player weapon sprites
    //
    if(ShowGun && player->cameratarget == player->mo &&
        !(player->cheats & CF_SPECTATOR))
    {
        R_RenderPlayerSprites(player);
    }
    
    if(devparm)
        spriteRenderTic = (I_GetTimeMS() - spriteRenderTic);
    
    if(devparm)
        R_DrawReadDisk();
    
    if(devparm)
        renderTic = (I_GetTimeMS() - renderTic);
    
    dglDisable(GL_CULL_FACE);

    //
    // check for new console commands
    //
    NetUpdate();
}


