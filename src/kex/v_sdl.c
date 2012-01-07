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
// $Author$
// $Revision$
// $Date$
//
//
// DESCRIPTION:
//	SDL Stuff
//
//-----------------------------------------------------------------------------
#ifdef RCSID
static const char rcsid[] = "$Id$";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "m_misc.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_sdl.h"
#include "d_main.h"
#include "r_gl.h"

#ifdef _WIN32
#include "i_xinput.h"
#endif

static void V_GetEvent(SDL_Event *Event);
static void V_ReadMouse(void);
void V_UpdateGrab(void);

//================================================================================
// Video
//================================================================================

SDL_Surface *screen;
int	video_width;
int	video_height;
dboolean window_focused;

//
// V_InitScreen
//

void V_InitScreen(void)
{
	int		newwidth;
    int		newheight;
	int		p;

	InWindow = (int)v_windowed.value;
	video_width = (int)v_width.value;
	video_height = (int)v_height.value;

	if(M_CheckParm("-window"))		InWindow=true;
    if(M_CheckParm("-fullscreen"))	InWindow=false;

	newwidth = newheight = 0;

	 p = M_CheckParm("-width");
    if(p && p < myargc - 1)
		newwidth = datoi(myargv[p+1]);

    p = M_CheckParm("-height");
    if(p && p < myargc - 1)
		newheight = datoi(myargv[p+1]);

    if(newwidth && newheight)
    {
		video_width = newwidth;
		video_height = newheight;
        CON_CvarSetValue(v_width.name, (float)video_width);
        CON_CvarSetValue(v_height.name, (float)video_height);
    }

	usingGL = false;
}

//
// V_ShutdownWait
//

void V_ShutdownWait(void)
{
	while(1)
	{
		static SDL_Event event;

		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT || 
				(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
			{
				V_Shutdown();
#ifndef USESYSCONSOLE
				exit(0);
#else
                return;
#endif
			}
		}

        I_Sleep(100);
        R_GLFinish();
	}
}

//
// V_Shutdown
//

void V_Shutdown(void)
{
	SDL_Quit();
}

//
// V_NetWaitScreen
// Blank screen display while waiting for players to join
//

void V_NetWaitScreen(void)
{
    uint32	flags = 0;

    V_InitScreen();
    flags |= SDL_SWSURFACE;

	if (!(screen = SDL_SetVideoMode(320, 240, 0, flags)))
	{
		V_Shutdown();
		exit(1);
	}

	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
}

//
// V_InitGL
//

void V_InitGL(void)
{
	uint32	flags = 0;

    V_InitScreen();

    if(v_depthsize.value != 8 &&
        v_depthsize.value != 16 &&
        v_depthsize.value != 24)
    {
        CON_CvarSetValue(v_depthsize.name, 24);
    }

    if(v_buffersize.value != 8 &&
        v_buffersize.value != 16 &&
        v_buffersize.value != 24
        && v_buffersize.value != 32)
    {
        CON_CvarSetValue(v_buffersize.name, 32);
    }

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, (int)v_buffersize.value);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (int)v_depthsize.value);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (int)v_vsync.value);

	flags |= SDL_OPENGL;

	if(!InWindow)
		flags |= SDL_FULLSCREEN;

	if (SDL_SetVideoMode(video_width, video_height, SDL_BPP, flags) == NULL)
		I_Error("V_Init: Failed to set opengl");

	R_GLInitialize();

	usingGL = true;

#ifdef USESYSCONSOLE
    I_ShowSysConsole(false);
#endif
}

//
// V_Init
//

void V_Init(void)
{
    char title[256];

	uint32 f = SDL_INIT_VIDEO;

#ifdef _DEBUG
	f |= SDL_INIT_NOPARACHUTE;
#endif

	putenv("SDL_VIDEO_CENTERED=1");

	if(SDL_Init(f) < 0)
	{
		printf("ERROR - Failed to initialize SDL");
		exit(1);
	}

    sprintf(title, "Doom64 - Version Date: %s", version_date);
	SDL_WM_SetCaption(title, "Doom64");

	V_InitInputs();
}

//
// V_StartTic
//

void V_StartTic (void)
{
	SDL_Event Event;

	while(SDL_PollEvent(&Event))
		V_GetEvent(&Event);

#ifdef _USE_XINPUT
    I_XInputPollEvent();
#endif

	V_ReadMouse();
}

//
// V_FinishUpdate
//

void V_FinishUpdate(void)
{
	V_UpdateGrab();
	R_GLFinish();
}

//================================================================================
// Input
//================================================================================

static SDL_Cursor* cursors[2] = { NULL, NULL };
float mouse_accelfactor;

int			UseJoystick;
int			UseMouse[2];
dboolean	DigiJoy;
int			DualMouse;

dboolean	MouseMode;//false=microsoft, true=mouse systems

//
// V_TranslateKey
//

static int V_TranslateKey(SDL_keysym* key)
{
	int rc = 0;
	
	switch (key->sym)
	{
	case SDLK_LEFT:			rc = KEY_LEFTARROW;			break;
	case SDLK_RIGHT:		rc = KEY_RIGHTARROW;		break;
	case SDLK_DOWN:			rc = KEY_DOWNARROW;			break;
	case SDLK_UP:			rc = KEY_UPARROW;			break;
	case SDLK_ESCAPE:		rc = KEY_ESCAPE;			break;
	case SDLK_RETURN:		rc = KEY_ENTER;				break;
	case SDLK_TAB:			rc = KEY_TAB;				break;
	case SDLK_F1:			rc = KEY_F1;				break;
	case SDLK_F2:			rc = KEY_F2;				break;
	case SDLK_F3:			rc = KEY_F3;				break;
	case SDLK_F4:			rc = KEY_F4;				break;
	case SDLK_F5:			rc = KEY_F5;				break;
	case SDLK_F6:			rc = KEY_F6;				break;
	case SDLK_F7:			rc = KEY_F7;				break;
	case SDLK_F8:			rc = KEY_F8;				break;
	case SDLK_F9:			rc = KEY_F9;				break;
	case SDLK_F10:			rc = KEY_F10;				break;
	case SDLK_F11:			rc = KEY_F11;				break;
	case SDLK_F12:			rc = KEY_F12;				break;
	case SDLK_BACKSPACE:	rc = KEY_BACKSPACE;			break;
	case SDLK_DELETE:		rc = KEY_DEL;				break;
	case SDLK_INSERT:		rc = KEY_INSERT;			break;
	case SDLK_PAGEUP:		rc = KEY_PAGEUP;			break;
	case SDLK_PAGEDOWN:		rc = KEY_PAGEDOWN;			break;
	case SDLK_HOME:			rc = KEY_HOME;				break;
	case SDLK_END:			rc = KEY_END;				break;
	case SDLK_PAUSE:		rc = KEY_PAUSE;				break;
	case SDLK_EQUALS:		rc = KEY_EQUALS;			break;
	case SDLK_MINUS:		rc = KEY_MINUS;				break;
	case SDLK_KP0:			rc = KEY_KEYPAD0;			break;
	case SDLK_KP1:			rc = KEY_KEYPAD1;			break;
	case SDLK_KP2:			rc = KEY_KEYPAD2;			break;
	case SDLK_KP3:			rc = KEY_KEYPAD3;			break;
	case SDLK_KP4:			rc = KEY_KEYPAD4;			break;
	case SDLK_KP5:			rc = KEY_KEYPAD5;			break;
	case SDLK_KP6:			rc = KEY_KEYPAD6;			break;
	case SDLK_KP7:			rc = KEY_KEYPAD7;			break;
	case SDLK_KP8:			rc = KEY_KEYPAD8;			break;
	case SDLK_KP9:			rc = KEY_KEYPAD9;			break;
	case SDLK_KP_PLUS:		rc = KEY_KEYPADPLUS;		break;
	case SDLK_KP_MINUS:		rc = KEY_KEYPADMINUS;		break;
	case SDLK_KP_DIVIDE:	rc = KEY_KEYPADDIVIDE;		break;
	case SDLK_KP_MULTIPLY:	rc = KEY_KEYPADMULTIPLY;	break;
	case SDLK_KP_ENTER:		rc = KEY_KEYPADENTER;		break;
	case SDLK_KP_PERIOD:	rc = KEY_KEYPADPERIOD;		break;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:		rc = KEY_RSHIFT;			break;
	case SDLK_LCTRL:
	case SDLK_RCTRL:		rc = KEY_RCTRL;				break;
	case SDLK_LALT:
	case SDLK_LMETA:
	case SDLK_RALT:
	case SDLK_RMETA:		rc = KEY_RALT;				break;
	case SDLK_CAPSLOCK:		rc = KEY_CAPS;				break;
	default:				rc = key->sym;				break;
	}

  return rc;

}

//
// V_SDLtoDoomMouseState
//

static int V_SDLtoDoomMouseState(Uint8 buttonstate)
{
	return 0
      | (buttonstate & SDL_BUTTON(SDL_BUTTON_LEFT)      ? 1 : 0)
      | (buttonstate & SDL_BUTTON(SDL_BUTTON_MIDDLE)    ? 2 : 0)
      | (buttonstate & SDL_BUTTON(SDL_BUTTON_RIGHT)     ? 4 : 0);
}

//
// V_UpdateFocus
//

static void V_UpdateFocus(void)
{
	Uint8 state;
	state = SDL_GetAppState();

	// We should have input (keyboard) focus and be visible 
	// (not minimised)
	window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);
}

// V_CenterMouse
// Warp the mouse back to the middle of the screen
//

static void V_CenterMouse(void)
{
	// Warp the the screen center
	SDL_WarpMouse((unsigned short)(video_width/2), (unsigned short)(video_height/2));

	// Clear any relative movement caused by warping
	SDL_PumpEvents();
	SDL_GetRelativeMouseState(NULL, NULL);
}

//
// V_MouseShouldBeGrabbed
//

static dboolean V_MouseShouldBeGrabbed()
{
	// if the window doesnt have focus, never grab it
	if(!window_focused)
		return false;

	if(!InWindow)
		return true;

	// when menu is active or game is paused, release the mouse 
	if(menuactive || paused)
		return false;

	// only grab mouse when playing levels (but not demos)
	return (gamestate == GS_LEVEL) && !demoplayback;
}

//
// V_ReadMouse
//

static void V_ReadMouse(void)
{
	int x, y;
	event_t ev;

	SDL_GetRelativeMouseState(&x, &y);

	if(x != 0 || y != 0) 
	{
		ev.type = ev_mouse;
		ev.data1 = V_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
		ev.data2 = x << 5;
		ev.data3 = (-y) << 5;
		D_PostEvent(&ev);
	}

	if(V_MouseShouldBeGrabbed())
		V_CenterMouse();
}

//
// V_MouseAccelChange
//

void V_MouseAccelChange(void)
{
	mouse_accelfactor = v_macceleration.value / 200.0f + 1.0f;
}

//
// V_MouseAccel
//

int V_MouseAccel(int val)
{
	if(!v_macceleration.value)
		return val;
	
	if(val < 0)
		return -V_MouseAccel(-val);
	
	return (int)(pow((double)val, (double)mouse_accelfactor));
}

//
// V_ActivateMouse
//

static void V_ActivateMouse(void)
{
	SDL_SetCursor(cursors[1]);
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_ShowCursor(1);
}

//
// V_DeactivateMouse
//

static void V_DeactivateMouse(void)
{
	SDL_SetCursor(cursors[0]);
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(1);
}

//
// V_UpdateGrab
//

void V_UpdateGrab(void)
{
	static dboolean currently_grabbed = false;
	dboolean grab;
	
	grab = V_MouseShouldBeGrabbed();
	if (grab && !currently_grabbed)
	{
		V_ActivateMouse();
	}

	if (!grab && currently_grabbed)
	{
		V_DeactivateMouse();
	}
	
	currently_grabbed = grab;
}

//
// V_GetEvent
//

static void V_GetEvent(SDL_Event *Event)
{
	event_t event;
    uint32 mwheeluptic = 0, mwheeldowntic = 0;
    uint32 tic = gametic;

	switch(Event->type)
	{
	case SDL_KEYDOWN:
		event.type = ev_keydown;
		event.data1 = V_TranslateKey(&Event->key.keysym);
		D_PostEvent(&event);
		break;
	
	case SDL_KEYUP:
		event.type = ev_keyup;
		event.data1 = V_TranslateKey(&Event->key.keysym);
		D_PostEvent(&event);
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if(!window_focused)
			break;

        if(Event->button.button == SDL_BUTTON_WHEELUP)
        {
            event.type = ev_keydown;
            event.data1 = KEY_MWHEELUP;
            mwheeluptic = tic;
        }
        else if(Event->button.button == SDL_BUTTON_WHEELDOWN)
        {
            event.type = ev_keydown;
            event.data1 = KEY_MWHEELDOWN;
            mwheeldowntic = tic;
        }
        else
        {
            event.type = ev_mouse;
		    event.data1 = V_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
        }

        event.data2 = event.data3 = 0;
        D_PostEvent(&event);
		break;

	case SDL_ACTIVEEVENT:
	case SDL_VIDEOEXPOSE:
		V_UpdateFocus();
		break;
	
	case SDL_QUIT:
		I_Quit();
		break;

	default:
		break;
	}

    if(mwheeluptic && mwheeluptic + 1 < tic)
    {
        event.type = ev_keyup;
        event.data1 = KEY_MWHEELUP;
        D_PostEvent(&event);
        mwheeluptic = 0;
    }

    if(mwheeldowntic && mwheeldowntic + 1 < tic)
    {
        event.type = ev_keyup;
        event.data1 = KEY_MWHEELDOWN;
        D_PostEvent(&event);
        mwheeldowntic = 0;
    }
}

//
// V_InitInputs
//

void V_InitInputs(void)
{
	Uint8 data[1] = { 0x00 };

	SDL_PumpEvents();
	cursors[0] = SDL_GetCursor();
	cursors[1] = SDL_CreateCursor(data, data, 8, 1, 0, 0);

	UseMouse[0] = 1;
	UseMouse[1] = 2;

	V_CenterMouse();
	V_MouseAccelChange();
}

