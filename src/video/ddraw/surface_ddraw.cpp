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

//Filename    : surface_ddraw.cpp
//Description : direct draw surface class

#include <surface_ddraw.h>

#include <ALL.h>
#include <OVGA.h>

//-------- Begin of function SurfaceDDraw::SurfaceDDraw ----------//

SurfaceDDraw::SurfaceDDraw(LPDIRECTDRAWSURFACE2 surface)
{
        memset( &buf_des, 0, sizeof(buf_des) );
	cur_buf_ptr = NULL;
	buf_locked = 0;
	dd_buf = surface;
}
//-------- End of function SurfaceDDraw::SurfaceDDraw ----------//


//-------- Begin of function SurfaceDDraw::~SurfaceDDraw ----------//

SurfaceDDraw::~SurfaceDDraw()
{
	if( dd_buf )
	{
		if( buf_locked )
			unlock_buf();

		dd_buf->Release();
		dd_buf = NULL;
	}
}
//-------- End of function SurfaceDDraw::~SurfaceDDraw ----------//


//------------- Begin of function SurfaceDDraw::lock_buf --------------//
//
// Return true when surface is locked, false when there is an error.
//
int SurfaceDDraw::lock_buf()
{
        memset( &buf_des, 0, sizeof(buf_des) );

        buf_des.dwSize = sizeof(buf_des);

        int rc = dd_buf->Lock(NULL, &buf_des, DDLOCK_WAIT, NULL);

        cur_buf_ptr = (char*) buf_des.lpSurface;

        if( rc==DD_OK )
		buf_locked = 1;

	return buf_locked;
}
//------------- End of function SurfaceDDraw::lock_buf --------------//


//------------- Begin of function SurfaceDDraw::unlock_buf --------------//
//
// Return true when surface is unlocked, false when there is an error.
//
int SurfaceDDraw::unlock_buf()
{
	if( !buf_locked )
		return 0;

	int rc = dd_buf->Unlock(NULL);

	if( rc==DD_OK )
		buf_locked = 0;

	return !buf_locked;
}
//--------------- End of function SurfaceDDraw::unlock_buf --------------//


//-------- Begin of function VgaBuf::activate_pal ----------//
//
// Activate a palette to the current direct draw surface buffer.
//
int SurfaceDDraw::activate_pal(LPDIRECTDRAWPALETTE ddPalPtr)
{
	HRESULT rc = dd_buf->SetPalette(ddPalPtr);

	if( rc == DDERR_SURFACELOST )
	{
		dd_buf->Restore();
		rc = dd_buf->SetPalette(ddPalPtr);
	}

	if( rc != DD_OK )
		return 0;
	return 1;
}
//--------- End of function SurfaceDDraw::activate_pal ----------//


//-------- Begin of function SurfaceDDraw::is_buf_lost ----------//
//
int SurfaceDDraw::is_buf_lost()
{
	return dd_buf->IsLost() == DDERR_SURFACELOST;
}
//--------- End of function SurfaceDDraw::is_buf_lost ----------//


//-------- Begin of function SurfaceDDraw::restore_buf ----------//
//
// Restore buffers that have been lost.
//
int SurfaceDDraw::restore_buf()
{
	if( dd_buf->Restore() != DD_OK )
		 return 0;
	return 1;
}
//--------- End of function SurfaceDDraw::restore_buf ----------//


//-------- Begin of function SurfaceDDraw::blt_virtual_buf --------//
//
// Blit entire source surface to local destination surface.
//
void SurfaceDDraw::blt_virtual_buf( SurfaceDDraw *source )
{
	RECT bltRect;

	bltRect.left   = 0;
	bltRect.top    = 0;
	bltRect.right  = VGA_WIDTH-1;
	bltRect.bottom = VGA_HEIGHT-1;

	dd_buf->BltFast( 0, 0,
			 source->dd_buf,   // src surface
			 &bltRect,         // src rect (all of it)
			 DDBLTFAST_WAIT );
}
//--------- End of function SurfaceDDraw::blt_virtual_buf ---------//


//------------ Begin of function SurfaceDDraw::write_bmp_file --------------//
//
// Load a BMP file into the current VgaBuf DIB object.
//
// <char*> fileName - the name of the BMP file.
//
// return : <int> 1-succeeded, 0-failed.
//
int SurfaceDDraw::write_bmp_file(char* fileName)
{
	File bmpFile;

	bmpFile.file_create(fileName, 1, 0); // 1-handle error, 0-disable variable file size

	//------------ Write the file header ------------//

	BITMAPFILEHEADER bmpFileHdr;

	bmpFileHdr.bfType 		= 0x4D42; // set the type to "BM"
	bmpFileHdr.bfSize 		= buf_size();
	bmpFileHdr.bfReserved1 = 0;
	bmpFileHdr.bfReserved2 = 0;
	bmpFileHdr.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;

	bmpFile.file_write(&bmpFileHdr, sizeof(bmpFileHdr));

	//------------ Write in the info header -----------//

	BITMAPINFOHEADER bmpInfoHdr;

	bmpInfoHdr.biSize          = sizeof(BITMAPINFOHEADER);
	bmpInfoHdr.biWidth         = buf_des.dwWidth;
	bmpInfoHdr.biHeight        = buf_des.dwHeight;
	bmpInfoHdr.biPlanes        = 1; 
	bmpInfoHdr.biBitCount      = 8;
	bmpInfoHdr.biCompression   = BI_RGB; 
	bmpInfoHdr.biSizeImage	   = buf_size();
	bmpInfoHdr.biXPelsPerMeter = 0;
	bmpInfoHdr.biYPelsPerMeter = 0; 
	bmpInfoHdr.biClrUsed       = 0; 
	bmpInfoHdr.biClrImportant  = 0; 

	bmpFile.file_write(&bmpInfoHdr, sizeof(bmpInfoHdr));

	//------------ write the color table -----------//

	LPDIRECTDRAWPALETTE ddPalettePtr; // get the direct draw surface's palette
	dd_buf->GetPalette(&ddPalettePtr);

	PALETTEENTRY *palEntries = (PALETTEENTRY*) mem_add( sizeof(PALETTEENTRY)*256 );
	ddPalettePtr->GetEntries(0, 0, 256, palEntries);

	RGBQUAD *colorTable = (RGBQUAD*) mem_add( sizeof(RGBQUAD)*256 ); // allocate a color table with 256 entries 

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

	for( int y=buf_height()-1 ; y>=0 ; y-- ) // write in reversed order as DIB's vertical order is reversed
		bmpFile.file_write(buf_ptr(0,y), buf_width());

	//------------ close the file -----------//

	bmpFile.file_close();

	return 1;
}
//------------ End of function SurfaceDDraw::write_bmp_file --------------//
