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

//Filename    : OSPY.CPP
//Description : Object Spy

#include <OPOWER.h>
#include <OGAME.h>
#include <ODATE.h>
#include <ONEWS.h>
#include <OFONT.h>
#include <OUNIT.h>
#include <OWORLD.h>
#include <OBUTTON.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <ONATION.h>
#include <ORACERES.h>
#include <OSYS.h>
#include <OSPY.h>

//----- Define constants for viewing secret menu ------//

#define SECRET_REPORT_COUNT 7

static const char* 	secret_report_str_array[] = { "Kingdoms", "Villages", "Economy", "Trade", "Military", "Technology", "Espionage" };
static char	 	secret_view_mode_array[]  = { MODE_NATION, MODE_TOWN, MODE_ECONOMY, MODE_TRADE, MODE_MILITARY, MODE_TECH, MODE_SPY };
static char	 	secret_view_skill_array[] = { 40, 20, 30, 30, 50, 40, 90 };

static Button	button_secret_report_array[SECRET_REPORT_COUNT];
static Button	button_secret_report_cancel;


//--------- Begin of function SpyArray::SpyArray ----------//

SpyArray::SpyArray() : DynArrayB(sizeof(Spy), 10)
{
}
//--------- End of function SpyArray::SpyArary ----------//


//------- Begin of function SpyArray::~SpyArray ----------//
//
SpyArray::~SpyArray()
{
	deinit();
}
//--------- End of function SpyArray::~SpyArray ----------//


//--------- Begin of function SpyArray::init ----------//
//
void SpyArray::init()
{
}
//---------- End of function SpyArray::init ----------//


//--------- Begin of function SpyArray::deinit ----------//
//
void SpyArray::deinit()
{
	if( size()==0 )
		return;

	//-------- zap the array -----------//

	zap();
}
//---------- End of function SpyArray::deinit ----------//


//--------- Begin of function SpyArray::add_spy ----------//
//
// <int> unitRecno - unit recno of the spy
// <int> spySkill  - spying skill of the unit
//
// return: <int> recno of the spy record added
//
int SpyArray::add_spy(int unitRecno, int spySkill)
{
	Spy 	spy;
	Unit* unitPtr = unit_array[unitRecno];

	memset( &spy, 0, sizeof(spy) );

	spy.spy_place 			= SPY_MOBILE;
	spy.spy_place_para	= unitRecno;
	spy.spy_skill 			= spySkill;
	spy.spy_loyalty 		= unitPtr->loyalty;
	spy.race_id			   = unitPtr->race_id;
	spy.name_id				= unitPtr->name_id;

	err_when( unitPtr->race_id < 1 || unitPtr->race_id > MAX_RACE );

	err_when( nation_array.is_deleted(unitPtr->nation_recno) );

	spy.true_nation_recno    = unitPtr->nation_recno;
	spy.cloaked_nation_recno = unitPtr->nation_recno;

	//--- spies hold a use right of the name id even though the unit itself will register the usage right of the name already ---//

	race_res[spy.race_id]->use_name_id(spy.name_id);		// the spy will free it up in deinit(). Keep an additional right because when a spy is assigned to a town, the normal program will free up the name id., so we have to keep an additional copy

	//------- link in the spy_array -------//

	linkin( &spy );

	((Spy*)get())->spy_recno = recno();

	return recno();
}
//---------- End of function SpyArray::add_spy ----------//


//--------- Begin of function SpyArray::add_spy ----------//
//
// This overloaded version of add_spy() just add a spy without
// setting parameters of the Spy.
//
// return: <int> recno of the spy record added
//
int SpyArray::add_spy()
{
	Spy spy;

	memset( &spy, 0, sizeof(spy) );

	linkin( &spy );

	((Spy*)get())->spy_recno = recno();

	return recno();
}
//---------- End of function SpyArray::add_spy ----------//


//--------- Begin of function SpyArray::del_spy ----------//
//
// <int> spyRecno - recno of the spy to be deleted
//
void SpyArray::del_spy(int spyRecno)
{
	spy_array[spyRecno]->deinit();

	linkout(spyRecno);
}
//---------- End of function SpyArray::del_spy ----------//


//--------- Begin of function SpyArray::next_day ----------//
//
void SpyArray::next_day()
{
	int  spyCount = size();
	Spy* spyPtr;

	for( int i=1 ; i<=spyCount ; i++ )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		spyPtr->next_day();

		if( spy_array.is_deleted(i) )
			continue;

		if( nation_array[spyPtr->true_nation_recno]->is_ai() )
			spyPtr->process_ai();
	}

	//---------- update Firm::sabotage_level ----------//

	if( info.game_date%15==0 )
		process_sabotage();
}
//---------- End of function SpyArray::next_day ----------//


//--------- Begin of function SpyArray::find_town_spy ----------//
//
// Find a spy meeting the specific criteria
//
// <int> townRecno - town recno of the spy to find
// <int> raceId    - race id. of the spy to find
// <int> spySeq	 - sequence id. of the spy in spy_array
//
// return: <int> recno of the spy found
//
int SpyArray::find_town_spy(int townRecno, int raceId, int spySeq)
{
	int  spyCount=size(), matchCount=0;
	Spy* spyPtr;

	for( int i=1 ; i<=spyCount ; i++ )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place==SPY_TOWN &&
			 spyPtr->spy_place_para==townRecno &&
			 spyPtr->race_id==raceId )
		{

			if( ++matchCount == spySeq )
				return i;
		}
	}

	return 0;
}
//---------- End of function SpyArray::find_town_spy ----------//


//--------- Begin of function SpyArray::process_sabotage ----------//
//
void SpyArray::process_sabotage()
{
	Spy*  spyPtr;
	Firm* firmPtr;

	//-------- reset firms' sabotage_level -------//

	int i;
	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firm_array[i]->sabotage_level = 0;
	}

	//------- increase firms' sabotage_level -----//

	for( i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->action_mode == SPY_SABOTAGE )
		{
			err_when( spyPtr->spy_place != SPY_FIRM );

			firmPtr = firm_array[spyPtr->spy_place_para];

			firmPtr->sabotage_level += spyPtr->spy_skill/5;

			if( firmPtr->sabotage_level > 100 )
				firmPtr->sabotage_level = 100;
		}
	}
}
//---------- End of function SpyArray::process_sabotage ----------//


//--------- Begin of function SpyArray::mobilize_all_spy ----------//
//
// Mobilize all spies of the specific nation in the specific place.
//
// <int> spyPlace     - place id. of the spy
// <int> spyPlacePara - town or firm recno of the spy's staying
// <int> nationRecno	 - recno of the nation which the spy should
//							   be mobilized.
//
void SpyArray::mobilize_all_spy(int spyPlace, int spyPlacePara, int nationRecno)
{
	Spy* spyPtr;

	for( int i=size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place == spyPlace &&
			 spyPtr->spy_place_para == spyPlacePara &&
			 spyPtr->true_nation_recno == nationRecno )
		{
			if( spyPtr->spy_place == SPY_TOWN )
				spyPtr->mobilize_town_spy();

			else if( spyPtr->spy_place == SPY_FIRM )
				spyPtr->mobilize_firm_spy();
		}
	}
}
//---------- End of function SpyArray::mobilize_all_spy ----------//


//--------- Begin of function SpyArray::disp_view_secret_menu ---------//
//
void SpyArray::disp_view_secret_menu(int spyRecno, int refreshFlag)
{
	if( refreshFlag != INFO_REPAINT )
		return;

	//------------------------------------//

	vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+42 );

	font_san.put_paragraph( INFO_X1+7, INFO_Y1+5, INFO_X2-7, INFO_Y2-5,
									"Steal which type of secrets?" );

	//------------------------------------//

	int  y=INFO_Y1+45;

	err_when( spy_array.is_deleted(spyRecno) );

	Spy* spyPtr = spy_array[spyRecno];

	for( int i=0 ; i<SECRET_REPORT_COUNT ; i++ )
	{
		if( spyPtr->spy_skill >= secret_view_skill_array[i] )
		{
			button_secret_report_array[i].paint_text( INFO_X1, y, INFO_X2, y+21, secret_report_str_array[i] );
			y+=23;
		}
		else
		{
			button_secret_report_array[i].reset();
		}
	}

	button_secret_report_cancel.paint_text( INFO_X1, y, INFO_X2, y+22, "Cancel" );
}
//----------- End of function SpyArray::disp_view_secret_menu -----------//


//--------- Begin of function SpyArray::detect_view_secret_menu ---------//
//
// <int> spyRecno    - recno of the spy to view secret info.
// <int> nationRecno - recno of the nation which this spy is going to investigate
//
int SpyArray::detect_view_secret_menu(int spyRecno, int nationRecno)
{
	//---- detect secret report button ----//

	int rc=0;

	for( int i=0 ; i<SECRET_REPORT_COUNT ; i++ )
	{
		if( button_secret_report_array[i].detect() )
		{
			sys.set_view_mode( secret_view_mode_array[i], nationRecno, spyRecno );
			rc=1;
			break;
		}
	}

	//-------- detect cancel button --------//

	if( button_secret_report_cancel.detect() )
		rc = 1;

	return rc;
}
//----------- End of function SpyArray::detect_view_secret_menu -----------//


//--------- Begin of function SpyArray::update_firm_spy_count ---------//
//
// Update the player_spy_count of this firm. This function is called
// when the firm change its nation.
//
// <int> firmRecno - recno of the firm to be updated.
//
void SpyArray::update_firm_spy_count(int firmRecno)
{
	//---- recalculate Firm::player_spy_count -----//

	Spy*  spyPtr;
	Firm* firmPtr = firm_array[firmRecno];

	firmPtr->player_spy_count = 0;

	for( int i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place == SPY_FIRM &&
			 spyPtr->spy_place_para == firmRecno &&
			 spyPtr->true_nation_recno == nation_array.player_recno )
		{
			firmPtr->player_spy_count++;
		}
	}
}
//----------- End of function SpyArray::update_firm_spy_count -----------//


//--------- Begin of function SpyArray::change_cloaked_nation ---------//
//
// Change the cloak of all the spies in the specific place.
//
// This function is called when a firm or town change nation.
//
// <int> spyPlace        - spy place
// <int> spyPlacePara    - spy place para
// <int> fromNationRecno - change any spies in the place whose cloaked_nation_recno
// <int> toNationRecno     is fromNationRecno to toNationRecno.
//
void SpyArray::change_cloaked_nation(int spyPlace, int spyPlacePara, int fromNationRecno, int toNationRecno)
{
	Spy* spyPtr;

	for( int i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->cloaked_nation_recno != fromNationRecno )
			continue;

		if( spyPtr->spy_place != spyPlace )
			continue;

		//--- check if the spy is in the specific firm or town ---//

		if( spyPlace == SPY_FIRM || spyPlace == SPY_TOWN )  // only check spy_place_para when spyPlace is SPY_TOWN or SPY_FIRM
		{
			if( spyPtr->spy_place_para != spyPlacePara )
				continue;
		}

		if(spyPlace==spyPtr->spy_place && spyPlacePara==spyPtr->spy_place_para &&
			spyPtr->true_nation_recno==toNationRecno)
			spyPtr->set_action_mode(SPY_IDLE);

		//----- if the spy is associated with a unit (mobile or firm overseer), we call Unit::spy_chnage_nation() ---//

		if( spyPlace == SPY_FIRM )
		{
			int firmOverseerRecno = firm_array[spyPtr->spy_place_para]->overseer_recno;

			if( firmOverseerRecno && unit_array[firmOverseerRecno]->spy_recno == i )
			{
				unit_array[firmOverseerRecno]->spy_change_nation(toNationRecno, COMMAND_AUTO);
				continue;
			}
		}
		else if( spyPlace == SPY_MOBILE )
		{
			unit_array[spyPtr->spy_place_para]->spy_change_nation(toNationRecno, COMMAND_AUTO);
			continue;
		}

		//---- otherwise, just change the spy cloak ----//

		spyPtr->cloaked_nation_recno = toNationRecno;
	}
}
//----------- End of function SpyArray::change_cloaked_nation -----------//


//--------- Begin of function SpyArray::total_spy_skill_level ---------//
//
// Calculate the combined skill levels of all the spies of the
// specific nation in the specific place.
//
// <int> spyPlace        - spy place
// <int> spyPlacePara    - spy place para
// <int> spyNationRecno  - nation recno
// <int&> spyCount		 - the total no. of spies meeting the criteria.
//
int SpyArray::total_spy_skill_level(int spyPlace, int spyPlacePara, int spyNationRecno, int& spyCount)
{
	int  totalSpyLevel=0;
	Spy* spyPtr;

	spyCount = 0;

	for( int i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->true_nation_recno != spyNationRecno )
			continue;

		if( spyPtr->spy_place != spyPlace )
			continue;

		if( spyPtr->spy_place_para != spyPlacePara )
			continue;

		spyCount++;

		totalSpyLevel += spyPtr->spy_skill;
	}

	return totalSpyLevel;
}
//----------- End of function SpyArray::total_spy_skill_level -----------//


//-------- Begin of function SpyArray::catch_spy ------//
//
// <int> spyPlace - either SPY_TOWN or SPY_FIRM
// <int> spyPlacePara - town_recno or firm_recno
//
int SpyArray::catch_spy(int spyPlace, int spyPlacePara)
{
	int nationRecno, totalPop;

	if( spyPlace == SPY_TOWN )
	{
		Town* townPtr = town_array[spyPlacePara];

		nationRecno = townPtr->nation_recno;
		totalPop    = townPtr->population;
	}
	else if( spyPlace == SPY_FIRM )
	{
		Firm* firmPtr = firm_array[spyPlacePara];

		nationRecno = firmPtr->nation_recno;
		totalPop    = firmPtr->worker_count + (firmPtr->overseer_recno>0);
	}
	else
		err_here();

	//--- calculate the total of anti-spy skill in this town ----//

	int enemySpyCount=0, counterSpySkill=0;
	Spy* spyPtr;

	int i;
	for( i=size() ; i>0 ; i-- )
	{
		if( is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place == spyPlace &&
			 spyPtr->spy_place_para == spyPlacePara )
		{
			if( spyPtr->true_nation_recno == nationRecno )
				counterSpySkill += spyPtr->spy_skill;
			else
				enemySpyCount++;
		}
	}

	//----- if all villagers are enemy spies ----//

	if( enemySpyCount == totalPop )
		return 0;

	err_when( enemySpyCount > totalPop );

	//-------- try to catch enemy spies now ------//

	for( i=spy_array.size() ; i>0 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->action_mode == SPY_IDLE )		// it is very hard to get caught in sleep mode
			continue;

		if( spyPtr->spy_place == spyPlace &&
			 spyPtr->spy_place_para == spyPlacePara &&
			 spyPtr->true_nation_recno != nationRecno )	// doesn't get caught in sleep mode
		{
			int escapeChance = 100 + spyPtr->spy_skill - counterSpySkill;

			escapeChance = MAX( spyPtr->spy_skill/10, escapeChance );

			if( m.random(escapeChance) == 0 )
			{
				spyPtr->get_killed(); 		// only catch one spy per calling
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function SpyArray::catch_spy --------//


//--------- Begin of function SpyArray::set_action_mode ----------//
//
// Set all spies in the given place to the specific action mode.
//
void SpyArray::set_action_mode(int spyPlace, int spyPlacePara, int actionMode)
{
	int  spyCount=size();
	Spy* spyPtr;

	for( int i=1 ; i<=spyCount ; i++ )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place==spyPlace &&
			 spyPtr->spy_place_para==spyPlacePara )
		{
			spyPtr->set_action_mode(actionMode);
		}
	}
}
//---------- End of function SpyArray::set_action_mode ----------//


//--------- Begin of function SpyArray::ai_spy_town_rebel ----------//
//
// Tell the AI spies in the town that a rebellion is happening.
//
// When a rebellion happens, all the AI spies in the village will mobilize
// and turn its cloak back to a nation that is not at war with the enemy
// (and notification flag should be off.) and move to a safe place
// (near to one of your towns). Then the spy reaches thedestination, it will
// become idle and then the AI processing function on idle spy will be
// called and handle the spy.
//
// <int> townRecno - recno of the town with rebellion happening.
//
void SpyArray::ai_spy_town_rebel(int townRecno)
{
	int  spyCount=size();
	Spy* spyPtr;

	for( int i=1 ; i<=spyCount ; i++ )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place==SPY_TOWN &&
			 spyPtr->spy_place_para==townRecno &&
			 nation_array[spyPtr->true_nation_recno]->is_ai() )
		{
			//-------- mobilize the spy ----------//

			int unitRecno = spyPtr->mobilize_town_spy();

			//----- think new action for the spy ------//

			if( unitRecno )
				spyPtr->think_mobile_spy_new_action();
		}
	}
}
//---------- End of function SpyArray::ai_spy_town_rebel ----------//


//--------- Begin of function SpyArray::needed_view_secret_skill ----------//
//
int SpyArray::needed_view_secret_skill(int viewMode)
{
	for( int i=0 ; i<SECRET_REPORT_COUNT ; i++ )
	{
		if( secret_view_mode_array[i] == viewMode )
			return secret_view_skill_array[i];
	}

	return 0;
}
//---------- End of function SpyArray::needed_view_secret_skill ----------//


#ifdef DEBUG

//------- Begin of function SpyArray::operator[] -----//

Spy* SpyArray::operator[](int recNo)
{
	Spy* spyPtr = (Spy*) get(recNo);

	if( !spyPtr || spyPtr->spy_recno==0 )
		err.run( "SpyArray[] is deleted" );

	return spyPtr;
}

//--------- End of function SpyArray::operator[] ----//

#endif
