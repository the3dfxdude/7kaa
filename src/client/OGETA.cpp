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

// Filename     : OGETA.CPP
// Description  : simple get field

#include <OGETA.h>
#include <OVGA.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <KEY.h>
#include <OSYS.h>
#include <COLCODE.h>


GetA::GetA()
{
	input_field = NULL;
	field_len = 0;
	font_ptr = NULL;
	align_flag = 0;
	enable_flag = 1;
	mouse_drag_flag = 0;
	back_ground_bitmap = NULL;
}

void GetA::init( int x1, int y1, int x2, char *field, unsigned length, 
					 Font *fontPtr, char align, char detectEsc)
{
	x = x1;
	y = y1;
	x_limit = x2;

	input_field = field;
	field_len = length;
	font_ptr = fontPtr;
	align_flag = align;
	enable_flag = 1;
	esc_key_flag = detectEsc;
	mouse_drag_flag = 0;
	back_ground_bitmap = NULL;

	select_whole();
}

int GetA::height()
{
	return font_ptr->max_font_height;
}


void GetA::clear()
{
	input_field[0] = '\0';
	cursor_pos = 0;
	clear_select();
}

// return 0 for no input
// return key code pressed such as KEY_RETURN, KEY_ESCAPE ...

unsigned GetA::detect_key()
{
	if( !enable_flag )
		return 0;

	err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
	err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );

	if( mouse.is_key_event() )
	{
		unsigned keyCode = mouse.key_code;
		unsigned	shiftPressed = mouse.event_skey_state & SHIFT_KEY_MASK;
		// printable character
		if( keyCode >= ' ' && keyCode <= 0xff )
		{
			if( strlen(input_field)-(mark_end() - mark_begin()) < field_len)
			{
				// insert character
				memmove( input_field+mark_begin()+1, input_field+mark_end(), 
					strlen(input_field)-mark_end()+1);
				input_field[mark_begin()] = keyCode;
				cursor_pos = mark_begin()+1;
				clear_select();
			}
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		else if( keyCode == KEY_DEL )
		{
			if( is_select() )
			{
				err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
				err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
				// erase selected area
				memmove( input_field+mark_begin(), input_field+mark_end(),
					strlen(input_field)-mark_end()+1);
				cursor_pos = mark_begin();
				clear_select();
			}
			else 
			{
				if(strlen(input_field) > cursor_pos)
				{
					err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
					memmove( input_field+cursor_pos, input_field+cursor_pos+1, 
						strlen(input_field)-cursor_pos);
					err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
				}
			}
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		else if( keyCode == KEY_BACK_SPACE )
		{
			if( is_select() )
			{
				err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
				err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
				// erase selected area
				memmove( input_field+mark_begin(), input_field+mark_end(),
					strlen(input_field)-mark_end()+1);
				cursor_pos = mark_begin();
				clear_select();
			}
			else 
			{
				if(cursor_pos > 0)
				{
					err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
					memmove( input_field+cursor_pos-1, input_field+cursor_pos, 
						strlen(input_field)-cursor_pos+1);
					cursor_pos--;
					err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
					clear_select();
				}
			}
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		if( keyCode == KEY_LEFT )
		{
			if(cursor_pos > 0)
				cursor_pos--;
			if( !shiftPressed )
				clear_select();
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		if( keyCode == KEY_RIGHT )
		{
			if(cursor_pos < strlen(input_field))
				cursor_pos++;
			if( !shiftPressed )
				clear_select();
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		if( keyCode == KEY_HOME)
		{
			cursor_pos = 0;
			if( !shiftPressed )
				clear_select();
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		if( keyCode == KEY_END)
		{
			cursor_pos = strlen(input_field);
			if( !shiftPressed )
				clear_select();
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
			err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
			return keyCode;
		}
		if( esc_key_flag && keyCode == KEY_ESC )
		{
			// if esc_key_flag is 0 and ESC key pressed, still return 0
			clear();
			return keyCode;
		}
		if( keyCode == KEY_RETURN || keyCode == KEY_UP ||
			keyCode == KEY_DOWN || keyCode == KEY_TAB )
		{
			return keyCode;
		}
	}

	return 0;
}

void GetA::paint(int paintCursor)
{
	err_when( cursor_pos < 0 || cursor_pos > strlen(input_field) );
	err_when( mark_cursor_pos < 0 || mark_cursor_pos > strlen(input_field) );
	int cursorX = font_ptr->text_width(input_field, cursor_pos);
	int leftX = font_ptr->text_width(input_field, mark_begin());
	int rightX = font_ptr->text_width(input_field, mark_end());

	// create a temp buffer to store character
	int rightLimit = x_limit - x;
	int textWidth = font_ptr->text_width(input_field, -1, rightLimit ) + 1;
	int textHeight = font_ptr->max_font_height;
	char *bitmap = sys.common_data_buf;
	err_when( 2*sizeof(short) + textWidth * textHeight > COMMON_DATA_BUF_SIZE );

	*(short *)bitmap = textWidth;
	bitmap += sizeof(short);
	*(short *)bitmap = textHeight;
	bitmap += sizeof(short);

	memset( bitmap, TRANSPARENT_CODE, textWidth * textHeight );
	if( back_ground_bitmap && !Vga::use_back_buf )
	{
		short backGroundWidth = *(short *)back_ground_bitmap;
		short backGroundHeight = *(1+(short *)back_ground_bitmap);
		// fill backGround
		switch(align_flag)
		{
		case 0:
			IMGbltArea(bitmap, textWidth, 0, 0, back_ground_bitmap, 
				0, 0, MIN(textWidth, backGroundWidth)-1, MIN(textHeight, backGroundHeight)-1 );
			break;
		case 1:
			{
				int l = (x_limit - x + 1 - textWidth ) / 2;
				if( l >= 0 && l < backGroundWidth )
				IMGbltArea(bitmap, textWidth, 0, 0, back_ground_bitmap, 
					l, 0, MIN(l+textWidth, backGroundWidth)-1, MIN(textHeight, backGroundHeight)-1 );
			}
			break;
		case -1:
			{
				int l = x_limit - textWidth + 1 - x;
				if( l < backGroundWidth )
					IMGbltArea(bitmap, textWidth, 0, 0, back_ground_bitmap, 
						l, 0, MIN(x_limit-x+1, backGroundWidth)-1, MIN(textHeight, backGroundHeight)-1 );
			}
			break;
		default:
			err_here();
		}
	}
	font_ptr->put_to_buffer(bitmap, textWidth, 0, 0, input_field);

	if( paintCursor && enable_flag && cursorX < x_limit )
	{
		// horizontal bar for selected area
		if( leftX < textWidth )
		{
			IMGbar( bitmap, textWidth, 
				leftX, font_ptr->height()-1, MIN(rightX, textWidth-1), font_ptr->height()-1, 0);
		}

		// vertical bar
		if( cursorX < textWidth )
			IMGbar( bitmap, textWidth, cursorX, 0, cursorX, font_ptr->height()-1, 0);
	}

	if( !Vga::use_back_buf)
	{
		mouse.hide_area(x,y, x_limit, y+font_ptr->max_font_height-1 );
		switch( align_flag )
		{
		case 0:		// left justified
			if( !back_ground_bitmap )
			{
				if( x+textWidth <= x_limit )		// fill right
					vga.blt_buf( x+textWidth, y, x_limit, y + font_ptr->max_font_height-1, 0);
				IMGjoinTrans(vga_front.buf_ptr(), vga_front.buf_pitch(), 
					vga_back.buf_ptr(), vga_back.buf_pitch(),
					x, y, sys.common_data_buf);
			}
			else
			{
				short backGroundWidth = *(short *)back_ground_bitmap;
				short backGroundHeight = *(1+(short *)back_ground_bitmap);
				if( textWidth < backGroundWidth && x+textWidth <= x_limit )		// fill right
					vga_front.put_bitmap_area(x, y, back_ground_bitmap,
					textWidth, 0, MIN(x_limit-x, backGroundWidth-1), backGroundHeight-1 );
				vga_front.put_bitmap_trans(x, y, sys.common_data_buf);
			}
			break;

		case 1:		// center justified
			if( !back_ground_bitmap )
			{
				int l = x + (x_limit - x + 1 - textWidth ) / 2;
				if( x < l )
				{
					vga.blt_buf( x, y, l-1, y + font_ptr->max_font_height-1, 0);
				}
				if( l+textWidth <= x_limit )
				{
					vga.blt_buf( l+textWidth, y, x_limit, y + font_ptr->max_font_height-1, 0);
				}
				IMGjoinTrans(vga_front.buf_ptr(), vga_front.buf_pitch(), 
					vga_back.buf_ptr(), vga_back.buf_pitch(),
					l, y, sys.common_data_buf);
			}
			else
			{
				int l = x + (x_limit - x + 1 - textWidth ) / 2;
				short backGroundWidth = *(short *)back_ground_bitmap;
				short backGroundHeight = *(1+(short *)back_ground_bitmap);
				if( x < l && l-x <= backGroundWidth)
				{
					vga_front.put_bitmap_area(x, y, back_ground_bitmap, 
						0, 0, MIN(l-x, backGroundWidth)-1, backGroundHeight-1);
				}
				if( l+textWidth <= x_limit && l+textWidth-x < backGroundWidth)
				{
					vga_front.put_bitmap_area(x, y, back_ground_bitmap,
						l+textWidth-x, 0, MIN(x_limit-x+1, backGroundWidth)-1, backGroundHeight-1);
				}
				vga_front.put_bitmap_trans(l, y, sys.common_data_buf);
			}
			break;

		case -1:		// right justified
			if( !back_ground_bitmap )
			{
				int l = x_limit - textWidth + 1;
				if( x < l )		// fill left
					vga.blt_buf( x, y, l-1, y + font_ptr->max_font_height-1, 0);
				IMGjoinTrans(vga_front.buf_ptr(), vga_front.buf_pitch(), 
					vga_back.buf_ptr(), vga_back.buf_pitch(),
					l, y, sys.common_data_buf);
			}
			else
			{
				short backGroundWidth = *(short *)back_ground_bitmap;
				short backGroundHeight = *(1+(short *)back_ground_bitmap);
				int l = x_limit - textWidth + 1;
				if( x < l )
					vga_front.put_bitmap_area(0, 0, back_ground_bitmap,
					0, 0, MIN(l-x, backGroundWidth)-1, backGroundHeight-1 );
				vga_front.put_bitmap_trans(l, y, sys.common_data_buf);
			}
			break;
		
		default:
			err_here();
		}
		mouse.show_area();
	}
	else
	{
		switch( align_flag )
		{
		case 0:		// left justified
			IMGbltTrans( vga_back.buf_ptr(), vga_back.buf_pitch(),
				x, y, sys.common_data_buf);
			break;

		case 1:		// center justified
			{
				int l = x + (x_limit - x + 1 - textWidth ) / 2;
				IMGbltTrans( vga_back.buf_ptr(), vga_back.buf_pitch(),
					l, y, sys.common_data_buf);
			}

			// BUGHERE : fill left and right
			break;

		case -1:		// right justified
			IMGbltTrans( vga_back.buf_ptr(), vga_back.buf_pitch(),
				x_limit - textWidth + 1, y, sys.common_data_buf);
			break;
		
		default:
			err_here();
		}
	}
}

int GetA::cursor_x(int curPos)
{
	switch( align_flag )
	{
	case 0:		// left justified
		return x + font_ptr->text_width(input_field, curPos);
	case 1:		// center justified
		return x + ((x_limit-x) - font_ptr->text_width(input_field))/2 + font_ptr->text_width(input_field, curPos);
	case -1:		// right justified
		return x_limit - font_ptr->text_width(input_field) + font_ptr->text_width(input_field, curPos);
	default:
		err_here();
		return 0;
	}
}


// return 1 for selected
int GetA::detect_click()
{
	if( !enable_flag )
		return 0;

	if( !mouse_drag_flag )
	{
		int clickCount = mouse.any_click(x-font_ptr->max_font_width, y, x_limit, y+height()-1);
		if( clickCount == 1)
		{
			// set cursor_pos
			// scan from the last character, until the clicked x is
			// less than the character x
			for( cursor_pos = strlen(input_field); 
				cursor_pos > 0 && mouse.click_x() < cursor_x(cursor_pos);
				--cursor_pos );
			err_when( cursor_pos < 0 || cursor_pos > strlen(input_field));
			clear_select();
			mouse_drag_flag = 1;
			return 1;
		}
		else if( clickCount > 1 )
		{
			select_whole();
			return 1;
		}
	}
	else
	{
		if( !mouse.left_press )
		{
			mouse_drag_flag = 0;
		}
		for( cursor_pos = strlen(input_field); 
			cursor_pos > 0 && mouse.cur_x < cursor_x(cursor_pos);
			--cursor_pos );
		err_when( cursor_pos < 0 || cursor_pos > strlen(input_field));
		return 1;
	}
	return 0;
}


unsigned GetA::detect()
{
	unsigned keyCode = detect_key();
	if( keyCode )
		return keyCode;
	else
		return detect_click();
}

unsigned GetA::mark_begin()
{
	return MIN( cursor_pos, mark_cursor_pos );
}

unsigned GetA::mark_end()
{
	return MAX( cursor_pos, mark_cursor_pos );
}

void GetA::select_whole()
{
	cursor_pos = strlen(input_field);
	mark_cursor_pos = 0;
}

GetAGroup::GetAGroup(int n)
{
	geta_num = n;
   geta_array = new GetA[n];
	focused_geta = 0;
	enable_flag = 0;
}


GetAGroup::~GetAGroup()
{
	delete[] geta_array;
}


void GetAGroup::paint()
{
	for( int i = 0; i < geta_num; ++i )
	{
		geta_array[i].paint(focused_geta == i);
	}
}

int GetAGroup::detect()
{
	if( !enable_flag )
		return 0;

	err_when( focused_geta < 0 || focused_geta > geta_num);

	GetA *getPtr = &geta_array[focused_geta];
	err_when( !getPtr->enable_flag );

	unsigned keyCode = getPtr->detect_key();
	if( keyCode )
	{
		getPtr->paint(1);
		// detect change field focus button
		if( keyCode == KEY_RETURN || keyCode == KEY_DOWN || keyCode == KEY_TAB )
		{
			set_focus(-1);
		}
		else if( keyCode == KEY_UP )
		{
			set_focus(-2);
		}
		return 1;
	}

	// detect clicking on the area
	for( int i = 0; i < geta_num; ++i )
	{
		getPtr = &geta_array[i];
		if( getPtr->detect_click() )
		{
			set_focus(i);
			return 1;
		}
	}
	return 0;
}


// n = -1, next
// n = -2, prev
// n = -3, top
// n = -4, bottom
int GetAGroup::set_focus(int n, int paintFlag)
{
	int oldFocused = focused_geta;
	int i, newFocused;
	enable_flag = 1;
	err_when( n < -4 || n >= geta_num );

	switch(n)
	{
	case -1:
		for( i = 0, newFocused = (oldFocused+1) % geta_num;
			i < geta_num && !geta_array[newFocused].enable_flag;
			++i, newFocused = (newFocused+1) % geta_num );
		break;
	case -2:
		for( i = 0, newFocused = (oldFocused+geta_num-1) % geta_num; 
			i < geta_num && !geta_array[newFocused].enable_flag;
			++i, newFocused = (newFocused+geta_num-1) % geta_num );
		break;
	case -3:
		for( i = 0, newFocused = 0; 
			i < geta_num && !geta_array[newFocused].enable_flag;
			++i, newFocused = (newFocused+1) % geta_num );
		break;
	case -4:
		for( i = 0, newFocused = geta_num-1; 
			i < geta_num && !geta_array[newFocused].enable_flag;
			++i, newFocused = (newFocused+geta_num-1) % geta_num );
		break;
	default:
		newFocused = n;
	}

	err_when( !geta_array[newFocused].enable_flag );
	focused_geta = newFocused;

	if( paintFlag )
	{
		if( oldFocused != newFocused )
			geta_array[oldFocused].paint(0);
		geta_array[newFocused].paint(1);
	}

	return 1;
}

GetA& GetAGroup::operator[](int n)
{
	err_when( n < 0 || n >= geta_num);
	return geta_array[n];
}

int GetAGroup::operator()()
{
	return focused_geta;
}




