#include "m_fixed.h"
#include "tables.h"
#include "r_local.h"
#include "z_zone.h"
#include "p_local.h"
#include "w_wad.h"

dboolean        skyvisible = false;
int             skypicnum = -1;
int             skybackdropnum = -1;
int             skyflatnum = -1;
skydef_t*       sky;
int             thunderCounter = 0;
int             lightningCounter = 0;
int             thundertic = 1;
dboolean        skyfadeback = false;
fixed_t         scrollfrac;
gfx_t           gfx_skydata[5];
uint32          gfx_skypalparams[5];
int             gfx_skylumpnums[5];

static lumpinfo_t*  gfx_skylump[5];
static int          cloud_offsetx = 0;
static int          cloud_offsety = 0;

//
// R_DrawSwitch
//

static void R_DrawSwitch(seg_t* seg, dtexture texture, fixed_t top, fixed_t bottom)
{
    fixed_t cenx;
    fixed_t ceny;
    fixed_t f1;
    fixed_t f2;
    fixed_t s1;
    fixed_t s2;
    light_t* light;
    int x1;
    int x2;
    int y1;
    int y2;
    int z1;
    int z2;
    int r;
    int g;
    int b;

    I_CheckGFX();

    light = &lights[frontsector->colors[LIGHT_THING]];

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

    cenx    = (seg->linedef->v1->x + seg->linedef->v2->x) >> 1;
    ceny    = (seg->linedef->v1->y + seg->linedef->v2->y) >> 1;
    f1      = FixedMul(2*FRACUNIT, dcos(seg->angle + ANG90));
    f2      = FixedMul(2*FRACUNIT, dsin(seg->angle + ANG90));
    s1      = FixedMul(16*FRACUNIT, dcos(seg->angle));
    s2      = FixedMul(16*FRACUNIT, dsin(seg->angle));
    x1      = F2DSFIXED((cenx - s1) - f1);
    x2      = F2DSFIXED((cenx + s1) - f1);
    y1      = F2DSFIXED((ceny - s2) - f2);
    y2      = F2DSFIXED((ceny + s2) - f2);
    z1      = F2DSFIXED(top);
    z2      = F2DSFIXED(bottom);

    R_LoadTexture(texture, false, false);

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_BACK | POLY_MODULATION | POLY_FOG;
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_TEX_COORD   = COORD_PACK(0, 0);
    GFX_VERTEX16    = VERTEX_PACK(x2, z1);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);
    GFX_TEX_COORD   = COORD_PACK(32, 0);
    GFX_VERTEX16    = VERTEX_PACK(x1, z1);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
    GFX_TEX_COORD   = COORD_PACK(0, 32);
    GFX_VERTEX16    = VERTEX_PACK(x2, z2);
    GFX_VERTEX16    = VERTEX_PACK(y2, 0);
    GFX_TEX_COORD   = COORD_PACK(32, 32);
    GFX_VERTEX16    = VERTEX_PACK(x1, z2);
    GFX_VERTEX16    = VERTEX_PACK(y1, 0);
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
    line_t* line;
    side_t* side;

    I_CheckGFX();

    r1 = l1->active_r;
    g1 = l1->active_g;
    b1 = l1->active_b;
    r2 = l2->active_r;
    g2 = l2->active_g;
    b2 = l2->active_b;

    line = seg->linedef;
    side = seg->sidedef;

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

    if(line->flags & ML_TWOSIDED)
    {
        int height = 0;
        int sideheight1 = 0;
        int sideheight2 = 0;

        height = frontsector->ceilingheight - frontsector->floorheight;

        if(bottom != frontsector->floorheight)
        {
            if(!(line->flags & ML_BLENDFULLTOP))
            {
                sideheight1 = seg->backsector->ceilingheight - seg->frontsector->floorheight;
                sideheight2 = seg->frontsector->ceilingheight - seg->backsector->ceilingheight;

                //
                // math for RGB values is done in fixed point first
                //
                if(!nolights)
                {
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
            }

            if(line->flags & ML_INVERSEBLEND)
            {
                r1 = l2->active_r;
                g1 = l2->active_g;
                b1 = l2->active_b;
                r2 = l1->active_r;
                g2 = l1->active_g;
                b2 = l1->active_b;
            }
        }
        else if(top != frontsector->ceilingheight && !(line->flags & ML_BLENDFULLBOTTOM))
        {
            sideheight1 = seg->backsector->floorheight - seg->frontsector->floorheight;
            sideheight2 = seg->frontsector->ceilingheight - seg->backsector->floorheight;

            //
            // math for RGB values is done in fixed point first
            //
            if(!nolights)
            {
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
    }

    x1 = F2DSFIXED(side->v1->x);
    x2 = F2DSFIXED(side->v2->x);
    y1 = F2DSFIXED(side->v1->y);
    y2 = F2DSFIXED(side->v2->y);
    z1 = F2DSFIXED(top);
    z2 = F2DSFIXED(bottom);

    R_LoadTexture(texture,
        (line->flags & ML_HMIRROR),
        (line->flags & ML_VMIRROR));

    if(nolights)
        r1 = r2 = g1 = g2 = b1 = b2 = 255;

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_BACK | POLY_MODULATION | POLY_FOG;
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
    fixed_t     offset;
    
    linedef = seg->linedef;
    sidedef = seg->sidedef;

    if(!linedef)
        return;

    col = sidedef->length;
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
                    col + sidedef->textureoffset,
                    sidedef->textureoffset,
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
                    col + sidedef->textureoffset,
                    sidedef->textureoffset,
                    v1,
                    v2
                    );
            }
            
            top = btop;
        }

        if(SWITCHMASK(linedef->flags))
        {
            if(SWITCHMASK(linedef->flags) == ML_SWITCHX02)
            {
                offset = seg->backsector->floorheight - (16*FRACUNIT - (seg->sidedef->rowoffset));
                R_DrawSwitch(seg, seg->sidedef->toptexture, offset, offset - (32*FRACUNIT));
            }
            else if(SWITCHMASK(linedef->flags) == ML_SWITCHX04)
            {
                offset = seg->backsector->ceilingheight + (16*FRACUNIT + (seg->sidedef->rowoffset));
                R_DrawSwitch(seg, seg->sidedef->bottomtexture, offset + (32*FRACUNIT), offset);
            }
            else
            {
                if(seg->backsector->floorheight > seg->frontsector->floorheight)
                {
                    offset = seg->backsector->floorheight - (16*FRACUNIT - (seg->sidedef->rowoffset));
                    R_DrawSwitch(seg, seg->sidedef->midtexture, offset, offset - (32*FRACUNIT));
                }
                else if(seg->backsector->ceilingheight < seg->frontsector->ceilingheight)
                {
                    offset = seg->backsector->ceilingheight + (16*FRACUNIT + (seg->sidedef->rowoffset));
                    R_DrawSwitch(seg, seg->sidedef->midtexture, offset + (32*FRACUNIT), offset);
                }
            }
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

            if(SWITCHMASK(linedef->flags))
            {
                if(SWITCHMASK(linedef->flags) == ML_SWITCHX02)
                {
                    offset = seg->frontsector->floorheight + (16*FRACUNIT + (seg->sidedef->rowoffset));
                    R_DrawSwitch(seg, seg->sidedef->toptexture, offset + (32*FRACUNIT), offset);
                }
                else if(SWITCHMASK(linedef->flags) == ML_SWITCHX04)
                {
                    offset = seg->frontsector->floorheight + (16*FRACUNIT + (seg->sidedef->rowoffset));
                    R_DrawSwitch(seg, seg->sidedef->bottomtexture, offset + (32*FRACUNIT), offset);
                }
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
            col + sidedef->textureoffset,
            sidedef->textureoffset,
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
        seg_t* seg = leafs[subsector->leaf + i].seg;

        if(seg->sidedef)
        {
            if(seg->sidedef->draw)
            {
                R_DrawSeg(seg);
                seg->sidedef->draw = false;
            }
        }
    }

    if(!skyvisible && (frontsector->ceilingpic == skyflatnum || frontsector->floorpic == skyflatnum))
        skyvisible = true;

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

    if(thing->subsector->sector->lightlevel && !(thing->flags & MF_MISSILE))
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
// R_DrawLaser
//

static void R_DrawLaser(mobj_t* thing)
{
    laser_t* laser;
    int spritenum;
    fixed_t c;
    fixed_t s;
    fixed_t dx1;
    fixed_t dx2;

    // must have data present
    if(!thing->extradata)
        return;

    laser = (laser_t*)thing->extradata;
    spritenum = spriteinfo[SPR_BOLT].spriteframes[0].lump[0];

    if(!R_LoadSprite(SPR_BOLT, thing->frame & FF_FRAMEMASK, 0, 0, NULL, NULL, NULL, NULL))
        return;

    I_CheckGFX();

    // get angles
    s = dsin(laser->angle + ANG90);
    c = dcos(laser->angle + ANG90);

    dx1 = -INT2F(spritetopoffset[spritenum]);
    dx2 = dx1 + INT2F(spriteheight[spritenum] << 2);

    GFX_POLY_FORMAT =
        POLY_ALPHA((thing->alpha >> 3)) |
        POLY_ID(0)                      |
        POLY_CULL_NONE                  |
        POLY_MODULATION;

#define LASER_VERTEX(x, a, d) F2DSFIXED(laser->x + FixedMul(a, d))

    GFX_COLOR       = RGB15(31, 0, 0);
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_TEX_COORD   = COORD_PACK(0, 0);
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(x1, c, dx1), F2DSFIXED(laser->z1));
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(y1, s, dx1), 0);
    GFX_TEX_COORD   = COORD_PACK(spritewidth[spritenum], 0);
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(x2, c, dx1), F2DSFIXED(laser->z2));
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(y2, s, dx1), 0);
    GFX_TEX_COORD   = COORD_PACK(0, spriteheight[spritenum]);
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(x1, c, dx2), F2DSFIXED(laser->z1));
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(y1, s, dx2), 0);
    GFX_TEX_COORD   = COORD_PACK(spritewidth[spritenum], spriteheight[spritenum]);
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(x2, c, dx2), F2DSFIXED(laser->z2));
    GFX_VERTEX16    = VERTEX_PACK(LASER_VERTEX(y2, s, dx2), 0);

#undef LASER_VERTEX
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

        if(mobj->flags & MF_RENDERLASER)
            R_DrawLaser(mobj);
        else
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

    if(!R_LoadSprite(psp->state->sprite, psp->state->frame & FF_FRAMEMASK,
        0, 0, &x, &y, &width, &height))
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

    GFX_ORTHO();

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
    GFX_VERTEX16    = VERTEX_PACK(-4, 0);
    GFX_TEX_COORD   = COORD_PACK(width, 0);
    GFX_VERTEX16    = VERTEX_PACK(width + x, y);
    GFX_VERTEX16    = VERTEX_PACK(-4, 0);
    GFX_TEX_COORD   = COORD_PACK(0, height);
    GFX_VERTEX16    = VERTEX_PACK(x, height + y);
    GFX_VERTEX16    = VERTEX_PACK(-4, 0);
    GFX_TEX_COORD   = COORD_PACK(width, height);
    GFX_VERTEX16    = VERTEX_PACK(width + x, height + y);
    GFX_VERTEX16    = VERTEX_PACK(-4, 0);

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
            GFX_VERTEX16    = VERTEX_PACK(-4, 0);
            GFX_VERTEX16    = VERTEX_PACK(width + x, y);
            GFX_VERTEX16    = VERTEX_PACK(-4, 0);
            GFX_VERTEX16    = VERTEX_PACK(x, height + y);
            GFX_VERTEX16    = VERTEX_PACK(-4, 0);
            GFX_VERTEX16    = VERTEX_PACK(width + x, height + y);
            GFX_VERTEX16    = VERTEX_PACK(-4, 0);
        }
    }
}

//
// R_DrawSkyPic
//

static void R_DrawSkyPic(int lump, int yoffset, dboolean backdrop)
{
    short* lumpdata;
    int width;
    int height;
    int pw;
    int ph;
    int z;
    byte* data;
    int offset;
    int index = 0;

    if(lump == -1)
        return;

    if(lump == gfx_skylumpnums[0])
        index = 0;
    else if(lump == gfx_skylumpnums[1])
        index = 1;
    else if(lump == gfx_skylumpnums[2])
        index = 2;
    else if(lump == gfx_skylumpnums[3])
        index = 3;
    else if(lump == gfx_skylumpnums[4])
        index = 4;

    I_CheckGFX();

    gfx_skylump[index] = &lumpinfo[lump];

    if(gfx_skylump[index]->cache == NULL)
        W_CacheLumpNum(lump, PU_CACHE);

    lumpdata = (short*)gfx_skylump[index]->cache;
    width = lumpdata[0];
    height = lumpdata[1];
    data = (byte*)(lumpdata + 4);
    pw = R_PadTextureDims(width);
    ph = R_PadTextureDims(height);

    if(!I_AllocVBlock(
        &gfx_skydata[index],
        data,
        pw * height,
        TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
        R_GetTextureSize(pw),
        R_GetTextureSize(ph),
        GL_RGB256))
        return;

    offset = -((viewangle >> 22) & 0xff);

    if(backdrop)
        z = 0xFFC;
    else
        z = 0x1000;

    height = (ph << 5);
    yoffset <<= 5;

    GFX_POLY_FORMAT = POLY_ALPHA(31) | POLY_ID(0) | POLY_CULL_NONE | POLY_MODULATION;
    GFX_TEX_FORMAT  = gfx_skydata[index].params;
    GFX_PAL_FORMAT  = gfx_skypalparams[index];
    GFX_COLOR       = RGB15(31, 31, 31);
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_TEX_COORD   = COORD_PACK(offset, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x1000, height - yoffset);
    GFX_VERTEX16    = VERTEX_PACK(z, 0);
    GFX_TEX_COORD   = COORD_PACK(256 + offset, 0);
    GFX_VERTEX16    = VERTEX_PACK(0x1000, height - yoffset);
    GFX_VERTEX16    = VERTEX_PACK(z, 0);
    GFX_TEX_COORD   = COORD_PACK(offset, ph);
    GFX_VERTEX16    = VERTEX_PACK(-0x1000, -yoffset);
    GFX_VERTEX16    = VERTEX_PACK(z, 0);
    GFX_TEX_COORD   = COORD_PACK(256 + offset, ph);
    GFX_VERTEX16    = VERTEX_PACK(0x1000, -yoffset);
    GFX_VERTEX16    = VERTEX_PACK(z, 0);
}

//
// R_DrawCloud
//

static void R_DrawCloud(int lump)
{
    short* lumpdata;
    byte* data;
    int offset;

    if(lump == -1)
        return;

    GFX_CLEAR_COLOR = sky->skycolor[1];
    MATRIX_PUSH = 0;

    gluPerspective(45, 256.0f / 192.0f, 0.002f, 1000);

    gfx_skylump[4] = &lumpinfo[lump];

    if(gfx_skylump[4]->cache == NULL)
        W_CacheLumpNum(lump, PU_CACHE);

    lumpdata = (short*)gfx_skylump[4]->cache;
    data = (byte*)(lumpdata + 4);

    if(!I_AllocVBlock(
        &gfx_skydata[4],
        data,
        4096,
        TEXGEN_OFF | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T,
        TEXTURE_SIZE_64,
        TEXTURE_SIZE_64,
        GL_RGB256))
        return;

    cloud_offsetx = (cloud_offsetx - (dcos(viewangle) >> 15)) & 0x3FF;
    cloud_offsety = (cloud_offsety + (dsin(viewangle) >> 14)) & 0x3FF;
    offset = cloud_offsetx - ((viewangle >> ANGLETOFINESHIFT) >> 1);

#define CLOUD_UV_PACK(u, v) (((((u) << 4) + offset) & 0xFFFF) | ((((v) << 4) + cloud_offsety) << 16))

    GFX_POLY_FORMAT = POLY_ALPHA(18) | POLY_ID(0x3F) | POLY_CULL_NONE | POLY_MODULATION;
    GFX_TEX_FORMAT  = gfx_skydata[4].params;
    GFX_PAL_FORMAT  = gfx_skypalparams[4];
    GFX_BEGIN       = GL_TRIANGLE_STRIP;
    GFX_COLOR       = sky->skycolor[0];
    GFX_TEX_COORD   = CLOUD_UV_PACK(0, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x6000, 0x2000);
    GFX_VERTEX16    = VERTEX_PACK(0, 0);
    GFX_TEX_COORD   = CLOUD_UV_PACK(256, 0);
    GFX_VERTEX16    = VERTEX_PACK(0x5000, 0x2000);
    GFX_VERTEX16    = VERTEX_PACK(0, 0);
    GFX_COLOR       = sky->skycolor[2];
    GFX_TEX_COORD   = CLOUD_UV_PACK(0, 256);
    GFX_VERTEX16    = VERTEX_PACK(-0x6000, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);
    GFX_TEX_COORD   = CLOUD_UV_PACK(256, 256);
    GFX_VERTEX16    = VERTEX_PACK(0x5000, 0);
    GFX_VERTEX16    = VERTEX_PACK(-0x7FFF, 0);

#undef CLOUD_UV_PACK

    MATRIX_POP = 1;
}

//
// R_DrawSky
//

void R_DrawSky(void)
{
    if(sky->flags & SKF_VOID)
        GFX_CLEAR_COLOR = sky->skycolor[2];
    else if(sky->flags & SKF_CLOUD)
        R_DrawCloud(skypicnum);
    else
        R_DrawSkyPic(skypicnum, 0, false);

    if(sky->flags & SKF_BACKGROUND)
        R_DrawSkyPic(skybackdropnum, 64, true);
}

