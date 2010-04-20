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

//Filename    : OSPY2.CPP
//Description : Spy AI functions

#include <OINFO.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <ONATION.h>
#include <OSPY.h>

//--------- Begin of function Spy::process_ai ----------//
//
void Spy::process_ai()
{
	if( spy_recno%30 == info.game_date%30 )		// think about changing actions once 30 days
		think_reward();

	switch( spy_place )
	{
		case SPY_TOWN:
			if( spy_recno%30 == info.game_date%30 )
				think_town_spy();
			break;

		case SPY_FIRM:
			if( spy_recno%30 == info.game_date%30 )
				think_firm_spy();
			break;

		case SPY_MOBILE:
			if( spy_recno%5 == info.game_date%5 )
				think_mobile_spy();
			break;
	}
}
//---------- End of function Spy::process_ai ----------//


//--------- Begin of function Spy::think_town_spy ----------//
//
void Spy::think_town_spy()
{
	Town* townPtr = town_array[spy_place_para];

	if( townPtr->nation_recno == true_nation_recno )		// anti-spy
		return;

	//------ if it's an independent town ------//

	if( townPtr->nation_recno == 0 )
	{
		set_action_mode(SPY_SOW_DISSENT);

		//--- if the resistance has already drop low enough, the spy no longer needs to be in the town ---//

		if( townPtr->race_loyalty_array[race_id-1] < MIN_INDEPENDENT_DEFEND_LOYALTY  )
		{
			mobilize_town_spy();
		}
	}
	else
	{
		//-------------- if it's a nation town -------------//
		//
		// Set to sleep mode in most time so the spying skill can increase
		// gradually, when the loyalty level of the village falls to near
		// rebel level, set all of your spies in the village to sow dissent
		// mode and cause rebellion in the enemy village.
		//
		//--------------------------------------------------//

		Nation* trueNation = nation_array[true_nation_recno];

		if( townPtr->average_loyalty() < 50 - trueNation->pref_loyalty_concern/10 )		// pref_loyalty_concern actually does apply to here, we just use a preference var so that the decision making process will vary between nations
		{
			set_action_mode(SPY_SOW_DISSENT);
		}
		else
		{
			if( m.random(5)==0 )			// 20% chance of sowing dissents.
				set_action_mode(SPY_SOW_DISSENT);
			else
				set_action_mode(SPY_IDLE);
		}
	}
}
//---------- End of function Spy::think_town_spy ----------//


//--------- Begin of function Spy::think_firm_spy ----------//
//
void Spy::think_firm_spy()
{
	Firm* firmPtr = firm_array[spy_place_para];

	if( firmPtr->nation_recno == true_nation_recno )		// anti-spy
		return;

	//-------- try to capturing the firm --------//

	if( capture_firm() )
		return;

	//-------- think about bribing ---------//

	if( think_bribe() )
		return;

	//-------- think about assassinating ---------//

	if( think_assassinate() )
		return;

	//------ think about changing spy mode ----//

	else if( m.random(3)==0 )           // 1/10 chance to set it to idle to prevent from being caught
	{
		set_action_mode(SPY_IDLE);
	}
	else if( m.random(2)==0 &&
				can_sabotage() && firmPtr->is_operating() && firmPtr->productivity >= 20 )
	{
		set_action_mode(SPY_SABOTAGE);
	}
	else
	{
		set_action_mode(SPY_SOW_DISSENT);
	}
}
//---------- End of function Spy::think_firm_spy ----------//


//--------- Begin of function Spy::think_bribe ----------//
//
int Spy::think_bribe()
{
	Firm* firmPtr = firm_array[spy_place_para];

	//--- only bribe enemies in military camps ---//

	if( firmPtr->firm_id != FIRM_CAMP )
		return 0;

   //----- only if there is an overseer in the camp -----//

	if( !firmPtr->overseer_recno )
		return 0;

	//---- see if the overseer can be bribe (kings and your own spies can't be bribed) ----//

	if( !firmPtr->can_spy_bribe(0, true_nation_recno ) )		// 0-bribe the overseer
		return 0;

	//------ first check our financial status ------//

	Nation* ownNation = nation_array[true_nation_recno];
	Unit*   overseerUnit = unit_array[firmPtr->overseer_recno];

	if( spy_skill < MIN(50, overseerUnit->skill.skill_level) ||
		 !ownNation->ai_should_spend(30) )
	{
		return 0;
	}

	//----- think about how important is this firm -----//

	int firmImportance = 0;
	Town* townPtr;

	for( int i=firmPtr->linked_town_count-1 ; i>=0 ; i-- )
	{
		townPtr = town_array[ firmPtr->linked_town_array[i] ];

		if( townPtr->nation_recno == firmPtr->nation_recno )
			firmImportance += townPtr->population * 2;
		else
			firmImportance += townPtr->population;
	}

	//------- think about which one to bribe -------//

	//-- first get the succeedChange if the bribe amount is zero --//

	int succeedChange = firmPtr->spy_bribe_succeed_chance( 0, spy_recno, 0 );		// first 0 - $0 bribe amount, 3rd 0 - bribe the overseer

	//-- then based on it, figure out how much we have to offer to bribe successfully --//

	int bribeAmount = MAX_BRIBE_AMOUNT * (100-succeedChange) / 100;

	bribeAmount = MAX(100, bribeAmount);

	//--- only bribe when the nation has enough money ---//

	if( !ownNation->ai_should_spend(30, (float)bribeAmount) )
		return 0;

	//------- try to bribe the commander ----//

	int newSpyRecno = firmPtr->spy_bribe(bribeAmount, spy_recno, 0);

	if( !newSpyRecno )		// bribing failed
		return 1;				// return 1 as the spy has been killed

	Spy* newSpy = spy_array[newSpyRecno];

	err_when( newSpy->true_nation_recno != true_nation_recno );
	err_when( newSpy->spy_place != SPY_FIRM );

	if( newSpy->capture_firm() )			// try to capture the firm now
	{
		err_when( firm_array[newSpy->spy_place_para]->nation_recno != true_nation_recno );

		newSpy->drop_spy_identity();		// drop the spy identity of the newly bribed spy if the capture is successful, this will save the spying costs
	}

	return 1;
}
//---------- End of function Spy::think_bribe ----------//


//--------- Begin of function Spy::think_reward ----------//
//
// Think about rewarding this spy.
//
int Spy::think_reward()
{
	Nation* ownNation = nation_array[true_nation_recno];

	//----------------------------------------------------------//
	// The need to secure high loyalty on this unit is based on:
	// -its skill
	// -its combat level
	// -soldiers commanded by this unit
	//----------------------------------------------------------//

	int neededLoyalty = spy_skill * (100+ownNation->pref_loyalty_concern) / 100;

	neededLoyalty = MAX( UNIT_BETRAY_LOYALTY+10, neededLoyalty );		// 10 points above the betray loyalty level to prevent betrayal
	neededLoyalty = MIN( 100, neededLoyalty );

	//------- if the loyalty is already high enough ------//

	if( spy_loyalty >= neededLoyalty )
		return 0;

	//---------- see how many cash & profit we have now ---------//

	int rewardNeedRating = neededLoyalty - spy_loyalty;

	if( spy_loyalty < UNIT_BETRAY_LOYALTY+5 )
		rewardNeedRating += 50;

	if( ownNation->ai_should_spend(rewardNeedRating) )
	{
		reward(COMMAND_AI);
		return 1;
	}

	return 0;
}
//---------- End of function Spy::think_reward ----------//


//--------- Begin of function Spy::think_mobile_spy ----------//
//
int Spy::think_mobile_spy()
{
	Unit* unitPtr = unit_array[spy_place_para];

   //--- if the spy is on the ship, nothing can be done ---//

	if( !unitPtr->is_visible() )
		return 0;

	//---- if the spy has stopped and there is no new action ----//

	if( unitPtr->is_ai_all_stop() &&
		 (!notify_cloaked_nation_flag || cloaked_nation_recno==0) )
	{
		return think_mobile_spy_new_action();
	}

	return 0;
}
//---------- End of function Spy::think_mobile_spy ----------//


//-------- Begin of function Spy::think_mobile_spy_new_action --------//
//
int Spy::think_mobile_spy_new_action()
{
	Nation* trueNation = nation_array[true_nation_recno];

	err_when( spy_place != SPY_MOBILE );

	int spyRegionId = unit_array[spy_place_para]->region_id();

	//----- try to sneak into an enemy camp ------//

	int firmRecno = trueNation->think_assign_spy_target_camp(race_id, spyRegionId);

	if( firmRecno )
	{
		Firm* firmPtr = firm_array[firmRecno];

		return add_assign_spy_action( firmPtr->loc_x1, firmPtr->loc_y1, firmPtr->nation_recno );
	}

	//--- try to sneak into an enemy town or an independent town ---//

	int townRecno = trueNation->think_assign_spy_target_town(race_id, spyRegionId);

	if( townRecno )
	{
		Town* townPtr = town_array[townRecno];

		return add_assign_spy_action( townPtr->loc_x1, townPtr->loc_y1, townPtr->nation_recno );
	}

	//------ think if we should drop the spy identity -------//

	int dropIdentity = 0;

	//-------- if we already have too many spies --------//

	if( trueNation->total_spy_count > trueNation->total_population * (10+trueNation->pref_spy/5) / 100 )		// 10% to 30%
	{
		dropIdentity = 1;
	}

	//--- the expense of spies should not be too large ---//

	else if( trueNation->expense_365days(EXPENSE_SPY) >
		 trueNation->expense_365days() * (50+trueNation->pref_counter_spy) / 400 )
	{
		dropIdentity = 1;
	}

	else //------- try to assign to one of our own towns -------//
	{
		int townRecno = trueNation->think_assign_spy_own_town(race_id, spyRegionId);

		if( townRecno )
		{
			Town* townPtr = town_array[townRecno];

			return add_assign_spy_action( townPtr->loc_x1, townPtr->loc_y1, townPtr->nation_recno );
		}
		else
		{
			dropIdentity = 1;
		}
	}

	//---------- drop spy identity now --------//

	if( dropIdentity )
	{
		drop_spy_identity();
		return 1;
	}

	return 0;
}
//---------- End of function Spy::think_mobile_spy_new_action --------//


//-------- Begin of function Spy::add_assign_spy_action --------//
//
int Spy::add_assign_spy_action(int destXLoc, int destYLoc, int cloakedNationRecno)
{
	err_when( spy_place != SPY_MOBILE );
	err_when( unit_array.is_deleted(spy_place_para) );

	return nation_array[true_nation_recno]->add_action( destXLoc, destYLoc,
			 -1, -1, ACTION_AI_ASSIGN_SPY, cloakedNationRecno, 1, spy_place_para );
}
//---------- End of function Spy::add_assign_spy_action --------//


//--------- Begin of function Spy::ai_spy_being_attacked ----------//
//
// This function is called when this spy is under attack.
//
// <int> attackerUnitRecno - recno of the attacker unit.
//
int Spy::ai_spy_being_attacked(int attackerUnitRecno)
{
	err_when( spy_place != SPY_MOBILE );

	Unit*   attackerUnit = unit_array[attackerUnitRecno];
	Unit*   spyUnit     = unit_array[spy_place_para];
	Nation* trueNation = nation_array[true_nation_recno];

	//----- if we are attacking our own units -----//

	if( attackerUnit->true_nation_recno() == true_nation_recno )
	{
		if( spy_skill > 50-trueNation->pref_spy/10 ||
			 spyUnit->hit_points < spyUnit->max_hit_points * (100-trueNation->pref_military_courage/2) / 100 )
		{
			change_cloaked_nation( true_nation_recno );
			return 1;
		}
	}
	else
	{
		//---- if this unit is attacking units of other nations -----//
		//
		// If the nation this spy cloaked into is at war with the spy's
		// true nation and the nation which the spy is currently attacking
		// is not at war with the spy's true nation, then change
		// the spy's cloak to the non-hostile nation.
		//
		//-----------------------------------------------------------//

		if( trueNation->get_relation_status(attackerUnit->nation_recno) != NATION_HOSTILE &&
			 trueNation->get_relation_status(cloaked_nation_recno) == NATION_HOSTILE )
		{
			if( spy_skill > 50-trueNation->pref_spy/10 ||
				 spyUnit->hit_points < spyUnit->max_hit_points * (100-trueNation->pref_military_courage/2) / 100 )
			{
				change_cloaked_nation( true_nation_recno );
				return 1;
			}
		}
	}

   return 0;
}
//---------- End of function Spy::ai_spy_being_attacked ----------//


//--------- Begin of function Spy::think_assassinate ----------//
//
int Spy::think_assassinate()
{
	Firm* firmPtr = firm_array[spy_place_para];

	//--- only bribe enemies in military camps ---//

	if( firmPtr->firm_id != FIRM_CAMP )
		return 0;

	//----- only if there is an overseer in the camp -----//

	if( !firmPtr->overseer_recno )
		return 0;

	//---- get the attack and defense rating ----//

	int attackRating, defenseRating, otherDefenderCount;

	if( !get_assassinate_rating(firmPtr->overseer_recno, attackRating, defenseRating, otherDefenderCount) )		// return 0 if assassination is not possible
		return 0;

	Nation* trueNation = nation_array[true_nation_recno];

	if( attackRating + m.random(20+trueNation->pref_spy/2) > defenseRating )		// the random number is to increase the chance of attempting assassination
	{
		assassinate(firmPtr->overseer_recno, COMMAND_AI);
		return 1;
	}

	return 0;
}
//---------- End of function Spy::think_assassinate ----------//

