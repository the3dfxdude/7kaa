//Filename    : OSCROLL.CPP
//Description : Object scroll bar

#include <ALL.H>
#include <KEY.H>
#include <OSYS.H>
#include <OMOUSE.H>
#include <OVGA.H>
#include <OINFO.H>
#include <OIMGRES.H>
#include <OSCROLL.H>


//------- Define constant -----------//

#define PRESS_INT_TIME  200      // Press interval time, prevent too fast
											// and uncontrollable scrolling, 200ms

#define MINI_INDICATOR_HEIGHT 10 // Minimum height of scroll bar indicator


//------ Begin of function ScrollBar::init -------//
//
// <int> type = 1-Vertical  scroll bar  (VERTICAL)
//              2-Horizontal scroll bar (HORIZONTAL)
//
// <int> x1,y1,x2,y2 = the coordination of the scroll bar
//
// <int> pageSkipRec = no. of records when page skip up or down
// <int> dispMaxRec  = no. of records can be displayed on a page
// <int> totalRec    = total no. of records
// [int] pageType    = yes or no, refered when down button is pressed
//                     (default : 1)
// [int] ifFlag   = if call vga.d3_panel_down instead of VgaBuf::d3_panel_down
//						  (default: 0)
// [int] vgaFrontOnly = do all the bitmap processing on the front buffer only
//								(default: 0)
void ScrollBar::init(int itype, int ix1, int iy1, int ix2, int iy2,
							int pageSkipRec, int dispMaxRec, int totalRec, int pageType, int ifFlag, int vgaFrontOnly)
{
	x1 = ix1;
	y1 = iy1;
	x2 = ix2;
	y2 = iy2;

	type = itype;

	skip_rec_num  = pageSkipRec;
	disp_rec_num  = dispMaxRec;
	total_rec_num = totalRec;
	page_type     = pageType;
	if_flag		  = ifFlag;
   vga_front_only = vgaFrontOnly;

	if( total_rec_num==0 )       // at must at least be 1, otherwise divided by zero error will appear
		total_rec_num=1;

	if( type == VERTICAL )
		slot_height = y2-y1-28;
	else
		slot_height = x2-x1-28;

	indicator_height = slot_height * disp_rec_num / total_rec_num;

	if( indicator_height < MINI_INDICATOR_HEIGHT )
		indicator_height = MINI_INDICATOR_HEIGHT;

	next_press_time = 0;
}
//------- End of function ScrollBar::init -------//


//------- Begin of function ScrollBar::paint -------//
//
// [int] topRecNo = the top record no. (default : 1)
//
void ScrollBar::paint(int topRecNo)
{
	err_when( type == -1 );      // haven't been init()

	if( if_flag )
	{
		vga.d3_panel_down( x1, y1, x2, y2, vga_front_only );      // scroll panel

		//-------- cursor button --------//

		if( type == VERTICAL )
		{
			vga.d3_panel_up( x1+2, y1+2 , x2-2, y1+13, vga_front_only);  // up button
			vga.d3_panel_up( x1+2, y2-13, x2-2, y2-2 , vga_front_only);  // down button
		}
		else
		{
			vga.d3_panel_up( x1+2 , y1+2, x1+13, y2-2, vga_front_only);  // left button
			vga.d3_panel_up( x2-13, y1+2, x2-2, y2-2 , vga_front_only);  // right button
		}
	}
	else
	{
		Vga::active_buf->d3_panel_down( x1, y1, x2, y2 );      // scroll panel

		//-------- cursor button --------//

		if( type == VERTICAL )
		{
			Vga::active_buf->d3_panel_up( x1+2, y1+2 , x2-2, y1+13);  // up button
			Vga::active_buf->d3_panel_up( x1+2, y2-13, x2-2, y2-2 );  // down button
		}
		else
		{
			Vga::active_buf->d3_panel_up( x1+2 , y1+2, x1+13, y2-2);  // left button
			Vga::active_buf->d3_panel_up( x2-13, y1+2, x2-2, y2-2 );  // right button
		}
	}

	//--------- display cursor bitmap ----------//

	if( type == VERTICAL )
	{
		image_icon.put_front( x1+4, y1+4 , "SCROLL_U" );
		image_icon.put_front( x1+4, y2-11, "SCROLL_D" );
	}
	else
	{
		image_icon.put_front( x1+4 , y1+4, "SCROLL_L" );
		image_icon.put_front( x2-11, y1+4, "SCROLL_R" );
	}
}
//--------- End of function ScrollBar::paint ------------//


//------- Begin of function ScrollBar::refresh -------//
//
// <int> topRecNo     = the new top record no.
// [int] forceRefresh = in normal case, only refresh when new top record
//                      no. <> current top record no.
//                      but forceRefresh will force refreshing anyway
//                      ( default : 0)
//
// [int] pageSkipRec = no. of records when page skip up or down
// [int] dispMaxRec  = no. of records can be displayed on a page
// [int] totalRec    = total no. of records
//		       ( default: no change for the above 3 parameters )
//
void ScrollBar::refresh(int topRecNo, int forceRefresh, int pageSkipRec, int dispMaxRec, int totalRec)
{
	int oldTotalRec = total_rec_num;

	err_when( type == -1 );      // haven't been init()

   if( pageSkipRec >= 0 )
      skip_rec_num  = pageSkipRec;

   if( dispMaxRec >= 0 )
      disp_rec_num  = dispMaxRec;

   if( totalRec >= 0 )
		total_rec_num = totalRec;

   if( total_rec_num==0 )       // at must at least be 1, otherwise divided by zero error will appear
		total_rec_num=1;

   if( type == VERTICAL )
      slot_height = y2-y1-28;
   else
      slot_height = x2-x1-28;

	indicator_height = slot_height * disp_rec_num / total_rec_num;

	if( indicator_height < MINI_INDICATOR_HEIGHT )
		indicator_height = MINI_INDICATOR_HEIGHT;

	//------------- refresh display ----------------//

	int newY;

	if( (newY=rec_to_y(topRecNo)) != indicator_y ||
		 total_rec_num != oldTotalRec || forceRefresh )
	{
		indicator_y = newY;

		if( if_flag )
		{
			if( type == VERTICAL )
			{
				if( !vga.use_back_buf && !vga_front_only )
					vga.blt_buf( x1, y1+12, x2, y2-13, 0 );

				Vga::active_buf->draw_d3_up_border( x1+2, indicator_y, x2-2, indicator_y+indicator_height-1 );
			}
			else
			{
				if( !vga.use_back_buf && !vga_front_only )
					vga.blt_buf( x1+12, y1, x2-13, y2, 0 );

				Vga::active_buf->draw_d3_up_border( indicator_y, y1+2, indicator_y+indicator_height-1, y2-2 );
			}
		}
		else
		{
			if( type == VERTICAL )
			{
				Vga::active_buf->d3_panel_down_clear( x1, y1+12, x2, y2-13 );
				Vga::active_buf->d3_panel_up( x1+2, indicator_y, x2-2, indicator_y+indicator_height-1 );
			}
			else
			{
				Vga::active_buf->d3_panel_down_clear( x1+12, y1, x2-13, y2 );
				Vga::active_buf->d3_panel_up( indicator_y, y1+2, indicator_y+indicator_height-1, y2-2 );
			}
		}

      top_rec_no = topRecNo;
   }
}
//--------- End of function ScrollBar::refresh ------------//


//------- Begin of function ScrollBar::detect -------//
//
// Return : =0 if not pressed on scroll bar
//          >0 the top record no.
//
int ScrollBar::detect()
{
   int pos, topRecNo=top_rec_no;

   top_rec_flag = 1;   // whether the recno returned by detect()
                       // is the top_recno or only current recno, is returned by detect_is_top()

   //----------- detect for pressing up button --------------//

	if( type==VERTICAL   && mouse.any_click( x1+2, y1+2, x2-2 , y1+13 ) ||
		 type==HORIZONTAL && mouse.any_click( x1+2, y1+2, x1+13, y2-2  ) )
	{
		if( --topRecNo < 1 )
			 topRecNo = 1;

		top_rec_flag=0;   // return by detect_is_top()
	}

	//----------- detect for pressing down button ------------//

	if( type==VERTICAL   && mouse.any_click( x1+2, y2-13, x2-2, y2-2 ) ||
		 type==HORIZONTAL && mouse.any_click( x2-13,y1+2 , x2-2, y2-2 ) )
	{
		 if( page_type )  // top record can't be the last record
		 {
			  if( ++topRecNo > total_rec_num - disp_rec_num+1 )
				  topRecNo = total_rec_num - disp_rec_num + 1;
		 }
		 else             // top record can be the last record
		 {
			  if( ++topRecNo > total_rec_num )
				  topRecNo = total_rec_num;
		 }

		 top_rec_flag=0;
	}

	//------ detect for pressing on scroll slot but not indicator -------//

	if( type==VERTICAL   && mouse.any_click( x1+2 , y1+14, x2-2 , y2-14 ) ||
		 type==HORIZONTAL && mouse.any_click( x1+14, y1+2 , x2-14, y2-2  ) ||
		 mouse.key_code == KEY_PGUP || mouse.key_code == KEY_PGDN )
	{
		if( type==VERTICAL )
			pos = mouse.click_y();
		else
			pos = mouse.click_x();

		if( pos < indicator_y || mouse.key_code == KEY_PGUP )   // page up
		{
			if( ( topRecNo -= skip_rec_num ) < 1 )
				topRecNo = 1;
		}

		else if( pos >= indicator_y+indicator_height || mouse.key_code == KEY_PGDN )   // page down
		{
			if( (topRecNo += skip_rec_num) > total_rec_num - disp_rec_num+1 )
			{
				topRecNo = total_rec_num - disp_rec_num + 1;

				if ( topRecNo < 1 )
					topRecNo = 1;
			}
		}
   }

   //--------- check if display all needed -----------//

   if( topRecNo != top_rec_no )
   {
		if( m.get_time() >= next_press_time )  // prevent too fast scrolling
		{
			refresh(topRecNo,1);

			next_press_time = m.get_time() + PRESS_INT_TIME;

			return topRecNo;
		}
   }


   //-------- detect for pulling indicator ------------//

   if( type==VERTICAL   && mouse.any_click( x1+2 , indicator_y, x2-2, indicator_y+indicator_height-1 ) ||
       type==HORIZONTAL && mouse.any_click( indicator_y, y1+2, indicator_y+indicator_height-1, y2-2  ) )
   {
      int lastMouseY = -1;

      while( mouse.left_press )
		{
			sys.yield();

			if( type==VERTICAL )
         {
				if( mouse.left_press && lastMouseY != mouse.cur_y )
            {
					indicator_y = mouse.cur_y-indicator_height/2;

               if( indicator_y < y1+14 )
                  indicator_y = y1+14;

               if( indicator_y > y2-14-indicator_height )
						indicator_y = y2-14-indicator_height;

					if( if_flag )
					{
						if( !vga.use_back_buf && !vga_front_only )
							vga.blt_buf( x1, y1+12, x2, y2-13, 0 );

						Vga::active_buf->draw_d3_up_border( x1+2, indicator_y, x2-2, indicator_y+indicator_height-1 );
					}
					else
					{
						Vga::active_buf->d3_panel_down_clear( x1, y1+12, x2, y2-13 );      // scroll panel
						Vga::active_buf->d3_panel_down( x1+2, indicator_y, x2-2, indicator_y+indicator_height-1 );
					}

					lastMouseY = mouse.cur_y;
				}
			}
			else
			{
				if( mouse.left_press && lastMouseY != mouse.cur_x )
				{
					indicator_y = mouse.cur_x-indicator_height/2;

					if( indicator_y < x1+14 )
						indicator_y = x1+14;

					if( indicator_y > x2-14-indicator_height )
						indicator_y = x2-14-indicator_height;

					if( if_flag )
					{
						if( !vga.use_back_buf && !vga_front_only )
							vga.blt_buf( x1+12, y1, x2-13, y2, 0 );

						Vga::active_buf->draw_d3_up_border( indicator_y, y1+2, indicator_y+indicator_height-1, y2-2 );
					}
					else
					{
						Vga::active_buf->d3_panel_down_clear( x1+12, y1, x2-13, y2 );
						Vga::active_buf->d3_panel_down ( indicator_y, y1+2, indicator_y+indicator_height-1, y2-2 );
					}

					lastMouseY = mouse.cur_x;
				}
			}
		}

		refresh(y_to_rec(indicator_y),1);

		return top_rec_no;
	}

	//-----------------------------------------------------------//

	return 0;
}
//--------- End of function ScrollBar::detect ------------//


//--------- Begin of function ScrollBar::page_up -------//
//
// Asked by client function to page up
//
// [int] skipLess - no. of record number should skip less
//                  (default : 0)
//                  (e.g. when scrolling text, want the last line of
//                   previous page be the first line of current page,
//                   then pass 1 as skipLess)
//
// return : <int> the top recno after scrolling
//
int ScrollBar::page_up(int skipLess)
{
   if( top_rec_no > 1 )
   {
      if( (top_rec_no-=skip_rec_num-skipLess) < 1 )
         top_rec_no = 1;

      refresh(top_rec_no,1);
	}

	return top_rec_no;
}
//--------- End of function ScrollBar::page_up ----------//


//--------- Begin of function ScrollBar::page_down -------//
//
// Asked by client function to page up
//
// [int] skipLess - no. of record number should skip less
//                  (default : 0)
//                  (e.g. when scrolling text, want the last line of
//                   previous page be the first line of current page,
//                   then pass 1 as skipLess)
//
// return : <int> the top recno after scrolling
//
int ScrollBar::page_down(int skipLess)
{
   if( top_rec_no < total_rec_num - disp_rec_num + 1 )
   {
      if( (top_rec_no += skip_rec_num-skipLess) > total_rec_num - disp_rec_num + 1 )
      {
         top_rec_no = total_rec_num - disp_rec_num + 1;

         if( top_rec_no < 1 )
            top_rec_no = 1;
      }

		refresh(top_rec_no,1);
	}

   return top_rec_no;
}
//--------- End of function ScrollBar::page_down ----------//


//--------- Begin of function ScrollBar::detect_top_rec -------//
//
// return whether the recno returned by detect()
// is the top_recno or only current recno
//
int ScrollBar::detect_top_rec()
{
   if( top_rec_flag )
   {
      if( top_rec_no > total_rec_num - disp_rec_num+1 )
         return total_rec_num - disp_rec_num + 1;
      else
         return top_rec_no;
   }
   else
      return 0;
}
//--------- End of function ScrollBar::detect_top_rec ----------//


//--------- Begin of function ScrollBar::rec_to_y -------//
//
// Convert from record no. to y position of scroll bar indicator
//
int ScrollBar::rec_to_y(int recNo)
{
   int t;

   if( total_rec_num <= 1 || disp_rec_num == total_rec_num )
      t = 0;
   else
   {
      t = (slot_height-indicator_height) * (recNo-1) / (total_rec_num-disp_rec_num);

		if( t+indicator_height > slot_height )
			t = slot_height-indicator_height;
	}

	if( type == VERTICAL )
		return y1 + 14 + t;
	else
		return x1 + 14 + t;
}
//---------- End of function ScrollBar::rec_to_y ----------//



//--------- Begin of function ScrollBar::y_to_rec -------//
//
// Convert from y position of scroll bar indicator to record no.
//
int ScrollBar::y_to_rec(int y)
{
  int t;

  if( total_rec_num == 0 )
     return 0;

  if( slot_height-indicator_height == 0 || disp_rec_num == total_rec_num )
	  return 1;

  if( type == VERTICAL )
     t = y1;
  else
     t = x1;

  return (y-t-14) * (total_rec_num-disp_rec_num) / (slot_height-indicator_height) + 1;
}
//---------- End of function ScrollBar::y_to_rec ----------//


