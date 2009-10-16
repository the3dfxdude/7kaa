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

//Filename	  : OINFO.H
//Description : Header file for class Info

#ifndef __OINFO_H
#define __OINFO_H

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif

#ifndef __OWORLD_H
#include <OWORLD.h>
#endif

//---------- Define constant ---------//

enum { INFO_REPAINT=10, INFO_UPDATE, INFO_SPECIAL };

//--------- Define section positions ---------//

#define TOP_MENU_X1	 4
#define TOP_MENU_Y1   4
#define TOP_MENU_X2   (ZOOM_X2-4)
#define TOP_MENU_Y2	 30

#define INFO_X1 	586
#define INFO_Y1   265
#define INFO_X2   790
#define INFO_Y2	589

#define MSG_X1		INFO_X1
#define MSG_Y1 	(INFO_Y2-29)
#define MSG_X2		INFO_X2
#define MSG_Y2		INFO_Y2

//------- Nation report  detail display mode -------//

enum { MAX_NATION_REPORT_MODE = 5 };

enum { NATION_REPORT_INFO=1,
		 NATION_REPORT_TALK,
		 NATION_REPORT_CHAT,
		 NATION_REPORT_TALK_LOG,
		 NATION_REPORT_DEBUG
	  };

enum { MAX_RANK_TYPE	= 5 };

enum { CHAT_STR_LEN=100 };

enum { MAX_REMOTE_CHAT_STR=10 };

//-------- Define chat_receiver_type --------//

enum { MAX_CHAT_RECEIVER_TYPE = 3 };

enum { CHAT_RECEIVER_CURRENT=1,
		 CHAT_RECEIVER_ALL_ALLY,
		 CHAT_RECEIVER_ALL_HUMAN };

//--------- Define struct ChatInfo ----------//

#pragma pack(1)
struct ChatInfo
{
	int   received_date;
	char	from_nation_recno;
	char	chat_str[CHAT_STR_LEN+1];
};
#pragma pack()

//--------- Define struct TalkMsgDisp --------//

struct TalkMsgDisp
{
	short recno;
	int	date;
	char	is_reply;
};

//----------- Define class Info -------------//

#pragma pack(1)
class Info
{
public:
	int  	game_start_date;   // set to Game::game_start_year in Info::init(), the actual date the game begins, not the scenario begins
	int  	game_date;
	int  	game_day;
	int  	game_month;
	int  	game_year;

	int  	goal_deadline;
	short	goal_difficulty;
	short goal_score_bonus;

	int  	week_day;      // = game_date&7
	int  	year_day;		  // the nth day in a year
	int  	year_passed;   // no. of years passed since playing

	int  random_seed;

	unsigned long start_play_time;	// the time player start playing the game today
	unsigned long total_play_time;  	// total time the player has played in all saved games

	//------- report browsers recno -------//

	short viewing_nation_recno;		// which nation the player is viewing at with the reports.
	short	viewing_spy_recno;			// the recno of the spy viewing the secret report of other nations
	short	default_viewing_nation_recno;

	short browse_nation_recno;
	short browse_race_recno;
	short browse_firm_recno;
	short browse_income_recno;
	short browse_expense_recno;
	short browse_troop_recno;
	short browse_unit_recno;
	short browse_tech_recno;
	short browse_god_recno;
	short browse_town_recno;
	short browse_spy_recno;
	short browse_caravan_recno;
	short browse_ship_recno;
	short	browse_talk_msg_recno;
	short browse_news_recno;
	short browse_ai_action_recno;
	short browse_ai_attack_recno;

	//------ vars of the nation report ------//

	char	nation_report_mode;
	short	last_talk_nation_recno;
	char 	player_reply_mode;

	//------ chat string --------//

	char		chat_receiver_type;
	char  	player_chat_str[CHAT_STR_LEN+1];
	ChatInfo remote_chat_array[MAX_REMOTE_CHAT_STR];

	//---------------------------------//

	char  last_write_offset;				// it is used for an offset locator for excluding static member vars during reading and writing files

	char* play_time_str(); // return total time played in string
	char* game_duration_str();

	short get_report_data(int recNo);
	short	get_report_data2(int recNo);

	//------ keep a copy of bitmap of the panel texture -----//

	char*		info_background_bitmap;		// declare them static, so they won't be saved
	DynArray report_array;
	DynArray report_array2;
	DynArray talk_msg_disp_array;

public:
	Info();
	~Info();

	void	init();
	void	deinit();

	void	init_random_seed(int randomSeed);

	void 	disp_panel();

	void	disp();
	void	update();
	int 	detect();

	void	next_day();

	void 	save_game_scr();
	void 	rest_game_scr();
	void 	free_game_scr();

	void	draw_selected();
	void	disp_heading();

	int   disp_loyalty(int x, int y, int x2, int curLoyalty, int targetLoyalty, int nationRecno, int refreshFlag);

	void	disp_military(int refreshFlag);
	void	detect_military();

	void	disp_nation(int refreshFlag);
	void 	detect_nation();

	void	disp_economy(int refreshFlag);
	void 	detect_economy();

	void	disp_town(int refreshFlag);
	void 	detect_town();

	void	disp_trade(int refreshFlag);
	void 	detect_trade();

	void	disp_tech(int refreshFlag);
	void 	detect_tech();

	void	disp_rank(int refreshFlag);
	void 	detect_rank();

	void	disp_spy(int refreshFlag);
	void 	detect_spy();

	void	disp_ai_action(int refreshFlag);
	void 	detect_ai_action();

	void	disp_news_log(int refreshFlag);
	void 	detect_news_log();

	void	init_player_reply(int talkToNationRecno);
	void 	player_reply_chat(int withNationRecno);
	void	process_viewing_spy();

	void  set_rank_data(int onlyHasContact);
	char* get_rank_pos_str(int rankType, int nationRecno);
	int   get_rank_score(int rankType, int nationRecno);		// score functions
	int 	get_total_score(int nationRecno);

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);
};
#pragma pack()

extern Info info;

//------------------------------------------//

#endif
