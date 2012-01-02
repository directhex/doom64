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
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "SDL.h"
#include "SDL_opengl.h"

#include "doomdef.h"
#include "doomstat.h"
#include "r_gl.h"
#include "i_system.h"
#include "r_main.h"
#include "con_console.h"

// ======================== OGL Extensions ===================================

PFNGLMULTITEXCOORD2FARBPROC         glExtMultiTexCoord2fARB         = NULL;
PFNGLMULTITEXCOORD2FVARBPROC        glExtMultiTexCoord2fvARB        = NULL;
PFNGLACTIVETEXTUREARBPROC           glExtActiveTextureARB           = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC     glExtClientActiveTextureARB     = NULL;
PFNGLLOCKARRAYSEXTPROC              glExtLockArrays                 = NULL;
PFNGLUNLOCKARRAYSEXTPROC            glExtUnlockArrays               = NULL;
PFNGLMULTIDRAWARRAYSEXTPROC         glExtMultiDrawArrays            = NULL;
PFNGLFOGCOORDFEXTPROC               glExtFogCoordf                  = NULL;
PFNGLFOGCOORDPOINTEREXTPROC         glExtFogCoordPointer            = NULL;
PFNGLBINDBUFFERARBPROC              glExtBindBufferARB              = NULL;
PFNGLDELETEBUFFERSARBPROC           glExtDeleteBuffersARB           = NULL;
PFNGLGENBUFFERSARBPROC              glExtGenBuffersARB              = NULL;
PFNGLBUFFERDATAARBPROC              glExtBufferDataARB              = NULL;
PFNGLMAPBUFFERARBPROC               glExtMapBufferARB               = NULL;
PFNGLUNMAPBUFFERARBPROC             glExtUnmapBufferARB             = NULL;

//
// R_GLCheckExtension
//

dboolean R_GLCheckExtension(const char *ext, dboolean required)
{
    if(R_GLCheckExt(ext))
    {
        CON_Printf(WHITE, "GL Extension: %s = true\n", ext);
        return true;
    }
    else
    {
        if(required)
            I_Error("R_GLCheckExtension: missing extension %s is required", ext);
        else
            CON_Printf(YELLOW, "GL Extension: %s = false\n", ext);
    }
    
    return false;
}

//
// R_GLRegisterProc
//

void* R_GLRegisterProc(const char *address, dboolean required)
{
    void *proc = SDL_GL_GetProcAddress(address);
    
    if(!proc && required)
        I_Error("R_GLRegisterProc: Failed to get proc address: %s", address);
    
    return proc;
}

//
// R_GLInitExtensions
//

void R_GLInitExtensions(void)
{
    R_GLCheckExtension("GL_ARB_multitexture", true);
    R_GLCheckExtension("GL_EXT_multi_draw_arrays", false);
    R_GLCheckExtension("GL_ARB_vertex_buffer_object", false);
    R_GLCheckExtension("GL_EXT_fog_coord", false);
    R_GLCheckExtension("GL_ARB_texture_non_power_of_two", false);
    R_GLCheckExtension("GL_ARB_vertex_program", false);
    R_GLCheckExtension("GL_ARB_fragment_program", false);

    if(!R_GLCheckExtension("GL_ARB_texture_env_add", false))
        R_GLCheckExtension("GL_EXT_texture_env_add", true);

    if(!R_GLCheckExtension("GL_ARB_texture_env_combine", false))
        R_GLCheckExtension("GL_EXT_texture_env_combine", true);

    glExtMultiTexCoord2fARB         = R_GLRegisterProc("glMultiTexCoord2fARB", true);
    glExtMultiTexCoord2fvARB        = R_GLRegisterProc("glMultiTexCoord2fvARB", true);
    glExtActiveTextureARB           = R_GLRegisterProc("glActiveTextureARB", true);
    glExtClientActiveTextureARB     = R_GLRegisterProc("glClientActiveTextureARB", true);
    glExtLockArrays                 = R_GLRegisterProc("glLockArraysEXT", false);
    glExtUnlockArrays               = R_GLRegisterProc("glUnlockArraysEXT", false);
    glExtMultiDrawArrays            = R_GLRegisterProc("glMultiDrawArraysEXT", false);
    glExtFogCoordf                  = R_GLRegisterProc("glFogCoordfEXT", false);
    glExtFogCoordPointer            = R_GLRegisterProc("glFogCoordPointerEXT", false);
    glExtBindBufferARB              = R_GLRegisterProc("glBindBufferARB", false);
    glExtDeleteBuffersARB           = R_GLRegisterProc("glDeleteBuffersARB", false);
    glExtGenBuffersARB              = R_GLRegisterProc("glGenBuffersARB", false);
    glExtBufferDataARB              = R_GLRegisterProc("glBufferDataARB", false);
    glExtMapBufferARB               = R_GLRegisterProc("glMapBufferARB", false);
    glExtUnmapBufferARB             = R_GLRegisterProc("glUnmapBufferARB", false);
}

//
// R_GLCheckExt
//

dboolean R_GLCheckExt(const char *ext)
{
    const byte *extensions = NULL;
    const byte *start;
    byte *where, *terminator;
    
    // Extension names should not have spaces.
    where = (byte *) strchr(ext, ' ');
    if (where || *ext == '\0')
        return 0;
    
    extensions = dglGetString(GL_EXTENSIONS);
    
    start = extensions;
    for(;;)
    {
        where = (byte *)strstr((const char *) start, ext);
        if(!where)
            break;
        terminator = where + dstrlen(ext);
        if(where == start || *(where - 1) == ' ')
        {
            if(*terminator == ' ' || *terminator == '\0')
                return true;
            start = terminator;
        }
    }
    return false;
}
