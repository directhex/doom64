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
//---------------------------------------------------------------------

#ifndef R_CLIPPER_H
#define R_CLIPPER_H

dboolean    R_Clipper_SafeCheckRange(angle_t startAngle, angle_t endAngle);
void        R_Clipper_SafeAddClipRange(angle_t startangle, angle_t endangle);
void        R_Clipper_Clear(void);

extern float frustum[6][4];

angle_t     R_FrustumAngle(void);
void        R_FrustrumSetup(void);
dboolean    R_FrustrumTestVertex(vtx_t* vertex, int count);

#endif