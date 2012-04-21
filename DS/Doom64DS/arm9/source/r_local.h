#ifndef __R_LOCAL__
#define __R_LOCAL__

#include "doomdata.h"
#include "t_bsp.h"
#include "tables.h"

extern fixed_t      viewx;
extern fixed_t      viewy;
extern fixed_t      viewz;
extern angle_t      viewangle;
extern angle_t      viewpitch;
extern fixed_t      quakeviewx;
extern fixed_t      quakeviewy;
extern angle_t      viewangleoffset;
extern rcolor       flashcolor;
extern fixed_t      viewsin[2];
extern fixed_t      viewcos[2];

extern int          numvertexes;
extern vertex_t     *vertexes;
extern int          numsegs;
extern seg_t        *segs;
extern int          numsectors;
extern sector_t     *sectors;
extern int          numsubsectors;
extern subsector_t  *subsectors;
extern int          numnodes;
extern node_t       *nodes;
extern int          numleafs;
extern leaf_t       *leafs;
extern int          numsides;
extern side_t       *sides;
extern int          numlines;
extern line_t       *lines;
extern light_t      *lights;
extern int          numlights;
extern macroinfo_t  macros;

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.
extern int          skyflatnum;

//
// TEXTURE DATA
//

extern dtexture*    gfxtextures;
extern int          t_start;
extern int          t_end;
extern int          numtextures;
extern dtexture*    gfxsprites;
extern int          s_start;
extern int          s_end;
extern int          numsprites;

//
// R_CLIPPER
//
dboolean        R_Clipper_SafeCheckRange(angle_t startAngle, angle_t endAngle);
void            R_Clipper_SafeAddClipRange(angle_t startangle, angle_t endangle);
void            R_Clipper_Clear(void);
angle_t         R_FrustumAngle(void);

//
// R_MAIN
//
subsector_t*    R_PointInSubsector(fixed_t x, fixed_t y);
angle_t         R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
angle_t         _R_PointToAngle(fixed_t x, fixed_t y);
angle_t         R_PointToPitch(fixed_t z1, fixed_t z2, fixed_t dist);
int             _R_PointOnSide(fixed_t x, fixed_t y, node_t* node);
void            R_Init(void);
void            R_DrawFrame(void);
gl_texture_data *R_GetTexturePointer(dtexture texture);

//
// R_BSP
//
#define MAXSUBSECTORS   1024

extern subsector_t  *ssectlist[MAXSUBSECTORS];
extern subsector_t  **nextssect;
extern sector_t     *frontsector;

void            R_RenderBSPNode(int bspnum);

//
// R_DRAW
//
void            R_DrawScene(void);

//
// R_COLORS
//
enum
{
    LIGHT_FLOOR,
    LIGHT_CEILING,
    LIGHT_THING,
    LIGHT_UPRWALL,
    LIGHT_LWRWALL
};

void            R_RefreshBrightness(void);
void            R_SetLightFactor(float lightfactor);

#endif