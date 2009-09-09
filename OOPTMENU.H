// Filename    : OOPTMENU.H
// Description : in-game option menu (async version)


#ifndef __OOPTMENU_H
#define __OOPTMENU_H


#include <OCONFIG.H>
#include <OBUTTCUS.H>
#include <OBUTT3D.H>
#include <OSLIDCUS.H>

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