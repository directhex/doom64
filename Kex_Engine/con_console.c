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
// DESCRIPTION: Main console functions
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include "version.h"

#include "doomstat.h"
#include "con_console.h"
#include "z_zone.h"
#include "st_stuff.h"
#include "g_actions.h"
#include "m_shift.h"
#include "m_misc.h"
#include "i_system.h"

#define CONSOLE_PROMPTCHAR      '>'
#define MAX_CONSOLE_LINES       256//must be power of 2
#define CONSOLETEXT_MASK        (MAX_CONSOLE_LINES-1)
#define CMD_HISTORY_SIZE        64
#define CONSOLE_Y               160

typedef struct
{
    int		len;
    dword	color;
    char	line[1];
} conline_t;

enum
{
    CST_UP,
    CST_RAISE,
    CST_LOWER,
    CST_DOWN
};

#define     CON_BUFFERSIZE  100

conline_t   **ConsoleText;
int         ConsoleHead;
int         ConsoleMinLine;
dboolean    ConsoleOn = false;
int         ConsolePos=0;//bottom of console, in pixels
char        ConsoleLineBuffer[CON_BUFFERSIZE];
int         ConsoleLineLen;
dboolean    ConsoleState=CST_UP;
char        ConsoleInputBuff[MAX_CONSOLE_INPUT_LEN];
int         ConsoleInputLen;
int         PrevCommands[CMD_HISTORY_SIZE];
int         PrevCommandHead;
int         NextCommand;
dboolean    ConsoleInitialized = false;

//
// CON_Init
//

void CON_Init(void)
{
    int i;
    
    CON_CvarInit();
    
    ConsoleText = (conline_t **)Z_Malloc(sizeof(conline_t *) * MAX_CONSOLE_LINES, PU_STATIC, NULL);
    ConsoleHead = 0;
    ConsoleMinLine = 0;
    
    for(i = 0; i < MAX_CONSOLE_LINES; i++)
        ConsoleText[i] = NULL;
    
    for(i = 0; i < MAX_CONSOLE_INPUT_LEN; i++)
        ConsoleInputBuff[i] = 0;

    ConsoleLineLen = 0;
    ConsoleInputLen = 1;
    ConsoleInputBuff[0] = CONSOLE_PROMPTCHAR;
    
    for(i = 0; i < CMD_HISTORY_SIZE; i++)
        PrevCommands[i] = -1;
    
    PrevCommandHead = 0;
    NextCommand = 0;

#ifdef USESYSCONSOLE
    {
        extern HWND hwndBuffer;
        char *buff;
        int i = SendMessage(hwndBuffer, WM_GETTEXTLENGTH, 0, 0);

        buff = Z_Alloca(i);

        SendMessage(hwndBuffer, WM_GETTEXT, i, (LPARAM)buff);
        CON_AddText(buff);
    }
#endif

    ConsoleInitialized = true;
}

//
// CON_AddLine
//

void CON_AddLine(char *line, int len)
{
    conline_t   *cline;
    int         i;
    dboolean    recursed = false;
    
    if(!ConsoleLineBuffer)
        return;//not initialised yet
    if(recursed)
        return;//later call to Z_Malloc can fail and call I_Error/I_Printf...
    
    recursed = true;
    
    if(!line)
        return;
    if(len == -1)
        len = dstrlen(line);
    cline = (conline_t *)Z_Malloc(sizeof(conline_t)+len, PU_STATIC, NULL);
    cline->len = len;
    if(len)
        dmemcpy(cline->line, line, len);
    cline->line[len] = 0;
    ConsoleHead = (ConsoleHead + CONSOLETEXT_MASK) & CONSOLETEXT_MASK;
    ConsoleMinLine = ConsoleHead;
    
    ConsoleText[ConsoleHead] = cline;
    ConsoleText[ConsoleHead]->color = WHITE;
    
    i = (ConsoleHead + CONSOLETEXT_MASK) & CONSOLETEXT_MASK;
    if(ConsoleText[i])
    {
        Z_Free(ConsoleText[i]);
        ConsoleText[i] = NULL;
    }
    
    recursed = false;
}

//
// CON_AddText
//

void CON_AddText(char *text)
{
    char    *src;
    char    c;
    
    if(!ConsoleLineBuffer)
        return;
    
    src = text;
    c = *(src++);
    while(c)
    {
#ifdef USESYSCONSOLE
        if(*src == '\r' && *(src + 1) == '\n')
        {
            *src = 0x20;
            *(src + 1) = '\n';
        }
        else if(*src == '\r')
            *src = '\n';
#endif

        if((c == '\n') || (ConsoleLineLen >= CON_BUFFERSIZE))
        {
            CON_AddLine(ConsoleLineBuffer, ConsoleLineLen);
            ConsoleLineLen = 0;
        }
        if(c != '\n')
            ConsoleLineBuffer[ConsoleLineLen++] = c;
        
        c = *(src++);
    }
}

//
// CON_Printf
//

void CON_Printf(rcolor clr, const char *s, ...)
{
    static char msg[MAX_MESSAGE_SIZE];
    va_list	va;
    
    va_start(va, s);
    vsprintf(msg, s, va);
    va_end(va);
    
    I_Printf(msg);
    ConsoleText[ConsoleHead]->color = clr;
}

//
// CON_Warnf
//

void CON_Warnf(const char *s, ...)
{
    static char msg[MAX_MESSAGE_SIZE];
    va_list	va;
    
    va_start(va, s);
    vsprintf(msg, s, va);
    va_end(va);
    
    CON_Printf(YELLOW, msg);
}

//
// CON_ParseKey
//

static dboolean shiftdown = false;

void CON_ParseKey(char c)
{
    if(c == KEY_BACKSPACE)
    {
        if(ConsoleInputLen > 1)
            ConsoleInputBuff[--ConsoleInputLen] = 0;

        return;
    }
    
    if(shiftdown)
        c = shiftxform[c];

    if(ConsoleInputLen >= MAX_CONSOLE_INPUT_LEN - 2)
        ConsoleInputLen = MAX_CONSOLE_INPUT_LEN - 2;

    ConsoleInputBuff[ConsoleInputLen++] = c;
}

//
// CON_Ticker
//

static dboolean keyheld = false;
static dboolean lastevent = 0;
static int lastkey = 0;
static int ticpressed = 0;

void CON_Ticker(void)
{
    if(!ConsoleOn)
        return;

    if(keyheld && ((gametic - ticpressed) >= 15))
        CON_ParseKey(lastkey);
}

//
// CON_Responder
//

void G_ClearInput(void);

dboolean CON_Responder(event_t* ev)
{
    int c;
    dboolean clearheld = true;
    
    if((ev->type != ev_keyup) && (ev->type != ev_keydown))
        return false;
    
    c = ev->data1;
    lastkey = c;
    lastevent = ev->type;

    if(ev->type == ev_keydown && !keyheld)
    {
        keyheld = true;
        ticpressed = gametic;
    }
    else
    {
        keyheld = false;
        ticpressed = 0;
    }
    
    if(c == KEY_SHIFT)
    {
        if(ev->type == ev_keydown)
            shiftdown = true;
        else if(ev->type == ev_keyup)
            shiftdown = false;
    }
    
    switch(ConsoleState)
    {
    case CST_DOWN:
    case CST_LOWER:
        if(ev->type == ev_keydown)
        {
            switch(c)
            {
            case '`':
                ConsoleState = CST_UP;
                ConsoleOn = false;
                break;
                
            case KEY_ESCAPE:
                ConsoleInputLen = 1;
                break;
                
            case KEY_TAB:
                CON_CvarAutoComplete(&ConsoleInputBuff[1]);
                break;
                
            case KEY_ENTER:
                if(ConsoleInputLen <= 1)
                    break;
                
                ConsoleInputBuff[ConsoleInputLen]=0;
                CON_AddLine(ConsoleInputBuff, ConsoleInputLen);

                PrevCommands[PrevCommandHead] = ConsoleHead;
                PrevCommandHead++;
                NextCommand = PrevCommandHead;

                if(PrevCommandHead >= CMD_HISTORY_SIZE)
                    PrevCommandHead = 0;

                PrevCommands[PrevCommandHead] = -1;
                G_ExecuteCommand(&ConsoleInputBuff[1]);
                ConsoleInputLen = 1;
                CONCLEARINPUT();
                break;
                
            case KEY_UPARROW:
                c = NextCommand - 1;
                if(c < 0)
                    c = CMD_HISTORY_SIZE - 1;
                
                if(PrevCommands[c] == -1)
                    break;
                
                NextCommand = c;
                c = PrevCommands[NextCommand];
                if(ConsoleText[c])
                {
                    ConsoleInputLen = ConsoleText[c]->len;
                    dmemcpy(ConsoleInputBuff, ConsoleText[PrevCommands[NextCommand]]->line, ConsoleInputLen);
                }
                break;
                
            case KEY_DOWNARROW:
                if(PrevCommands[NextCommand] == -1)
                    break;
                
                c = NextCommand + 1;
                if(c >= CMD_HISTORY_SIZE)
                    c = 0;
                
                if(PrevCommands[c] == -1)
                    break;
                
                NextCommand = c;
                ConsoleInputLen = ConsoleText[PrevCommands[NextCommand]]->len;
                dmemcpy(ConsoleInputBuff, ConsoleText[PrevCommands[NextCommand]]->line, ConsoleInputLen);
                break;

            case KEY_MWHEELUP:
            case KEY_PAGEUP:
                if(ConsoleHead < MAX_CONSOLE_LINES)
                    ConsoleHead++;
                break;

            case KEY_MWHEELDOWN:
            case KEY_PAGEDOWN:
                if(ConsoleHead > ConsoleMinLine)
                    ConsoleHead--;
                break;
                
            default:
                if(c == KEY_SHIFT)
                    break;

                clearheld = false;
                CON_ParseKey(c);
                break;
            }

            if(clearheld)
            {
                keyheld = false;
                ticpressed = 0;
            }
        }
        return true;
        
    case CST_UP:
    case CST_RAISE:
        if(c == '`')
        {
            if(ev->type == ev_keydown)
            {
                ConsoleState = CST_DOWN;
                ConsoleOn = true;
                G_ClearInput();
            }
            return false;
        }
        break;
    }
    
    return false;
}

static const symboldata_t confontmap[256] =
{
    { 0, 1, 13, 16 },
    { 14, 1, 13, 16 },
    { 28, 1, 13, 16 },
    { 42, 1, 13, 16 },
    { 56, 1, 13, 16 },
    { 70, 1, 13, 16 },
    { 84, 1, 13, 16 },
    { 98, 1, 13, 16 },
    { 112, 1, 13, 16 },
    { 126, 1, 13, 16 },
    { 140, 1, 13, 16 },
    { 154, 1, 13, 16 },
    { 168, 1, 13, 16 },
    { 182, 1, 13, 16 },
    { 196, 1, 13, 16 },
    { 210, 1, 13, 16 },
    { 224, 1, 13, 16 },
    { 238, 1, 13, 16 },
    { 0, 18, 13, 16 },
    { 14, 18, 13, 16 },
    { 28, 18, 13, 16 },
    { 42, 18, 13, 16 },
    { 56, 18, 13, 16 },
    { 70, 18, 13, 16 },
    { 84, 18, 13, 16 },
    { 98, 18, 13, 16 },
    { 112, 18, 13, 16 },
    { 126, 18, 13, 16 },
    { 140, 18, 13, 16 },
    { 154, 18, 13, 16 },
    { 168, 18, 13, 16 },
    { 182, 18, 13, 16 },
    { 196, 18, 5, 16 },
    { 202, 18, 5, 16 },
    { 208, 18, 5, 16 },
    { 214, 18, 10, 16 },
    { 225, 18, 8, 16 },
    { 234, 18, 13, 16 },
    { 0, 35, 9, 16 },
    { 10, 35, 3, 16 },
    { 14, 35, 6, 16 },
    { 21, 35, 6, 16 },
    { 28, 35, 9, 16 },
    { 38, 35, 9, 16 },
    { 48, 35, 5, 16 },
    { 54, 35, 7, 16 },
    { 62, 35, 5, 16 },
    { 68, 35, 6, 16 },
    { 75, 35, 8, 16 },
    { 84, 35, 8, 16 },
    { 93, 35, 8, 16 },
    { 102, 35, 8, 16 },
    { 111, 35, 8, 16 },
    { 120, 35, 8, 16 },
    { 129, 35, 8, 16 },
    { 138, 35, 8, 16 },
    { 147, 35, 8, 16 },
    { 156, 35, 8, 16 },
    { 165, 35, 6, 16 },
    { 172, 35, 6, 16 },
    { 179, 35, 9, 16 },
    { 189, 35, 9, 16 },
    { 199, 35, 9, 16 },
    { 209, 35, 7, 16 },
    { 217, 35, 13, 16 },
    { 231, 35, 9, 16 },
    { 241, 35, 8, 16 },
    { 0, 52, 9, 16 },
    { 10, 52, 9, 16 },
    { 20, 52, 8, 16 },
    { 29, 52, 8, 16 },
    { 38, 52, 9, 16 },
    { 48, 52, 9, 16 },
    { 58, 52, 5, 16 },
    { 64, 52, 6, 16 },
    { 71, 52, 8, 16 },
    { 80, 52, 7, 16 },
    { 88, 52, 11, 16 },
    { 100, 52, 9, 16 },
    { 110, 52, 10, 16 },
    { 121, 52, 8, 16 },
    { 130, 52, 10, 16 },
    { 141, 52, 8, 16 },
    { 150, 52, 9, 16 },
    { 160, 52, 9, 16 },
    { 170, 52, 9, 16 },
    { 180, 52, 9, 16 },
    { 190, 52, 13, 16 },
    { 204, 52, 9, 16 },
    { 214, 52, 9, 16 },
    { 224, 52, 9, 16 },
    { 234, 52, 6, 16 },
    { 241, 52, 6, 16 },
    { 248, 52, 6, 16 },
    { 0, 69, 11, 16 },
    { 12, 69, 8, 16 },
    { 21, 69, 8, 16 },
    { 30, 69, 8, 16 },
    { 39, 69, 8, 16 },
    { 48, 69, 8, 16 },
    { 57, 69, 8, 16 },
    { 66, 69, 8, 16 },
    { 75, 69, 5, 16 },
    { 81, 69, 8, 16 },
    { 90, 69, 8, 16 },
    { 99, 69, 3, 16 },
    { 103, 69, 4, 16 },
    { 108, 69, 7, 16 },
    { 116, 69, 3, 16 },
    { 120, 69, 11, 16 },
    { 132, 69, 8, 16 },
    { 141, 69, 8, 16 },
    { 150, 69, 8, 16 },
    { 159, 69, 8, 16 },
    { 168, 69, 5, 16 },
    { 174, 69, 7, 16 },
    { 182, 69, 6, 16 },
    { 189, 69, 8, 16 },
    { 198, 69, 8, 16 },
    { 207, 69, 11, 16 },
    { 219, 69, 7, 16 },
    { 227, 69, 8, 16 },
    { 236, 69, 7, 16 },
    { 244, 69, 8, 16 },
    { 0, 86, 7, 16 },
    { 8, 86, 8, 16 },
    { 17, 86, 11, 16 },
    { 29, 86, 13, 16 },
    { 43, 86, 13, 16 },
    { 57, 86, 13, 16 },
    { 71, 86, 13, 16 },
    { 85, 86, 13, 16 },
    { 99, 86, 13, 16 },
    { 113, 86, 13, 16 },
    { 127, 86, 13, 16 },
    { 141, 86, 13, 16 },
    { 155, 86, 13, 16 },
    { 169, 86, 13, 16 },
    { 183, 86, 13, 16 },
    { 197, 86, 13, 16 },
    { 211, 86, 13, 16 },
    { 225, 86, 13, 16 },
    { 239, 86, 13, 16 },
    { 0, 103, 13, 16 },
    { 14, 103, 13, 16 },
    { 28, 103, 13, 16 },
    { 42, 103, 13, 16 },
    { 56, 103, 13, 16 },
    { 70, 103, 13, 16 },
    { 84, 103, 13, 16 },
    { 98, 103, 13, 16 },
    { 112, 103, 13, 16 },
    { 126, 103, 13, 16 },
    { 140, 103, 13, 16 },
    { 154, 103, 13, 16 },
    { 168, 103, 13, 16 },
    { 182, 103, 13, 16 },
    { 196, 103, 13, 16 },
    { 210, 103, 13, 16 },
    { 224, 103, 13, 16 },
    { 238, 103, 5, 16 },
    { 244, 103, 5, 16 },
    { 0, 120, 8, 16 },
    { 9, 120, 8, 16 },
    { 18, 120, 8, 16 },
    { 27, 120, 8, 16 },
    { 36, 120, 7, 16 },
    { 44, 120, 8, 16 },
    { 53, 120, 8, 16 },
    { 62, 120, 13, 16 },
    { 76, 120, 7, 16 },
    { 84, 120, 8, 16 },
    { 93, 120, 9, 16 },
    { 103, 120, 7, 16 },
    { 111, 120, 13, 16 },
    { 125, 120, 8, 16 },
    { 134, 120, 7, 16 },
    { 142, 120, 9, 16 },
    { 152, 120, 7, 16 },
    { 160, 120, 7, 16 },
    { 168, 120, 8, 16 },
    { 177, 120, 8, 16 },
    { 186, 120, 8, 16 },
    { 195, 120, 5, 16 },
    { 201, 120, 8, 16 },
    { 210, 120, 7, 16 },
    { 218, 120, 7, 16 },
    { 226, 120, 8, 16 },
    { 235, 120, 13, 16 },
    { 0, 137, 13, 16 },
    { 14, 137, 13, 16 },
    { 28, 137, 7, 16 },
    { 36, 137, 9, 16 },
    { 46, 137, 9, 16 },
    { 56, 137, 9, 16 },
    { 66, 137, 9, 16 },
    { 76, 137, 9, 16 },
    { 86, 137, 9, 16 },
    { 96, 137, 12, 16 },
    { 109, 137, 9, 16 },
    { 119, 137, 8, 16 },
    { 128, 137, 8, 16 },
    { 137, 137, 8, 16 },
    { 146, 137, 8, 16 },
    { 155, 137, 5, 16 },
    { 161, 137, 5, 16 },
    { 167, 137, 5, 16 },
    { 173, 137, 5, 16 },
    { 179, 137, 9, 16 },
    { 189, 137, 9, 16 },
    { 199, 137, 10, 16 },
    { 210, 137, 10, 16 },
    { 221, 137, 10, 16 },
    { 232, 137, 10, 16 },
    { 243, 137, 10, 16 },
    { 0, 154, 11, 16 },
    { 12, 154, 10, 16 },
    { 23, 154, 9, 16 },
    { 33, 154, 9, 16 },
    { 43, 154, 9, 16 },
    { 53, 154, 9, 16 },
    { 63, 154, 9, 16 },
    { 73, 154, 8, 16 },
    { 82, 154, 8, 16 },
    { 91, 154, 8, 16 },
    { 100, 154, 8, 16 },
    { 109, 154, 8, 16 },
    { 118, 154, 8, 16 },
    { 127, 154, 8, 16 },
    { 136, 154, 8, 16 },
    { 145, 154, 11, 16 },
    { 157, 154, 8, 16 },
    { 166, 154, 8, 16 },
    { 175, 154, 8, 16 },
    { 184, 154, 8, 16 },
    { 193, 154, 8, 16 },
    { 202, 154, 3, 16 },
    { 206, 154, 3, 16 },
    { 210, 154, 3, 16 },
    { 214, 154, 3, 16 },
    { 218, 154, 8, 16 },
    { 227, 154, 8, 16 },
    { 236, 154, 8, 16 },
    { 245, 154, 8, 16 },
    { 0, 171, 8, 16 },
    { 9, 171, 8, 16 },
    { 18, 171, 8, 16 },
    { 27, 171, 9, 16 },
    { 37, 171, 8, 16 },
    { 46, 171, 8, 16 },
    { 55, 171, 8, 16 },
    { 64, 171, 8, 16 },
    { 73, 171, 8, 16 },
    { 82, 171, 8, 16 },
    { 91, 171, 8, 16 },
    { 100, 171, 8, 16 }
};

//
// CON_DrawConsoleText
//

static vtx_t vtxstring[MAX_MESSAGE_SIZE];

static float CON_DrawConsoleText(float x, float y, rcolor color, float scale, const char* string, ...)
{
    int c = 0;
    int i = 0;
    int vi = 0;
    float vx1 = 0.0f;
    float vy1 = 0.0f;
    float vx2 = 0.0f;
    float vy2 = 0.0f;
    float tx1 = 0.0f;
    float tx2 = 0.0f;
    float ty1 = 0.0f;
    float ty2 = 0.0f;
    char msg[MAX_MESSAGE_SIZE];
    va_list	va;
    float width;
    float height;
    int pic;

    va_start(va, string);
    vsprintf(msg, string, va);
    va_end(va);
    
    pic = R_BindGfxTexture("CONFONT", true);

    width = (float)gfxwidth[pic];
    height = (float)gfxheight[pic];

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    dglSetVertex(vtxstring);
    
    R_GLToggleBlend(1);
    R_GLEnable2D(0);

    for(i = 0, vi = 0; i < dstrlen(msg); i++, vi += 4)
    {
        vx1 = x;
        vy1 = y;
        
        c = msg[i];
        if(c == '\n' || c == '\t')
            continue;   // villsa: safety check
        else
        {
            vx2 = vx1 + ((float)confontmap[c].w * scale);
            vy2 = vy1 - ((float)confontmap[c].h * scale);
            
            tx1 = ((float)confontmap[c].x / width) + 0.001f;
            tx2 = (tx1 + (float)confontmap[c].w / width) - 0.002f;
            
            ty1 = ((float)confontmap[c].y / height);
            ty2 = ty1 + (((float)confontmap[c].h / height));

            vtxstring[vi + 0].x     = vx1;
            vtxstring[vi + 0].y     = vy1;
            vtxstring[vi + 0].tu    = tx1;
            vtxstring[vi + 0].tv    = ty2;
            vtxstring[vi + 1].x     = vx2;
            vtxstring[vi + 1].y     = vy1;
            vtxstring[vi + 1].tu    = tx2;
            vtxstring[vi + 1].tv    = ty2;
            vtxstring[vi + 2].x     = vx2;
            vtxstring[vi + 2].y     = vy2;
            vtxstring[vi + 2].tu    = tx2;
            vtxstring[vi + 2].tv    = ty1;
            vtxstring[vi + 3].x     = vx1;
            vtxstring[vi + 3].y     = vy2;
            vtxstring[vi + 3].tu    = tx1;
            vtxstring[vi + 3].tv    = ty1;

            dglSetVertexColor(vtxstring + vi, color, 4);

            dglTriangle(vi + 0, vi + 1, vi + 2);
            dglTriangle(vi + 0, vi + 2, vi + 3);

            if(devparm) vertCount += 4;
            
            x += ((float)confontmap[c].w * scale);
        }
    }

    dglDrawGeometry(vi, vtxstring);
    
    R_GLDisable2D();
    R_GLToggleBlend(0);

    return x;
}

//
// CON_Draw
//

#define CONFONT_SCALE   ((float)(video_width * SCREENHEIGHT) / (float)(video_width * video_height))
#define CONFONT_YPAD    (16 * CONFONT_SCALE)

void CON_Draw(void)
{
    int     line;
    float   y = 0;
    float   x = 0;
    float   inputlen;
    
    if(!ConsoleLineBuffer)
        return;

    if(!ConsoleOn)
        return;
    
    R_GLEnable2D(1);
    R_GLToggleBlend(1);

    dglDisable(GL_TEXTURE_2D);
    dglColor4ub(0, 0, 0, 128);
    dglRectf(SCREENWIDTH, CONSOLE_Y + CONFONT_YPAD, 0, 0);

    R_GLToggleBlend(0);
    
    dglColor4f(0, 1, 0, 1);
    dglBegin(GL_LINES);
    dglVertex2f(0, CONSOLE_Y - 1);
    dglVertex2f(SCREENWIDTH, CONSOLE_Y - 1);
    dglVertex2f(0, CONSOLE_Y + CONFONT_YPAD);
    dglVertex2f(SCREENWIDTH, CONSOLE_Y + CONFONT_YPAD);
    dglEnd();
    dglEnable(GL_TEXTURE_2D);
    
    line = ConsoleHead;
    
    y = CONSOLE_Y - 2;
    while(ConsoleText[line] && y > 0)
    {
        CON_DrawConsoleText(0, y, ConsoleText[line]->color, CONFONT_SCALE, "%s", ConsoleText[line]->line);
        line = (line + 1) & CONSOLETEXT_MASK;
        y -= CONFONT_YPAD;
    }

    CON_DrawConsoleText(SCREENWIDTH - (64 * CONFONT_SCALE), CONSOLE_Y - 2,
        RED, CONFONT_SCALE, "rev. %s", PACKAGE_VERSION);
    
    y = CONSOLE_Y + CONFONT_YPAD;

    inputlen = CON_DrawConsoleText(x, y, WHITE, CONFONT_SCALE, "%s", ConsoleInputBuff);
    CON_DrawConsoleText(x + inputlen, y, WHITE, CONFONT_SCALE, "_");
}
