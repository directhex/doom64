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

    an = (ANG180 + ANG45);//R_FrustumAngle();

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
    gfxtextures = (dtexture*)Z_Malloc(sizeof(dtexture) * numtextures, PU_STATIC, NULL);

    memset(gfxtextures, -1, sizeof(dtexture) * numtextures);
}

//
// R_InitSprites
//

static void R_InitSprites(void)
{
    s_start     = W_GetNumForName("S_START") + 1;
    s_end       = W_GetNumForName("S_END") - 1;
    numsprites  = (s_end - s_start) + 1;
    gfxsprites  = (dtexture*)Z_Malloc(sizeof(dtexture) * numsprites, PU_STATIC, NULL);

    memset(gfxsprites, -1, sizeof(dtexture) * numsprites);
}

//
// R_GetTextureSize
//

int R_GetTextureSize(int size)
{
    if(size == 8)
        return TEXTURE_SIZE_8;
    if(size == 16)
        return TEXTURE_SIZE_16;
    if(size == 32)
        return TEXTURE_SIZE_32;
    if(size == 64)
        return TEXTURE_SIZE_64;
    if(size == 128)
        return TEXTURE_SIZE_128;
    if(size == 256)
        return TEXTURE_SIZE_256;

	return 0;
}

//
// R_LoadTexture
//

int R_LoadTexture(dtexture texture)
{
    short* gfx;
    int i;
    int w;
    int h;
    byte* data;
    byte* pal;
    uint16 paldata[16];
    dboolean ok;

    gfx = (short*)W_CacheLumpNum(t_start + texture, PU_CACHE);
    w = gfx[0];
    h = gfx[1];
    data = (byte*)(gfx + 4);
    pal = (byte*)(gfx + 4 + (((w * h) >> 1) >> 1));

    for(i = 0; i < 16; i++)
    {
        paldata[i] = RGB8(pal[0], pal[1], pal[2]);
        pal += 4;
    }

    glGenTextures(1, &gfxtextures[texture]);
    glBindTexture(0, gfxtextures[texture]);
    ok = glTexImage2D(
        0,
        0,
        GL_RGB16,
        R_GetTextureSize(w),
        R_GetTextureSize(h),
        0,
        TEXGEN_OFF|GL_TEXTURE_WRAP_S|GL_TEXTURE_WRAP_T,
        data);

    glColorTableEXT(0, 0, 16, 0, 0, paldata);

    return ok;
}

//
// R_PrecacheLevel
// Loads and binds all world textures before level startup
//

void R_PrecacheLevel(void)
{
    char *texturepresent;
    int	i;;
    
    glResetTextures();

    texturepresent = (char*)Z_Alloca(numtextures);
    
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

        if(sectors[i].flags & MS_LIQUIDFLOOR)
            texturepresent[sectors[i].floorpic + 1] = 1;
    }
    
    for(i = 0; i < numtextures; i++)
    {
        if(texturepresent[i])
        {
            R_LoadTexture(i);
        }
    }
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
    int i;
    int f;

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

    glRotatef(-TRUEANGLES(viewangle) + 90, 0.0f, 1.0f, 0.0f);

    MATRIX_SCALE        =  0x1000;
    MATRIX_SCALE        =  0x1000;
    MATRIX_SCALE        = -0x1000;
    MATRIX_TRANSLATE    = -F2INT(viewx);
    MATRIX_TRANSLATE    = -F2INT(viewz);
    MATRIX_TRANSLATE    = -F2INT(viewy);

    D_IncValidCount();
    D_UpdateTiccmd();

    R_RenderView();

    GFX_CONTROL = (GFX_CONTROL & 0xF0FF) | 0x400;
    GFX_FOG_COLOR = sky->fogcolor;

    for(i = 0; i < 16; i++)
        GFX_FOG_TABLE[i] = 0;

    for(i = 16, f = 0; i < 32; i++)
        GFX_FOG_TABLE[i] = f++ * 6;

    GFX_FOG_TABLE[31] = 0x7F;
    GFX_FOG_OFFSET = 0x7777;

    R_DrawScene();
}

