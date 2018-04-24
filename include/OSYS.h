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

//Filename	  : OSYS.H
//Description : Header file for Game class

#ifndef __OSYS_H
#define __OSYS_H

#include <ALL.h>
#include <storage_constants.h>
#include <stdint.h>

//------ Define common data buffer size  ------//

enum { COMMON_DATA_BUF_SIZE = 64*1024 };			// keep a 64K common buffer for all temporary memory storage like reading files temporarily into memory

//-------------- define constant ------------//

#define FRAMES_PER_DAY	10			// no. of frames per day

#define MAX_SCENARIO_PATH 2


//------------ sys_flag --------------//

enum { SYS_PREGAME=0, SYS_RUN };

//--------- game version ---------//

enum { VERSION_FULL,
		 VERSION_DEMO,
		 VERSION_MULTIPLAYER_ONLY };

//--------- Define info modes --------//

enum { VIEW_MODE_COUNT=10 };

enum { MODE_NORMAL,
		 MODE_NATION,
		 MODE_TOWN,
		 MODE_ECONOMY,
		 MODE_TRADE,
		 MODE_MILITARY,
		 MODE_TECH,
		 MODE_SPY,
		 MODE_RANK,
		 MODE_NEWS_LOG,
		 MODE_AI_ACTION,
	  };

//### begin alex 11/8 ###//
//------------------ define debug_seed_status_flag ----------------//
enum	{	NO_DEBUG_SYN = 0,
			DEBUG_SYN_LOAD_AND_COMPARE_ONCE,
			DEBUG_SYN_AUTO_SAVE,
			DEBUG_SYN_AUTO_LOAD,
		};
//#### end alex 11/8 ####//

//-------- Define class Sys -----------//

class Sys
{
public:
	char		game_version;			// VERSION_???

	char		sys_flag;
	char		init_flag;
	char		signal_exit_flag; // 0 - not exiting; 1 - exit to OS; 2 - exit to main menu
	char		need_redraw_flag;    // set to 1 if task switched back. After redraw, clear it
	char		toggle_full_screen_flag;
	char		cheat_enabled_flag;
	char		user_pause_flag;
	char		disp_fps_flag;

	char 		view_mode;				// the view mode can be MODE_???

	char		map_need_redraw;
	char		zoom_need_redraw;

	long        loaded_random_seed;   // only when loading a game, the random seed of the saved game; to be set as the seed for misc upon completing loading.

	//------ frame related vars -----//

	int 		day_frame_count;
	uint32_t	next_frame_time;		// next frame's time for maintaining a specific game speed

	//----- multiplayer vars ----//

	char		is_mp_game;
	uint32_t 	frame_count;  			// frame count, for is_sync_frame only
	char		is_sync_frame;			// whether sync should take place at the current frame (for handling one sync per n frames)
	char		mp_save_flag;			// indicate a request to save game in multi-player game
	uint32_t	mp_save_frame;			// save game in which frame

	//---- continous key string -----//

	enum { MAX_KEY_STR = 10 };       // Maximum 10 different key string

	int  key_str_pos[MAX_KEY_STR];  // for detecting cheating codes

	//-------- statistic --------//

	uint32_t	last_second_time;
	int		frames_in_this_second;
	int	 	frames_per_second;   // the actual frames per second

	//------- file paths -------//

	char  	cdrom_drive;

	char    dir_config[MAX_PATH+1];
	char  	dir_image[MAX_PATH+1];
	char  	dir_encyc[MAX_PATH+1];
	char  	dir_encyc2[MAX_PATH+1];
	char  	dir_music[MAX_PATH+1];
	char  	dir_movie[MAX_PATH+1];
	char  	dir_tutorial[MAX_PATH+1];

	union
	{
		char dir_scenario[MAX_PATH+1];
		char dir_scenario_path[MAX_SCENARIO_PATH][MAX_PATH+1];
	};

	//------- other vars --------//

	char*		common_data_buf;
	char		debug_session;
	char		testing_session;

public:
	Sys();
	~Sys();

	int		init();
	void		deinit();
	void		deinit_directx();
	void		deinit_objects();

	void		run(int=0);
	void		yield();
	void		yield_wsock_msg();

	void 		set_speed(int frameSpeed, int remoteCall=0);
	void 		set_view_mode(int viewMode, int viewingNationRecno=0, int viewingSpyRecno=0);
	// ##### begin Gilbert 22/10 #######//
	void		disp_view_mode(int observeMode=0);
	// ##### end Gilbert 22/10 #######//
	void		capture_screen();

	void 		disp_frame();
	void 		blt_virtual_buf();

	void		pause();
	void		unpause();

	void		show_error_dialog(const char *formatStr, ...);

	void		mp_request_save(uint32_t frame);
	void		mp_clear_request_save();

	//-------------- single player syn. game testing functions --------------//
	void		sp_create_seed_file(char *filename);
	void		sp_close_seed_file();
	void		sp_load_seed_file();
	void		sp_record_match_seed();
	void		sp_record_seed();
	void		sp_write_seed();
	void		sp_compare_seed();
	void		sp_seed_pos_reset();
	void		sp_seed_pos_set(int pos);

	//---- for setting game directories ----//

	int 		set_config_dir();
	int		chdir_to_game_dir();
	int		set_game_dir();

	//-------- for load/save games --------//

	int 		write_file(File* filePtr);
	int		read_file(File* filePtr);
	void		save_game();
	void		load_game();

private:
	int		init_directx();
	int 		init_objects();

	void		main_loop(int);
	void		detect();
	void		process();

	void		disp_button();
	void 		detect_button();

	void		disp_view();
	void		update_view();
	void		detect_view();

	void 		disp_map();
	void 		disp_zoom();

	int		should_next_frame();
	int		is_mp_sync( int *unreadyPlayerFlag );
	void		auto_save();

	void 		blt_next_frame();
	void		disp_frames_per_second();

	void		process_key(unsigned scanCode, unsigned skeyState);

	void		detect_letter_key(unsigned scanCode, unsigned skeyState);
	void		detect_function_key(unsigned scanCode, unsigned skeyState);
	void		detect_cheat_key(unsigned scanCode, unsigned skeyState);
	int			detect_debug_cheat_key(unsigned scanCode, unsigned skeyState);
	int 		detect_set_speed(unsigned scanCode, unsigned skeyState);

	int 		detect_key_str(int keyStrId, const char* keyStr);
};

extern Sys sys;
#ifdef DEBUG
extern char debug2_enable_flag;
extern File seedCompareFile;
//### begin alex 11/8 ###//
extern char debug_seed_status_flag;
//#### end alex 11/8 ####//
extern int	debug_sim_game_type;
#endif

//-------------------------------------//

#endif


