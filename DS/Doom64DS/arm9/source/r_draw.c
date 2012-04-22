#include "m_fixed.h"
#include "tables.h"
#include "r_local.h"
#include "z_zone.h"
#include "w_wad.h"

int         skypicnum = -1;
int         skybackdropnum = -1;
int         skyflatnum = -1;
skydef_t*   sky;
int         thunderCounter = 0;
int         lightningCounter = 0;
int         thundertic = 1;
dboolean    skyfadeback = false;

//
// R_DrawSimpleSky
//

static dtexture gfxskypic = -1;

void R_DrawSimpleSky(void)
{
    if(gfxskypic == -1)
    {
        short* gfx;
        int i;
        int w;
        int h;
        byte* data;
        byte* pal;
        uint16 paldata[256];

        gfx = (short*)W_CacheLumpNum(skypicnum, PU_CACHE);
        w = gfx[0];
        h = gfx[1];
        data = (byte*)(gfx + 4);
        pal = (byte*)(gfx + 4 + ((w * h) >> 1));

        for(i = 0; i < 256; i++)
        {
            paldata[i] = RGB8(pal[0], pal[1], pal[2]);
            pal += 3;
        }

        glGenTextures(1, &gfxskypic);
        glBindTexture(0, gfxskypic);
        glTexImage2D(
            0,
            0,
            GL_RGB256,
            R_GetTextureSize(w),
            R_GetTextureSize(h),
            0,
            TEXGEN_OFF|GL_TEXTURE_WRAP_S|GL_TEXTURE_WRAP_T,
            data);

        glColorTableEXT(0, 0, 256, 0, 0, paldata);
    }
    else
        glBindTexture(0, gfxskypic);

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_NONE | POLY_MODULATION;
    GFX_COLOR       = RGB15(31, 31, 31);
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_TEX_COORD   = COORD_PACK(0, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);
    GFX_TEX_COORD   = COORD_PACK(256, 0);
    GFX_VERTEX16    = VERTEX_PACK(0x7FFF, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);
    GFX_TEX_COORD   = COORD_PACK(0, -128);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0x5FFF);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);
    GFX_TEX_COORD   = COORD_PACK(256, -128);
    GFX_VERTEX16    = VERTEX_PACK(0x7FFF, 0x5FFF);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);
}

//
// R_DrawLine
//

static void R_DrawLine(seg_t* seg, fixed_t top, fixed_t bottom,
                       dtexture texture, light_t* l1, light_t* l2,
                       fixed_t u1, fixed_t u2, fixed_t v1, fixed_t v2,
                       dboolean midsided)
{
    int x1, x2;
    int y1, y2;
    int z1, z2;
    int r1, r2;
    int g1, g2;
    int b1, b2;
    gl_texture_data *tex;

    r1 = l1->active_r;
    g1 = l1->active_g;
    b1 = l1->active_b;
    r2 = l2->active_r;
    g2 = l2->active_g;
    b2 = l2->active_b;

    if(seg->linedef->flags & ML_TWOSIDED)
    {
        int height = 0;
        int sideheight1 = 0;
        int sideheight2 = 0;

        height = frontsector->ceilingheight - frontsector->floorheight;

        if(bottom != frontsector->floorheight)
        {
            if(!(seg->linedef->flags & ML_BLENDFULLTOP))
            {
                sideheight1 = seg->backsector->ceilingheight - seg->frontsector->floorheight;
                sideheight2 = seg->frontsector->ceilingheight - seg->backsector->ceilingheight;

                r1 = F2INT(FixedMul(FixedDiv(INT2F(l1->active_r), height), sideheight1));
                g1 = F2INT(FixedMul(FixedDiv(INT2F(l1->active_g), height), sideheight1));
                b1 = F2INT(FixedMul(FixedDiv(INT2F(l1->active_b), height), sideheight1));
                r2 = F2INT(FixedMul(FixedDiv(INT2F(l2->active_r), height), sideheight2));
                g2 = F2INT(FixedMul(FixedDiv(INT2F(l2->active_g), height), sideheight2));
                b2 = F2INT(FixedMul(FixedDiv(INT2F(l2->active_b), height), sideheight2));

                r2 = MIN(r1+r2, 255);
                g2 = MIN(g1+g2, 255);
                b2 = MIN(b1+b2, 255);
                r1 = l1->active_r;
                g1 = l1->active_g;
                b1 = l1->active_b;
            }

            if(seg->linedef->flags & ML_INVERSEBLEND)
            {
                r1 = l2->active_r;
                g1 = l2->active_g;
                b1 = l2->active_b;
                r2 = l1->active_r;
                g2 = l1->active_g;
                b2 = l1->active_b;
            }
        }
        else if(top != frontsector->ceilingheight && !(seg->linedef->flags & ML_BLENDFULLBOTTOM))
        {
            sideheight1 = seg->backsector->floorheight - seg->frontsector->floorheight;
            sideheight2 = seg->frontsector->ceilingheight - seg->backsector->floorheight;

            r1 = F2INT(FixedMul(FixedDiv(INT2F(l1->active_r), height), sideheight1));
            g1 = F2INT(FixedMul(FixedDiv(INT2F(l1->active_g), height), sideheight1));
            b1 = F2INT(FixedMul(FixedDiv(INT2F(l1->active_b), height), sideheight1));
            r2 = F2INT(FixedMul(FixedDiv(INT2F(l2->active_r), height), sideheight2));
            g2 = F2INT(FixedMul(FixedDiv(INT2F(l2->active_g), height), sideheight2));
            b2 = F2INT(FixedMul(FixedDiv(INT2F(l2->active_b), height), sideheight2));

            r1 = MIN(r1+r2, 255);
            g1 = MIN(g1+g2, 255);
            b1 = MIN(b1+b2, 255);
            r2 = l2->active_r;
            g2 = l2->active_g;
            b2 = l2->active_b;
        }
    }

    x1 = F2INT(seg->v1->x);
    x2 = F2INT(seg->v2->x);
    y1 = F2INT(seg->v1->y);
    y2 = F2INT(seg->v2->y);
    z1 = F2INT(top);
    z2 = F2INT(bottom);

    if(gfxtextures[texture] == -1)
        R_LoadTexture(texture);
    else
    {
        if((tex = R_GetTexturePointer(gfxtextures[texture])))
        {
            if(glGlob->activeTexture != gfxtextures[texture])
            {
                if(seg->linedef->flags & ML_HMIRROR)
                    tex->texFormat |= GL_TEXTURE_FLIP_S;
	            else
		            tex->texFormat &= ~GL_TEXTURE_FLIP_S;

                if(seg->linedef->flags & ML_VMIRROR)
		            tex->texFormat |= GL_TEXTURE_FLIP_T;
	            else
		            tex->texFormat &= ~GL_TEXTURE_FLIP_T;

                if(seg->linedef->flags & ML_DRAWMIDTEXTURE && midsided)
                    tex->texFormat |= GL_TEXTURE_COLOR0_TRANSPARENT;
                else
                    tex->texFormat &= ~GL_TEXTURE_COLOR0_TRANSPARENT;

                GFX_TEX_FORMAT = tex->texFormat;
                glGlob->activeTexture = gfxtextures[texture];

                if(tex->palIndex)
                {
                    gl_palette_data *pal =
                        (gl_palette_data*)DynamicArrayGet(&glGlob->palettePtrs, tex->palIndex);

                    GFX_PAL_FORMAT = pal->addr;
                    glGlob->activePalette = tex->palIndex;
                }
                else
                    GFX_PAL_FORMAT = glGlob->activePalette = 0;
            }
        }
        else
            GFX_TEX_FORMAT = GFX_PAL_FORMAT = glGlob->activePalette = glGlob->activeTexture = 0;
    }

    if(nolights)
        r1 = r2 = g1 = g2 = b1 = b2 = 255;

    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_COLOR       = RGB8(r1, g1, b1);
    GFX_TEX_COORD   = COORD_PACK(F2INT(u1), F2INT(v1));
    GFX_VERTEX16    = VERTEX_PACK(x2, z1);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);
    GFX_TEX_COORD   = COORD_PACK(F2INT(u2), F2INT(v1));
    GFX_VERTEX16    = VERTEX_PACK(x1, z1);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
    GFX_COLOR       = RGB8(r2, g2, b2);
    GFX_TEX_COORD   = COORD_PACK(F2INT(u1), F2INT(v2));
    GFX_VERTEX16    = VERTEX_PACK(x2, z2);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);
    GFX_TEX_COORD   = COORD_PACK(F2INT(u2), F2INT(v2));
    GFX_VERTEX16    = VERTEX_PACK(x1, z2);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
}

//
// R_DrawSeg
//

static void R_DrawSeg(seg_t* seg)
{
    line_t*     linedef;
    side_t*     sidedef;
    fixed_t     top;
    fixed_t     bottom;
    fixed_t     btop;
    fixed_t     bbottom;
    light_t*    l1;
    light_t*    l2;
    fixed_t     col;
    fixed_t     row;
    
    linedef = seg->linedef;
    sidedef = seg->sidedef;

    if(!linedef)
        return;

    col = seg->length;
    row = sidedef->rowoffset;

    if(linedef->flags & ML_BLENDING)
    {
        l1 = &lights[frontsector->colors[LIGHT_UPRWALL]];
        l2 = &lights[frontsector->colors[LIGHT_LWRWALL]];
    }
    else
    {
        l1 = &lights[frontsector->colors[LIGHT_THING]];
        l2 = &lights[frontsector->colors[LIGHT_THING]];
    }
    
    top = frontsector->ceilingheight;
    bottom = frontsector->floorheight;
    
    if(seg->backsector)
    {
        btop = seg->backsector->ceilingheight;
        bbottom = seg->backsector->floorheight;
        
        if((frontsector->ceilingpic == skyflatnum) && (seg->backsector->ceilingpic == skyflatnum))
            btop = top;
        
        //
        // botom side seg
        //
        if(bottom < bbottom)
        {
            if(seg->sidedef[0].bottomtexture != 1)
            {
                fixed_t v1;
                fixed_t v2;

                if(linedef->flags & ML_DONTPEGBOTTOM)
                {
                    v1 = row + (top - bbottom);
					v2 = row + (top - bottom);
                }
                else
                {
                    v1 = row;
					v2 = row + (bbottom - bottom);
                }

                R_DrawLine(
                    seg,
                    bbottom,
                    bottom,
                    sidedef->bottomtexture,
                    l1,
                    l2,
                    (seg->offset + col) + sidedef->textureoffset,
                    seg->offset + sidedef->textureoffset,
                    v1,
                    v2,
                    false
                    );
            }
            
            bottom = bbottom;
        }
        
        //
        // upper side seg
        //
        if(top > btop)
        {
            if(seg->sidedef[0].toptexture != 1)
            {
                fixed_t v1;
                fixed_t v2;

                if(linedef->flags & ML_DONTPEGTOP)
                {
                    v1 = row;
					v2 = row + (top - btop);
                }
                else
                {
                    v2 = row;
					v1 = row - (top - btop);
                }

                R_DrawLine(
                    seg,
                    top,
                    btop,
                    sidedef->toptexture,
                    l1,
                    l2,
                    (seg->offset + col) + sidedef->textureoffset,
                    seg->offset + sidedef->textureoffset,
                    v1,
                    v2,
                    false
                    );
            }
            
            top = btop;
        }
    }

    //
    // middle side seg
    //
    if(sidedef->midtexture != 1)
    {
        if(seg->backsector)
        {
            if(!(linedef->flags & ML_DRAWMIDTEXTURE))
                return;

            btop = seg->backsector->ceilingheight;
            bbottom = seg->backsector->floorheight;

            if((frontsector->ceilingpic == skyflatnum) && (seg->backsector->ceilingpic == skyflatnum))
                btop = top;
        
            if(bottom < bbottom)
                bottom = bbottom;
        
            if(top > btop)
                top = btop;
        }
        
        if(!(linedef->flags & ML_SWITCHX02 && linedef->flags & ML_SWITCHX04))
        {
            R_DrawLine(
            seg,
            top,
            bottom,
            sidedef->midtexture,
            l1,
            l2,
            (seg->offset + col) + sidedef->textureoffset,
            seg->offset + sidedef->textureoffset,
            row,
            (top - bottom) + row,
            seg->linedef->flags & ML_TWOSIDED ? true : false
            );
        }
    }
}

//
// R_DrawSubsector
//

static void R_DrawSubsector(subsector_t* ss, fixed_t height, dtexture texture, light_t* light)
{
    int i;
    int x;
    int y;
    int z;
    int tx;
    int ty;
    fixed_t tsx;
    fixed_t tsy;
    int mapx;
    int mapy;
    int length;

    if(nolights)
        GFX_COLOR = 0x1F7FFF;
    else
        GFX_COLOR = RGB8(light->active_r, light->active_g, light->active_b);

    tx      = F2INT(leafs[ss->leaf].vertex->x) & 0x3F;
    ty      = F2INT(leafs[ss->leaf].vertex->y) & 0x3F;
    tsx     = leafs[ss->leaf].vertex->x;
    tsy     = leafs[ss->leaf].vertex->y;
    mapx    = 0;
    mapy    = 0;
    z       = F2INT(height);

    if(gfxtextures[texture] == -1)
        R_LoadTexture(texture);
    else
        glBindTexture(0, gfxtextures[texture]);

#define DRAWSSECT(index)                    \
    v = leafs[index].vertex;                \
    length = F2INT(tsx - v->x) + mapx;      \
    tu = length - tx;                       \
    mapx = length;                          \
    tsx = v->x;                             \
    length = F2INT(tsy - v->y) + mapy;      \
    tv = length - ty;                       \
    mapy = length;                          \
    tsy = v->y;                             \
    x = F2INT(v->x);                        \
    y = F2INT(v->y);                        \
    GFX_TEX_COORD   = COORD_PACK(tu, tv);   \
    GFX_VERTEX16    = VERTEX_PACK(x, z);    \
    GFX_VERTEX16    = VERTEX_PACK(y, 0)

    GFX_BEGIN = GL_TRIANGLES;

    for(i = 0; i < ss->numleafs - 2; i++)
    {
        fixed_t tu;
        fixed_t tv;
        vertex_t* v;

        DRAWSSECT(ss->leaf + 1 + i);
        DRAWSSECT(ss->leaf + 2 + i);
        DRAWSSECT(ss->leaf + 0);
    }

#undef DRAWSSECT
}

//
// R_DrawLeafs
//

static void R_DrawLeafs(subsector_t* subsector)
{
    int i;
    light_t* l;

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_BACK | POLY_MODULATION | POLY_FOG;

    for(i = 0; i < subsector->numleafs; i++)
    {
        seg_t* seg;

        seg = leafs[subsector->leaf + i].seg;
        if(seg->draw)
            R_DrawSeg(seg);
    }

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_BACK | POLY_MODULATION | POLY_FOG;

    if(viewz <= frontsector->ceilingheight && frontsector->ceilingpic != skyflatnum)
    {
        l = &lights[frontsector->colors[LIGHT_CEILING]];
        R_DrawSubsector(subsector, frontsector->ceilingheight, frontsector->ceilingpic, l);
    }

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_FRONT | POLY_MODULATION | POLY_FOG;

    if(viewz >= frontsector->floorheight && frontsector->floorpic != skyflatnum)
    {
        l = &lights[frontsector->colors[LIGHT_FLOOR]];
        R_DrawSubsector(subsector, frontsector->floorheight, frontsector->floorpic, l);
    }
}

//
// R_DrawScene
//

void R_DrawScene(void)
{
    for(nextssect = nextssect - 1; nextssect >= ssectlist; nextssect--)
    {
        subsector_t* sub;

        sub = *nextssect;
        frontsector = sub->sector;

        R_DrawLeafs(sub);
    }
}

