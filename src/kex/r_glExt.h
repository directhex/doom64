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
// DESCRIPTION: OpenGL extensions
//
//-----------------------------------------------------------------------------

#ifndef __R_GLEXT_H__
#define __R_GLEXT_H__

#include "SDL_opengl.h"

extern PFNGLMULTITEXCOORD2FARBPROC      glExtMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC     glExtMultiTexCoord2fvARB;
extern PFNGLACTIVETEXTUREARBPROC        glExtActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC  glExtClientActiveTextureARB;
extern PFNGLLOCKARRAYSEXTPROC           glExtLockArrays;
extern PFNGLUNLOCKARRAYSEXTPROC         glExtUnlockArrays;
extern PFNGLMULTIDRAWARRAYSEXTPROC      glExtMultiDrawArrays;
extern PFNGLFOGCOORDFEXTPROC            glExtFogCoordf;
extern PFNGLFOGCOORDPOINTEREXTPROC      glExtFogCoordPointer;
extern PFNGLBINDBUFFERARBPROC           glExtBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC        glExtDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC           glExtGenBuffersARB;
extern PFNGLBUFFERDATAARBPROC           glExtBufferDataARB;
extern PFNGLMAPBUFFERARBPROC            glExtMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC          glExtUnmapBufferARB;

void R_GLInitExtensions(void);
dboolean R_GLCheckExt(const char *ext);

#endif