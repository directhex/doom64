#include <math.h>

#include "m_fixed.h"
#include "tables.h"
#include "r_local.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "d_main.h"

// render view globals
fixed_t         viewx;
fixed_t         viewy;
fixed_t         viewz;
angle_t         viewangle;
angle_t         viewpitch;
fixed_t         quakeviewx;
fixed_t         quakeviewy;
angle_t         viewangleoffset;
fixed_t         viewsin[2];
fixed_t         viewcos[2];
int             frametic = 0;

// sprite info globals
spritedef_t     *spriteinfo;

// gfx texture globals
int             t_start;
int             t_end;
int             numtextures;

// gfx sprite globals
int             s_start;
int             s_end;
int             numgfxsprites;
short           *spriteoffset;
short           *spritetopoffset;
short           *spritewidth;
short           *spriteheight;
byte            *spritetiles;
short           **spritetilelist;

//
// R_PointToAngle2
//
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    return _R_PointToAngle(x2 - x1, y2 - y1);
}

//
// R_PointToPitch
//

angle_t R_PointToPitch(fixed_t z1, fixed_t z2, fixed_t dist)
{
    return R_PointToAngle2(0, z1, dist, z2);
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
    
    if(!node->dx)
    {
        if(x <= node->x)
            return node->dy > 0;
        
        return node->dy < 0;
    }
    if(!node->dy)
    {
        if(y <= node->y)
            return node->dx < 0;
        
        return node->dx > 0;
    }
    
    dx = (x - node->x);
    dy = (y - node->y);
    
    left = F2INT(node->dy) * F2INT(dx);
    right = F2INT(dy) * F2INT(node->dx);
    
    if(right < left)
    {
        // front side
        return 0;
    }

    // back side
    return 1;
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
// R_RenderView
//

static void R_RenderView(void)
{
    angle_t an;

    nextssect = ssectlist;
    vissprite = visspritelist;

    an = (ANG180 + ANG45);//R_FrustumAngle();

    R_Clipper_Clear();
    R_Clipper_SafeAddClipRange(viewangle + an, viewangle - an);
    R_RenderBSPNode(numnodes - 1);
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

    R_InitData();
}

//
// R_DrawFrame
//

void R_DrawFrame(void)
{
    angle_t pitch;
    angle_t angle;
    fixed_t cam_z;
    mobj_t* viewcamera;
    player_t* player;
    pspdef_t* psp;

    //
    // setup view rotation/position
    //
    player = &players[consoleplayer];
    viewcamera = player->cameratarget;
    angle = (viewcamera->angle + quakeviewx) + viewangleoffset;
    pitch = viewcamera->pitch + ANG90;
    cam_z = (viewcamera == player->mo ? player->viewz : viewcamera->z) + quakeviewy;

    if(viewcamera == player->mo)
        pitch += player->recoilpitch;

    viewangle = angle;
    viewpitch = pitch;
    viewx = viewcamera->x;
    viewy = viewcamera->y;
    viewz = cam_z;

    viewsin[0]  = dsin(viewangle + ANG90);
    viewsin[1]  = dsin(viewpitch - ANG90);
    viewcos[0]  = dcos(viewangle + ANG90);
    viewcos[1]  = dcos(viewpitch - ANG90);

    MATRIX_CONTROL      = GL_PROJECTION;
    MATRIX_IDENTITY     = 0;
    MATRIX_CONTROL      = GL_MODELVIEW;
    MATRIX_IDENTITY     = 0;

    MATRIX_PUSH = 0;

    MATRIX_TRANSLATE    = -(16 << 4);
    MATRIX_TRANSLATE    = -0;
    MATRIX_TRANSLATE    = -0;

    MATRIX_PUSH = 0;

    gluPerspective(74, 256.0f / 192.0f, 0.002f, 1000);

    glRotatef(-TRUEANGLES(viewpitch) + 90, 1.0f, 0.0f, 0.0f);
    glRotatef(-TRUEANGLES(viewangle) + 90, 0.0f, 1.0f, 0.0f);

    MATRIX_SCALE        =  0x1000;
    MATRIX_SCALE        =  0x1000;
    MATRIX_SCALE        = -0x1000;
    MATRIX_TRANSLATE    = -F2DSFIXED(viewx);
    MATRIX_TRANSLATE    = -F2DSFIXED(viewz);
    MATRIX_TRANSLATE    = -F2DSFIXED(viewy);

    D_IncValidCount();
    D_UpdateTiccmd();

    R_RenderView();
    R_DrawScene();

    MATRIX_POP = GFX_MTX_STACK_LEVEL;

    for(psp = player->psprites; psp < &player->psprites[NUMPSPRITES]; psp++)
    {
        if(psp->state && player->cameratarget == player->mo)
            R_DrawPSprite(psp, player->mo->subsector->sector, player);
    }
}

