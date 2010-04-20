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

//Filename    : OFIRMIF3.CPP
//Description : Firm interface functions - part 3

#include <OVGA.h>
#include <OFONT.h>
#include <OINFO.h>
#include <OSYS.h>
#include <OBUTT3D.h>
#include <OMOUSE.h>
#include <ONEWS.h>
#include <OUNIT.h>
#include <ORACERES.h>
#include <ONATION.h>
#include <OSPY.h>
#include <OFIRM.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif

//---------- Define constant ------------//

#define BRIBE_OPTION_HEIGHT 	19

//-------- Define bribe amounts ---------//

#define BRIBE_AMOUNT_COUNT  6
#define MAX_BRIBE_AMOUNT	 4000

static short bribe_amount_array[] = { 500, 1000, 1500, 2000, 3000, 4000 };

//-------- Define static vars ---------//

static Button3D button_cancel;

//------ declare static functions -------//

static void disp_bribe_button(int y, int bribeAmount, int buttonUp);


//--------- Begin of function Firm::disp_bribe_menu ---------//
//
void Firm::disp_bribe_menu(int refreshFlag)
{
	//---- if the briber or bribe target is no longer valid -----//

	if( bribe_result == BRIBE_NONE )
	{
		if( !validate_cur_bribe() )
		{
			firm_menu_mode = FIRM_MENU_MAIN;
			bribe_result   = BRIBE_NONE;
			info.disp();
			return;
		}
	}

	//------------------------------------//

	if( refreshFlag != INFO_REPAINT )
		return;

	//------ display the bribe menu ------//

	if( bribe_result == BRIBE_NONE )
	{
		int y=INFO_Y1;

		font_san.d3_put( INFO_X1, y, INFO_X2, y+19, "Bribe" );
		y+=22;

		disp_bribe_unit( y );
		y+=49;

		for( int i=0 ; i<BRIBE_AMOUNT_COUNT ; i++ )
		{
			disp_bribe_button( y, bribe_amount_array[i], 1);

			err_when( bribe_amount_array[i] > MAX_BRIBE_AMOUNT );

			y += BRIBE_OPTION_HEIGHT+2;
		}

		disp_bribe_button( y, 0, 1);
	}

	//------ display the bribe result -----//

	else
	{
		int x=INFO_X1+4, y=INFO_Y1+4, y2=y+font_san.height()-1;

		if( bribe_result == BRIBE_SUCCEED )
		{
			vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+24 );

			font_san.center_put( INFO_X1, y, INFO_X2, y2, "Bribing Succeeded." );
		}
		else
		{
			vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+62 );

			font_san.center_put( INFO_X1, y	  , INFO_X2, y2, "Bribing Failed." );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Your Spy Was Caught" );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "And Executed." );
		}

		y+=26;
		button_cancel.paint( INFO_X1, y, 'A', "CONTINUE" );
	}
}
//----------- End of function Firm::disp_bribe_menu -----------//


//--------- Begin of function Firm::detect_bribe_menu ---------//
//
void Firm::detect_bribe_menu()
{
	//----- if it is display the bribe result right now -----//

	if( bribe_result != BRIBE_NONE )
	{
		if( button_cancel.detect() )
		{
			firm_menu_mode = FIRM_MENU_MAIN;
			bribe_result   = BRIBE_NONE;
			info.disp();
		}

		return;
	}

	//-------------------------------------------//

	int i, y=INFO_Y1+22+49;

	for( i=0 ; i<BRIBE_AMOUNT_COUNT ; i++ )
	{
		if( mouse.single_click(INFO_X1, y, INFO_X2, y+BRIBE_OPTION_HEIGHT-1) )
		{
			disp_bribe_button( y, bribe_amount_array[i], 0);		// 0-display pressed button

			while( mouse.left_press )
			{
				sys.yield();
				mouse.get_event();
			}

			//--------- bribe now ---------//

			// ####### begin Gilbert 13/10 #######//
#ifdef USE_DPLAY
			if( !remote.is_enable() )
			{
#endif
				spy_bribe(bribe_amount_array[i], action_spy_recno, selected_worker_id);
				action_spy_recno = 0;
#ifdef USE_DPLAY
			}
			else
			{
				// packet structure <firm recno> <spy recno> <bribe target : worker (0=overseer)> <amount>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_BRIBE, 4*sizeof(short));
				*shortPtr = firm_recno;
				shortPtr[1] = action_spy_recno;
				shortPtr[2] = selected_worker_id;
				shortPtr[3] = bribe_amount_array[i];
			}
#endif
			// ####### end Gilbert 13/10 #######//
		}

		y += BRIBE_OPTION_HEIGHT+2;
	}

	//------ detect the cancel button --------//

	if( mouse.single_click(INFO_X1, y, INFO_X2, y+BRIBE_OPTION_HEIGHT-1) )
	{
		disp_bribe_button( y, 0, 0);		// 0-display pressed button

		while( mouse.left_press )
		{
			sys.yield();
			mouse.get_event();
		}

		firm_menu_mode = FIRM_MENU_MAIN;
		info.disp();
	}
}
//----------- End of function Firm::detect_bribe_menu -----------//


//--------- Begin of function Firm::validate_cur_bribe ---------//
//
// Whether the current bribe action is still valid.
//
int Firm::validate_cur_bribe()
{
	if( spy_array.is_deleted(action_spy_recno) ||
		 spy_array[action_spy_recno]->true_nation_recno != nation_array.player_recno )
	{
		return 0;
	}

	return can_spy_bribe( selected_worker_id, spy_array[action_spy_recno]->true_nation_recno );
}
//----------- End of function Firm::validate_cur_bribe -----------//


//-------- Begin of static function disp_bribe_button --------//
//
static void disp_bribe_button(int y, int bribeAmount, int buttonUp)
{
	if( buttonUp )
		vga.d3_panel_up( INFO_X1, y, INFO_X2, y+BRIBE_OPTION_HEIGHT-1 );
	else
		vga.d3_panel_down( INFO_X1, y, INFO_X2, y+BRIBE_OPTION_HEIGHT-1 );

	//--------- if display cancel button ---------//

	if( bribeAmount==0 )
		font_san.center_put( INFO_X1, y, INFO_X2, y+BRIBE_OPTION_HEIGHT-1, "Cancel" );
	else
	{
		String str;

		#ifdef GERMAN
			str  = m.format(bribeAmount,2);
			str += " anbieten";
		#else
			str  = translate.process("Offer ");
			str += m.format(bribeAmount,2);
		#endif

		font_san.center_put( INFO_X1, y, INFO_X2, y+BRIBE_OPTION_HEIGHT-1, str );
	}
}
//--------- End of static function disp_bribe_button ---------//


//--------- Begin of function Firm::can_spy_bribe ---------//
//
// <int> bribeWorkerId - worker id. in this firm to bribe
//								 0 - if bribe an overseer.
//
// <int> briberNationRecno - the nation recno of the briber.
//
int Firm::can_spy_bribe(int bribeWorkerId, int briberNationRecno)
{
	int canBribe=0;
	int spyRecno;

	err_when( bribeWorkerId < 0 || bribeWorkerId > MAX_WORKER );

	if( bribeWorkerId )		// the overseer is selected
		spyRecno = worker_array[bribeWorkerId-1].spy_recno;
	else
		spyRecno = unit_array[overseer_recno]->spy_recno;

	if( spyRecno )
	{
		canBribe = spy_array[spyRecno]->true_nation_recno != briberNationRecno; 		// only when the unit is not yet a spy of the player. Still display the bribe button when it's a spy of another nation
	}
	else
	{
		if( bribeWorkerId )
			canBribe = worker_array[bribeWorkerId-1].race_id>0;		// cannot bribe if it's a weapon
		else
			canBribe = unit_array[overseer_recno]->rank_id != RANK_KING;		// cannot bribe a king
	}

	return canBribe;
}
//----------- End of function Firm::can_spy_bribe -----------//


//--------- Begin of function Firm::spy_bribe ---------//
//
// The money the spy offers to bribe the unit.
//
// <int>   bribeAmount	  - the amount offered
// <short> birberSpyRecno - spy recno of the briber
// <short> workerId		  - if 0, then bribe the overseer,
//									 if >0, then bribe a worker.
//
// return: <int> >0 - bribing succeeded, return the spy recno of the bribed unit (as it has been turned into a spy)
//					  0 - bribing failed
//
int Firm::spy_bribe(int bribeAmount, short briberSpyRecno, short workerId)
{
	if( !can_spy_bribe(workerId, spy_array[briberSpyRecno]->true_nation_recno) )		// this can happen in multiplayer as there is a one frame delay when the message is sent and when it is processed
		return 0;

	//---------------------------------------//

	int succeedChance = spy_bribe_succeed_chance(bribeAmount, briberSpyRecno, workerId);

	Spy* spyPtr = spy_array[briberSpyRecno];

	nation_array[spyPtr->true_nation_recno]->add_expense( EXPENSE_BRIBE, (float) bribeAmount, 0 );

	//------ if the bribe succeeds ------//

	if( succeedChance > 0 && m.random(100) < succeedChance )
	{
		int spyRecno = spy_array.add_spy();		// add a new Spy record

		Spy* newSpy = spy_array[spyRecno];

		newSpy->spy_skill = 10;
		newSpy->action_mode = SPY_IDLE;
		newSpy->spy_loyalty = MIN( 100, MAX(30,succeedChance) );		// within the 30-100 range

		newSpy->true_nation_recno    = spyPtr->true_nation_recno;
		newSpy->cloaked_nation_recno = spyPtr->cloaked_nation_recno;

		if( workerId )
		{
			Worker* workerPtr = worker_array+workerId-1;

			workerPtr->spy_recno = spyRecno;
			newSpy->race_id = workerPtr->race_id;
			newSpy->name_id = workerPtr->name_id;

			err_when( newSpy->race_id < 1 || newSpy->race_id > MAX_RACE );

			if( !newSpy->name_id )		// if this worker does not have a name, give him one now as a spy must reserve a name (see below on use_name_id() for reasons)
				newSpy->name_id = race_res[newSpy->race_id]->get_new_name_id();
		}
		else if( overseer_recno )
		{
			Unit* unitPtr = unit_array[overseer_recno];

			unitPtr->spy_recno = spyRecno;
			newSpy->race_id = unitPtr->race_id;
			newSpy->name_id = unitPtr->name_id;

			err_when( newSpy->race_id < 1 || newSpy->race_id > MAX_RACE );
		}
		else
			err_here();

		newSpy->set_place( SPY_FIRM, firm_recno );

		//-- Spy always registers its name twice as his name will be freed up in deinit(). Keep an additional right because when a spy is assigned to a town, the normal program will free up the name id., so we have to keep an additional copy

		race_res[newSpy->race_id]->use_name_id(newSpy->name_id);

		bribe_result = BRIBE_SUCCEED;

		if( firm_recno == firm_array.selected_recno )
			info.disp();

		return newSpy->spy_recno;
	}
	else //------- if the bribe fails --------//
	{
		spyPtr->get_killed(0);		// the spy gets killed when the action failed.
											// 0 - don't display new message for the spy being killed, so we already display the msg on the interface
		bribe_result = BRIBE_FAIL;

		if( firm_recno == firm_array.selected_recno )
			info.disp();

		return 0;
	}
}
//----------- End of function Firm::spy_bribe -----------//


//--------- Begin of function Firm::spy_birbe_succeed_chance ---------//
//
// The money the spy offers to bribe the unit.
//
// <int>   bribeAmount	  - the amount offered
// <short> birberSpyRecno - spy recno of the briber
// <short> workerId		  - if 0, then bribe the overseer,
//									 if >0, then bribe a worker.
//
// return: <int> 1 - bribing succeeded
//					  0 - bribing failed
//
int Firm::spy_bribe_succeed_chance(int bribeAmount, short briberSpyRecno, short workerId)
{
	Spy* spyPtr = spy_array[briberSpyRecno];

	err_when( spyPtr->spy_place != SPY_FIRM );
	err_when( spyPtr->spy_place_para != firm_recno );

	//---- if the bribing target is a worker ----//

	int unitLoyalty, unitRaceId, targetSpyRecno, unitCommandPower;

	if( workerId )
	{
		Worker* workerPtr = worker_array+workerId-1;

		unitLoyalty = workerPtr->loyalty();
		unitRaceId  = workerPtr->race_id;
		unitCommandPower = 0;
		targetSpyRecno = workerPtr->spy_recno;
	}
	else if( overseer_recno )
	{
		Unit* unitPtr = unit_array[overseer_recno];

		unitLoyalty = unitPtr->loyalty;
		unitRaceId  = unitPtr->race_id;
		unitCommandPower = unitPtr->commander_power();
		targetSpyRecno = unitPtr->spy_recno;
	}
	else
		err_here();

	err_when( unitRaceId < 1 || unitRaceId > MAX_RACE );

	//---- determine whether the bribe will be successful ----//

	int succeedChance;

	if( targetSpyRecno )		// if the bribe target is also a spy
	{
		err_when( spy_array[targetSpyRecno]->true_nation_recno == spyPtr->true_nation_recno );		// the player shouldn't be able to bribe units of his own

		succeedChance = 0;
	}
	else
	{
		succeedChance = spyPtr->spy_skill - unitLoyalty - unitCommandPower
							 + (int) nation_array[spyPtr->true_nation_recno]->reputation
							 + 200 * bribeAmount / MAX_BRIBE_AMOUNT;

		//-- the chance is higher if the spy or the spy's king is racially homongenous to the bribe target,

		int spyKingRaceId = nation_array[ spyPtr->true_nation_recno ]->race_id;

		succeedChance += race_res.is_same_race(spyPtr->race_id, unitRaceId) * 10 +
							  race_res.is_same_race(spyKingRaceId, unitRaceId) * 10;

		if( unitLoyalty > 60 )			// harder for bribe units with over 60 loyalty
			succeedChance -= (unitLoyalty-60);

		if( unitLoyalty > 70 )			// harder for bribe units with over 70 loyalty
			succeedChance -= (unitLoyalty-70);

		if( unitLoyalty > 80 )			// harder for bribe units with over 80 loyalty
			succeedChance -= (unitLoyalty-80);

		if( unitLoyalty > 90 )			// harder for bribe units with over 90 loyalty
			succeedChance -= (unitLoyalty-90);

		if( unitLoyalty == 100 )
			succeedChance = 0;
	}

	return succeedChance;
}
//----------- End of function Firm::spy_birbe_succeed_chance -----------//


//--------- Begin of function Firm::disp_bribe_unit ---------//
//
void Firm::disp_bribe_unit(int dispY1)
{
	//---------------- paint the panel -----------------//

	vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+46);

	//------- get the info of the bribe target ---------//

	// ####### begin Gilbert 8/8 ########//
	int 	raceId, unitLoyalty, unitId, rankId;
	const char* unitName;

	if( selected_worker_id )
	{
		Worker* workerPtr = worker_array+selected_worker_id-1;

		raceId 		= workerPtr->race_id;
		unitId      = workerPtr->unit_id;
		unitLoyalty = workerPtr->loyalty();
		unitName    = race_res[raceId]->get_name(workerPtr->name_id);
		rankId      = workerPtr->rank_id;
	}
	else if( overseer_recno )
	{
		Unit* unitPtr = unit_array[overseer_recno];

		raceId		= unitPtr->race_id;
		unitId      = unitPtr->unit_id;
		unitLoyalty = unitPtr->loyalty;
		unitName    = unitPtr->unit_name();
		rankId      = unitPtr->rank_id;
	}
	else
		err_here();

	//--------- display info of the bribe target ---------//

	int x=INFO_X1+6, y=dispY1+4;

	vga_front.put_bitmap(x, y, unit_res[unitId]->get_large_icon_ptr(rankId) );
	font_san.put( x+UNIT_LARGE_ICON_WIDTH+6, y+4, unitName );

	//------- display skill and productivity ---------//

	String str;

	str  = translate.process("Loyalty: ");
	str += unitLoyalty;

	font_san.disp( x+UNIT_LARGE_ICON_WIDTH+6, y+20, str, INFO_X2-10 );
}
//----------- End of function Firm::disp_bribe_unit -----------//


//--------- Begin of function Firm::disp_assassinate_result ---------//
//
void Firm::disp_assassinate_result(int refreshFlag)
{
	if( refreshFlag != INFO_REPAINT )
		return;

	int x=INFO_X1+4, y=INFO_Y1+4, y2=y+font_san.height()-1;

	if( assassinate_result == ASSASSINATE_SUCCEED_AT_LARGE )
	{
		vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+43 );

		font_san.center_put( INFO_X1, y, INFO_X2, y2, "Assassination Succeeded." );
		font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Your Spy Escaped." );
	}
	else if( assassinate_result == ASSASSINATE_SUCCEED_KILLED )
	{
		#ifdef GERMAN
			vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+80 );

			font_san.center_put( INFO_X1, y, INFO_X2, y2, "Assassination Succeeded." );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Your Spy" );				// German text is longer
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Was Caught" );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "And Executed." );
		#else
			vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+62 );

			font_san.center_put( INFO_X1, y, INFO_X2, y2, "Assassination Succeeded." );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Your Spy Was Caught" );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "And Executed." );
		#endif
	}
	else
	{
		#ifdef GERMAN
			vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+80 );

			font_san.center_put( INFO_X1, y	  , INFO_X2, y2, "Assassination Failed." );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Your Spy" );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Was Caught" );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "And Executed." );
		#else
			vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+62 );

			font_san.center_put( INFO_X1, y	  , INFO_X2, y2, "Assassination Failed." );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "Your Spy Was Caught" );
			font_san.center_put( INFO_X1, y+=18, INFO_X2, y2+=18, "And Executed." );
		#endif
	}

	y+=26;
	button_cancel.paint( INFO_X1, y, 'A', "CONTINUE" );
}
//----------- End of function Firm::disp_assassinate_result -----------//


//--------- Begin of function Firm::detect_assassinate_result ---------//
//
void Firm::detect_assassinate_result()
{
	//----- if it is display the bribe result right now -----//

	if( button_cancel.detect() )
	{
		firm_menu_mode = FIRM_MENU_MAIN;
		assassinate_result = 0;
		info.disp();
	}
}
//----------- End of function Firm::detect_assassinate_result -----------//

