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

//Filename    : syswin.h
//Description : Header file for window management class 'SysWindow'

#ifndef __SYSWINDOW_H
#define __SYSWINDOW_H

#include <windows.h>

//-------- Define class SysWindow -----------//

class SysWindow
{
private:
	char init_flag;

public:
	HINSTANCE	app_hinstance; // handle of the application running
	HWND		main_hwnd;

	SysWindow();
	~SysWindow();

	int  init();
	void deinit();

	void handle_messages();

	void flag_redraw();
};


//-------------------------------------//

extern SysWindow window;

#endif //__SYSWINDOW_H
