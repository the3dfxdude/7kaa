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

//Filename    : OR_RANK.CPP
//Description : Ranking report

#include <ODATE.h>
#include <OFONT.h>
#include <OGAME.h>
#include <OCONFIG.h>
#include <OVBROWIF.h>
#include <ONATION.h>
#include <OINFO.h>

//------------- Define coordinations -----------//

enum { NATION_BROWSE_X1 = ZOOM_X1+6,
		 NATION_BROWSE_Y1 = ZOOM_Y1+6,
		 NATION_BROWSE_X2 = ZOOM_X2-6,
		 NATION_BROWSE_Y2 = NATION_BROWSE_Y1+220,
	 };

enum { NATION_SCORE_X1 = ZOOM_X1+6,
		 NATION_SCORE_Y1 = NATION_BROWSE_Y2+6,
		 NATION_SCORE_X2 = ZOOM_X2-6,
		 NATION_SCORE_Y2 = NATION_SCORE_Y1+150,
	 };

enum { NATION_GOAL_X1 = ZOOM_X1+6,
		 NATION_GOAL_Y1 = NATION_SCORE_Y2+6,
		 NATION_GOAL_X2 = ZOOM_X2-6,
		 NATION_GOAL_Y2 = NATION_GOAL_Y1+110,
	 };

enum { PLAY_TIME_X1 = ZOOM_X1+6,
		 PLAY_TIME_X2 = ZOOM_X2-6,
	 };

//----------- Define static variables ----------//

static VBrowseIF 	browse_nation;
static int		 	nation_rank_data_array[MAX_RANK_TYPE][MAX_NATION];

//----------- Define static functions ----------//

static void  put_nation_rec(int recNo, int x, int y, int refreshFlag);
static int   nation_filter(int recNo=0);
static void  disp_score();
static void	 disp_goal();
static void	 disp_play_time(int y1);

//--------- Begin of function Info::disp_rank ---------//
//
void Info::disp_rank(int refreshFlag)
{
	set_rank_data(1);		// 1-only set those nations that have contact with us

	int x=NATION_BROWSE_X1+9;
	int y=NATION_BROWSE_Y1+3;

	vga_back.d3_panel_up(NATION_BROWSE_X1, NATION_BROWSE_Y1, NATION_BROWSE_X2, NATION_BROWSE_Y1+32 );

	font_san.put( x	 , y+7, "Kingdom" );
	font_san.put( x+180, y+7, "Population" );
	font_san.put( x+264, y+7, "Military" );
	font_san.put( x+332, y+7, "Economy" );
	font_san.put( x+406, y+7, "Reputation" );

#if(defined(SPANISH))
	font_san.put( x+484, y   , "Lucha" );
	font_san.put( x+484, y+13, "Fryhtan" );
#else
	// FRENCH, German and US
	font_san.put( x+484, y   , "Fryhtan" );
	font_san.put( x+484, y+13, "Battling" );
#endif

	if( refreshFlag == INFO_REPAINT )
	{
		browse_nation.init( NATION_BROWSE_X1, NATION_BROWSE_Y1+34, NATION_BROWSE_X2, NATION_BROWSE_Y2,
								  0, 22, nation_filter(), put_nation_rec, 1 );

		browse_nation.open(browse_nation_recno);
	}
	else
	{
		browse_nation.paint();
		browse_nation.open(browse_nation_recno, nation_filter());
	}

	//----- display score -------//

	disp_score();

	//------ display goal -------//

	if( !game.game_has_ended )	// if the ending screen has already appeared once, don't display the goal
	{
		disp_goal();
		y = NATION_GOAL_Y2+6;
	}
	else
	{
		y = NATION_GOAL_Y1;
	}

	//----- display total playing time -----//

	disp_play_time(y);
}
//----------- End of function Info::disp_rank -----------//


//--------- Begin of static function disp_score ---------//
//
static void disp_score()
{
	int x=NATION_SCORE_X1+6, y=NATION_SCORE_Y1+6;

	static const char* rankStrArray[] =
	{ "Population Score", "Military Score", "Economic Score",
	  "Reputation Score", "Fryhtan Battling Score" };

	vga.d3_panel_down( NATION_SCORE_X1, NATION_SCORE_Y1, NATION_SCORE_X2, NATION_SCORE_Y2 );

	//------ display individual scores ---------//

	int rankScore, totalScore=0;
	int viewNationRecno = nation_filter(browse_nation.recno());

	for( int i=0 ; i<MAX_RANK_TYPE ; i++, y+=16 )
	{
		rankScore   = info.get_rank_score(i+1, viewNationRecno);
		totalScore += rankScore;

		font_san.put( x    , y, rankStrArray[i] );
		font_san.put( x+300, y, rankScore, 1 );
	}

	vga_back.bar( x, y, x+340, y+1, V_BLACK );
	y+=4;

	//-------- display thte total score --------//

	font_san.put( x    , y+2, "Total Score" );
	font_san.put( x+300, y+2, totalScore, 1 );
	y+=20;

	vga_back.bar( x, y, x+340, y+1, V_BLACK );
	y+=4;

	//-------- display the final score ---------//

 	int	 difficultyRating = config.difficulty_rating;
	int	 finalScore = totalScore * difficultyRating / 100;
	String str;

	str  = translate.process("Final Score");
	str += ":  ";
	str += totalScore;
	str += " X ";

	int x2 = font_san.put( x, y+12, str ) + 5;

	str  = difficultyRating;
	str += " ";
	str += translate.process( "(Difficulty Rating)" );

	font_san.center_put( x2, y+1, x2+156, y+15, str );
	vga_back.bar( x2   , y+16, x2+156, y+17, V_BLACK );
	font_san.put( x2+65, y+19, 100 );

	//------- if the player has cheated -------//

	if( nation_array[viewNationRecno]->cheat_enabled_flag )
	{
		str  = "X  0 ";
		str += translate.process( "(Cheated)" );
		str += "  ";

		finalScore = 0;
	}
	else
		str = "";

	str += "=  ";
	str += finalScore;

	font_san.put( x2+170, y+12, str);

	y+=36;
}
//----------- End of static function disp_score -----------//


//--------- Begin of static function disp_goal ---------//
//
static void disp_goal()
{
	//----- if the ending screen has already appeared once -----//

	if( game.game_has_ended )
		return;

	//------------------------------------//

	int x=NATION_GOAL_X1+6, y=NATION_GOAL_Y1+6;

	int goalCount = 1 + config.goal_destroy_monster +
						 config.goal_population_flag +
						 config.goal_economic_score_flag +
						 config.goal_total_score_flag;

	vga.d3_panel_down( NATION_GOAL_X1, NATION_GOAL_Y1, NATION_GOAL_X2, NATION_GOAL_Y2 );

	//------------------------------------//

	String str;

	if( goalCount > 1 )
		str = "GOAL: Achieve One of the Following";
	else
		str = "GOAL: Defeat All Other Kingdoms";

	if( config.goal_year_limit_flag )
		str += " Before ";

	str = translate.process(str);

	if( config.goal_year_limit_flag )
		str += date.date_str( info.goal_deadline );

	//--------------------------------------//

	str += ".";
	font_san.put( x, y, str );

	y+=18;

	if( goalCount==1 )
		return;

	//-----------------------------------//

	str = "Defeat All Other Kingdoms.";

	font_san.put( x, y, str );
	y+=16;

	//-----------------------------------//

	if( config.goal_destroy_monster )
	{
		str = "Destroy All Fryhtans.";

		font_san.put( x, y, str );

		y+=16;
	}

	//-----------------------------------//

	if( config.goal_population_flag )
	{
		#if(defined(SPANISH))
			str  = "Alcanzar una población de ";
			str += config.goal_population;
			str += ".";
		#elif(defined(FRENCH))
			str  = "Atteindre une population de ";
			str += config.goal_population;
			str += ".";
		#elif(defined(GERMAN))
			str  = "Bevölkerungszahl von ";
			str += config.goal_population;
			str += " erreichen.";
		#else
			str  = "Achieve a Population of ";
			str += config.goal_population;
			str += ".";
		#endif

		font_san.put( x, y, str );

		y+=16;
	}

	//-----------------------------------//

	if( config.goal_economic_score_flag )
	{
		#if(defined(SPANISH))
			str  = "Alcanzar unos Puntos por Economía de ";
			str += config.goal_economic_score;
			str += ".";
		#elif(defined(FRENCH))
			str  = "Atteindre un score économique de ";
			str += config.goal_economic_score;
			str += ".";
		#elif(defined(GERMAN))
			str  = "Ökonom.-Punkte von ";
			str += config.goal_economic_score;
			str += " erreichen.";
		#else
			str  = "Achieve an Economic Score of ";
			str += config.goal_economic_score;
			str += ".";
		#endif

		font_san.put( x, y, str );

		y+=16;
	}

	//-----------------------------------//

	if( config.goal_total_score_flag )
	{
		#if(defined(SPANISH))
			str  = "Alcanzar unos Puntos Totales de ";
			str += config.goal_total_score;
			str += ".";
		#elif(defined(FRENCH))
			str  = "Atteindre un score global de ";
			str += config.goal_total_score;
			str += ".";
		#elif(defined(GERMAN))
			str  = "Gesamtpunkte von ";
			str += config.goal_total_score;
			str += " erreichen.";
		#else
			str  = "Achieve a Total Score of ";
			str += config.goal_total_score;
			str += ".";
		#endif

		font_san.put( x, y, str );

		y+=16;
	}
}
//----------- End of static function disp_goal -----------//


//--------- Begin of static function disp_play_time ---------//
//
static void disp_play_time(int y1)
{
	vga.d3_panel_down( PLAY_TIME_X1, y1, PLAY_TIME_X2, y1+24 );

	String str;

	str  = "Total Playing Time";

	str  = translate.process(str);
	str += ": ";
	str += info.play_time_str();

	font_san.put( PLAY_TIME_X1+6, y1+6, str );
}
//----------- End of static function disp_play_time -----------//


//--------- Begin of function Info::detect_rank ---------//
//
void Info::detect_rank()
{
	//------- detect nation browser ------//

	if( browse_nation.detect() )
	{
		browse_nation_recno = browse_nation.recno();
		return;
	}
}
//----------- End of function Info::detect_rank -----------//


//-------- Begin of static function nation_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of nations of this nation
//
// 2. when <int> recNo is given :
//    - return the nation recno in nation_array of the given recno.
//
static int nation_filter(int recNo)
{
	int    	i, nationCount=0;
	Nation*  viewingNation = NULL;
	
	if( nation_array.player_recno )
		viewingNation = nation_array[info.viewing_nation_recno];

	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( i==info.viewing_nation_recno ||
			 !viewingNation ||
			 viewingNation->get_relation(i)->has_contact )
		{
			nationCount++;
		}

		if( recNo && nationCount==recNo )
			return i;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return nationCount;
}
//----------- End of static function nation_filter -----------//


//-------- Begin of static function put_nation_rec --------//
//
static void put_nation_rec(int recNo, int x, int y, int refreshFlag)
{
	int	  nationRecno = nation_filter(recNo);
	Nation* nationPtr   = nation_array[nationRecno];

	x+=3;
	y+=5;

	nationPtr->disp_nation_color(x, y+4);

	font_san.put( x+20, y, nationPtr->nation_name() );

	font_san.put( x+210, y, info.get_rank_pos_str(1, nationRecno) );
	font_san.put( x+270, y, info.get_rank_pos_str(2, nationRecno) );
	font_san.put( x+352, y, info.get_rank_pos_str(3, nationRecno) );
	font_san.put( x+435, y, info.get_rank_pos_str(4, nationRecno) );
  	font_san.put( x+500, y, info.get_rank_pos_str(5, nationRecno) );
}
//----------- End of static function put_nation_rec -----------//


//-------- Begin of function Info::set_rank_data --------//
//
// <int> onlyHasContact - if this is 1, then only nations
//								  that have contact with the viewing
//								  nation is counted. Otherwise all nations
//								  are counted.
//
void Info::set_rank_data(int onlyHasContact)
{
	Nation* viewingNation = NULL; 
	Nation* nationPtr;
	int 	  rankPos=0;

	if( nation_array.player_recno && !nation_array.is_deleted(info.viewing_nation_recno) )
		viewingNation = nation_array[info.viewing_nation_recno];

	memset( nation_rank_data_array, 0, sizeof(nation_rank_data_array) );

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( onlyHasContact )
		{
			if( viewingNation && !viewingNation->get_relation(i)->has_contact )
				continue;
		}

		nationPtr = nation_array[i];

		nation_rank_data_array[0][i-1] = nationPtr->population_rating;

		nation_rank_data_array[1][i-1] = nationPtr->military_rating;

		nation_rank_data_array[2][i-1] = nationPtr->economic_rating;

		nation_rank_data_array[3][i-1] = (int) nationPtr->reputation;

		nation_rank_data_array[4][i-1] = (int) nationPtr->kill_monster_score;
	}
}
//----------- End of static function Info::set_rank_data -----------//


//-------- Begin of function Info::get_rank_pos_str --------//
//
char* Info::get_rank_pos_str(int rankType, int nationRecno)
{
	Nation* viewingNation = NULL; 
	int curNationRankData = nation_rank_data_array[rankType-1][nationRecno-1];
	int rankPos=1;

	if( nation_array.player_recno && !nation_array.is_deleted(info.viewing_nation_recno) )
		viewingNation = nation_array[info.viewing_nation_recno];

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i == nationRecno )
			continue;

		if( viewingNation && !viewingNation->get_relation(i)->has_contact )
			continue;

		if( nation_rank_data_array[rankType-1][i-1] > curNationRankData )		// if another nation's value is higher than the given nation's value
			rankPos++;
	}

	return m.num_th(rankPos);
}
//----------- End of function Info::get_rank_pos_str -----------//


//-------- Begin of function Info::get_rank_score --------//
//
// Get the score of the given nation in the given ranking type.
//
int Info::get_rank_score(int rankType, int nationRecno)
{
	int maxValue;

	switch( rankType )
	{
		case 1:     	// population
			maxValue = 100;
			break;

		case 2:        // military strength
			maxValue = 200;
			break;

		case 3:        // economic strength
			maxValue = 6000;
			break;

		case 4:        // reputation
			maxValue = 100;				// so the maximum score of the reputation portion is 50 only
			break;

		case 5: 			// monsters slain score
			maxValue = 1000;
			break;
	}

	int rankScore = 100 * nation_rank_data_array[rankType-1][nationRecno-1] / maxValue;

	return MAX(0, rankScore);
}
//----------- End of function Info::get_rank_score -----------//


//-------- Begin of function Info::get_total_score --------//
//
// Get the score of the given nation.
//
int Info::get_total_score(int nationRecno)
{
	int totalScore=0;

	for( int i=0 ; i<MAX_RANK_TYPE ; i++ )
	{
		totalScore += get_rank_score(i+1, nationRecno);
	}

	return totalScore;
}
//----------- End of function Info::get_total_score -----------//
