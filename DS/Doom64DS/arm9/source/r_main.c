#include <math.h>

#include "m_fixed.h"
#include "tables.h"
#include "r_local.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "d_main.h"

fixed_t viewx;
fixed_t viewy;
fixed_t viewz;
angle_t viewangle;
angle_t viewpitch;
fixed_t quakeviewx;
fixed_t quakeviewy;
angle_t viewangleoffset;
rcolor  flashcolor;
fixed_t viewsin[2];
fixed_t viewcos[2];

dtexture *gfxtextures;
int t_start;
int t_end;
int numtextures;

dtexture *gfxsprites;
int s_start;
int s_end;
int numsprites;

int skyflatnum = -1;

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
        side = _R_PointOnSide (x, y, node);
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

    an = R_FrustumAngle();

    R_Clipper_Clear();
    R_Clipper_SafeAddClipRange(viewangle + an, viewangle - an);
    R_RenderBSPNode(numnodes - 1);
    // TODO - clip sprites
}

//
// R_InitTextures
//

static void R_InitTextures(void)
{
    t_start     = W_GetNumForName("T_START") + 1;
    t_end       = W_GetNumForName("T_END") - 1;
    numtextures = (t_end - t_start) + 1;
    gfxtextures = (dtexture*)Z_Calloc(sizeof(dtexture) * numtextures, PU_STATIC, NULL);
}

//
// R_InitSprites
//

static void R_InitSprites(void)
{
    s_start     = W_GetNumForName("S_START") + 1;
    s_end       = W_GetNumForName("S_END") - 1;
    numsprites  = (s_end - s_start) + 1;
    gfxsprites  = (dtexture*)Z_Calloc(sizeof(dtexture) * numsprites, PU_STATIC, NULL);
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
    R_InitSprites();
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

    viewsin[0]  = dsin(viewangle);
    viewsin[1]  = dsin(viewpitch - ANG90);
    viewcos[0]  = dcos(viewangle);
    viewcos[1]  = dcos(viewpitch - ANG90);

    MATRIX_CONTROL      = GL_PROJECTION;
    MATRIX_IDENTITY     = 0;

    gluPerspective(74, 256.0f / 192.0f, 0.002f, 1000);

    MATRIX_CONTROL      = GL_MODELVIEW;
    MATRIX_IDENTITY     = 0;

    glRotatef(-TRUEANGLES(viewangle) - 90, 0.0f, 1.0f, 0.0f);

    MATRIX_SCALE        =  0x1000;
    MATRIX_SCALE        =  0x1000;
    MATRIX_SCALE        = -0x1000;
    MATRIX_TRANSLATE    = -F2INT(viewx);
    MATRIX_TRANSLATE    = -F2INT(viewz);
    MATRIX_TRANSLATE    = -F2INT(viewy);

    D_IncValidCount();
    D_UpdateTiccmd();

    R_RenderView();

    // TODO - fog

    R_DrawScene();
}

