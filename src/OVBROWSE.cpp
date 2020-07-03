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

//Filename    : OVBROWSE.CPP
//Description : Object Visual Browse

#include <ALL.h>
#include <OINFO.h>
#include <OMOUSE.h>
#include <OVGA.h>
#include <OFONT.h>
#include <OPOWER.h>
#include <OVBROWSE.h>

//--------- Begin of function VBrowse::init --------//
//
// <int> x1, y1, x2, y2  = the coordinations of the list box
// <int> rec_width       = record area width
//                         (0 if get the maximum width)
// <int> rec_height      = record area height
// <int> totalRec        = total no. of records (for generating scroll bar indicator)
//
// <BrowDispFP> dispFunc = The pointer to function which will return the
//                         content of the record.
//                         When call this function a <int> rec_no parameter
//                         will pass to it.
// [int] dispFrame       = whether display a frame highlighting current
//                         item or not
//                         (default : 1)
// [int] minInterSpace   = minimum inter-record space
//                         (default : MIN_SPACE)
//
void VBrowse::init(int inX1, int inY1, int inX2, int inY2,
						 int recWidth, int recHeight,
						 int totalRec, BrowDispFP dispFunc,
						 int dispFrame, int minInterSpace)
{
	x1 = inX1;
	y1 = inY1;
	x2 = inX2;
	y2 = inY2;

	ix1 = x1+6;          // content area coordination
	iy1 = y1+6;
	ix2 = x2-6-SCROLL_BAR_WIDTH;
   iy2 = y2-6;

   //---------------------------------------------//

	if( recWidth <= 0 )
      rec_width = ix2-ix1+1;
   else
      rec_width = recWidth;

   rec_height = recHeight;

	disp_rec_func   = dispFunc;
	disp_frame      = dispFrame;

	x_max_rec = (ix2-ix1+minInterSpace+1) / (rec_width +minInterSpace);
	y_max_rec = (iy2-iy1+minInterSpace+1) / (rec_height+minInterSpace);

	detect_pull_flag = 1;		// whether detect pulling records or not
	press_record = 0;		// for pulling records upwards and downloads

   err_if( x_max_rec <= 0 || y_max_rec <= 0 )
      err_now( "VBrowse::init() error : disp_max_rec <= 0" );

   //--- calculate the suitable inter-record space -------//

   if( x_max_rec > 1 )
      rec_x_space = minInterSpace + (ix2-ix1+minInterSpace+1) % (rec_width +minInterSpace)/(x_max_rec-1);
   else
      rec_x_space = minInterSpace;

   if( y_max_rec > 1 )
      rec_y_space = minInterSpace + (iy2-iy1+minInterSpace+1) % (rec_height+minInterSpace)/(y_max_rec-1);
   else
      rec_y_space = minInterSpace;

	init_var(totalRec,-1);

   init_flag = 1;

	//------- paint the list box --------//

   paint();
}
//---------- End of function VBrowse::init ---------//


//---------- Begin of function VBrowse::init_var -------//
//
// Initialize variables for visual browser
//
void VBrowse::init_var(int totalRec, int recNo)
{
   if( totalRec >= 0 )          // if totalRec == -1, then no change
   {
      none_record = totalRec==0;

      if( totalRec == 0 )
         total_rec_num = 1;        // prevent divided by zero if it is a closed box
      else
         total_rec_num = totalRec;
	}

   //--- maximum records can be display on a browse box ---//

   disp_max_rec = x_max_rec * y_max_rec;

   if( disp_max_rec > total_rec_num )
      disp_max_rec = total_rec_num;

	//------- changable variables --------//

   rec_no     = recNo;

   top_rec_no = recNo - disp_max_rec/2;
   top_rec_no = MAX(1,top_rec_no);

   if( top_rec_no+disp_max_rec-1 > total_rec_num )      // if there is an empty area at the lower part of the browser
   {
      top_rec_no = total_rec_num-disp_max_rec+1;
      top_rec_no = MAX(top_rec_no,1);
   }

   //------------- initialize scroll bar ---------------//
   //
	// If display frame      , then scroll unit is 1 record
   // If don't display frame, then the scroll unit is disp_max_rec record
   //
   //---------------------------------------------------//

   scroll_bar.init( 1, x2-SCROLL_BAR_WIDTH, y1, x2, y2, disp_max_rec,
		    (disp_frame && x_max_rec==1 ? 1 : disp_max_rec),    // only use record as scroller unit when it's a vertical browser with frame
		    total_rec_num, 1 );
}
//----------- End of function VBrowse::init_var ----------//


//--------- Begin of function VBrowse::open ---------//
//
// Open the list box
//
// [int] recNo = the initial record no.
//               -1 if keep current record no.
//               (default : 1, the top record)
//
// [int] newTotalRec - new total record count. This is usually given
//							  when in INFO_UPDATE mode.
//							  (default: no change)
//
void VBrowse::open(int recNo, int newTotalRec)
{
	state = 1;    // state = OPENED

	if( newTotalRec >= 0 )
	{
		if( total_rec_num < x_max_rec * y_max_rec || newTotalRec < x_max_rec * y_max_rec )
			init_var(newTotalRec, recNo); // recalculate disp_max_rec
		else
			total_rec_num = newTotalRec;
	}

	if( recNo > 0 )      // keep current record no.
		rec_no = recNo;

	if( rec_no<1 )
		rec_no=1;

	if( rec_no > total_rec_num )
		rec_no = total_rec_num;

	//------- set the top recno ----------//

	if( !newTotalRec )
	{
		top_rec_no = rec_no - disp_max_rec/2;
		top_rec_no = MAX(1,top_rec_no);
	}
	else	// keep the original top_rec_no
	{
		if( rec_no > top_rec_no+disp_max_rec-1 )
			top_rec_no = rec_no-disp_max_rec+1;

		if( rec_no < top_rec_no )
			top_rec_no = rec_no;
	}

	if( top_rec_no+disp_max_rec-1 > total_rec_num )      // if there is an empty area at the lower part of the browser
	{
		top_rec_no = total_rec_num-disp_max_rec+1;
		top_rec_no = MAX(top_rec_no,1);
	}

	//------- display all records -------//

	disp_all();
}
//--------- End of function VBrowse::open ---------//


//--------- Begin of function VBrowse::close ---------//
//
// Close the list box, disable scroll bar and list content
//
void VBrowse::close()
{
	state = 0;   // state = CLOSED

	//-------- paint the closed outlook of the list box -------//

	Vga::active_buf->d3_panel_up( x1+1, y1+1, x2-SCROLL_BAR_WIDTH-3, y2-1  );    // the place a large block on the list content box
}
//--------- End of function VBrowse::close ---------//


//--------- Begin of function VBrowse::paint ---------//
//
// Display the movie list box
//
void VBrowse::paint()
{
	Vga::active_buf->d3_panel_down( x1, y1, x2-SCROLL_BAR_WIDTH-3, y2 );  // the list content box

	Vga::active_buf->bar( x2-SCROLL_BAR_WIDTH-2, y1, x2-SCROLL_BAR_WIDTH-1, y2, Vga::active_buf->color_up );

	scroll_bar.paint();
}
//--------- End of function VBrowse::paint ---------//


//------- Begin of function VBrowse::refresh ---------//
//
// Reset the no. of record and current record pointer and cause displaying
//
// [int] newRecNo      = new current record no.
//                       (default : -1, no change)
// [int] newTotalRec   = new total no. of records
//                       (default : -1, no change)
//
void VBrowse::refresh(int newRecNo, int newTotalRec)
{
   disp_one(rec_no,CLEAR_HIGH);

   if( newRecNo>=0 || newTotalRec>=0 )  // if current recno and total recno has no change, don't do the recalculation
   {
      if( newRecNo >= 1 )
         rec_no = newRecNo;

      if( newTotalRec != -1 && rec_no > newTotalRec )
         rec_no = newTotalRec;

      if( rec_no==0 && newTotalRec>0 )     // if originally rec_no==0, and total_rec is also 0, when the new total_rec is > 0, the rec_no should also be adjusted
         rec_no=1;

      init_var(newTotalRec,rec_no);
   }

   disp_all();
}
//---------- End of function VBrowse::refresh ----------//


//------- Begin of function VBrowse::update ---------//
//
// Yield all records which currently display in the vbrowse
//
void VBrowse::update()
{
	int recNo, x, y;

	if( none_record )
		return;

	for( recNo=top_rec_no ; recNo<=total_rec_num && recNo<top_rec_no+disp_max_rec ; recNo++ )
	{
		y = iy1 + (recNo-top_rec_no)/x_max_rec * (rec_height+rec_y_space);
		x = ix1 + (recNo-top_rec_no)%x_max_rec * (rec_width+rec_x_space);

		disp_rec( recNo, x, y, INFO_UPDATE );        // call user defined function
	}
}
//---------- End of function VBrowse::update ----------//


//--------- Begin of function VBrowse::detect ------//
//
// Detect for any mouse action and carry out relative response
//
// Return : <int> >0 if the new current record number
//                =0 if no action
//
int VBrowse::detect()
{
   int recNo=0, rc;

   double_click = 0;

   if( state == 0 || none_record )             // closed
      return 0;

   //-- Detect pulling on record if the browser is with frame --//
	
	if( detect_pull_flag && disp_frame && x_max_rec==1 )  // only pull when the vbrowser is vertical, horizontal is technically possible, but not used practically
	{
		if( detect_pull() )
			return rec_no;
	}

	//--------- Detect pressing on record if disp_frame is 1 ---------//

	if( disp_frame )
	{
		if( mouse.any_click(ix1, iy1, ix2, iy2) )
		{
			recNo = top_rec_no +
					  (mouse.click_y()-iy1) / (rec_height+rec_y_space) * x_max_rec +
					  (mouse.click_x()-ix1) / (rec_width +rec_x_space);

			if( detect_pull_flag )
				press_record = 1;
		}

		if( mouse.press_area(ix1, iy1, ix2, iy2) )
		{
			recNo = top_rec_no +
				(mouse.cur_y-iy1) / (rec_height+rec_y_space) * x_max_rec +
				(mouse.cur_x-ix1) / (rec_width +rec_x_space);

			if( detect_pull_flag )
				press_record = 1;
		}

		if( recNo>0 && recNo<=top_rec_no-1+disp_max_rec )   // if it is not in the empty browser area
		{
			if( mouse.click_count()==2 )   // double clicked
				double_click = 1;

			if( recNo != rec_no )            // if user point to a new record
			{
				disp_one(rec_no, CLEAR_HIGH);
				disp_one(recNo , DISP_HIGH );    // 2 means display record content only

				rec_no = recNo;
				return rec_no;
			}

			if( double_click )     // even if current record no. doesn't change, return recno if double clicked
				return rec_no;
		}
	}
	else //---- when disp_frame is 0, fast scrolling is enabled ----//
	{
		if( mouse.any_click( ix1, iy1, ix2, iy2 ) )
		{
			int newTopRec;

			if( mouse.click_y() < iy1+ ( (iy2-iy1) >> 1 ) )
				newTopRec = scroll_bar.page_up();   // page up
			else
				newTopRec = scroll_bar.page_down(); // page down

			if( top_rec_no != newTopRec )
			{
				double_click = mouse.click_count()==2;      // double clicked

				rec_no = top_rec_no = newTopRec;
				disp_all();

				return rec_no;
			}
		}
	}

	//------- detect for pressing on scroll bar -----//

	if( ( rc=scroll_bar.detect() ) > 0 )
	{
		disp_one(rec_no,CLEAR_HIGH);
		rec_no = rc;

		if( disp_frame && x_max_rec==1 ) // only use record as scroller unit when it's a vertical browser with frame, refer to scroll_bar.init() in VBrowse::init_var()
		{
			if( rec_no < top_rec_no )
			{
				top_rec_no = rec_no;
				disp_all();
			}

			else if( rec_no >= top_rec_no + disp_max_rec )
			{
				top_rec_no = rec_no - disp_max_rec + 1;
				if( top_rec_no < 1 )
					top_rec_no = 1;
				disp_all();
			}

			else
				disp_one(rec_no,DISP_HIGH);
		}
		else
		{
			top_rec_no = rc;
			disp_all();
		}

		return rec_no;
	}

	return 0;
}
//----------- End of function VBrowse::detect -------//


//---------- Begin of function VBrowse::detect_pull ---------//
//
// Detect pulling on record if it is a vertical browser with frame.
//
int VBrowse::detect_pull()
{
	if( press_record )                  // test whether user continue pressing it
	{
		press_record = mouse.left_press;
	}

	if( !( mouse.left_press || mouse.click_count() ) )
		return 0;

	if( !press_record )
      return 0;

   //--------- detect for pulling record upwards ---------//

   if( ( x_max_rec==1 && mouse.press_area( x1, 0 , x2, y1 ) ) ||
       ( y_max_rec==1 && mouse.press_area(  0, y1, x1, y2 ) ) )
   {
      if( rec_no > 1 )
      {
         disp_one(rec_no, CLEAR_HIGH);

         rec_no--;
         if( rec_no < top_rec_no )
            top_rec_no--;

         disp_all();
         return 1;
      }
   }

   //---- detect for pressing down button or pulling record downwards ---//

   if( ( x_max_rec==1 && mouse.press_area( x1, y2, x2, VGA_HEIGHT-1 ) ) ||
       ( y_max_rec==1 && mouse.press_area( x2, y1, VGA_WIDTH-1, y2  ) ) )
   {
		if( rec_no < total_rec_num )
      {
			disp_one(rec_no, CLEAR_HIGH);

         rec_no++;
         if( rec_no >= top_rec_no+disp_max_rec )
            top_rec_no++;

         disp_all();
         return 1;
      }
   }

   return 0;
}
//---------- End of function VBrowse::detect_pull ---------//


//--------- Begin of function VBrowse::detect_right ------//
//
// Detect for pressing right mouse button on the record
//
// Return : <int> >0 if the record number
//                =0 if no action
//
int VBrowse::detect_right()
{
   if( state == 0 || none_record )             // closed
      return 0;

   if( mouse.single_click( ix1, iy1, ix2, iy2, 1 ) )    // 1-right mouse button
   {
      int recNo;

      recNo = top_rec_no +
              (mouse.click_y(1)-iy1) / (rec_height+rec_y_space) * x_max_rec +
              (mouse.click_x(1)-ix1) / (rec_width +rec_x_space);

      if( recNo <= top_rec_no-1+disp_max_rec )
      {
         disp_one(rec_no,CLEAR_HIGH);
         disp_one(recNo, DISP_HIGH);    // 2 means display record content only

         rec_no = recNo;
         return recNo;
      }
   }

   return 0;
}
//----------- End of function VBrowse::detect_right -------//


//-------- Begin of function VBrowse::disp_all ----------//
//
// Display all records on screen, highlight cur_rec_no
//
void VBrowse::disp_all()
{
   int recNo;

	Vga::active_buf->bar( ix1, iy1, ix2, iy2, VBROWSE_COLOR_BACK ); // clear background

	int scrollRecno = (disp_frame && x_max_rec==1) ? rec_no : top_rec_no;

   scroll_bar.refresh( scrollRecno, 1, disp_max_rec,
		       (disp_frame && x_max_rec==1 ? 1 : disp_max_rec),
		       total_rec_num );

   for( recNo=top_rec_no ; recNo<=total_rec_num && recNo<top_rec_no+disp_max_rec ; recNo++ )
   {
      disp_one( recNo, DISP_REC );

      if( recNo == rec_no )
         disp_one( recNo, DISP_HIGH );
   }
}
//--------- End of function VBrowse::disp_all ----------//


//-------- Begin of function VBrowse::disp_one ----------//
//
// <int> recNo     = the no. of the record to display
// <int> putType   = DISP_REC  - display record
//                   DISP_HIGH - highlight rect
//                   CLEAR_HIGH- clear highlight rect
//
void VBrowse::disp_one(int recNo, int dispType)
{
	if( none_record )
		return;

	int x,y;

	y = iy1 + (recNo-top_rec_no)/x_max_rec * (rec_height+rec_y_space);
	x = ix1 + (recNo-top_rec_no)%x_max_rec * (rec_width+rec_x_space);

	//---- put a outline rect around the record if it is highlight ---//

	if( disp_frame && dispType == CLEAR_HIGH )
		Vga::active_buf->rect( x-2, y-2, x+rec_width+1, y+rec_height+1, 2, VBROWSE_COLOR_BACK );

	if( dispType == DISP_REC )
		disp_rec( recNo, x, y, INFO_REPAINT );  // call user defined function

   if( disp_frame && dispType == DISP_HIGH )
   {
		Vga::active_buf->rect( x-2, y-2, x+rec_width+1, y+rec_height+1, 2, VBROWSE_COLOR_HIGH );

		int scrollRecno = (disp_frame && x_max_rec==1) ? recNo : top_rec_no;

		scroll_bar.refresh( scrollRecno, 0, disp_max_rec,
			 (disp_frame && x_max_rec==1 ? 1 : disp_max_rec),
					total_rec_num );
	}
}
//--------- End of function VBrowse::disp_one ----------//


//-------- Begin of function VBrowse::disp_rec --------//
//
void VBrowse::disp_rec(int recNo, int x, int y, int refreshFlag)
{
	(*disp_rec_func)( recNo, x, y, refreshFlag );  // call user defined function
}
//----------- End of function VBrowse::disp_rec -----------//
