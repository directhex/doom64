#include "m_fixed.h"
#include "tables.h"
#include "r_local.h"
#include "z_zone.h"
#include "w_wad.h"

//
// R_GetTextureSize
//

static int R_GetTextureSize(int size)
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

static int R_LoadTexture(dtexture texture)
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
// R_DrawLine
//

static void R_DrawLine(seg_t* seg, fixed_t top, fixed_t bottom,
                       dtexture texture, light_t* l1, light_t* l2,
                       fixed_t u1, fixed_t u2, fixed_t v1, fixed_t v2)
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

    r1 >>= 3;
    g1 >>= 3;
    b1 >>= 3;
    r2 >>= 3;
    g2 >>= 3;
    b2 >>= 3;

    x1 = F2INT(seg->v1->x);
    x2 = F2INT(seg->v2->x);
    y1 = F2INT(seg->v1->y);
    y2 = F2INT(seg->v2->y);
    z1 = F2INT(top);
    z2 = F2INT(bottom);

    if(gfxtextures[texture] == -1)
        R_LoadTexture(texture);
    else
        glBindTexture(0, gfxtextures[texture]);

    if((tex = R_GetTexturePointer(gfxtextures[texture])))
    {
        if(seg->linedef->flags & ML_HMIRROR)
            tex->texFormat |= GL_TEXTURE_FLIP_S;
	    else
		    tex->texFormat &= ~GL_TEXTURE_FLIP_S;

        if(seg->linedef->flags & ML_VMIRROR)
		    tex->texFormat |= GL_TEXTURE_FLIP_T;
	    else
		    tex->texFormat &= ~GL_TEXTURE_FLIP_T;
    }

    if(nolights)
        r1 = r2 = g1 = g2 = b1 = b2 = 31;

    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_COLOR       = RGB15(r1, g1, b1);
    GFX_TEX_COORD   = COORD_PACK(F2INT(u1), F2INT(v1));
    GFX_VERTEX16    = VERTEX_PACK(x2, z1);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);
    GFX_TEX_COORD   = COORD_PACK(F2INT(u2), F2INT(v1));
    GFX_VERTEX16    = VERTEX_PACK(x1, z1);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
    GFX_COLOR       = RGB15(r2, g2, b2);
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
                    v2
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
                    v2
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
            (top - bottom) + row
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
    fixed_t tx;
    fixed_t ty;

    if(nolights)
        GFX_COLOR = 0x1F7FFF;
    else
    {
        GFX_COLOR = RGB15(
            light->active_r >> 3,
            light->active_g >> 3,
            light->active_b >> 3
            );
    }

    tx = (leafs[ss->leaf].vertex->x >> 6) & ~(FRACUNIT - 1);
    ty = (leafs[ss->leaf].vertex->y >> 6) & ~(FRACUNIT - 1);

    if(gfxtextures[texture] == -1)
        R_LoadTexture(texture);
    else
        glBindTexture(0, gfxtextures[texture]);

    GFX_BEGIN = GL_TRIANGLES;

    z = F2INT(height);

    for(i = 0; i < ss->numleafs - 2; i++)
    {
        fixed_t tu;
        fixed_t tv;

        x = F2INT(leafs[ss->leaf + 1 + i].vertex->x);
        y = F2INT(leafs[ss->leaf + 1 + i].vertex->y);
        tu = F2INT((leafs[ss->leaf + 1 + i].vertex->x >> 6) - tx);
        tv = -F2INT((leafs[ss->leaf + 1 + i].vertex->y >> 6) - ty);

        GFX_TEX_COORD   = COORD_PACK(tu, tv);
        GFX_VERTEX16    = VERTEX_PACK(x, z);
        GFX_VERTEX16    = VERTEX_PACK(y, 0);

        x = F2INT(leafs[ss->leaf + 2 + i].vertex->x);
        y = F2INT(leafs[ss->leaf + 2 + i].vertex->y);
        tu = F2INT((leafs[ss->leaf + 2 + i].vertex->x >> 6) - tx);
        tv = -F2INT((leafs[ss->leaf + 2 + i].vertex->y >> 6) - ty);

        GFX_TEX_COORD   = COORD_PACK(tu, tv);
        GFX_VERTEX16    = VERTEX_PACK(x, z);
        GFX_VERTEX16    = VERTEX_PACK(y, 0);

        x = F2INT(leafs[ss->leaf + 0].vertex->x);
        y = F2INT(leafs[ss->leaf + 0].vertex->y);
        tu = F2INT((leafs[ss->leaf + 0 + i].vertex->x >> 6) - tx);
        tv = -F2INT((leafs[ss->leaf + 0 + i].vertex->y >> 6) - ty);

        GFX_TEX_COORD   = COORD_PACK(tu, tv);
        GFX_VERTEX16    = VERTEX_PACK(x, z);
        GFX_VERTEX16    = VERTEX_PACK(y, 0);
    }
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

    if(viewz <= frontsector->ceilingheight)
    {
        l = &lights[frontsector->colors[LIGHT_CEILING]];
        R_DrawSubsector(subsector, frontsector->ceilingheight, frontsector->ceilingpic, l);
    }

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_FRONT | POLY_MODULATION | POLY_FOG;

    if(viewz >= frontsector->floorheight)
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