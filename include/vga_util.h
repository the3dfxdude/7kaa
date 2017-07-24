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

//Filename    : vga_util.h
//Description : Vga utilities that should work with any backend.

#ifndef __VGA_UTIL_H
#define __VGA_UTIL_H

//-------- Define class VgaUtil ----------------//

class VgaUtil
{
public:
	VgaUtil();
	~VgaUtil();

	void d3_panel_up(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
	void d3_panel_down(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
	void d3_panel2_up(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
	void d3_panel2_down(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
	void separator(int x1, int y1, int x2, int y2);

	void blt_buf(int x1, int y1, int x2, int y2, int putMouseCursor=1);

	void disp_image_file(const char* fileName,int x1=0, int y1=0);
	void finish_disp_image_file();
};

extern VgaUtil vga_util;

//--------------------------------------------//

#endif
