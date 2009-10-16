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

//Filename    : OSLIDER.H
//Description : Header of object Slider

#ifndef __OSLIDER_H
#define __OSLIDER_H

//--------- Define class Slider --------//

class Slider
{
public:
   short slider_x1, slider_y1, slider_x2, slider_y2;
   short bar_width;

   char  bar_color, bar_color2;

   int*  var_ptr;
   int   max_value, std_value;

public:
   void init( int,int,int,int,int,int*,int,int=0,int=0 );

   void paint();
   void refresh();
   int  detect();
};


//------- Define class SliderGroup --------//

class SliderGroup
{
public:
   int     slider_num;
   Slider* slider_array;

public:
   SliderGroup(int);
   ~SliderGroup()                { delete slider_array; }

   void paint();
   int  detect();

   Slider& operator[](int recNo) { if( recNo>slider_num ) recNo=0; return slider_array[recNo]; }
};

//-----------------------------------------//

#endif
