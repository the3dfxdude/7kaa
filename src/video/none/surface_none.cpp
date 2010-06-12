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

//Filename    : surface_none.cpp
//Description : Dummy surface class

#include <surface_none.h>
#include <OVGA.h>

//-------- Begin of function SurfaceNone::SurfaceNone ----------//

SurfaceNone::SurfaceNone()
{
}
//-------- End of function SurfaceNone::SurfaceNone ----------//


//-------- Begin of function SurfaceNone::~SurfaceNone ----------//

SurfaceNone::~SurfaceNone()
{
}
//-------- End of function SurfaceNone::~SurfaceNone ----------//


//------------- Begin of function SurfaceNone::lock_buf --------------//
//
// Return true when surface is locked, false when there is an error.
//
int SurfaceNone::lock_buf()
{
	return 0;
}
//------------- End of function SurfaceNone::lock_buf --------------//


//------------- Begin of function SurfaceNone::unlock_buf --------------//
//
// Return true when surface is unlocked, false when there is an error.
//
int SurfaceNone::unlock_buf()
{
	return 0;
}
//--------------- End of function SurfaceNone::unlock_buf --------------//


//-------- Begin of function SurfaceNone::activate_pal ----------//
//
// Activate a palette to the current direct draw surface buffer.
//
int SurfaceNone::activate_pal()
{
	return 1;
}
//--------- End of function SurfaceNone::activate_pal ----------//


//-------- Begin of function SurfaceNone::is_buf_lost ----------//
//
int SurfaceNone::is_buf_lost()
{
	return 0;
}
//--------- End of function SurfaceNone::is_buf_lost ----------//


//-------- Begin of function SurfaceNone::restore_buf ----------//
//
// Restore buffers that have been lost.
//
int SurfaceNone::restore_buf()
{
	return 1;
}
//--------- End of function SurfaceNone::restore_buf ----------//


//-------- Begin of function SurfaceNone::blt_virtual_buf --------//
//
// Blit entire source surface to local destination surface.
//
void SurfaceNone::blt_virtual_buf( SurfaceNone *source )
{
}
//--------- End of function SurfaceNone::blt_virtual_buf ---------//


//------------ Begin of function SurfaceNone::write_bmp_file --------------//
//
// Load a BMP file into the current VgaBuf DIB object.
//
// <char*> fileName - the name of the BMP file.
//
// return : <int> 1-succeeded, 0-failed.
//
int SurfaceNone::write_bmp_file(char* fileName)
{
	return 1;
}
//------------ End of function SurfaceNone::write_bmp_file --------------//
