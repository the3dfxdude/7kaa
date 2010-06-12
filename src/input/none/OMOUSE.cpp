/*
 * Seven Kingdoms: Ancient Adversaries
 *
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

//--------- Start of MouseNone::MouseNone ---------//
//
MouseNone::MouseNone()
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
}
//---------- End of MouseNone::MouseNone ---------//


//---------- Begin of MouseNone::~MouseNone --------//
//
// Deinitialize the mouse driver, reset event handler
//
MouseNone::~MouseNone()
{
	deinit();
}
//------------ End of MouseNone::~MouseNone --------//


//------------ Start of MouseNone::init ------------//
//
void MouseNone::init()
{
}
//------------- End of MouseNone::init -------------//


//------------ Start of MouseNone::deinit ------------//
//
void MouseNone::deinit()
{
}
//------------- End of MouseNone::deinit -------------//


//--------- Start of MouseNone::hide -------//
//
// Suspend the mouse function, use resume() to resume to function
//
void MouseNone::hide()
{
	mouse_cursor.hide_all_flag=1;

	mouse_cursor.process(cur_x, cur_y);
}
//---------- End of MouseNone::hide --------//


//--------- Start of MouseNone::show -------//
//
// Resume the mouse function which is previously hideed by hide()
//
void MouseNone::show()
{
	mouse_cursor.hide_all_flag=0;

	mouse_cursor.process(cur_x, cur_y);
}
//---------- End of MouseNone::show --------//


//--------- Begin of MouseNone::hide_area ----------//
//
void MouseNone::hide_area(int x1, int y1, int x2, int y2)
{
}
//--------- End of MouseNone::hide_area --------------//


//--------- Begin of MouseNone::show_area ----------//
//
void MouseNone::show_area()
{
}
//--------- End of MouseNone::show_area --------------//


//--------- Start of MouseNone::add_event ---------//
//
// Called by handler interrupt to procss the state
//
void MouseNone::add_event(MouseEvent *mouseEvent)
{
}
//----------- End of MouseNone::add_event ----------//


//--------- Start of MouseNone::add_key_event ---------//
//
// Called by key handler to save the key pressed
//
void MouseNone::add_key_event(unsigned scanCode, unsigned long timeStamp)
{
}
//----------- End of MouseNone::add_key_event ----------//


//--------- Start of MouseNone::get_event ---------//
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
int MouseNone::get_event()
{
   return 1;
}
//----------- End of MouseNone::get_event ----------//


//--------- Begin of MouseNone::in_area ----------//
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
int MouseNone::in_area(int x1, int y1, int x2, int y2)
{
   return 0;
}
//--------- End of MouseNone::in_area --------------//


//--------- Begin of MouseNone::press_area ----------//
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
int MouseNone::press_area(int x1, int y1, int x2, int y2, int buttonId)
{
   return 0;
}
//--------- End of MouseNone::press_area --------------//


//--------- Begin of MouseNone::set_boundary ----------//
//
// for each parameter, put -1 to mean unchange
//
void MouseNone::set_boundary(int x1, int y1, int x2, int y2)
{
}
//--------- End of MouseNone::set_boundary ----------//


//--------- Begin of MouseNone::reset_boundary ----------//
void MouseNone::reset_boundary()
{
}
//--------- End of MouseNone::set_boundary ----------//


//--------- Begin of MouseNone::single_click ----------//
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
int MouseNone::single_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseNone::single_click --------------//


//--------- Begin of MouseNone::double_click ----------//
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
int MouseNone::double_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseNone::double_click --------------//


//--------- Begin of MouseNone::any_click ----------//
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
int MouseNone::any_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseNone::any_click --------------//


//--------- Begin of MouseNone::any_click ----------//
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
int MouseNone::any_click(int buttonId)
{
   return 0;
}
//--------- End of MouseNone::any_click --------------//


//--------- Begin of MouseNone::release_click ----------//
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
int MouseNone::release_click(int x1, int y1, int x2, int y2,int buttonId)
{
   return 0;
}
//--------- End of MouseNone::release_click --------------//


//--------- Begin of MouseNone::poll_event ----------//
//
// Poll mouse events from the direct mouse VXD.
//
void MouseNone::poll_event()
{
}
//--------- End of MouseNone::poll_event --------------//


// ####### begin Gilbert 31/10 #########//
//--------- Begin of MouseNone::update_skey_state ----------//
// called after task switch to get the lastest state of ctrl/alt/shift key
void MouseNone::update_skey_state()
{
}
//--------- End of MouseNone::update_skey_state ----------//
// ####### end Gilbert 31/10 #########//


//--------- Begin of MouseNone::wait_press ----------//
//
// Wait until one of the mouse buttons is pressed.
//
// [int] timeOutSecond - no. of seconds to time out. If not
//								 given, there will be no time out.
//
// return: <int> 1-left mouse button
//					  2-right mouse button
//
int MouseNone::wait_press(int timeOutSecond)
{
   return 0;
}
//--------- End of MouseNone::wait_press --------------//


//--------- Begin of MouseNone::reset_click ----------//
//
// Reset queued mouse clicks.
//
void MouseNone::reset_click()
{
}
//--------- End of MouseNone::reset_click --------------//


// ------ Begin of MouseNone::is_key -------//
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
int MouseNone::is_key(unsigned scanCode, unsigned short skeyState, unsigned short charValue, unsigned flags)
{
   return 0;
}
// ------ End of MouseNone::is_key -------//


// ------ Begin of MouseNone::is_key -------//
int MouseNone::is_key(unsigned scanCode, unsigned short skeyState, char *keyStr, unsigned flags)
{
   return 0;
}
// ------ End of MouseNone::is_key -------//
