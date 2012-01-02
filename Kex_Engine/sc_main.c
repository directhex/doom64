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
// DESCRIPTION:
//
//
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif

#ifdef _MSC_VER
#include "i_opndir.h"
#else
#include <dirent.h>
#endif

#include "doomdef.h"
#include "doomtype.h"
#include "z_zone.h"
#include "w_wad.h"
#include "m_misc.h"
#include "sc_main.h"

scparser_t sc_parser;

//
// SC_Open
//

static void SC_Open(char* name)
{
    int lump;

    lump = W_CheckNumForName(name);

    if(lump <= -1)
    {
        sc_parser.buffsize   = M_ReadFile(name, &sc_parser.buffer);

        if(sc_parser.buffsize == -1)
            I_Error("SC_Open: %s not found", name);
    }
    else
    {
        sc_parser.buffer     = W_CacheLumpNum(lump, PU_STATIC);
        sc_parser.buffsize   = W_LumpLength(lump);
    }

    sc_parser.pointer_start  = sc_parser.buffer;
    sc_parser.pointer_end    = sc_parser.buffer + sc_parser.buffsize;
    sc_parser.linepos        = 1;
    sc_parser.rowpos         = 1;
    sc_parser.buffpos        = 0;
}

//
// SC_Close
//

static void SC_Close(void)
{
    Z_Free(sc_parser.buffer);

    sc_parser.buffer         = NULL;
    sc_parser.buffsize       = 0;
    sc_parser.pointer_start  = NULL;
    sc_parser.pointer_end    = NULL;
    sc_parser.linepos        = 0;
    sc_parser.rowpos         = 0;
    sc_parser.buffpos        = 0;
}

//
// SC_Compare
//

static void SC_Compare(char* token)
{
    sc_parser.find(false);
    if(dstricmp(sc_parser.token, token))
    {
        I_Error("SC_Compare: Expected '%s', found '%s' (line = %i, pos = %i)",
        token, sc_parser.token, sc_parser.linepos, sc_parser.rowpos);
    }
}

//
// SC_ReadTokens
//

static int SC_ReadTokens(void)
{
    return (sc_parser.buffpos < sc_parser.buffsize);
}

//
// SC_GetString
//

static char* SC_GetString(void)
{
    sc_parser.compare("="); // expect a '='
    sc_parser.find(false);  // get next token which should be a string

    return sc_parser.token;
}

//
// SC_GetInteger
//

static int SC_GetInteger(void)
{
    sc_parser.compare("="); // expect a '='
    sc_parser.find(false);  // get next token which should be an integer

    return datoi(sc_parser.token);
}

//
// SC_SetData
//

static int SC_SetData(byte* data, const scdatatable_t* table)
{
    int i;
    dboolean ok = false;

    for(i = 0; table[i].token; i++)
    {
        if(!dstricmp(table[i].token, sc_parser.token))
        {
            byte* pointer = ((byte*)data + table[i].ptroffset);
            char* name;
            byte rgb[3];

            ok = true;

            switch(table[i].type)
            {
            case 's':
                name = sc_parser.getstring();
                dstrncpy((char*)pointer, name, dstrlen(name));
                break;
            case 'S':
                name = sc_parser.getstring();
                dstrupr(name);
                dstrncpy((char*)pointer, name, dstrlen(name));
                break;
            case 'i':
                *(int*)pointer = sc_parser.getint();
                break;
            case 'b':
                *(int*)pointer = true;
                break;
            case 'c':
                sc_parser.compare("="); // expect a '='
                sc_parser.find(false);
                rgb[0] = dhtoi(sc_parser.token);
                sc_parser.find(false);
                rgb[1] = dhtoi(sc_parser.token);
                sc_parser.find(false);
                rgb[2] = dhtoi(sc_parser.token);
                *(rcolor*)pointer = D_RGBA(rgb[0], rgb[1], rgb[2], 0xFF);
                break;
            }

            break;
        }
    }

    return ok;
}

//
// SC_Find
//

static int SC_Find(dboolean forceupper)
{
    char c = 0;
    int i = 0;
    dboolean comment = false;
    dboolean havetoken = false;
    dboolean string = false;

    dmemset(sc_parser.token, 0, 256);

    while(sc_parser.readtokens())
    {
        c = sc_parser.getchar();

        if(c == '/')
            comment = true;

        if(comment == false)
        {
            if(c == '"')
            {
                if(!string)
                {
                    string = true;
                    continue;
                }
                else if(havetoken)
                    return true;
            }

            if(!string)
            {
                if(c > ' ')
                {
                    havetoken = true;
                    sc_parser.token[i++] =
                        forceupper ? toupper(c) : c;
                }
                else if(havetoken)
                    return true;
            }
            else
            {
                if(c >= ' ')
                {
                    havetoken = true;
                    sc_parser.token[i++] =
                        forceupper ? toupper(c) : c;
                }
            }
        }

        if(c == '\n')
        {
            sc_parser.linepos++;
            sc_parser.rowpos = 1;
            comment = false;
            if(string)
                sc_parser.token[i++] = c;
        }
    }

    return false;
}

//
// SC_GetChar
//

static char SC_GetChar(void)
{
    sc_parser.rowpos++;
    return sc_parser.buffer[sc_parser.buffpos++];
}

//
// SC_Error
//

static void SC_Error(char* function)
{
    if(sc_parser.token[0] < ' ')
        return;

    I_Error("%s: Unknown token: '%s' (line = %i, pos = %i)",
        function, sc_parser.token, sc_parser.linepos, sc_parser.rowpos);
}

//
// SC_Init
//

void SC_Init(void)
{
    //
    // clear variables
    //

    dmemset(&sc_parser, 0, sizeof(scparser_t));

    //
    // setup lexer routines
    //

    sc_parser.open          = SC_Open;
    sc_parser.close         = SC_Close;
    sc_parser.compare       = SC_Compare;
    sc_parser.find          = SC_Find;
    sc_parser.getchar       = SC_GetChar;
    sc_parser.getstring     = SC_GetString;
    sc_parser.getint        = SC_GetInteger;
    sc_parser.setdata       = SC_SetData;
    sc_parser.readtokens    = SC_ReadTokens;
    sc_parser.error         = SC_Error;
}


