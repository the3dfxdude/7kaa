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

//Filename    : vga_sdl.cpp
//Description : VGA manipulation functions (No output)

#include <OVGA.h>
#include <surface.h>

//------ Define static class member vars ---------//

char    VgaBase::use_back_buf = 0;
char    VgaBase::opaque_flag  = 0;
VgaBuf* VgaBase::active_buf   = &vga_front;      // default: front buffer

//-------- Begin of function VgaNone::VgaNone ----------//

VgaNone::VgaNone()
{
}
//-------- End of function VgaNone::VgaNone ----------//


//-------- Begin of function VgaNone::~VgaNone ----------//

VgaNone::~VgaNone()
{
   deinit();
}
//-------- End of function VgaNone::~VgaNone ----------//


//-------- Begin of function VgaNone::init ----------//

int VgaNone::init()
{
   return 1;
}
//-------- End of function VgaNone::init ----------//


//-------- Begin of function VgaBuf::init_front ----------//
//
// Create a direct draw front buffer.
//
int VgaNone::init_front(VgaBuf *b)
{
   return 1;
}
//-------- End of function VgaBuf::init_front ----------//


//-------- Begin of function VgaBuf::init_back ----------//
//
// Create a direct draw back buffer.
//
// [DWORD] w      : width of the surface [default 0 : VGA_WIDTH]
// [DWORD] h      : height of the surface [default 0 : VGA_HEIGHT]
//
int VgaNone::init_back( VgaBuf *b, unsigned long w, unsigned long h )
{
   return 1;
}
//-------- End of function VgaBuf::init_back ----------//


//-------- Begin of function VgaNone::deinit ----------//

void VgaNone::deinit()
{
}
//-------- End of function VgaNone::deinit ----------//


//--------- Start of function VgaNone::init_pal ----------//
//
// Loads the default game palette specified by fileName. Creates the ddraw
// palette.
//
int VgaNone::init_pal(const char* fileName)
{
   return 1;
}
//----------- End of function VgaNone::init_pal ----------//

//--------- Start of function VgaNone::refresh_palette ----------//
//
// When the system changes the palette, this function will set
// the palette back to the correct entries.
//
void VgaNone::refresh_palette()
{
}
//----------- End of function VgaNone::refresh_palette ----------//


//-------- Begin of function VgaNone::activate_pal ----------//
//
// we are getting the palette focus, select our palette
//
void VgaNone::activate_pal(VgaBuf* vgaBufPtr)
{
}
//--------- End of function VgaNone::activate_pal ----------//


//-------- Begin of function VgaNone::set_custom_palette ----------//
//
// Read the custom palette specified by fileName and set to display.
//
int VgaNone::set_custom_palette(char *fileName)
{
   return 1;
}
//--------- End of function VgaNone::set_custom_palette ----------//


//--------- Begin of function VgaNone::free_custom_palette ----------//
//
// Frees the custom palette and restores the game palette.
//
void VgaNone::free_custom_palette()
{
}
//--------- End of function VgaNone::free_custom_palette ----------//


//-------- Begin of function VgaNone::adjust_brightness ----------//
//
// <int> changeValue - the value to add to the RGB values of
//                     all the colors in the palette.
//                     the value can be from -255 to 255.
//
// <int> preserveContrast - whether preserve the constrast or not
//
void VgaNone::adjust_brightness(int changeValue)
{
}
//--------- End of function VgaNone::adjust_brightness ----------//


//-------- Begin of function VgaNone::handle_messages --------//
void VgaNone::handle_messages()
{
}
//-------- End of function VgaNone::handle_messages --------//

//-------- Begin of function VgaNone::flag_redraw --------//
void VgaNone::flag_redraw()
{
}
//-------- End of function VgaNone::flag_redraw ----------//

//-------- Begin of function VgaNone::toggle_full_screen --------//
void VgaNone::toggle_full_screen()
{
}
//-------- End of function VgaNone::toggle_full_screen ----------//
