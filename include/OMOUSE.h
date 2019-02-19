/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2015 Jesse Allen
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

//Filename    : OMOUSE.H
//Description : Input event class

#ifndef __OMOUSE_H
#define __OMOUSE_H

//-------- Define macro constant --------//
//
// Button id, for Mouse internal use only,
// not appeal to mouse driver parameters
//
//---------------------------------------//

// event_type
enum MouseEventType
{
	LEFT_BUTTON = 0,
	RIGHT_BUTTON = LEFT_BUTTON+1,
	KEY_PRESS = 2,
	LEFT_BUTTON_RELEASE = 4,
	RIGHT_BUTTON_RELEASE = LEFT_BUTTON_RELEASE+1,
	KEY_RELEASE = 6,
};

//------- Define struct MouseEvent --------//

struct MouseEvent               // event buffer structure
{
	MouseEventType	event_type;   // mouse state

	unsigned long  time;               // time event occurred
	unsigned short skey_state;   // speical key state, such as LEFT_SHIFT_KEY_MASK ...
	// int	state;              // state mask of mouse button

	int      x, y;               // mousecursor coordinates
	unsigned scan_code;          // if scan_code>0 then it's a key press event
};

#define LEFT_BUTTON_MASK	1
#define RIGHT_BUTTON_MASK	2


// bit flag of any skey_state, event_skey_state
#define LEFT_SHIFT_KEY_MASK 0x001
#define RIGHT_SHIFT_KEY_MASK 0x002
#define SHIFT_KEY_MASK 0x003
#define LEFT_CONTROL_KEY_MASK 0x004
#define RIGHT_CONTROL_KEY_MASK 0x008
#define CONTROL_KEY_MASK 0x00c
#define LEFT_ALT_KEY_MASK 0x010
#define RIGHT_ALT_KEY_MASK 0x020
#define ALT_KEY_MASK 0x030
#define NUM_LOCK_STATE_MASK 0x040
#define CAP_LOCK_STATE_MASK 0x080
#define SCROLL_LOCK_STATE_MASK 0x100
#define INSERT_STATE_MASK 0x200
#define GRAPH_KEY_MASK 0x400

//----- Define the upper limit for mouse coordination ------//

#define MOUSE_X_UPPER_LIMIT   (VGA_WIDTH-5)
#define MOUSE_Y_UPPER_LIMIT	(VGA_HEIGHT-5)

//------ Default settting ---------//

#define DEFAULT_DOUBLE_SPEED_THRESHOLD 8
#define DEFAULT_TRIPLE_SPEED_THRESHOLD 16

//-------- Define struct MouseClick -------//

struct MouseClick               // MultiClick buffer structure
{
	int x, y;
	int release_x, release_y;	// where mouse is release
	int count;          // number of clicks
	unsigned long time;           // time of last click
	unsigned long release_time;	 // time of last release
};

//--------- Define class Mouse ------------//

class Mouse
{
private:
	char  init_flag;
	char* vga_update_buf;

	// ------ mouse setting ---------- //
	int	double_speed_threshold;				// default DEFAULT_DOUBLE_SPEED_THRESHOLD
	int	triple_speed_threshold;				// default DEFAULT_TRIPLE_SPEED_THRESHOLD

	//-------- click buffer ---------//
	MouseClick click_buffer[2];    // left button & right button only

	//-------- event buffer ---------//

	enum { EVENT_BUFFER_SIZE = 20 };  // No. of events can be stored in buffer

	MouseEvent event_buffer[EVENT_BUFFER_SIZE];

	int head_ptr;        // head pointer to the event buffer
	int tail_ptr;        // tail pointer to the event buffer

public:
	char handle_flicking;

	//------- real-time mouse state -------//

	int cur_x, cur_y;
	int left_press, right_press;

	//------- real-time keyboard state ---------//
	unsigned short skey_state;		// such as LEFT_SHIFT_KEY_MASK

	//------- boundary of mouse cursor hot spot --------//
	int	bound_x1;
	int	bound_y1;
	int	bound_x2;
	int	bound_y2;

	//-------- click & key buffer ---------//

	unsigned short event_skey_state;
	char has_mouse_event;		 // if has_mouse_event, mouse_event_type is valid
	MouseEventType mouse_event_type;
											 // use : LEFT_BUTTON=0, RIGHT_BUTTON=1
	unsigned scan_code;             // key pressed, keyboard event
	unsigned key_code;				// converted from scan_code and event_skey_state

	//-------- wheel/touch scrolling ---------//

	bool scrolling = false;
	int scroll_sensitivity = 10;
	int scroll_x = 0, scroll_y = 0;
	double scroll_prev_y = 0.0, scroll_prev_x = 0.0;

public:
	Mouse();
	~Mouse();

	void	init();
	void	deinit();

	void 	add_event(MouseEvent *);
	void 	add_event(MouseEventType type);
	void 	add_key_event(unsigned, unsigned long);
	int  	get_event();
	void	poll_event();
	void    process_mouse_motion(int x, int y);
	// #### begin Gilbert 31/10 #########//
	void	update_skey_state();
	// #### end Gilbert 31/10 #########//

	void 	show();
	void 	hide();
	void 	hide_area(int,int,int,int);
	void 	show_area();

	//--- functions for real-time mouse state accessing ---//

	int  	in_area(int,int,int,int);
	int  	press_area(int,int,int,int,int=0);
	int 	wait_press(int timeOutSecond=0);
	bool    get_scroll(int * x, int * y);

	//---- functions of mouse cursor boundary ------//
	void	set_boundary(int, int, int, int);
	void	reset_boundary();

	//----- functions for queue event accessing ----//

	int  	single_click(int,int,int,int,int=0);
	int  	double_click(int,int,int,int,int=0);
	int  	any_click   (int,int,int,int,int=0);
	int  	any_click(int=0);
	int	release_click(int,int,int,int,int=0);

	int  	click_x(int buttonId=0)     { return click_buffer[buttonId].x; }
	int  	click_y(int buttonId=0)     { return click_buffer[buttonId].y; }
	int  	release_x(int buttonId=0)   { return click_buffer[buttonId].release_x; }
	int  	release_y(int buttonId=0)   { return click_buffer[buttonId].release_y; }
	int  	click_count(int buttonId=0) { return click_buffer[buttonId].count; }

	int	is_mouse_event()            { return has_mouse_event; }
	int	is_key_event()              { return scan_code; }
	int	is_any_event()              { return has_mouse_event || scan_code; }
	int	is_press_button_event()     { return has_mouse_event && (mouse_event_type == LEFT_BUTTON || mouse_event_type == RIGHT_BUTTON); }
	int	is_release_button_event()   { return has_mouse_event && (mouse_event_type == LEFT_BUTTON_RELEASE || mouse_event_type == RIGHT_BUTTON_RELEASE); }

	void	reset_click();

	static int is_key(unsigned keyCode, unsigned short skeyState, unsigned short charValue, unsigned flags = 0 );
	static int is_key(unsigned keyCode, unsigned short skeyState, char *keyStr, unsigned flags = 0 );
	// see omouse2.h for flags

	void disp_count_start();
	void disp_count_end();

private:
	int micky_to_displacement(int d);
};
//---------- End of define class ---------------//

extern Mouse mouse;

#endif
