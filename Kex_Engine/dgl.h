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

#ifndef __DGL_H__
#define __DGL_H__

#include <math.h>

#include "SDL_opengl.h"
#include "r_gl.h"
#include "r_glExt.h"
#include "i_system.h"

//#define LOG_GLFUNC_CALLS
//#define DEBUG_GLFUNC_CALLS

#ifdef DEBUG_GLFUNC_CALLS
static void dglGetError(const char *message)
{
    GLint err = glGetError();
    if(err != GL_NO_ERROR)
    {
        I_Printf("\nGL ERROR %d on gl function: %s\n\n", err, message);
        I_Sleep(1000);
    }
}
#endif

d_inline static void dglVertex3i (GLint x, GLint y, GLint z)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glVertex3i(x=%i, y=%i, z=%i)\n", x, y, z);
#endif
    glVertex3i(x, y, z);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glVertex3i");
#endif
}

d_inline static void dglClear (GLbitfield mask)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glClear(mask=0x%x)\n", mask);
#endif
    glClear(mask);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glClear");
#endif
}

d_inline static void dglGetBooleanv (GLenum pname, GLboolean *params)
{
    glGetBooleanv(pname, params);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glGetBooleanv");
#endif
}

d_inline static void dglColor4ubv (const GLubyte *v)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glColor4ubv(v=%p)\n", v);
#endif
    glColor4ubv(v);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glColor4ubv");
#endif
}

d_inline static void dglScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glScissor(x=0x%x, y=0x%x, width=0x%x, height=0x%x)\n", x, y, width, height);
#endif
    glScissor(x, y, width, height);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glScissor");
#endif
}

d_inline static void dglViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glViewport(x=0x%x, y=0x%x, width=0x%x, height=0x%x)\n", x, y, width, height);
#endif
    glViewport(x, y, width, height);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glViewport");
#endif
}

d_inline static void dglMultMatrixf (const GLfloat *m)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glMultMatrixf(n=%p)\n", m);
#endif
    glMultMatrixf(m);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glMultMatrixf");
#endif
}

d_inline static void dglFogf (GLenum pname, GLfloat param)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glFogf(pname=0x%x, param=%f)\n", pname, param);
#endif
    glFogf(pname, param);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glFogf");
#endif
}

d_inline static void dglFogi (GLenum pname, GLint param)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glFogi(pname=0x%x, param=0x%x)\n", pname, param);
#endif
    glFogi(pname, param);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glFogi");
#endif
}

d_inline static void dglBlendFunc (GLenum sfactor, GLenum dfactor)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glBlendFunc(sfactor=0x%x, dfactor=0x%x)\n", sfactor, dfactor);
#endif
    glBlendFunc(sfactor, dfactor);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glBlendFunc");
#endif
}

d_inline static void dglAlphaFunc (GLenum func, GLclampf ref)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glAlphaFunc(func=0x%x, ref=0x%x)\n", func, ref);
#endif
    glAlphaFunc(func, ref);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glAlphaFunc");
#endif
}

d_inline static void dglDepthFunc (GLenum func)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDepthFunc(func=0x%x)\n", func);
#endif
    glDepthFunc(func);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDepthFunc");
#endif
}

d_inline static void dglHint (GLenum target, GLenum mode)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glHint(target=0x%x, mode=0x%x)\n", target, mode);
#endif
    glHint(target, mode);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glHint");
#endif
}

d_inline static void dglShadeModel (GLenum mode)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glShadeModel(mode=0x%x)\n", mode);
#endif
    glShadeModel(mode);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glShadeModel");
#endif
}

d_inline static void dglDeleteTextures (GLsizei n, const GLuint *textures)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDeleteTextures(n=0x%x, textures=%p)\n", n, textures);
#endif
    glDeleteTextures(n, textures);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDeleteTextures");
#endif
}

d_inline static void dglClearDepth (GLclampd depth)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glClearDepth(depth=0x%x)\n", depth);
#endif
    glClearDepth(depth);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glClearDepth");
#endif
}

d_inline static void dglFogfv (GLenum pname, const GLfloat *params)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glFogfv(pname=0x%x, params=%p)\n", pname, params);
#endif
    glFogfv(pname, params);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glFogfv");
#endif
}

d_inline static void dglTranslated (GLdouble x, GLdouble y, GLdouble z)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTranslated(x=%f, y=%f, z=%f)\n", x, y, z);
#endif
    glTranslated(x, y, z);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTranslated");
#endif
}

d_inline static void dglTexEnvfv (GLenum target, GLenum pname, const GLfloat *params)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexEnvfv(target=0x%x, pname=0x%x, params=%p)\n", target, pname, params);
#endif
    glTexEnvfv(target, pname, params);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexEnvfv");
#endif
}

d_inline static void dglDisableClientState (GLenum array)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDisableClientState(array=0x%x)\n", array);
#endif
    glDisableClientState(array);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDisableClientState");
#endif
}

d_inline static void dglDrawArrays (GLenum mode, GLint first, GLsizei count)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDrawArrays(mode=0x%x, first=0x%x, count=0x%x)\n", mode, first, count);
#endif
    glDrawArrays(mode, first, count);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDrawArrays");
#endif
}

d_inline static void dglDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDrawElements(mode=0x%x, count=0x%x, type=0x%x, indices=%p)\n", mode, count, type, indices);
#endif
    glDrawElements(mode, count, type, indices);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDrawElements");
#endif
}

d_inline static void dglColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glColorPointer(size=0x%x, type=0x%x, stride=0x%x, pointer=%p)\n", size, type, stride, pointer);
#endif
    glColorPointer(size, type, stride, pointer);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glColorPointer");
#endif
}

d_inline static void dglVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glVertexPointer(size=0x%x, type=0x%x, stride=0x%x, stride=%p)\n", size, type, stride, pointer);
#endif
    glVertexPointer(size, type, stride, pointer);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glVertexPointer");
#endif
}

d_inline static void dglTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexCoordPointer(size=0x%x, type=0x%x, stride=0x%x, stride=%p)\n", size, type, stride, pointer);
#endif
    glTexCoordPointer(size, type, stride, pointer);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexCoordPointer");
#endif
}

d_inline static void dglEnableClientState (GLenum array)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glEnableClientState(array=0x%x)\n", array);
#endif
    glEnableClientState(array);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glEnableClientState");
#endif
}

d_inline static void dglVertex2fv (const GLfloat *v)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glVertex2fv(v=%p)\n", v);
#endif
    glVertex2fv(v);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glVertex2fv");
#endif
}

d_inline static void dglTexCoord2fv (const GLfloat *v)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexCoord2fv(v=%p)\n", v);
#endif
    glTexCoord2fv(v);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexCoord2fv");
#endif
}

d_inline static void dglVertex2i (GLint x, GLint y)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glVertex2i(x=%i, y=%i)\n", x, y);
#endif
    glVertex2i(x, y);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glVertex2i");
#endif
}

d_inline static void dglColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glColor4f(red=%f, green=%f, blue=%f, alpha=%f)\n", red, green, blue, alpha);
#endif
    glColor4f(red, green, blue, alpha);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glColor4f");
#endif
}

d_inline static void dglTexEnvf (GLenum target, GLenum pname, GLfloat param)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexEnvf(target=0x%x, pname=0x%x, param=%f)\n", target, pname, param);
#endif
    glTexEnvf(target, pname, param);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexEnvf");
#endif
}

d_inline static void dglTexEnvi (GLenum target, GLenum pname, GLint param)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexEnvi(target=0x%x, pname=0x%x, param=0x%x)\n", target, pname, param);
#endif
    glTexEnvi(target, pname, param);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexEnvi");
#endif
}

d_inline static void dglRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glRotatef(angle=%f, x=%f, y=%f, z=%f)\n", angle, x, y, z);
#endif
    glRotatef(angle, x, y, z);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glRotatef");
#endif
}

d_inline static void dglTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTranslatef(x=%f, y=%f, z=%f)\n", x, y, z);
#endif
    glTranslatef(x, y, z);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTranslatef");
#endif
}

d_inline static void dglPushMatrix (void)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glPushMatrix\n");
#endif
    glPushMatrix();
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glPushMatrix");
#endif
}

d_inline static void dglLoadIdentity (void)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glLoadIdentity\n");
#endif
    glLoadIdentity();
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glLoadIdentity");
#endif
}

d_inline static void dglMatrixMode (GLenum mode)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glMatrixMode(n=0x%x)\n", mode);
#endif
    glMatrixMode(mode);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glMatrixMode");
#endif
}

d_inline static void dglDisable (GLenum cap)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDisable(cap=0x%x)\n", cap);
#endif
    glDisable(cap);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDisable");
#endif
}

d_inline static void dglClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glClearColor(red=0x%x, green=0x%x, blue=0x%x, alpha=0x%x)\n", red, green, blue, alpha);
#endif
    glClearColor(red, green, blue, alpha);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glClearColor");
#endif
}

d_inline static void dglEnable (GLenum cap)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glEnable(cap=0x%x)\n", cap);
#endif
    glEnable(cap);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glEnable");
#endif
}

d_inline static void dglPopMatrix (void)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glPopMatrix\n");
#endif
    glPopMatrix();
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glPopMatrix");
#endif
}

d_inline static void dglVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glVertex3f(x=%f, y=%f, z=%f)\n", x, y, z);
#endif
    glVertex3f(x, y, z);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glVertex3f");
#endif
}

d_inline static void dglTexCoord2f (GLfloat s, GLfloat t)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexCoord2f(s=%f, t=%f)\n", s, t);
#endif
    glTexCoord2f(s, t);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexCoord2f");
#endif
}

d_inline static void dglColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glColor4ub(red=0x%x, green=0x%x, blue=0x%x, alpha=0x%x)\n", red, green, blue, alpha);
#endif
    glColor4ub(red, green, blue, alpha);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glColor4ub");
#endif
}

d_inline static void dglColor3ub (GLubyte red, GLubyte green, GLubyte blue)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glColor3ub(red=0x%x, green=0x%x, blue=0x%x)\n", red, green, blue);
#endif
    glColor3ub(red, green, blue);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glColor3ub");
#endif
}

d_inline static void dglPolygonMode (GLenum face, GLenum mode)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glPolygonMode(face=0x%x, mode=0x%x)\n", face, mode);
#endif
    glPolygonMode(face, mode);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glPolygonMode");
#endif
}

d_inline static void dglVertex2f (GLfloat x, GLfloat y)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glVertex2f(x=%f, y=%f)\n", x, y);
#endif
    glVertex2f(x, y);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glVertex2f");
#endif
}

d_inline static void dglScalef (GLfloat x, GLfloat y, GLfloat z)
{
    glScalef(x, y, z);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glScalef");
#endif
}

d_inline static void dglOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    glOrtho(left, right, bottom, top, zNear, zFar);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glOrtho");
#endif
}

d_inline static void dglFinish (void)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glFinish\n");
#endif
    glFinish();
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glFinish");
#endif
}

d_inline static void dglReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    glReadPixels(x, y, width, height, format, type, pixels);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glReadPixels");
#endif
}

d_inline static void dglCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glCopyTexSubImage2D");
#endif
}

d_inline static void dglTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexImage2D");
#endif
}

d_inline static void dglTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexSubImage2D");
#endif
}

d_inline static void dglDepthMask (GLboolean flag)
{
    glDepthMask(flag);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDepthMask");
#endif
}

d_inline static void dglDepthRange (GLclampd zNear, GLclampd zFar)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glDepthRange(zNear=%f, zFar=%f)\n", zNear, zFar);
#endif
    glDepthRange(zNear, zFar);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glDepthRange");
#endif
}

d_inline static void dglGenTextures (GLsizei n, GLuint *textures)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glGenTextures(n=0x%x, textures=%p)\n", n, textures);
#endif
    glGenTextures(n, textures);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glGenTextures");
#endif
}

d_inline static void dglBindTexture (GLenum target, GLuint texture)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glBindTexture(target=0x%x, texture=0x%x)\n", target, texture);
#endif
    glBindTexture(target, texture);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glBindTexture");
#endif
}

d_inline static void dglTexParameteri (GLenum target, GLenum pname, GLint param)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glTexParameteri(target=0x%x, pname=0x%x, param=0x%x)\n", target, pname, param);
#endif
    glTexParameteri(target, pname, param);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glTexParameteri");
#endif
}

d_inline static void dglCullFace (GLenum mode)
{
    glCullFace(mode);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glCullFace");
#endif
}

d_inline static void dglRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glRectf(x1=%f, y1=%f, x2=%f, y2=%f)\n", x1, y1, x2, y2);
#endif
    glRectf(x1, y1, x2, y2);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glRectf");
#endif
}

d_inline static void dglRecti (GLint x1, GLint y1, GLint x2, GLint y2)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glRecti(x1=%i, y1=%i, x2=%i, y2=%i)\n", x1, y1, x2, y2);
#endif
    glRecti(x1, y1, x2, y2);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glRecti");
#endif
}

d_inline static void dglMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t)
{
    glExtMultiTexCoord2fARB(target, s, t);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtMultiTexCoord2fARB");
#endif
}

d_inline static void dglMultiTexCoord2fv (GLenum target, const GLfloat *v)
{
    glExtMultiTexCoord2fvARB(target, v);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtMultiTexCoord2fvARB");
#endif
}

d_inline static void dglActiveTexture (GLenum texture)
{
    glExtActiveTextureARB(texture);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtActiveTextureARB");
#endif
}

d_inline static void dglClientActiveTexture (GLenum texture)
{
    glExtClientActiveTextureARB(texture);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtClientActiveTextureARB");
#endif
}

d_inline static void dglLockArrays (GLint i, GLsizei n)
{
    if(glExtLockArrays)
    {
        glExtLockArrays(i, n);
#ifdef DEBUG_GLFUNC_CALLS
        dglGetError("glExtLockArrays");
#endif
    }
}

d_inline static void dglUnlockArrays (void)
{
    if(glExtUnlockArrays)
    {
        glExtUnlockArrays();
#ifdef DEBUG_GLFUNC_CALLS
        dglGetError("glExtUnlockArrays");
#endif
    }
}

d_inline static void dglGetDoublev (GLenum pname, GLdouble *params)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glGetDoublev(pname=0x%x, params=%p)\n", pname, params);
#endif
    glGetDoublev(pname, params);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glGetDoublev");
#endif
}

d_inline static void dglMultiDrawArrays (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glMultiDrawArrays(mode=0x%x, first=%p, count=%p, primcount=0x%x)\n", mode, first, count, primcount);
#endif
    glExtMultiDrawArrays(mode, first, count, primcount);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtMultiDrawArrays");
#endif
}

d_inline static void dglFogCoordf (GLfloat coord)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glFogCoordf(coord=%f)\n", coord);
#endif
    glExtFogCoordf(coord);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtFogCoordf");
#endif
}

d_inline static void dglFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
#ifdef LOG_GLFUNC_CALLS
    I_Printf("glFogCoordPointer(type=0x%x, stride=0x%x, pointer=%p)\n", type, stride, pointer);
#endif
    glExtFogCoordPointer(type, stride, pointer);
#ifdef DEBUG_GLFUNC_CALLS
    dglGetError("glExtFogCoordPointer");
#endif
}

#define dglBegin(mode)      glBegin(mode)
#define dglEnd()            glEnd()
#define dglGetString(name)  glGetString(name)

//
// CUSTOM ROUTINES
//

extern d_inline void dglSetVertex(vtx_t *vtx);
extern d_inline void dglTriangle(int v0, int v1, int v2);
extern d_inline void dglDrawGeometry(dword count, vtx_t *vtx);
extern d_inline void dglFrustum(int width, int height, rfloat fovy, rfloat znear);
extern d_inline void dglSetVertexColor(vtx_t *v, rcolor c, word count);
extern d_inline void dglGetColorf(rcolor color, float* argb);
extern d_inline void dglTexCombReplace(void);
extern d_inline void dglTexCombColor(int t, rcolor c, int func);
extern d_inline void dglTexCombColorf(int t, float* f, int func);
extern d_inline void dglTexCombModulate(int t, int s);
extern d_inline void dglTexCombAdd(int t, int s);
extern d_inline void dglTexCombInterpolate(int t, float a);
extern d_inline void dglTexCombReplaceAlpha(int t);


#endif // __DGL_H__

