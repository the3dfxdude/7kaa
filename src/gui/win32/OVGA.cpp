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

//Filename    : OVGA.CPP
//Description : VGA manipulation functions (Direct Draw version)

#define DEBUG_LOG_LOCAL 1

#include <windowsx.h>

#include <ALL.h>
#include <IMGFUN.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OCOLTBL.h>
#include <OFILE.h>
#include <OSYS.h>
#include <syswin.h>
#include <OVGA.h>
#include <OLOG.h>
// ##### begin Gilbert 16/9 #######//
#include <OVGALOCK.h>
// ##### end Gilbert 16/9 #######//

//-------- Define constant --------//

#define UP_OPAQUE_COLOR       (VGA_GRAY+10)
#define DOWN_OPAQUE_COLOR     (VGA_GRAY+13)

//------ Define static class member vars ---------//

char    Vga::use_back_buf = 0;
char    Vga::opaque_flag  = 0;
VgaBuf* Vga::active_buf   = &vga_front;      // default: front buffer

char    low_video_memory_flag = 0;

//-------- Begin of function Vga::Vga ----------//

Vga::Vga()
{
	memset( this, 0, sizeof(Vga) );

   vga_color_table = new ColorTable;
}
//-------- End of function Vga::Vga ----------//


//-------- Begin of function Vga::~Vga ----------//

Vga::~Vga()
{
   deinit();      // 1-is final

   delete vga_color_table;

   err_when( back_up_pal );      // this must be free up immediately after its use
}
//-------- End of function Vga::~Vga ----------//


//-------- Begin of function Vga::init ----------//

BOOL Vga::init()
{
   const char* warnStr = "Warning: Due to the low memory of your display card, "
                   "you may experience problems when you quit the game or "
                   "switch tasks during the game. "
                   "To avoid this problem, set your Windows display "
                   "to 800x600 256 color mode before running the game.";

   //--------- Initialize DirectDraw object --------//

   if( !init_dd() )
      return FALSE;

   // get current display mode
   DDSURFACEDESC ddsd;
   DDSCAPS  ddsCaps;
   DWORD    dwTotal;
	DWORD    dwFree;

	memset(&ddsd, 0, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);
	ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

	if( dd_obj->GetDisplayMode(&ddsd) == DD_OK &&
		dd_obj->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree) == DD_OK )
	{
		if( dwFree < VGA_WIDTH*VGA_HEIGHT*VGA_BPP/8 &&
			!(ddsd.dwWidth==VGA_WIDTH && ddsd.dwHeight==VGA_HEIGHT && (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)) )
		{
			// not enough memory except same video mode

			ShowCursor(TRUE);
			// approximation of video memory required, actual video memory used should be calculated from vga_(true)_front->buf_pitch()

			extern char new_config_dat_flag;

			if( new_config_dat_flag )
			{
				MessageBox(window.main_hwnd, warnStr,
					WIN_TITLE, MB_OK | MB_ICONWARNING | MB_SETFOREGROUND );
			}

			low_video_memory_flag = 1;

         ShowCursor(FALSE);
      }
   }

   if( !set_mode() )
      return FALSE;

   return TRUE;
}
//-------- End of function Vga::init ----------//


//-------- Begin of function Vga::init_dd ----------//

BOOL Vga::init_dd()
{
   if(dd_obj)        // the Direct Draw object has been initialized already
      return TRUE;

   //--------- Create direct draw object --------//

   DEBUG_LOG("Attempt DirectDrawCreate");
   LPDIRECTDRAW dd1Obj;
   int rc = DirectDrawCreate( NULL, &dd1Obj, NULL );
   DEBUG_LOG("DirectDrawCreate finish");

   if( rc != DD_OK )
   {
#ifdef DEBUG
      debug_msg("DirectDrawCreate failed err=%d", rc);
#endif
      return FALSE;
   }

   //-------- Query DirectDraw2 interface --------//

   DEBUG_LOG("Attempt Query DirectDraw2");
   rc = dd1Obj->QueryInterface(IID_IDirectDraw2, (void **)&dd_obj);
   DEBUG_LOG("Query DirectDraw2 finish");
   if( rc != DD_OK )
   {
#ifdef DEBUG
      debug_msg("Query DirectDraw2 failed err=%d", rc);
#endif
      dd1Obj->Release();
      return FALSE;
   }

   dd1Obj->Release();

   return TRUE;
}
//-------- End of function Vga::init_dd ----------//


//-------- Begin of function Vga::set_mode ----------//

BOOL Vga::set_mode()
{
   DWORD   dwStyle;
   HRESULT rc;

   //-----------------------------------------------------------//
   // Convert it to a plain window
   //-----------------------------------------------------------//

   dwStyle = GetWindowStyle(window.main_hwnd);
   dwStyle |= WS_POPUP;
   dwStyle &= ~(WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX);
   SetWindowLong(window.main_hwnd, GWL_STYLE, dwStyle);

   //-----------------------------------------------------------//
   // grab exclusive mode if we are going to run as fullscreen
   // otherwise grab normal mode.
   //-----------------------------------------------------------//

   DEBUG_LOG("Attempt DirectDraw SetCooperativeLevel");
   rc = dd_obj->SetCooperativeLevel( window.main_hwnd,
                        DDSCL_EXCLUSIVE |
                        DDSCL_FULLSCREEN );
   DEBUG_LOG("DirectDraw SetCooperativeLevel finish");

   if( rc != DD_OK )
   {
#ifdef DEBUG
      debug_msg("SetCooperativeLevel failed err=%d", rc);
#endif
      return FALSE;
   }

   //-------------- set Direct Draw mode ---------------//

   DEBUG_LOG("Attempt DirectDraw SetDisplayMode");
   // ###### begin Gilbert 25/7 #######//
   // IDirectDraw2::SetDisplayMode requires 5 parameters
   rc = dd_obj->SetDisplayMode( VGA_WIDTH, VGA_HEIGHT, VGA_BPP, 0, 0);
   // ###### end Gilbert 25/7 #######//
   DEBUG_LOG("DirectDraw SetDisplayMode finish");

   if( rc != DD_OK )
   {
#ifdef DEBUG
      debug_msg("SetMode failed err=%d", rc);
#endif
      return FALSE;
   }

   //----------- display the system cursor -------------//

   SetCursor(NULL);

   return TRUE;
}
//-------- End of function Vga::set_mode ----------//


//-------- Begin of function Vga::deinit ----------//

void Vga::deinit()
{
   release_pal();

   if( dd_obj )
   {
      //DEBUG_LOG("Attempt vga.dd_obj->RestoreDisplayMode()");
      // dd_obj->RestoreDisplayMode();
      //DEBUG_LOG("vga.dd_obj->RestoreDisplayMode() finish");

      DEBUG_LOG("Attempt vga.dd_obj->Release()");
      dd_obj->Release();
      DEBUG_LOG("vga.dd_obj->Release() finish");
      dd_obj = NULL;
   }
}
//-------- End of function Vga::deinit ----------//

//--------- Start of function Vga::init_surface ----------//
//
// VgaBuf should call Vga to get the system's surface,
// however, it is currently used the other way around.
//
void Vga::init_surface(VgaBuf* surface, enum vga_surface_type t)
{
   if( !dd_obj ) return;

   if( t == VGA_FRONT )
   {
      surface->init_front(dd_obj);
   }
   else if( t == VGA_BACK )
   {
      surface->init_back(dd_obj);
   }
}
//-------- End of function Vga::init_surface ----------//

//--------- Start of function Vga::load_pal ----------//
//
// Load the palette from a file and set it to the front buf.
//
BOOL Vga::load_pal(const char* fileName)
{
   char palBuf[256][3];
   File palFile;

   palFile.file_open(fileName);
   palFile.file_seek(8);               // bypass the header info
   palFile.file_read(palBuf, 256*3);
   palFile.file_close();

    //--- Create a Direct Draw Palette and associate it with the front buffer ---//

   if( dd_pal == NULL )
   {
      for(int i=0; i<256; i++)
      {
         pal_entry_buf[i].peRed   = palBuf[i][0];
         pal_entry_buf[i].peGreen = palBuf[i][1];
         pal_entry_buf[i].peBlue  = palBuf[i][2];
      }

      HRESULT rc = dd_obj->CreatePalette( DDPCAPS_8BIT, pal_entry_buf, &dd_pal, NULL );

      if( rc != DD_OK )
         return FALSE;
   }

   init_color_table();
   init_gray_remap_table();

   return TRUE;
}
//----------- End of function Vga::load_pal ----------//

//--------- Start of function Vga::refresh_palette ----------//
//
// When the system changes the palette, this function will set
// the palette back to the correct entries.
//
void Vga::refresh_palette()
{
   // we can't restore if dd_pal is not initialized
   if (!dd_pal) return;

   // if we are temporarily overriding, then we should be okay
   if (back_up_pal) return;

   // restore palette
   dd_pal->SetEntries(0, 0, 256, pal_entry_buf);
}
//----------- End of function Vga::refresh_palette ----------//

//--------- Start of function Vga::init_color_table ----------//

void Vga::init_color_table()
{
   //----- initialize interface color table -----//

   PalDesc palDesc( (unsigned char*) pal_entry_buf, sizeof(PALETTEENTRY), 256, 8);
   vga_color_table->generate_table( MAX_BRIGHTNESS_ADJUST_DEGREE, palDesc, ColorTable::bright_func );
}
//----------- End of function Vga::init_color_table ----------//


//--------- Start of function Vga::release_pal ----------//

void Vga::release_pal()
{
   // ##### begin Gilbert 16/9 #######//
   if( dd_pal )
   {
      while( dd_pal->Release() );
      dd_pal = NULL;
   }
   // ##### end Gilbert 16/9 #######//
}
//----------- End of function Vga::release_pal ----------//


//-------- Begin of function Vga::activate_pal ----------//
//
// we are getting the palette focus, select our palette
//
void Vga::activate_pal(VgaBuf* vgaBufPtr)
{
   vgaBufPtr->activate_pal(dd_pal);
}
//--------- End of function Vga::activate_pal ----------//


//-------- Begin of function Vga::adjust_brightness ----------//
//
// <int> changeValue - the value to add to the RGB values of
//                     all the colors in the palette.
//                     the value can be from -255 to 255.
//
// <int> preserveContrast - whether preserve the constrast or not
//
void Vga::adjust_brightness(int changeValue)
{
   //---- find out the maximum rgb value can change without affecting the contrast ---//

   int          i;
   int          newRed, newGreen, newBlue;
   PALETTEENTRY palBuf[256];

   //------------ change palette now -------------//

   for( i=0 ; i<256 ; i++ )
   {
      newRed   = (int)pal_entry_buf[i].peRed   + changeValue;
      newGreen = (int)pal_entry_buf[i].peGreen + changeValue;
      newBlue  = (int)pal_entry_buf[i].peBlue  + changeValue;

      palBuf[i].peRed   = MIN(255, MAX(newRed,0) );
      palBuf[i].peGreen = MIN(255, MAX(newGreen,0) );
      palBuf[i].peBlue  = MIN(255, MAX(newBlue,0) );
   }

   //------------ set palette ------------//

   vga_front.temp_unlock();

   dd_pal->SetEntries( 0, 0, 256, palBuf );

   vga_front.temp_restore_lock();
}
//--------- End of function Vga::adjust_brightness ----------//


//--------- Begin of function Vga::blt_buf ----------//
//
// Blt the back buffer to the front buffer.
//
// <int> x1, y1, x2, y2 - the coordinations of the area to be blit
// [int] putBackCursor  - whether put a mouse cursor onto the back buffer
//                        before blitting.
//                        (default: 1)
//
BOOL Vga::blt_buf(int x1, int y1, int x2, int y2, int putBackCursor)
{
   if( putBackCursor )
   {
      mouse_cursor.hide_area_flag = 0;    // do not call mouse.hide_area() which will double paint the cursor

      mouse_cursor.hide_x1 = x1;
      mouse_cursor.hide_y1 = y1;
      mouse_cursor.hide_x2 = x2;
      mouse_cursor.hide_y2 = y2;

      //-------- put mouse cursor ---------//

      mouse_cursor.disp_back_buf(x1, y1, x2, y2);
   }
   else
   {
      mouse.hide_area(x1, y1, x2, y2);
   }

   //--------------------------------------//

   IMGcopy( vga_front.buf_ptr(), vga_front.buf_pitch(),
      vga_back.buf_ptr(), vga_back.buf_pitch(), x1, y1, x2, y2 );

   //--------------------------------------//

   if( putBackCursor )
      mouse_cursor.hide_area_flag = 0;    // do not call mouse.show_area() which will double paint the cursor
   else
      mouse.show_area();

   return TRUE;
}
//---------- End of function Vga::blt_buf ----------//


//----------- Begin of function Vga::d3_panel_up ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void Vga::d3_panel_up(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
{
   err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

   VgaBuf* vgaBuf;

   if( vgaFrontOnly )
      vgaBuf = &vga_front;
   else
      vgaBuf = &vga_back;

   if( !drawBorderOnly )
   {
      if( Vga::opaque_flag )
         vgaBuf->bar(x1+1, y1+1, x2-1, y2-1, UP_OPAQUE_COLOR);
      else
         vgaBuf->adjust_brightness(x1+1, y1+1, x2-1, y2-1, IF_UP_BRIGHTNESS_ADJUST);
   }

   mouse.hide_area( x1,y1,x2,y2 );

   //--------- white border on top and left sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+1,y1,x2,y1, IF_LIGHT_BORDER_COLOR );    // top side
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1,y1,x1,y2  , IF_LIGHT_BORDER_COLOR );    // left side

   //--------- black border on bottom and right sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+1,y2,x2,y2, IF_DARK_BORDER_COLOR );     // bottom side
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x2,y1+1,x2,y2, IF_DARK_BORDER_COLOR );     // right side

   //-------------------------------------------//

   mouse.show_area();

   //----- blt the area from the back buffer to the front buffer ------//

   if( !vgaFrontOnly && !use_back_buf )      // only blt the back to the front is the active buffer is the front
      vga.blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function Vga::d3_panel_up ------------//


//----------- Begin of function Vga::d3_panel_down ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void Vga::d3_panel_down(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
{
   err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

   VgaBuf* vgaBuf;

   if( vgaFrontOnly )
      vgaBuf = &vga_front;
   else
      vgaBuf = &vga_back;

   if( !drawBorderOnly )
   {
      if( Vga::opaque_flag )
         vgaBuf->bar(x1+1, y1+1, x2-1, y2-1, DOWN_OPAQUE_COLOR);
      else
         vgaBuf->adjust_brightness(x1+1, y1+1, x2-1, y2-1, IF_DOWN_BRIGHTNESS_ADJUST);
   }

   mouse.hide_area( x1,y1,x2,y2 );

   //--------- white border on top and left sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+1,y1,x2,y1, IF_DARK_BORDER_COLOR );    // top side
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1,y1,x1,y2  , IF_DARK_BORDER_COLOR );    // left side

   //--------- black border on bottom and right sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+1,y2,x2,y2, IF_LIGHT_BORDER_COLOR );   // bottom side
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x2,y1+1,x2,y2, IF_LIGHT_BORDER_COLOR );   // right side

   //-------------------------------------------//

   mouse.show_area();

   //----- blt the area from the back buffer to the front buffer ------//

   if( !vgaFrontOnly && !use_back_buf )      // only blt the back to the front is the active buffer is the front
      vga.blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function Vga::d3_panel_down ------------//


//----------- Begin of function Vga::d3_panel2_up ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void Vga::d3_panel2_up(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
{
   err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

   VgaBuf* vgaBuf;

   if( vgaFrontOnly )
      vgaBuf = &vga_front;
   else
      vgaBuf = &vga_back;

   if( !drawBorderOnly )
      vgaBuf->adjust_brightness(x1+2, y1+2, x2-3, y2-3, IF_UP_BRIGHTNESS_ADJUST);

   mouse.hide_area( x1,y1,x2,y2 );

   //--------- white border on top and left sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1,y1,x2-3,y1+1,0x9a );
   vgaBuf->draw_pixel(x2-2, y1, 0x9a);
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1,y1+2,x1+1,y2-3, 0x9a );    // left side
   vgaBuf->draw_pixel(x1, y2-2, 0x9a);

   //--------- black border on bottom and right sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x2-2,y1+2,x2-1,y2-1, 0x90 );     // bottom side
   vgaBuf->draw_pixel(x2-1, y1+1, 0x90);
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+2,y2-2,x2-3,y2-1, 0x90 );      // right side
   vgaBuf->draw_pixel(x1+1, y2-1, 0x90);

   //--------- junction between white and black border --------//
   vgaBuf->draw_pixel(x2-1, y1, 0x97);
   vgaBuf->draw_pixel(x2-2, y1+1, 0x97);
   vgaBuf->draw_pixel(x1, y2-1, 0x97);
   vgaBuf->draw_pixel(x1+1, y2-2, 0x97);

   //--------- gray shadow on bottom and right sides -----------//
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x2, y1+1, x2, y2, 0x97);
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+1, y2, x2-1, y2, 0x97);

   //-------------------------------------------//

   mouse.show_area();

   //----- blt the area from the back buffer to the front buffer ------//

   if( !vgaFrontOnly && !use_back_buf )      // only blt the back to the front is the active buffer is the front
      vga.blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function Vga::d3_panel_up ------------//


//----------- Begin of function Vga::d3_panel2_down ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void Vga::d3_panel2_down(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
{
   err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

   VgaBuf* vgaBuf;

   if( vgaFrontOnly )
      vgaBuf = &vga_front;
   else
      vgaBuf = &vga_back;

   if( !drawBorderOnly )
      vgaBuf->adjust_brightness(x1+2, y1+2, x2-3, y2-3, IF_DOWN_BRIGHTNESS_ADJUST);

   mouse.hide_area( x1,y1,x2,y2 );

   //--------- black border on top and left sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1,y1,x2-3,y1+1,0x90 );
   vgaBuf->draw_pixel(x2-2, y1, 0x90);
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1,y1+2,x1+1,y2-3, 0x90 );    // left side
   vgaBuf->draw_pixel(x1, y2-2, 0x90);

   //--------- while border on bottom and right sides -----------//

   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x2-2,y1+2,x2-1,y2-1, 0x9a );     // bottom side
   vgaBuf->draw_pixel(x2-1, y1+1, 0x9a);
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+2,y2-2,x2-3,y2-1, 0x9a );      // right side
   vgaBuf->draw_pixel(x1+1, y2-1, 0x9a);

   //--------- junction between white and black border --------//
   vgaBuf->draw_pixel(x2-1, y1, 0x97);
   vgaBuf->draw_pixel(x2-2, y1+1, 0x97);
   vgaBuf->draw_pixel(x1, y2-1, 0x97);
   vgaBuf->draw_pixel(x1+1, y2-2, 0x97);

   //--------- remove shadow, copy from back  -----------//
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x2, y1+1, x2, y2, 0x9c);
   IMGbar( vgaBuf->buf_ptr(), vgaBuf->buf_pitch(), x1+1, y2, x2-1, y2, 0x9c);

   mouse.show_area();

   //----- blt the area from the back buffer to the front buffer ------//

   if( !vgaFrontOnly && !use_back_buf )      // only blt the back to the front is the active buffer is the front
      vga.blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function Vga::d3_panel2_down ------------//


//------------- Start of function Vga::separator --------------//
//
// Draw a VGA separator line
//
// Syntax : separator( x1, y1, x2, y2 )
//
// int x1,y1       - the top left vertex of the separator
// int x2,y2       - the bottom right vertex of the separator
//
void Vga::separator(int x1, int y1, int x2, int y2)
{
   err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

   if( y1+1==y2 )       // horizontal line
   {
      vga_front.adjust_brightness(x1, y1, x2, y1, IF_UP_BRIGHTNESS_ADJUST);
      vga_front.adjust_brightness(x1, y2, x2, y2, IF_DOWN_BRIGHTNESS_ADJUST);
   }
   else
   {
      vga_front.adjust_brightness(x1, y1, x1, y2, IF_UP_BRIGHTNESS_ADJUST);
      vga_front.adjust_brightness(x2, y1, x2, y2, IF_DOWN_BRIGHTNESS_ADJUST);
   }
}
//--------------- End of function Vga::separator --------------//


//----------- Begin of function Vga::init_gray_remap_table ----------//
//
// Initialize a gray remap table for VgaBuf::convert_gray to use.
//
void Vga::init_gray_remap_table()
{
   //------ create a color to gray-scale remap table ------//

   #define FIRST_GRAY_COLOR   0x90
   #define GRAY_SCALE_COUNT   16    // no. of gray colors

// #define FIRST_GRAY_COLOR   0x96
// #define GRAY_SCALE_COUNT   10    // no. of gray colors

   PALETTEENTRY* palEntry = vga.pal_entry_buf;
   int i, grayIndex;

   for( i=0 ; i<256 ; i++, palEntry++ )
   {
      //--------------------------------------------------------//
      //
      // How to calculate the gray index (0-31)
      //
      // formula is : grey = red * 0.3 + green * 0.59 + blue * 0.11
      //              the range of the result value is 0-255
      //              this value is then divided by 8 to 0-31
      //
      //--------------------------------------------------------//

      grayIndex = ((int)palEntry->peRed * 30 + (int)palEntry->peGreen * 59 +
                   (int)palEntry->peBlue * 11) / 100 / (256/GRAY_SCALE_COUNT);

      gray_remap_table[i] = FIRST_GRAY_COLOR + grayIndex;
   }
}
//--------- End of function Vga::init_gray_remap_table -----------//

