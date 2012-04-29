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
rcolor          flashcolor;
fixed_t         viewsin[2];
fixed_t         viewcos[2];

// sprite info globals
spritedef_t     *spriteinfo;
spriteframe_t   sprtemp[29];
int             maxframe;
char*           spritename;

// gfx texture globals
dtexture        *gfxtextures;
int             t_start;
int             t_end;
int             numtextures;
int             *gfx_tex_prevtic;

// gfx sprite globals
dtexture        *gfxsprites;
int             s_start;
int             s_end;
int             numgfxsprites;
int             *gfx_spr_prevtic;
int             *spritepalindex;
short           *spriteoffset;
short           *spritetopoffset;
short           *spritewidth;
short           *spriteheight;

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
// R_InitTextures
//

static void R_InitTextures(void)
{
    t_start         = W_GetNumForName("T_START") + 1;
    t_end           = W_GetNumForName("T_END") - 1;
    numtextures     = (t_end - t_start) + 1;
    gfxtextures     = (dtexture*)Z_Malloc(sizeof(dtexture) * numtextures, PU_STATIC, NULL);
    gfx_tex_prevtic = (int*)Z_Calloc(sizeof(int) * numtextures, PU_STATIC, NULL);

    memset(gfxtextures, -1, sizeof(dtexture) * numtextures);
}

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//

static void R_InstallSpriteLump(int lump, unsigned frame, unsigned rotation, dboolean flipped)
{
    int	r;
    
    if(frame >= 29 || rotation > 8)
        I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);
    
    if((int)frame > maxframe)
        maxframe = frame;
    
    if(rotation == 0)
    {
        // the lump should be used for all rotations
        if((sprtemp[frame].rotate == false))
            I_Error("R_InitSprites: Sprite %s frame %c has multiple rot=0 lump", spritename, 'A'+frame);
        
        if(sprtemp[frame].rotate == true)
            I_Error("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump", spritename, 'A'+frame);
        
        sprtemp[frame].rotate = false;
        for(r = 0; r < 8; r++)
        {
            sprtemp[frame].lump[r] = lump - s_start;
            sprtemp[frame].flip[r] = (byte)flipped;
        }
        return;
    }
    
    // the lump is only used for one rotation
    if(sprtemp[frame].rotate == false)
        I_Error("R_InitSprites: Sprite %s frame %c has rotations and a rot=0 lump", spritename, 'A'+frame);
    
    sprtemp[frame].rotate = true;
    
    // make 0 based
    rotation--;
    if((sprtemp[frame].lump[rotation] != -1))
        I_Error ("R_InitSprites: Sprite %s : %c : %c has two lumps mapped to it",
        spritename, 'A'+frame, '1'+rotation);
    
    sprtemp[frame].lump[rotation] = lump - s_start;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}

//
// R_InitSprites
//

static void R_InitSprites(void)
{
    char**  check;
    int     i;
    int     l;
    int     frame;
    int     rotation;
    int     start;
    int     end;
    int     patched;
    int     sprnum;

    s_start         = W_GetNumForName("S_START") + 1;
    s_end           = W_GetNumForName("S_END") - 1;
    numgfxsprites   = (s_end - s_start) + 1;
    gfxsprites      = (dtexture*)Z_Malloc(sizeof(dtexture) * numgfxsprites, PU_STATIC, NULL);
    gfx_spr_prevtic = (int*)Z_Calloc(sizeof(int) * numgfxsprites, PU_STATIC, NULL);
    spritewidth     = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spriteheight    = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spriteoffset    = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spritetopoffset = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spritepalindex  = (int*)Z_Calloc(sizeof(int) * NUMSPRITES, PU_STATIC, NULL);

    memset(gfxsprites, -1, sizeof(dtexture) * numgfxsprites);

    // count the number of sprite names
    check = sprnames;
    while(*check != NULL)
        check++;
    
    sprnum = check-sprnames;
    
    if(!sprnum)
        return;
    
    spriteinfo = Z_Malloc(sprnum * sizeof(*spriteinfo), PU_STATIC, NULL);
    
    start = s_start - 1;
    end = s_end + 1;
    
    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    
    for(i = 0; i < sprnum; i++)
    {
        spritename = sprnames[i];
        memset(sprtemp,-1, sizeof(sprtemp));
        
        maxframe = -1;
        
        // scan the lumps,
        //  filling in the frames for whatever is found
        
        for(l = start + 1; l < end; l++)
        {
            // 20120422 villsa - gcc is such a crybaby sometimes...
            if(!strncmp(lumpinfo[l].name, sprnames[i], 4))
            {
                frame = lumpinfo[l].name[4] - 'A';
                rotation = lumpinfo[l].name[5] - '0';
                
                patched = l;
                
                R_InstallSpriteLump(patched, frame, rotation, false);
                
                if(lumpinfo[l].name[6])
                {
                    frame = lumpinfo[l].name[6] - 'A';
                    rotation = lumpinfo[l].name[7] - '0';
                    R_InstallSpriteLump (l, frame, rotation, true);
                }
            }
        }
        
        // check the frames that were found for completeness
        if (maxframe == -1)
        {
            spriteinfo[i].numframes = 0;
            continue;
        }
        
        maxframe++;
        
        for(frame = 0; frame < maxframe; frame++)
        {
            switch((int)sprtemp[frame].rotate)
            {
            case -1:
                // no rotations were found for that frame at all
                I_Error ("R_InitSprites: No patches found for %s frame %c", sprnames[i], frame+'A');
                break;
                
            case 0:
                // only the first rotation is needed
                break;
                
            case 1:
                // must have all 8 frames
                for(rotation = 0; rotation < 8; rotation++)
                    if (sprtemp[frame].lump[rotation] == -1)
                        I_Error ("R_InitSprites: Sprite %s frame %c is missing rotations",
                        sprnames[i], frame+'A');
                    break;
            }
        }
        
        // allocate space for the frames present and copy sprtemp to it
        spriteinfo[i].numframes = maxframe;
        spriteinfo[i].spriteframes =
            Z_Malloc(maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
        memcpy(spriteinfo[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }
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
// R_PadTextureDims
//

#define MAXTEXSIZE	256
#define MINTEXSIZE	8

int R_PadTextureDims(int n)
{
    int mask = MINTEXSIZE;
    
    while(mask < MAXTEXSIZE)
    {
        if(n == mask || (n & (mask-1)) == n)
            return mask;
        
        mask <<= 1;
    }
    return n;
}

//
// R_PadTexture
//

static byte gfx_padbuffer[256 * 256];
static byte* R_PadTexture(byte* in, int width, int height,
                          int newwidth, int newheight, dboolean rgb256)
{
    byte* out = NULL;
    int row;
    unsigned int size;
    unsigned int colsize;

    size = (newwidth * height);
    colsize = width;

    if(!rgb256)
    {
        size >>= 1;
        colsize >>= 1;
    }

    out = (byte*)gfx_padbuffer;

    for(row = 0; row < height; row++)
    {
        byte* d1;
        byte* d2;
        unsigned int stride1;
        unsigned int stride2;

        stride1 = (row * newwidth);
        stride2 = (row * width);

        if(!rgb256)
        {
            stride1 >>= 1;
            stride2 >>= 1;
        }

        d1 = out + stride1;
        d2 = in + stride2;
        memset(d1, 0, newwidth);
        memcpy(d1, d2, colsize);
    }

    return out;
}

//
// R_FlushTextures
//

void R_FlushTextures(void)
{
    int i;

    for(i = 0; i < numtextures; i++)
    {
        if(gfxtextures[i] == -1)
            continue;

        if((gametic - gfx_tex_prevtic[i]) >= 8)
        {
            glDeleteTextures(1, &gfxtextures[i]);
            gfxtextures[i] = -1;
        }
    }
}

//
// R_LoadTexture
//

void R_LoadTexture(dtexture texture)
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

    ok = glGenTextures(1, &gfxtextures[texture]);
    glBindTexture(0, gfxtextures[texture]);

    if(!ok)
        I_Printf("%i missed\n", texture);

    swiWaitForVBlank();

    if(!glTexImage2D(
        0,
        0,
        GL_RGB16,
        R_GetTextureSize(w),
        R_GetTextureSize(h),
        0,
        TEXGEN_OFF|GL_TEXTURE_WRAP_S|GL_TEXTURE_WRAP_T,
        data))
    {
        I_Printf("making room for %i\n", texture);
        R_FlushTextures();

        if(!glTexImage2D(
            0,
            0,
            GL_RGB16,
            R_GetTextureSize(w),
            R_GetTextureSize(h),
            0,
            TEXGEN_OFF|GL_TEXTURE_WRAP_S|GL_TEXTURE_WRAP_T,
            data))
        {
            gfxtextures[texture] = -1;

            //I_Error("R_LoadTexture: Texture lump cache overflow - %i", t_start + texture);
            //GFX_TEX_FORMAT = 0;
            //GFX_PAL_FORMAT = 0;
            return;
        }
    }

    glColorTableEXT(0, 0, 16, 0, 0, paldata);
}

//
// R_FlushSprites
//

static void R_FlushSprites(void)
{
    int i;

    for(i = 0; i < numgfxsprites; i++)
    {
        if(gfxsprites[i] == -1)
            continue;

        if((gametic - gfx_spr_prevtic[i]) >= 2)
        {
            glDeleteTextures(1, &gfxsprites[i]);
            gfxsprites[i] = -1;
        }
    }
}

//
// R_FreeSprite
//

static void R_FreeSprite(int size)
{
    int i;
    gl_texture_data* tex;

    for(i = 0; i < numgfxsprites; i++)
    {
        if(gfxsprites[i] == -1)
            continue;

        tex = R_GetTexturePointer(gfxsprites[i]);

        if(size < tex->texSize)
            continue;

        if(gfx_spr_prevtic[i] < gametic)
        {
            glDeleteTextures(1, &gfxsprites[i]);
            gfxsprites[i] = -1;

            return;
        }
    }
}

//
// R_LoadSprite
//

dboolean flushsprites = false;

dboolean R_LoadSprite(int sprite, int frame, int rotation,
                  int *x, int *y, int *w, int *h)
{
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int spritenum;

    sprdef      = &spriteinfo[sprite];
    sprframe    = &sprdef->spriteframes[frame];
    spritenum   = sprframe->lump[rotation];

    if(gfxsprites[spritenum] == -1)
    {
        short* gfx;
        int i;
        int width;
        int height;
        int pw;
        int ph;
        byte* data;
        byte* pal = NULL;
        uint16 paldata[256];
        dboolean ext;
        int numpal;
        byte* out = NULL;

        gfx                         = (short*)W_CacheLumpNum(s_start + spritenum, PU_CACHE);
        width                       = gfx[0];
        height                      = gfx[1];
        pw                          = R_PadTextureDims(width);
        ph                          = R_PadTextureDims(height);
        spritewidth[spritenum]      = width;
        spriteheight[spritenum]     = height;
        spriteoffset[spritenum]     = gfx[2];
        spritetopoffset[spritenum]  = gfx[3];
        ext                         = gfx[4];
        data                        = (byte*)(gfx + 5);
        numpal                      = 16;

        out = (byte*)R_PadTexture(data, width, height, pw, ph, ext);

        if(!ext)
        {
            pal = (byte*)(gfx + 5 + (((width * height) >> 1) >> 1));
            numpal = 16;
        }
        else if(ext == 2)
        {
            pal = (byte*)(gfx + 5 + ((width * height) >> 1));
            numpal = 256;
        }
        else
        {
            char palname[9];

            sprintf(palname, "PAL%s0", sprnames[sprite]);
            pal = (byte*)W_CacheLumpName(palname, PU_CACHE);
            numpal = 256;
        }

        for(i = 0; i < numpal; i++)
        {
            paldata[i] = RGB8(pal[0], pal[1], pal[2]);
            pal += (ext == 1) ? 3 : 4;
        }

        glGenTextures(1, &gfxsprites[spritenum]);
        glBindTexture(0, gfxsprites[spritenum]);

        swiWaitForVBlank();

        if(!glTexImage2D(
            0,
            0,
            ext ? GL_RGB256 : GL_RGB16,
            R_GetTextureSize(pw),
            R_GetTextureSize(ph),
            0,
            TEXGEN_OFF|GL_TEXTURE_COLOR0_TRANSPARENT,
            out))
        {
            R_FreeSprite(pw * ph);
            //I_Error("R_LoadSprite: Sprite lump cache overflow - %i", s_start + spritenum);
            if(!glTexImage2D(
                0,
                0,
                ext ? GL_RGB256 : GL_RGB16,
                R_GetTextureSize(pw),
                R_GetTextureSize(ph),
                0,
                TEXGEN_OFF|GL_TEXTURE_COLOR0_TRANSPARENT,
                out))
            {
                flushsprites = true;
                return false;
            }
        }

        /*if(ext)
        {
            if(!spritepalindex[sprite])
            {
                glColorTableEXT(0, 0, numpal, 0, 0, paldata);
                glGetColorTableParameterEXT(0, GL_COLOR_TABLE_FORMAT_EXT, &spritepalindex[sprite]);
            }
            else
                GFX_PAL_FORMAT = spritepalindex[sprite];
        }
        else*/
            glColorTableEXT(0, 0, numpal, 0, 0, paldata);
    }
    
    glBindTexture(0, gfxsprites[spritenum]);

    if(spritepalindex[sprite])
        GFX_PAL_FORMAT = spritepalindex[sprite];

    if(x) *x = spriteoffset[spritenum];
    if(y) *y = spritetopoffset[spritenum];
    if(w) *w = spritewidth[spritenum];
    if(h) *h = spriteheight[spritenum];

    gfx_spr_prevtic[spritenum] = gametic;

    return true;
}

//
// R_PrecacheLevel
// Loads and binds all world textures before level startup
//

void R_PrecacheLevel(void)
{
    int	i;
    mobj_t* mo;

    glResetTextures();

    memset(gfx_tex_prevtic, 0, sizeof(int) * numtextures);
    memset(gfx_spr_prevtic, 0, sizeof(int) * numgfxsprites);

    for(i = 0; i < numsides; i++)
    {
        W_CacheLumpNum(t_start + sides[i].toptexture, PU_CACHE);
        W_CacheLumpNum(t_start + sides[i].bottomtexture, PU_CACHE);
        W_CacheLumpNum(t_start + sides[i].midtexture, PU_CACHE);
    }

    for(i = 0; i < numsectors; i++)
    {
        W_CacheLumpNum(t_start + sectors[i].ceilingpic, PU_CACHE);
        W_CacheLumpNum(t_start + sectors[i].floorpic, PU_CACHE);
    }

    for(mo = mobjhead.next; mo != &mobjhead; mo = mo->next)
    {
        int j;

        if(!mo->sprite)
            continue;

        spritedef_t	*sprdef;
        sprdef = &spriteinfo[mo->sprite];

        for(j = 0; j < sprdef->numframes; j++)
        {
            spriteframe_t *sprframe;
            int p;

            sprframe = &sprdef->spriteframes[j];
            if(sprframe->rotate)
            {
                for(p = 0; p < 8; p++)
                    W_CacheLumpNum(s_start + sprframe->lump[p], PU_CACHE);
            }
            else
                W_CacheLumpNum(s_start + sprframe->lump[0], PU_CACHE);
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

    glPushMatrix();

    MATRIX_TRANSLATE    = -(16 << 4);
    MATRIX_TRANSLATE    = -0;
    MATRIX_TRANSLATE    = -0;

    glPushMatrix();

    gluPerspective(74, 256.0f / 192.0f, 0.002f, 1000);

    glRotatef(-TRUEANGLES(viewpitch) + 90, 1.0f, 0.0f, 0.0f);
    glRotatef(-TRUEANGLES(viewangle) + 90, 0.0f, 1.0f, 0.0f);

    MATRIX_SCALE        =  0x10000;
    MATRIX_SCALE        =  0x10000;
    MATRIX_SCALE        = -0x10000;
    MATRIX_TRANSLATE    = -F2INT(viewx);
    MATRIX_TRANSLATE    = -F2INT(viewz);
    MATRIX_TRANSLATE    = -F2INT(viewy);

    D_IncValidCount();
    D_UpdateTiccmd();

    flushsprites = false;

    R_RenderView();
    R_DrawScene();

    psp = &player->psprites[ps_weapon];
    for(psp = player->psprites; psp < &player->psprites[NUMPSPRITES]; psp++)
    {
        if(psp->state)
            R_DrawPSprite(psp, player->mo->subsector->sector, player);
    }

    glPopMatrix(2);

    swiWaitForVBlank();
    GFX_FLUSH = 0;

    if(flushsprites)
        R_FlushSprites();
}

