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

#include <OSYS.h>
#include <OVGA.h>

#include <RESOURCE.h>

//------- Define static functions -----------//

static long FAR PASCAL main_win_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//-------- Begin of function VgaDDraw::create_window --------//
//
int VgaDDraw::create_window()
{
   app_hinstance = (HINSTANCE)GetModuleHandle(NULL);

   //--------- register window class --------//

   WNDCLASS    wc;
   BOOL        rc;

   wc.style          = CS_DBLCLKS;
   wc.lpfnWndProc    = main_win_proc;
   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hInstance      = app_hinstance;
   wc.hIcon          = LoadIcon( app_hinstance, MAKEINTATOM(IDI_ICON1));
   wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
   wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wc.lpszMenuName   = NULL;
   wc.lpszClassName  = WIN_CLASS_NAME;

   rc = RegisterClass( &wc );

   if( !rc )
      return 0;

   //--------- create window -----------//

   main_hwnd = CreateWindowEx(
       WS_EX_APPWINDOW | WS_EX_TOPMOST,
       WIN_CLASS_NAME,
       WIN_TITLE,
       WS_VISIBLE |    // so we dont have to call ShowWindow
       WS_POPUP,
       0,
       0,
       GetSystemMetrics(SM_CXSCREEN),
       GetSystemMetrics(SM_CYSCREEN),
       NULL,
       NULL,
       app_hinstance,
       NULL );

   if( !main_hwnd )
      return 0;

   UpdateWindow( main_hwnd );
   SetFocus( main_hwnd );

   ShowCursor(FALSE);

   return 1;
}
//-------- End of function VgaDDraw::create_window --------//

//-------- Begin of function VgaDDraw::destroy_window --------//
//
void VgaDDraw::destroy_window()
{
/*
   extern char low_video_memory_flag;

   if( low_video_memory_flag )
   {
      ShowWindow(sys.main_hwnd, SW_MINIMIZE );

      unsigned curTime = m.get_time();
      while( m.get_time() < curTime + 4000 );
   }
*/
   //---------------------------------------//

   if (main_hwnd) {
      PostMessage(main_hwnd, WM_CLOSE, 0, 0);

      MSG msg;

      while( GetMessage(&msg, NULL, 0, 0) )
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }

      main_hwnd = NULL;
   }
}
//-------- End of function VgaDDraw::destroy_window --------//

//--------- Begin of static function main_win_proc --------//
//
// Callback for all Windows messages
//
static long FAR PASCAL main_win_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch( message )
   {
      case WM_CREATE:
         vga.main_hwnd = hWnd;
         break;

      case WM_ACTIVATEAPP:
         // ####### begin Gilbert 3/11 #######//
         // sys.active_flag = (BOOL)wParam && GetForegroundWindow() == hWnd && !IsIconic(hWnd);
         sys.active_flag = (BOOL)wParam && !IsIconic(hWnd);
         // ####### end Gilbert 3/11 #######//

         //--------------------------------------------------------------//
         // while we were not-active something bad happened that caused us
         // to pause, like a surface restore failing or we got a palette
         // changed, now that we are active try to fix things
         //--------------------------------------------------------------//

         if( sys.active_flag )
         {
            sys.unpause();
            sys.need_redraw_flag = 1;      // for Sys::disp_frame to redraw the screen
         }
         else
            sys.pause();
         break;

       case WM_DESTROY:
          vga.main_hwnd = NULL;
          // game.deinit();          // end of game
          sys.deinit_directx();
          PostQuitMessage( 0 );
          break;

       case WM_ERASEBKGND:
          // do not erase the background
          return 1;

       case WM_PALETTECHANGED:
          // if we changed the palette, do nothing
          if ((HWND)wParam == hWnd) break;

          // set the current palette again
          vga.refresh_palette();
          break;

       default:
          break;
   }

   return DefWindowProc(hWnd, message, wParam, lParam);
}
//--------- End of static function main_win_proc --------//

//-------- Begin of function VgaDDraw::handle_messages --------//
void VgaDDraw::handle_messages()
{
   static int lastTick;

   int tick = GetTickCount();
   if (lastTick == tick)
      return;
   lastTick = tick;

   MSG msg;
   while (PeekMessage(&msg, main_hwnd, 0, 0, PM_NOREMOVE))
   {
      BOOL r;

      r = GetMessage(&msg, main_hwnd, 0, 0);
      if (r == -1)
      {
         // not handled
         return;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
}
//-------- End of function VgaDDraw::handle_messages --------//

//-------- Begin of function VgaDDraw::flag_redraw --------//
void VgaDDraw::flag_redraw()
{
   InvalidateRect(main_hwnd, NULL, TRUE);
}
//-------- End of function VgaDDraw::flag_redraw ----------//

