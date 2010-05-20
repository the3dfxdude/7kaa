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

//Filename    : OVGA3.CPP
//Description : VGA drawing functions

#include <ALL.h>
#include <IMGFUN.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OCOLTBL.h>
#include <OVGA.h>
#include <vga_util.h>

//-------- Define constant --------//

#define UP_OPAQUE_COLOR       (VGA_GRAY+10)
#define DOWN_OPAQUE_COLOR     (VGA_GRAY+13)


//--------- Begin of function VgaUtil::blt_buf ----------//
//
// Blt the back buffer to the front buffer.
//
// <int> x1, y1, x2, y2 - the coordinations of the area to be blit
// [int] putBackCursor  - whether put a mouse cursor onto the back buffer
//                        before blitting.
//                        (default: 1)
//
BOOL VgaUtil::blt_buf(int x1, int y1, int x2, int y2, int putBackCursor)
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
//---------- End of function VgaUtil::blt_buf ----------//


//----------- Begin of function VgaUtil::d3_panel_up ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void VgaUtil::d3_panel_up(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
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

   if( !vgaFrontOnly && !vga.use_back_buf )      // only blt the back to the front is the active buffer is the front
      blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function VgaUtil::d3_panel_up ------------//


//----------- Begin of function VgaUtil::d3_panel_down ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void VgaUtil::d3_panel_down(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
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

   if( !vgaFrontOnly && !vga.use_back_buf )      // only blt the back to the front is the active buffer is the front
      blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function VgaUtil::d3_panel_down ------------//


//----------- Begin of function VgaUtil::d3_panel2_up ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void VgaUtil::d3_panel2_up(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
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

   if( !vgaFrontOnly && !vga.use_back_buf )      // only blt the back to the front is the active buffer is the front
      blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function VgaUtil::d3_panel_up ------------//


//----------- Begin of function VgaUtil::d3_panel2_down ------------//
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//                      (default: 0)
// [int] drawBorderOnly = draw border only, do not brighten the center area
//                        (default: 0)
//
void VgaUtil::d3_panel2_down(int x1,int y1,int x2,int y2,int vgaFrontOnly,int drawBorderOnly)
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

   if( !vgaFrontOnly && !vga.use_back_buf )      // only blt the back to the front is the active buffer is the front
      blt_buf(x1, y1, x2, y2, 0);
}
//------------- End of function VgaUtil::d3_panel2_down ------------//


//------------- Start of function VgaUtil::separator --------------//
//
// Draw a VGA separator line
//
// Syntax : separator( x1, y1, x2, y2 )
//
// int x1,y1       - the top left vertex of the separator
// int x2,y2       - the bottom right vertex of the separator
//
void VgaUtil::separator(int x1, int y1, int x2, int y2)
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
//--------------- End of function VgaUtil::separator --------------//
