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

//Filename    : OGAMEND.CPP
//Description : Game ending screen

#include <OVGA.h>
#include <vga_util.h>
#include <OVGALOCK.h>
#include <ODATE.h>
#include <OREMOTE.h>
#include <OBOX.h>
#include <OCONFIG.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <ORACERES.h>
#include <OGAME.h>
#include <OGAMHALL.h>
#include <ONATION.h>
#include <OMOUSECR.h>
#include <OMUSIC.h>
#include <OOPTMENU.h>
#include <OINGMENU.h>
#include <PlayerStats.h>
// ####### begin Gilbert 29/10 ########//
#include <OPOWER.h>
// ####### end Gilbert 29/10 ########//
#include "gettext.h"


#define TXT_X1 ((VGA_WIDTH >> 1) - 280)
#define TXT_Y1 ((VGA_HEIGHT >> 1) - 184)
#define BG_X1  ((VGA_WIDTH >> 1) - 400)
#define BG_Y1  ((VGA_HEIGHT >> 1) - 300)


//-------- Declare static vars & functions ---------//

static int  disp_score(int winFlag);
static void disp_goal_str(int winNationRecno);
static void disp_losing_str(int surrenderToNationRecno);
static void disp_retire_str();
static void disp_ranking();
static void disp_stat();

static void put_stat(int y, const char* desStr, const char* dispStr);
static void put_stat(int y, const char* desStr, int dispValue);
static void put_ranking(int y, int nationRecno);

extern SaveGameInfo current_game_info;

//---------- Begin of function Game::game_end --------//
//
// <int> winNationRecno - the recno of the nation that won the game.
//								  0 - if you are just defeated or surrender
//										to another kingdom, and no other kingdom
//									   has won yet.
// [int] playerDestroyed - whether the player's nation has been destroyed or not. 
//									(default: 0)
// [int] surrenderToNationRecno - the nation your surrender to.
// [int] retireFlag		 - 1 if the player retires
//
void Game::game_end(int winNationRecno, int playerDestroyed, int surrenderToNationRecno, int retireFlag)
{
	//--- set scenario as complete if they didn't retire ---//
	if(!retireFlag)
		playerStats.set_scenario_play_status(current_game_info.game_name, nsPlayerStats::PlayStatus::COMPLETED);


	//--- skip all game ending screens if in demo mode ---//

	if( game_mode == GAME_DEMO )
	{
		sys.signal_exit_flag = 2;
		return;
	}

	//--- if the player has already won/lost the game and is just staying/observing the game ---//

	if( game_has_ended && !retireFlag )		// don't repeat displaying the winning/losing screen
		return;

	// ------ quit any menu mode ------//
	if( option_menu.is_active() )
	{
		option_menu.abort();
	}
	if( ::in_game_menu.is_active() )
	{
		::in_game_menu.abort();
	}

	//------ set the quit siginal -------//

	sys.signal_exit_flag = 2;		// set it first to disable Power::mouse.handler()
	mouse_cursor.set_frame(0);
	// ####### begin Gilbert 29/10 #######//
	mouse_cursor.set_icon(CURSOR_NORMAL);
	// ####### end Gilbert 29/10 #######//

	info.save_game_scr();

	int useBackBuf = vga.use_back_buf;

	vga.use_front();

	//------- display the winning/losing picture -------//

	int songId = 10;
	int winFlag = 0;

	if( !retireFlag )		// don't display this when retire
	{
		const char* fileName;

		if( winNationRecno && (winNationRecno == nation_array.player_recno) )
		{
			fileName = race_res[(~nation_array)->race_id]->code;
			songId = 9;
			winFlag = 1;
		}
		else
			fileName = "LOSEGAME";

		vga_util.disp_image_file(fileName, BG_X1, BG_Y1);

		music.play(songId, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0);
		mouse.wait_press(60);		// 60 seconds to time out
	}
	else
	{
		music.play(songId, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0);
	}

	//------- display the statistic -------//

	vga_util.disp_image_file("RESULTS", BG_X1, BG_Y1);

	if( winNationRecno )
	{
		disp_goal_str(winNationRecno);
	}
	else if( retireFlag )
	{
		disp_retire_str();
	}
	else
	{
		disp_losing_str(surrenderToNationRecno);
	}

	disp_stat();

	mouse.wait_press(60);		// 60 seconds to time out

	//-------- display ranking and score ----------//

	vga_util.disp_image_file("RESULTS", BG_X1, BG_Y1);

	info.set_rank_data(0);		// count all nations, not only those that have contact with the player

	disp_ranking();

	int totalScore = disp_score(winFlag);

	mouse.wait_press(60);		// 60 seconds to time out

	//--- if the player has managed to get into the hall of fame ---//

	if( !game_has_ended )
	{
		if( !hall_of_fame.add_hall_of_fame(totalScore) )
			vga_util.finish_disp_image_file();		// if add_hall_of_fame() has displayed the bitmap, it should have called vga_util.finish_disp_image_file() already
	}
	else
	{
		vga_util.finish_disp_image_file();
	}

	//--------- set game_has_ended to 1 --------//

	music.stop();
	game_has_ended = 1;

	//----------- reset all goals -----------//

	config.goal_destroy_monster     = 0;
	config.goal_population_flag     = 0;
	config.goal_economic_score_flag = 0;
	config.goal_total_score_flag 	  = 0;
	config.goal_year_limit_flag     = 0;

	//--- otherwise, ask if the player wants to stay in the game ---//

	#ifndef DEMO		// cannot continue to stay in the game in the demo version

	if( !retireFlag && !remote.is_enable() )		// can't stay in the game in a multiplayer game
	{
		vga_front.bar( 0, 0, VGA_WIDTH-1, VGA_HEIGHT-1, V_BLACK );		// clear the screen

		// ###### begin Gilbert 29/10 ######//
		char powerWinFlag = power.win_opened;
		power.win_opened = 1;
		if( box.ask( _("Do you want to continue to stay in the game?"), _("Yes"), _("No") ) )
			sys.signal_exit_flag = 0;
		power.win_opened = powerWinFlag;
		// ###### end Gilbert 29/10 ######//
	}

	#endif

	//-------- if it quits now ----------//

	if( sys.signal_exit_flag )
	{
		info.free_game_scr();

		vga.use_back_buf = useBackBuf;
	}
	else
	{
		//---- otherwise restore the screen and continue to play ----//

		info.rest_game_scr();

		vga.use_back_buf = useBackBuf;

		//---- reveal the whole world for staying in the game after being destroyed ----//

		if( playerDestroyed && !retireFlag )
		{
			world.unveil(0, 0, MAX_WORLD_X_LOC-1, MAX_WORLD_Y_LOC-1);
			world.visit(0, 0, MAX_WORLD_X_LOC-1, MAX_WORLD_Y_LOC-1, 0, 0);

			config.blacken_map = 0;
			config.fog_of_war  = 0;
		}
	}
}
//----------- End of function Game::game_end ---------//


//----------- Begin of static function disp_goal_str -----------//

static void disp_goal_str(int winNationRecno)
{
	Nation* winNation = nation_array[winNationRecno];
	String  str, str2;

	str2 = "";

	//---- if the player has achieved one of its goals ----//

	if( winNation->goal_destroy_monster_achieved() )
	{
		str = _("All Fryhtans have been Destroyed !");
		if( winNationRecno == nation_array.player_recno )
		{
			str2 = _("Your Kingdom has Achieved the Highest Fryhtan Battling Score !");
		}
		else
		{
			// TRANSLATORS: <King>'s Kingdom has Achieved the Highest Fryhtan Battling Score !
			snprintf(str2, MAX_STR_LEN+1, _("%s's Kingdom has Achieved the Highest Fryhtan Battling Score !"), nation_array[winNationRecno]->king_name(1));
		}
	}

	//-----------------------------------//

	else if( winNation->goal_population_achieved() )
	{
		if( winNationRecno == nation_array.player_recno )
		{
			// TRANSLATORS: Part of "Your Kingdom has Reached its Population/Economic Score/Total Score Goal of <Number> !"
			str = _("Your Kingdom has Reached");
		}
		else
		{
			// TRANSLATORS: Part of "<King>'s Kingdom has Reached its Population/Economic Score/Total Score Goal of <Number> !"
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom has Reached"), nation_array[winNationRecno]->king_name(1));
		}
		// TRANSLATORS: Part of "Your Kingdom (or <King's Kingdom>) has Reached its Population Goal of <Number> !"
		snprintf(str2, MAX_STR_LEN+1, _("its Population Goal of %s !"), misc.format(config.goal_population));
	}

	//-----------------------------------//

	else if( winNation->goal_economic_score_achieved() )
	{
		if( winNationRecno == nation_array.player_recno )
		{
			// TRANSLATORS: Part of "Your Kingdom has Reached its Population/Economic Score/Total Score Goal of <Number> !"
			str = _("Your Kingdom has Reached");
		}
		else
		{
			// TRANSLATORS: Part of "<King>'s Kingdom has Reached its Population/Economic Score/Total Score Goal of <Number> !"
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom has Reached"), nation_array[winNationRecno]->king_name(1));
		}
		// TRANSLATORS: Part of "Your Kingdom (or <King's Kingdom>) has Reached its Economic Score Goal of <Number> !"
		snprintf(str2, MAX_STR_LEN+1, _("its Economic Score Goal of %s !"), misc.format(config.goal_economic_score));
	}

	//-----------------------------------//

	else if( winNation->goal_total_score_achieved() )
	{
		if( winNationRecno == nation_array.player_recno )
		{
			// TRANSLATORS: Part of "Your Kingdom has Reached its Population/Economic Score/Total Score Goal of <Number> !"
			str = _("Your Kingdom has Reached");
		}
		else
		{
			// TRANSLATORS: Part of "<King>'s Kingdom has Reached its Population/Economic Score/Total Score Goal of <Number> !"
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom has Reached"), nation_array[winNationRecno]->king_name(1));
		}
		// TRANSLATORS: Part of "Your Kingdom (or <King's Kingdom>) has Reached its Total Score Goal of <Number> !"
		snprintf(str2, MAX_STR_LEN+1, _("its Total Score Goal of %s !"), misc.format(config.goal_total_score));
	}

	//-----------------------------------//

	else			// ( winNation->goal_destroy_nation_achieved() )
	{
		if( winNationRecno == nation_array.player_recno )
		{
			str = _("Your Kingdom has Defeated All Other Kingdoms !");
		}
		else
		{
			// TRANSLATORS: <King>'s Kingdom has Defeated All Other Kingdoms !
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom has Defeated All Other Kingdoms !"), nation_array[winNationRecno]->king_name(1));
		}
	}

	//-----------------------------------//

	int y=40;

	if( winNationRecno != nation_array.player_recno )
	{
		font_bible.center_put(BG_X1, BG_Y1 + 30, VGA_WIDTH-1, BG_Y1 + 60, _("You Have Lost the Game !") );
		y=60;
	}

	font_bible.center_put(BG_X1, BG_Y1 + y   , VGA_WIDTH-1, BG_Y1+y+30, str  );
	font_bible.center_put(BG_X1, BG_Y1 + y+30, VGA_WIDTH-1, BG_Y1+y+60, str2 );
}
//----------- End of static function disp_goal_str -----------//


//----------- Begin of static function disp_losing_str -----------//

static void disp_losing_str(int surrenderToNationRecno)
{
	String str;

	if( surrenderToNationRecno )		// you surrender to another kingdom
	{
		// TRANSLATORS: You Surrendered to <King>'s Kingdom on <Date>.
		snprintf(str, MAX_STR_LEN+1, _("You Surrendered to %s's Kingdom on %s."), nation_array[surrenderToNationRecno]->king_name(1), date.date_str(info.game_date));
	}

	// You failed to achieve the goal within the time limit

	else if( config.goal_year_limit_flag && info.game_date >= info.goal_deadline )
	{
		str = _("Your Kingdom has Failed to Achieve its Goal Within the Time Limit.");
	}
	else		// you're defeated by another kingdom
	{
		str = _("Your Kingdom has Gone Down to Ignominious Defeat !");
	}

	font_bible.center_put(BG_X1, BG_Y1, VGA_WIDTH-1, BG_Y1 + 139, str );
}
//----------- End of static function disp_losing_str -----------//


//----------- Begin of static function disp_retire_str -----------//

static void disp_retire_str()
{
	String str;

	// TRANSLATORS: You Retired on <Date>.
	snprintf(str, MAX_STR_LEN+1, _("You Retired on %s."), date.date_str( info.game_date ));

	font_bible.center_put(BG_X1, BG_Y1, VGA_WIDTH - 1, BG_Y1 + 139, str);
}
//----------- End of static function disp_retire_str -----------//


//-------- Begin of static function disp_stat --------//
//
static void disp_stat()
{
	int y = BG_Y1 + 140;

	Nation* nationPtr = ~nation_array;

	put_stat( y		, _("Duration of Your Rule"), info.game_duration_str() );
	put_stat( y+=20, _("Total Gaming Time"), info.play_time_str() );

	put_stat( y+=30, _("Final Population"), nationPtr->all_population() );
	put_stat( y+=20, _("Final Treasure")  , misc.format((int)nationPtr->cash,2) );

	put_stat( y+=30, _("Enemy Soldiers Dispatched"), nationPtr->enemy_soldier_killed );
	put_stat( y+=20, _("King's Soldiers Martyred")  , nationPtr->own_soldier_killed );

	put_stat( y+=30, _("Enemy Weapons Destroyed")		  , nationPtr->enemy_weapon_destroyed );
	put_stat( y+=20, _("King's Weapons Rendered Obsolete"), nationPtr->own_weapon_destroyed );

	put_stat( y+=30, _("Enemy Ships Sunk")   , nationPtr->enemy_ship_destroyed );
	put_stat( y+=20, _("King's Ships Missing"), nationPtr->own_ship_destroyed );

	put_stat( y+=30, _("Enemy Buildings Destroyed")   , nationPtr->enemy_firm_destroyed );
	put_stat( y+=20, _("King's Buildings Cleared"), nationPtr->own_firm_destroyed );

	put_stat( y+=30, _("Enemy Civilians Collaterally Damaged"), nationPtr->enemy_civilian_killed );
	put_stat( y+=20, _("King's Civilians Cruelly Murdered")    , nationPtr->own_civilian_killed );
}
//----------- End of static function disp_stat -----------//


//-------- Begin of static function put_stat --------//
//
static void put_stat(int y, const char* desStr, const char* dispStr)
{
	font_bible.put(BG_X1 + 140, y, desStr );
	font_bible.put(BG_X1 + 570, y, dispStr );
}
//----------- End of static function put_stat -----------//


//-------- Begin of static function put_stat --------//
//
static void put_stat(int y, const char* desStr, int dispValue)
{
	font_bible.put(BG_X1 + 140, y, desStr );
	font_bible.put(BG_X1 + 570, y, misc.format(dispValue) );
}
//----------- End of static function put_stat -----------//


//-------- Begin of static function disp_ranking --------//
//
static void disp_ranking()
{
	//--------- display descriptions ---------//

	int x= BG_X1+20, y= BG_Y1+76;

	font_bible.put( x+20 , y+7, _("Kingdom") );
	font_bible.put( x+260, y+7, _("Population") );
	font_bible.put( x+370, y+7, _("Military") );
	font_bible.put( x+470, y+7, _("Economy") );
	font_bible.put( x+562, y+7, _("Reputation") );

#if(defined(SPANISH))
	font_bible.put( x+670, y   , "Lucha" );
	font_bible.put( x+670, y+14, "Fryhtan" );
#else
	// TRANSLATORS: Part of "Fryhtan Battling"
	font_bible.put( x+670, y   , _("Fryhtan") );
	// TRANSLATORS: Part of "Fryhtan Battling"
	font_bible.put( x+670, y+14, _("Battling") );
#endif

	//--------- display rankings -----------//

	put_ranking(y+=36, nation_array.player_recno);

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_array.player_recno )
			continue;

		put_ranking( y+=30, i );
	}
}
//----------- End of static function disp_ranking -----------//


//-------- Begin of static function put_ranking --------//
//
static void put_ranking(int y, int nationRecno)
{
	Nation* nationPtr = nation_array[nationRecno];

	int x= BG_X1+20;

	nationPtr->disp_nation_color(x, y+5);

	font_bible.put( x+20, y, nationPtr->nation_name() );

	int y2 = y+font_bible.height()-1;

	font_bible.center_put( x+260, y, x+340, y2, info.get_rank_pos_str(1, nationRecno) );
	font_bible.center_put( x+370, y, x+435, y2, info.get_rank_pos_str(2, nationRecno) );
	font_bible.center_put( x+470, y, x+540, y2, info.get_rank_pos_str(3, nationRecno) );
	font_bible.center_put( x+562, y, x+640, y2, info.get_rank_pos_str(4, nationRecno) );
	font_bible.center_put( x+670, y, x+730, y2, info.get_rank_pos_str(5, nationRecno) );
}
//----------- End of static function put_ranking -----------//


//--------- Begin of static function disp_score ---------//
//
static int disp_score(int winFlag)
{
	int x= BG_X1+200, y= BG_Y1+360;

	static const char* rankStrArray[] =
	{ N_("Population Score"), N_("Military Score"), N_("Economic Score"),
	  N_("Reputation Score"), N_("Fryhtan Battling Score") };

	//------ display individual scores ---------//

	int rankScore, totalScore=0;
	int viewNationRecno = nation_array.player_recno;

	for( int i=0 ; i<MAX_RANK_TYPE ; i++, y+=22 )
	{
		rankScore   = info.get_rank_score(i+1, viewNationRecno);
		totalScore += rankScore;

		font_bible.put( x    , y, _(rankStrArray[i]) );
		font_bible.put( x+300, y, rankScore );
	}

	vga_front.bar( x, y, x+340, y+1, V_BLACK );
	y+=7;

	//-------- display thte total score --------//

	font_bible.put( x    , y+2, _("Total Score") );
	font_bible.put( x+300, y+2, totalScore );
	y+=28;

	vga_front.bar( x, y, x+340, y+1, V_BLACK );
	y+=4;

	//-------- display the final score ---------//

	int	 difficultyRating = config.difficulty_rating;
	int	 finalScore = totalScore * difficultyRating / 100;
	String str;

	// TRANSLATORS: Final Score:  <Number> X
	snprintf( str, MAX_STR_LEN+1, _("Final Score:  %s X "), misc.format(totalScore) );

	x = font_bible.put( x, y+20, str )+5;

	// TRANSLATORS: <Number> (Difficulty Rating)
	snprintf( str, MAX_STR_LEN+1, _("%s (Difficulty Rating)"), misc.format(difficultyRating) );

	font_bible.center_put( x, y+4, x+170, y+1+font_bible.height(), str );
	vga_front.bar( x, y+27, x+170, y+28, V_BLACK );
	font_bible.put( x+70, y+30, 100 );

	//---- if this is a scenario and there are score bonus ----//

	str = "";

	if( info.goal_score_bonus && winFlag &&
		 !nation_array[viewNationRecno]->cheat_enabled_flag )		// if cheated, don't display bonus score, as there is no space for displaying both
	{
		finalScore += info.goal_score_bonus;
		// TRANSLATORS: +  <Number> (Bonus)  =  <Number>
		snprintf( str, MAX_STR_LEN+1, _("+  %d (Bonus)  =  %s"), info.goal_score_bonus, misc.format(finalScore) );
	}

	//------- if the player has cheated -------//

	if( nation_array[viewNationRecno]->cheat_enabled_flag )
	{
		str = _("X  0 (Cheated)  =  0");
	}

	font_bible.put( x+180, y+18, str);

	return finalScore;
}
//----------- End of static function disp_score -----------//
