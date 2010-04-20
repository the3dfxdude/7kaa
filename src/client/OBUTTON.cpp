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

//Filename    : OBUTTON.CPP
//Description : Button Object

#include <KEY.h>
#include <ALL.h>
#include <OHELP.h>
#include <OFONT.h>
#include <OSYS.h>
#include <OMOUSE.h>
#include <OVGA.h>
#include <OIMGRES.h>
#include <OBUTTON.h>

//----------- Begin of function Button::Button -----------//
//
Button::Button()
{
	init_flag    = 0;
	x1           = -1;
	enable_flag  = 1;
	button_key   = 0;    // set by set_key()
	use_texture_flag  = 0;

	help_code[0] = NULL;

	font_ptr = &font_san;
}
//-------------- End of function Button::Button ----------//


//-------- Begin of function Button::set_help_code -------//
//
void Button::set_help_code(const char* helpCode)
{
	strncpy( help_code, helpCode, HELP_CODE_LEN );

	help_code[HELP_CODE_LEN] = NULL;
}
//--------- End of function Button::set_help_code --------//


//-------- Begin of function Button::set_font -------//
//
// Set the font of the button, if not set, default use font_san.
//
// <Font*> fontPtr = pointer to the font
//
void Button::set_font(Font* fontPtr)
{
   err_when(!fontPtr);

   font_ptr = fontPtr;
}
//--------- End of function Button::set_font --------//


//-------- Begin of function Button::create_text -------//
//
// Given only the top left position of the button, it will calculate
// the width and height of the button based on the given text and
// current button font.
//
// Syntax : create(<int>,<int>,<int>,<int>,<int>,<int>,<char>)
//
// <int>   x1, y1      = coordination of the button
// <char*> textPtr     = text pointer, depended on the type of the button
//
// [int]   elastic     = Whether the button is elastic
//                       Elastic button will pop up immediately when button release
//                       Non-elastic button will remain pushed until pop() is called
//                       (default : 1)
//
// [int]  defIsPushed  = default is_pushed : 1-Pushed, 0-Non-pushed
//                         (default : 0)
//
void Button::create_text(int pX1, int pY1, const char* textPtr, char pElastic, char defIsPushed)
{
   int pX2, pY2;

   pX2 = pX1 + font_ptr->text_width( textPtr ) + 9;
   pY2 = pY1 + font_ptr->height() + 7;

	create( BUTTON_TEXT,pX1,pY1,pX2,pY2,textPtr,pElastic,defIsPushed);
}
//-------- End of function Button::create_text -----//


//-------- Begin of function Button::paint_text -------//
//
// Given only the top left position of the button, it will calculate
// the width and height of the button based on the given text and
// current button font.
//
// Syntax : create(<int>,<int>,<int>,<int>,<int>,<int>,<char>)
//
// <int>   x1, y1      = coordination of the button
// <char*> textPtr     = text pointer, depended on the type of the button
//
// [int]   elastic     = Whether the button is elastic
//                       Elastic button will pop up immediately when button release
//                       Non-elastic button will remain pushed until pop() is called
//                       (default : 1)
//
// [int]   defIsPushed = default is_pushed : 1-Pushed, 0-Non-pushed
//                         (default : 0)
//
// Note : it use the color setting in (vga) and (font_san)
//
void Button::paint_text(int pX1, int pY1, const char* textPtr, char pElastic, char defIsPushed)
{
	int pX2, pY2;

	pX2 = pX1 + font_ptr->text_width( textPtr ) + 9;
	pY2 = pY1 + font_ptr->height() + 7;

	create( BUTTON_TEXT,pX1,pY1,pX2,pY2,textPtr,pElastic,defIsPushed);
	paint();
}
//-------- End of function Button::paint_text -----//


//-------- Begin of function Button::create -------//
//
// Syntax : create(<int>,<int>,<int>,<int>,<int>,<int>,<char>)
//
// <int>   buttonType = BUTTON_TEXT (1) - text button, BUTTON_BITMAP (2) - icon button
// <int>   x1, y1     = coordination of the button
// <int>   x2, y2     = coordination of the button
// <void*> bodyPtr    = text or icon pointer or pointer to user defined function,
//                      depended on the type of the button allow NULL icon
//                      pointer, which no icon will be displayed
// [int]   elastic    = Whether the button is elastic
//                      Elastic button will pop up immediately when button release
//                      Non-elastic button will remain pushed until pop() is called
//                      (default : 1)
//
// [int]   defIsPushed = default is_pushed : 1-Pushed, 0-Non-pushed
//                       (default : 0)
//
// Note : it use the color setting in (vga) and (font_san)
//
void Button::create(int buttonType, int pX1, int pY1, int pX2, int pY2,
						  const void* bodyPtr, char pElastic, char defIsPushed)
{
	int strLen;

	init_flag = 1;

	button_type = buttonType;

	x1 = pX1;
	y1 = pY1;
	x2 = pX2;
	y2 = pY2;

	elastic 	   = pElastic;
	is_pushed   = defIsPushed;
	enable_flag = 1;

	//------------------------------------//

	if( buttonType == BUTTON_TEXT )
	{
		strLen = strlen((char*)bodyPtr);  // copy the string to class member buffer
												 // some string are temporary, we need it for repaint
		if( strLen > STR_BUF_LEN )
			strLen = STR_BUF_LEN;

		memcpy( str_buf, bodyPtr, strLen );
		str_buf[strLen] = NULL;
	}
	else
		body_ptr = bodyPtr;
}
//--------- End of function Button::create --------//


//-------- Begin of function Button::set_body -------//
//
// <void*> bodyPtr    = text or icon pointer or pointer to user defined function,
//
void Button::set_body(void* bodyPtr)
{
   if( button_type == BUTTON_TEXT )
   {
      int strLen = strlen((char*)bodyPtr);  // copy the string to class member buffer
                                     // some string are temporary, we need it for repaint
      if( strLen > STR_BUF_LEN )
         strLen = STR_BUF_LEN;

      memcpy( str_buf, bodyPtr, strLen );
      str_buf[strLen] = NULL;
   }
   else
      body_ptr = bodyPtr;
}
//--------- End of function Button::set_body --------//


//--------- Begin of function Button::hide ----------//
//
// Hide and disable the button
//
// Syntax : hide(char)
//
// <char> backColor = background color
//
void Button::hide(char backColor)
{
   if( init_flag )
   {
      init_flag = 0;

      if( x1 >= 0 )        // when create() hasn't been called x1 is -1
			Vga::active_buf->bar( x1,y1,x2,y2,backColor );
   }
}
//------------ End of function Button::hide ---------//


//--------- Begin of function Button::show ----------//
//
// Show a button hiden by hide().
//
void Button::show()
{
   if( !init_flag )
   {
      init_flag = 1;
		paint(0, 1);
   }
}
//------------ End of function Button::show ---------//


//----------- Begin of function Button::paint -----------//
//
// [int] defIsPushed = default is_pushed : 1-Pushed, 0-Non-pushed
//                       (default : is_pushed)
//
// [int] repaintBody =  repaint the button body or only repaint panel sides
//                      (default : 1), 0 when called by detect()
//
void Button::paint(int defIsPushed, int repaintBody)
{
	if( !init_flag )
		return;

	int colorUp    = Vga::active_buf->color_up;
	int colorLight = Vga::active_buf->color_light;

	Vga::active_buf->color_light = (char) V_WHITE;   // don't use layer colors

	if( enable_flag )
	{
		if( defIsPushed >= 0 )
			is_pushed = defIsPushed;

		if( !is_pushed )
		{
			if( use_texture_flag )
				vga.d3_panel_up( x1, y1, x2, y2 );
			else
			{
				Vga::active_buf->color_up      = Vga::active_buf->color_down;
				Vga::active_buf->d3_panel_up( x1, y1, x2, y2, 1, repaintBody || button_type==1 );
			}
		}
		else
		{
			if( use_texture_flag )
			{
				vga.d3_panel_down( x1, y1, x2, y2 );
			}
			else
			{
				if( repaintBody || button_type==1 )      // text button
					Vga::active_buf->d3_panel_up( x1, y1, x2, y2, 2 );

				Vga::active_buf->d3_panel_down( x1,y1,x2,y2, 1, 0 );
			}
		}
	}
	else         // button disabled
	{
		Vga::active_buf->color_up = Vga::active_buf->color_up+1;
		Vga::active_buf->d3_panel_up( x1, y1, x2, y2, 1, repaintBody );
	}

	Vga::active_buf->color_up    = colorUp;
	Vga::active_buf->color_light = colorLight;

	//--------- put button body -------------//

	if( button_type == BUTTON_TEXT )      // text button
	{
		int tx = x1 + ((x2-x1) - font_ptr->text_width(str_buf))/2;
		int ty = y1 + ((y2-y1) - font_ptr->height())/2 - 1;

		tx = MAX( tx, x1+4 );
		ty = MAX( ty, y1+1 );

		font_ptr->put( tx, ty, str_buf, 0, x2-3 );
	}

	else if( button_type == BUTTON_BITMAP )
	{
		if( body_ptr && repaintBody )
		{
			int tx = x1 + ((x2-x1+1) - *((short*)body_ptr))/2;
			int ty = y1 + ((y2-y1+1) - *((short*)body_ptr+1))/2;

			Vga::active_buf->put_bitmap( tx, ty, (char*) body_ptr );   // 0 means not clear background
		}
	}
	else   // BUTTON_UDF, user defined function
	{
		if( body_ptr )
			(*((ButtonFP*)body_ptr))(x1+3,x2+3,y1-3,y2-3);
	}
}
//---------- End of function Button::paint -----------//


//-------- Begin of function Button::detect -----------//
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
int Button::detect(unsigned keyCode1, unsigned keyCode2, int detectRight, int suspendPop)
{
   int rc=0;

   if( !init_flag || !enable_flag )
      return 0;

	help.set_help( x1, y1, x2, y2, help_code );

   if( mouse.any_click(x1,y1,x2,y2,LEFT_BUTTON) )
		rc=1;

   else if( detectRight && mouse.any_click(x1,y1,x2,y2,RIGHT_BUTTON) )
      rc=2;

   else if(mouse.key_code)
   {
      unsigned mouseKey=mouse.key_code;

      if( mouseKey >= 'a' && mouseKey <= 'z' )   // non-case sensitive comparsion
         mouseKey -= 32;                         // convert from lower case to upper case

      if( mouseKey == keyCode1 || mouseKey == keyCode2 || mouseKey == button_key )
      {
         rc=3;
      }
   }

   if( !rc )
      return 0;

	//----- paint the button with pressed shape ------//

	#define PRESSED_TIMEOUT_SECONDS  1      // 1 seconds
	DWORD timeOutTime = m.get_time()+PRESSED_TIMEOUT_SECONDS*1000;

	if( elastic )
	{
		if( !is_pushed )
			paint(1,0);         // 0-no need to repaint button body (text or icon)

		while( (rc==1 && mouse.left_press) || (rc==2 && mouse.right_press) )
		{
			sys.yield();
			mouse.get_event();

			if( m.get_time() >= timeOutTime )
				break;
		}

		if( elastic )
			paint(0,0);
	}
	else         // inelastic button
	{
		if( suspendPop )
			is_pushed = 1;
		else
			is_pushed = !is_pushed;

		paint(is_pushed,0);

		while( (rc==1 && mouse.left_press) || (rc==2 && mouse.right_press) )
		{
			sys.yield();
			mouse.get_event();

			if( m.get_time() >= timeOutTime )
				break;
		}
   }

   return rc;
}
//----------- End of function Button::detect -------------//


//--------- Begin of function Button::wait_press ----------//
//
// Wait for user to press the button
//
// [int] timeOut - 1=enable inactive timeout
//                 0=disable inactive timeout
//                 (default : 1 )
//
void Button::wait_press(int timeOut)
{
	#define INACTIVE_TIMEOUT_SECONDS  10      // 10 seconds

	int   lastMouseX= -1, lastMouseY;
	DWORD timeOutTime = m.get_time()+INACTIVE_TIMEOUT_SECONDS*1000;

	mouse.get_event();			// clean up previous mouse events

	while( !detect(KEY_RETURN,KEY_ESC) && !mouse.any_click(1) )  // 1-only right mouse button
	{
		sys.yield();
		mouse.get_event();

		//--- when the user is inactive for a certain time, ----//
		//--------- close the report automatically -------------//

		if( timeOut )
		{
			if( lastMouseX == mouse.cur_x && lastMouseY == mouse.cur_y )
			{
				if( m.get_time() >= timeOutTime )
					break;
			}
			else
			{
				lastMouseX  = mouse.cur_x;
				lastMouseY  = mouse.cur_y;
				timeOutTime = m.get_time()+INACTIVE_TIMEOUT_SECONDS*1000; 
			}
		}
	}
}
//----------- End of function Button::wait_press ----------//


//.........................................................//


//-------- Begin of function ButtonGroup::ButtonGroup -------//

ButtonGroup::ButtonGroup(int buttonNum)
{
   button_pressed = 0;
   button_num     = buttonNum;
	button_array   = new Button[buttonNum];
}
//---------- End of function ButtonGroup::ButtonGroup -------//


//----------- Begin of function ButtonGroup::~ButtonGroup -----------//
//
ButtonGroup::~ButtonGroup()
{
	delete[] button_array;
}
//-------------- End of function ButtonGroup::~ButtonGroup ----------//


//--------- Begin of function ButtonGroup::paint ----------//
//
// Paint all buttons in this button nation.
//
// [int] buttonPressed = the default pressed button
//                       (default no change to button_pressed)
//
void ButtonGroup::paint(int buttonPressed)
{
   int i;

   if( buttonPressed >= 0 )
      button_pressed = buttonPressed;

   for( i=0 ; i<button_num ; i++ )
		button_array[i].paint(button_pressed==i);
}
//----------- End of function ButtonGroup::paint ----------//


//--------- Begin of function ButtonGroup::detect ----------//
//
// Detect all buttons in this button nation.
// Since only one button can be pressed at one time,
// so if any one of them is pressed, the previously pressed one
// will be pop up.
//
// Return : <int> -1  - if no button pressed
//                >=0 - the record no. of the button pressed
//
int ButtonGroup::detect()
{
   int i;

   for( i=0 ; i<button_num ; i++ )
   {
		if( !button_array[i].is_pushed && button_array[i].detect() )
      {
          button_array[button_pressed].pop();
          button_pressed = i;
          return i;
      }
   }

   return -1;
}
//----------- End of function ButtonGroup::detect ----------//


//--------- Begin of function ButtonGroup::push ----------//
//
// Push the specified button.
//
// <int> buttonId = Id. of the button.
//
void ButtonGroup::push(int buttonId)
{
   int i;

   button_pressed = buttonId;

   for( i=0 ; i<button_num ; i++ )
   {
      if( i==buttonId )
         button_array[i].push();
      else
         button_array[i].pop();
   }
}
//----------- End of function ButtonGroup::push ----------//


//--------- Begin of function ButtonGroup::operator[] ----------//
//
// <int> buttonId = Id. of the button, start from 0
//
Button& ButtonGroup::operator[](int buttonId)
{
   err_when( buttonId<0 || buttonId >= button_num );

   return button_array[buttonId];
}
//----------- End of function ButtonGroup::operator[] ----------//
