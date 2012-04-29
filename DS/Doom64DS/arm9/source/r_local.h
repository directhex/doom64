#ifndef __R_LOCAL__
#define __R_LOCAL__

#include "doomdata.h"
#include "t_bsp.h"
#include "tables.h"
#include "p_pspr.h"
#include "d_player.h"

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

//
// TEXTURE DATA
//

extern dtexture*    gfxtextures;
extern int          t_start;
extern int          t_end;
extern int          numtextures;
extern int*         gfx_tex_prevtic;
extern dtexture*    gfxsprites;
extern int          s_start;
extern int          s_end;
extern int          numgfxsprites;
extern int*         gfx_spr_prevtic;
extern int*         spritepalindex;

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

extern spritedef_t	*spriteinfo;
extern short        *spritewidth;
extern short        *spriteheight;
extern short        *spriteoffset;
extern short        *spritetopoffset;

subsector_t*    R_PointInSubsector(fixed_t x, fixed_t y);
angle_t         R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
angle_t         _R_PointToAngle(fixed_t x, fixed_t y);
angle_t         R_PointToPitch(fixed_t z1, fixed_t z2, fixed_t dist);
int             R_PointOnSide(fixed_t x, fixed_t y, node_t* node);
void            R_Init(void);
void            R_DrawFrame(void);
void            R_PrecacheLevel(void);
int             R_GetTextureSize(int size);
void            R_LoadTexture(dtexture texture);
dboolean        R_LoadSprite(int sprite, int frame, int rotation, int *x, int *y, int *w, int *h);

static inline gl_texture_data *R_GetTexturePointer(dtexture texture)
{
    return (gl_texture_data*)DynamicArrayGet(&glGlob->texturePtrs, texture);
}

//
// R_BSP
//
#define MAXSUBSECTORS   1024
#define MAXSPRITES      128

extern subsector_t  *ssectlist[MAXSUBSECTORS];
extern subsector_t  **nextssect;
extern mobj_t       *visspritelist[MAXSPRITES];
extern mobj_t       **vissprite;
extern sector_t     *frontsector;

void            R_RenderBSPNode(int bspnum);

//
// R_DRAW
//
// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.
extern int          skyflatnum;
extern skydef_t*    sky;
extern int          skypicnum;
extern int          skybackdropnum;
extern dboolean     skyfadeback;
extern fixed_t      scrollfrac;

void            R_DrawScene(void);
void            R_DrawSimpleSky(void);
void            R_DrawPSprite(pspdef_t *psp, sector_t* sector, player_t *player);

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