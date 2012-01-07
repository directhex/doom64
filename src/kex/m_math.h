// Emacs style mode select	 -*- C++ -*-
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
//-----------------------------------------------------------------------------

#ifndef __M_MATH_H__
#define __M_MATH_H__

void M_CrossProduct(float *out, float *vec1, float *vec2);
void M_Normalize3(float *out);
fixed_t M_DotProduct(fixed_t a1, fixed_t b1, fixed_t c1, fixed_t a2, fixed_t b2, fixed_t c2);
fixed_t M_PointToZ(plane_t* plane, fixed_t x, fixed_t y);
fixed_t M_FacePlaneDistance(plane_t* plane, fixed_t x, fixed_t y, fixed_t z);
angle_t M_AlignPitchToPlane(plane_t* plane, angle_t angle, float threshold);
void M_MovePlane(plane_t* plane, fixed_t height);
fixed_t M_DiffPlaneHeight(plane_t* plane, fixed_t diff);

#endif /*__M_MATH_H__*/