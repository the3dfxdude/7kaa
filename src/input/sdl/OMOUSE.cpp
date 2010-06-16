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

//Filename    : OMOUSE.CPP
//Description : Mouse handling Object

#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OMOUSE2.h>
#include <KEY.h>
#include <OSYS.h>

#define VGA_UPDATE_BUF_SIZE	(100*100)
static int update_x1, update_y1, update_x2, update_y2;          // coordination of the last double-buffer update area

//--------- Start of MouseSDL::MouseSDL ---------//
//
MouseSDL::MouseSDL()
{
	handle_flicking = 0;
	cur_x = cur_y = 0;
	left_press = right_press = 0;
	skey_state = 0;
	bound_x1 = 0;
	bound_y1 = 0;
	bound_x2 = 0;
	bound_y2 = 0;
	event_skey_state = 0;
	has_mouse_event = 0;
	mouse_event_type = (MouseEventType)0;
	scan_code = 0;
	key_code = 0;
	vga_update_buf = NULL;
	memset(click_buffer, 0, sizeof(MouseClick) * 2);
}
//---------- End of MouseSDL::MouseSDL ---------//


//---------- Begin of MouseSDL::~MouseSDL --------//
//
// Deinitialize the mouse driver, reset event handler
//
MouseSDL::~MouseSDL()
{
	deinit();
}
//------------ End of MouseSDL::~MouseSDL --------//


//------------ Start of MouseSDL::init ------------//
//
void MouseSDL::init()
{
}
//------------- End of MouseSDL::init -------------//


//------------ Start of MouseSDL::deinit ------------//
//
void MouseSDL::deinit()
{
}
//------------- End of MouseSDL::deinit -------------//


//--------- Start of MouseSDL::hide -------//
//
// Suspend the mouse function, use resume() to resume to function
//
void MouseSDL::hide()
{
	mouse_cursor.hide_all_flag=1;

	mouse_cursor.process(cur_x, cur_y);
}
//---------- End of MouseSDL::hide --------//


//--------- Start of MouseSDL::show -------//
//
// Resume the mouse function which is previously hideed by hide()
//
void MouseSDL::show()
{
	mouse_cursor.hide_all_flag=0;

	mouse_cursor.process(cur_x, cur_y);
}
//---------- End of MouseSDL::show --------//


//--------- Begin of MouseSDL::hide_area ----------//
//
void MouseSDL::hide_area(int x1, int y1, int x2, int y2)
{
	mouse_cursor.hide_area_flag++;

	if( mouse_cursor.hide_area_flag!=1 )		// only process for the first call of hide_area()
		return;

	mouse_cursor.hide_x1 = x1;
	mouse_cursor.hide_y1 = y1;
	mouse_cursor.hide_x2 = x2;
	mouse_cursor.hide_y2 = y2;

	int curX = cur_x - mouse_cursor.hot_spot_x;
	int curY = cur_y - mouse_cursor.hot_spot_y;

	if( m.is_touch( x1, y1, x2, y2, curX, curY,
						 curX+mouse_cursor.icon_width-1,
						 curY+mouse_cursor.icon_height-1 ) )
	{
		if( handle_flicking )
		{
			update_x1 = MIN(x1, curX);
			update_y1 = MIN(y1, curY);
			update_x2 = MAX(x2, curX+mouse_cursor.icon_width-1);
			update_y2 = MAX(y2, curY+mouse_cursor.icon_height-1);

			update_x1 = MAX(0, update_x1);
			update_y1 = MAX(0, update_y1);
			update_x2 = MIN(VGA_WIDTH-1 , update_x2);
			update_y2 = MIN(VGA_HEIGHT-1, update_y2);

			err_when( (update_x2-update_x1+1) * (update_y2-update_y1+1) > VGA_UPDATE_BUF_SIZE );

			//---- save the update area of the back buf to a temp buffer ----//

			vga_back.read_bitmap( update_x1, update_y1, update_x2, update_y2, vga_update_buf );

			//--- copy the update area from the front buf to the back buf ---//

			IMGcopy( vga_back.buf_ptr(), vga_back.buf_pitch(), vga_front.buf_ptr(), vga_front.buf_pitch(),
				update_x1, update_y1, update_x2, update_y2 );

			//-- redirect the front VGA buffer pointer to the back VGA buffer --//

			vga_front.set_buf_ptr( vga_back.buf_ptr() );
		}

		//------ hide up the mouse cursor --------//

		mouse_cursor.process(cur_x, cur_y);
	}
}
//--------- End of MouseSDL::hide_area --------------//


//--------- Begin of MouseSDL::show_area ----------//
//
void MouseSDL::show_area()
{
	mouse_cursor.hide_area_flag--;

	if( mouse_cursor.hide_area_flag!=0 )		// only process for the first call of hide_area()
		return;

	int curX = cur_x - mouse_cursor.hot_spot_x;
	int curY = cur_y - mouse_cursor.hot_spot_y;

	if( m.is_touch( mouse_cursor.hide_x1, mouse_cursor.hide_y1,
						 mouse_cursor.hide_x2, mouse_cursor.hide_y2,
						 curX, curY, curX+mouse_cursor.icon_width-1,
						 curY+mouse_cursor.icon_height-1 ) )
	{
		//----- redisplay the mouse cursor ------//

		mouse_cursor.process(cur_x, cur_y);

		if( handle_flicking )
		{
			//--- copy the update area from the back buf to the front buf ---//

			IMGcopy( vga_front.buf_ptr(), vga_front.buf_pitch(), vga_back.buf_ptr(), vga_back.buf_pitch(), 
				update_x1, update_y1, update_x2, update_y2 );

			//--- restore the update area of the back buf with the temp buffer ---//

			vga_back.fast_put_bitmap( update_x1, update_y1, vga_update_buf );

			//--- restore the VGA front buffer's buf ptr ---//

			vga_front.set_default_buf_ptr();
		}
	}
}
//--------- End of MouseSDL::show_area --------------//


//--------- Start of MouseSDL::add_event ---------//
//
// Called by handler interrupt to procss the state
//
void MouseSDL::add_event(MouseEvent *mouseEvent)
{
}
//----------- End of MouseSDL::add_event ----------//


//--------- Start of MouseSDL::add_key_event ---------//
//
// Called by key handler to save the key pressed
//
void MouseSDL::add_key_event(unsigned scanCode, unsigned long timeStamp)
{
}
//----------- End of MouseSDL::add_key_event ----------//


//--------- Start of MouseSDL::get_event ---------//
//
// Get next event from the event buffer
//
// return : <int> 1 - event fetched from the event queue
//                0 - not event remained in the buffer
//
// to know what type of event :
// 1. check is_mouse_event() or is_key_event() 
// 2. if is_mouse_event(), check mouse_event_type
//			if( LEFT_BUTTON or LEFT_BUTTON_RELEASE, read click_buffer[LEFT_BUTTON]
//			if( RIGHT_BUTTON or RIGHT_BUTTON_RELEASE, read click_buffer[RIGHT_BUTTON]
// 3. if is_key_event(), check event_skey_state, scan_code and key_code 
//
int MouseSDL::get_event()
{
   return 1;
}
//----------- End of MouseSDL::get_event ----------//


//--------- Begin of MouseSDL::in_area ----------//
//
// <Real-time access>
//
// Detect whether mouse cursor is in the specified area
//
// <int> x1,y1,x2,y2 = the coordinations of the area
//
// Return : 1 - if the mouse cursor is in the area
//          0 - if not
//
int MouseSDL::in_area(int x1, int y1, int x2, int y2)
{
	return( cur_x >= x1 && cur_y >= y1 && cur_x <= x2 && cur_y <= y2 );
}
//--------- End of MouseSDL::in_area --------------//


//--------- Begin of MouseSDL::press_area ----------//
//
// <Real-time access>
//
// Detect whether the specified area has been pressed by mouse
//
// <int> x1,y1,x2,y2 = the coordinations of the area
// <int> buttonId    = which button ( 0=left, 1-right, 2-left or right )
//
// Return : 1 - if the area has been pressed (left button)
//			   1 - if the area has been pressed (right button)
//          0 - if not
//
int MouseSDL::press_area(int x1, int y1, int x2, int y2, int buttonId)
{
	if( cur_x >= x1 && cur_y >= y1 && cur_x <= x2 && cur_y <= y2 )
	{
		if( left_press && (buttonId==0 || buttonId==2) )    // Left button
			return 1;

		if( right_press && (buttonId==1 || buttonId==2) )   // Right button
			return 2;
	}

	return 0;
}
//--------- End of MouseSDL::press_area --------------//


//--------- Begin of MouseSDL::set_boundary ----------//
//
// for each parameter, put -1 to mean unchange
//
void MouseSDL::set_boundary(int x1, int y1, int x2, int y2)
{
	if( x1 >= 0)
		bound_x1 = x1;
	if( y1 >= 0)
		bound_y1 = y1;
	if( x2 >= 0)
		bound_x2 = x2 > MOUSE_X_UPPER_LIMIT ? MOUSE_X_UPPER_LIMIT : x2;
	if( y2 >= 0)
		bound_y2 = y2 > MOUSE_Y_UPPER_LIMIT ? MOUSE_Y_UPPER_LIMIT : y2;
}
//--------- End of MouseSDL::set_boundary ----------//


//--------- Begin of MouseSDL::reset_boundary ----------//
void MouseSDL::reset_boundary()
{
	bound_x1 = 0;
	bound_y1 = 0;
	bound_x2 = MOUSE_X_UPPER_LIMIT;
	bound_y2 = MOUSE_Y_UPPER_LIMIT;
}
//--------- End of MouseSDL::set_boundary ----------//


//--------- Begin of MouseSDL::single_click ----------//
//
// <Event queue access>
//
// Detect whether the specified area has been single clicked by mouse
//
// <int> x1,y1,x2,y2 = the coordinations of the area
// [int] buttonId    = which button ( 0=left, 1-right, 2-both)
//                     (default:0)
//
// Return : 1 - if the area has been clicked (left click)
//				2 - if the area has been clicked (right click)
//          0 - if not
//
int MouseSDL::single_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseSDL::single_click --------------//


//--------- Begin of MouseSDL::double_click ----------//
//
// <Event queue access>
//
// Detect whether the specified area has been double clicked by mouse
//
// Note : when a mouse double click, it will FIRST generate a SINGLE
//        click signal and then a DOUBLE click signal.
//        Because a double click is consisted of two single click
//
// <int> x1,y1,x2,y2 = the coordinations of the area
// [int] buttonId    = which button ( 0=left, 1-right, 2-left or right)
//                     (default:0)
//
// Return : 1 - if the area has been clicked
//          0 - if not
//
int MouseSDL::double_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseSDL::double_click --------------//


//--------- Begin of MouseSDL::any_click ----------//
//
// <Event queue access>
//
// Detect whether the specified area has been single or double clicked by mouse
//
// <int> x1,y1,x2,y2 = the coordinations of the area
// [int] buttonId    = which button ( 0=left, 1-right, 2-left or right)
//                     (default:0)
//
// Return : >0 - the no. of click if the area has been clicked
//          0  - if not
//
int MouseSDL::any_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseSDL::any_click --------------//


//--------- Begin of MouseSDL::any_click ----------//
//
// <Event queue access>
//
// Detect whether the specified area has been single or double clicked by mouse
// Only check button, don't check mouse coordination
//
// [int] buttonId = which button ( 0=left, 1-right, 2-left or right)
//                  (default:0)
//
// Return : >0 - the no. of click if the area has been clicked
//          0  - if not
//
int MouseSDL::any_click(int buttonId)
{
   return 0;
}
//--------- End of MouseSDL::any_click --------------//


//--------- Begin of MouseSDL::release_click ----------//
//
// <Event queue access>
//
// Detect whether the specified area has been released button by mouse
//
// <int> x1,y1,x2,y2 = the coordinations of the area
// [int] buttonId    = which button ( 0=left, 1-right, 2-both)
//                     (default:0)
//
// Return : 1 - if the area has been clicked (left click)
//				2 - if the area has been clicked (right click)
//          0 - if not
//
int MouseSDL::release_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseSDL::release_click --------------//


//--------- Begin of MouseSDL::poll_event ----------//
//
// Poll mouse events from the direct mouse VXD.
//
void MouseSDL::poll_event()
{
}
//--------- End of MouseSDL::poll_event --------------//


// ####### begin Gilbert 31/10 #########//
//--------- Begin of MouseSDL::update_skey_state ----------//
// called after task switch to get the lastest state of ctrl/alt/shift key
void MouseSDL::update_skey_state()
{
}
//--------- End of MouseSDL::update_skey_state ----------//
// ####### end Gilbert 31/10 #########//


//--------- Begin of MouseSDL::wait_press ----------//
//
// Wait until one of the mouse buttons is pressed.
//
// [int] timeOutSecond - no. of seconds to time out. If not
//								 given, there will be no time out.
//
// return: <int> 1-left mouse button
//					  2-right mouse button
//
int MouseSDL::wait_press(int timeOutSecond)
{
	while( mouse.left_press || mouse.any_click() || mouse.key_code )		// avoid repeat clicking
	{
		sys.yield();
		mouse.get_event();
	}

	int rc=0;
	unsigned int timeOutTime = m.get_time() + timeOutSecond*1000;

	while(1)
	{
		sys.yield();
		mouse.get_event();

		if( sys.debug_session )
			sys.blt_virtual_buf();

		if( right_press || mouse.key_code==KEY_ESC )
		{
			rc = 2;
			break;
		}

		if( left_press || mouse.key_code )
		{
			rc = 1;
			break;
		}

		if( timeOutSecond && m.get_time() > timeOutTime )
			break;
	}

	while( mouse.left_press || mouse.any_click() || mouse.key_code )		// avoid repeat clicking 
	{
		sys.yield();
		mouse.get_event();
	}

	return rc;
}
//--------- End of MouseSDL::wait_press --------------//


//--------- Begin of MouseSDL::reset_click ----------//
//
// Reset queued mouse clicks.
//
void MouseSDL::reset_click()
{
	click_buffer[0].count=0;
	click_buffer[1].count=0;
}
//--------- End of MouseSDL::reset_click --------------//


// ------ Begin of MouseSDL::is_key -------//
// compare key with key code
// e.g. to test a key is alt-a,
// call mouse.is_key(mouse.scan_code, mouse.event_skey_state, 'a', K_CHAR_KEY | K_IS_ALT)
//
// pass 0 as charValue to disable checking in charValue
// e.g pressed key is 'a'
// mouse.is_key(mouse.scan_code, mouse.event_skey_state, (WORD) 0, K_CHAR_KEY) returns 'a'
// but if key pressed is alt-a
// the same function call returns 0
// use mouse.is_key(mouse.scan_code, mouse.event_skey_state, (WORD) 0, K_CHAR_KEY | K_IS_ALT ) instead
//
int MouseSDL::is_key(unsigned scanCode, unsigned short skeyState, unsigned short charValue, unsigned flags)
{
   return 0;
}
// ------ End of MouseSDL::is_key -------//


// ------ Begin of MouseSDL::is_key -------//
int MouseSDL::is_key(unsigned scanCode, unsigned short skeyState, char *keyStr, unsigned flags)
{
   return 0;
}
// ------ End of MouseSDL::is_key -------//
