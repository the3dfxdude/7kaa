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

// Filename    : OINGMENU.H
// Description : in-game menu (async version)


#ifndef __OINGMENU_H
#define __OINGMENU_H

class InGameMenu
{
public:
	enum { GAME_OPTION_COUNT = 8 };

	int	active_flag;
	int	refresh_flag;

	char game_menu_option_flag[GAME_OPTION_COUNT];
	static unsigned menu_hot_key[GAME_OPTION_COUNT];

public:
	InGameMenu();

	int	is_active()		{ return active_flag; }
	void	enter(char untilExitFlag);
	void	disp(int needRepaint=0);
	int	detect();
	void	exit(int action);
	void	abort();
};

extern InGameMenu in_game_menu;

#endif
