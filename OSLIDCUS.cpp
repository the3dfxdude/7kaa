// Filename    : OSLIDCUS.CPP
// Description : custom slide bar


#include <OMOUSE.h>
#include <OSLIDCUS.h>

// ------- begin of inline function bound ---------//

inline int bound(int v, int lowerLimit, int upperLimit)
{
	err_when( lowerLimit > upperLimit );
	return v<lowerLimit ? lowerLimit : (v>upperLimit ? upperLimit : v);
}
// ------- end of inline function bound ---------//


// ------- begin of function SlideBar::init_scroll ---------//

void SlideBar::init_scroll(short x1, short y1, short x2, short y2, int viewSize, SlideBarFP dispFunc)
{
	scrn_x1 = x1;
	scrn_y1 = y1;
	scrn_x2 = x2;
	scrn_y2 = y2;
	scrn_bar_width = 0;
	scroll_type = 1;

	view_size = viewSize;
	disp_func = dispFunc;
	drag_flag = 0;
}
// ------- end of function SlideBar::init_scroll ---------//


// ------- begin of function SlideBar::init_slide ---------//
// set view_size to 0
void SlideBar::init_slide(short x1, short y1, short x2, short y2, short barWidth, SlideBarFP dispFunc)
{
	scrn_x1 = x1;
	scrn_y1 = y1;
	scrn_x2 = x2;
	scrn_y2 = y2;
	scrn_bar_width = barWidth;
	scroll_type = 0;

	view_size = 0;
	disp_func = dispFunc;
	drag_flag = 0;
}
// ------- end of function SlideBar::init_scroll ---------//


void SlideBar::set(int minRecno, int maxRecno, int viewRecno)
{
	min_recno = minRecno;
	max_recno = maxRecno;
	view_recno = viewRecno;

	// empty if (max_recno - min_recno + scroll_type == 0)
	err_when( max_recno - min_recno + scroll_type < 0);
}

int SlideBar::set_view_recno(int viewRecno)
{
	view_recno = viewRecno;
	if( view_recno > max_recno - view_size + scroll_type )
		view_recno = max_recno - view_size + scroll_type;
	if( view_recno < min_recno )
		view_recno = min_recno;
	return view_recno;
}

void SlideBar::set_min_recno(int minRecno)
{
	min_recno = minRecno;
	err_when( max_recno - min_recno + scroll_type < 0);
	if( view_recno > max_recno - view_size + scroll_type )
		view_recno = max_recno - view_size + scroll_type;
	if( view_recno < min_recno )
		view_recno = min_recno;
}

void SlideBar::set_max_recno(int maxRecno)
{
	max_recno = maxRecno;
	err_when( max_recno - min_recno + scroll_type < 0);
	if( view_recno > max_recno - view_size + scroll_type )
		view_recno = max_recno - view_size + scroll_type;
	if( view_recno < min_recno )
		view_recno = min_recno;
}


int SlideBar::is_empty()
{
	return max_recno - min_recno + scroll_type <= 0;
}

// [int] disablePaint : don't call paint (default = 0)
// return 2 if the view_recno does not change
int SlideBar::detect()
{
	int retValue = 0;

	if( !drag_flag )
	{
		short rectLeft = rect_left();
		short rectRight = rect_right();

		if( mouse.single_click(scrn_x1, scrn_y1, scrn_x2, scrn_y2) )
		{
			short clickX = mouse.click_x();
			if( clickX >= rectLeft && clickX <= rectRight )
			{
				drag_flag = 1;
				drag_cur_x = clickX;
				drag_rect_left = rectLeft;
				drag_last_view_recno = view_recno;
				retValue = 2;
			}
			else
			{
				// suppose click position will be the center of the button
				short newScrollButtonX1 = clickX - (rectRight - rectLeft)/2;
				view_recno = calc_view_recno(newScrollButtonX1);
				drag_flag = 1;
				drag_cur_x = clickX;
				drag_rect_left = rect_left();		// rect_left() != rectLeft
				drag_last_view_recno = view_recno;
			}
			retValue = 1;
		}
	}
	else
	{
		// detect draggin on the scroll bar, don't detect any other buttons
		if( !mouse.left_press )
		{
			drag_flag = 0;
		}

		int oldValue = view_recno;
		int dragX = mouse.cur_x;
		int newScrollButtonX1 = dragX - drag_cur_x + drag_rect_left;
		
		// control the boundary of scrollButtonY1
		drag_last_view_recno = view_recno;
		view_recno = calc_view_recno(newScrollButtonX1);


		retValue = drag_last_view_recno == view_recno ? 2 : 1;
	}

	if( retValue == 1 )
		paint();
	return retValue;
}


void SlideBar::paint()
{
	(*disp_func)(this, 1);
}


void SlideBar::paint(int newViewRecno)
{
	set_view_recno(newViewRecno);
	paint();
	//view_recno = bound(newViewRecno, min_recno, max_recno);
	//(*disp_func)(this, 1);
}

// instruction for disp_func
//
// for scroll bar style, 
// draw a rectangle from rect_left() to rect_right()
// display from min_recno to max_recno
//
// for slider style,
// draw the slide button at (rect_left(), scrn_y2)

short SlideBar::rect_left()
{
	err_when( view_recno < min_recno );
	// err_when( view_recno > max_view_recno() );
	if( is_empty() )
		return scrn_x1;		// empty
	else
		return bound( scrn_x1 + (view_recno-min_recno)*(scrn_x2-scrn_x1+1-scrn_bar_width)/(max_recno-min_recno+scroll_type),
			scrn_x1, scrn_x2 );
}

short SlideBar::rect_right()
{
	// slide bar style need not call rect_right(), it should be rect_left() + scrn_bar_width - 1
	// as view_size = 0
	err_when( view_recno < min_recno );
	// err_when( view_recno > max_view_recno() );
	if( is_empty() )
		return scrn_x2;		// empty
	else
		return bound( scrn_x1+scrn_bar_width-1 + (view_recno+view_size-min_recno)*(scrn_x2-scrn_x1+1-scrn_bar_width)/(max_recno-min_recno+scroll_type),
			scrn_x1, scrn_x2 );
}

// reverse function, pass the proposed rect_left to return the corresponding view_recno
int SlideBar::calc_view_recno(short scrnX)
{
	int nomin = (scrnX-scrn_x1)*(max_recno-min_recno+scroll_type);
	int demon = (scrn_x2-scrn_x1+1-scrn_bar_width);
	int roundDivision = (2*nomin+demon) / demon / 2;

	return bound( min_recno + roundDivision, min_recno, max_view_recno() );
}

int SlideBar::max_view_recno()
{
	if( is_empty() )
		return min_recno;
	else
		return bound( max_recno - view_size + scroll_type, min_recno, max_recno);
}


// .....................................................//



// ------- begin of function SlideVBar::init_scroll ---------//

void SlideVBar::init_scroll(short x1, short y1, short x2, short y2, int viewSize, SlideVBarFP dispFunc)
{
	scrn_x1 = x1;
	scrn_y1 = y1;
	scrn_x2 = x2;
	scrn_y2 = y2;
	scrn_bar_height = 0;
	scroll_type = 1;

	view_size = viewSize;
	disp_func = dispFunc;
	drag_flag = 0;
}
// ------- end of function SlideVBar::init_scroll ---------//


// ------- begin of function SlideVBar::init_slide ---------//
// set view_size to 0
void SlideVBar::init_slide(short x1, short y1, short x2, short y2, short barHeight, SlideVBarFP dispFunc)
{
	scrn_x1 = x1;
	scrn_y1 = y1;
	scrn_x2 = x2;
	scrn_y2 = y2;
	scrn_bar_height = barHeight;
	scroll_type = 0;

	view_size = 0;
	disp_func = dispFunc;
	drag_flag = 0;
}
// ------- end of function SlideVBar::init_scroll ---------//


void SlideVBar::set(int minRecno, int maxRecno, int viewRecno)
{
	min_recno = minRecno;
	max_recno = maxRecno;
	view_recno = viewRecno;

	// empty if (max_recno - min_recno + scroll_type == 0)
	err_when( max_recno - min_recno + scroll_type < 0);
}

int SlideVBar::set_view_recno(int viewRecno)
{
	view_recno = viewRecno;
	if( view_recno > max_recno - view_size + scroll_type )
		view_recno = max_recno - view_size + scroll_type;
	if( view_recno < min_recno )
		view_recno = min_recno;
	return view_recno;
}

void SlideVBar::set_min_recno(int minRecno)
{
	min_recno = minRecno;
	err_when( max_recno - min_recno + scroll_type < 0);
	if( view_recno > max_recno - view_size + scroll_type )
		view_recno = max_recno - view_size + scroll_type;
	if( view_recno < min_recno )
		view_recno = min_recno;
}

void SlideVBar::set_max_recno(int maxRecno)
{
	max_recno = maxRecno;
	err_when( max_recno - min_recno + scroll_type < 0);
	if( view_recno > max_recno - view_size + scroll_type )
		view_recno = max_recno - view_size + scroll_type;
	if( view_recno < min_recno )
		view_recno = min_recno;
}


int SlideVBar::is_empty()
{
	return max_recno - min_recno + scroll_type <= 0;
}

// [int] disablePaint : don't call paint (default = 0)
// return 2 if the view_recno does not change
int SlideVBar::detect()
{
	int retValue = 0;

	if( !drag_flag )
	{
		short rectTop = rect_top();
		short rectBottom = rect_bottom();

		if( mouse.single_click(scrn_x1, scrn_y1, scrn_x2, scrn_y2) )
		{
			short clickY = mouse.click_y();
			if( clickY >= rectTop && clickY <= rectBottom )
			{
				drag_flag = 1;
				drag_cur_y = clickY;
				drag_rect_top = rectTop;
				drag_last_view_recno = view_recno;
				retValue = 2;
			}
			else
			{
				// suppose click position will be the center of the button
				short newScrollButtonY1 = clickY - (rectBottom - rectTop)/2;
				view_recno = calc_view_recno(newScrollButtonY1);
				drag_flag = 1;
				drag_cur_y = clickY;
				drag_rect_top = rect_top();		// rect_top() != rectTop
				drag_last_view_recno = view_recno;
			}
			retValue = 1;
		}
	}
	else
	{
		// detect draggin on the scroll bar, don't detect any other buttons
		if( !mouse.left_press )
		{
			drag_flag = 0;
		}

		int oldValue = view_recno;
		int dragY = mouse.cur_y;
		int newScrollButtonY1 = dragY - drag_cur_y + drag_rect_top;
		
		// control the boundary of scrollButtonY1
		drag_last_view_recno = view_recno;
		view_recno = calc_view_recno(newScrollButtonY1);


		retValue = drag_last_view_recno == view_recno ? 2 : 1;
	}

	if( retValue == 1 )
		paint();
	return retValue;
}


void SlideVBar::paint()
{
	(*disp_func)(this, 1);
}


void SlideVBar::paint(int newViewRecno)
{
	set_view_recno(newViewRecno);
	paint();
	//view_recno = bound(newViewRecno, min_recno, max_recno);
	//(*disp_func)(this, 1);
}

// instruction for disp_func
//
// for scroll bar style, 
// draw a rectangle from rect_top() to rect_bottom()
// display from min_recno to max_recno
//
// for slider style,
// draw the slide button at (rect_top(), scrn_y2)

short SlideVBar::rect_top()
{
	err_when( view_recno < min_recno );
	// err_when( view_recno > max_view_recno() );
	if( is_empty() )
		return scrn_y1;		// empty
	else
		return bound( scrn_y1 + (view_recno-min_recno)*(scrn_y2-scrn_y1+1-scrn_bar_height)/(max_recno-min_recno+scroll_type),
			scrn_y1, scrn_y2 );
}

short SlideVBar::rect_bottom()
{
	// slide bar style need not call rect_bottom(), it should be rect_top() + scrn_bar_height - 1
	// as view_size = 0
	err_when( view_recno < min_recno );
	// err_when( view_recno > max_view_recno() );
	if( is_empty() )
		return scrn_y2;		// empty
	else
		return bound( scrn_y1+scrn_bar_height-1 + (view_recno+view_size-min_recno)*(scrn_y2-scrn_y1+1-scrn_bar_height)/(max_recno-min_recno+scroll_type),
			scrn_y1, scrn_y2 );
}

// reverse function, pass the proposed rect_top to return the corresponding view_recno
int SlideVBar::calc_view_recno(short scrnY)
{
	int nomin = (scrnY-scrn_y1)*(max_recno-min_recno+scroll_type);
	int demon = scrn_y2-scrn_y1+1-scrn_bar_height;
	int roundDivision = (2*nomin+demon) / demon / 2;

	return bound( min_recno + roundDivision, min_recno, max_view_recno() );
}

int SlideVBar::max_view_recno()
{
	if( is_empty() )
		return min_recno;		// empty
	else
		return bound( max_recno - view_size + scroll_type, min_recno, max_recno);
}
