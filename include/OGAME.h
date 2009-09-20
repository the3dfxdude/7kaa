//Filename	  : OGAME.H
//Description : Header file for Game class

#ifndef __OGAME_H
#define __OGAME_H

#ifndef __WINDOWS_
#include <windows.h>
#endif

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OCONFIG_H
#include <OCONFIG.h>
#endif

//-------- Define game modes -----------//

enum { GAME_PREGAME=1,
		 GAME_SINGLE_PLAYER,		// game_mode is set once the player clicks an option on the screen, the game doesn't have to be actually running.
		 GAME_TUTORIAL,
		 GAME_TEST,
		 GAME_MULTI_PLAYER,
		 GAME_CAMPAIGN,
		 GAME_ENCYCLOPEDIA,
		 // ####### begin Gilbert 2/9 ######//
		 GAME_CREDITS
		 // ####### end Gilbert 2/9 ######//
	  };

//--------- Define struct ColorRemapMethod ----------//

struct ColorRemapMethod
{
	int	primary_color;
	int	secondary_color;
};

//--------- Define struct ColorRemap ----------//

struct ColorRemap
{
public:
	char  main_color;
	char	color_table[256];

public:
	void	set_remap_color(ColorRemapMethod*);
   void	load(char*);
};

//----- define struct ScenInfo -------//

struct ScenInfo
{
	enum { SCEN_NAME_LEN=80 };

	char* 	file_name;
	char  	scen_name[SCEN_NAME_LEN+1];
//#ifdef AMPLUS
	char		dir_id;			// real path look from DIR_SCENARIO_PATH(dir_id)
//#endif
	short		goal_difficulty;
	short 	goal_score_bonus;
};

//-------- Define class Game -----------//

struct Location;
struct NewNationPara;

class Game
{
public:
	char			init_flag;
	char			started_flag;
	char			game_mode;
	char			game_has_ended;		// whether game_end() has been called once already and the player is now just staying in the game to continue to play or observe

	//-------- color remap info -------//

	ColorRemap	color_remap_array[MAX_COLOR_SCHEME+1];

public:
	Game();

	int			init(int loadGameCall=0);
	void			deinit(int loadGameCall=0);

	void			main_menu();
   void			in_game_menu();
	int			in_game_option_menu();
	void 			game_end(int winNationRecno, int playerDestroyed=0, int surrenderToNationRecno=0, int retireFlag=0);

	int 			select_run_scenario();
	int 			select_scenario(int scenCount, ScenInfo* scenInfoArray);
	int 			run_scenario(ScenInfo* scenInfo);

	void 			demo_disp_ad_page();
	void			demo_disp_logo();

	void 			mp_broadcast_setting();

	char*			get_color_remap_table(int nationRecno, int selectedFlag);
	// ###### begin Gilbert 24/10 #######//
	static void	disp_gen_map_status( int curStep, int maxStep, int section );
	// ###### end Gilbert 24/10 #######//

	// ###### begin Gilbert 13/2 #######//
	void 			multi_player_menu(char *cmdLine);
	// ###### end Gilbert 13/2 #######//

	int 			write_file(File* filePtr);
	int			read_file(File* filePtr);

private:
	void			init_remap_table();

	void			disp_version();
	void			run_main_menu_option(int);
	void 			single_player_menu();
//	void 			multi_player_menu();

	void			single_player_game(int);
	void			multi_player_game(char *cmdLine);
	void			test_game();
	void			load_mp_game(char *saveFileName, char *cmdLine);
	void			view_encyclopedia();
	void			view_credits();

	//------- multiplayer game functions -------//

	int 			mp_select_mode(char *saveGameName);
	int			mp_select_option(NewNationPara*, int*);
	int			mp_select_service();
	int			mp_select_session();
	void			mp_disp_players();
	int			mp_select_load_option(char *);
};

extern Game game;
extern char game_demo_mode, game_design_mode;

//-------------------------------------//

#endif


