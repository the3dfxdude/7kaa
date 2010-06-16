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

//-------- Begin of function VgaSDL::VgaSDL ----------//

VgaSDL::VgaSDL()
{
   front = NULL;
   memset(game_pal, 0, sizeof(SDL_Color)*VGA_PALETTE_SIZE);
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
   if (SDL_Init(SDL_INIT_VIDEO))
      return 0;

   front = SDL_SetVideoMode(VGA_WIDTH, VGA_HEIGHT, VGA_BPP, SDL_HWSURFACE|SDL_HWPALETTE);
   if (!front)
   {
      SDL_Quit();
      return 0;
   }

   SDL_WM_SetCaption(WIN_TITLE, WIN_TITLE);

   sys.active_flag = 1;

   return 1;
}
//-------- End of function VgaSDL::init ----------//


//-------- Begin of function VgaBuf::init_front ----------//
//
// Create a direct draw front buffer.
//
int VgaSDL::init_front(VgaBuf *b)
{
   b->init(new SurfaceSDL(front), 1);
   SDL_SetColors(front, game_pal, 0, VGA_PALETTE_SIZE);
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
int VgaSDL::init_back( VgaBuf *b, unsigned long w, unsigned long h )
{
   SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
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
   SDL_SetPalette(surface, SDL_LOGPAL, game_pal, 0, VGA_PALETTE_SIZE);
   b->init(wrapper, 0);
   return 1;
}
//-------- End of function VgaBuf::init_back ----------//


//-------- Begin of function VgaSDL::deinit ----------//

void VgaSDL::deinit()
{
   if (vga_color_table) delete vga_color_table;
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
// When the system changes the palette, this function will set
// the palette back to the correct entries.
//
void VgaSDL::refresh_palette()
{
   SDL_SetColors(front, game_pal, 0, VGA_PALETTE_SIZE);
}
//----------- End of function VgaSDL::refresh_palette ----------//


//-------- Begin of function VgaSDL::activate_pal ----------//
//
// we are getting the palette focus, select our palette
//
void VgaSDL::activate_pal(VgaBuf* vgaBufPtr)
{
   SurfaceSDL *surface = vgaBufPtr->get_buf();
   surface->activate_pal(game_pal, 0, VGA_PALETTE_SIZE);
}
//--------- End of function VgaSDL::activate_pal ----------//


//-------- Begin of function VgaSDL::set_custom_palette ----------//
//
// Read the custom palette specified by fileName and set to display.
//
int VgaSDL::set_custom_palette(char *fileName)
{
   return 1;
}
//--------- End of function VgaSDL::set_custom_palette ----------//


//--------- Begin of function VgaSDL::free_custom_palette ----------//
//
// Frees the custom palette and restores the game palette.
//
void VgaSDL::free_custom_palette()
{
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
}
//--------- End of function VgaSDL::adjust_brightness ----------//


//-------- Begin of function VgaSDL::handle_messages --------//
void VgaSDL::handle_messages()
{
   SDL_Event event;

   while (SDL_PollEvent(&event))
   {

      switch (event.type) {
      case SDL_ACTIVEEVENT:
         sys.active_flag = event.active.gain;
         if (sys.active_flag) 
         {
            sys.need_redraw_flag = 1;
            sys.unpause();
         } else {
            sys.pause();
         }
         break;
      case SDL_MOUSEMOTION:
      {
         mouse.cur_x = event.motion.x;
         mouse.cur_y = event.motion.y;
         sys.need_redraw_flag = 1;
         break;
      }
      case SDL_MOUSEBUTTONDOWN:
         if (event.button.button == SDL_BUTTON_LEFT)
         {
            mouse.left_press = (event.button.state == SDL_PRESSED);
         }
         else if (event.button.button == SDL_BUTTON_RIGHT)
         {
            mouse.right_press = (event.button.state == SDL_PRESSED);
         }
         break;
      case SDL_MOUSEBUTTONUP:
         if (event.button.button == SDL_BUTTON_LEFT)
         {
            mouse.left_press = !(event.button.state == SDL_RELEASED);
         } 
         else if (event.button.button == SDL_BUTTON_RIGHT)
         {
            mouse.right_press = !(event.button.state == SDL_RELEASED);
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

