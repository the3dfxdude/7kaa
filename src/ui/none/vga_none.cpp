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

//Filename    : vga_none.cpp
//Description : VGA manipulation functions (No output)

#include <OVGA.h>
#include <surface.h>

//------ Define static class member vars ---------//

char    VgaNone::use_back_buf = 0;
char    VgaNone::opaque_flag  = 0;
VgaBuf* VgaNone::active_buf   = &vga_front;      // default: front buffer

//-------- Begin of function VgaNone::VgaNone ----------//

VgaNone::VgaNone()
   : game_pal{},
     vga_color_table(nullptr)
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
   PalDesc palDesc( (unsigned char*) game_pal, 3, 256, 8);
   vga_color_table = new ColorTable;
   vga_color_table->generate_table( MAX_BRIGHTNESS_ADJUST_DEGREE, palDesc, ColorTable::bright_func );

   vga_front.init(new Surface(VGA_WIDTH, VGA_HEIGHT), 0);
   vga_back.init(new Surface(VGA_WIDTH, VGA_HEIGHT), 0);
   return 1;
}
//-------- End of function VgaNone::init ----------//


//-------- Begin of function VgaNone::deinit ----------//

void VgaNone::deinit()
{
   vga_front.deinit();
   vga_back.deinit();
   delete vga_color_table;
   vga_color_table = nullptr;
}
//-------- End of function VgaNone::deinit ----------//


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

//-------- Begin of function VgaNone::is_full_screen --------//
//
int VgaNone::is_full_screen()
{
   return 0;
}
//-------- End of function VgaNone::is_full_screen ----------/


//-------- Begin of function VgaNone::is_input_grabbed --------//
//
int VgaNone::is_input_grabbed()
{
   return 0;
}
//-------- End of function VgaNone::is_input_grabbed ----------/


//-------- Begin of function VgaNone::set_full_screen_mode --------//
void VgaNone::set_full_screen_mode(int mode)
{
}
//-------- End of function VgaNone::set_full_screen_mode ----------//


//-------- Begin of function VgaNone::set_window_grab --------//
void VgaNone::set_window_grab(int mode)
{
}
//-------- End of function VgaNone::set_window_grab ----------//


//-------- Beginning of function VgaNone::flip ----------//
void VgaNone::flip()
{
}
//-------- End of function VgaNone::flip ----------//

//-------- Beginning of function VgaNone::save_status_report ---------//
void VgaNone::save_status_report()
{
}
//-------- End of function VgaNone::save_status_report ----------//

