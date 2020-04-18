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

//Filename    : OBUTT3D.CPP
//Description : 3D Button Object

#include <OSYS.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OHELP.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <OBUTT3D.h>

//---------- define static vars ----------//

static char save_back_buf[BUTTON_ACTION_WIDTH * BUTTON_ACTION_HEIGHT + 4];


//-------- Begin of function Button3D::Button3D -------//
//
Button3D::Button3D()
{
	init_flag = 0;
	enable_flag = 0;
	button_key = 0;
	button_wait = 0;
}
//--------- End of function Button3D::Button3D -------//


//-------- Begin of function Button3D::create -------//
//
// Create BUTTON_TYPE_SHELL button.
//
// create(<int>,<int>,<int>,<int>,<char>)
//
// <int>   x1, y1     = coordination of the button
// <char>  buttonStyle = style id. of the button
// <char*> buttonName = name of the button
// [int]   elasticFlag= Whether the button is elastic
//                      Elastic button will pop up immediately when button release
//                      Non-elastic button will remain pushed until pop() is called
//                      (default : 1)
// [int]   defIsPushed = default pushed_flag : 1-Pushed, 0-Non-pushed
//                       (default : 0)
//
void Button3D::create(int pX1, int pY1, char buttonStyle,
							 const char* buttonName, char elasticFlag, char defIsPushed)
{
	init_flag = 1;

	//------ get the button bitmap ------//

	char buttonFileName[] = "BUTUP_A";

	buttonFileName[6] = buttonStyle;	// up button bitmap

	button_up_ptr = image_button.get_ptr(buttonFileName);

	buttonFileName[3] = 'D';		// down button bitmap
	buttonFileName[4] = 'N';

	button_down_ptr = image_button.get_ptr(buttonFileName);

	buttonFileName[3] = 'D';		// disabled button bitmap
	buttonFileName[4] = 'S';

	button_disabled_ptr = image_button.get_ptr(buttonFileName);

	//--------- set button size --------//

	short* iconPtr = (short*) button_up_ptr;

	x1 = pX1;
	y1 = pY1;
	x2 = x1+ *iconPtr++ - 1;
	y2 = y1+ *iconPtr - 1;

	//------ set button parameters -----//

	button_type	 = BUTTON_TYPE_SHELL;
	button_style = buttonStyle;
	elastic_flag = elasticFlag;
	pushed_flag  = defIsPushed;
	enable_flag  = 1;

	err_when( strlen(buttonName) > 8 );
	strcpy( help_code, buttonName );

	icon_ptr = image_button.get_ptr(buttonName);
}
//--------- End of function Button3D::create --------//


//-------- Begin of function Button3D::create -------//
//
// Create BUTTON_TYPE_BITMAP button.
//
void Button3D::create(int pX1, int pY1, const char* upButtonName,
							 const char* downButtonName, char elasticFlag, char defIsPushed)
{
	init_flag = 1;

	//--------- set button size --------//

	button_up_ptr   = image_button.get_ptr(upButtonName);
	button_down_ptr = image_button.get_ptr(downButtonName);

	err_when( !button_up_ptr );
	err_when( !button_down_ptr );

	x1 = pX1;
	y1 = pY1;
	x2 = x1+ *((short*)button_up_ptr) - 1;
	y2 = y1+ *((short*)(button_up_ptr)+1) - 1;

	//------ set button parameters -----//

	button_type	 = BUTTON_TYPE_BITMAP;
	elastic_flag = elasticFlag;
	pushed_flag  = defIsPushed;
	enable_flag  = 1;

	err_when( strlen(upButtonName) > 8 );
	strcpy( help_code, upButtonName );
}
//--------- End of function Button3D::create --------//


//-------- Begin of function Button3D::update_bitmap -------//
//
// Update the bitmap of the button and repaint the button.
//
void Button3D::update_bitmap(char* buttonName)
{
	err_when( strlen(buttonName) > 8 );
	strcpy( help_code, buttonName );

	icon_ptr = image_button.get_ptr(buttonName);

	paint();
}
//--------- End of function Button3D::update_bitmap --------//


//-------- Begin of function Button3D::set_help_code -------//
//
void Button3D::set_help_code(const char* helpCode)
{
	strncpy( help_code, helpCode, HELP_CODE_LEN );

	help_code[HELP_CODE_LEN] = '\0';
}
//--------- End of function Button3D::set_help_code --------//


//----------- Begin of function Button3D::paint -----------//
//
// [int] defIsPushed = default pushed_flag : 1-Pushed, 0-Non-pushed
//                       (default : pushed_flag)
//
void Button3D::paint(int defIsPushed)
{
	if( !init_flag )
		return;

	if( !vga.use_back_buf )
		mouse.hide_area(x1, y1, x2, y2 );

	if( defIsPushed >= 0 )
		pushed_flag = defIsPushed;

	//------ display the button button -------//

	if( button_type == BUTTON_TYPE_SHELL && icon_ptr )
	{
		int tx = x1 + ((x2-x1+1) - *((short*)icon_ptr))/2;
		int ty = y1 + ((y2-y1+1) - *((short*)icon_ptr+1))/2;

		tx -= 2;
		ty -= 3;

		//----------------------------------//

		if( enable_flag )
		{
			if( pushed_flag )
				Vga::active_buf->put_bitmap_trans_decompress( x1, y1, button_down_ptr );
			else
				Vga::active_buf->put_bitmap_trans_decompress( x1, y1, button_up_ptr );

			Vga::active_buf->put_bitmap_trans_decompress( tx, ty, (char*) icon_ptr );   // 0 means not clear background
		}
		else         // button disabled
		{
			//--- put it on the back buffer, darken it and blt it back to the front buffer ---//

			if( !vga.use_back_buf )
				vga_back.read_bitmap( x1, y1, x1+BUTTON_ACTION_WIDTH-1, y1+BUTTON_ACTION_HEIGHT-1, save_back_buf );

			//------ display and darken ----//

			vga_back.put_bitmap_trans_decompress( x1, y1, button_up_ptr );
			vga_back.put_bitmap_trans_decompress( tx, ty, (char*) icon_ptr );   // 0 means not clear background

			vga_back.adjust_brightness( x1+1, y1+1, x1+BUTTON_ACTION_WIDTH-4, y1+BUTTON_ACTION_HEIGHT-2, -5 );

			//--- blt the button from the back to the front and restore the back buf ---//

			if( !vga.use_back_buf )
			{
				vga_util.blt_buf( x1, y1, x1+BUTTON_ACTION_WIDTH-1, y1+BUTTON_ACTION_HEIGHT-1, 0 );
				vga_back.put_bitmap( x1, y1, save_back_buf );
			}
		}
	}
	else if( button_type == BUTTON_TYPE_BITMAP )
	{
		if( pushed_flag )
			Vga::active_buf->put_bitmap_trans_decompress( x1, y1, button_down_ptr );
		else
			Vga::active_buf->put_bitmap_trans_decompress( x1, y1, button_up_ptr );
	}

	//--------------------------------------//

	if( !vga.use_back_buf )
		mouse.show_area();
}
//---------- End of function Button3D::paint -----------//


//-------- Begin of function Button3D::detect -----------//
//
// Detect whether the button has been pressed,
// if so, act correspondly.
// Check for left mouse button only
//
// [unsigned] keyCode1 = if the specified key is pressed, emulate button pressed
//                       (default : 0)
//
// [unsigned] keyCode2 = if the specified key is pressed, emulate button pressed
//                       (default : 0)
//
// [int] detectRight   = whether also detect the right button or not
//                       (default : 0)
//
// [int] suspendPop    = don't pop up the button even it should
//                       (defalut : 0)
//
// Return : 1 - if left mouse button pressed
//          2 - if right mouse button pressed
//          3 - the key is pressed (only when keyCode is specified)
//          0 - if not
//
int Button3D::detect(unsigned keyCode1, unsigned keyCode2, int detectRight, int suspendPop)
{
	int rc = 0;

	if( !init_flag )
		return 0;

	help.set_help( x1, y1, x2, y2, help_code );

	if( !enable_flag )
		return 0;

	//---------------------------------------------//

	if( button_wait )
	{
		// handle button press in progress

		int in_rect = mouse.cur_x >= x1 && mouse.cur_y >= y1 && mouse.cur_x <= x2 && mouse.cur_y <= y2;
		int mouse_press = (button_wait == 1 && mouse.left_press)||(button_wait==2 && mouse.right_press);

		if( in_rect && mouse_press && misc.get_time() < button_wait_timeout )
			return 0;

		if( in_rect )
			rc = button_wait;

		if( elastic_flag )
			paint(0);

		button_wait = 0;

		return rc;
	}

	//---------------------------------------------//

	#define PRESSED_TIMEOUT_SECONDS  1      // 1 seconds

	if( mouse.any_click(x1,y1,x2,y2,LEFT_BUTTON) )
	{
		if( elastic_flag )
		{
			button_wait = 1;
			button_wait_timeout = misc.get_time()+PRESSED_TIMEOUT_SECONDS*1000;
		}
		else
			rc = 1;
	}

	else if( detectRight && mouse.any_click(x1,y1,x2,y2,RIGHT_BUTTON) )
	{
		if( elastic_flag )
		{
			button_wait = 2;
			button_wait_timeout = misc.get_time()+PRESSED_TIMEOUT_SECONDS*1000;
		}
		else
			rc = 2;
	}

	else if( mouse.unique_key_code )
	{
		unsigned mouseKey=mouse.unique_key_code;

		if( mouseKey == keyCode1 || mouseKey == keyCode2 || mouseKey == button_key )
		{
			rc = 3;
		}
	}

	if( !button_wait && !rc )
		return 0;

	//----- paint the button with pressed shape ------//

	if( elastic_flag )
	{
		if( !pushed_flag )
			paint(1);
	}
	else         // inelastic_flag button
	{
		if( suspendPop )
			pushed_flag = 1;
		else
			pushed_flag = !pushed_flag;

		paint(pushed_flag);
	}

	return rc;
}
//----------- End of function Button3D::detect -------------//


//-------- Begin of function Button3D::hide -----------//
//
// Disable and hide the button.
//
void Button3D::hide()
{
	if( !init_flag )
		return;

	vga_util.blt_buf( x1, y1, x2, y2, 0 );

	enable_flag=0;
}
//----------- End of function Button3D::hide -------------//
