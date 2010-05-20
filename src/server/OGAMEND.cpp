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
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OBOX.h>
#include <OCONFIG.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <ORACERES.h>
#include <OGAME.h>
#include <OGFILE.h>
#include <ONATION.h>
#include <OMOUSECR.h>
#include <OMUSIC.h>
#include <OOPTMENU.h>
#include <OINGMENU.h>
// ####### begin Gilbert 29/10 ########//
#include <OPOWER.h>
// ####### end Gilbert 29/10 ########//



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

	//--------- set game_has_ended to 1 --------//

	music.stop();
	game_has_ended = 1;

}
//----------- End of function Game::game_end ---------//


//----------- Begin of static function disp_goal_str -----------//

static void disp_goal_str(int winNationRecno)
{
	Nation* winNation = nation_array[winNationRecno];
	String  str, str2;

	if( winNationRecno == nation_array.player_recno )
		str = translate.process("Your Kingdom");
	else
		str = nation_array[winNationRecno]->nation_name();

	str2 = "";

	//---- if the player has achieved one of its goals ----//

	if( winNation->goal_destroy_monster_achieved() )
	{
		str2  = str;
		str   = translate.process("All Fryhtans have been Destroyed !");
		str2 += translate.process(" has Achieved the Highest Fryhtan Battling Score !");
	}

	//-----------------------------------//

	else if( winNation->goal_population_achieved() )
	{
		str  += translate.process( " has Reached" );

		str2  = translate.process( "its Population Goal of " );
		str2 += config.goal_population;
		str2 += " !";
	}

	//-----------------------------------//

	else if( winNation->goal_economic_score_achieved() )
	{
		str  += translate.process( " has Reached" );

		str2  = translate.process( "its Economic Score Goal of " );
		str2 += config.goal_economic_score;
		str2 += " !";
	}

	//-----------------------------------//

	else if( winNation->goal_total_score_achieved() )
	{
		str  += translate.process( " has Reached" );

		str2  = translate.process( "its Total Score Goal of " );
		str2 += config.goal_total_score;
		str2 += " !";
	}

	//-----------------------------------//

	else			// ( winNation->goal_destroy_nation_achieved() )
	{
		str += translate.process( " has Defeated All Other Kingdoms !" );
	}

	//-----------------------------------//

	int y=40;

	if( winNationRecno != nation_array.player_recno )
	{
		font_bible.center_put(0, 30, VGA_WIDTH-1, 60, "You Have Lost the Game !" );
		y=60;
	}

	font_bible.center_put(0, y   , VGA_WIDTH-1, y+30, str  );
	font_bible.center_put(0, y+30, VGA_WIDTH-1, y+60, str2 );
}
//----------- End of static function disp_goal_str -----------//


//----------- Begin of static function disp_losing_str -----------//

static void disp_losing_str(int surrenderToNationRecno)
{
	String str;

	if( surrenderToNationRecno )		// you surrender to another kingdom
	{
		str  = translate.process( "You Surrendered to " );
		str += nation_array[surrenderToNationRecno]->nation_name();
		str += " ";
		str += translate.process( "on ");
		str += date.date_str(info.game_date);
		str += ".";
	}

	// You failed to achieve the goal within the time limit

	else if( config.goal_year_limit_flag && info.game_date >= info.goal_deadline )
	{
		str = "Your Kingdom has Failed to Achieve its Goal Within the Time Limit.";
	}
	else		// you're defeated by another kingdom
	{
		str = "Your Kingdom has Gone Down to Ignominious Defeat !";
	}

	font_bible.center_put(0, 0, VGA_WIDTH-1, 139, str );
}
//----------- End of static function disp_losing_str -----------//


//----------- Begin of static function disp_retire_str -----------//

static void disp_retire_str()
{
	String str;

#if(defined(SPANISH))
	str  = "Te has retirado el ";
	str += date.date_str( info.game_date );
	str += ".";
#elif(defined(FRENCH))
	str  = "Vous avez abandonné le ";
	str += date.date_str( info.game_date );
	str += ".";
#elif(defined(GERMAN))
	str  = "Sie haben am ";
	str += date.date_str( info.game_date );
	str += " aufgegeben.";
#else
	str  = "You Retired on ";
	str += date.date_str( info.game_date );
	str += ".";
#endif

	font_bible.center_put(0, 0, VGA_WIDTH-1, 139, str );
}
//----------- End of static function disp_retire_str -----------//


//-------- Begin of static function disp_stat --------//
//
static void disp_stat()
{
	int y=140;

	Nation* nationPtr = ~nation_array;

	put_stat( y		, "Duration of Your Rule", info.game_duration_str() );
	put_stat( y+=20, "Total Gaming Time", info.play_time_str() );

	put_stat( y+=30, "Final Population", nationPtr->all_population() );
	put_stat( y+=20, "Final Treasure"  , m.format((int)nationPtr->cash,2) );

	put_stat( y+=30, "Enemy Soldiers Dispatched", nationPtr->enemy_soldier_killed );
	put_stat( y+=20, "King's Soldiers Martyred"  , nationPtr->own_soldier_killed );

	put_stat( y+=30, "Enemy Weapons Destroyed"		  , nationPtr->enemy_weapon_destroyed );
	put_stat( y+=20, "King's Weapons Rendered Obsolete", nationPtr->own_weapon_destroyed );

	put_stat( y+=30, "Enemy Ships Sunk"   , nationPtr->enemy_ship_destroyed );
	put_stat( y+=20, "King's Ships Missing", nationPtr->own_ship_destroyed );

	put_stat( y+=30, "Enemy Buildings Destroyed"   , nationPtr->enemy_firm_destroyed );
	put_stat( y+=20, "King's Buildings Cleared", nationPtr->own_firm_destroyed );

	put_stat( y+=30, "Enemy Civilians Collaterally Damaged", nationPtr->enemy_civilian_killed );
	put_stat( y+=20, "King's Civilians Cruelly Murdered"    , nationPtr->own_civilian_killed );
}
//----------- End of static function disp_stat -----------//


//-------- Begin of static function put_stat --------//
//
static void put_stat(int y, const char* desStr, const char* dispStr)
{
	font_bible.put( 140, y, desStr );
	font_bible.put( 570, y, dispStr );
}
//----------- End of static function put_stat -----------//


//-------- Begin of static function put_stat --------//
//
static void put_stat(int y, const char* desStr, int dispValue)
{
	font_bible.put( 140, y, desStr );
	font_bible.put( 570, y, m.format(dispValue) );
}
//----------- End of static function put_stat -----------//


//-------- Begin of static function disp_ranking --------//
//
static void disp_ranking()
{
	//--------- display descriptions ---------//

	int x=20, y=76;

	font_bible.put( x+20 , y+7, "Kingdom" );
	font_bible.put( x+260, y+7, "Population" );
	font_bible.put( x+370, y+7, "Military" );
	font_bible.put( x+470, y+7, "Economy" );
	font_bible.put( x+562, y+7, "Reputation" );

#if(defined(SPANISH))
	font_bible.put( x+670, y   , "Lucha" );
	font_bible.put( x+670, y+14, "Fryhtan" );
#else
	font_bible.put( x+670, y   , "Fryhtan" );
	font_bible.put( x+670, y+14, "Battling" );
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

	int x=20;

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
	int x=200, y=360;

	static const char* rankStrArray[] =
	{ "Population Score", "Military Score", "Economic Score",
	  "Reputation Score", "Fryhtan Battling Score" };

	//------ display individual scores ---------//

	int rankScore, totalScore=0;
	int viewNationRecno = nation_array.player_recno;

	for( int i=0 ; i<MAX_RANK_TYPE ; i++, y+=22 )
	{
		rankScore   = info.get_rank_score(i+1, viewNationRecno);
		totalScore += rankScore;

		font_bible.put( x    , y, rankStrArray[i] );
		font_bible.put( x+300, y, rankScore );
	}

	vga_front.bar( x, y, x+340, y+1, V_BLACK );
	y+=7;

	//-------- display thte total score --------//

	font_bible.put( x    , y+2, "Total Score" );
	font_bible.put( x+300, y+2, totalScore );
	y+=28;

	vga_front.bar( x, y, x+340, y+1, V_BLACK );
	y+=4;

	//-------- display the final score ---------//

	int	 difficultyRating = config.difficulty_rating;
	int	 finalScore = totalScore * difficultyRating / 100;
	String str;

	str  = translate.process("Final Score");
	str += ":  ";
	str += totalScore;
	str += " X ";

	x = font_bible.put( x, y+20, str )+5;

	str  = difficultyRating;
	str += " ";
	str += translate.process( "(Difficulty Rating)" );

	font_bible.center_put( x, y+4, x+170, y+1+font_bible.height(), str );
	vga_front.bar( x, y+27, x+170, y+28, V_BLACK );
	font_bible.put( x+70, y+30, 100 );

	//---- if this is a scenario and there are score bonus ----//

	str = "";

	if( info.goal_score_bonus && winFlag &&
		 !nation_array[viewNationRecno]->cheat_enabled_flag )		// if cheated, don't display bonus score, as there is no space for displaying both
	{
		str += "+  ";
		str += info.goal_score_bonus;
		str += " (Bonus)  ";
		finalScore += info.goal_score_bonus;
	}

	//------- if the player has cheated -------//

	if( nation_array[viewNationRecno]->cheat_enabled_flag )
	{
		str  = "X  0 ";
		str += translate.process( "(Cheated)" );
		str += "  ";

		finalScore = 0;
	}

	str += "=  ";
	str += finalScore;

	font_bible.put( x+180, y+18, str);

	return finalScore;
}
//----------- End of static function disp_score -----------//
