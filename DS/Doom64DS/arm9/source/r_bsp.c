#include <math.h>

#include "ds.h"
#include "r_local.h"
#include "d_main.h"
#include "z_zone.h"
#include "p_local.h"

sector_t* frontsector;

subsector_t *ssectlist[MAXSUBSECTORS];
subsector_t **nextssect = NULL;

int	checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},
    {2,0,2,1},
    {0,0,0,0},
    {3,1,3,0},
    {0},
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};

//
// R_CheckBBox
//

dboolean R_CheckBBox(fixed_t* bspcoord)
{	
    angle_t     angle1;
    angle_t     angle2;
    int         boxpos;
    const int*  check;
    
    // Find the corners of the box
    // that define the edges from current viewpoint.
    boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : viewx < bspcoord[BOXRIGHT] ? 1 : 2) +
        (viewy >= bspcoord[BOXTOP] ? 0 : viewy > bspcoord[BOXBOTTOM] ? 4 : 8);
    
    if(boxpos == 5)
        return true;
    
    check = checkcoord[boxpos];
    angle1 = R_PointToAngle2(bspcoord[check[0]], bspcoord[check[1]], viewx, viewy) - viewangle;
    angle2 = R_PointToAngle2(bspcoord[check[2]], bspcoord[check[3]], viewx, viewy) - viewangle;
    
    return R_Clipper_SafeCheckRange(angle2 + viewangle, angle1 + viewangle);
}

//
// R_AddClipLine
// Clips the given segment
// and adds any visible pieces to the line list.
//

static void R_AddClipLine(seg_t* line)
{
    angle_t angle1;
    angle_t angle2;

    line->draw = false;
    
    if(line->v1->validcount != validcount)
    {
        line->v1->clipspan = R_PointToAngle2(line->v1->x, line->v1->y, viewx, viewy);
        line->v1->validcount = validcount;
    }
    
    if(line->v2->validcount != validcount)
    {
        line->v2->clipspan = R_PointToAngle2(line->v2->x, line->v2->y, viewx, viewy);
        line->v2->validcount = validcount;
    }
    
    angle1 = line->v1->clipspan;
    angle2 = line->v2->clipspan;
    
    // Back side, i.e. backface culling	- read: endAngle >= startAngle!
    if(angle2 - angle1 < ANG180 || !line->linedef)
        return;
    
    if(!R_Clipper_SafeCheckRange(angle2, angle1))
        return;
    
    if(!(line->linedef->flags & (ML_DRAWMIDTEXTURE|ML_DONTOCCLUDE)))
    {
        if(line->backsector)
        {
            if((line->frontsector->ceilingpic != skyflatnum &&
                line->frontsector->floorpic != skyflatnum) &&
                (line->backsector->ceilingpic != skyflatnum &&
                line->backsector->floorpic != skyflatnum))
            {
                if((line->backsector->floorheight == line->backsector->ceilingheight) ||
                    line->backsector->ceilingheight <= line->frontsector->floorheight ||
                    line->backsector->floorheight >= line->frontsector->ceilingheight)
                    R_Clipper_SafeAddClipRange(angle2, angle1);
            }
        }
        else if(!line->backsector) // sanity check
            R_Clipper_SafeAddClipRange(angle2, angle1);
    }
    
    line->linedef->flags |= ML_MAPPED;
    line->draw = true;
}

//
// R_Subsector
//

void R_Subsector(int num)
{
    subsector_t	*sub;
    int i;

    if(num >= numsubsectors)
        I_Error("R_Subsector: ss %i with numss = %i", num, numsubsectors);

    if(nextssect - ssectlist >= MAXSUBSECTORS)
        return;
    
    sub = &subsectors[num];
    frontsector = sub->sector;

    *nextssect = sub;
    nextssect++;

    for(i = 0; i < sub->numlines; i++)
        R_AddClipLine(&segs[sub->firstline + i]);
}

//
// R_RenderBSPNode
//

void R_RenderBSPNode(int bspnum)
{
    node_t  *bsp;
    int     side;
    
    while(!(bspnum & NF_SUBSECTOR))
    {
        bsp = &nodes[bspnum];
        
        // Decide which side the view point is on.
        side = _R_PointOnSide(viewx, viewy, bsp);
        
        // check the front space
        if(R_CheckBBox(bsp->bbox[side]))
            R_RenderBSPNode(bsp->children[side]);
        
        // continue down the back space
        if(!R_CheckBBox(bsp->bbox[side^1]))
            return;
        
        bspnum = bsp->children[side^1];
    }
    
    // subsector with contents
    // add all the drawable elements in the subsector
    if(bspnum == -1)
        bspnum = 0;
    
    R_Subsector(bspnum & ~NF_SUBSECTOR);
}

