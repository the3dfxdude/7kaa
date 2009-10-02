//Filename    : ONEWS.H
//Description : Object News

#ifndef __ONEWS_H
#define __ONEWS_H

#ifndef __OBUTTON_H
#include <OBUTTON.h>
#endif

#ifndef __OVBROWSE_H
#include <OVBROWSE.h>
#endif

#ifndef __ODYNARRB_H
#include <ODYNARRB.h>
#endif

//----- Maximum no. of news ---------//

enum { MAX_NEWS=2000 };

//--------- Define constant --------------//

#define DISP_NEWS_DAYS	60       // how long a news should stay on the screen before it disappears
#define DISP_NEWS_COUNT 5			// maximum no. of news message displayed on the screen at a time

//-------- Define news type ---------//

enum { NEWS_TYPE_NUM=1 };
enum { NEWS_NORMAL=0 };

enum { NEWS_WHO_NUM=4 };
enum { NEWS_DISP_ALL=0, NEWS_DISP_FRIENDLY, NEWS_DISP_PLAYER, NEWS_DISP_NONE };

//------- Define other constant -------//

enum { DESTROYER_NATION=1, DESTROYER_REBEL, DESTROYER_MONSTER, DESTROYER_UNKNOWN };

enum { NEWS_LOC_TOWN=1, NEWS_LOC_FIRM, NEWS_LOC_UNIT, NEWS_LOC_ANY };		// for News::loc_type

//--------- Define news id. ----------//

enum { NEWS_DIPLOMACY=1,
		 NEWS_TOWN_REBEL,
		 NEWS_MIGRATE,
		 NEWS_NEW_NATION,
		 NEWS_NATION_DESTROYED,
		 NEWS_NATION_SURRENDER,
		 NEWS_KING_DIE,
		 NEWS_NEW_KING,
		 NEWS_FIRM_DESTROYED,		// when your firm is destroyed
		 NEWS_FIRM_CAPTURED,			// when your firm is taken over by another nation or you take over the firm of another nation
		 NEWS_TOWN_DESTROYED,		// when the last unit in the town is killed by an enemy
		 NEWS_TOWN_ABANDONED, 		// when all villagers have left the village
		 NEWS_TOWN_SURRENDERED,		// A town surrenders to another natino
		 NEWS_MONSTER_KING_KILLED,
		 NEWS_MONSTER_FIRM_DESTROYED,
		 NEWS_SCROLL_ACQUIRED,
		 NEWS_MONSTER_GOLD_ACQUIRED,
		 NEWS_YOUR_SPY_KILLED,
		 NEWS_ENEMY_SPY_KILLED,
		 NEWS_UNIT_BETRAY, 			// your unit betray or other nation's betray and turn towards you
		 NEWS_UNIT_ASSASSINATED,
		 NEWS_ASSASSINATOR_CAUGHT,
		 NEWS_GENERAL_DIE,
		 NEWS_RAW_EXHAUST,
		 NEWS_TECH_RESEARCHED,
		 NEWS_LIGHTNING_DAMAGE,
		 NEWS_EARTHQUAKE_DAMAGE,
		 NEWS_GOAL_DEADLINE,
		 NEWS_WEAPON_SHIP_WORN_OUT,
		 NEWS_FIRM_WORN_OUT,
		 NEWS_CHAT_MSG,
		 NEWS_MULTI_RETIRE,
		 NEWS_MULTI_QUIT_GAME,
		 NEWS_MULTI_SAVE_GAME,
		 NEWS_MULTI_CONNECTION_LOST,
	  };

//------- Define struct News ---------//

#pragma pack(1)
struct News
{
public:
	char  id;             // id. of the news, NEWS_???

	char  type;           // news type   // type may be > NEWS_TYPE_NUM, for indicating that the news has been displayed in the stock window, do display it on the newspaper again
	char  news_type()     { return type%NEWS_TYPE_NUM; }

	long  news_date;           	 // date of the news

	char  nation_color1;     // nation color, can't use nation_recno directly, because it may bankrupt one day
	char  nation_color2;
	char	nation_race_id1;
	char	nation_race_id2;
	int   nation_name_id1;   // nation res. id of the nation that generate the news
	int   nation_name_id2;   // if the news is related to two nations (e.g. one nation buys the stock of another nation)

	char* nation_name1();
	char* nation_name2();

	char* king_name1(int addColor=0);
	char* king_name2(int addColor=0);

	short short_para1;
	short short_para2;
	short short_para3;
	short short_para4;
	short short_para5;

	char  loc_type;
	short loc_type_para;
	WORD  loc_type_para2;	// must use WORD as it will be used to store unit name id. 
	short loc_x, loc_y;		// location where the news happens

public:
	int 	put(int y, int detectAction, int& newsHeight);
	char* msg();         // return the news msg
	int	is_major();

	void 	set_loc(int xLoc, int yLoc, int locType, int locTypePara=0, int locTypePara2=0);
	int	is_loc_valid();

	//---- functions for return news string ----//

	void  diplomacy();
	void  town_rebel();
	void  migrate();
	void  new_nation();
	void 	nation_destroyed();
	void 	nation_surrender();
	void  king_die();
	void  new_king();
	void  firm_destroyed();
	void  firm_captured();
	void  town_destroyed();
	void  town_abandoned();
	void	town_surrendered();
	void  monster_king_killed();
	void  monster_firm_destroyed();
	void  scroll_acquired();
	void	monster_gold_acquired();
	void  your_spy_killed();
	void  enemy_spy_killed();
	void  unit_betray();
	void  unit_assassinated();
	void	assassinator_caught();
	void  general_die();
	void  raw_exhaust();
	void  tech_researched();
	void  lightning_damage();
	void  earthquake_damage();
	void	goal_deadline();
	void	weapon_ship_worn_out();
	void	firm_worn_out();
	void	chat_msg();
	void  multi_retire();
	void  multi_quit_game();
	void  multi_save_game();
	void  multi_connection_lost();
};
#pragma pack()

//-------- Define class NewsArray ----------//

class Unit;
class Font;

class NewsArray : public DynArray
{
public:
	//------ display options ------//

	char  news_type_option[NEWS_TYPE_NUM];
	char  news_who_option;
	char	news_add_flag;

	int	last_clear_recno;

public:
	NewsArray();

	void  init();
	void  deinit();

	void 	enable()      	{ news_add_flag = 1; }
	void	disable()		{ news_add_flag = 0; }

	void  reset();
	void  default_setting();
	void	set_font(Font*);

	int	detect();
	void	disp();
	int   put(int detectAction);

	void	clear_news_disp();

	News* add_news(int newsId, int newsType, int nationRecno=0, int nationRecno2=0, int forceAdd=0);
	void 	remove(int newsId, int shortPara1);

	//------ functions for adding news -------//

	void	diplomacy(int talkMsgRecno);
	void 	town_rebel(int townRecno, int rebelCount);
	void	migrate(int srcTownRecno, int desTownRecno, int raceId, int migratedCount, int firmRecno=0);
	void 	new_nation(int kingUnitRecno);
	void	nation_destroyed(int nationRecno);
	void	nation_surrender(int nationRecno, int toNationRecno);
	void  king_die(int nationRecno);
	void  new_king(int nationRecno, int kingUnitRecno);
	void  firm_destroyed(int firmRecno, Unit* attackUnit);
	void  firm_captured(int firmRecno, int takeoverNationRecno, int spyTakeover);
	void  town_destroyed(int townNameId, int xLoc, int yLoc, Unit* attackUnit);
	void  town_abandoned(int townRecno);
	void	town_surrendered(int townRecno, int toNationRecno);
	void  monster_king_killed(int monsterId, int xLoc, int yLoc);
	void  monster_firm_destroyed(int monsterId, int xLoc, int yLoc);
	void 	scroll_acquired(int acquireNationRecno, int scrollRaceId);
	void	monster_gold_acquired(int goldAmt);
	void  spy_killed(int spyRecno);
	void  unit_betray(int unitRecno, int betrayToNationRecno);
	void  unit_assassinated(int unitRecno, int spyKilled);
	void  assassinator_caught(int spyRecno, int targetRankId);
	void  general_die(int unitRecno);
	void  raw_exhaust(int rawId, int xLoc, int yLoc);
	void  tech_researched(int techId, int techVersion);
	void  lightning_damage(int xLoc, int yLoc, int objectId, int recno, int objectDie);
	void  earthquake_damage(int unitDamage, int unitDie, int townDamage, int firmDamage, int firmDie);
	void	goal_deadline(int yearLeft, int monthLeft);
	void 	weapon_ship_worn_out(int unitId, int weaponLevel);
	void 	firm_worn_out(int firmRecno);
	void 	chat_msg(int nationRecno, char* chatStr);
	void  multi_retire(int nationRecno);
	void  multi_quit_game(int nationRecno);
	void  multi_save_game();
	void  multi_connection_lost(int nationRecno);

	//--------------------------------------------//

	int   write_file(File*);
	int   read_file(File*);

	News* operator[](int recNo);
};

extern NewsArray news_array;

//-------------------------------------------//

#endif
