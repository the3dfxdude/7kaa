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

//Filename    : OBUTTCUS.CPP
//Description : Custom Button Object

#include <OSYS.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <OBUTTCUS.h>
#include <OFONT.h>

ButtonCustom customButton1;

//-------- Begin of function ButtonCustom::ButtonCustom -------//
//
ButtonCustom::ButtonCustom() : custom_para(NULL, 0)
{
	init_flag = 0;
	enable_flag = 0;
	button_key = 0;
	button_wait = 0;
}
//--------- End of function ButtonCustom::ButtonCustom -------//


//-------- Begin of function ButtonCustom::create -------//
//
// <int>   x1, y1, x2, y2 = coordination of the button
// <ButtonCustomFP> funcPtr = function to be called when need to paint
// <ButtonCustomPara> funcPara         = applicated defined integer
// [char]   elasticFlag= Whether the button is elastic
//                      Elastic button will pop up immediately when button release
//                      Non-elastic button will remain pushed until pop() is called
//                      (default : 1)
// [char]   defIsPushed = default pushed_flag : 1-Pushed, 0-Non-pushed
//                       (default : 0)
//
void ButtonCustom::create( int pX1, int pY1, int pX2, int pY2, 
	ButtonCustomFP funcPtr, ButtonCustomPara funcPara, char elasticFlag, char defIsPushed)
{
	init_flag = 1;

	//--------- set button size --------//

	x1 = pX1;
	y1 = pY1;
	x2 = pX2;
	y2 = pY2;

	//------ set button parameters -----//

	body_fp      = funcPtr;
	custom_para  = funcPara;
	elastic_flag = elasticFlag;
	pushed_flag  = defIsPushed;
	enable_flag  = 1;
}
//--------- End of function ButtonCustom::create --------//


//----------- Begin of function ButtonCustom::paint -----------//
//
// [int] defIsPushed = default pushed_flag : 1-Pushed, 0-Non-pushed
//                       (default : pushed_flag)
// [int] repaintBody = parameter passed to body_fp, (default 0)
//
void ButtonCustom::paint(int defIsPushed, int repaintBody)
{
	if( !init_flag )
		return;

	if( !vga.use_back_buf )
		mouse.hide_area(x1, y1, x2, y2 );

	if( defIsPushed >= 0 )
		pushed_flag = defIsPushed;

	//------ display the button button -------//

	(*body_fp)(this, repaintBody);

	//--------------------------------------//

	if( !vga.use_back_buf )
		mouse.show_area();
}
//---------- End of function ButtonCustom::paint -----------//


//-------- Begin of function ButtonCustom::detect -----------//
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
int ButtonCustom::detect(unsigned keyCode1, unsigned keyCode2, int detectRight, int suspendPop)
{
	int rc = 0;

	if( !init_flag || !enable_flag )
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
	button_wait_timeout = misc.get_time()+PRESSED_TIMEOUT_SECONDS*1000;

	// check for new button presses
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
	else if( mouse.key_code )
	{
		unsigned mouseKey=mouse.key_code;

		if( mouseKey >= 'a' && mouseKey <= 'z' )   // non-case sensitive comparsion
			mouseKey -= 32;                         // convert from lower case to upper case

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
//----------- End of function ButtonCustom::detect -------------//


//-------- Begin of function ButtonCustom::hide -----------//
//
// Disable and hide the button.
//
void ButtonCustom::hide()
{
	if( !init_flag )
		return;

	vga_util.blt_buf( x1, y1, x2, y2, 0 );

	enable_flag=0;
}
//----------- End of function ButtonCustom::hide -------------//


//----------- Begin of function ButtonCustom::disp_text_button_func -------------//
//
// a custom function to disp button with text only
//
// button->custom_para.ptr is the pointer of char
//
void ButtonCustom::disp_text_button_func(ButtonCustom *button, int repaintBody)
{
	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;

	// modify x1,y1, x2,y2 to the button body
	if( button->pushed_flag )
	{
		vga_util.d3_panel2_down(x1, y1, x2, y2);
		x1++;
		y1++;
	}
	else
	{
		vga_util.d3_panel2_up(button->x1, button->y1, button->x2, button->y2);
		x2--;
		y2--;
	}

	// put name
	font_bible.center_put(x1, y1, x2, y2, (char *)button->custom_para.ptr );
}
//----------- End of function ButtonCustom::disp_text_button_func -------------//


// ............................................................


//-------- Begin of function ButtonCustomGroup::ButtonCustomGroup -------//

ButtonCustomGroup::ButtonCustomGroup(int buttonNum)
{
   button_pressed = 0;
   button_num     = buttonNum;
	button_array   = new ButtonCustom[buttonNum];
}
//---------- End of function ButtonCustomGroup::ButtonCustomGroup -------//


//----------- Begin of function ButtonCustomGroup::~ButtonCustomGroup -----------//
//
ButtonCustomGroup::~ButtonCustomGroup()
{
	delete[] button_array;
}
//-------------- End of function ButtonCustomGroup::~ButtonCustomGroup ----------//


//--------- Begin of function ButtonCustomGroup::paint ----------//
//
// Paint all buttons in this button nation.
//
// [int] buttonPressed = the default pressed button
//                       (default no change to button_pressed)
//
void ButtonCustomGroup::paint(int buttonPressed)
{
   int i;

   if( buttonPressed >= 0 )
      button_pressed = buttonPressed;

   for( i=0 ; i<button_num ; i++ )
		button_array[i].paint(button_pressed==i);
}
//----------- End of function ButtonCustomGroup::paint ----------//


//--------- Begin of function ButtonCustomGroup::detect ----------//
//
// Detect all buttons in this button nation.
// Since only one button can be pressed at one time,
// so if any one of them is pressed, the previously pressed one
// will be pop up.
//
// Return : <int> -1  - if no button pressed
//                >=0 - the record no. of the button pressed
//
int ButtonCustomGroup::detect()
{
   int i;

   for( i=0 ; i<button_num ; i++ )
   {
		if( !button_array[i].pushed_flag && button_array[i].detect() )
      {
          button_array[button_pressed].pop();
          button_pressed = i;
          return i;
      }
   }

   return -1;
}
//----------- End of function ButtonCustomGroup::detect ----------//


//--------- Begin of function ButtonCustomGroup::push ----------//
//
// Push the specified button.
//
// <int> buttonId = Id. of the button.
//
void ButtonCustomGroup::push(int buttonId, int paintFlag)
{
   int i;

   button_pressed = buttonId;

	if( paintFlag )
		for( i=0 ; i<button_num ; i++ )
		{
			if( i==buttonId )
				button_array[i].push();
			else
				button_array[i].pop();
		}
	else
		for( i=0 ; i<button_num ; i++ )
		{
			if( i==buttonId )
				button_array[i].pushed_flag = 1;
			else
				button_array[i].pushed_flag = 0;
		}
}
//----------- End of function ButtonCustomGroup::push ----------//


//--------- Begin of function ButtonCustomGroup::operator[] ----------//
//
// <int> buttonId = Id. of the button, start from 0
//
ButtonCustom& ButtonCustomGroup::operator[](int buttonId)
{
   err_when( buttonId<0 || buttonId >= button_num );

   return button_array[buttonId];
}
//----------- End of function ButtonCustomGroup::operator[] ----------//
