/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

//Filename    : OVGABUF.CPP
//Description : VgaBuf class, SDL_Surface version

#include <ALL.h>
#include <OMOUSE.h>
#include <IMGFUN.h>
#include <OSYS.h>
#include <OWORLD.h>
#include <OVGA.h>
#include <OVGABUF.h>
#include <gettext.h>
#include <CmdLine.h>

//-------- Begin of function VgaBuf::VgaBuf ----------//

VgaBuf::VgaBuf()
{
	surface = NULL;
	cur_buf_ptr = NULL;
	buf_locked = 0;
	is_front = 0;
	save_locked_flag = 0;
}
//-------- End of function VgaBuf::VgaBuf ----------//


//-------- Begin of function VgaBuf::~VgaBuf ----------//

VgaBuf::~VgaBuf()
{
	deinit();
}
//-------- End of function VgaBuf::~VgaBuf ----------//


//------ Begin of function VgaBuf::init --------//
//
// Create a direct draw back buffer.
//
// [int] w : width of the surface [default 0 : VGA_WIDTH]
// [int] h : height of the surface [default 0 : VGA_HEIGHT]
//
void VgaBuf::init(char front, int w, int h)
{
	if( !w )
		w = VGA_WIDTH;
	if( !h )
		h = VGA_HEIGHT;
	surface = SDL_CreateRGBSurface(0, w, h, VGA_BPP, 0, 0, 0, 0);
	if( !surface )
	{
		if( front )
			sys.show_error_dialog(_("An error occurred creating the front surface!"));
		else
			sys.show_error_dialog(_("An error occurred creating the back surface!"));
		return;
	}
	cur_buf_ptr = (char*)surface->pixels;
	is_front = front;
}
//-------- End of function VgaBuf::init ----------//


//------ Begin of function VgaBuf::deinit --------//

void VgaBuf::deinit()
{
	if( surface )
	{
		SDL_FreeSurface(surface);
		surface = NULL;
	}
	cur_buf_ptr = NULL;
}
//-------- End of function VgaBuf::deinit ----------//


//------- Begin of function VgaBuf::activate_pal ----------//
//
// Set a palette for the current surface.
//
void VgaBuf::activate_pal(SDL_Color *pal)
{
	SDL_SetPaletteColors(surface->format->palette, pal, 0, 256);
}
//--------- End of function VgaBuf::activate_pal ----------//


//------------- Begin of function VgaBuf::lock_buf --------------//

void VgaBuf::lock_buf()
{
	err_if( buf_locked )
		err_now( "VgaBuf::lock_buf() error, buffer already locked." );

	if( 1 )
		buf_locked = 1;
	else
	{
		if( is_front )
			err_now( "VgaBuf::lock_buf() locking front buffer failed." );
		else
			err_now( "VgaBuf::lock_buf() locking back buffer failed." );
	}
}
//--------------- End of function VgaBuf::lock_buf --------------//


//------------- Begin of function VgaBuf::unlock_buf --------------//

void VgaBuf::unlock_buf()
{
	// ####### begin Gilbert 16/9 #####//
	if( !surface )
		return;
	// ####### end Gilbert 16/9 #####//
	err_when( !buf_locked );

	if( 1 )
	{
		buf_locked = 0;
		if( cmd_line.enable_if && is_front )
			vga.flip();
	}
	else
	{
		if( is_front )
			err_now( "VgaBuf::unlock_buf() unlocking front buffer failed." );
		else
			err_now( "VgaBuf::unlock_buf() unlocking back buffer failed." );
	}
}
//--------------- End of function VgaBuf::unlock_buf --------------//


//------------- Begin of function VgaBuf::temp_unlock --------------//
//
// Unlock the Vga buffer temporarily.
//
void VgaBuf::temp_unlock()
{
	// ######### begin Gilbert 16/9 ########//
	save_locked_flag = buf_locked;

	if( buf_locked )
		unlock_buf();
	// ######### end Gilbert 16/9 ########//
}
//--------------- End of function VgaBuf::temp_unlock --------------//


//------------- Begin of function VgaBuf::temp_restore_lock --------------//
//
// Restore the previous lock stage.
//
void VgaBuf::temp_restore_lock()
{
	if( save_locked_flag )
		lock_buf();
}
//--------------- End of function VgaBuf::temp_restore_lock --------------//


//------------- Begin of function VgaBuf::put_bitmap --------------//
//
// Put a bitmap on the surface buffer
//
void VgaBuf::put_bitmap(int x,int y,char* bitmapPtr)
{
	err_when( !buf_locked );

	if( is_front )
		mouse.hide_area( x, y, x+*((short*)bitmapPtr)-1, y+*(((short*)bitmapPtr)+1)-1 );

	IMGblt(buf_ptr(), buf_pitch(), x, y, bitmapPtr);

	if( is_front )
		mouse.show_area();
}
//--------------- End of function VgaBuf::put_bitmap --------------//


//------- Begin of function VgaBuf::put_bitmap_trans --------//
//
// Put a bitmap on the surface buffer and hide the mouse cursor while displaying
//
void VgaBuf::put_bitmap_trans(int x,int y,char* bitmapPtr)
{
	err_when( !buf_locked );

	if( is_front )
		mouse.hide_area( x, y, x+*((short*)bitmapPtr)-1, y+*(((short*)bitmapPtr)+1)-1 );

	IMGbltTrans(buf_ptr(), buf_pitch(), x, y, bitmapPtr);

	if( is_front )
		mouse.show_area();
}
//--------- End of function VgaBuf::put_bitmap_trans --------//


//------- Begin of function VgaBuf::put_bitmap_remap --------//
//
// Put a bitmap on the surface buffer and hide the mouse cursor while displaying
//
void VgaBuf::put_bitmap_remap(int x,int y,char* bitmapPtr,char *colorTable)
{
	err_when( !buf_locked );

	if( is_front )
		mouse.hide_area( x, y, x+*((short*)bitmapPtr)-1, y+*(((short*)bitmapPtr)+1)-1 );

	IMGbltRemap(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTable);

	if( is_front )
		mouse.show_area();
}
//--------- End of function VgaBuf::put_bitmap_remap --------//


//---------- Begin of function VgaBuf::save_area_common_buf ----------//
//
// Save screen area to sys.common_data_buf.
//
void VgaBuf::save_area_common_buf(int x1, int y1, int x2, int y2)
{
	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	long saveSize = sizeof(short)*6 + (x2-x1+1) * (y2-y1+1);

	err_if( saveSize > COMMON_DATA_BUF_SIZE )
		err_now( "VgaBuf::save_area_common_buf()" );

	short* shortPtr = (short*) sys.common_data_buf;

	*shortPtr++ = x1;
	*shortPtr++ = y1;
	*shortPtr++ = x2;
	*shortPtr++ = y2;

	//-------- read screen ---------//

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );  // if the mouse cursor is in that area, hide it

	read_bitmap( x1,y1,x2,y2, (char*) shortPtr );

	if( is_front )
		mouse.show_area();
}
//------------ End of function VgaBuf::save_area_common_buf ----------//


//---------- Begin of function VgaBuf::rest_area_common_buf ----------//
//
// Restore screen area to the screen from Vga image buffer.
// This screen should be previously saved by save_area()
//
void VgaBuf::rest_area_common_buf()
{
	short* shortPtr = (short*) sys.common_data_buf;

	int x1 = *shortPtr++;
	int y1 = *shortPtr++;
	int x2 = *shortPtr++;
	int y2 = *shortPtr++;

	put_bitmap( x1, y1, (char*) shortPtr );
}
//------------ End of function VgaBuf::rest_area_common_buf ----------//


//---------- Begin of function VgaBuf::save_area ---------//
//
// save_area() differs from save_area() as :
//
// save_area() save the screen to a user-defined buffer.
// save_area()  save the screen to the global screen saving buffer in Vga object
//
// <int>   x1,y1,x2,y2 = the area of the screen
// [char*] saveScr     = the pointer to the previous saved buffer
//                       (default : initialize a new buffer)
//
char* VgaBuf::save_area(int x1, int y1, int x2, int y2, char* saveScr )
{
	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	long newSize = sizeof(short)*6 + (x2-x1+1) * (y2-y1+1);

	saveScr = mem_resize( saveScr, newSize );

	short* shortPtr = (short*) saveScr;

	*shortPtr++ = x1;
	*shortPtr++ = y1;
	*shortPtr++ = x2;
	*shortPtr++ = y2;

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );  // if the mouse cursor is in that area, hide it

	read_bitmap( x1,y1,x2,y2, (char*) shortPtr );

	if( is_front )
		mouse.show_area();

   return saveScr;
}
//------------ End of function VgaBuf::save_area ---------//


//----------- Begin of function VgaBuf::rest_area --------//
//
// Restore previously saved screen
//
// char* saveScr     = the previously saved screen
// [int] releaseFlag = whether release the buffer of the saved screen or not
//                     (default : 1)
//
void VgaBuf::rest_area(char* saveScr, int releaseFlag)
{
   int  x1,y1,x2,y2;

   if( saveScr == NULL )
      return;

   short* shortPtr = (short*) saveScr;

	x1 = *shortPtr++;
   y1 = *shortPtr++;
   x2 = *shortPtr++;
   y2 = *shortPtr++;

   err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	put_bitmap( x1, y1, (char*) shortPtr );

	if( releaseFlag )
		mem_del( saveScr );
}
//------------ End of function VgaBuf::rest_area ----------//


//---------- Begin of function VgaBuf::put_large_bitmap ---------//
//
// Put a picture on the screen, when a picture's size is > 64K
// Pass a file pointer to put_large_bitmap() for continously reading the file
//
// Syntax
//
// <int>   x, y    = the location of the picture on the screen
// <FILE*> filePtr = pointer to the
//
//--------- Format of Picture buffer -------//
//
// int          = whether this picture buffer contains the palette or not
//                -1 (0xFF) if has, otherwise not
// char[256][3] = VGA color palette
//
// <int> = picture width
// <int> = picture height
//
// char[...]    = non-compressed flat picture bitmap
//
//---------------------------------------------//

void VgaBuf::put_large_bitmap(int x1, int y1, File* filePtr)
{
	if( filePtr == NULL )
		return;

	int pictWidth = filePtr->file_get_short();
	int hasPalette=0;

	//------- read in color palette if this picture has one -----//
/*
	if( pictWidth == -1 )   // if the has palette in it
	{
		//------- set all palette to dark black ------//

		ColorPal colorPal;

		colorPal.red=0;
		colorPal.green=0;
		colorPal.blue=0;

		palette.set_single(colorPal);

		//-------- get the palette of the picture --------//

		ColorPal palBuf[256];

		filePtr->file_read( palBuf, sizeof(ColorPal)*256 );

		palette.set_fade_in(0, 255, palBuf, 15 );  // 15 steps for the whole fade in process, each step 0.01 seconds, 10 steps = 0.1 second

		hasPalette=1;

		pictWidth = filePtr->file_get_short();
	}
*/
	//------ read in bitmap and display it --------//

	int pictHeight = filePtr->file_get_short();
	int x2 = x1 + pictWidth  - 1;
	int y2 = y1 + pictHeight - 1;

	long pictSize = (long) x2 * y2;

	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	//---- if pict size less than 64K, read in the picture in one step ----//

	if( pictSize <= COMMON_DATA_BUF_SIZE )
	{
		filePtr->file_read( sys.common_data_buf, pictSize );

		if( is_front )
			mouse.hide_area( x1,y1,x2,y2 );  // if the mouse cursor is in that area, hide it

		put_bitmap2( x1, y1, pictWidth, pictHeight, sys.common_data_buf );

		if( is_front )
			mouse.show_area();
	}
	else //----- if the picture size > 64K, read in line by line -----//
	{
		int bufferLine = COMMON_DATA_BUF_SIZE / pictWidth;   // MAX. no. of lines can be in the buffer
		int ty=y1+bufferLine-1;

		if( ty> y2 )
			ty=y2;

		while( y1<=y2 )
		{
			filePtr->file_read( sys.common_data_buf, (unsigned)pictWidth * (ty-y1+1) );

			if( is_front )
				mouse.hide_area( x1,y1,x2,ty );  // if the mouse cursor is in that area, hide it

			put_bitmap2( x1, y1, pictWidth, ty-y1+1, sys.common_data_buf );

			if( is_front )
				mouse.show_area();

			y1 += bufferLine;

			if( (ty+=bufferLine) > y2 )
				ty=y2;
		}
	}
/*
	if( hasPalette )
		while( !palette.fade_in() );
*/
}
//----------- End of function VgaBuf::put_large_bitmap --------//


//----------- Begin of function VgaBuf::convert_gray ----------//
//
// Convert an specified area of the bitmap from color to gray-scale.
//
void VgaBuf::convert_gray(int x1, int y1, int x2, int y2)
{
	remap_bar(x1, y1, x2, y2, vga.gray_remap_table);
}
//--------- End of function VgaBuf::convert_gray -----------//
