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
#include <OVGA.h>
#include <OLOG.h>
// ##### begin Gilbert 16/9 #######//
#include <OVGALOCK.h>
// ##### end Gilbert 16/9 #######//
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Vga);

//------ Define static class member vars ---------//

char    VgaBase::use_back_buf = 0;
char    VgaBase::opaque_flag  = 0;
VgaBuf* VgaBase::active_buf   = &vga_front;      // default: front buffer

char    low_video_memory_flag = 0;

//-------- Begin of function VgaDDraw::Vga ----------//

VgaDDraw::VgaDDraw()
{
   memset( this, 0, sizeof(Vga) );  // FIXME

   vga_color_table = new ColorTable;

   // window related
   main_hwnd = NULL;
   app_hinstance = NULL;
}
//-------- End of function VgaDDraw::Vga ----------//


//-------- Begin of function VgaDDraw::~Vga ----------//

VgaDDraw::~VgaDDraw()
{
   deinit();      // 1-is final

   delete vga_color_table;
}
//-------- End of function VgaDDraw::~Vga ----------//


//-------- Begin of function VgaDDraw::init ----------//

int VgaDDraw::init()
{
   const char* warnStr = "Warning: Due to the low memory of your display card, "
                   "you may experience problems when you quit the game or "
                   "switch tasks during the game. "
                   "To avoid this problem, set your Windows display "
                   "to 800x600 256 color mode before running the game.";

   if( !create_window() )
      return 0;

   //--------- Initialize DirectDraw object --------//

   if( !init_dd() )
      return 0;

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
				MessageBox(main_hwnd, warnStr,
					WIN_TITLE, MB_OK | MB_ICONWARNING | MB_SETFOREGROUND );
			}

			low_video_memory_flag = 1;

         ShowCursor(FALSE);
      }
   }

   if( !set_mode() )
      return 0;

   return 1;
}
//-------- End of function VgaDDraw::init ----------//


//-------- Begin of function VgaDDraw::init_dd ----------//

int VgaDDraw::init_dd()
{
   if(dd_obj)        // the Direct Draw object has been initialized already
      return 1;

   //--------- Create direct draw object --------//

   DEBUG_LOG("Attempt DirectDrawCreate");
   LPDIRECTDRAW dd1Obj;
   int rc = DirectDrawCreate( NULL, &dd1Obj, NULL );
   DEBUG_LOG("DirectDrawCreate finish");

   if( rc != DD_OK )
   {
      ERR("DirectDrawCreate failed err=%d", rc);
      return 0;
   }

   //-------- Query DirectDraw2 interface --------//

   DEBUG_LOG("Attempt Query DirectDraw2");
   rc = dd1Obj->QueryInterface(IID_IDirectDraw2, (void **)&dd_obj);
   DEBUG_LOG("Query DirectDraw2 finish");
   if( rc != DD_OK )
   {
      ERR("Query DirectDraw2 failed err=%d", rc);
      dd1Obj->Release();
      return 0;
   }

   dd1Obj->Release();

   return 1;
}
//-------- End of function VgaDDraw::init_dd ----------//


//-------- Begin of function VgaBuf::init_front ----------//
//
// Create a direct draw front buffer.
//
int VgaDDraw::init_front(VgaBuf *b)
{
   DDSURFACEDESC       ddsd;
   HRESULT             rc;
   Surface             *surface;

   //---------------------------------------------//
   // Create the Front Buffer
   //---------------------------------------------//

   ZeroMemory( &ddsd, sizeof(ddsd) );
   ddsd.dwSize = sizeof( ddsd );

   ddsd.dwFlags = DDSD_CAPS;
   ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

   surface = create_surface( &ddsd );
   if (!surface)
   {
      err.run ( "Error creating Direct Draw front surface!!" );
      return 0;
   }

   b->init(surface, 1);
}
//-------- End of function VgaBuf::init_front ----------//


//-------- Begin of function VgaBuf::init_back ----------//
//
// Create a direct draw back buffer.
//
// [DWORD] w      : width of the surface [default 0 : VGA_WIDTH]
// [DWORD] h      : height of the surface [default 0 : VGA_HEIGHT]
//
int VgaDDraw::init_back( VgaBuf *b, DWORD w, DWORD h )
{
   DDSURFACEDESC       ddsd;
   Surface             *surface;

   //--------- fill in surface desc -----------//

   memset( &ddsd, 0, sizeof( ddsd ) );
   ddsd.dwSize = sizeof( ddsd );
   ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;

   ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

   ddsd.dwWidth  = w ? w : VGA_WIDTH;
   ddsd.dwHeight = h ? h : VGA_HEIGHT;

   surface = create_surface( &ddsd );
   if( !surface )
   {
      err.run( "Error creating direct draw back surface!!" );
      return 0;
   }

   b->init(surface, 0);
}
//-------- End of function VgaBuf::init_back ----------//


//-------- Begin of function VgaDDraw::set_mode ----------//

int VgaDDraw::set_mode()
{
   DWORD   dwStyle;
   HRESULT rc;

   //-----------------------------------------------------------//
   // Convert it to a plain window
   //-----------------------------------------------------------//

   dwStyle = GetWindowStyle(main_hwnd);
   dwStyle |= WS_POPUP;
   dwStyle &= ~(WS_OVERLAPPED | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX);
   SetWindowLong(main_hwnd, GWL_STYLE, dwStyle);

   //-----------------------------------------------------------//
   // grab exclusive mode if we are going to run as fullscreen
   // otherwise grab normal mode.
   //-----------------------------------------------------------//

   DEBUG_LOG("Attempt DirectDraw SetCooperativeLevel");
   rc = dd_obj->SetCooperativeLevel( main_hwnd,
                        DDSCL_EXCLUSIVE |
                        DDSCL_FULLSCREEN );
   DEBUG_LOG("DirectDraw SetCooperativeLevel finish");

   if( rc != DD_OK )
   {
      ERR("SetCooperativeLevel failed err=%d", rc);
      return 0;
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
      ERR("SetMode failed err=%d", rc);
      return 0;
   }

   //----------- display the system cursor -------------//

   SetCursor(NULL);

   return 1;
}
//-------- End of function VgaDDraw::set_mode ----------//


//-------- Begin of function VgaDDraw::deinit ----------//

void VgaDDraw::deinit()
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

   destroy_window();
}
//-------- End of function VgaDDraw::deinit ----------//


//--------- Start of function VgaDDraw::init_pal ----------//
//
// Loads the default game palette specified by fileName. Creates the ddraw
// palette.
//
int VgaDDraw::init_pal(const char* fileName)
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
         game_pal[i].peRed   = palBuf[i][0];
         game_pal[i].peGreen = palBuf[i][1];
         game_pal[i].peBlue  = palBuf[i][2];
      }

      HRESULT rc = dd_obj->CreatePalette( DDPCAPS_8BIT, game_pal, &dd_pal, NULL );

      if( rc != DD_OK )
         return 0;
   }

   init_color_table();
   init_gray_remap_table();

   return 1;
}
//----------- End of function VgaDDraw::init_pal ----------//

//--------- Start of function VgaDDraw::refresh_palette ----------//
//
// When the system changes the palette, this function will set
// the palette back to the correct entries.
//
void VgaDDraw::refresh_palette()
{
   // we can't restore if dd_pal is not initialized
   if (!dd_pal) return;

   // restore current palette
   if (custom_pal)
      dd_pal->SetEntries(0, 0, 256, custom_pal);
   else
      dd_pal->SetEntries(0, 0, 256, game_pal);
}
//----------- End of function VgaDDraw::refresh_palette ----------//

//--------- Start of function VgaDDraw::init_color_table ----------//

void VgaDDraw::init_color_table()
{
   //----- initialize interface color table -----//

   PalDesc palDesc( (unsigned char*) game_pal, sizeof(PALETTEENTRY), 256, 8);
   vga_color_table->generate_table( MAX_BRIGHTNESS_ADJUST_DEGREE, palDesc, ColorTable::bright_func );
}
//----------- End of function VgaDDraw::init_color_table ----------//


//--------- Start of function VgaDDraw::release_pal ----------//
//
// Releases the ddraw palette.
//
void VgaDDraw::release_pal()
{
   if (custom_pal)
   {
      mem_del(custom_pal);
      custom_pal = NULL;
   }

   // ##### begin Gilbert 16/9 #######//
   if( dd_pal )
   {
      while( dd_pal->Release() );
      dd_pal = NULL;
   }
   // ##### end Gilbert 16/9 #######//
}
//----------- End of function VgaDDraw::release_pal ----------//


//-------- Begin of function VgaDDraw::activate_pal ----------//
//
// we are getting the palette focus, select our palette
//
void VgaDDraw::activate_pal(VgaBuf* vgaBufPtr)
{
   Surface *s = vgaBufPtr->get_buf();
   s->activate_pal(dd_pal);
}
//--------- End of function VgaDDraw::activate_pal ----------//


//-------- Begin of function VgaDDraw::set_custom_palette ----------//
//
// Read the custom palette specified by fileName and set to display.
//
int VgaDDraw::set_custom_palette(char *fileName)
{
   if (!custom_pal)
      custom_pal = (LPPALETTEENTRY)mem_add(sizeof(PALETTEENTRY)*256);

   char palBuf[256][3];
   File palFile;

   palFile.file_open(fileName);
   palFile.file_seek(8);     				// bypass the header info
   palFile.file_read(palBuf, 256*3);
   palFile.file_close();

   for(int i=0; i<256; i++)
   {
      custom_pal[i].peRed   = palBuf[i][0];
      custom_pal[i].peGreen = palBuf[i][1];
      custom_pal[i].peBlue  = palBuf[i][2];
      custom_pal[i].peFlags = 0;
   }

   return !dd_pal->SetEntries(0, 0, 256, custom_pal);
}
//--------- End of function VgaDDraw::set_custom_palette ----------//


//--------- Begin of function VgaDDraw::free_custom_palette ----------//
//
// Frees the custom palette and restores the game palette.
//
void VgaDDraw::free_custom_palette()
{
   if (custom_pal)
   {
      mem_del(custom_pal);
      custom_pal = NULL; 
   }
   if (dd_pal)
      dd_pal->SetEntries(0, 0, 256, game_pal);
}
//--------- End of function VgaDDraw::free_custom_palette ----------//


//-------- Begin of function VgaDDraw::adjust_brightness ----------//
//
// <int> changeValue - the value to add to the RGB values of
//                     all the colors in the palette.
//                     the value can be from -255 to 255.
//
// <int> preserveContrast - whether preserve the constrast or not
//
void VgaDDraw::adjust_brightness(int changeValue)
{
   //---- find out the maximum rgb value can change without affecting the contrast ---//

   int          i;
   int          newRed, newGreen, newBlue;
   PALETTEENTRY palBuf[256];

   //------------ change palette now -------------//

   for( i=0 ; i<256 ; i++ )
   {
      newRed   = (int)game_pal[i].peRed   + changeValue;
      newGreen = (int)game_pal[i].peGreen + changeValue;
      newBlue  = (int)game_pal[i].peBlue  + changeValue;

      palBuf[i].peRed   = MIN(255, MAX(newRed,0) );
      palBuf[i].peGreen = MIN(255, MAX(newGreen,0) );
      palBuf[i].peBlue  = MIN(255, MAX(newBlue,0) );
   }

   //------------ set palette ------------//

   vga_front.temp_unlock();

   dd_pal->SetEntries( 0, 0, 256, palBuf );

   vga_front.temp_restore_lock();
}
//--------- End of function VgaDDraw::adjust_brightness ----------//


//----------- Begin of function VgaDDraw::init_gray_remap_table ----------//
//
// Initialize a gray remap table for VgaBuf::convert_gray to use.
//
void VgaDDraw::init_gray_remap_table()
{
   //------ create a color to gray-scale remap table ------//

   #define FIRST_GRAY_COLOR   0x90
   #define GRAY_SCALE_COUNT   16    // no. of gray colors

// #define FIRST_GRAY_COLOR   0x96
// #define GRAY_SCALE_COUNT   10    // no. of gray colors

   PALETTEENTRY* palEntry = game_pal;
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
//--------- End of function VgaDDraw::init_gray_remap_table -----------//


//----------- Begin of function VgaDDraw::create_surface ----------//
//
// Create a DirectDraw Surface
//
// On success, the pointer to the surface is returned. The caller is
// responsible for releasing the surface when done.
//
// On failure, the return is NULL.
//
Surface* VgaDDraw::create_surface(LPDDSURFACEDESC ddsd)
{
   LPDIRECTDRAWSURFACE2 dd_buf;
   LPDIRECTDRAWSURFACE dd1Buf;
   HRESULT rc;

   rc = dd_obj->CreateSurface( ddsd, &dd1Buf, NULL );
   if( rc != DD_OK )
      return NULL;

   rc = dd1Buf->QueryInterface(IID_IDirectDrawSurface2, (void **)&dd_buf);
   if( rc != DD_OK )
   {
      dd1Buf->Release();
      return NULL;
   }

   dd1Buf->Release();

   return new Surface(dd_buf);
}
//--------- End of function VgaDDraw::create_surface -----------//
