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

// Filename    : OGETA.H
// Description : get field, non-blocking


#ifndef __OGETA_H
#define __OGETA_H

class Font;

// ---------- Define class GetA ---------- //

class GetA
{
public:
	short x;
	short y;
	short x_limit;
	char*	input_field;
	unsigned	field_len;
	unsigned	cursor_pos;
	unsigned mark_cursor_pos;
	Font*	font_ptr;
	char	align_flag;
	char	enable_flag;
	char	esc_key_flag;		// detect esc key to clear the field
	char* back_ground_bitmap;

	char    hide_flag;
	char*   hide_field;
	char*   disp_field;

	char	mouse_drag_flag;

public:
	GetA();
	~GetA();
	void	init(int x1, int y1, int x2, char *field, unsigned length, Font *, char align, char detectEsc, char hide=0);
	int	height();
	void	clear();
	unsigned	detect_key();
	int	detect_click();
	unsigned	detect();
	void	paint(int paintCursor=1);
	int	cursor_x(int curPos);
	void	clear_select()				{ mark_cursor_pos = cursor_pos; }
	unsigned mark_begin();
	unsigned mark_end();
	int	is_select()					{ return mark_cursor_pos != cursor_pos; }
	void	select_whole();
};

// ---------- define class GetAGroup ---------- //

class GetAGroup
{
public:
   int		geta_num;
   GetA*		geta_array;
	int		focused_geta;
	char		enable_flag;

public:
	GetAGroup(int);
	~GetAGroup();

   void	paint();
   int	detect();
	int	set_focus(int, int paintFlag=1);

   GetA& operator[](int);
	int	operator()();
};


#endif


