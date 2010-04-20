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

//Filename    : OMOUSE.CPP
//Description : Mouse handling Object

#if(defined(SPANISH))

#include <ALL.h>
#include <OSYS.h>
#include <OVGA.h>
#include <OPOWER.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OMOUSE2.h>
#include <KEY.h>
#include <OVGALOCK.h>
#include <string.h>

#ifdef WIN32
#include <cctype>
#endif

//--- define the size of a buffer for real-time vga screen updating ---//

#define VGA_UPDATE_BUF_SIZE	(100*100)		// 100 x 100

//--------- Define Click Threshold -----------//
//
// Clock tick is incremented 1000 times per second or once per millisecond.
//
// The minimum time interval between consequent mouse click is set to
// = 0.3 seconds
//
//--------------------------------------------//

static DWORD click_threshold = (LONG)(0.3 * 1000);

//------- Define static functions -----------//

LRESULT CALLBACK key_hook_proc(int nCode, WORD wParam, LONG lParam);
// static unsigned translate_key(unsigned scanCode, unsigned short skeyState);

//------- define constant --------//

#define MOUSE_BUFFER_SIZE 100
#define KEYB_BUFFER_SIZE 16
#define DOUBLE_SPEED_THREHOLD 8

//------- define static storage -----------//

static DIDEVICEOBJECTDATA mouse_data[MOUSE_BUFFER_SIZE];
static DIDEVICEOBJECTDATA keyb_data[KEYB_BUFFER_SIZE];
static int update_x1, update_y1, update_x2, update_y2;		// coordination of the last double-buffer update area

//--------- Start of Mouse::Mouse ---------//
//
Mouse::Mouse()
{
	init_flag = 0;
	handle_flicking = 0;
	vga_update_buf = NULL;
	memset(&key_hook_handle, 0, sizeof(HHOOK));
	memset(&direct_input, 0, sizeof(LPDIRECTINPUT));
	memset(&di_mouse_device, 0, sizeof(LPDIRECTINPUTDEVICE));
	memset(&di_keyb_device, 0, sizeof(LPDIRECTINPUTDEVICE));
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
//---------- End of Mouse::Mouse ---------//


//---------- Begin of Mouse::~Mouse --------//
//
// Deinitialize the mouse driver, reset event handler
//
Mouse::~Mouse()
{
	deinit();
}
//------------ End of Mouse::~Mouse --------//


//------------ Start of Mouse::init ------------//
//
void Mouse::init(HINSTANCE hinst, HWND hwnd, LPDIRECTINPUT createdDirectInput)
{
	//-------- set starting position ---------//

	POINT pt;

	GetCursorPos(&pt);

	cur_x = pt.x;
	cur_y = pt.y;

/*
	//------ install keyboard hook ---------//

	key_hook_handle = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC) key_hook_proc, sys.app_hinstance, NULL);

	if( !key_hook_handle )
		err.run( "Failed installing keyboard hook." );
*/
	//-------- initialize DirectInput Mouse device--------//
/*
	direct_mouse_handle = DMouseOpen();

	if( !direct_mouse_handle )
		err.run( "Failed installing direct mouse." );
*/
	HRESULT hr;
	if( createdDirectInput )
	{
		direct_input = createdDirectInput;
		hr = direct_input->AddRef();
	}
	else
	{
		hr = DirectInputCreate(hinst, DIRECTINPUT_VERSION, &direct_input, NULL);
	}
	if(hr)
		err.run( "Failed creating DirectInput");

	// ####### begin Gilbert 12/9 #######//
	{
		VgaFrontLock vgaLock;
		hr = direct_input->CreateDevice(GUID_SysMouse,&di_mouse_device,NULL);
	}
	// ####### end Gilbert 12/9 #######//

	if(hr)
		err.run( "Failed creating mouse interface from DirectInput");

	// ------- set cooperative level --------//
	hr = di_mouse_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);

	// ------- set data format ---------//
	if(!hr)
		hr = di_mouse_device->SetDataFormat(&c_dfDIMouse);

	// ------- set relative coordinate mode --------//
// DirectInput default is relative
	if(!hr)
	{
		DIPROPDWORD propDword;
		propDword.diph.dwSize = sizeof(DIPROPDWORD);
		propDword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		propDword.diph.dwHow = DIPH_DEVICE;			// Entire device
		propDword.diph.dwObj =  0;						// Entire device, so zero
		propDword.dwData = DIPROPAXISMODE_REL;
		hr = di_mouse_device->SetProperty(DIPROP_AXISMODE, &propDword.diph);
	}

	// ------- set buffer size --------//
	if(!hr)
	{
		DIPROPDWORD propDword;
		propDword.diph.dwSize = sizeof(DIPROPDWORD);
		propDword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		propDword.diph.dwHow = DIPH_DEVICE;			// Entire device
		propDword.diph.dwObj =  0;						// Entire device, so zero
		propDword.dwData = MOUSE_BUFFER_SIZE * sizeof(DIDEVICEOBJECTDATA);
		hr = di_mouse_device->SetProperty(DIPROP_BUFFERSIZE, &propDword.diph);
	}

	if(hr)
		err.run( "Failed initializing mouse interface");


	// ------- create direct input keyboard interface --------- //
	// ####### begin Gilbert 12/9 #######//
	{
		VgaFrontLock vgaLock;
		hr = direct_input->CreateDevice(GUID_SysKeyboard,&di_keyb_device,NULL);
	}
	// ####### end Gilbert 12/9 #######//

	if(hr)
		err.run( "Failed creating keyboard interface from DirectInput");

	// ------- set cooperative level --------//
	hr = di_keyb_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	// ------- set data format ---------//
	if(!hr)
		hr = di_keyb_device->SetDataFormat(&c_dfDIKeyboard);

	// ------- set buffer size --------//
	if(!hr)
	{
		DIPROPDWORD propDword;
		propDword.diph.dwSize = sizeof(DIPROPDWORD);
		propDword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		propDword.diph.dwHow = DIPH_DEVICE;			// Entire device
		propDword.diph.dwObj =  0;						// Entire device, so zero
		propDword.dwData = KEYB_BUFFER_SIZE * sizeof(DIDEVICEOBJECTDATA);
		hr = di_keyb_device->SetProperty(DIPROP_BUFFERSIZE, &propDword.diph);
	}

	if(hr)
		err.run( "Failed initializing keyboard interface");

	// ------- get initial keyboard state ----------//
	skey_state = 0;
	if(GetKeyState(VK_LSHIFT) < 0)
		skey_state |= LEFT_SHIFT_KEY_MASK;
	if(GetKeyState(VK_RSHIFT) < 0)
		skey_state |= RIGHT_SHIFT_KEY_MASK;
	if(GetKeyState(VK_LCONTROL) < 0)
		skey_state |= LEFT_CONTROL_KEY_MASK;
	if(GetKeyState(VK_RCONTROL) < 0)
		skey_state |= RIGHT_CONTROL_KEY_MASK;
	if(GetKeyState(VK_LMENU) < 0)
		skey_state |= LEFT_ALT_KEY_MASK;
	if(GetKeyState(VK_RMENU) < 0)
		skey_state |= GRAPH_KEY_MASK;
	if(GetKeyState(VK_NUMLOCK) & 1)
		skey_state |= NUM_LOCK_STATE_MASK;
	if(GetKeyState(VK_CAPITAL) & 1)
		skey_state |= CAP_LOCK_STATE_MASK;
	if(GetKeyState(VK_SCROLL) & 1)
		skey_state |= SCROLL_LOCK_STATE_MASK;
	skey_state |= INSERT_STATE_MASK;		// enable insert mode by default

	//------- initialize VGA update buffer -------//

	vga_update_buf = mem_add( VGA_UPDATE_BUF_SIZE );

	// ------ initialize mouse boundary ---------//
	reset_boundary();

	// ------- initialize event queue ---------//
	head_ptr = tail_ptr = 0;

	//--------------------------------------------//

	init_flag = 1;
}
//------------- End of Mouse::init -------------//


//------------ Start of Mouse::deinit ------------//
//
void Mouse::deinit()
{
	if( init_flag )
	{
		// UnhookWindowsHookEx(key_hook_handle);

		// DMouseClose(direct_mouse_handle);
		init_flag = 0;
	}

	if( vga_update_buf )
	{
		mem_del(vga_update_buf);
		vga_update_buf = NULL;
	}

	if( di_keyb_device )
	{
		di_keyb_device->Unacquire();
		di_keyb_device->Release();
		di_keyb_device = NULL;
	}
	if( di_mouse_device )
	{
		di_mouse_device->Unacquire();
		di_mouse_device->Release();
		di_mouse_device = NULL;
	}
	if( direct_input )
	{
		direct_input->Release();
		direct_input = NULL;
	}
}
//------------- End of Mouse::deinit -------------//


//--------- Start of Mouse::hide -------//
//
// Suspend the mouse function, use resume() to resume to function
//
void Mouse::hide()
{
	mouse_cursor.hide_all_flag=1;

	mouse_cursor.process(cur_x, cur_y);
}
//---------- End of Mouse::hide --------//


//--------- Start of Mouse::show -------//
//
// Resume the mouse function which is previously hideed by hide()
//
void Mouse::show()
{
	mouse_cursor.hide_all_flag=0;

	mouse_cursor.process(cur_x, cur_y);
}
//---------- End of Mouse::show --------//


//--------- Begin of Mouse::hide_area ----------//
//
void Mouse::hide_area(int x1, int y1, int x2, int y2)
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
//--------- End of Mouse::hide_area --------------//


//--------- Begin of Mouse::show_area ----------//
//
void Mouse::show_area()
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
//--------- End of Mouse::show_area --------------//


//--------- Start of Mouse::add_event ---------//
//
// Called by handler interrupt to procss the state
//
void Mouse::add_event(MouseEvent *mouseEvent)
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
//----------- End of Mouse::add_event ----------//


//--------- Start of Mouse::add_key_event ---------//
//
// Called by key handler to save the key pressed
//
void Mouse::add_key_event(unsigned scanCode, DWORD timeStamp)
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
//----------- End of Mouse::add_key_event ----------//


//--------- Start of Mouse::get_event ---------//
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
int Mouse::get_event()
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
		key_code = mouse.is_key(mouse.scan_code, mouse.event_skey_state, (WORD) 0, K_CHAR_KEY);
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
//----------- End of Mouse::get_event ----------//


//--------- Begin of Mouse::in_area ----------//
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
int Mouse::in_area(int x1, int y1, int x2, int y2)
{
	return( cur_x >= x1 && cur_y >= y1 && cur_x <= x2 && cur_y <= y2 );
}
//--------- End of Mouse::in_area --------------//


//--------- Begin of Mouse::press_area ----------//
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
int Mouse::press_area(int x1, int y1, int x2, int y2, int buttonId)
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
//--------- End of Mouse::press_area --------------//


//--------- Begin of Mouse::set_boundary ----------//
//
// for each parameter, put -1 to mean unchange
//
void Mouse::set_boundary(int x1, int y1, int x2, int y2)
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
//--------- End of Mouse::set_boundary ----------//


//--------- Begin of Mouse::reset_boundary ----------//
void Mouse::reset_boundary()
{
	bound_x1 = 0;
	bound_y1 = 0;
	bound_x2 = MOUSE_X_UPPER_LIMIT;
	bound_y2 = MOUSE_Y_UPPER_LIMIT;
}
//--------- End of Mouse::set_boundary ----------//


//--------- Begin of Mouse::single_click ----------//
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
int Mouse::single_click(int x1, int y1, int x2, int y2,int buttonId)
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
//--------- End of Mouse::single_click --------------//


//--------- Begin of Mouse::double_click ----------//
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
int Mouse::double_click(int x1, int y1, int x2, int y2,int buttonId)
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
//--------- End of Mouse::double_click --------------//


//--------- Begin of Mouse::any_click ----------//
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
int Mouse::any_click(int x1, int y1, int x2, int y2,int buttonId)
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
//--------- End of Mouse::any_click --------------//


//--------- Begin of Mouse::any_click ----------//
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
int Mouse::any_click(int buttonId)
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
//--------- End of Mouse::any_click --------------//


//--------- Begin of Mouse::release_click ----------//
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
int Mouse::release_click(int x1, int y1, int x2, int y2,int buttonId)
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
//--------- End of Mouse::release_click --------------//


//------- Begin of function key_hook_proc --------//

LRESULT CALLBACK key_hook_proc(int nCode, WORD wParam, LONG lParam)
{
	if (nCode >= 0)
		mouse.add_key_event(wParam, m.get_time());

	return CallNextHookEx(mouse.key_hook_handle, nCode, wParam, lParam);
}
//-------- End of function key_hook_proc --------//


//--------- Begin of Mouse::poll_event ----------//
//
// Poll mouse events from the direct mouse VXD.
//
void Mouse::poll_event()
{
	if(!init_flag)
		return;

	// ---- acquire mouse device and count the message queued ----//
	HRESULT hr;
	if( (hr = di_mouse_device->Acquire()) != DI_OK && hr != S_FALSE)
		return;

	if( (hr = di_keyb_device->Acquire()) != DI_OK && hr != S_FALSE)
		return;

	// HRESULT hr;
	DWORD mouseLen, keybLen;

	int moveFlag = 0;
	while( 1 )
	{
		mouseLen = MOUSE_BUFFER_SIZE;
		keybLen = KEYB_BUFFER_SIZE;

		if( (hr = di_mouse_device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), 
				mouse_data, &mouseLen, 0)) != DI_OK && hr != S_FALSE )
			mouseLen = 0;
		if( (hr = di_keyb_device->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), 
				keyb_data, &keybLen, 0)) != DI_OK && hr != S_FALSE)
			keybLen = 0;

		if( !mouseLen && !keybLen )
			break;

		DIDEVICEOBJECTDATA *mouseMsg = mouse_data, *keybMsg = keyb_data;

		while( mouseLen > 0 || keybLen > 0)
		{
			// merge mouse event and keyboard event
			MouseEvent ev;
			int earlyDevice = 0;			// 1 = mouse, 2 = keyboard
			if( mouseLen > 0 && keybLen > 0 )
			{
				if( DISEQUENCE_COMPARE(mouseMsg->dwSequence, <=, keybMsg->dwSequence) )
					earlyDevice = 1;
				else
					earlyDevice = 2;
			}
			else if( mouseLen > 0)
			{
				earlyDevice = 1;
			}
			else if( keybLen > 0)
			{
				earlyDevice = 2;
			}

			if( earlyDevice == 1 )
			{
				if (mouseMsg->dwOfs == DIMOFS_BUTTON0)
				{
					if( mouseMsg->dwData & 0x80)
					{
						// mouse button pressed
						//ev.state = left_press = LEFT_BUTTON_MASK;
						left_press = LEFT_BUTTON_MASK;
						ev.event_type = LEFT_BUTTON;
						ev.x = cur_x;
						ev.y = cur_y;
						ev.time = mouseMsg->dwTimeStamp;
						ev.scan_code = 0;
						ev.skey_state = skey_state;
						add_event(&ev);
					}
					else
					{
						// mouse button released
						left_press = 0;
						ev.event_type = LEFT_BUTTON_RELEASE;
						ev.x = cur_x;
						ev.y = cur_y;
						ev.time = mouseMsg->dwTimeStamp;
						ev.scan_code = 0;
						ev.skey_state = skey_state;
						add_event(&ev);
						reset_boundary();			// reset_boundary whenever left button is released
					}
				}
				else if (mouseMsg->dwOfs == DIMOFS_BUTTON1)
				{
					if( mouseMsg->dwData & 0x80)
					{
						// mouse button pressed
						right_press = RIGHT_BUTTON_MASK;
						ev.event_type = RIGHT_BUTTON;
						ev.x = cur_x;
						ev.y = cur_y;
						ev.time = mouseMsg->dwTimeStamp;
						ev.scan_code = 0;
						ev.skey_state = skey_state;
						add_event(&ev);
					}
					else
					{
						// mouse button released
						right_press = 0;
						ev.event_type = RIGHT_BUTTON_RELEASE;
						ev.x = cur_x;
						ev.y = cur_y;
						ev.time = mouseMsg->dwTimeStamp;
						ev.scan_code = 0;
						ev.skey_state = skey_state;
						add_event(&ev);
					}
				}
				else if (mouseMsg->dwOfs == DIMOFS_BUTTON2)
				{
					// not interested
				}
				else if (mouseMsg->dwOfs == DIMOFS_BUTTON3)
				{
					// not interested
				}
				else if (mouseMsg->dwOfs == DIMOFS_X)
				{
					cur_x += micky_to_displacement(mouseMsg->dwData);
					if(cur_x < bound_x1)
						cur_x = bound_x1;
					if(cur_x > bound_x2)
						cur_x = bound_x2;
					moveFlag = 1;
				}
				else if (mouseMsg->dwOfs == DIMOFS_Y)
				{
					cur_y += micky_to_displacement(mouseMsg->dwData);
					if(cur_y < bound_y1)
						cur_y = bound_y1;
					if(cur_y > bound_y2)
						cur_y = bound_y2;
					moveFlag = 1;
				}
				else if (mouseMsg->dwOfs == DIMOFS_Z)
				{
					// not interested
				}
				--mouseLen;
				++mouseMsg;
			}
			else if( earlyDevice == 2 )
			{
				// trap keys, such as shift, ctrl, alt, numlock, caplock and scrolllock
				// add to event if the key is none of the above
				switch( keybMsg->dwOfs )
				{
				case DIK_LSHIFT:
					if( keybMsg->dwData & 0x80 )
						skey_state |= LEFT_SHIFT_KEY_MASK;
					else
						skey_state &= ~LEFT_SHIFT_KEY_MASK;
					break;
				case DIK_RSHIFT:
					if( keybMsg->dwData & 0x80 )
						skey_state |= RIGHT_SHIFT_KEY_MASK;
					else
						skey_state &= ~RIGHT_SHIFT_KEY_MASK;
					break;
				case DIK_LCONTROL:
					if( keybMsg->dwData & 0x80 )
						skey_state |= LEFT_CONTROL_KEY_MASK;
					else
						skey_state &= ~LEFT_CONTROL_KEY_MASK;
					break;
				case DIK_RCONTROL:
					if( keybMsg->dwData & 0x80 )
						skey_state |= RIGHT_CONTROL_KEY_MASK;
					else
						skey_state &= ~RIGHT_CONTROL_KEY_MASK;
					break;
				case DIK_LMENU:
					if( keybMsg->dwData & 0x80 )
						skey_state |= LEFT_ALT_KEY_MASK;
					else
						skey_state &= ~LEFT_ALT_KEY_MASK;
					break;
				case DIK_RMENU:
					if( keybMsg->dwData & 0x80 )
						skey_state |= GRAPH_KEY_MASK;
					else
						skey_state &= ~GRAPH_KEY_MASK;
					break;
				case DIK_CAPITAL:
					if(keybMsg->dwData & 0x80)
						skey_state ^= CAP_LOCK_STATE_MASK;
					break;
				case DIK_NUMLOCK:
					if(keybMsg->dwData & 0x80)
						skey_state ^= NUM_LOCK_STATE_MASK;
					break;
				case DIK_SCROLL:
					if(keybMsg->dwData & 0x80)
						skey_state ^= SCROLL_LOCK_STATE_MASK;
					break;
				case DIK_INSERT:
					if(keybMsg->dwData & 0x80)
					{
						// insert is a special case, it toggle skey_state and
						// append event queue
						skey_state ^= INSERT_STATE_MASK;
						add_key_event(keybMsg->dwOfs, keybMsg->dwTimeStamp);
					}
					break;
				default:
					if( keybMsg->dwData & 0x80 )
					{
						add_key_event(keybMsg->dwOfs, keybMsg->dwTimeStamp);
						// ####### begin Gilbert 29/8 ######//
						// capture screen
						if(keybMsg->dwOfs == DIK_F11 && (skey_state & CONTROL_KEY_MASK))
						{
							sys.capture_screen();
						}
						// ####### end Gilbert 29/8 ######//
					}
				}
				--keybLen;
				++keybMsg;
			}
		}
	}

	// ####### begin Gilbert 12/4 ########//
	if(moveFlag)
	{
		mouse_cursor.process(cur_x, cur_y);     // repaint mouse cursor
		power.mouse_handler();
	}
	// ####### end Gilbert 12/4 ########//
}
//--------- End of Mouse::poll_event --------------//


// ####### begin Gilbert 31/10 #########//
//--------- Begin of Mouse::update_skey_state ----------//
// called after task switch to get the lastest state of ctrl/alt/shift key
void Mouse::update_skey_state()
{
	// ------- get initial keyboard state ----------//
	skey_state = 0;
	if(GetKeyState(VK_LSHIFT) < 0)
		skey_state |= LEFT_SHIFT_KEY_MASK;
	if(GetKeyState(VK_RSHIFT) < 0)
		skey_state |= RIGHT_SHIFT_KEY_MASK;
	if(GetKeyState(VK_LCONTROL) < 0)
		skey_state |= LEFT_CONTROL_KEY_MASK;
	if(GetKeyState(VK_RCONTROL) < 0)
		skey_state |= RIGHT_CONTROL_KEY_MASK;
	if(GetKeyState(VK_LMENU) < 0)
		skey_state |= LEFT_ALT_KEY_MASK;
	if(GetKeyState(VK_RMENU) < 0)
		skey_state |= GRAPH_KEY_MASK;
	if(GetKeyState(VK_NUMLOCK) & 1)
		skey_state |= NUM_LOCK_STATE_MASK;
	if(GetKeyState(VK_CAPITAL) & 1)
		skey_state |= CAP_LOCK_STATE_MASK;
	if(GetKeyState(VK_SCROLL) & 1)
		skey_state |= SCROLL_LOCK_STATE_MASK;
	skey_state |= INSERT_STATE_MASK;		// enable insert mode by default
}
//--------- End of Mouse::update_skey_state ----------//
// ####### end Gilbert 31/10 #########//


//--------- Begin of Mouse::wait_press ----------//
//
// Wait until one of the mouse buttons is pressed.
//
// [int] timeOutSecond - no. of seconds to time out. If not
//								 given, there will be no time out.
//
// return: <int> 1-left mouse button
//					  2-right mouse button
//
int Mouse::wait_press(int timeOutSecond)
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
//--------- End of Mouse::wait_press --------------//


//--------- Begin of Mouse::reset_click ----------//
//
// Reset queued mouse clicks.
//
void Mouse::reset_click()
{
	click_buffer[0].count=0;
	click_buffer[1].count=0;
}
//--------- End of Mouse::reset_click --------------//


// ------ Begin of Mouse::mickey_to_displacment -------//
long Mouse::micky_to_displacement(DWORD w)
{
	long d = (long)w ;
	// long a = abs(d);
	// return a >= double_speed_threshold ? (a >= triple_speed_threshold ? 3 * d : 2 * d) : d;
	return abs(d) >= double_speed_threshold ? d+d : d;
}
// ------ End of Mouse::mickey_to_displacment -------//


// ------ Begin of Mouse::is_key -------//
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
int Mouse::is_key(unsigned scanCode, unsigned short skeyState, unsigned short charValue, unsigned flags)
{
	unsigned short priChar = 0, shiftChar = 0, capitalChar = 0, altChar = 0;
	unsigned onNumPad = 0;

	switch(scanCode)
	{
	case DIK_ESCAPE: priChar = shiftChar = capitalChar = KEY_ESC; break;
	case DIK_1: priChar = capitalChar = '1'; shiftChar = '!'; altChar = '|'; break;
	case DIK_2: priChar = capitalChar = '2'; shiftChar = '\"'; altChar = '@'; break;
	case DIK_3: priChar = capitalChar = '3'; shiftChar = (UCHAR)'·'; altChar = '#'; break;
	case DIK_4: priChar = capitalChar = '4'; shiftChar = '$'; break;
	case DIK_5: priChar = capitalChar = '5'; shiftChar = '%'; break;
	case DIK_6: priChar = capitalChar = '6'; shiftChar = '&'; break;
	case DIK_7: priChar = capitalChar = '7'; shiftChar = '/'; break;
	case DIK_8: priChar = capitalChar = '8'; shiftChar = '('; break;
	case DIK_9: priChar = capitalChar = '9'; shiftChar = ')'; break;
	case DIK_0: priChar = capitalChar = '0'; shiftChar = '='; break;
	//case DIK_MINUS: priChar = capitalChar = '-'; shiftChar = '_'; break;
	case 0x0c:  priChar = capitalChar = '\''; shiftChar = '?'; break;
	//case DIK_EQUALS: priChar = capitalChar = '='; shiftChar = '+'; break;
	case 0x0d:  priChar = capitalChar = (UCHAR)'¡'; shiftChar = (UCHAR)'¿'; break;
	case DIK_BACK: priChar = capitalChar = shiftChar = KEY_BACK_SPACE; break;   // backspace
	case DIK_TAB: priChar = capitalChar = shiftChar = KEY_TAB; break;
	case DIK_Q: priChar = 'q'; capitalChar = shiftChar = 'Q'; break;
	case DIK_W: priChar = 'w'; capitalChar = shiftChar = 'W'; break;
	case DIK_E: priChar = 'e'; capitalChar = shiftChar = 'E'; break;
	case DIK_R: priChar = 'r'; capitalChar = shiftChar = 'R'; break;
	case DIK_T: priChar = 't'; capitalChar = shiftChar = 'T'; break;
	case DIK_Y: priChar = 'y'; capitalChar = shiftChar = 'Y'; break;
	case DIK_U: priChar = 'u'; capitalChar = shiftChar = 'U'; break;
	case DIK_I: priChar = 'i'; capitalChar = shiftChar = 'I'; break;
	case DIK_O: priChar = 'o'; capitalChar = shiftChar = 'O'; break;
	case DIK_P: priChar = 'p'; capitalChar = shiftChar = 'P'; break;
//	case DIK_LBRACKET: priChar = capitalChar = '['; shiftChar = '{'; break;
	case 0x1a:  priChar = capitalChar = '`'; shiftChar = '^'; altChar = '['; break;
//	case DIK_RBRACKET: priChar = capitalChar = ']'; shiftChar = '}'; break;
	case 0x1b:  priChar = capitalChar = '+';shiftChar = '*'; altChar = ']'; break;
	case DIK_NUMPADENTER:		// Enter on numeric keypad
		onNumPad = 1;			// fall through
	case DIK_RETURN: priChar = capitalChar = shiftChar = KEY_RETURN;	break;
	case DIK_A: priChar = 'a'; capitalChar = shiftChar = 'A'; break;
	case DIK_S: priChar = 's'; capitalChar = shiftChar = 'S'; break;
	case DIK_D: priChar = 'd'; capitalChar = shiftChar = 'D'; break;
	case DIK_F: priChar = 'f'; capitalChar = shiftChar = 'F'; break;
	case DIK_G: priChar = 'g'; capitalChar = shiftChar = 'G'; break;
	case DIK_H: priChar = 'h'; capitalChar = shiftChar = 'H'; break;
	case DIK_J: priChar = 'j'; capitalChar = shiftChar = 'J'; break;
	case DIK_K: priChar = 'k'; capitalChar = shiftChar = 'K'; break;
	case DIK_L: priChar = 'l'; capitalChar = shiftChar = 'L'; break;
	//case DIK_SEMICOLON: priChar = capitalChar = ';'; shiftChar = ':'; altChar = '~'; break;
	case 0x27:  priChar = (UCHAR)'ñ'; capitalChar = shiftChar = (UCHAR)'Ñ'; break;
	//case DIK_APOSTROPHE: priChar = capitalChar = '\''; shiftChar = '\"'; break;
	case 0x28:  priChar = capitalChar = (UCHAR)'´'; shiftChar = (UCHAR)'¨'; altChar = '{'; break;
	//case DIK_GRAVE: priChar = capitalChar = '~'; shiftChar = '`'; break;
	case 0x29:  priChar = capitalChar = (UCHAR)'º'; shiftChar = (UCHAR)'ª'; altChar = '\\'; break;
//	case DIK_BACKSLASH: priChar = capitalChar = '\\'; shiftChar = '|'; break;
	case 0x2b:  priChar = (UCHAR)'ç'; capitalChar = shiftChar = (UCHAR)'Ç'; altChar = '}'; break;
	case DIK_Z: priChar = 'z'; capitalChar = shiftChar = 'Z'; break;
	case DIK_X: priChar = 'x'; capitalChar = shiftChar = 'X'; break;
	case DIK_C: priChar = 'c'; capitalChar = shiftChar = 'C'; break;
	case DIK_V: priChar = 'v'; capitalChar = shiftChar = 'V'; break;
	case DIK_B: priChar = 'b'; capitalChar = shiftChar = 'B'; break;
	case DIK_N: priChar = 'n'; capitalChar = shiftChar = 'N'; break;
	case DIK_M: priChar = 'm'; capitalChar = shiftChar = 'M'; break;
	case DIK_COMMA: priChar = capitalChar = ','; shiftChar = ';'; break;
	case DIK_PERIOD: priChar = capitalChar = '.'; shiftChar = ':'; break;
	// case DIK_SLASH: priChar = capitalChar = '/'; shiftChar = '\?'; break;
	case 0x35:  priChar = capitalChar = '-'; shiftChar = '_'; break;
	case DIK_MULTIPLY: priChar = capitalChar = shiftChar = '*'; onNumPad = 1; break; // * on numeric keypad
	case DIK_SPACE: priChar = capitalChar = shiftChar = ' '; break;
	case DIK_ADD: priChar = capitalChar = shiftChar = '+'; onNumPad = 1; break; // + on numeric keypad
	case DIK_DIVIDE: priChar = capitalChar = shiftChar = '/'; onNumPad = 1; break;		// / on numeric keypad
	case DIK_SUBTRACT: priChar = capitalChar = shiftChar = '-'; onNumPad = 1; break;	// - on numeric keypad
		
	case DIK_NUMPAD7: priChar = shiftChar = capitalChar = '7'; onNumPad = 1; break;
	case DIK_NUMPAD8: priChar = shiftChar = capitalChar = '8'; onNumPad = 1; break;
	case DIK_NUMPAD9: priChar = shiftChar = capitalChar = '9'; onNumPad = 1; break;
	case DIK_NUMPAD4: priChar = shiftChar = capitalChar = '4'; onNumPad = 1; break;
	case DIK_NUMPAD5: priChar = shiftChar = capitalChar = '5'; onNumPad = 1; break;
	case DIK_NUMPAD6: priChar = shiftChar = capitalChar = '6'; onNumPad = 1; break;
	case DIK_NUMPAD1: priChar = shiftChar = capitalChar = '1'; onNumPad = 1; break;
	case DIK_NUMPAD2: priChar = shiftChar = capitalChar = '2'; onNumPad = 1; break;
	case DIK_NUMPAD3: priChar = shiftChar = capitalChar = '3'; onNumPad = 1; break;
	case DIK_NUMPAD0: priChar = shiftChar = capitalChar = '0'; onNumPad = 1; break;
	case DIK_DECIMAL: priChar = shiftChar = capitalChar = '.'; onNumPad = 1; break;

	// function keys
	case DIK_F1: priChar = shiftChar = capitalChar = KEY_F1; break;
	case DIK_F2: priChar = shiftChar = capitalChar = KEY_F2; break;
	case DIK_F3: priChar = shiftChar = capitalChar = KEY_F3; break;
	case DIK_F4: priChar = shiftChar = capitalChar = KEY_F4; break;
	case DIK_F5: priChar = shiftChar = capitalChar = KEY_F5; break;
	case DIK_F6: priChar = shiftChar = capitalChar = KEY_F6; break;
	case DIK_F7: priChar = shiftChar = capitalChar = KEY_F7; break;
	case DIK_F8: priChar = shiftChar = capitalChar = KEY_F8; break;
	case DIK_F9: priChar = shiftChar = capitalChar = KEY_F9; break;
	case DIK_F10: priChar = shiftChar = capitalChar = KEY_F10; break;
	case DIK_F11: priChar = shiftChar = capitalChar = KEY_F11; break;
	case DIK_F12: priChar = shiftChar = capitalChar = KEY_F12; break;

	// arrow keys
	case DIK_HOME: priChar = shiftChar = capitalChar = KEY_HOME; break;
	case DIK_UP: priChar = shiftChar = capitalChar = KEY_UP; break;
	case DIK_PRIOR: priChar = shiftChar = capitalChar = KEY_PGUP; break;
	case DIK_LEFT: priChar = shiftChar = capitalChar = KEY_LEFT; break;
	case DIK_RIGHT: priChar = shiftChar = capitalChar = KEY_RIGHT; break;
	case DIK_END: priChar = shiftChar = capitalChar = KEY_END; break;
	case DIK_DOWN: priChar = shiftChar = capitalChar = KEY_DOWN; break;
	case DIK_NEXT: priChar = shiftChar = capitalChar = KEY_PGDN; break;
	case DIK_INSERT: priChar = shiftChar = capitalChar = KEY_INS; break;
	case DIK_DELETE: priChar = shiftChar = capitalChar = KEY_DEL; break;

	// other keys
	case 0x56: priChar = capitalChar = '<'; shiftChar = '>'; break;

	// other keys found in Japanese keyboard
	case DIK_NUMPADCOMMA: priChar = shiftChar = capitalChar = ','; break;
	case DIK_NUMPADEQUALS: priChar = shiftChar = capitalChar = '='; break;
	case DIK_AT: priChar = shiftChar = capitalChar = '@'; break;
	case DIK_COLON: priChar = shiftChar = capitalChar = ':'; break;
	case DIK_UNDERLINE: priChar = shiftChar = capitalChar = '_'; break;
	}

	// BUGHERE : numpad key is not translated when numlock is off

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
		if( (skeyState & GRAPH_KEY_MASK) && altChar )
		{
			outChar = altChar;
		}
		else
		{
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
		}
	}

	if(!retFlag)
		return 0;

	int retFlag2 = (charValue == 0) || outChar == charValue
		|| ((flags & K_IGNORE_SHIFT) && shiftChar == charValue)
		|| ((flags & K_IGNORE_CAP_LOCK) && capitalChar == charValue)
#ifdef WIN32
		|| ((flags & K_CASE_INSENSITIVE) && outChar == (unsigned short) std::tolower(charValue));
#else
		|| ((flags & K_CASE_INSENSITIVE) && outChar == (unsigned short) tolower(charValue));
#endif

	if(retFlag2)
		return outChar;
	else
		return 0;
}
// ------ End of Mouse::is_key -------//


// ------ Begin of Mouse::is_key -------//
int Mouse::is_key(unsigned scanCode, unsigned short skeyState, char *keyStr, unsigned flags)
{
	int len = strlen(keyStr);
	if( len == 0)
		return 0;
	if( len == 1)
		return is_key(scanCode, skeyState, keyStr[0], flags);

	char *priChar = NULL;
	char *numLockChar = NULL;
	int onNumPad = 0;

	switch(scanCode)
	{
	case DIK_F1: numLockChar = priChar = "F1"; break;
	case DIK_F2: numLockChar = priChar = "F2"; break;
	case DIK_F3: numLockChar = priChar = "F3"; break;
	case DIK_F4: numLockChar = priChar = "F4"; break;
	case DIK_F5: numLockChar = priChar = "F5"; break;
	case DIK_F6: numLockChar = priChar = "F6"; break;
	case DIK_F7: numLockChar = priChar = "F7"; break;
	case DIK_F8: numLockChar = priChar = "F8"; break;
	case DIK_F9: numLockChar = priChar = "F9"; break;
	case DIK_F10: numLockChar = priChar = "F10"; break;
	case DIK_F11: numLockChar = priChar = "F11"; break;
	case DIK_F12: numLockChar = priChar = "F12"; break;

	case DIK_NUMPAD7: priChar = "HOME"; numLockChar = "7"; onNumPad = 1; break;
	case DIK_NUMPAD8: priChar = "UP"; numLockChar = "8"; onNumPad = 1; break;
	case DIK_NUMPAD9: priChar = "PAGE UP"; numLockChar = "9"; onNumPad = 1; break;
	case DIK_NUMPAD4: priChar = "LEFT"; numLockChar = "4"; onNumPad = 1; break;
	case DIK_NUMPAD5: priChar = "CENTER"; numLockChar = "5"; onNumPad = 1; break;
	case DIK_NUMPAD6: priChar = "RIGHT"; numLockChar = "6"; onNumPad = 1; break;
	case DIK_NUMPAD1: priChar = "END"; numLockChar = "1"; onNumPad = 1; break;
	case DIK_NUMPAD2: priChar = "DOWN"; numLockChar = "2"; onNumPad = 1; break;
	case DIK_NUMPAD3: priChar = "PAGE DOWN"; numLockChar = "3"; onNumPad = 1; break;
	case DIK_NUMPAD0: priChar = "INSERT"; numLockChar = "0"; onNumPad = 1; break;
	case DIK_DECIMAL: priChar = "DELETE"; numLockChar = "."; onNumPad = 1; break;

	// keys above arrow keys
	case DIK_HOME: priChar = numLockChar = "HOME"; break;
	case DIK_UP: priChar = numLockChar = "UP"; break;
	case DIK_PRIOR: priChar = numLockChar = "PAGE UP"; break;
	case DIK_LEFT: priChar = numLockChar = "LEFT"; break;
	case DIK_RIGHT: priChar = numLockChar = "RIGHT"; break;
	case DIK_END: priChar = numLockChar = "END"; break;
	case DIK_DOWN: priChar = numLockChar = "DOWN"; break;
	case DIK_NEXT: priChar = numLockChar = "PAGE DOWN"; break;
	case DIK_INSERT: priChar = numLockChar = "INSERT"; break;
	case DIK_DELETE: priChar = numLockChar = "DELETE"; break;

	// 104-key only
	case DIK_LWIN: priChar = numLockChar = "LEFT WINDOW"; break;
	case DIK_RWIN: priChar = numLockChar = "RIGHT WINDOW"; break;
	case DIK_APPS: priChar = numLockChar = "APP MENU"; break;
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

	char *outChar = skeyState & NUM_LOCK_STATE_MASK ? numLockChar : priChar;
	int retFlag2 = outChar ? !strcmp(outChar, keyStr) : 0;

	return retFlag && retFlag2;
}
// ------ End of Mouse::is_key -------//

#endif
