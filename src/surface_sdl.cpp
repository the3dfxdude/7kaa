/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2010 Jesse Allen
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

//Filename    : surface_sdl.cpp
//Description : SDL surface class

#include <surface_sdl.h>
#include <OVGA.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Surface);

//-------- Begin of function SurfaceSDL::SurfaceSDL ----------//

SurfaceSDL::SurfaceSDL(SDL_Surface *s)
{
	surface = s;
	save_buf = NULL;
}
//-------- End of function SurfaceSDL::SurfaceSDL ----------//


//-------- Begin of function SurfaceSDL::~SurfaceSDL ----------//

SurfaceSDL::~SurfaceSDL()
{
	SDL_FreeSurface(surface);
}
//-------- End of function SurfaceSDL::~SurfaceSDL ----------//


//------------- Begin of function SurfaceSDL::lock_buf --------------//
//
// Return true when surface is locked, false when there is an error.
//
int SurfaceSDL::lock_buf()
{
	return 1;
}
//------------- End of function SurfaceSDL::lock_buf --------------//


//------------- Begin of function SurfaceSDL::unlock_buf --------------//
//
// Return true when surface is unlocked, false when there is an error.
//
int SurfaceSDL::unlock_buf()
{
	return 1;
}
//--------------- End of function SurfaceSDL::unlock_buf --------------//


//-------- Begin of function SurfaceSDL::activate_pal ----------//
//
// Activate a palette to the current direct draw surface buffer.
//
int SurfaceSDL::activate_pal(SDL_Color *pal, int firstcolor, int ncolors)
{
	return SDL_SetPaletteColors(surface->format->palette,
				    pal,
				    firstcolor,
				    ncolors);
}
//--------- End of function SurfaceSDL::activate_pal ----------//


//-------- Begin of function SurfaceSDL::is_buf_lost ----------//
//
int SurfaceSDL::is_buf_lost()
{
	return 0;
}
//--------- End of function SurfaceSDL::is_buf_lost ----------//


//-------- Begin of function SurfaceSDL::restore_buf ----------//
//
// Restore buffers that have been lost.
//
int SurfaceSDL::restore_buf()
{
	return 1;
}
//--------- End of function SurfaceSDL::restore_buf ----------//


//-------- Begin of function SurfaceSDL::blt_virtual_buf --------//
//
// Blit entire source surface to local destination surface.
//
// Only called when sys.debug_session is used.
//
void SurfaceSDL::blt_virtual_buf( SurfaceSDL *source )
{
}
//--------- End of function SurfaceSDL::blt_virtual_buf ---------//


//------------ Begin of function SurfaceSDL::write_bmp_file --------------//
//
// Load a BMP file into the current VgaBuf DIB object.
//
// <char*> fileName - the name of the BMP file.
//
// return : <int> 1-succeeded, 0-failed.
//
int SurfaceSDL::write_bmp_file(char* fileName)
{
	return !SDL_SaveBMP(surface, fileName);
}
//------------ End of function SurfaceSDL::write_bmp_file --------------//


//------------ Begin of function SurfaceSDL::get_surface --------------//
SDL_Surface *SurfaceSDL::get_surface()
{
        return surface;
}
//------------ End of function SurfaceSDL::get_surface --------------//
