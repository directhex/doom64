#include "m_fixed.h"
#include "tables.h"
#include "r_local.h"
#include "z_zone.h"
#include "w_wad.h"

//
// R_DrawLine
//

static void R_DrawLine(seg_t* seg, fixed_t top, fixed_t bottom,
                       dtexture texture, light_t* l1, light_t* l2)
{
    int x1, x2;
    int y1, y2;
    int z1, z2;
    int r1, r2;
    int g1, g2;
    int b1, b2;

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

    GFX_BEGIN = GL_TRIANGLE_STRIP;
    GFX_COLOR = RGB15(r1, g1, b1);
    GFX_VERTEX16 = VERTEX_PACK(x2, z1);
    GFX_VERTEX16 = VERTEX_PACK(y2, 0);
    GFX_VERTEX16 = VERTEX_PACK(x1, z1);
    GFX_VERTEX16 = VERTEX_PACK(y1, 0);
    GFX_COLOR = RGB15(r2, g2, b2);
    GFX_VERTEX16 = VERTEX_PACK(x2, z2);
    GFX_VERTEX16 = VERTEX_PACK(y2, 0);
    GFX_VERTEX16 = VERTEX_PACK(x1, z2);
    GFX_VERTEX16 = VERTEX_PACK(y1, 0);
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
    
    linedef = seg->linedef;
    sidedef = seg->sidedef;

    if(!linedef)
        return;

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
                R_DrawLine(seg, bbottom, bottom, sidedef->bottomtexture, l1, l2);
            
            bottom = bbottom;
        }
        
        //
        // upper side seg
        //
        if(top > btop)
        {
            if(seg->sidedef[0].toptexture != 1)
                R_DrawLine(seg, top, btop, sidedef->toptexture, l1, l2);
            
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
            R_DrawLine(seg, top, bottom, sidedef->midtexture, l1, l2);
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

    GFX_COLOR = RGB15(
        light->active_r >> 3,
        light->active_g >> 3,
        light->active_b >> 3
        );
    GFX_BEGIN = GL_TRIANGLE_STRIP;

    z = F2INT(height);

    for(i = 0; i < ss->numleafs - 2; i++)
    {
        x = F2INT(leafs[ss->leaf + 1 + i].vertex->x);
        y = F2INT(leafs[ss->leaf + 1 + i].vertex->y);

        GFX_VERTEX16 = VERTEX_PACK(x, z);
        GFX_VERTEX16 = VERTEX_PACK(y, 0);

        x = F2INT(leafs[ss->leaf + 2 + i].vertex->x);
        y = F2INT(leafs[ss->leaf + 2 + i].vertex->y);

        GFX_VERTEX16 = VERTEX_PACK(x, z);
        GFX_VERTEX16 = VERTEX_PACK(y, 0);

        x = F2INT(leafs[ss->leaf + 0].vertex->x);
        y = F2INT(leafs[ss->leaf + 0].vertex->y);

        GFX_VERTEX16 = VERTEX_PACK(x, z);
        GFX_VERTEX16 = VERTEX_PACK(y, 0);
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