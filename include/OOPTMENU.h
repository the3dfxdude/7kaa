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

// Filename    : OOPTMENU.H
// Description : in-game option menu (async version)


#ifndef __OOPTMENU_H
#define __OOPTMENU_H


#include <OCONFIG.h>
#include <OBUTTCUS.h>
#include <OBUTT3D.h>
#include <OSLIDCUS.h>

class OptionMenu
{
public:
	int	active_flag;
	int	refresh_flag;
	int	update_flag;

	Config	old_config;

	SlideBar se_vol_slide;
	SlideBar music_vol_slide;
	SlideBar frame_speed_slide;
	SlideBar scroll_speed_slide;
	ButtonCustom race_button[MAX_RACE];
	ButtonCustomGroup help_group;
	ButtonCustomGroup news_group;
	ButtonCustomGroup report_group;
	ButtonCustomGroup show_icon_group;
	ButtonCustomGroup show_path_group;
	Button3D start_button, cancel_button;

public:
	OptionMenu();

	int	is_active()		{ return active_flag; }
	void	enter(char untilExitFlag);
	void	disp(int needRepaint=0);
	int	detect();
	void	exit(int action);
	void	abort();

	static int slide_to_percent_volume(int slideVolume);
	static int percent_to_slide_volume(int percentVolume);
};

extern OptionMenu option_menu;

#endif