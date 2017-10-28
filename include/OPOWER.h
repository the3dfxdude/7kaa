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

//Filename    : OPOWER.H
//Description : Header file of Object Power

#ifndef __OPOWER_H
#define __OPOWER_H

#ifndef __ALL_H
#include <ALL.h>
#endif

//-------- Define command id. -----------//

enum { COMMAND_BUILD_FIRM=1,
		 COMMAND_ASSIGN,
		 COMMAND_BURN,
		 COMMAND_SETTLE,
		 COMMAND_SET_CARAVAN_STOP,
		 COMMAND_SET_SHIP_STOP,
		 COMMAND_BUILD_WALL,
		 COMMAND_DESTRUCT_WALL,
		 COMMAND_GOD_CAST_POWER,
	  };

// --------- define ScreenObjectType --------//

enum ScreenObjectType
{
	SCREEN_OBJECT_NONE,
	SCREEN_OBJECT_FRIEND_UNIT,
	SCREEN_OBJECT_UNIT_GROUP,
	SCREEN_OBJECT_ENEMY_UNIT,
	SCREEN_OBJECT_SPY_UNIT,		// our spy, shealthed to other nation, selected object only
	SCREEN_OBJECT_FRIEND_TOWN,
	SCREEN_OBJECT_ENEMY_TOWN,
	SCREEN_OBJECT_FRIEND_FIRM,
	SCREEN_OBJECT_ENEMY_FIRM,
	SCREEN_OBJECT_WALL,
	SCREEN_OBJECT_SITE,
};


//-------- Map modes ------------//

enum { MAP_MODE_NUM = 5 };
enum { MAP_NORMAL=0, MAP_CLIMATE, MAP_RESOURCE, MAP_PROFIT, MAP_LINK };

//----------- Define constant -----------//

enum { MAX_KEY_STR = 5 };       // Maximum 5 different key string

//--------- Define class Power ----------//

struct Location;

class Power
{
public:
	int   		command_id;
	int   		command_unit_recno;
	int         command_para;

	char			win_opened;
	char			enable_flag;

	int  			key_str_pos[MAX_KEY_STR];  // for detecting cheating codes

public:
	Power();
	~Power();

	void			init();

	void 			enable()		{ enable_flag=1; }
	void 			disable()	{ enable_flag=0; }

	void			issue_command(int,int=0,int=0);
	void			reset_command()			{ command_id=0; }

	void			mouse_handler();
	void  		reset_selection();

	char*			get_link_icon(char linkStatus, int sameNation);

	int 			write_file(File* filePtr);
	int			read_file(File* filePtr);

	//------- cursor related functions ------//

	int			choose_cursor(int scrnX, int scrnY,
						ScreenObjectType selectedObjectType, short selectedObjectRecno,
						ScreenObjectType pointingObjectType, short pointingObjectRecno);

	int			choose_cursor_units(short selectedUnitRecno, short pointingUnitRecno);
	int			choose_cursor_unit_group(short pointingUnitRecno);

	ScreenObjectType	find_selected_type( short *);
	ScreenObjectType	find_pointing_type( Location *, short *);

public:
	int  			detect_frame();
	int 			detect_action();
	// ###### begin Gilbert 31/7 ######//
	// Location* 	test_detect(int curX, int curY);
	Location*       test_detect(int curX, int curY, char *mobileType=NULL);
	// ###### end Gilbert 31/7 ######//
	int 		 	detect_select(int selX1, int selY1, int selX2, int selY2, int recallGroup, int shiftSelect);
	int			detect_scroll();
	// ###### begin Gilbert 22/10 #######//
	int			unit_can_assign_firm(int unitRecno, int firmRecno, int ownNationRecno);
	// ###### end Gilbert 22/10 #######//
	//### begin alex 19/3 ###//
	short			select_active_unit(short *selectedArray, short selectedCount);
	//#### end alex 19/3 ####//
};

extern Power power;

//---------------------------------------//

#endif
