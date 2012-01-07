// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Author$
// $Revision$
// $Date$
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif

#include "doomtype.h"
#include "doomdef.h"
#include "t_bsp.h"
#include "tables.h"

//
// M_CrossProduct
//

void M_CrossProduct(float *out, float *vec1, float *vec2)
{
    out[0] = vec2[2] * vec1[1] - vec1[2] * vec2[1];
    out[1] = vec2[0] * vec1[2] - vec1[0] * vec2[2];
    out[2] = vec1[0] * vec2[1] - vec2[0] * vec1[1];
}

//
// FixedDot
//

fixed_t M_DotProduct(fixed_t a1, fixed_t b1,
                     fixed_t c1, fixed_t a2,
                     fixed_t b2, fixed_t c2)
{
    return 
        FixedMul(a1, a2) +
        FixedMul(b1, b2) +
        FixedMul(c1, c2);
}

//
// M_Normalize3
//

void M_Normalize3(float *out)
{
    float d;

    d = (float)sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);

    if(d != 0.0f)
    {
        out[0]  = out[0] * 1.0f / d;
        out[1]  = out[1] * 1.0f / d;
        out[2]  = out[2] * 1.0f / d;
    }
}

//
// M_PointToZ
//

fixed_t M_PointToZ(plane_t* plane, fixed_t x, fixed_t y)
{
    return FixedMul(plane->nc, -plane->d -
        (FixedMul(plane->a, x) + FixedMul(plane->b, y)));
}

//
// M_FacePlaneDistance
//

fixed_t M_FacePlaneDistance(plane_t* plane, fixed_t x, fixed_t y, fixed_t z)
{
    if((plane->a | plane->b) == 0)
        return 0;   // plane is flat

    return M_DotProduct(plane->a, plane->b, plane->c, x, y, z);
}

//
// M_AlignPitchToPlane
//

angle_t M_AlignPitchToPlane(plane_t* plane, angle_t angle, float threshold)
{
    float n1[3];
    float n2[3];
    float an;

    if((plane->a | plane->b) == 0)
        return 0;   // plane is flat

    n1[0] = threshold * (float)cos((float)(angle >> ANGLETOFINESHIFT) * FINERADIANS);
    n1[1] = 0;
    n1[2] = 0;

    n2[0] = F2D3D(-plane->a);
    n2[1] = F2D3D(-plane->b);
    n2[2] = F2D3D(-plane->c);

    an = (float)acos(n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2]);

    return ((angle_t)(an / FINERADIANS) << ANGLETOFINESHIFT) - ANG90;
}

//
// M_MovePlane
//

void M_MovePlane(plane_t* plane, fixed_t height)
{
    plane->d = plane->d - FixedMul(height, plane->c);
}

//
// M_DiffPlaneHeight
//

fixed_t M_DiffPlaneHeight(plane_t* plane, fixed_t diff)
{
    return FixedMul(diff - plane->d, plane->nc);
}