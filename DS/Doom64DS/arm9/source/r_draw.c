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
fixed_t     scrollfrac;

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
                       fixed_t u1, fixed_t u2, fixed_t v1, fixed_t v2)
{
    int x1, x2;
    int y1, y2;
    int z1, z2;
    int r1, r2;
    int g1, g2;
    int b1, b2;

    I_CheckGFX();

    r1 = l1->active_r;
    g1 = l1->active_g;
    b1 = l1->active_b;
    r2 = l2->active_r;
    g2 = l2->active_g;
    b2 = l2->active_b;

    //
    // sequenced lighting is too expensive on the DS
    // instead of creating a glowing plane, just amplify the
    // RGB values
    //
    if(frontsector->lightlevel && frontsector->special == 205)
    {
        r1 = MIN(r1 + (frontsector->lightlevel << 1), 255);
        g1 = MIN(g1 + (frontsector->lightlevel << 1), 255);
        b1 = MIN(b1 + (frontsector->lightlevel << 1), 255);
        r2 = MIN(r2 + (frontsector->lightlevel << 1), 255);
        g2 = MIN(g2 + (frontsector->lightlevel << 1), 255);
        b2 = MIN(b2 + (frontsector->lightlevel << 1), 255);
    }

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

                //
                // math for RGB values is done in fixed point first
                //
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

            //
            // math for RGB values is done in fixed point first
            //
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

    x1 = F2DSFIXED(seg->v1->x);
    x2 = F2DSFIXED(seg->v2->x);
    y1 = F2DSFIXED(seg->v1->y);
    y2 = F2DSFIXED(seg->v2->y);
    z1 = F2DSFIXED(top);
    z2 = F2DSFIXED(bottom);

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_BACK | POLY_MODULATION | POLY_FOG;

    R_LoadTexture(texture,
        (seg->linedef->flags & ML_HMIRROR),
        (seg->linedef->flags & ML_VMIRROR));

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

    //
    // create a glowing plane on top of the geometry for
    // specialized sectors
    //
    if(frontsector->lightlevel && frontsector->special != 205)
    {
        int lightlevel = ((frontsector->lightlevel << 1) >> 3);

        if(lightlevel)
        {
            GFX_TEX_FORMAT = 0;
            GFX_PAL_FORMAT = 0;
            GFX_POLY_FORMAT =
                POLY_ALPHA(lightlevel) |
                POLY_ID(2) |
                POLY_CULL_BACK |
                POLY_MODULATION |
                POLY_FOG |
                POLY_DEPTHTEST_EQUAL;

            GFX_BEGIN       = GL_TRIANGLE_STRIP;
            GFX_COLOR       = RGB8(r1, g1, b1);
            GFX_VERTEX16    = VERTEX_PACK(x2, z1);
            GFX_VERTEX16    = VERTEX_PACK(y2, 0);
            GFX_VERTEX16    = VERTEX_PACK(x1, z1);
            GFX_VERTEX16    = VERTEX_PACK(y1, 0);
            GFX_COLOR       = RGB8(r2, g2, b2);
            GFX_VERTEX16    = VERTEX_PACK(x2, z2);
            GFX_VERTEX16    = VERTEX_PACK(y2, 0);
            GFX_VERTEX16    = VERTEX_PACK(x1, z2);
            GFX_VERTEX16    = VERTEX_PACK(y1, 0);
        }
    }
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
        fixed_t v1;
        fixed_t v2;

        v1 = row;
        v2 = row + (top - bottom);

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

            if(linedef->flags & ML_DONTPEGMID)
            {
                v1 = row - (top - btop);
                v2 = row + (((top + btop) - (bottom + bbottom)) >> 1);
            }
        }
        else
        {
            if(linedef->flags & ML_DONTPEGTOP)
            {
                v1 = ((row - bottom) - (top - bottom));
                v2 = ((row - bottom) - (top - bottom)) + (top - bottom);
            }
            else if(linedef->flags & ML_DONTPEGBOTTOM)
            {
                v1 = row - (top - bottom);
                v2 = row;
            }
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
            v1,
            v2
            );
        }
    }
}

//
// R_DrawSubsector
//

static void R_DrawSubsector(subsector_t* ss, fixed_t height,
                            dtexture texture, light_t* light,
                            fixed_t xoffset, fixed_t yoffset)
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
    int r;
    int g;
    int b;

    I_CheckGFX();

    r = light->active_r;
    g = light->active_g;
    b = light->active_b;

    //
    // sequenced lighting is too expensive on the DS
    // instead of creating a glowing plane, just amplify the
    // RGB values
    //
    if(frontsector->lightlevel && frontsector->special == 205)
    {
        r = MIN(r + (frontsector->lightlevel << 1), 255);
        g = MIN(g + (frontsector->lightlevel << 1), 255);
        b = MIN(b + (frontsector->lightlevel << 1), 255);
    }

    if(nolights)
        GFX_COLOR = 0x1F7FFF;
    else
        GFX_COLOR = RGB8(r, g, b);

    tx      = F2INT(leafs[ss->leaf].vertex->x + xoffset) & 0x3F;
    ty      = F2INT(leafs[ss->leaf].vertex->y - yoffset) & 0x3F;
    tsx     = leafs[ss->leaf].vertex->x;
    tsy     = leafs[ss->leaf].vertex->y;
    mapx    = 0;
    mapy    = 0;
    z       = F2DSFIXED(height);

    if(texture >= 0)
        R_LoadTexture(texture, false, false);
    else
    {
        GFX_TEX_FORMAT = 0;
        GFX_PAL_FORMAT = 0;
    }

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
    x = F2DSFIXED(v->x);                    \
    y = F2DSFIXED(v->y);                    \
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
    fixed_t x, y;

    for(i = 0; i < subsector->numleafs; i++)
    {
        seg_t* seg;

        seg = leafs[subsector->leaf + i].seg;
        if(seg->draw)
            R_DrawSeg(seg);
    }

    if(viewz <= frontsector->ceilingheight && frontsector->ceilingpic != skyflatnum)
    {
        if(frontsector->flags & MS_SCROLLCEILING)
        {
            x = frontsector->xoffset;
            y = frontsector->yoffset;
        }
        else
            x = y = 0;

        l = &lights[frontsector->colors[LIGHT_CEILING]];

        GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_BACK | POLY_MODULATION | POLY_FOG;

        R_DrawSubsector(subsector,
            frontsector->ceilingheight,
            frontsector->ceilingpic, l, x, y);

        //
        // create a glowing plane on top of the geometry for
        // specialized sectors
        //
        if(frontsector->lightlevel && frontsector->special != 205)
        {
            int lightlevel = ((frontsector->lightlevel << 1) >> 3);

            if(lightlevel)
            {
                GFX_POLY_FORMAT =
                    POLY_ALPHA(lightlevel) |
                    POLY_ID(2) |
                    POLY_CULL_BACK |
                    POLY_MODULATION |
                    POLY_FOG |
                    POLY_DEPTHTEST_EQUAL;

                R_DrawSubsector(subsector,
                    frontsector->ceilingheight,
                    -1, l, 0, 0);
            }
        }
    }

    if(viewz >= frontsector->floorheight && frontsector->floorpic != skyflatnum)
    {
        if(frontsector->flags & MS_SCROLLFLOOR)
        {
            x = frontsector->xoffset;
            y = frontsector->yoffset;
        }
        else
            x = y = 0;

        l = &lights[frontsector->colors[LIGHT_FLOOR]];

        GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_FRONT | POLY_MODULATION | POLY_FOG;

        R_DrawSubsector(subsector,
            frontsector->floorheight,
            frontsector->floorpic, l, x, y);

        //
        // create a glowing plane on top of the geometry for
        // specialized sectors
        //
        if(frontsector->lightlevel && frontsector->special != 205)
        {
            int lightlevel = ((frontsector->lightlevel << 1) >> 3);

            if(lightlevel)
            {
                GFX_POLY_FORMAT =
                    POLY_ALPHA(lightlevel) |
                    POLY_ID(2) |
                    POLY_CULL_FRONT |
                    POLY_MODULATION |
                    POLY_FOG |
                    POLY_DEPTHTEST_EQUAL;

                R_DrawSubsector(subsector,
                    frontsector->floorheight,
                    -1, l, 0, 0);
            }
        }
    }
}

//
// R_DrawSprite
//

static void R_DrawSprite(mobj_t* thing)
{
    spriteframe_t*  sprframe;
    angle_t ang;
    int rot;
    int flipoffs;
    int tu1;
    int tu2;
    int tx1;
    int tx2;
    short alpha;
    rcolor color;
    int offx;
    int offy;
    int width;
    int height;
    fixed_t x1, x2;
    fixed_t y1, y2;
    fixed_t z1, z2;

    alpha = thing->alpha >> 3;

    if(alpha <= 0)
        return;

    I_CheckGFX();

    sprframe = &spriteinfo[thing->sprite].spriteframes[thing->frame & FF_FRAMEMASK];
    
    if(sprframe->rotate)
    {
        // choose a different rotation based on player view
        ang = R_PointToAngle2(viewx, viewy, thing->x, thing->y);
        rot = (ang-thing->angle + (unsigned)(ANG45 / 2) * 9) >> 29;
    }
    else
        // use single rotation for all views
        rot = 0;

    if(!R_LoadSprite(thing->sprite, thing->frame & FF_FRAMEMASK, rot, thing->info->palette,
        &offx, &offy, &width, &height))
        return;

    if(thing->frame & FF_FULLBRIGHT || nolights)
        color = RGB15(31, 31, 31);
    else
    {
        light_t *light;

        light = &lights[thing->subsector->sector->colors[LIGHT_THING]];
        color = RGB8(light->active_r, light->active_g, light->active_b);
    }

    if(sprframe->flip[rot])
    {
        flipoffs = width - offx;
        tu1 = 0;
        tu2 = width;
        tx1 = width - flipoffs;
        tx2 = flipoffs;
    }
    else
    {
        flipoffs = width - offx;
        tu1 = width;
        tu2 = 0;
        tx1 = flipoffs;
        tx2 = width - flipoffs;
    }

    x1 = INT2DSFIXED(F2INT(thing->x) - FixedMul(viewcos[0], tx1));
    y1 = INT2DSFIXED(F2INT(thing->y) - FixedMul(viewsin[0], tx1));
    x2 = INT2DSFIXED(F2INT(thing->x) + FixedMul(viewcos[0], tx2));
    y2 = INT2DSFIXED(F2INT(thing->y) + FixedMul(viewsin[0], tx2));
    z1 = INT2DSFIXED(F2INT(thing->z) + offy - height);
    z2 = INT2DSFIXED(F2INT(thing->z) + offy);

    GFX_POLY_FORMAT =
        POLY_ALPHA(alpha)   |
        POLY_ID(0)          |
        POLY_CULL_BACK      |
        POLY_MODULATION     |
        POLY_FOG            |
        POLY_NEW_DEPTH;

    GFX_COLOR       = color;
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_TEX_COORD   = COORD_PACK(tu1, 0);
    GFX_VERTEX16    = VERTEX_PACK(x1, z2);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
    GFX_TEX_COORD   = COORD_PACK(tu2, 0);
    GFX_VERTEX16    = VERTEX_PACK(x2, z2);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);
    GFX_TEX_COORD   = COORD_PACK(tu1, height);
    GFX_VERTEX16    = VERTEX_PACK(x1, z1);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
    GFX_TEX_COORD   = COORD_PACK(tu2, height);
    GFX_VERTEX16    = VERTEX_PACK(x2, z1);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);

    if(thing->subsector->sector->lightlevel)
    {
        int lightlevel = ((thing->subsector->sector->lightlevel << 1) >> 3);

        if(lightlevel)
        {
            GFX_TEX_FORMAT = 0;
            GFX_PAL_FORMAT = 0;
            GFX_POLY_FORMAT =
                POLY_ALPHA(lightlevel) |
                POLY_ID(2) |
                POLY_CULL_BACK |
                POLY_MODULATION |
                POLY_FOG |
                POLY_DEPTHTEST_EQUAL;

            GFX_COLOR       = color;
            GFX_BEGIN       = GL_TRIANGLE_STRIP;
            GFX_VERTEX16    = VERTEX_PACK(x1, z2);
            GFX_VERTEX16    = VERTEX_PACK(y1, 0);
            GFX_VERTEX16    = VERTEX_PACK(x2, z2);
            GFX_VERTEX16    = VERTEX_PACK(y2, 0);
            GFX_VERTEX16    = VERTEX_PACK(x1, z1);
            GFX_VERTEX16    = VERTEX_PACK(y1, 0);
            GFX_VERTEX16    = VERTEX_PACK(x2, z1);
            GFX_VERTEX16    = VERTEX_PACK(y2, 0);
        }
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

    for(vissprite = vissprite - 1; vissprite >= visspritelist; vissprite--)
    {
        mobj_t* mobj;

        mobj = *vissprite;
        R_DrawSprite(mobj);
    }
}

//
// R_DrawPSprite
//

void R_DrawPSprite(pspdef_t *psp, sector_t* sector, player_t *player)
{
    rcolor      color;
    short       alpha;
    fixed_t     x;
    fixed_t     y;
    int         width;
    int         height;
    uint32      polyflags;

    I_CheckGFX();

    alpha = (((player->mo->alpha * psp->alpha) / 0xff) >> 3);

    if(!R_LoadSprite(psp->state->sprite, psp->state->frame & FF_FRAMEMASK, 0, 0, &x, &y, &width, &height))
        return;

    x = F2INT(psp->sx) - x - 32;
    y = F2INT(psp->sy) - y - 48;

    if(player->onground)
    {
        x += (quakeviewx >> 24);
        y += (quakeviewy >> 16);
    }
    
    if(psp->state->frame & FF_FULLBRIGHT || nolights)
        color = RGB15(31, 31, 31);
    else
    {
        light_t *light;

        light = &lights[sector->colors[LIGHT_THING]];
        color = RGB8(light->active_r, light->active_g, light->active_b);
    }

    GFX_ORTHO(0);

    polyflags = POLY_ALPHA(alpha) | POLY_ID(0) | POLY_CULL_NONE | POLY_MODULATION;

    //
    // hack for plasma gun sprite;
    // set the depth test to equal when
    // drawing the animating plasma tube
    //
    if(psp == &players->psprites[ps_flash] && psp->state->sprite == SPR_PLAS)
        polyflags |= POLY_DEPTHTEST_EQUAL;

    GFX_POLY_FORMAT = polyflags;
    GFX_COLOR       = color;
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_TEX_COORD   = COORD_PACK(0, 0);
    GFX_VERTEX16    = VERTEX_PACK(x, y);
    GFX_VERTEX16    = VERTEX_PACK(0, 0);
    GFX_TEX_COORD   = COORD_PACK(width, 0);
    GFX_VERTEX16    = VERTEX_PACK(width + x, y);
    GFX_VERTEX16    = VERTEX_PACK(0, 0);
    GFX_TEX_COORD   = COORD_PACK(0, height);
    GFX_VERTEX16    = VERTEX_PACK(x, height + y);
    GFX_VERTEX16    = VERTEX_PACK(0, 0);
    GFX_TEX_COORD   = COORD_PACK(width, height);
    GFX_VERTEX16    = VERTEX_PACK(width + x, height + y);
    GFX_VERTEX16    = VERTEX_PACK(0, 0);

    if(sector->lightlevel)
    {
        int lightlevel = ((sector->lightlevel << 1) >> 3);

        if(lightlevel)
        {
            GFX_TEX_FORMAT = 0;
            GFX_PAL_FORMAT = 0;
            GFX_POLY_FORMAT =
                POLY_ALPHA(lightlevel) |
                POLY_ID(2) |
                POLY_CULL_NONE |
                POLY_MODULATION |
                POLY_DEPTHTEST_EQUAL;

            GFX_COLOR       = color;
            GFX_BEGIN       = GL_TRIANGLE_STRIP;
            GFX_VERTEX16    = VERTEX_PACK(x, y);
            GFX_VERTEX16    = VERTEX_PACK(0, 0);
            GFX_VERTEX16    = VERTEX_PACK(width + x, y);
            GFX_VERTEX16    = VERTEX_PACK(0, 0);
            GFX_VERTEX16    = VERTEX_PACK(x, height + y);
            GFX_VERTEX16    = VERTEX_PACK(0, 0);
            GFX_VERTEX16    = VERTEX_PACK(width + x, height + y);
            GFX_VERTEX16    = VERTEX_PACK(0, 0);
        }
    }
}

