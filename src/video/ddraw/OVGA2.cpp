/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

//Filename   : OVGA2.CPP
//Description: Functions for display big image

#include <OSTR.h>
#include <OSYS.h>
#include <OFILE.h>
#include <OMOUSE.h>
#include <OVGALOCK.h>
#include <OVGA.h>

//---------- Begin of function Vga::disp_image_file --------//
//
// <char*> fileName - file name of the image
// [int]   x1, y1   - the top left display position of the image
//							 (default: 0)
//
void Vga::disp_image_file(const char* fileName, int x1, int y1)
{
	//-------- backup and switch palette ----------//

	if( !back_up_pal )		// only save the palette when there isn't one saved already
		back_up_pal = new VgaCustomPalette(NULL);

	//---- load the interface into the back buffer ----//

	File pictFile;
	String str;

	str  = DIR_IMAGE;
	str += fileName;
	str += ".ICN";

	if( pictFile.file_open(str,0) )
	{
		vga_back.put_large_bitmap(x1, y1, &pictFile);
		pictFile.file_close();
	}

	//-------- hide and change mouse cursor --------//

	mouse.hide();

	//------ turn screen dark and blt the buffer ---------//

	vga_front.bar( 0, 0, VGA_WIDTH-1, VGA_HEIGHT-1, 0 );
	sys.blt_virtual_buf();

	//------- Set custom palette -------//

	{
		str  = DIR_IMAGE;
		str += fileName;
		str += ".COL";

		err_when( !m.is_file_exist(str) );

		VgaFrontLock vgaLock;
		set_custom_palette(str);
	}

	//------- bilt the back buffer to the front ---------//

	vga.blt_buf( 0,0, vga_back.buf_width()-1, vga_back.buf_height()-1, 0 );
}
//----------- End of function Vga::disp_image_file ---------//


//---------- Begin of function Vga::finish_disp_image_file --------//
//
void Vga::finish_disp_image_file()
{
	//------- exiting: turn dark --------//

	vga_front.bar( 0, 0, VGA_WIDTH-1, VGA_HEIGHT-1, 0 );
	sys.blt_virtual_buf();

	//----- palette restore when back_up_pal destruct ----//

	{
		VgaFrontLock vgaLock;

		err_when( !back_up_pal );

		delete back_up_pal;
		back_up_pal = NULL;
	}

	mouse.show();
}
//----------- End of function Vga::finish_disp_image_file ---------//
