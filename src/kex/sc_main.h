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
//
//-----------------------------------------------------------------------------


#ifndef __SC_MAIN__
#define __SC_MAIN__

typedef struct
{
    char    token[512];
    char*   buffer;
    char*   pointer_start;
    char*   pointer_end;
    int     linepos;
    int     rowpos;
    int     buffpos;
    int     buffsize;
    void    (*open)(void*);
    void    (*close)(void);
    void    (*compare)(void*);
    int     (*find)(dboolean);
    char    (*getchar)(void);
    char*   (*getstring)(void);
    int     (*getint)(void);
    int     (*setdata)(void*, void*);
    int     (*readtokens)(void);
    void    (*error)(void*);
} scparser_t;

extern scparser_t sc_parser;

typedef struct
{
    char*   token;
    int     ptroffset;
    char    type;
} scdatatable_t;

void SC_Init(void);

#endif // __SC_MAIN__