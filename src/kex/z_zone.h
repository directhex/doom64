// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2000 James Haley
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//--------------------------------------------------------------------------
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//      Remark: this was the only stuff that, according
//       to John Carmack, might have been useful for
//       Quake.
//
// Rewritten by Lee Killough, though, since it was not efficient enough.
//
//---------------------------------------------------------------------

#ifndef __Z_ZONE__
#define __Z_ZONE__

#include "d_keywds.h" // haleyjd 05/22/02

// Remove all definitions before including system definitions

#undef malloc
#undef free
#undef realloc
#undef calloc
#undef strdup

// Include system definitions so that prototypes become
// active before macro replacements below are in effect.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef LINUX
// Linux needs strings.h too, for strcasecmp etc.
#include <strings.h>
#endif
#include <assert.h>

#include "psnprntf.h"

// Uncomment this to log all memory operations to a file
//#define ZONEFILE

// Uncomment this to see real-time memory allocation
// statistics, and to enable extra debugging features
//#define INSTRUMENTED

// ZONE MEMORY

// PU - purge tags.
enum 
{
   PU_FREE,    // block is free
   PU_STATIC,  // block is static (remains until explicitly freed)
   PU_MAPLUMP, // block is allocated for data stored in map wads
   PU_AUDIO,   // allocation of midi data
   PU_LEVEL,   // allocation belongs to level (freed at next level load)
   PU_LEVSPEC, // used for thinker_t's (same as PU_LEVEL basically)
   PU_CACHE,   // block is cached (may be implicitly freed at any time!)

   PU_MAX      // Must always be last -- killough
};

#define PU_PURGELEVEL PU_CACHE        /* First purgable tag's level */

void*   (Z_Malloc)(size_t size, int tag, void **ptr, const char *, int);
void    (Z_Free)(void *ptr, const char *, int);
void    (Z_FreeTags)(int lowtag, int hightag, const char *, int);
void    (Z_ChangeTag)(void *ptr, int tag, const char *, int);
void    (Z_Init)(void);
void*   (Z_Calloc)(size_t n, int tag, void **user, const char *, int);
void*   (Z_Realloc)(void *p, size_t n, int tag, void **user, const char *, int);
char*   (Z_Strdup)(const char *s, int tag, void **user, const char *, int);
void*   (Z_Alloca)(size_t n, const char *file, int line);
void    (Z_CheckHeap)(const char *,int);   // killough 3/22/98: add file/line info
int     (Z_CheckTag)(void *,const char *,int);
void    (Z_Touch)(void *ptr, const char *, int);

void Z_DumpHistory(char *);

#define Z_Free(a)          (Z_Free)     (a,      __FILE__,__LINE__)
#define Z_FreeTags(a,b)    (Z_FreeTags) (a,b,    __FILE__,__LINE__)
#define Z_ChangeTag(a,b)   (Z_ChangeTag)(a,b,    __FILE__,__LINE__)
#define Z_Malloc(a,b,c)    (Z_Malloc)   (a,b,c,  __FILE__,__LINE__)
#define Z_Strdup(a,b,c)    (Z_Strdup)   (a,b,c,  __FILE__,__LINE__)
#define Z_Calloc(a,b,c)    (Z_Calloc)   (a,b,c,  __FILE__,__LINE__)
#define Z_Realloc(a,b,c,d) (Z_Realloc)  (a,b,c,d,__FILE__,__LINE__)
#define Z_Alloca(a)        (Z_Alloca)   (a,      __FILE__,__LINE__)
#define Z_CheckHeap()      (Z_CheckHeap)(        __FILE__,__LINE__)
#define Z_CheckTag(a)      (Z_CheckTag) (a,      __FILE__,__LINE__)
#define Z_Touch(a)         (Z_Touch)    (a,      __FILE__,__LINE__)

#define malloc(n)          (Z_Malloc) (n,    PU_STATIC,0,__FILE__,__LINE__)
#define free(p)            (Z_Free)   (p,                __FILE__,__LINE__)
#define realloc(p,n)       (Z_Realloc)(p,n,  PU_STATIC,0,__FILE__,__LINE__)
#define calloc(n1)          (Z_Calloc)(n1,   PU_STATIC,0,__FILE__,__LINE__)
#define strdup(s)          (Z_Strdup) (s,    PU_STATIC,0,__FILE__,__LINE__)

int Z_TagUsage(int tag);
void Z_ZoneHistory(char *);
void Z_PrintZoneHeap(void);
void Z_DumpCore(void);
int Z_FreeMemory(void);
void Z_FreeAlloca(void);

#ifdef ZONEFILE
void Z_LogPrintf(const char *msg, ...);
#endif

#ifdef INSTRUMENTED
void Z_PrintStats(void);
#endif

#endif

