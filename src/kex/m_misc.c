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
// $Author$
// $Revision$
// $Date$
//
//
// DESCRIPTION:
//      Main loop menu stuff.
//      Default Config File.
//		Executable arguments
//		BBox stuff
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char
rcsid[] = "$Id$";
#endif

#ifdef _WIN32
#include <io.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "doomdef.h"
#include "w_wad.h"
#include "i_system.h"
#include "v_sdl.h"
#include "am_map.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "r_local.h"
#include "m_misc.h"
#include "g_controls.h"
#include "g_actions.h"
#include "r_texture.h"
#include "z_zone.h"
#include "g_settings.h"
#include "g_local.h"
#include "st_stuff.h"
#include "i_png.h"

#include "Ext/ChocolateDoom/net_client.h"

int		myargc;
char**	myargv;


//
// M_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present

int M_CheckParm (char *check)
{
    int		i;
    
    for (i = 1;i<myargc;i++)
    {
        if(!dstricmp(check, myargv[i]) )//strcasecmp
            return i;
    }
    
    return 0;
}

//
// M_ClearBox
//

void M_ClearBox (fixed_t *box)
{
    box[BOXTOP] = box[BOXRIGHT] = MININT;
    box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}

//
// M_AddToBox
//

void M_AddToBox(fixed_t* box, fixed_t x, fixed_t y)
{
    if(x<box[BOXLEFT])
        box[BOXLEFT] = x;
    else if(x>box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if(y<box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    else if(y>box[BOXTOP])
        box[BOXTOP] = y;
}


//
// M_WriteFile
//

dboolean M_WriteFile(char const* name, void* source, int length)
{
    FILE *fp;
    dboolean result;
    
    errno = 0;
   
    if(!(fp = fopen(name, "wb")))
        return 0;
   
    I_BeginRead();
    result = (fwrite(source, 1, length, fp) == (dword)length);
    fclose(fp);
   
    if(!result)
        remove(name);
   
    return result;
}

//
// M_WriteTextFile
//

dboolean M_WriteTextFile(char const* name, char* source, int length)
{
    int handle;
    int count;
    
    handle = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    if(handle == -1)
        return false;
    
    count = write(handle, source, length);
    close(handle);
    
    if(count < length)
        return false;
    
    return true;
}


//
// M_ReadFile
//

int M_ReadFile(char const* name, byte** buffer)
{
    FILE *fp;

    errno = 0;
    
    if((fp = fopen(name, "rb")))
    {
        size_t length;
        
        I_BeginRead();

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = Z_Malloc(length, PU_STATIC, 0);
      
        if(fread(*buffer, 1, length, fp) == length)
        {
            fclose(fp);
            return length;
        }
        
        fclose(fp);
   }

    //I_Error("M_ReadFile: Couldn't read file %s: %s", name,
        //errno ? strerror(errno) : "(Unknown Error)");
   
   return -1;
}

//
// M_FileLength
//

long M_FileLength(FILE *handle)
{ 
    long savedpos;
    long length;

    // save the current position in the file
    savedpos = ftell(handle);
    
    // jump to the end and find the length
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);

    // go back to the old location
    fseek(handle, savedpos, SEEK_SET);

    return length;
}

//
// M_NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// killough 11/98: rewritten
//

void M_NormalizeSlashes(char *str)
{
    char *p;
   
    // Convert all slashes/backslashes to DIR_SEPARATOR
    for(p = str; *p; p++)
    {
        if((*p == '/' || *p == '\\') && *p != DIR_SEPARATOR)
            *p = DIR_SEPARATOR;
    }

    // Collapse multiple slashes
    for(p = str; (*str++ = *p); )
        if(*p++ == DIR_SEPARATOR)
            while(*p == DIR_SEPARATOR)
                p++;
}

//
// W_FileExists
// Check if a wad file exists
//

int M_FileExists(char *filename)
{
    FILE *fstream;

    fstream = fopen(filename, "r");

    if (fstream != NULL)
    {
        fclose(fstream);
        return 1;
    }
    else
    {
        // If we can't open because the file is a directory, the 
        // "file" exists at least!

        if(errno == 21)
            return 2;
    }

    return 0;
}

//
// M_SaveDefaults
//

void M_SaveDefaults(void)
{
    FILE		*fh;
    
    fh=fopen(ConfigFileName, "wt");
    if (fh)
    {
        G_OutputBindings(fh);
        fclose(fh);
    }
}

//
// M_LoadDefaults
//

void M_LoadDefaults(void)
{
    G_InitActions();
    G_InitCmds();
    G_LoadSettings();
}

//
// M_ScreenShot
//

void M_ScreenShot(void)
{
    char	name[13];
    int		shotnum=0;
    FILE	*fh;
    byte	*buff;
    byte	*png;
    int		size;
    
    while(shotnum < 1000)
    {
        sprintf(name, "sshot%03d.png", shotnum);
        if(access(name, 0) != 0)
            break;
        shotnum++;
    }
    
    if(shotnum >= 1000)
        return;
    
    fh = fopen(name, "wb");
    if(!fh)
        return;
    
    if((video_height % 2))	// height must be power of 2
        return;
    
    buff = R_GLGetScreen(video_width, video_height);
    size = 0;
    
    // Get PNG image
    
    png = I_PNGCreate(video_width, video_height, buff, &size);
    fwrite(png, size, 1, fh);
    
    Z_Free(png);
    fclose(fh);

    I_Printf("Saved Screenshot %s\n", name);
}

//
// M_CacheThumbNail
// Thumbnails are assumed they are
// uncompressed 128x128 RGB textures
//

int M_CacheThumbNail(byte** data)
{
    byte* buff;
    byte* tbn;

    buff = R_GLGetScreen(video_width, video_height);
    tbn = Z_Calloc((128 * 128) * 3, PU_STATIC, 0);

    gluScaleImage(GL_RGB, video_width, video_height,
        GL_UNSIGNED_BYTE, buff, 128, 128, GL_UNSIGNED_BYTE, tbn);
    
    Z_Free(buff);

    *data = tbn;
    return (128 * 128) * 3;
}

//
//
// STRING DRAWING ROUTINES
//
//

static vtx_t vtxstring[MAX_MESSAGE_SIZE];

//
// M_DrawText
//

int M_DrawText(int x, int y, rcolor color, float scale, dboolean wrap, const char* string, ...)
{
    int c;
    int i;
    int vi = 0;
    int	col;
    const float size = 0.03125f;
    float fcol, frow;
    int start = 0;
    dboolean fill = false;
    char msg[MAX_MESSAGE_SIZE];
    va_list	va;
    const int ix = x;
    
    va_start(va, string);
    vsprintf(msg, string, va);
    va_end(va);
    
    R_GLToggleBlend(1);
    
    if(!r_fillmode.value)
    {
        dglEnable(GL_TEXTURE_2D);
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        r_fillmode.value = 1.0f;
        fill = true;
    }
    
    R_BindGfxTexture("SFONT", true);

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    
    R_GLSetOrthoScale(scale);
    R_GLEnable2D(0);

    dglSetVertex(vtxstring);

    for(i = 0, vi = 0; i < dstrlen(msg); i++, vi += 4)
    {
        c = toupper(msg[i]);
        if(c == '\t')
        {
            while(x % 64) x++;
            continue;
        }
        if(c == '\n')
        {
            y += ST_FONTWHSIZE;
            x = ix;
            continue;
        }
        if(c == 0x20)
        {
            if(wrap)
            {
                if(x > 192)
                {
                    y += ST_FONTWHSIZE;
                    x = ix;
                    continue;
                }
            }
        }
        else
        {
            start = (c - ST_FONTSTART);
            col = start & (ST_FONTNUMSET - 1);
            
            fcol = (col * size);
            frow = (start >= ST_FONTNUMSET) ? 0.5f : 0.0f;

            vtxstring[vi + 0].x     = (float)x;
            vtxstring[vi + 0].y     = (float)y;
            vtxstring[vi + 0].tu    = fcol + 0.0015f;
            vtxstring[vi + 0].tv    = frow + size;
            vtxstring[vi + 1].x     = (float)x + ST_FONTWHSIZE;
            vtxstring[vi + 1].y     = (float)y;
            vtxstring[vi + 1].tu    = (fcol + size) - 0.0015f;
            vtxstring[vi + 1].tv    = frow + size;
            vtxstring[vi + 2].x     = (float)x + ST_FONTWHSIZE;
            vtxstring[vi + 2].y     = (float)y + ST_FONTWHSIZE;
            vtxstring[vi + 2].tu    = (fcol + size) - 0.0015f;
            vtxstring[vi + 2].tv    = frow + 0.5f;
            vtxstring[vi + 3].x     = (float)x;
            vtxstring[vi + 3].y     = (float)y + ST_FONTWHSIZE;
            vtxstring[vi + 3].tu    = fcol + 0.0015f;
            vtxstring[vi + 3].tv    = frow + 0.5f;

            dglSetVertexColor(vtxstring + vi, color, 4);

            dglTriangle(vi + 0, vi + 1, vi + 2);
            dglTriangle(vi + 0, vi + 2, vi + 3);

            if(devparm) vertCount += 4;
            
            
        }
        x += ST_FONTWHSIZE;
    }
    
    dglDrawGeometry(vi, vtxstring);

    R_GLDisable2D();
    
    if(fill)
    {
        dglDisable(GL_TEXTURE_2D);
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        r_fillmode.value = 0.0f;
    }
    
    R_GLToggleBlend(0);
    R_GLSetOrthoScale(1.0f);
    
    return x;
}

const symboldata_t symboldata[] =	//0x5B9BC
{
    { 120, 14, 13, 13 },
    { 134, 14, 9, 13 },
    { 144, 14, 14, 13 },
    { 159, 14, 14, 13 },
    { 174, 14, 16, 13 },
    { 191, 14, 13, 13 },
    { 205, 14, 13, 13 },
    { 219, 14, 14, 13 },
    { 234, 14, 14, 13 },
    { 0, 29, 13, 13 },
    { 67, 28, 14, 13 },	// -
    { 36, 28, 15, 14 },	// %
    { 28, 28, 7, 14 },	// !
    { 14, 29, 6, 13 },	// .
    { 52, 28, 13, 13 },	// ?
    { 21, 29, 6, 13 },	// :
    { 0, 0, 13, 13 },
    { 14, 0, 13, 13 },
    { 28, 0, 13, 13 },
    { 42, 0, 14, 13 },
    { 57, 0, 14, 13 },
    { 72, 0, 10, 13 },
    { 87, 0, 15, 13 },
    { 103, 0, 15, 13 },
    { 119, 0, 6, 13 },
    { 126, 0, 13, 13 },
    { 140, 0, 14, 13 },
    { 155, 0, 11, 13 },
    { 167, 0, 15, 13 },
    { 183, 0, 16, 13 },
    { 200, 0, 15, 13 },
    { 216, 0, 13, 13 },
    { 230, 0, 15, 13 },
    { 246, 0, 13, 13 },
    { 0, 14, 14, 13 },
    { 15, 14, 14, 13 },
    { 30, 14, 13, 13 },
    { 44, 14, 15, 13 },
    { 60, 14, 15, 13 },
    { 76, 14, 15, 13 },
    { 92, 14, 13, 13 },
    { 106, 14, 13, 13 },
    { 83, 31, 10, 11 },
    { 93, 31, 10, 11 },
    { 103, 31, 11, 11 },
    { 114, 31, 11, 11 },
    { 125, 31, 11, 11 },
    { 136, 31, 11, 11 },
    { 147, 31, 12, 11 },
    { 159, 31, 12, 11 },
    { 171, 31, 4, 11 },
    { 175, 31, 10, 11 },
    { 185, 31, 11, 11 },
    { 196, 31, 9, 11 },
    { 205, 31, 12, 11 },
    { 217, 31, 13, 11 },
    { 230, 31, 12, 11 },
    { 242, 31, 11, 11 },
    { 0, 43, 12, 11 },
    { 12, 43, 11, 11 },
    { 23, 43, 11, 11 },
    { 34, 43, 10, 11 },
    { 44, 43, 11, 11 },
    { 55, 43, 12, 11 },
    { 67, 43, 13, 11 },
    { 80, 43, 13, 11 },
    { 93, 43, 10, 11 },
    { 103, 43, 11, 11 },
    { 0, 95, 108, 11 },
    { 108, 95, 6, 11 },
    { 0, 54, 32, 26 },
    { 32, 54, 32, 26 },
    { 64, 54, 32, 26 },
    { 96, 54, 32, 26 },
    { 128, 54, 32, 26 },
    { 160, 54, 32, 26 },
    { 192, 54, 32, 26 },
    { 224, 54, 32, 26 },
    { 134, 97, 7, 11 },
    { 114, 95, 20, 18 },
    { 105, 80, 15, 15 },
    { 120, 80, 15, 15 },
    { 135, 80, 15, 15 },
    { 150, 80, 15, 15 },
    { 45, 80, 15, 15 },
    { 60, 80, 15, 15 },
    { 75, 80, 15, 15 },
    { 90, 80, 15, 15 },
    { 165, 80, 15, 15 },
    { 180, 80, 15, 15 },
    { 0, 80, 15, 15 },
    { 15, 80, 15, 15 },
    { 195, 80, 15, 15 },
    { 30, 80, 15, 15 },
    { 156, 96, 13, 13 },
    { 143, 96, 13, 13 },
    { 169, 96, 7, 13 },
    { -1, -1, -1, -1 }
};

//
// M_CenterSmbText
//

int M_CenterSmbText(const char* string)
{
    int width = 0;
    char t = 0;
    int id = 0;
    int len = 0;
    int i = 0;
    float scale;
    
    len = dstrlen(string);
    
    for(i = 0; i < len; i++)
    {
        t = string[i];
        
        switch(t)
        {
        case 0x20: width += 6;
            break;
        case '-': width += symboldata[SM_MISCFONT].w;
            break;
        case '%': width += symboldata[SM_MISCFONT + 1].w;
            break;
        case '!': width += symboldata[SM_MISCFONT + 2].w;
            break;
        case '.': width += symboldata[SM_MISCFONT + 3].w;
            break;
        case '?': width += symboldata[SM_MISCFONT + 4].w;
            break;
        case ':': width += symboldata[SM_MISCFONT + 5].w;
            break;
        default:
            if(t >= 'A' && t <= 'Z')
            {
                id = t - 'A';
                width += symboldata[SM_FONT1 + id].w;
            }
            if(t >= 'a' && t <= 'z')
            {
                id = t - 'a';
                width += symboldata[SM_FONT2 + id].w;
            }
            if(t >= '0' && t <= '9')
            {
                id = t - '0';
                width += symboldata[SM_NUMBERS + id].w;
            }
            break;
        }
    }

    scale = R_GLGetOrthoScale();

    if(scale != 1.0f)
        return ((int)(160.0f / scale) - (width / 2));
    
    return (160 - (width / 2));
}

//
// M_DrawSmbText
//

int M_DrawSmbText(int x, int y, rcolor color, const char* string)
{
    int c = 0;
    int i = 0;
    int vi = 0;
    int index = 0;
    float vx1 = 0.0f;
    float vy1 = 0.0f;
    float vx2 = 0.0f;
    float vy2 = 0.0f;
    float tx1 = 0.0f;
    float tx2 = 0.0f;
    float ty1 = 0.0f;
    float ty2 = 0.0f;
    float smbwidth;
    float smbheight;
    int pic;
    
    if(x <= -1)
        x = M_CenterSmbText(string);
    
    y += 14;
    
    pic = R_BindGfxTexture("SYMBOLS", true);

    smbwidth = (float)gfxwidth[pic];
    smbheight = (float)gfxheight[pic];

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);

    dglSetVertex(vtxstring);
    
    R_GLToggleBlend(1);
    R_GLEnable2D(0);

    for(i = 0, vi = 0; i < dstrlen(string); i++, vi += 4)
    {
        vx1 = (float)x;
        vy1 = (float)y;
        
        c = string[i];
        if(c == '\n' || c == '\t')
            continue;   // villsa: safety check
        else if(c == 0x20)
        {
            x += 6;
            continue;
        }
        else
        {
            if(c >= '0' && c <= '9')    index = (c - '0') + SM_NUMBERS;
            if(c >= 'A' && c <= 'Z')    index = (c - 'A') + SM_FONT1;
            if(c >= 'a' && c <= 'z')    index = (c - 'a') + SM_FONT2;
            if(c == '-')                index = SM_MISCFONT;
            if(c == '%')                index = SM_MISCFONT + 1;
            if(c == '!')                index = SM_MISCFONT + 2;
            if(c == '.')                index = SM_MISCFONT + 3;
            if(c == '?')                index = SM_MISCFONT + 4;
            if(c == ':')                index = SM_MISCFONT + 5;

            // [kex] use 'printf' style formating for special symbols
            if(c == '/')
            {
                c = string[++i];

                switch(c)
                {
                    // up arrow
                case 'u':
                    index = SM_MICONS + 17;
                    break;
                    // down arrow
                case 'd':
                    index = SM_MICONS + 16;
                    break;
                    // right arrow
                case 'r':
                    index = SM_MICONS + 18;
                    break;
                    // left arrow
                case 'l':
                    index = SM_MICONS;
                    break;
                    // cursor box
                case 'b':
                    index = SM_MICONS + 1;
                    break;
                    // thermbar
                case 't':
                    index = SM_THERMO;
                    break;
                    // thermcursor
                case 's':
                    index = SM_THERMO + 1;
                    break;
                default:
                    return 0;
                }
            }
            
            vx2 = vx1 + symboldata[index].w;
            vy2 = vy1 - symboldata[index].h;
            
            tx1 = ((float)symboldata[index].x / smbwidth) + 0.001f;
            tx2 = (tx1 + (float)symboldata[index].w / smbwidth) - 0.002f;
            
            ty1 = ((float)symboldata[index].y / smbheight);
            ty2 = ty1 + (((float)symboldata[index].h / smbheight));

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
            
            x += symboldata[index].w;
        }
    }

    dglDrawGeometry(vi, vtxstring);
    
    R_GLDisable2D();
    R_GLToggleBlend(0);
    
    return x;
}

//
// M_DrawNumber
//
//

void M_DrawNumber(int x, int y, int num, int type, rcolor c)
{
    int digits[16];
    int nx = 0;
    int count;
    int j;
    char str[2];

    for(count = 0, j = 0; count < 16; count++, j++)
    {
        digits[j] = num % 10;
        nx += symboldata[SM_NUMBERS + digits[j]].w;

        num /= 10;

        if(!num)
            break;
    }

    if(type == 0)
        x -= (nx >> 1);

    if(type == 0 || type == 1)
    {
        if(count < 0)
            return;

        while(count >= 0)
        {
            sprintf(str, "%i", digits[j]);
            M_DrawSmbText(x, y, c, str);

            x += symboldata[SM_NUMBERS + digits[j]].w;

            count--;
            j--;
        }
    }
    else
    {
        if(count < 0)
            return;

        j = 0;

        while(count >= 0)
        {
            x -= symboldata[SM_NUMBERS + digits[j]].w;

            sprintf(str, "%i", digits[j]);
            M_DrawSmbText(x, y, c, str);

            count--;
            j++;
        }
    }
}


