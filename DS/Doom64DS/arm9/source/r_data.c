#include "m_fixed.h"
#include "r_local.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "st_main.h"

uint32* gfx_base = (uint32*)VRAM_A;
byte    gfx_tex_buffer[GFX_BUFFER_SIZE];

static int              gfx_texpal_stride = 0;
static uint32*          gfx_tex_params;
static uint32*          gfx_texpal_params;
static uint32*          gfx_spr_params;
static uint32*          gfx_sprpal_params;
static uint32           gfx_extpal_params[NUMSPRITES][8];
static byte**           gfx_tex_cache;
static byte**           gfx_spr_cache;
static vramblock_t**    gfx_tex_blocks;
static vramblock_t**    gfx_spr_blocks;
static byte             gfx_padbuffer[256 * 256];

//
// R_CachePalette
// Returns the palette parameter for generic texture lumps
//

uint32 R_CachePalette(const char* name)
{
    uint16* lump;
    uint16* pal;
    int size;
    int stride;
    int width;
    int height;

    stride = gfx_texpal_stride;
    lump = (uint16*)W_CacheLumpName(name, PU_AUTO);
    width = lump[0];
    height = lump[1];
    size = (256 << 1);
    pal = lump + 4 + ((width * height) >> 1);

    gfx_texpal_stride += size;

    return I_SetPalette(pal, stride, size);
}

//
// R_CopyPic
//

byte* R_CopyPic(byte* pic, int x, int y, int rows, int colsize,
               int copysize, int mainwidth)
{
    int i;

    memset(gfx_padbuffer, 0, R_PadTextureDims(rows) * colsize);

    for(i = 0; i < rows; i++)
    {
        byte* src;
        byte* dst;

        src = &pic[((i + y) * mainwidth) + x];
        dst = &gfx_padbuffer[i * colsize];

        memcpy(dst, src, copysize);
    }

    return gfx_padbuffer;
}

//
// R_CacheTexture
//

static void R_CacheTexturePalette(int index)
{
    byte* gfx;
    byte* pal;
    int size;
    uint16* paldata;

    gfx = (byte*)W_CacheLumpNum(t_start + index, PU_STATIC);
    size = (((8 << gfx[0]) * (8 << gfx[1])) >> 1);
    pal = (gfx + 4 + size);
    size = (16 << 1);
    paldata = (uint16*)pal;

    gfx_texpal_params[index] = I_SetPalette(paldata, gfx_texpal_stride, size);
    gfx_texpal_stride += size;

    Z_Free(gfx);
}

//
// R_SetupSpriteData
//

static void R_SetupSpriteData(int spritenum)
{
    short* gfx;
    byte* data;
    int size;
    byte* pal;
    uint16* paldata;

    gfx = (short*)W_CacheLumpNum(s_start + spritenum, PU_STATIC);

    spritewidth[spritenum]      = gfx[0];
    spriteheight[spritenum]     = gfx[1];
    spriteoffset[spritenum]     = gfx[2];
    spritetopoffset[spritenum]  = gfx[3];

    if(!gfx[4])
    {
        data = (byte*)(gfx + 5);

        pal = data + ((gfx[0] * gfx[1]) >> 1);
        size = (16 << 1);
        paldata = (uint16*)pal;

        gfx_sprpal_params[spritenum] = I_SetPalette(paldata, gfx_texpal_stride, size);
        gfx_texpal_stride += size;
    }
    else
        spritetiles[spritenum] = gfx[5];

    Z_Free(gfx);
}

//
// R_CacheExternalPalette
//

static void R_CacheExternalPalette(int sprite)
{
    char palname[9];
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int spritenum;
    short* gfx;
    byte* data;
    int i;

    sprdef = &spriteinfo[sprite];
    sprframe = &sprdef->spriteframes[0];
    spritenum = sprframe->lump[0];

    if(gfx_sprpal_params[spritenum] == 0)
    {
        gfx = (short*)W_CacheLumpNum(s_start + spritenum, PU_STATIC);

        if(gfx[4] == 2)
        {
            byte* pal;
            uint16* paldata;
            int size;

            data = (byte*)(gfx + 6 + ((sizeof(short) * gfx[5]) >> 1));

            pal = data + (gfx[0] * gfx[1]);
            size = (256 << 1);
            paldata = (uint16*)pal;

            gfx_extpal_params[sprite][0] = I_SetPalette(paldata, gfx_texpal_stride, size);
            gfx_texpal_stride += size;
        }
        else if(gfx[4] == 1)
        {
            for(i = 0; i < 8; i++)
            {
                int lump;
                uint16* paldata;
                int size;

                sprintf(palname, "PAL%s%i", sprnames[sprite], i);
                lump = W_CheckNumForName(palname);

                if(lump == -1)
                    continue;

                paldata = (uint16*)W_CacheLumpNum(lump, PU_STATIC);
                size = (256 << 1);

                gfx_extpal_params[sprite][i] = I_SetPalette(paldata, gfx_texpal_stride, size);
                gfx_texpal_stride += size;
            }
        }

        Z_Free(gfx);
    }
}

//
// R_InitPalettes
//

static void R_InitPalettes(void)
{
    int i;

    vramSetBankE(VRAM_E_LCD);

    for(i = 0; i < numtextures; i++)
    {
        if(gfx_texpal_params[i] == 0)
            R_CacheTexturePalette(i);
    }

    for(i = 0; i < numgfxsprites; i++)
    {
        if(gfx_sprpal_params[i] == 0)
            R_SetupSpriteData(i);
    }

    for(i = 1; i < NUMSPRITES; i++)
        R_CacheExternalPalette(i);

    ST_CachePalettes();

    vramSetBankE(VRAM_E_TEX_PALETTE);

    if(gfx_texpal_stride >= 0x10000)
        I_Error("R_InitPalettes: palette cache overflowed by %d",
        gfx_texpal_stride - 0x10000);
}

//
// R_InitTextures
//

static void R_InitTextures(void)
{
    t_start             = W_GetNumForName("T_START") + 1;
    t_end               = W_GetNumForName("T_END") - 1;
    swx_start           = W_FindNumForName("SWX") + 1;
    numtextures         = (t_end - t_start) + 1;
    gfx_tex_params      = (uint32*)Z_Calloc(sizeof(uint32) * numtextures, PU_STATIC, NULL);
    gfx_texpal_params   = (uint32*)Z_Calloc(sizeof(uint32) * numtextures, PU_STATIC, NULL);
    gfx_tex_cache       = (byte**)Z_Calloc(sizeof(*gfx_tex_cache) * numtextures, PU_STATIC, NULL);
    gfx_tex_blocks      = (vramblock_t**)Z_Calloc(sizeof(*gfx_tex_blocks) * numtextures, PU_STATIC, NULL);
}

//
// R_InstallSpriteLump
// Local function for R_InitSprites.
//

static int              maxframe;
static spriteframe_t    sprtemp[29];
static char*            spritename;

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

    s_start             = W_GetNumForName("S_START") + 1;
    s_end               = W_GetNumForName("S_END") - 1;
    numgfxsprites       = (s_end - s_start) + 1;
    spritewidth         = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spriteheight        = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spriteoffset        = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    spritetopoffset     = (short*)Z_Calloc(sizeof(short) * numgfxsprites, PU_STATIC, NULL);
    gfx_spr_params      = (uint32*)Z_Calloc(sizeof(uint32) * numgfxsprites, PU_STATIC, NULL);
    gfx_sprpal_params   = (uint32*)Z_Calloc(sizeof(uint32) * numgfxsprites, PU_STATIC, NULL);
    gfx_spr_cache       = (byte**)Z_Calloc(sizeof(*gfx_spr_cache) * numgfxsprites, PU_STATIC, NULL);
    spritetiles         = (byte*)Z_Calloc(numgfxsprites, PU_STATIC, NULL);
    spritetilelist      = (short**)Z_Calloc(sizeof(*spritetilelist) * numgfxsprites, PU_STATIC, NULL);
    gfx_spr_blocks      = (vramblock_t**)Z_Calloc(sizeof(*gfx_spr_blocks) * numgfxsprites, PU_STATIC, NULL);

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
// R_InitData
//

void R_InitData(void)
{
    R_InitTextures();
    R_InitSprites();
    R_InitPalettes();
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
    
    while(mask <= MAXTEXSIZE)
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
// R_LoadTexture
//

void R_LoadTexture(dtexture texture, dboolean flip_s, dboolean flip_t)
{
    byte* gfx;
    int w, dw;
    int h, dh;
    byte* data;
    short* pal;
    uint32 flags;
    int size;

    if(gfx_tex_cache[texture] == NULL)
        gfx_tex_cache[texture] = (byte*)W_CacheLumpNum(t_start + texture, PU_CACHE);

    gfx = gfx_tex_cache[texture];
        
    dw = gfx[0];
    dh = gfx[1];
    w = (8 << dw);
    h = (8 << dh);
    data = gfx + 4;
    size = ((w * h) >> 1);
    pal = (short*)(gfx + 4 + size);

    flags = TEXGEN_OFF|GL_TEXTURE_WRAP_S|GL_TEXTURE_WRAP_T;
        
    if(pal[0] == 0)
        flags |= GL_TEXTURE_COLOR0_TRANSPARENT;

    if(flip_s)
        flags |= GL_TEXTURE_FLIP_S;

    if(flip_t)
        flags |= GL_TEXTURE_FLIP_T;

    if(!I_AllocVBlock(gfx_tex_params, gfx_tex_blocks, data, texture, size, flags, dw, dh, GL_RGB16))
    {
        GFX_TEX_FORMAT = 0;
        GFX_PAL_FORMAT = 0;
        return;
    }

    GFX_TEX_FORMAT = gfx_tex_params[texture];
    GFX_PAL_FORMAT = gfx_texpal_params[texture];
}

//
// R_LoadSprite
//

dboolean R_LoadSprite(int sprite, int frame, int rotation, int palindex,
                  int *x, int *y, int *w, int *h)
{
    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int spritenum;
    short* gfx;
    int width;
    int height;
    int pw;
    int ph;
    dboolean ext;
    byte* data;
    int size;

    if(sprite == SPR_SPOT)
        return false;

    sprdef      = &spriteinfo[sprite];
    sprframe    = &sprdef->spriteframes[frame];
    spritenum   = sprframe->lump[rotation];

    if(gfx_spr_cache[spritenum] == NULL)
        gfx_spr_cache[spritenum] = (byte*)W_CacheLumpNum(s_start + spritenum, PU_CACHE);

    gfx     = (short*)gfx_spr_cache[spritenum];
    width   = gfx[0];
    height  = gfx[1];
    pw      = R_PadTextureDims(width);
    ph      = R_PadTextureDims(height);
    ext     = gfx[4];
    size    = (pw * height);

    if(ext)
        data = (byte*)(gfx + 6 + ((sizeof(short) * spritetiles[spritenum]) >> 1));
    else
        data = (byte*)(gfx + 5);

    if(!ext)
        size >>= 1;

    if(gfx_spr_params[spritenum] == 0)
    {
        byte* out = (byte*)R_PadTexture(data, width, height, pw, ph, ext);
        data = out;
    }

    if(!I_AllocVBlock(
        gfx_spr_params,
        gfx_spr_blocks,
        data,
        spritenum,
        size,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
        R_GetTextureSize(pw),
        R_GetTextureSize(ph),
        ext ? GL_RGB256 : GL_RGB16))
    {
        GFX_TEX_FORMAT = 0;
        GFX_PAL_FORMAT = 0;
        return false;
    }

    GFX_TEX_FORMAT = gfx_spr_params[spritenum];

    if(palindex >= 8 || palindex < 0)
        palindex = 0;

    if(ext)
        GFX_PAL_FORMAT = gfx_extpal_params[sprite][palindex];
    else
        GFX_PAL_FORMAT = gfx_sprpal_params[spritenum];

    if(x) *x = spriteoffset[spritenum];
    if(y) *y = spritetopoffset[spritenum];
    if(w) *w = spritewidth[spritenum];
    if(h) *h = spriteheight[spritenum];

    return true;
}

//
// R_PrecacheLevel
// Loads and binds all world textures before level startup
//

void R_PrecacheLevel(void)
{
    int	i;

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
}

