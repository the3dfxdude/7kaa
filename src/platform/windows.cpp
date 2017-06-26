/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2017 Jesse Allen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : windows.cpp
//Description : Support for windows

#ifdef USE_WINDOWS

#include <SDL2/SDL.h>
#include <windows.h>

typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

BOOL(WINAPI *SetProcessDPIAware)(void); // Vista and later
HRESULT(WINAPI *SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS dpiAwareness); // Windows 8.1 and later

// Based on the example provided by Eric Wasylishen
// https://discourse.libsdl.org/t/sdl-getdesktopdisplaymode-resolution-reported-in-windows-10-when-using-app-scaling/22389
void WIN_InitDPI()
{
	void* userDLL;
	void* shcoreDLL;

	shcoreDLL = SDL_LoadObject("SHCORE.DLL");
	if (shcoreDLL)
	{
		SetProcessDpiAwareness = (HRESULT(WINAPI *)(PROCESS_DPI_AWARENESS)) SDL_LoadFunction(shcoreDLL, "SetProcessDpiAwareness");
	}

	if (SetProcessDpiAwareness)
	{
		/* Try Windows 8.1+ version */
		HRESULT result = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		return;
	}

	userDLL = SDL_LoadObject("USER32.DLL");
	if (userDLL)
	{
		SetProcessDPIAware = (BOOL(WINAPI *)(void)) SDL_LoadFunction(userDLL, "SetProcessDPIAware");
	}

	if (SetProcessDPIAware)
	{
		/* Try Vista - Windows 8 version.
		This has a constant scale factor for all monitors.
		*/
		BOOL success = SetProcessDPIAware();
	}
}

#endif
