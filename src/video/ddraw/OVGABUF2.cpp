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

//Filename    : OVGABUF2.CPP
//Description : OVGABUF direct draw surface class - part 2

#include <ALL.h>
#include <IMGFUN.h>
#include <OMOUSE.h>
#include <OPOWER.h>
#include <OCOLTBL.h>
#include <OVGA.h>
#include <OVGABUF.h>
#include <OIMGRES.h>
#include <OFONT.h>

//------- Define static class member vars ---------//

char VgaBuf::color_light =(char)0x9E;    // color of the light panel side
char VgaBuf::color_dark  =(char)0x99;    // color of the dark panel side
char VgaBuf::color_up	 =(char)0x9D;    // color of up_panel surface
char VgaBuf::color_down  =(char)0x9C;    // color of down_panel surface
char VgaBuf::color_push  =(char)0x9B;    // color of pushed button surface
char VgaBuf::color_border=(char)0x98;    // color of the border

//------------- Begin of function VgaBuf::bar --------------//
//
// Draw a bar without bliting
//
// Syntax : bar( x1, y1, x2, y2, color )
//
// <int> x1,y1       - the top left vertex of the bar
// <int> x2,y2       - the bottom right vertex of the bar
// <int> color index - the index of a 256 color palette
//
void VgaBuf::bar(int x1,int y1,int x2,int y2,int colorCode)
{
	err_when( !buf_locked );

	if( is_front )
		mouse.hide_area(x1,y1,x2,y2);

	IMGbar(buf_ptr(), buf_pitch(), x1, y1, x2, y2, colorCode);

	if( is_front )
		mouse.show_area();
}
//--------------- End of function VgaBuf::bar --------------//


//------------- Begin of function VgaBuf::pixelize --------------//
//
// Put an area of evenly distributed pixels on the given area.
//
// <int> x1,y1     - the top left vertex of the bar
// <int> x2,y2     - the bottom right vertex of the bar
// <int> colorCode - colorCode of the pixels
//
void VgaBuf::pixelize(int x1,int y1,int x2,int y2,int colorCode)
{
	err_when( !buf_locked );

	if( is_front )
		mouse.hide_area(x1,y1,x2,y2);

   //---------------------------------------//

	// ###### begin Gilbert 7/7 #########//
	int	bufPitch = buf_pitch();
	int   x, y, lineRemainBytes = bufPitch - (x2-x1+1);
	char* writePtr = buf_ptr() + bufPitch * y1 + x1;

	for( y=y1 ; y<=y2 ; y++ )
	{
		if( y&1 )
		{
			writePtr+=bufPitch;
		}
		else
		{
			for( x=x1 ; x<=x2 ; x++, writePtr++ )
			{
				if( x&1 )
					*writePtr = colorCode;
			}

			writePtr+=lineRemainBytes;
		}
	}
	// ###### end Gilbert 7/7 #########//

	//---------------------------------------//

	if( is_front )
		mouse.show_area();
}
//--------------- End of function VgaBuf::pixelize --------------//


//------------- Begin of function VgaBuf::rect --------------//
//
// Draw a rect on VGA screen
//
// Syntax : rect( x1, y1, x2, y2, lineWidth, color )
//
// int x1,y1       - the top left vertex of the rect
// int x2,y2       - the bottom right vertex of the rect
// int lineThick   - thickness of the lines of the rect
// int color       - the color of the rect
//
void VgaBuf::rect(int x1,int y1,int x2,int y2,int lt, int color)
{
	lt--;

	bar(x1,y1,x2,y1+lt,color);
	bar(x1,y1,x1+lt,y2,color);
	bar(x1,y2-lt,x2,y2,color);
	bar(x2-lt,y1,x2,y2,color);
}
//--------------- End of function VgaBuf::rect --------------//


//------------- Start of function VgaBuf::d3_rect --------------//
//
// Draw a d3 rect on VGA screen
//
// Syntax : d3_rect( x1, y1, x2, y2, lineWidth, color )
//
// int x1,y1       - the top left vertex of the d3_rect
// int x2,y2       - the bottom right vertex of the d3_rect

void VgaBuf::d3_rect(int x1,int y1,int x2,int y2)
{
	rect( x1+1, y1+1, x2, y2, 1, V_WHITE );
	rect( x1, y1, x2-1, y2-1, 1, VGA_GRAY+8 );
}
//--------------- End of function VgaBuf::d3_rect --------------//

/*

//------------- Start of function VgaBuf::tile --------------//
//
// Fill an area with a specific tile
//
// <int>   x1,y1    - the top left vertex of the tile
// <int>   x2,y2    - the bottom right vertex of the tile
// <char*> tileName - the name of the tile
//
void VgaBuf::tile(int x1,int y1,int x2,int y2,char* tileName)
{
   if( image_init_flag )
   {
      err_if( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=image_width || y2>=image_height )
         err_now( "VgaBuf::tile (image)" );

		char* tilePtr = icon_sym.get_ptr(tileName);

      if( tilePtr )
         IMGtile(x1,y1,x2,y2,tilePtr);
   }
   else
   {
      err_if( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT )
         err_now( "VgaBuf::tile (direct)" );

		if( is_front )
			mouse.hide_area( x1,y1,x2,y2 );

		char* tilePtr = icon_sym.get_ptr(tileName);

		if( tilePtr )
			VGAtile(x1,y1,x2,y2,tilePtr);

		if( is_front )
			mouse.show_area();
	}
}
//--------------- End of function VgaBuf::tile --------------//

*/

//------------- Start of function VgaBuf::separator --------------//
//
// Draw a VGA separator line
//
// Syntax : separator( x1, y1, x2, y2 )
//
// int x1,y1       - the top left vertex of the separator
// int x2,y2       - the bottom right vertex of the separator
//
void VgaBuf::separator(int x1, int y1, int x2, int y2)
{
   if( y1+1==y2 )       // horizontal line
   {
      bar( x1, y1, x2, y1, V_WHITE        );
      bar( x1, y2, x2, y2, color_dark );
	}
   else
   {
      bar( x1, y1, x1, y2, V_WHITE        );
      bar( x2, y1, x2, y2, color_dark );
   }
}
//--------------- End of function VgaBuf::separator --------------//


//------------- Start of function VgaBuf::indicator --------------//
//
// <int>   x1, y1, x2, y2 = coordination of the indicator
// <float> curValue       = the value of the bar
// <float> maxValue       = MAX value, the bar width = maxBarWidth * curValue / maxValue
// <int>   indiColor      = color of the indicator
// [int]   backColor      = background color
//                          (default : vga.color_down)
//			    (-2 if don't paint background color)
//
void VgaBuf::indicator(int x1, int y1, int x2, int y2, float curValue,
		    float maxValue, int indiColor, int backColor)
{
   if( backColor == -1 )
      backColor = color_down;

   if( curValue > maxValue )
		curValue = maxValue;

   if( curValue > 0 )
   {
      int barWidth = (int) ((float)(x2-x1) * curValue / maxValue);

		int halfHeight = (y2-y1+1)/2-1;
		int tx2        = x1+barWidth;
		int y;

		indiColor+=halfHeight;

		for( y=y1 ; y<y1+halfHeight ; y++, indiColor-- )
			bar( x1, y, tx2, y, indiColor );

		for( ; y<=y2 ; y++, indiColor++ )
			bar( x1, y, tx2, y, indiColor );

		if( backColor != -2 )	// -2 if don't paint background color
		{
			if( x1+barWidth < x2 )
			bar( x1+barWidth+1, y1, x2, y2, backColor );
		}
	}
	else
	{
		if( backColor != -2 )	// -2 if don't paint background color
			bar( x1, y1, x2, y2, backColor );
   }
}
//--------------- End of function VgaBuf::indicator --------------//


//------------- Start of function VgaBuf::indicator --------------//
//
// <int>   barType        = bar style, bit 0 = disp curValue, bit 1 =  disp '/' and maxValue, bit 2 = longer bar, bit 3 = use another back buffer
// <int>   x1, y1         = coordination of the indicator
// <float> curValue       = the value of the bar
// <float> maxValue       = MAX value, the bar width = maxBarWidth * curValue / maxValue
// <int>   colorScheme    = color of the indicator // not used
//
void VgaBuf::indicator(int barType, int x1, int y1, float curValue, float maxValue, int colorScheme)
{
	// -------- get indicator bar background ------------//

	const unsigned int TEMP_BUFFER_SIZE = 0x2000;
	static char tempBuffer[TEMP_BUFFER_SIZE];

	char *bgPtr = image_button.get_ptr(barType & 4 ? (char*)"V_HP-BX" : (char*)"V_PW-BX");
	if( !bgPtr )
	{
		err_here();
		return;
	}

	// -------- calculate the boundary or the indicator ---------//
	int width = *(short *)bgPtr;
	int height = *(((short *)bgPtr) + 1);

	int bufferX1 = x1;
	int bufferY1 = y1;

	char *destPtr = buf_ptr();
	int destPitch = buf_pitch();

	if(barType & 8)
	{
		if( width * height + 2*sizeof(short) < TEMP_BUFFER_SIZE )
		{
			bufferX1 = 0;
			bufferY1 = 0;
			destPtr = tempBuffer;
			*(short *)destPtr = width;
			destPtr += sizeof(short);
			*(short *)destPtr = height;
			destPtr += sizeof(short);
			destPitch = width;
		}
		else
		{
			barType &= ~8;		// indicator too big, directly write to the buffer
		}
	}

	int x2 = x1 + width -1;
	int y2 = y1 + height -1;
	int bufferX2 = bufferX1 + width -1;
	int bufferY2 = bufferY1 + height -1;
	
	// -------- draw background ---------//

	if( is_front )
		mouse.hide_area(x1, y1, x2, y2 );

	IMGblt( destPtr, destPitch, bufferX1, bufferY1, bgPtr);

	const int barLeftMargin = 5;
	const int barRightMargin = 5;
	const int barTopMargin = 5;
	const int barBottomMargin = 5;

	// ---------- draw energy bar ----------//

	if( curValue > maxValue )
		curValue = maxValue;
	int barWidth = int( (width - barLeftMargin - barRightMargin ) * curValue / maxValue);
	int barX1 = bufferX1 + barLeftMargin;
	int barX2 = barX1 + barWidth - 1;
	int barHeight = height - (barTopMargin + barBottomMargin);
	int barY1 = bufferY1 + barTopMargin;

	unsigned int lc = 0;
	static unsigned char lineColor[8] = 
	{ VGA_LIGHT_BLUE, VGA_LIGHT_BLUE+1, VGA_DARK_BLUE, VGA_LIGHT_BLUE+2, 
		VGA_DARK_BLUE+1, VGA_LIGHT_BLUE+3, VGA_DARK_BLUE+2, VGA_DARK_BLUE+3 };

	if( barWidth > 0 )
	{
		if(barHeight >= 8)
		{
			int bY1 = barY1;
			for( int c = 0; c < 8; ++c)
			{
				int bY2 = bY1 + (lc+ barHeight -1)/8;
				IMGbar(destPtr, destPitch, barX1, bY1, barX2, bY2, lineColor[c] );
				lc += barHeight;
				bY1 += lc / 8;
				lc %= 8;
			}
		}
		else
		{
			for( int y = 0; y < barHeight; ++y)
			{
				int c = lc/barHeight;
				IMGbar(destPtr, destPitch, barX1, barY1+y, barX2, barY1+y, lineColor[c]);
				lc += 8;
			}
		}
	}

	// ---------- display value ----------//

	if( barType & 3)
	{
		String str;

		if( barType & 1)
			str  = (int) curValue;
		if( barType & 2)
		{
			str += "/";
			str += (int) maxValue;
		}

		font_hitpoint.center_put_to_buffer( destPtr, destPitch, bufferX1, bufferY1, bufferX2, bufferY2, str);
	}

	if(barType & 8)
	{
		put_bitmap(x1, y1, tempBuffer);
	}
	// ---------------------------------- //

	if( is_front )
		mouse.show_area();

}
//------------- End of function VgaBuf::indicator --------------//


//------------- Start of function VgaBuf::v_indicator --------------//
//
// Vertical indicator, all parameters are same as indicator() except
// it is vertical.
//
// <int>   x1, y1, x2, y2 = coordination of the indicator
// <float> curValue       = the value of the bar
// <float> maxValue       = MAX value, the bar width = maxBarWidth * curValue / maxValue
// <int>   indiColor      = color of the indicator
// [int]   backColor      = background color
//                          (default : vga.color_down)
//
void VgaBuf::v_indicator(int x1, int y1, int x2, int y2, float curValue,
                      float maxValue, int indiColor, int backColor)
{
   if( backColor == -1 )
      backColor = color_down;

	if( curValue > 0 )
   {
      int barHeight = (int) ((float)(y2-y1) * curValue / maxValue);

      int halfWidth = (x2-x1+1)/2-1;
		int ty1        = MAX(y2-barHeight,y1); // when curValue>0, even the actual bar width < 1, one pixel will also be painted
		int x;

		indiColor+=halfWidth;

		for( x=x1 ; x<x1+halfWidth ; x++, indiColor-- )
			bar( x, ty1, x, y2, indiColor );

		for( ; x<=x2 ; x++, indiColor++ )
			bar( x, ty1, x, y2, indiColor );

		if( y1 < y2-barHeight )
			bar( x1, y1, x2, y2-barHeight-1, backColor );
	}
	else
	{
		bar( x1, y1, x2, y2, backColor );
	}
}
//--------------- End of function VgaBuf::v_indicator --------------//


//---------- Begin of function VgaBuf::line -------------//
//
// Draw a line
//
// <int> x1,y1,x2,y2 = the coordination of the line
// <int> lineColor   = color of the line
//
void VgaBuf::line(int x1,int y1,int x2,int y2,int lineColor)
{
	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );  // if the mouse cursor is in that area, hide it

	IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1, y1, x2, y2, lineColor);

	if( is_front )
      mouse.show_area();
}
//------------ End of function VgaBuf::line -------------//


//---------- Begin of function VgaBuf::thick_line -------------//
//
// Draw a thick line
//
// <int> x1,y1,x2,y2 = the coordination of the line
// <int> lineColor   = color of the line
//
void VgaBuf::thick_line(int x1,int y1,int x2,int y2,int lineColor)
{
	err_when( x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );  // if the mouse cursor is in that area, hide it

	if( y1-y2 > abs(x2-x1) )   // keep thickness of the line to 3
	{
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1  , y1-1, x2  , y2-1, lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1  , y1  , x2  , y2  , lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1  , y1+1, x2  , y2+1, lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1+1, y1+1, x2+1, y2+1, lineColor );
	}

	else if( y2-y1 > abs(x2-x1) )
	{
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1+1, y1-1, x2+1, y2-1, lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1  , y1-1, x2  , y2-1, lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1  , y1  , x2  , y2  , lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1  , y1+1, x2  , y2+1, lineColor );
	}

	else
	{
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1, y1-1, x2, y2-1, lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1, y1  , x2, y2  , lineColor );
		IMGline(buf_ptr(), buf_pitch(), buf_width(), buf_height(), x1, y1+1, x2, y2+1, lineColor );
	}

	if( is_front )
      mouse.show_area();
}
//------------ End of function VgaBuf::thick_line -------------//


//----------- Begin of function VgaBuf::d3_panel_up ------------//
//
// Draw a Ms windows style 3D panel
//
// thickness can be 1-4
//
// <int> x1,y1,x2,y2 = the four vertex of the panel
// [int] thick       = thickness of the border
//                     (default:2)
// [int] paintCentre = paint the center area of the plane or not
//                     pass 0 if the area has just been painted with bar()
//                     (default:1)
//
void VgaBuf::d3_panel_up(int x1,int y1,int x2,int y2,int t,int paintCentre)
{
	// int i,x,y;

	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );

	//------------------------------------------------//

	// ####### begin Gilbert 12/7 ##########//
	if( paintCentre )
		bar(x1+2, y1+2, x2-3, y2-3, color_up );  // center

	//--------- white border on top and left sides -----------//

	bar(x1,y1,x2-3,y1+1,0x9a );
	draw_pixel(x2-2, y1, 0x9a);
	bar(x1,y1+2,x1+1,y2-3, 0x9a );    // left side
	draw_pixel(x1, y2-2, 0x9a);

	//--------- black border on bottom and right sides -----------//

	bar(x2-2,y1+2,x2-1,y2-1, 0x90 );     // bottom side
	draw_pixel(x2-1, y1+1, 0x90);
	bar(x1+2,y2-2,x2-3,y2-1, 0x90 );		 // right side
	draw_pixel(x1+1, y2-1, 0x90);

	//--------- junction between white and black border --------//
	draw_pixel(x2-1, y1, 0x97);
	draw_pixel(x2-2, y1+1, 0x97);
	draw_pixel(x1, y2-1, 0x97);
	draw_pixel(x1+1, y2-2, 0x97);

	//--------- gray shadow on bottom and right sides -----------//
	bar(x2, y1+1, x2, y2, 0x97);
	bar(x1+1, y2, x2-1, y2, 0x97);

	//-------------------------------------------//
	// ######## end Gilbert 12/7 ##########//

	if( is_front )
		mouse.show_area();
}
//------------- End of function VgaBuf::d3_panel_up ------------//


//----------- Begin of function VgaBuf::d3_panel_down ------------//
//
// Draw a Ms windows style 3D panel with panel pushed downwards
//
// <int> x1,y1,x2,y2 = the four vertex of the panel
// [int] thick       = thickness of the border
//                     (default:2)
// [int] paintCentre = paint the center area of the plane or not
//                     pass 0 if the area has just been painted with bar()
//                     (default:1)
//
void VgaBuf::d3_panel_down(int x1,int y1,int x2,int y2,int t,int paintCentre)
{
	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );

	//---------- main center area -----------//

	// ####### begin Gilbert 12/7 ##########//
	if( paintCentre )
		bar(x1+2, y1+2, x2-3, y2-3, color_down );  // center

	//--------- black border on top and left sides -----------//

	bar(x1,y1,x2-3,y1+1,0x90 );
	draw_pixel(x2-2, y1, 0x90);
	bar(x1,y1+2,x1+1,y2-3, 0x90 );    // left side
	draw_pixel(x1, y2-2, 0x90);

	//--------- while border on bottom and right sides -----------//

	bar(x2-2,y1+2,x2-1,y2-1, 0x9a );     // bottom side
	draw_pixel(x2-1, y1+1, 0x9a);
	bar(x1+2,y2-2,x2-3,y2-1, 0x9a );		 // right side
	draw_pixel(x1+1, y2-1, 0x9a);

	//--------- junction between white and black border --------//
	draw_pixel(x2-1, y1, 0x97);
	draw_pixel(x2-2, y1+1, 0x97);
	draw_pixel(x1, y2-1, 0x97);
	draw_pixel(x1+1, y2-2, 0x97);

	//--------- remove shadow, copy from back  -----------//
	bar(x2, y1+1, x2, y2, 0x9c);
	bar(x1+1, y2, x2-1, y2, 0x9c);
	// ####### end Gilbert 12/7 ##########//

	//----------- show mouse ----------//

	if( is_front )
		mouse.show_area();
}
//------------- End of function VgaBuf::d3_panel_down ------------//


//----------- Begin of function VgaBuf::d3_panel_up_clear ------------//
//
// clear the center of the either up or down panel
//
// <int> x1,y1,x2,y2 = the four vertex of the panel
// [int] thick       = thickness of the border
//                     (default:2)
//
void VgaBuf::d3_panel_up_clear(int x1,int y1,int x2,int y2,int t)
{
	// ####### begin Gilbert 15/7 #########//
	// bar( x1+t, y1+t, x2-t, y2-t, color_up );
	bar(x1+2, y1+2, x2-3, y2-3, color_up );
	// ####### end Gilbert 15/7 #########//
}
//------------- End of function VgaBuf::d3_panel_up_clear ------------//


//----------- Begin of function VgaBuf::d3_panel_down_clear ------------//
//
// clear the center of the either up or down panel
//
// <int> x1,y1,x2,y2 = the four vertex of the panel
// [int] thick       = thickness of the border
//                     (default:2)
//
void VgaBuf::d3_panel_down_clear(int x1,int y1,int x2,int y2,int t)
{
	// ####### begin Gilbert 15/7 #########//
	// bar( x1+t, y1+t, x2-t, y2-t, color_down );
	bar(x1+2, y1+2, x2-3, y2-3, color_down );
	// ####### end Gilbert 15/7 #########//
}
//------------- End of function VgaBuf::d3_panel_down_clear ------------//


//----------- Begin of function VgaBuf::adjust_brightness ------------//
//
// clear the center of the either up or down panel
//
// <int> x1,y1,x2,y2  = the four vertex of the panel
// <int> adjustDegree = the degree of brightness to adjust
//							   (a value from -10 to 10)
//
void VgaBuf::adjust_brightness(int x1,int y1,int x2,int y2,int adjustDegree)
{
	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );

	err_when( adjustDegree < -MAX_BRIGHTNESS_ADJUST_DEGREE ||
				 adjustDegree >  MAX_BRIGHTNESS_ADJUST_DEGREE );

	unsigned char* colorRemapTable = vga.vga_color_table->get_table(adjustDegree);

	remap_bar(x1, y1, x2, y2, colorRemapTable);

	if( is_front )
		mouse.show_area();
}
//------------- End of function VgaBuf::adjust_brightness ------------//


//----------- Begin of function VgaBuf::draw_d3_up_border ------------//
//
// Draw interface border.
//
void VgaBuf::draw_d3_up_border(int x1,int y1,int x2,int y2)
{
	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );

	//--------- white border on top and left sides -----------//

	IMGbar( buf_ptr(), buf_pitch(), x1+1,y1,x2,y1, IF_LIGHT_BORDER_COLOR );    // top side
	IMGbar( buf_ptr(), buf_pitch(), x1,y1,x1,y2  , IF_LIGHT_BORDER_COLOR );    // left side

	//--------- black border on bottom and right sides -----------//

	IMGbar( buf_ptr(), buf_pitch(), x1+1,y2,x2,y2, IF_DARK_BORDER_COLOR );     // bottom side
	IMGbar( buf_ptr(), buf_pitch(), x2,y1+1,x2,y2, IF_DARK_BORDER_COLOR );		 // right side

	//-------------------------------------------//

	if( is_front )
		mouse.show_area();
}
//------------- End of function VgaBuf::draw_d3_up_border ------------//


//----------- Begin of function VgaBuf::draw_d3_down_border ------------//
//
// Draw interface border.
//
void VgaBuf::draw_d3_down_border(int x1,int y1,int x2,int y2)
{
	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	if( is_front )
		mouse.hide_area( x1,y1,x2,y2 );

	//--------- white border on top and left sides -----------//

	IMGbar( buf_ptr(), buf_pitch(), x1+1,y1,x2,y1, IF_DARK_BORDER_COLOR );    // top side
	IMGbar( buf_ptr(), buf_pitch(), x1,y1,x1,y2  , IF_DARK_BORDER_COLOR );    // left side

	//--------- black border on bottom and right sides -----------//

	IMGbar( buf_ptr(), buf_pitch(), x1+1,y2,x2,y2, IF_LIGHT_BORDER_COLOR );     // bottom side
	IMGbar( buf_ptr(), buf_pitch(), x2,y1+1,x2,y2, IF_LIGHT_BORDER_COLOR );		 // right side

	//-------------------------------------------//

	if( is_front )
		mouse.show_area();
}
//------------- End of function VgaBuf::draw_d3_down_border ------------//


//------------- Begin of function VgaBuf::blt_buf ------------//
// copy put whole part of a vgaBuf to (x1,y1) of this VgaBuf
void VgaBuf::blt_buf( VgaBuf *srcBuf, int x1, int y1 )
{
	char *srcPtr = srcBuf->buf_ptr();
	int srcWidth = srcBuf->buf_width();
	int srcPitch = srcBuf->buf_pitch();
	int srcHeight = srcBuf->buf_height();
	char *destPtr = buf_ptr();
	int destPitch = buf_pitch();

	// this used to be assembly coded, but memcpy() does this better
	int dest = y1 * destPitch + x1;
	int src = 0;
	for (int y=0; y<srcHeight; ++y)
	{
		memcpy( &destPtr[dest], &srcPtr[src], srcWidth );
		dest += destPitch;
		src += srcPitch;
	}
}
//------------- End of function VgaBuf::blt_buf ------------//


//-------- Begin of function VgaBuf::blt_virtual_buf --------//
//
// Blit entire source surface to local destination surface.
//
void VgaBuf::blt_virtual_buf( VgaBuf *source )
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
//--------- End of function VgaBuf::blt_virtual_buf ---------//
