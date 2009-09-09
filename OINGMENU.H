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
