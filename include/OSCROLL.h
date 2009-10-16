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

//Filename    : OSCROLL.H
//Description : Header file of Object Scroll Bar

#ifndef __OSCROLL_H
#define __OSCROLL_H

#ifndef __ALL_H
#include <ALL.h>
#endif

//--------- Define class ScrollBar --------//

class ScrollBar
{
public:
   enum { VERTICAL=1, HORIZONTAL=2 };

private:
   char  type;		    // either VERTICAL or HORIZONTAL
   short x1,y1,x2,y2;       // coordinations of the list box

   short total_rec_num;     // total no. of records
   short top_rec_no;        // the record no. of the record on the top of the list box

   short skip_rec_num;
   short disp_rec_num;
   char  page_type;         // yes or no, for refered when user press down button
	char	if_flag;
	char  vga_front_only;
   char  top_rec_flag;

   short slot_height;       // height of the scroll slot
   short indicator_height;  // height of the scroll indicator
   short indicator_y;       // the position of the indicator

   DWORD	next_press_time;   // clock() must > next_press_time, so we
									 // would allow next mouse press action

public :
   ScrollBar()  { type= -1; }

	void  init(int,int,int,int,int,int,int,int,int=1,int=0,int=0);

	void  paint(int=1);
   void  refresh(int,int=0,int= -1,int= -1,int= -1);
   int   detect();
   int   detect_top_rec();

   int   page_up(int=0);
   int   page_down(int=0);

   int   rec_to_y(int);
   int   y_to_rec(int);
};

//-------------------------------------//

#endif
