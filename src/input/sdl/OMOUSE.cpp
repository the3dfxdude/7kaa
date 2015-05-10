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
#include <OPOWER.h>
#include <KEY.h>
#include <OSYS.h>
#include <ctype.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Mouse);

#define VGA_UPDATE_BUF_SIZE	(100*100)
static int update_x1, update_y1, update_x2, update_y2;          // coordination of the last double-buffer update area


//--------- Define Click Threshold -----------//
//
// Clock tick is incremented 1000 times per second or once per millisecond.
//
// The minimum time interval between consequent mouse click is set to
// = 0.3 seconds
//
//--------------------------------------------//

static unsigned long click_threshold = (long)(0.3 * 1000);


//--------- Start of MouseSDL::MouseSDL ---------//
//
MouseSDL::MouseSDL()
{
	handle_flicking = 0;
	vga_update_buf = NULL;
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
	memset(click_buffer, 0, sizeof(MouseClick) * 2);
	scan_code = 0;
	key_code = 0;
	memset(event_buffer, 0, sizeof(MouseEvent) * EVENT_BUFFER_SIZE);
	head_ptr = 0;
	tail_ptr = 0;
	double_speed_threshold = DEFAULT_DOUBLE_SPEED_THRESHOLD;
	triple_speed_threshold = DEFAULT_TRIPLE_SPEED_THRESHOLD;
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
	update_skey_state();

	//------- initialize VGA update buffer -------//

	vga_update_buf = mem_add( VGA_UPDATE_BUF_SIZE );

	// ------ initialize mouse boundary ---------//
	reset_boundary();

	// ------- initialize event queue ---------//
	head_ptr = tail_ptr = 0;

	SDL_StopTextInput();
	SDL_ShowCursor(SDL_DISABLE);
}
//------------- End of MouseSDL::init -------------//


//------------ Start of MouseSDL::deinit ------------//
//
void MouseSDL::deinit()
{
	if( vga_update_buf )
	{
		mem_del(vga_update_buf);
		vga_update_buf = NULL;
	}

	SDL_ShowCursor(SDL_DISABLE);
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
// Resume the mouse function which is previously hidden by hide()
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

	if( misc.is_touch( x1, y1, x2, y2, curX, curY,
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

	if( misc.is_touch( mouse_cursor.hide_x1, mouse_cursor.hide_y1,
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
	//---- call the game object to see if the mouse cursor icon needs to be changed, or if the nation selection square needs to be activated ----//

	power.mouse_handler();

	//--------- update the mouse cursor ----------//

	mouse_cursor.process(cur_x, cur_y);     // repaint mouse cursor

	//-------- save state into the event queue --------//

	if((head_ptr == tail_ptr-1) ||            // see if the buffer is full
		(head_ptr == EVENT_BUFFER_SIZE-1 && tail_ptr == 0))
	{
		return;
	}

	event_buffer[head_ptr] = *mouseEvent;

	if(++head_ptr >= EVENT_BUFFER_SIZE)       // increment the head ptr
		head_ptr = 0;
}
//----------- End of MouseSDL::add_event ----------//


//--------- Start of MouseSDL::add_key_event ---------//
//
// Called by key handler to save the key pressed
//
void MouseSDL::add_key_event(unsigned scanCode, unsigned long timeStamp)
{
	if((head_ptr == tail_ptr-1) ||               // see if the buffer is full
		(head_ptr == EVENT_BUFFER_SIZE-1 && tail_ptr == 0))
	{
		return;
	}

	MouseEvent *ev = event_buffer + head_ptr;

	ev->event_type = KEY_PRESS;
	ev->scan_code = scanCode;
	ev->skey_state = skey_state;
	ev->time = timeStamp;

	// put mouse state
	// ev->state = 0;			//ev->state = left_press | right_press;
	ev->x = cur_x;
	ev->y = cur_y;

	if(++head_ptr >= EVENT_BUFFER_SIZE)  // increment the head ptr
		head_ptr = 0;
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
	if(head_ptr == tail_ptr)     // no event queue left in the buffer
	{
		scan_code      =0;        // no keyboard event
		key_code       =0;
		has_mouse_event=0;        // no mouse event
		return 0;
	}

	//--------- get event from queue ---------//

	MouseEvent* eptr = event_buffer + tail_ptr;
	MouseClick* cptr;

	event_skey_state = eptr->skey_state;
	mouse_event_type = eptr->event_type;

	switch( eptr->event_type )
	{
	case LEFT_BUTTON:
	case RIGHT_BUTTON:
		// set count of other button to zero
		click_buffer[LEFT_BUTTON+RIGHT_BUTTON-eptr->event_type].count = 0;
		cptr = click_buffer + eptr->event_type;
		if( //eptr->event_type == LEFT_BUTTON	&&		// left button has double click
			 eptr->time - cptr->time < click_threshold )
			cptr->count++;
		else
			cptr->count = 1;

		cptr->time = eptr->time;
		cptr->x    = eptr->x;
      cptr->y    = eptr->y;
		scan_code       = 0;
		key_code        = 0;
      has_mouse_event = 1;
		break;

	case KEY_PRESS:
		scan_code = eptr->scan_code;
		key_code = mouse.is_key(scan_code, event_skey_state, (WORD) 0, K_CHAR_KEY);
		has_mouse_event = 0;
		break;

	case LEFT_BUTTON_RELEASE:
	case RIGHT_BUTTON_RELEASE:
		cptr = click_buffer + eptr->event_type - LEFT_BUTTON_RELEASE;
		cptr->release_time = eptr->time;
		cptr->release_x    = eptr->x;
		cptr->release_y    = eptr->y;
		scan_code          = 0;
		key_code           = 0;
		has_mouse_event    = 1;
		break;

	case KEY_RELEASE:
		// no action
		break;

	default:
		err_here();
   }

   if(++tail_ptr >= EVENT_BUFFER_SIZE)
      tail_ptr = 0;

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
	if( !has_mouse_event )
		return 0;

	MouseClick* cptr;

	if( buttonId==0 || buttonId==2 )     // left button
	{
		cptr = click_buffer+LEFT_BUTTON;

		if( mouse_event_type == LEFT_BUTTON
			 && cptr->count == 1
			 && cptr->x >= x1 && cptr->y >= y1
			 && cptr->x <= x2 && cptr->y <= y2 )
		{
			return 1;
		}
	}

	if( buttonId==1 || buttonId==2 )     // right button
	{
		cptr = click_buffer+RIGHT_BUTTON;

		if( mouse_event_type == RIGHT_BUTTON
			 && cptr->count == 1
			 && cptr->x >= x1 && cptr->y >= y1
			 && cptr->x <= x2 && cptr->y <= y2 )
		{
			return 2;
      }
   }

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
	if( !has_mouse_event )
      return 0;

   MouseClick* cptr;

   if( buttonId==0 || buttonId==2 )     // left button
   {
      cptr = click_buffer+LEFT_BUTTON;

      if( mouse_event_type == LEFT_BUTTON
			 && cptr->count == 2
			 && cptr->x >= x1 && cptr->y >= y1
          && cptr->x <= x2 && cptr->y <= y2 )
      {
         return 1;
      }
   }

	if( buttonId==1 || buttonId==2 )     // right button
   {
		cptr = click_buffer+RIGHT_BUTTON;

      if( mouse_event_type == RIGHT_BUTTON
			 && cptr->count == 2
          && cptr->x >= x1 && cptr->y >= y1
          && cptr->x <= x2 && cptr->y <= y2 )
      {
         return 1;
      }
   }

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
   if( !has_mouse_event )
      return 0;

	MouseClick* cptr;

	if( buttonId==0 || buttonId==2 )     // left button
   {
      cptr = click_buffer+LEFT_BUTTON;

      if( mouse_event_type == LEFT_BUTTON
			 && cptr->count >= 1
          && cptr->x >= x1 && cptr->y >= y1
          && cptr->x <= x2 && cptr->y <= y2 )
      {
			return cptr->count;
      }
   }

   if( buttonId==1 || buttonId==2 )     // right button
   {
      cptr = click_buffer+RIGHT_BUTTON;

      if( mouse_event_type == RIGHT_BUTTON
			 && cptr->count >= 1
          && cptr->x >= x1 && cptr->y >= y1
          && cptr->x <= x2 && cptr->y <= y2 )
      {
         return cptr->count;
      }
   }

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
	if( !has_mouse_event )
      return 0;

   MouseClick* cptr;

   if( buttonId==0 || buttonId==2 )     // left button
   {
      cptr = click_buffer+LEFT_BUTTON;

		if( mouse_event_type == LEFT_BUTTON && cptr->count >= 1 )
			return cptr->count;
   }

   if( buttonId==1 || buttonId==2 )     // right button
   {
      cptr = click_buffer+RIGHT_BUTTON;

      if( mouse_event_type == RIGHT_BUTTON && cptr->count >= 1 )
         return cptr->count;
   }

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
	if( !has_mouse_event )
		return 0;

	MouseClick* cptr;

	if( buttonId==0 || buttonId==2 )     // left button
	{
		cptr = click_buffer+LEFT_BUTTON;

		if( mouse_event_type == LEFT_BUTTON_RELEASE
			 && cptr->release_x >= x1 && cptr->release_y >= y1
			 && cptr->release_x <= x2 && cptr->release_y <= y2 )
		{
			return 1;
		}
	}

	if( buttonId==1 || buttonId==2 )     // right button
	{
		cptr = click_buffer+RIGHT_BUTTON;

		if( mouse_event_type == RIGHT_BUTTON_RELEASE
			 && cptr->release_x >= x1 && cptr->release_y >= y1
			 && cptr->release_x <= x2 && cptr->release_y <= y2 )
		{
			return 2;
      }
   }

   return 0;
}
//--------- End of MouseSDL::release_click --------------//


//--------- Begin of MouseSDL::poll_event ----------//
//
// Poll mouse events from the direct mouse VXD.
//
void MouseSDL::poll_event()
{
	SDL_Event event;
	int moveFlag;

	moveFlag = 0;

	while (SDL_PeepEvents(&event,
			1,
			SDL_GETEVENT,
			SDL_KEYDOWN,
			SDL_JOYBUTTONUP)) {

		MouseEvent ev;

		switch (event.type) {
		case SDL_MOUSEMOTION:
		// SDL already accelerates relative mouse motions.
		// Disable to let the user control speed outside of game.
#ifdef MOUSE_RELATIVE
			cur_x += micky_to_displacement(event.motion.xrel);
			cur_y += micky_to_displacement(event.motion.yrel);
#else
			cur_x = event.motion.x;
			cur_y = event.motion.y;
#endif
			if(cur_x < bound_x1)
				cur_x = bound_x1;
			if(cur_x > bound_x2)
				cur_x = bound_x2;
			if(cur_y < bound_y1)
				cur_y = bound_y1;
			if(cur_y > bound_y2)
				cur_y = bound_y2;
			moveFlag = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			ev.x = cur_x;
			ev.y = cur_y;
			ev.time = misc.get_time(); //mouseMsg->dwTimeStamp;
			ev.scan_code = 0;
			ev.skey_state = skey_state;

			if (event.button.button == SDL_BUTTON_LEFT) {
	//			left_press = (event.button.state == SDL_PRESSED);
				left_press = LEFT_BUTTON_MASK;
				ev.event_type = LEFT_BUTTON;
				add_event(&ev);
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				//right_press = (event.button.state == SDL_PRESSED);
				right_press = RIGHT_BUTTON_MASK;
				ev.event_type = RIGHT_BUTTON;
				add_event(&ev);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			ev.x = cur_x;
			ev.y = cur_y;
			ev.time = misc.get_time(); //mouseMsg->dwTimeStamp;
			ev.scan_code = 0;
			ev.skey_state = skey_state;

			if (event.button.button == SDL_BUTTON_LEFT) {
//				left_press = !(event.button.state == SDL_RELEASED);
				left_press = 0;
				ev.event_type = LEFT_BUTTON_RELEASE;
				add_event(&ev);
				reset_boundary();
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				//right_press = !(event.button.state == SDL_RELEASED);
				right_press = 0;
				ev.event_type = RIGHT_BUTTON_RELEASE;
				add_event(&ev);
			}
			break;
		case SDL_KEYDOWN:
		{
			int bypass = 0;
			int mod = event.key.keysym.mod &
					(KMOD_CTRL|KMOD_SHIFT|KMOD_ALT);
			if (mod == KMOD_LALT || mod == KMOD_RALT) {
				if (event.key.keysym.sym == SDLK_RETURN) {
					bypass = 1;
					sys.toggle_full_screen_flag = 1;
					sys.need_redraw_flag = 1;
				} else if (event.key.keysym.sym == SDLK_F4) {
					bypass = 1;
					sys.signal_exit_flag = 1;
				} else if (event.key.keysym.sym == SDLK_TAB) {
					bypass = 1;
					SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
					SDL_MinimizeWindow(window);
				}
			} else if (mod == KMOD_LCTRL || mod == KMOD_RCTRL) {
				if (event.key.keysym.sym == SDLK_g &&
						!vga.is_full_screen()) {
					static int grabbed = 0;
					bypass = 1;
					SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
					if (!grabbed) {
						SDL_SetWindowGrab(window, SDL_TRUE);
						grabbed = 1;
					} else {
						SDL_SetWindowGrab(window, SDL_FALSE);
						grabbed = 0;
					}
				}
			}
			if (!bypass) {
				update_skey_state();
				add_key_event(event.key.keysym.sym,
					      misc.get_time());
			}
			break;
		}
		case SDL_KEYUP:
			update_skey_state();
			break;
		case SDL_TEXTINPUT:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		default:
			ERR("unhandled event %d\n", event.type);
			break;
		}
	}


	if(moveFlag)
	{
		mouse_cursor.process(cur_x, cur_y);     // repaint mouse cursor
		power.mouse_handler();
	}
}
//--------- End of MouseSDL::poll_event --------------//


// ####### begin Gilbert 31/10 #########//
//--------- Begin of MouseSDL::update_skey_state ----------//
// called after task switch to get the lastest state of ctrl/alt/shift key
void MouseSDL::update_skey_state()
{
	int modstate = SDL_GetModState();

	skey_state = 0;
	if (modstate & KMOD_LSHIFT)
		skey_state |= LEFT_SHIFT_KEY_MASK;
	if (modstate & KMOD_RSHIFT)
		skey_state |= RIGHT_SHIFT_KEY_MASK;
	if (modstate & KMOD_LCTRL)
		skey_state |= LEFT_CONTROL_KEY_MASK;
	if (modstate & KMOD_RCTRL)
		skey_state |= RIGHT_CONTROL_KEY_MASK;
	if (modstate & KMOD_LALT)
		skey_state |= LEFT_ALT_KEY_MASK;
	if (modstate & KMOD_RALT)
#if(defined(FRENCH)||defined(SPANISH))
		skey_state |= GRAPH_KEY_MASK;
#else
		skey_state |= RIGHT_ALT_KEY_MASK;
#endif
	if (modstate & KMOD_NUM)
		skey_state |= NUM_LOCK_STATE_MASK;
	if (modstate & KMOD_CAPS)
		skey_state |= CAP_LOCK_STATE_MASK;
	if (modstate & KMOD_MODE) // Alt Gr key
		skey_state |= GRAPH_KEY_MASK;
	skey_state |= INSERT_STATE_MASK; // enable insert mode by default
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
	unsigned int timeOutTime = misc.get_time() + timeOutSecond*1000;

	while(1)
	{
		sys.yield();
		mouse.get_event();

		if( sys.signal_exit_flag == 1 )
		{
			break;
		}

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

		if( timeOutSecond && misc.get_time() > timeOutTime )
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


// ------ Begin of MouseSDL::mickey_to_displacment -------//
long MouseSDL::micky_to_displacement(unsigned long w)
{
	long d = (long)w ;
	// long a = abs(d);
	// return a >= double_speed_threshold ? (a >= triple_speed_threshold ? 3 * d : 2 * d) : d;
	return abs(d) >= double_speed_threshold ? d+d : d;
}
// ------ End of MouseSDL::mickey_to_displacment -------//


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
	unsigned short priChar = 0, shiftChar = 0, capitalChar = 0;
#if(defined(FRENCH)||defined(GERMAN)||defined(SPANISH))
	unsigned short altChar = 0;
#endif
	unsigned onNumPad = 0;

	switch(scanCode)
	{
	case SDLK_ESCAPE: priChar = shiftChar = capitalChar = KEY_ESC; break;
#if(defined(SPANISH))
	case SDLK_1: priChar = capitalChar = '1'; shiftChar = '!'; altChar = '|'; break;
#else
	case SDLK_1: priChar = capitalChar = '1'; shiftChar = '!'; break;
#endif
#if(defined(FRENCH)||defined(SPANISH))
	case SDLK_2: priChar = capitalChar = '2'; shiftChar = '\"'; altChar = '@'; break;
#elif(defined(GERMAN))
	case SDLK_2: priChar = capitalChar = '2'; shiftChar = '\"'; altChar = (UCHAR)'²'; break;
#else
	case SDLK_2: priChar = capitalChar = '2'; shiftChar = '@'; break;
#endif
#if(defined(FRENCH))
	case SDLK_3: priChar = capitalChar = '3'; shiftChar = '/'; altChar = (UCHAR)'£'; break;
#elif(defined(SPANISH))
	case SDLK_3: priChar = capitalChar = '3'; shiftChar = (UCHAR)'·'; altChar = '#'; break;
#elif(defined(GERMAN))
	case SDLK_3: priChar = capitalChar = '3'; shiftChar = (UCHAR)'§'; altChar = (UCHAR)'³'; break;
#else
	case SDLK_3: priChar = capitalChar = '3'; shiftChar = '#'; break;
#endif
	case SDLK_4: priChar = capitalChar = '4'; shiftChar = '$'; break;
	case SDLK_5: priChar = capitalChar = '5'; shiftChar = '%'; break;
#if(defined(FRENCH))
	case SDLK_6: priChar = capitalChar = '6'; shiftChar = '?'; altChar = (UCHAR)'¬';break;
#elif(defined(SPANISH))
	case SDLK_6: priChar = capitalChar = '6'; shiftChar = '&'; break;
#elif(defined(GERMAN))
	case SDLK_6: priChar = capitalChar = '6'; shiftChar = '&'; break;
#else
	case SDLK_6: priChar = capitalChar = '6'; shiftChar = '^'; break;
#endif
#if(defined(GERMAN))
	case SDLK_7: priChar = capitalChar = '7'; shiftChar = '/'; altChar = '{'; break;
	case SDLK_8: priChar = capitalChar = '8'; shiftChar = '('; altChar = '['; break;
	case SDLK_9: priChar = capitalChar = '9'; shiftChar = ')'; altChar = ']'; break;
	case SDLK_0: priChar = capitalChar = '0'; shiftChar = '='; altChar = '}'; break;
	case 0x0c:  priChar = capitalChar = (UCHAR)'ß'; shiftChar = '\?'; altChar = '\\'; break;
	case 0x0d:  priChar = capitalChar = (UCHAR)'´'; shiftChar = (UCHAR)'`'; break;
#elif(defined(SPANISH))
	case SDLK_7: priChar = capitalChar = '7'; shiftChar = '/'; break;
	case SDLK_8: priChar = capitalChar = '8'; shiftChar = '('; break;
	case SDLK_9: priChar = capitalChar = '9'; shiftChar = ')'; break;
	case SDLK_0: priChar = capitalChar = '0'; shiftChar = '='; break;
	case 0x0c:  priChar = capitalChar = '\''; shiftChar = '?'; break;
	case 0x0d:  priChar = capitalChar = (UCHAR)'¡'; shiftChar = (UCHAR)'¿'; break;
#else
	case SDLK_7: priChar = capitalChar = '7'; shiftChar = '&'; break;
	case SDLK_8: priChar = capitalChar = '8'; shiftChar = '*'; break;
	case SDLK_9: priChar = capitalChar = '9'; shiftChar = '('; break;
	case SDLK_0: priChar = capitalChar = '0'; shiftChar = ')'; break;
	case SDLK_MINUS: priChar = capitalChar = '-'; shiftChar = '_'; break;
	case SDLK_EQUALS: priChar = capitalChar = '='; shiftChar = '+'; break;
#endif
	case SDLK_BACKSPACE: priChar = capitalChar = shiftChar = KEY_BACK_SPACE; break;   // backspace
	case SDLK_TAB: priChar = capitalChar = shiftChar = KEY_TAB; break;
#if(defined(GERMAN))
	case SDLK_q: priChar = 'q'; capitalChar = shiftChar = 'Q'; altChar = '@'; break;
#else
	case SDLK_q: priChar = 'q'; capitalChar = shiftChar = 'Q'; break;
#endif
	case SDLK_w: priChar = 'w'; capitalChar = shiftChar = 'W'; break;
	case SDLK_e: priChar = 'e'; capitalChar = shiftChar = 'E'; break;
	case SDLK_r: priChar = 'r'; capitalChar = shiftChar = 'R'; break;
	case SDLK_t: priChar = 't'; capitalChar = shiftChar = 'T'; break;
#if(defined(GERMAN))
	case 0x15:  priChar = 'z'; capitalChar = shiftChar = 'Z'; break;
#else
	case SDLK_y: priChar = 'y'; capitalChar = shiftChar = 'Y'; break;
#endif
	case SDLK_u: priChar = 'u'; capitalChar = shiftChar = 'U'; break;
	case SDLK_i: priChar = 'i'; capitalChar = shiftChar = 'I'; break;
	case SDLK_o: priChar = 'o'; capitalChar = shiftChar = 'O'; break;
	case SDLK_p: priChar = 'p'; capitalChar = shiftChar = 'P'; break;
#if(defined(FRENCH))
	case 0x1a:  priChar = capitalChar = shiftChar = '^'; altChar = '['; break;
	case 0x1b:  priChar = capitalChar = (UCHAR)'¸';shiftChar = (UCHAR)'¨'; altChar = ']'; break;
#elif(defined(SPANISH))
	case 0x1a:  priChar = capitalChar = '`'; shiftChar = '^'; altChar = '['; break;
	case 0x1b:  priChar = capitalChar = '+';shiftChar = '*'; altChar = ']'; break;
#elif(defined(GERMAN))
	case 0x1a:  priChar = (UCHAR)'ü'; capitalChar = shiftChar = (UCHAR)'Ü'; break;
	case 0x1b:  priChar = capitalChar = shiftChar = '+'; altChar = '~'; break;
#else
	case SDLK_LEFTBRACKET: priChar = capitalChar = '['; shiftChar = '{'; break;
	case SDLK_RIGHTBRACKET: priChar = capitalChar = ']'; shiftChar = '}'; break;
#endif
	case SDLK_KP_ENTER:		// Enter on numeric keypad
		onNumPad = 1;			// fall through
	case SDLK_RETURN: priChar = capitalChar = shiftChar = KEY_RETURN;	break;
	case SDLK_a: priChar = 'a'; capitalChar = shiftChar = 'A'; break;
	case SDLK_s: priChar = 's'; capitalChar = shiftChar = 'S'; break;
	case SDLK_d: priChar = 'd'; capitalChar = shiftChar = 'D'; break;
	case SDLK_f: priChar = 'f'; capitalChar = shiftChar = 'F'; break;
	case SDLK_g: priChar = 'g'; capitalChar = shiftChar = 'G'; break;
	case SDLK_h: priChar = 'h'; capitalChar = shiftChar = 'H'; break;
	case SDLK_j: priChar = 'j'; capitalChar = shiftChar = 'J'; break;
	case SDLK_k: priChar = 'k'; capitalChar = shiftChar = 'K'; break;
	case SDLK_l: priChar = 'l'; capitalChar = shiftChar = 'L'; break;
#if(defined(FRENCH))
	case SDLK_SEMICOLON: priChar = capitalChar = ';'; shiftChar = ':'; altChar = '~'; break;
	case 0x28:  priChar = capitalChar = shiftChar = '`'; altChar = '{'; break;
	case 0x29:  priChar = capitalChar = '#'; shiftChar = '|'; altChar = '\\'; break;
	case 0x2b:  priChar = capitalChar = '<'; shiftChar = '>'; altChar = '}'; break;
#elif(defined(SPANISH))
	case 0x27:  priChar = (UCHAR)'ñ'; capitalChar = shiftChar = (UCHAR)'Ñ'; break;
	case 0x28:  priChar = capitalChar = (UCHAR)'´'; shiftChar = (UCHAR)'¨'; altChar = '{'; break;
	case 0x29:  priChar = capitalChar = (UCHAR)'º'; shiftChar = (UCHAR)'ª'; altChar = '\\'; break;
	case 0x2b:  priChar = (UCHAR)'ç'; capitalChar = shiftChar = (UCHAR)'Ç'; altChar = '}'; break;
#elif(defined(GERMAN))
	case 0x27:  priChar = (UCHAR)'ö'; capitalChar = shiftChar = (UCHAR)'Ö'; break;
	case 0x28:  priChar = (UCHAR)'ä'; capitalChar = shiftChar = (UCHAR)'Ä'; break;
	case 0x29:  priChar = capitalChar = '^'; shiftChar = (UCHAR)'°'; break;
	case 0x2b:  priChar = capitalChar = '#'; shiftChar = '\''; break;
#else
	case SDLK_SEMICOLON: priChar = capitalChar = ';'; shiftChar = ':'; break;
	case SDLK_QUOTEDBL: priChar = capitalChar = '\''; shiftChar = '\"'; break;
	case SDLK_BACKQUOTE: priChar = capitalChar = '~'; shiftChar = '`'; break; // Note: this can be reversed on certain keyboards (at the very least on Dutch USA-international keyboards) where ` is the base and ~ the shift
	case SDLK_BACKSLASH: priChar = capitalChar = '\\'; shiftChar = '|'; break;
#endif
#if(defined(GERMAN))
	case 0x2c:  priChar = 'y'; capitalChar = shiftChar = 'Y'; break;
#else
	case SDLK_z: priChar = 'z'; capitalChar = shiftChar = 'Z'; break;
#endif
	case SDLK_x: priChar = 'x'; capitalChar = shiftChar = 'X'; break;
	case SDLK_c: priChar = 'c'; capitalChar = shiftChar = 'C'; break;
	case SDLK_v: priChar = 'v'; capitalChar = shiftChar = 'V'; break;
	case SDLK_b: priChar = 'b'; capitalChar = shiftChar = 'B'; break;
	case SDLK_n: priChar = 'n'; capitalChar = shiftChar = 'N'; break;
#if(defined(GERMAN))
	case SDLK_m: priChar = 'm'; capitalChar = shiftChar = 'M'; altChar = (UCHAR)'µ'; break;
#else
	case SDLK_m: priChar = 'm'; capitalChar = shiftChar = 'M'; break;
#endif
#if(defined(FRENCH))
	case SDLK_COMMA: priChar = capitalChar = ','; shiftChar = '\''; altChar = (UCHAR)'¯'; break;
	case SDLK_PERIOD: priChar = capitalChar = '.'; shiftChar = '\"'; altChar = (UCHAR)'­'; break;
	case 0x35:  priChar = (UCHAR)'é'; capitalChar = shiftChar = (UCHAR)'É'; altChar = (UCHAR)'´'; break;
#elif(defined(SPANISH))
	case SDLK_COMMA: priChar = capitalChar = ','; shiftChar = ';'; break;
	case SDLK_PERIOD: priChar = capitalChar = '.'; shiftChar = ':'; break;
	case 0x35:  priChar = capitalChar = '-'; shiftChar = '_'; break;
#else
	case SDLK_COMMA: priChar = capitalChar = ','; shiftChar = '<'; break;
	case SDLK_PERIOD: priChar = capitalChar = '.'; shiftChar = '>'; break;
	case SDLK_SLASH: priChar = capitalChar = '/'; shiftChar = '\?'; break;
#endif
	case SDLK_KP_MULTIPLY: priChar = capitalChar = shiftChar = '*'; onNumPad = 1; break; // * on numeric keypad
	case SDLK_SPACE: priChar = capitalChar = shiftChar = ' '; break;
	case SDLK_KP_PLUS: priChar = capitalChar = shiftChar = '+'; onNumPad = 1; break; // + on numeric keypad
	case SDLK_KP_DIVIDE: priChar = capitalChar = shiftChar = '/'; onNumPad = 1; break;		// / on numeric keypad
	case SDLK_KP_MINUS: priChar = capitalChar = shiftChar = '-'; onNumPad = 1; break;	// - on numeric keypad
#if(defined(GERMAN))
	case 0x56:  priChar = capitalChar = '<'; shiftChar = '>'; altChar = '|'; break;
#endif
		
	case SDLK_KP_7: priChar = shiftChar = capitalChar = '7'; onNumPad = 1; break;
	case SDLK_KP_8: priChar = shiftChar = capitalChar = '8'; onNumPad = 1; break;
	case SDLK_KP_9: priChar = shiftChar = capitalChar = '9'; onNumPad = 1; break;
	case SDLK_KP_4: priChar = shiftChar = capitalChar = '4'; onNumPad = 1; break;
	case SDLK_KP_5: priChar = shiftChar = capitalChar = '5'; onNumPad = 1; break;
	case SDLK_KP_6: priChar = shiftChar = capitalChar = '6'; onNumPad = 1; break;
	case SDLK_KP_1: priChar = shiftChar = capitalChar = '1'; onNumPad = 1; break;
	case SDLK_KP_2: priChar = shiftChar = capitalChar = '2'; onNumPad = 1; break;
	case SDLK_KP_3: priChar = shiftChar = capitalChar = '3'; onNumPad = 1; break;
	case SDLK_KP_0: priChar = shiftChar = capitalChar = '0'; onNumPad = 1; break;
	case SDLK_KP_PERIOD: priChar = shiftChar = capitalChar = '.'; onNumPad = 1; break;

	// function keys
	case SDLK_F1: priChar = shiftChar = capitalChar = KEY_F1; break;
	case SDLK_F2: priChar = shiftChar = capitalChar = KEY_F2; break;
	case SDLK_F3: priChar = shiftChar = capitalChar = KEY_F3; break;
	case SDLK_F4: priChar = shiftChar = capitalChar = KEY_F4; break;
	case SDLK_F5: priChar = shiftChar = capitalChar = KEY_F5; break;
	case SDLK_F6: priChar = shiftChar = capitalChar = KEY_F6; break;
	case SDLK_F7: priChar = shiftChar = capitalChar = KEY_F7; break;
	case SDLK_F8: priChar = shiftChar = capitalChar = KEY_F8; break;
	case SDLK_F9: priChar = shiftChar = capitalChar = KEY_F9; break;
	case SDLK_F10: priChar = shiftChar = capitalChar = KEY_F10; break;
	case SDLK_F11: priChar = shiftChar = capitalChar = KEY_F11; break;
	case SDLK_F12: priChar = shiftChar = capitalChar = KEY_F12; break;

	// arrow keys
#if(defined(GERMAN))
	case 0x67:		// fall through, German keyboard called "Pos 1"
#endif
	case SDLK_HOME: priChar = shiftChar = capitalChar = KEY_HOME; break;
	case SDLK_UP: priChar = shiftChar = capitalChar = KEY_UP; break;
	case SDLK_PAGEUP: priChar = shiftChar = capitalChar = KEY_PGUP; break;
	case SDLK_LEFT: priChar = shiftChar = capitalChar = KEY_LEFT; break;
	case SDLK_RIGHT: priChar = shiftChar = capitalChar = KEY_RIGHT; break;
	case SDLK_END: priChar = shiftChar = capitalChar = KEY_END; break;
	case SDLK_DOWN: priChar = shiftChar = capitalChar = KEY_DOWN; break;
	case SDLK_PAGEDOWN: priChar = shiftChar = capitalChar = KEY_PGDN; break;
	case SDLK_INSERT: priChar = shiftChar = capitalChar = KEY_INS; break;
	case SDLK_DELETE: priChar = shiftChar = capitalChar = KEY_DEL; break;

#if(defined(SPANISH))
	// other keys
	case 0x56: priChar = capitalChar = '<'; shiftChar = '>'; break;
#endif

	// other keys found in Japanese keyboard
#if 0   //FIXME: Japanese
	case SDLK_NUMPADCOMMA: priChar = shiftChar = capitalChar = ','; break;
#endif
	case SDLK_KP_EQUALS: priChar = shiftChar = capitalChar = '='; break;
	case SDLK_AT: priChar = shiftChar = capitalChar = '@'; break;
	case SDLK_COLON: priChar = shiftChar = capitalChar = ':'; break;
	case SDLK_UNDERSCORE: priChar = shiftChar = capitalChar = '_'; break;
	default:
		MSG("unhandled key %x\n", scanCode);
	}

	// check flags
	int retFlag = 1;

	// check shift key state
	if( !(flags & K_IGNORE_SHIFT) )
	{
		if( flags & K_IS_SHIFT )
		{
			if( !(skeyState & SHIFT_KEY_MASK) )
				retFlag = 0;
		}
		else
		{
			if( skeyState & SHIFT_KEY_MASK )
				retFlag = 0;
		}
	}

	// check contrl key state
	if( !(flags & K_IGNORE_CTRL) )
	{
		if( flags & K_IS_CTRL )
		{
			if( !(skeyState & CONTROL_KEY_MASK) )
				retFlag = 0;
		}
		else
		{
			if( skeyState & CONTROL_KEY_MASK )
				retFlag = 0;
		}
	}

	// check alt key state
	if( !(flags & K_IGNORE_ALT) )
	{
		if( flags & K_IS_ALT )
		{
			if( !(skeyState & ALT_KEY_MASK) )
				retFlag = 0;
		}
		else
		{
			if( skeyState & ALT_KEY_MASK )
				retFlag = 0;
		}
	}

	// check numpad state
	if( !(flags & K_IGNORE_NUMPAD) )
	{
		if( flags & K_ON_NUMPAD )
		{
			if( !onNumPad )
				retFlag = 0;
		}
		else
		{
			if( onNumPad )
				retFlag = 0;
		}
	}

	unsigned outChar = priChar;
	if( flags & K_TRANSLATE_KEY ) 
	{
#if(defined(FRENCH)||defined(GERMAN)||defined(SPANISH))
		if( (skeyState & GRAPH_KEY_MASK) && altChar )
 		{
			outChar = altChar;
 		}
 		else
 		{
#endif
			if( priChar == capitalChar )
			{
				// non-letter
				outChar = skeyState & SHIFT_KEY_MASK ? shiftChar : priChar;
			}
			else
			{
				// letter
				outChar = skeyState & CAP_LOCK_STATE_MASK ? 
					(skeyState & SHIFT_KEY_MASK ? priChar : capitalChar) :
					(skeyState & SHIFT_KEY_MASK ? shiftChar : priChar) ;
			}
#if(defined(FRENCH)||defined(GERMAN)||defined(SPANISH))
		}
#endif
	}

	if(!retFlag)
		return 0;

	int retFlag2 = (charValue == 0) || outChar == charValue
		|| ((flags & K_IGNORE_SHIFT) && shiftChar == charValue)
		|| ((flags & K_IGNORE_CAP_LOCK) && capitalChar == charValue)
		|| ((flags & K_CASE_INSENSITIVE) && outChar == (unsigned short) tolower(charValue));

	if(retFlag2)
		return outChar;
	else
		return 0;
}
// ------ End of MouseSDL::is_key -------//


// ------ Begin of MouseSDL::is_key -------//
int MouseSDL::is_key(unsigned scanCode, unsigned short skeyState, char *keyStr, unsigned flags)
{
	int len = strlen(keyStr);

	if( len == 0)
		return 0;
	if( len == 1)
		return is_key(scanCode, skeyState, keyStr[0], flags);

	const char *priChar = NULL;
	const char *numLockChar = NULL;
	int onNumPad = 0;

	switch(scanCode)
	{
	case SDLK_F1: numLockChar = priChar = "F1"; break;
	case SDLK_F2: numLockChar = priChar = "F2"; break;
	case SDLK_F3: numLockChar = priChar = "F3"; break;
	case SDLK_F4: numLockChar = priChar = "F4"; break;
	case SDLK_F5: numLockChar = priChar = "F5"; break;
	case SDLK_F6: numLockChar = priChar = "F6"; break;
	case SDLK_F7: numLockChar = priChar = "F7"; break;
	case SDLK_F8: numLockChar = priChar = "F8"; break;
	case SDLK_F9: numLockChar = priChar = "F9"; break;
	case SDLK_F10: numLockChar = priChar = "F10"; break;
	case SDLK_F11: numLockChar = priChar = "F11"; break;
	case SDLK_F12: numLockChar = priChar = "F12"; break;

	case SDLK_KP_7: priChar = "HOME"; numLockChar = "7"; onNumPad = 1; break;
	case SDLK_KP_8: priChar = "UP"; numLockChar = "8"; onNumPad = 1; break;
	case SDLK_KP_9: priChar = "PAGE UP"; numLockChar = "9"; onNumPad = 1; break;
	case SDLK_KP_4: priChar = "LEFT"; numLockChar = "4"; onNumPad = 1; break;
	case SDLK_KP_5: priChar = "CENTER"; numLockChar = "5"; onNumPad = 1; break;
	case SDLK_KP_6: priChar = "RIGHT"; numLockChar = "6"; onNumPad = 1; break;
	case SDLK_KP_1: priChar = "END"; numLockChar = "1"; onNumPad = 1; break;
	case SDLK_KP_2: priChar = "DOWN"; numLockChar = "2"; onNumPad = 1; break;
	case SDLK_KP_3: priChar = "PAGE DOWN"; numLockChar = "3"; onNumPad = 1; break;
	case SDLK_KP_0: priChar = "INSERT"; numLockChar = "0"; onNumPad = 1; break;
	case SDLK_KP_PERIOD: priChar = "DELETE"; numLockChar = "."; onNumPad = 1; break;

	// keys above arrow keys
	case SDLK_HOME: priChar = numLockChar = "HOME"; break;
	case SDLK_UP: priChar = numLockChar = "UP"; break;
	case SDLK_PAGEUP: priChar = numLockChar = "PAGE UP"; break;
	case SDLK_LEFT: priChar = numLockChar = "LEFT"; break;
	case SDLK_RIGHT: priChar = numLockChar = "RIGHT"; break;
	case SDLK_END: priChar = numLockChar = "END"; break;
	case SDLK_DOWN: priChar = numLockChar = "DOWN"; break;
	case SDLK_PAGEDOWN: priChar = numLockChar = "PAGE DOWN"; break;
	case SDLK_INSERT: priChar = numLockChar = "INSERT"; break;
	case SDLK_DELETE: priChar = numLockChar = "DELETE"; break;

	// 104-key only
	case SDLK_LGUI: priChar = numLockChar = "LEFT WINDOW"; break;
	case SDLK_RGUI: priChar = numLockChar = "RIGHT WINDOW"; break;
	case SDLK_MENU: priChar = numLockChar = "APP MENU"; break;
	}

	// check flags
	int retFlag = 1;

	// check shift key state
	if( !(flags & K_IGNORE_SHIFT) )
	{
		if( flags & K_IS_SHIFT )
		{
			if( !(skeyState & SHIFT_KEY_MASK) )
				retFlag = 0;
		}
		else
		{
			if( skeyState & SHIFT_KEY_MASK )
				retFlag = 0;
		}
	}

	// check contrl key state
	if( !(flags & K_IGNORE_CTRL) )
	{
		if( flags & K_IS_CTRL )
		{
			if( !(skeyState & CONTROL_KEY_MASK) )
				retFlag = 0;
		}
		else
		{
			if( skeyState & CONTROL_KEY_MASK )
				retFlag = 0;
		}
	}

	// check alt key state
	if( !(flags & K_IGNORE_ALT) )
	{
		if( flags & K_IS_ALT )
		{
			if( !(skeyState & ALT_KEY_MASK) )
				retFlag = 0;
		}
		else
		{
			if( skeyState & ALT_KEY_MASK )
				retFlag = 0;
		}
	}

	// check numpad state
	if( !(flags & K_IGNORE_NUMPAD) )
	{
		if( flags & K_ON_NUMPAD )
		{
			if( !onNumPad )
				retFlag = 0;
		}
		else
		{
			if( onNumPad )
				retFlag = 0;
		}
	}

	const char *outChar = skeyState & NUM_LOCK_STATE_MASK ? numLockChar : priChar;
	int retFlag2 = outChar ? !strcmp(outChar, keyStr) : 0;

	return retFlag && retFlag2;
}
// ------ End of MouseSDL::is_key -------//


// ------ Begin of MouseDInput::disp_count_start -------//
void MouseSDL::disp_count_start()
{
	// unimplemented
}
// ------ End of MouseDInput::disp_count_start -------//


// ------ Begin of MouseDInput::disp_count_end -------//
void MouseSDL::disp_count_end()
{
	// unimplemented
}
// ------ End of MouseDInput::disp_count_end -------//
