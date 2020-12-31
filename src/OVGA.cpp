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

//Filename    : OVGA.cpp
//Description : VGA management class (SDL version)

#include <OVGA.h>
#include <OMOUSE.h>
#include <OCOLTBL.h>
#include <OSYS.h>
#include <dbglog.h>
#include <version.h>
#include <FilePath.h>
#include <ConfigAdv.h>

DBGLOG_DEFAULT_CHANNEL(Vga);

//--------- Declare static functions ---------//

static void init_dpi();
static int init_window_flags();
static void init_window_size();

//------ Define static class member vars ---------//

char    Vga::use_back_buf = 0;
char    Vga::opaque_flag  = 0;
VgaBuf* Vga::active_buf   = &vga_front;      // default: front buffer

//-------- Begin of function Vga::Vga ----------//

Vga::Vga()
{
   memset(game_pal, 0, sizeof(SDL_Color)*VGA_PALETTE_SIZE);
   custom_pal = NULL;
   vga_color_table = NULL;

   target = NULL;
   texture = NULL;
   renderer = NULL;
   window = NULL;
}
//-------- End of function Vga::Vga ----------//


//-------- Begin of function Vga::~Vga ----------//

Vga::~Vga()
{
   deinit();
}
//-------- End of function Vga::~Vga ----------//


//-------- Begin of function Vga::init ----------//

int Vga::init()
{
   SDL_Surface *icon;

#ifdef USE_WINDOWS
   init_dpi();
#endif

   win_grab_forced = 0;
   win_grab_user_mode = 0;
   mouse_mode = MOUSE_INPUT_ABS;
   boundary_set = 0;

   if (SDL_Init(SDL_INIT_VIDEO))
      return 0;

   init_window_size();

   // Save the mouse position to restore after mode change. If we don't do
   // this, then the old position gets recalculated, with the mode change
   // affecting the location, causing a jump.
   int mouse_x, mouse_y;
   SDL_GetGlobalMouseState(&mouse_x, &mouse_y);

   window = SDL_CreateWindow(WIN_TITLE,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             config_adv.vga_window_width,
                             config_adv.vga_window_height,
                             init_window_flags());
   if( !window )
      return 0;

   if( config_adv.vga_full_screen )
      set_window_grab(WINGRAB_ON);

   renderer = SDL_CreateRenderer(window, -1, 0);
   if( !renderer )
      return 0;

   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
   SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
   if( config_adv.vga_keep_aspect_ratio )
      SDL_RenderSetLogicalSize(renderer, VGA_WIDTH, VGA_HEIGHT);

   SDL_WarpMouseGlobal(mouse_x, mouse_y); // warp to initialize mouse by event queue

   Uint32 window_pixel_format = SDL_GetWindowPixelFormat(window);
   if (window_pixel_format == SDL_PIXELFORMAT_UNKNOWN)
   {
      ERR("Unknown pixel format: %s\n", SDL_GetError());
      return 0;
   }

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
      return 0;
   }

   target = SDL_CreateRGBSurface(0,
                                 VGA_WIDTH,
                                 VGA_HEIGHT,
                                 desktop_bpp,
                                 0, 0, 0, 0);
   if (!target)
   {
      return 0;
   }

   FilePath icon_path(sys.dir_image);
   icon_path += "7K_ICON.BMP";
   icon = SDL_LoadBMP(icon_path);
   if (icon)
   {
      Uint32 colorkey;
      colorkey = SDL_MapRGB(icon->format, 0, 0, 0);
      SDL_SetColorKey(icon, SDL_TRUE, colorkey);
      SDL_SetWindowIcon(window, icon);
      SDL_FreeSurface(icon);
   }

   return 1;
}
//-------- End of function Vga::init ----------//


//-------- Begin of function Vga::deinit ----------//

void Vga::deinit()
{
   SDL_SetRelativeMouseMode(SDL_FALSE);
   mouse_mode = MOUSE_INPUT_ABS;

   vga_back.deinit();
   if (sys.debug_session)
      vga_true_front.deinit();
   vga_front.deinit();

   if (vga_color_table)
      delete vga_color_table;
   vga_color_table = NULL;
   if( custom_pal )
      mem_del(custom_pal);
   custom_pal = NULL;

   if( target )
      SDL_FreeSurface(target);
   target = NULL;
   if( texture )
      SDL_DestroyTexture(texture);
   texture = NULL;
   if( renderer )
      SDL_DestroyRenderer(renderer);
   renderer = NULL;
   if( window )
      SDL_DestroyWindow(window);
   window = NULL;

   SDL_Quit();
}
//-------- End of function Vga::deinit ----------//


//--------- Start of function Vga::load_pal ----------//
//
// Loads the default game palette specified by fileName. Creates the ddraw
// palette.
//
int Vga::load_pal(const char* fileName)
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
//----------- End of function Vga::load_pal ----------//


//-------- Begin of function Vga::activate_pal ----------//
//
// we are getting the palette focus, select our palette
//
void Vga::activate_pal(VgaBuf* vgaBufPtr)
{
   vgaBufPtr->activate_pal(game_pal);
}
//--------- End of function Vga::activate_pal ----------//


//-------- Begin of function Vga::set_custom_palette ----------//
//
// Read the custom palette specified by fileName and set to display.
//
int Vga::set_custom_palette(char *fileName)
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

   vga_front.activate_pal(custom_pal);

   return 1;
}
//--------- End of function Vga::set_custom_palette ----------//


//--------- Begin of function Vga::free_custom_palette ----------//
//
// Frees the custom palette and restores the game palette.
//
void Vga::free_custom_palette()
{
   if (custom_pal)
   {
      mem_del(custom_pal);
      custom_pal = NULL; 
   }
   vga_front.activate_pal(game_pal);
}
//--------- End of function Vga::free_custom_palette ----------//


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

   vga_front.activate_pal(palBuf);

   vga_front.temp_restore_lock();
}
//--------- End of function Vga::adjust_brightness ----------//


//-------- Begin of function Vga::handle_messages --------//
void Vga::handle_messages()
{
   SDL_Event event;

   while( SDL_PollEvent(&event) )
   {
      switch (event.type)
      {
      case SDL_QUIT:
         sys.signal_exit_flag = 1;
         break;
      case SDL_MULTIGESTURE:
         if (event.mgesture.numFingers == 2) {
            mouse.process_scroll(event.mgesture.x, event.mgesture.y);
         }
         break;
      case SDL_FINGERDOWN:
         mouse.end_scroll();
         break;
      case SDL_MOUSEWHEEL:
          mouse.process_scroll(event.wheel.x, event.wheel.y * -1);
          break;
      case SDL_WINDOWEVENT:
         switch (event.window.event)
         {
            case SDL_WINDOWEVENT_EXPOSED:
            case SDL_WINDOWEVENT_RESIZED:
               sys.need_redraw_flag = 1;
               update_mouse_pos();
               boundary_set = 0;
               break;

            //case SDL_WINDOWEVENT_ENTER: // Do not respond to mouse focus
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_RESTORED:
               sys.need_redraw_flag = 1;
               if( !sys.is_mp_game && config_adv.vga_pause_on_focus_loss )
                  sys.unpause();

               // update ctrl/shift/alt key state
               mouse.update_skey_state();
               SDL_ShowCursor(SDL_DISABLE);
               break;

            //case SDL_WINDOWEVENT_LEAVE: // Do not respond to mouse focus
            case SDL_WINDOWEVENT_FOCUS_LOST:
            case SDL_WINDOWEVENT_MINIMIZED:
               if( !sys.is_mp_game && config_adv.vga_pause_on_focus_loss )
                  sys.pause();
               // turn the system cursor back on to get around a fullscreen
               // mouse grabbed problem on windows
               SDL_ShowCursor(SDL_ENABLE);
               break;
         }
         break;

      case SDL_MOUSEMOTION:
         if( mouse_mode == MOUSE_INPUT_ABS )
         {
            int logical_x, logical_y;
            if( config_adv.vga_keep_aspect_ratio )
            {
               logical_x = event.motion.x;
               logical_y = event.motion.y;
            }
            else
            {
               float xscale, yscale;
               get_window_scale(&xscale, &yscale);
               logical_x = ((float)event.motion.x / xscale);
               logical_y = ((float)event.motion.y / yscale);
            }
            if( win_grab_user_mode || win_grab_forced )
            {
               int real_x, real_y, do_warp;
               SDL_GetMouseState(&real_x, &real_y);
               do_warp = 0;
               if( !boundary_set )
                  update_boundary();
               if( real_x < bound_x1 )
               {
                  do_warp = 1;
                  real_x = bound_x1;
                  logical_x = mouse.bound_x1;
               }
               else if( real_x >= bound_x2 )
               {
                  do_warp = 1;
                  real_x = bound_x2;
                  logical_x = mouse.bound_x2;
               }
               if( real_y < bound_y1 )
               {
                  do_warp = 1;
                  real_y = bound_y1;
                  logical_y = mouse.bound_y1;
               }
               else if( real_y >= bound_y2 )
               {
                  do_warp = 1;
                  real_y = bound_y2;
                  logical_y = mouse.bound_y2;
               }
               if( do_warp )
               {
                  SDL_WarpMouseInWindow(window, real_x, real_y);
               }
            }
            mouse.process_mouse_motion(logical_x, logical_y);
         }
         else
         {
            mouse.process_mouse_motion(event.motion.xrel, event.motion.yrel);
         }
         break;
      case SDL_MOUSEBUTTONDOWN:
         if( event.button.button == SDL_BUTTON_LEFT )
         {
            mouse.add_event(LEFT_BUTTON);
         }
         else if( event.button.button == SDL_BUTTON_RIGHT )
         {
            mouse.add_event(RIGHT_BUTTON);
         }
         set_window_grab(WINGRAB_FORCE);
         break;
      case SDL_MOUSEBUTTONUP:
         if( event.button.button == SDL_BUTTON_LEFT )
         {
            mouse.add_event(LEFT_BUTTON_RELEASE);
            mouse.reset_boundary();
         }
         else if( event.button.button == SDL_BUTTON_RIGHT )
         {
            mouse.add_event(RIGHT_BUTTON_RELEASE);
         }
         set_window_grab(WINGRAB_RESTORE);
         break;
      case SDL_KEYDOWN:
      {
         int bypass = 0;
         int mod = event.key.keysym.mod &
            (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT);
         if( mod == KMOD_LALT || mod == KMOD_RALT )
         {
            if( event.key.keysym.sym == SDLK_RETURN )
            {
               bypass = 1;
               sys.toggle_full_screen_flag = 1;
            }
            else if( event.key.keysym.sym == SDLK_F4 )
            {
               bypass = 1;
               sys.signal_exit_flag = 1;
            }
            else if( event.key.keysym.sym == SDLK_TAB )
            {
               bypass = 1;
               SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
               SDL_MinimizeWindow(window);
            }
         }
         else if( mod == KMOD_LCTRL || mod == KMOD_RCTRL )
         {
            if( event.key.keysym.sym == SDLK_g )
            {
               bypass = 1;
               set_window_grab(WINGRAB_TOGGLE);
            }
            else if( event.key.keysym.sym == SDLK_m )
            {
               bypass = 1;
               if( mouse_mode == MOUSE_INPUT_ABS && !is_input_grabbed() )
                  set_mouse_mode( MOUSE_INPUT_REL_WARP );
               else if( mouse_mode != MOUSE_INPUT_ABS )
                  set_mouse_mode( MOUSE_INPUT_ABS );
            }
         }
         if( SDL_IsTextInputActive() && event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_z )
		bypass = 1;
         if( !bypass )
         {
            mouse.update_skey_state();
            mouse.add_key_event(event.key.keysym.sym, misc.get_time());
         }
         break;
      }
      case SDL_KEYUP:
         mouse.update_skey_state();
         break;
      case SDL_TEXTINPUT:
         mouse.add_typing_event(event.text.text, misc.get_time());
         break;
      case SDL_TEXTEDITING:
      case SDL_JOYAXISMOTION:
      case SDL_JOYBALLMOTION:
      case SDL_JOYHATMOTION:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
      default:
         MSG("unhandled event %d\n", event.type);
         break;
      }
   }
}
//-------- End of function Vga::handle_messages --------//

//-------- Begin of function Vga::flag_redraw --------//
void Vga::flag_redraw()
{
}
//-------- End of function Vga::flag_redraw ----------//


//-------- Begin of function Vga::is_full_screen --------//
//
int Vga::is_full_screen()
{
   return ((SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0);
}
//-------- End of function Vga::is_full_screen ----------//


//-------- Begin of function Vga::is_input_grabbed --------//
//
int Vga::is_input_grabbed()
{
   return ((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_GRABBED) != 0);
}
//-------- End of function Vga::is_input_grabbed ----------//


//-------- Begin of function Vga::update_boundary --------//
// Uses logical boundary coordinates and scales them to the actual boundary in
// the scaled window.
void Vga::update_boundary()
{
   float xscale, yscale;
   SDL_Rect rect;
   get_window_scale(&xscale, &yscale);
   SDL_RenderGetViewport(renderer, &rect);
   bound_x1 = ((float)(mouse.bound_x1 + rect.x) * xscale);
   bound_x2 = ((float)(mouse.bound_x2 + rect.x) * xscale);
   bound_y1 = ((float)(mouse.bound_y1 + rect.y) * yscale);
   bound_y2 = ((float)(mouse.bound_y2 + rect.y) * yscale);
   boundary_set = 1;
}
//-------- End of function Vga::update_boundary --------//


//-------- Begin of function Vga::update_mouse_pos ----------//
// Updates logical mouse position according to actual mouse state.
void Vga::update_mouse_pos()
{
   float xscale, yscale;
   SDL_Rect rect;
   int logical_x, logical_y, win_x, win_y, mouse_x, mouse_y;

   if( !window )
      return;

   get_window_scale(&xscale, &yscale);
   SDL_RenderGetViewport(renderer, &rect);
   SDL_GetWindowPosition(window, &win_x, &win_y);
   SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
   logical_x = ((float)(mouse_x - win_x) / xscale - rect.x);
   logical_y = ((float)(mouse_y - win_y) / yscale - rect.y);

   mouse.process_mouse_motion(logical_x, logical_y);
}
//---------- End of function Vga::update_mouse_pos ----------//


//-------- Begin of function Vga::set_full_screen_mode --------//
//
// mode -1: toggle
// mode  0: windowed
// mode  1: full screen without display mode change (stretched to desktop)
void Vga::set_full_screen_mode(int mode)
{
   int result, mouse_x, mouse_y;
   uint32_t flags = config_adv.vga_full_screen_desktop ?
      SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;

   switch (mode)
   {
      case -1:
         if( is_full_screen() )
            flags = 0;
         break;
      case 0:
         flags = 0;
         break;
      case 1:
         break;
      default:
         err_now("invalid mode");
   }

   // Save the mouse position to restore after mode change. If we don't do
   // this, then the old position gets recalculated, with the mode change
   // affecting the location, causing a jump.
   SDL_GetGlobalMouseState(&mouse_x, &mouse_y);

   result = SDL_SetWindowFullscreen(window, flags);
   if (result < 0) {
      ERR("Could not toggle fullscreen: %s\n", SDL_GetError());
      return;
   }

   sys.need_redraw_flag = 1;
   boundary_set = 0;
   if( flags ) // went full screen
      set_window_grab(WINGRAB_ON);
   else
      set_window_grab(WINGRAB_OFF);

   SDL_WarpMouseGlobal(mouse_x, mouse_y);
}
//-------- End of function Vga::set_full_screen_mode ----------//


//-------- Begin of function Vga::set_mouse_mode --------//
void Vga::set_mouse_mode(MouseInputMode mode)
{
   switch( mode )
   {
   case MOUSE_INPUT_REL:
      if( mouse_mode == MOUSE_INPUT_ABS )
         SDL_SetRelativeMouseMode(SDL_TRUE);
      SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");
      mouse_mode = MOUSE_INPUT_REL;
      break;
   case MOUSE_INPUT_REL_WARP:
      if( mouse_mode == MOUSE_INPUT_ABS )
         SDL_SetRelativeMouseMode(SDL_TRUE);
      SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
      mouse_mode = MOUSE_INPUT_REL_WARP;
      break;
   default:
      // absolute mode
      if( mouse_mode != MOUSE_INPUT_ABS )
         SDL_SetRelativeMouseMode(SDL_FALSE);
      mouse_mode = MOUSE_INPUT_ABS;
   }
}
//-------- End of function Vga::set_mouse_mode --------//


//-------- Begin of function Vga::set_window_grab --------//
//
// WINGRAB_OFF = Turn window grab off, except when forced on
// WINGRAB_ON = Turn window grab on
// WINGRAB_TOGGLE = Toggle window grab, used for the grab key
// WINGRAB_FORCE = Force grab on, even if user has it off
// WINGRAB_RESTORE = Disable forcing, and user's option
void Vga::set_window_grab(WinGrab mode)
{
   switch( mode )
   {
   case WINGRAB_OFF:
      if( win_grab_user_mode )
      {
         win_grab_user_mode = 0;
         if( !win_grab_forced )
         {
            SDL_SetWindowGrab(window, SDL_FALSE);
            if( mouse_mode != MOUSE_INPUT_ABS )
               set_mouse_mode(MOUSE_INPUT_ABS);
         }
      }
      break;
   case WINGRAB_ON:
      if( !win_grab_user_mode )
      {
         win_grab_user_mode = 1;
         if( !win_grab_forced )
         {
            SDL_SetWindowGrab(window, SDL_TRUE);
         }
      }
      break;
   case WINGRAB_TOGGLE:
      if( win_grab_user_mode )
      {
         win_grab_user_mode = 0;
         if( !win_grab_forced )
         {
            SDL_SetWindowGrab(window, SDL_FALSE);
            if( mouse_mode != MOUSE_INPUT_ABS )
               set_mouse_mode(MOUSE_INPUT_ABS);
         }
      }
      else
      {
         win_grab_user_mode = 1;
         if( !win_grab_forced )
         {
            SDL_SetWindowGrab(window, SDL_TRUE);
         }
      }
      break;
   case WINGRAB_FORCE:
      if( !win_grab_forced )
      {
         win_grab_forced = 1;
         if( !win_grab_user_mode )
         {
            SDL_SetWindowGrab(window, SDL_TRUE);
         }
      }
      break;
   case WINGRAB_RESTORE:
      if( win_grab_forced )
      {
         win_grab_forced = 0;
         if( !win_grab_user_mode )
         {
            SDL_SetWindowGrab(window, SDL_FALSE);
            if( mouse_mode != MOUSE_INPUT_ABS )
               set_mouse_mode(MOUSE_INPUT_ABS);
         }
      }
      break;
   default:
      err_now("invalid mode");
   }
}
//-------- End of function Vga::set_window_grab ----------//


//-------- Beginning of function Vga::flip ----------//
void Vga::flip()
{
   static Uint32 ticks = 0;
   Uint32 cur_ticks;

   if( !is_inited() )
      return;

   cur_ticks = SDL_GetTicks();
   if (cur_ticks > ticks + 17 || cur_ticks < ticks) {
      ticks = cur_ticks;
      SDL_BlitSurface(vga_front.surface, NULL, target, NULL);
      SDL_UpdateTexture(texture, NULL, target->pixels, target->pitch);
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
   }
}
//-------- End of function Vga::flip ----------//


//-------- Beginning of function Vga::save_status_report ----------//
void Vga::save_status_report()
{
   FilePath path(sys.dir_config);
   FILE *file;
   int num, i;
   const char *s;
   SDL_version ver;

   path += "sdl.txt";
   if( path.error_flag )
      return;

   file = fopen(path, "w");
   if( !file )
      return;

   fprintf(file, "=== Seven Kingdoms " SKVERSION " ===\n");
   s = SDL_GetPlatform();
   fprintf(file, "Platform: %s\n", s);
   if( SDL_BYTEORDER == SDL_BIG_ENDIAN )
      fprintf(file, "Big endian\n");
   else
      fprintf(file, "Little endian\n");
#ifndef HAVE_KNOWN_BUILD
   fprintf(file, "Binary built using unsupported configuration\n");
#endif

   s = SDL_GetCurrentVideoDriver();
   fprintf(file, "Current SDL video driver: %s\n", s);
   SDL_GetVersion(&ver);
   fprintf(file, "SDL version: %d.%d.%d\n", ver.major, ver.minor, ver.patch);
   SDL_VERSION(&ver);
   fprintf(file, "Compiled SDL version: %d.%d.%d\n\n", ver.major, ver.minor, ver.patch);

   fprintf(file, "-- Video drivers --\n");
   num = SDL_GetNumVideoDrivers();
   for( i=0; i<num; i++ )
   {
      s = SDL_GetVideoDriver(i);
      fprintf(file, "%d: %s\n", i, s);
   }
   fprintf(file, "\n");

   if( window )
   {
      int x, y, w, h = 0;
      SDL_RendererInfo info;
      SDL_Renderer *r = SDL_GetRenderer(window);

      fprintf(file, "-- Current window --\n");
      fprintf(file, "Active on display: %d\n", SDL_GetWindowDisplayIndex(window));
      SDL_GetWindowPosition(window, &x, &y);
      SDL_GetWindowSize(window, &w, &h);
      fprintf(file, "Geometry: %dx%d @ (%d, %d)\n", w, h, x, y);
      fprintf(file, "Pixel format: %s\n", SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(window)));
      fprintf(file, "Full screen: %s\n", is_full_screen() ? "yes" : "no");
      fprintf(file, "Input grabbed: %s\n\n", SDL_GetWindowGrab(window) ? "yes" : "no");
      if( r )
      {
         SDL_RendererInfo info;
         float xscale, yscale;
         SDL_Rect rect;

         SDL_GetRendererInfo(r, &info);
         get_window_scale(&xscale, &yscale);
         SDL_RenderGetViewport(renderer, &rect);
         SDL_RenderGetLogicalSize(renderer, &w, &h);

         fprintf(file, "-- Current renderer: %s --\n", info.name);
         fprintf(file, "Viewport: x=%d,y=%d,w=%d,h=%d\n", rect.x, rect.y, rect.w, rect.h);
         fprintf(file, "Scale: xscale=%f,yscale=%f\n", xscale, yscale);
         fprintf(file, "Logical size: w=%d, h=%d\n", w, h);
         fprintf(file, "Capabilities: %s\n", info.flags & SDL_RENDERER_ACCELERATED ? "hardware accelerated" : "software fallback");
         fprintf(file, "V-sync: %s\n", info.flags & SDL_RENDERER_PRESENTVSYNC ? "on" : "off");
         fprintf(file, "Rendering to texture supported: %s\n", info.flags & SDL_RENDERER_TARGETTEXTURE ? "yes" : "no");
         if( info.max_texture_width || info.max_texture_height )
            fprintf(file, "Maximum texture size: %dx%d\n", info.max_texture_width, info.max_texture_height);
         fprintf(file, "Pixel formats:\n");
         for( unsigned i=0; i<info.num_texture_formats; i++ )
            fprintf(file, "\t%s\n", SDL_GetPixelFormatName(info.texture_formats[i]));
      }
   }
   fprintf(file, "\n");

   if( texture )
   {
      uint32_t format = 0;
      int w, h = 0;
      SDL_QueryTexture(texture, &format, NULL, &w, &h);
      fprintf(file, "-- Streaming texture --\n");
      fprintf(file, "Size: %dx%d\n", w, h);
      fprintf(file, "Pixel format: %s\n\n", SDL_GetPixelFormatName(format));
   }

   num = SDL_GetNumVideoDisplays();
   for( i=0; i<num; i++ )
   {
      SDL_Rect rect;
      SDL_DisplayMode mode;
      float ddpi, hdpi, vdpi;
      int num_modes, cur_mode, j;
      num_modes = SDL_GetNumDisplayModes(i);
      cur_mode = SDL_GetCurrentDisplayMode(i, NULL);
      fprintf(file, "-- Display %d using mode %d--\n", i, cur_mode);
      for( j=0; j<num_modes; j++ )
      {
          if( !SDL_GetDisplayMode(i, j, &mode) )
              fprintf(file, "Mode %d: %dx%dx%ubpp %dHz format=%s driver=%p\n", j, mode.w, mode.h, SDL_BITSPERPIXEL(mode.format), mode.refresh_rate, SDL_GetPixelFormatName(mode.format), mode.driverdata);
      }
      if( !SDL_GetDisplayDPI(i, &ddpi, &hdpi, &vdpi) )
         fprintf(file, "DPI: diag=%f horiz=%f vert=%f\n", ddpi, hdpi, vdpi);
      if( !SDL_GetDisplayBounds(i, &rect) )
         fprintf(file, "Bounds: x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h);
#if SDL_VERSION_ATLEAST(2, 0, 5)
      if( !SDL_GetDisplayUsableBounds(i, &rect) ) // Note: requires SDL 2.0.5+
         fprintf(file, "Usable bounds: x=%d y=%d w=%d h=%d\n", rect.x, rect.y, rect.w, rect.h);
#endif
      fprintf(file, "\n");
   }

   num = SDL_GetNumRenderDrivers();
   for( i=0; i<num; i++ )
   {
      SDL_RendererInfo info;
      SDL_GetRenderDriverInfo(i, &info);
      fprintf(file, "-- Renderer %s (%d) --\n", info.name, i);
      fprintf(file, "Capabilities: %s\n", info.flags & SDL_RENDERER_ACCELERATED ? "hardware accelerated" : "software fallback");
      fprintf(file, "V-sync capable: %s\n", info.flags & SDL_RENDERER_PRESENTVSYNC ? "on" : "off");
      fprintf(file, "Rendering to texture supported: %s\n", info.flags & SDL_RENDERER_TARGETTEXTURE ? "yes" : "no");
      if( info.max_texture_width || info.max_texture_height )
         fprintf(file, "Maximum texture size: %dx%d\n", info.max_texture_width, info.max_texture_height);
      fprintf(file, "Pixel formats:\n");
      for( unsigned j=0; j<info.num_texture_formats; j++ )
         fprintf(file, "\t%s\n", SDL_GetPixelFormatName(info.texture_formats[j]));
      fprintf(file, "\n");
   }

   fclose(file);
   return;
}
//-------- End of function Vga::save_status_report ----------//


//-------- Beginning of function Vga::get_window_scale ----------//
void Vga::get_window_scale(float *xscale, float *yscale)
{
   if( config_adv.vga_keep_aspect_ratio )
   {
      SDL_RenderGetScale(renderer, xscale, yscale);
   }
   else
   {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);
      *xscale = (float)w / (float)VGA_WIDTH;
      *yscale = (float)h / (float)VGA_HEIGHT;
   }
}
//-------- End of function Vga::get_window_scale ----------//


#ifdef USE_WINDOWS

#include <windows.h>

typedef enum PROCESS_DPI_AWARENESS {
   PROCESS_DPI_UNAWARE = 0,
   PROCESS_SYSTEM_DPI_AWARE = 1,
   PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

BOOL(WINAPI *pSetProcessDPIAware)(void); // Vista and later
HRESULT(WINAPI *pSetProcessDpiAwareness)(PROCESS_DPI_AWARENESS dpiAwareness); // Windows 8.1 and later

// Based on the example provided by Eric Wasylishen
// https://discourse.libsdl.org/t/sdl-getdesktopdisplaymode-resolution-reported-in-windows-10-when-using-app-scaling/22389
static void init_dpi()
{
   void* userDLL;
   void* shcoreDLL;

   shcoreDLL = SDL_LoadObject("SHCORE.DLL");
   if (shcoreDLL)
   {
      pSetProcessDpiAwareness = (HRESULT(WINAPI *)(PROCESS_DPI_AWARENESS)) SDL_LoadFunction(shcoreDLL, "SetProcessDpiAwareness");
   }

   if (pSetProcessDpiAwareness)
   {
      /* Try Windows 8.1+ version */
      HRESULT result = pSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
      return;
   }

   userDLL = SDL_LoadObject("USER32.DLL");
   if (userDLL)
   {
      pSetProcessDPIAware = (BOOL(WINAPI *)(void)) SDL_LoadFunction(userDLL, "SetProcessDPIAware");
   }

   if (pSetProcessDPIAware)
   {
      /* Try Vista - Windows 8 version.
      This has a constant scale factor for all monitors.
      */
      BOOL success = pSetProcessDPIAware();
   }
}

#endif

static int init_window_flags()
{
   int flags = 0;
   if( config_adv.vga_full_screen )
      flags |= config_adv.vga_full_screen_desktop ?
         SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
   if( config_adv.vga_allow_highdpi )
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;
   return flags;
}

static void init_window_size()
{
   if( !config_adv.vga_full_screen_desktop )
   {
      // must match game's native resolution
      config_adv.vga_window_width = 800;
      config_adv.vga_window_height = 600;
      return;
   }

   if( config_adv.vga_window_width && config_adv.vga_window_height )
      return;

#if SDL_VERSION_ATLEAST(2, 0, 5)
   int display_idx;
   SDL_Window *size_win = SDL_CreateWindow(WIN_TITLE, 0, 0, 1, 1, 0);
   if( !size_win )
      goto unknown_display;

   display_idx = SDL_GetWindowDisplayIndex(size_win);
   SDL_DestroyWindow(size_win);
   if( display_idx < 0 )
      goto unknown_display;

   SDL_Rect rect;
   if( SDL_GetDisplayUsableBounds(display_idx, &rect)<0 )
      goto unknown_display;

   if( rect.w >= 1024 && rect.h >= 768 )
   {
      config_adv.vga_window_width = 1024;
      config_adv.vga_window_height = 768;
      return;
   }

   if( rect.w >= 800 && rect.h >= 600 )
   {
      config_adv.vga_window_width = 800;
      config_adv.vga_window_height = 600;
      return;
   }

   config_adv.vga_window_width = 640;
   config_adv.vga_window_height = 480;
   return;
#endif

unknown_display:
      config_adv.vga_window_width = 800;
      config_adv.vga_window_height = 600;
      return;
}
