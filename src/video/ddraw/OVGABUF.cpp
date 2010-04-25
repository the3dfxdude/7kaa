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

//Filename    : OVGABUF.CPP
//Description : OVGABUF direct draw surface class

#include <ALL.h>
#include <OMOUSE.h>
#include <IMGFUN.h>
#include <OSYS.h>
#include <OWORLD.h>
#include <OVGA.h>
#include <OVGABUF.h>

//-------- Begin of function VgaBuf::VgaBuf ----------//

VgaBuf::VgaBuf()
{
	memset( this, 0, sizeof(VgaBuf) );
}
//-------- End of function VgaBuf::VgaBuf ----------//


//-------- Begin of function VgaBuf::~VgaBuf ----------//

VgaBuf::~VgaBuf()
{
	deinit();
}
//-------- End of function VgaBuf::~VgaBuf ----------//


//-------- Begin of function VgaBuf::init_front ----------//
//
// Create a direct draw front buffer.
//
void VgaBuf::init_front(LPDIRECTDRAW2 ddPtr)
{
	DDSURFACEDESC       ddsd;
	HRESULT             rc;
	DDCAPS              ddcaps;

	//------ Get Direct Draw capacity info --------//

	ddcaps.dwSize = sizeof( ddcaps );

	if( ddPtr->GetCaps( &ddcaps, NULL ) != DD_OK )
		err.run( "Error creating Direct Draw front surface!" );

	//---------------------------------------------//
	// Create the Front Buffer
	//---------------------------------------------//

	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof( ddsd );

	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	LPDIRECTDRAWSURFACE dd1Buf;
	rc = ddPtr->CreateSurface( &ddsd, &dd1Buf, NULL );
	if( rc != DD_OK )
		err.run( "Error creating Direct Draw front surface!" );

	rc = dd1Buf->QueryInterface(IID_IDirectDrawSurface2, (void **)&dd_buf);
	if( rc != DD_OK )
	{
		dd1Buf->Release();
		err.run ( "Error creating Direct Draw front surface!!" );
	}

	dd1Buf->Release();

	is_front = 1;
}
//-------- End of function VgaBuf::init_front ----------//


//-------- Begin of function VgaBuf::init_back ----------//
//
// Create a direct draw back buffer.
//
// [DWORD] w      : width of the surface [default 0 : VGA_WIDTH]
// [DWORD] h      : height of the surface [default 0 : VGA_HEIGHT]
//
void VgaBuf::init_back( LPDIRECTDRAW2 ddPtr, DWORD w, DWORD h )
{
	DDSURFACEDESC       ddsd;
	HRESULT             rc;

	//--------- fill in surface desc -----------//

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	ddsd.dwWidth  = w ? w : VGA_WIDTH;
	ddsd.dwHeight = h ? h : VGA_HEIGHT;

	LPDIRECTDRAWSURFACE dd1Buf;
	rc = ddPtr->CreateSurface( &ddsd, &dd1Buf, NULL );
	if( rc != DD_OK )
		err.run( "Error creating direct draw back surface!" );

	rc = dd1Buf->QueryInterface(IID_IDirectDrawSurface2, (void **)&dd_buf);
	if( rc != DD_OK )
	{
		dd1Buf->Release();
		err.run( "Error creating direct draw back surface!!" );
	}

	dd1Buf->Release();
}
//-------- End of function VgaBuf::init_back ----------//


//------ Begin of function VgaBuf::deinit --------//

void VgaBuf::deinit()
{
	if( dd_buf )
	{
		if( buf_locked )
			unlock_buf();

		dd_buf->Release();
		dd_buf = NULL;
	}
}
//-------- End of function VgaBuf::deinit ----------//


//-------- Begin of function VgaBuf::activate_pal ----------//
//
// Activate a palette to the current direct draw surface buffer.
//
void VgaBuf::activate_pal(LPDIRECTDRAWPALETTE ddPalPtr)
{
	err_when(!ddPalPtr || !dd_buf);

	HRESULT rc = dd_buf->SetPalette(ddPalPtr);

	if( rc == DDERR_SURFACELOST )
	{
		dd_buf->Restore();
		rc = dd_buf->SetPalette(ddPalPtr);
	}

#ifdef DEBUG
	if( rc != DD_OK )
		debug_msg( "VgaBuf::activate_pal(), failed activating the palette" );
#endif
}
//--------- End of function VgaBuf::activate_pal ----------//


//-------- Begin of function VgaBuf::color_match ----------//

DWORD VgaBuf::color_match(COLORREF rgb)
{
	COLORREF 		rgbT;
	HDC 				hdc;
	DWORD 			dw = CLR_INVALID;
	DDSURFACEDESC 	ddsd;
	HRESULT 			hres;

	if( dd_buf->GetDC(&hdc) == DD_OK )
	{
		 rgbT = GetPixel(hdc, 0, 0);
		 SetPixel(hdc, 0, 0, rgb);
		 dd_buf->ReleaseDC(hdc);
	}

	ddsd.dwSize = sizeof(ddsd);
	hres = dd_buf->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

	if (hres == DD_OK)
	{
		 dw  = *(DWORD *)ddsd.lpSurface;
		 dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;
		 dd_buf->Unlock(NULL);
	}

	if (dd_buf->GetDC(&hdc) == DD_OK)
	{
		 SetPixel(hdc, 0, 0, rgbT);
		 dd_buf->ReleaseDC(hdc);
	}

	return dw;
}
//-------- End Endof function VgaBuf::color_match ----------//


//-------- Begin of function VgaBuf::is_buf_lost ----------//
//
BOOL VgaBuf::is_buf_lost()
{
	return dd_buf && dd_buf->IsLost() == DDERR_SURFACELOST;
}
//--------- End of function VgaBuf::is_buf_lost ----------//


//-------- Begin of function VgaBuf::restore_buf ----------//
//
// Restore buffers that have been lost.
//
BOOL VgaBuf::restore_buf()
{
	if( dd_buf == NULL || dd_buf->Restore() != DD_OK )
	{
#ifdef DEBUG
		 debug_msg("Error restoring direct draw buffer");
#endif
		 return FALSE;
	}

	return TRUE;
}
//--------- End of function VgaBuf::restore_buf ----------//


//------------- Begin of function VgaBuf::lock_buf --------------//

void VgaBuf::lock_buf()
{
	err_if( buf_locked )
		err_now( "VgaBuf::lock_buf() error, buffer already locked." );

	memset( &buf_des, 0, sizeof(buf_des) );

	buf_des.dwSize = sizeof(buf_des);

	int rc = dd_buf->Lock(NULL, &buf_des, DDLOCK_WAIT, NULL);

	cur_buf_ptr = (char*) buf_des.lpSurface;

	//--------------------------------------//

	if( rc==DD_OK )
		buf_locked = TRUE;
	else
	{
		if( is_front )
			err_now( "VgaBuf::lock_buf() locking front buffer failed." );
		else
			err_now( "VgaBuf::lock_buf() locking back buffer failed." );

#ifdef DEBUG
		debug_msg( "Failed to lock the buffer." );
#endif
	}
}
//--------------- End of function VgaBuf::lock_buf --------------//


//------------- Begin of function VgaBuf::unlock_buf --------------//

void VgaBuf::unlock_buf()
{
	// ####### begin Gilbert 16/9 #####//
	if( !dd_buf )
		return;
	// ####### end Gilbert 16/9 #####//
	err_when( !buf_locked );

	int rc = dd_buf->Unlock(NULL);

	if( rc==DD_OK )
		buf_locked = FALSE;
	else
	{
		switch(rc)
		{
		case DDERR_INVALIDOBJECT:
			err_now( "VgaBuf::unlock_buf error: DDERR_INVALIDOBJECT" );
				
		case DDERR_INVALIDPARAMS:
			err_now( "VgaBuf::unlock_buf error: DDERR_INVALIDPARAMS" );
				
		case DDERR_INVALIDRECT:
			err_now( "VgaBuf::unlock_buf error: DDERR_INVALIDRECT" );
				
		case DDERR_NOTLOCKED:
			err_now( "VgaBuf::unlock_buf error: DDERR_NOTLOCKED" );
				
		case DDERR_SURFACELOST:
			err_now( "VgaBuf::unlock_buf error: DDERR_SURFACELOST" );
		}

		if( is_front )
			err_now( "VgaBuf::unlock_buf() unlocking front buffer failed." );
		else
			err_now( "VgaBuf::unlock_buf() unlocking back buffer failed." );

#ifdef DEBUG
		debug_msg( "Failed to unlock the buffer." );
#endif
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


//------------ Begin of function VgaBuf::write_bmp_file --------------//
//
// Load a BMP file into the current VgaBuf DIB object.
//
// <char*> fileName - the name of the BMP file.
//
// return : <int> 1-succeeded, 0-failed.
//
int VgaBuf::write_bmp_file(char* fileName)
{
	 File				bmpFile;
	 BITMAPINFO*	bmpInfoPtr = NULL;
	 char*			bitmapPtr = NULL;

	 bmpFile.file_create(fileName, 1, 0);		// 1-handle error, 0-disable variable file size

	 //------------ Write the file header ------------//

	 BITMAPFILEHEADER bmpFileHdr;

	 bmpFileHdr.bfType 		= 0x4D42;			// set the type to "BM"
	 bmpFileHdr.bfSize 		= buf_size();
	 bmpFileHdr.bfReserved1 = 0;
	 bmpFileHdr.bfReserved2 = 0;
	 bmpFileHdr.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;

	 bmpFile.file_write(&bmpFileHdr, sizeof(bmpFileHdr));

	 //------------ Write in the info header -----------//

	 BITMAPINFOHEADER bmpInfoHdr;

	 bmpInfoHdr.biSize			 = sizeof(BITMAPINFOHEADER);
	 bmpInfoHdr.biWidth			 = buf_des.dwWidth;
	 bmpInfoHdr.biHeight			 = buf_des.dwHeight;
	 bmpInfoHdr.biPlanes			 = 1; 
	 bmpInfoHdr.biBitCount		 = 8;
    bmpInfoHdr.biCompression	 = BI_RGB; 
	 bmpInfoHdr.biSizeImage	    = buf_size();
	 bmpInfoHdr.biXPelsPerMeter = 0;
    bmpInfoHdr.biYPelsPerMeter = 0; 
	 bmpInfoHdr.biClrUsed		 = 0; 
    bmpInfoHdr.biClrImportant  = 0; 

	 bmpFile.file_write(&bmpInfoHdr, sizeof(bmpInfoHdr));

	 //------------ write the color table -----------//

	 LPDIRECTDRAWPALETTE ddPalettePtr;				// get the direct draw surface's palette
	 dd_buf->GetPalette(&ddPalettePtr);

	 PALETTEENTRY *palEntries = (PALETTEENTRY*) mem_add( sizeof(PALETTEENTRY)*256 );
	 ddPalettePtr->GetEntries(0, 0, 256, palEntries);
	
	 RGBQUAD *colorTable = (RGBQUAD*) mem_add( sizeof(RGBQUAD)*256 );		// allocate a color table with 256 entries 
		
	 for( int i=0 ; i<256 ; i++ )
	 {
		 colorTable[i].rgbBlue  = palEntries[i].peBlue;
		 colorTable[i].rgbGreen = palEntries[i].peGreen;
		 colorTable[i].rgbRed   = palEntries[i].peRed; 
		 colorTable[i].rgbReserved = 0;
	 }
		 
	 bmpFile.file_write(colorTable, sizeof(RGBQUAD)*256);

	 mem_del(palEntries);
	 mem_del(colorTable);

	 //----------- write the bitmap ----------//

	 for( int y=buf_height()-1 ; y>=0 ; y-- )					// write in reversed order as DIB's vertical order is reversed
		 bmpFile.file_write(buf_ptr(0,y), buf_width());

	 //------------ close the file -----------//

	 bmpFile.file_close();

	 return 1;
}
//------------ End of function VgaBuf::write_bmp_file --------------//


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

