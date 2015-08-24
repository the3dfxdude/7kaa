/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2015 Jesse Allen
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
//Description : VGA manipulation functions (SDL version)

#include <OVGA.h>
#include <OMOUSE.h>
#include <OSYS.h>
#include <surface.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Vga);


//------ Define static class member vars ---------//

char    VgaBase::use_back_buf = 0;
char    VgaBase::opaque_flag  = 0;
VgaBuf* VgaBase::active_buf   = &vga_front;      // default: front buffer

namespace
{
   int window_pitch;
}  // namespace

//-------- Begin of function VgaSDL::VgaSDL ----------//

VgaSDL::VgaSDL()
{
   front = NULL;
   memset(game_pal, 0, sizeof(SDL_Color)*VGA_PALETTE_SIZE);
   custom_pal = NULL;
   vga_color_table = NULL;
}
//-------- End of function VgaSDL::VgaSDL ----------//


//-------- Begin of function VgaSDL::~VgaSDL ----------//

VgaSDL::~VgaSDL()
{
   deinit();
}
//-------- End of function VgaSDL::~VgaSDL ----------//


//-------- Begin of function VgaSDL::init ----------//

int VgaSDL::init()
{
   SDL_Surface *icon;

   if (SDL_Init(SDL_INIT_VIDEO))
      return 0;

   SDL_DisplayMode mode;
   int window_width = 1024;
   int window_height = 768;

   if (SDL_GetDesktopDisplayMode(0, &mode) == 0)
   {
      if (mode.h < 1024)
      {
         window_width = 800;
         window_height = 600;
      }
   }
   else
   {
      ERR("Could not get desktop display mode: %s\n", SDL_GetError());
      SDL_Quit();
      return 0;
   }

   if (SDL_CreateWindowAndRenderer(window_width,
                                   window_height,
                                   0,
                                   &window,
                                   &renderer) < 0)
   {
      ERR("Could not create window and renderer: %s\n", SDL_GetError());
      SDL_Quit();
      return 0;
   }

   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
   SDL_RenderSetLogicalSize(renderer, VGA_WIDTH, VGA_HEIGHT);

   SDL_RendererInfo info;
   if (SDL_GetRendererInfo(renderer, &info) == 0)
   {
      MSG("Name of renderer: %s\n", info.name);
      MSG("Using software fallback: %s\n", info.flags & SDL_RENDERER_SOFTWARE ? "yes" : "no");
      MSG("Using hardware acceleration: %s\n", info.flags & SDL_RENDERER_ACCELERATED ? "yes" : "no");
      MSG("V-sync: %s\n", info.flags & SDL_RENDERER_PRESENTVSYNC ? "on" : "off");
      MSG("Rendering to texture support: %s\n", info.flags & SDL_RENDERER_TARGETTEXTURE ? "yes" : "no");
      MSG("Maximum texture width: %d\n", info.max_texture_width);
      MSG("Maximum texture height: %d\n", info.max_texture_height);
   }

   Uint32 window_pixel_format = SDL_GetWindowPixelFormat(window);
   if (window_pixel_format == SDL_PIXELFORMAT_UNKNOWN)
   {
      ERR("Unknown pixel format: %s\n", SDL_GetError());
      SDL_Quit();
      return 0;
   }
   MSG("Pixel format: %s\n", SDL_GetPixelFormatName(window_pixel_format));

   window_pitch = VGA_WIDTH * SDL_BYTESPERPIXEL(window_pixel_format);

   // Cannot use SDL_PIXELFORMAT_INDEX8:
   //   Palettized textures are not supported
   texture = SDL_CreateTexture(renderer,
                               window_pixel_format,
                               SDL_TEXTUREACCESS_STREAMING,
                               VGA_WIDTH,
                               VGA_HEIGHT);
   if (!texture)
   {
      ERR("Could not create texture: %s\n", SDL_GetError());
      SDL_Quit();
      return 0;
   }

   front = SDL_CreateRGBSurface(0,
                                VGA_WIDTH,
                                VGA_HEIGHT,
                                VGA_BPP,
                                0, 0, 0, 0);
   if (!front)
   {
      SDL_Quit();
      return 0;
   }

   int desktop_bpp = 0;
   if (SDL_PIXELTYPE(window_pixel_format) == SDL_PIXELTYPE_PACKED32)
   {
      desktop_bpp = 32;
   }
   else if (SDL_PIXELTYPE(window_pixel_format) == SDL_PIXELTYPE_PACKED16)
   {
      desktop_bpp = 16;
   }
   else if (SDL_PIXELTYPE(window_pixel_format) == SDL_PIXELTYPE_PACKED8)
   {
      desktop_bpp = 8;
   }
   else
   {
      ERR("Unsupported pixel type\n");
      SDL_Quit();
      return 0;
   }

   target = SDL_CreateRGBSurface(0,
                                 VGA_WIDTH,
                                 VGA_HEIGHT,
                                 desktop_bpp,
                                 0, 0, 0, 0);
   if (!target)
   {
      SDL_Quit();
      return 0;
   }

   icon = SDL_LoadBMP(DEFAULT_DIR_IMAGE "7k_icon.bmp");
   if (icon)
   {
      Uint32 colorkey;
      colorkey = SDL_MapRGB(icon->format, 0, 0, 0);
      SDL_SetColorKey(icon, SDL_TRUE, colorkey);
      SDL_SetWindowIcon(window, icon);
   }
   SDL_SetWindowTitle(window, WIN_TITLE);

   init_pal(DIR_RES"PAL_STD.RES");

   // Create the front and back buffers
   init_back(&vga_front);
   vga_front.is_front = 1; // set it to 1, overriding the setting in init_back()
   if (sys.debug_session) {
      init_back(&vga_true_front);
   }
   init_back(&vga_back);

   vga_front.lock_buf();
   vga_back.lock_buf();

   refresh_palette();

   return 1;
}
//-------- End of function VgaSDL::init ----------//


//-------- Begin of function VgaBuf::init_front ----------//
//
// Inform the front buffer of the actual surface.  This function retains
// compatibility with old direct draw code.
//
int VgaSDL::init_front(VgaBuf *b)
{
   b->init(new SurfaceSDL(front), 1);
   refresh_palette();
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
int VgaSDL::init_back(VgaBuf *b, unsigned long w, unsigned long h)
{
   SDL_Surface *surface = SDL_CreateRGBSurface(0,
                                               VGA_WIDTH,
                                               VGA_HEIGHT,
                                               VGA_BPP,
                                               0, 0, 0, 0);
   if (!surface)
   {
      ERR("Surface not created!\n");
      return 0;
   }

   SurfaceSDL *wrapper = new SurfaceSDL(surface);
   b->init(wrapper, 0);
   return 1;
}
//-------- End of function VgaBuf::init_back ----------//


//-------- Begin of function VgaSDL::deinit ----------//

void VgaSDL::deinit()
{
   SDL_SetRelativeMouseMode(SDL_FALSE);

   vga_back.deinit();
   if (sys.debug_session)
      vga_true_front.deinit();
   vga_front.deinit();

   if (vga_color_table) delete vga_color_table;
   SDL_FreeSurface(target);
   target = NULL;
   SDL_FreeSurface(front);
   front = NULL;
   SDL_DestroyTexture(texture);
   texture = NULL;
   window_pitch = 0;
   SDL_DestroyRenderer(renderer);
   renderer = NULL;
   SDL_DestroyWindow(window);
   window = NULL;
   SDL_Quit();
}
//-------- End of function VgaSDL::deinit ----------//


//--------- Start of function VgaSDL::init_pal ----------//
//
// Loads the default game palette specified by fileName. Creates the ddraw
// palette.
//
int VgaSDL::init_pal(const char* fileName)
{
   char palBuf[VGA_PALETTE_SIZE][3];
   File palFile;

   palFile.file_open(fileName);
   palFile.file_seek(8);               // bypass the header info
   palFile.file_read(palBuf, VGA_PALETTE_SIZE*3);
   palFile.file_close();

   for (int i = 0; i < VGA_PALETTE_SIZE; i++)
   {
      game_pal[i].r = palBuf[i][0];
      game_pal[i].g = palBuf[i][1];
      game_pal[i].b = palBuf[i][2];
   }

   //----- initialize interface color table -----//

   PalDesc palDesc( (unsigned char*) game_pal, sizeof(SDL_Color), VGA_PALETTE_SIZE, 8);
   vga_color_table = new ColorTable;
   vga_color_table->generate_table( MAX_BRIGHTNESS_ADJUST_DEGREE, palDesc, ColorTable::bright_func );

   return 1;
}
//----------- End of function VgaSDL::init_pal ----------//

//--------- Start of function VgaSDL::refresh_palette ----------//
//
// Update front buffers with the current palette.
//
void VgaSDL::refresh_palette()
{
   SurfaceSDL *fake_front = vga_front.get_buf();
   if (custom_pal) {
      fake_front->activate_pal(custom_pal, 0, VGA_PALETTE_SIZE);
      SDL_SetPaletteColors(front->format->palette,
                           custom_pal,
                           0,
                           VGA_PALETTE_SIZE);
   } else {
      fake_front->activate_pal(game_pal, 0, VGA_PALETTE_SIZE);
      SDL_SetPaletteColors(front->format->palette,
                           game_pal,
                           0,
                           VGA_PALETTE_SIZE);
   }
}
//----------- End of function VgaSDL::refresh_palette ----------//


//-------- Begin of function VgaSDL::activate_pal ----------//
//
// we are getting the palette focus, select our palette
//
void VgaSDL::activate_pal(VgaBuf* vgaBufPtr)
{
}
//--------- End of function VgaSDL::activate_pal ----------//


//-------- Begin of function VgaSDL::set_custom_palette ----------//
//
// Read the custom palette specified by fileName and set to display.
//
int VgaSDL::set_custom_palette(char *fileName)
{
   if (!custom_pal)
      custom_pal = (SDL_Color*)mem_add(sizeof(SDL_Color)*VGA_PALETTE_SIZE);

   char palBuf[VGA_PALETTE_SIZE][3];
   File palFile;

   palFile.file_open(fileName);
   palFile.file_seek(8);     				// bypass the header info
   palFile.file_read(palBuf, VGA_PALETTE_SIZE*3);
   palFile.file_close();

   for(int i=0; i<VGA_PALETTE_SIZE; i++)
   {
      custom_pal[i].r = palBuf[i][0];
      custom_pal[i].g = palBuf[i][1];
      custom_pal[i].b = palBuf[i][2];
   }

   refresh_palette();

   return 1;
}
//--------- End of function VgaSDL::set_custom_palette ----------//


//--------- Begin of function VgaSDL::free_custom_palette ----------//
//
// Frees the custom palette and restores the game palette.
//
void VgaSDL::free_custom_palette()
{
   if (custom_pal)
   {
      mem_del(custom_pal);
      custom_pal = NULL; 
   }
   refresh_palette();
}
//--------- End of function VgaSDL::free_custom_palette ----------//


//-------- Begin of function VgaSDL::adjust_brightness ----------//
//
// <int> changeValue - the value to add to the RGB values of
//                     all the colors in the palette.
//                     the value can be from -255 to 255.
//
// <int> preserveContrast - whether preserve the constrast or not
//
void VgaSDL::adjust_brightness(int changeValue)
{
   //---- find out the maximum rgb value can change without affecting the contrast ---//

   int          i;
   int          newRed, newGreen, newBlue;
   SDL_Color palBuf[VGA_PALETTE_SIZE];

   //------------ change palette now -------------//

   for( i=0 ; i<VGA_PALETTE_SIZE ; i++ )
   {
      newRed   = (int)game_pal[i].r + changeValue;
      newGreen = (int)game_pal[i].g + changeValue;
      newBlue  = (int)game_pal[i].b + changeValue;

      palBuf[i].r = MIN(255, MAX(newRed,0));
      palBuf[i].g = MIN(255, MAX(newGreen,0));
      palBuf[i].b = MIN(255, MAX(newBlue,0));
   }

   //------------ set palette ------------//

   vga_front.temp_unlock();

   SDL_SetPaletteColors(front->format->palette,
                        palBuf,
                        0,
                        VGA_PALETTE_SIZE);

   vga_front.temp_restore_lock();
}
//--------- End of function VgaSDL::adjust_brightness ----------//


//-------- Begin of function VgaSDL::handle_messages --------//
void VgaSDL::handle_messages()
{
   SDL_Event event;

   SDL_PumpEvents();

   while (SDL_PeepEvents(&event,
                         1,
                         SDL_GETEVENT,
                         SDL_QUIT,
                         SDL_SYSWMEVENT)) {

      switch (event.type)
	  {
	  case SDL_WINDOWEVENT:
		  switch (event.window.event)
		  {
		  //case SDL_WINDOWEVENT_ENTER: // Do not respond to mouse focus
		  case SDL_WINDOWEVENT_FOCUS_GAINED:
		  case SDL_WINDOWEVENT_RESTORED:
			 sys.need_redraw_flag = 1;
			 if (!sys.is_mp_game)
				sys.unpause();

			 // update ctrl/shift/alt key state
			 mouse.update_skey_state();
			 SDL_ShowCursor(SDL_DISABLE);
			 break;

		  //case SDL_WINDOWEVENT_LEAVE: // Do not respond to mouse focus
		  case SDL_WINDOWEVENT_FOCUS_LOST:
		  case SDL_WINDOWEVENT_MINIMIZED:
			 if (!sys.is_mp_game)
				sys.pause();
			 // turn the system cursor back on to get around a fullscreen
			 // mouse grabbed problem on windows
			 SDL_ShowCursor(SDL_ENABLE);
			 break;
			 
		  case SDL_WINDOWEVENT_EXPOSED:
			  sys.need_redraw_flag = 1;
			  break;
		  }
		  break;

	  case SDL_QUIT:
         sys.signal_exit_flag = 1;
         break;

	  default:
         ERR("unhandled event %d\n", event.type);
         break;
      }
   }
}
//-------- End of function VgaSDL::handle_messages --------//

//-------- Begin of function VgaSDL::flag_redraw --------//
void VgaSDL::flag_redraw()
{
}
//-------- End of function VgaSDL::flag_redraw ----------//


//-------- Begin of function VgaSDL::is_full_screen --------//
//
int VgaSDL::is_full_screen()
{
   return ((SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0);
}
//-------- End of function VgaSDL::is_full_screen ----------//


//-------- Begin of function VgaSDL::is_input_grabbed --------//
//
int VgaSDL::is_input_grabbed()
{
   return ((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_GRABBED) != 0);
}
//-------- End of function VgaSDL::is_input_grabbed ----------//


//-------- Begin of function VgaSDL::set_full_screen_mode --------//
//
// mode -1: toggle
// mode  0: windowed
// mode  1: full screen without display mode change (stretched to desktop)
void VgaSDL::set_full_screen_mode(int mode)
{
   int result = 0;
   uint32_t flags = 0;

   switch (mode)
   {
      case -1:
         flags = is_full_screen() ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
         break;
      case 0:
         break;
      case 1:
         flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
         break;
      default:
         err_now("invalid mode");
   }

   result = SDL_SetWindowFullscreen(window, flags);
   if (result < 0) {
      ERR("Could not toggle fullscreen: %s\n", SDL_GetError());
      return;
   }

   refresh_palette();
   sys.need_redraw_flag = 1;
   set_window_grab(flags == SDL_WINDOW_FULLSCREEN_DESKTOP);
}
//-------- End of function VgaSDL::set_full_screen_mode ----------//


//-------- Begin of function VgaSDL::set_window_grab --------//
//
// mode -1: toggle
// mode  0: unset grab
// mode  1: set grab
void VgaSDL::set_window_grab(int mode)
{
   SDL_bool grabbed = SDL_FALSE;

   switch (mode)
   {
      case -1:
         grabbed = is_input_grabbed() ? SDL_FALSE : SDL_TRUE;
         break;
      case 0:
         break;
      case 1:
         grabbed = SDL_TRUE;
         break;
      default:
         err_now("invalid mode");
   }
   SDL_SetWindowGrab(window, grabbed);
   SDL_SetRelativeMouseMode(grabbed);
}
//-------- End of function VgaSDL::set_window_grab ----------//


//-------- Beginning of function VgaSDL::flip ----------//
void VgaSDL::flip()
{
   static Uint32 ticks = 0;
   Uint32 cur_ticks = SDL_GetTicks();
   if (cur_ticks > ticks + 17 || cur_ticks < ticks) {
      SurfaceSDL *tmp = vga_front.get_buf();
      SDL_Surface *src = tmp->get_surface();
      ticks = cur_ticks;
      SDL_BlitSurface(src, NULL, target, NULL);
      SDL_UpdateTexture(texture, NULL, target->pixels, window_pitch);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
   }
}
//-------- End of function VgaSDL::flip ----------//
