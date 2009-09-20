//Filename    : OF_INN2.CPP
//Description : Firm Inn - AI functions

#include <ONATION.h>
#include <OINFO.h>
#include <OTOWN.h>
#include <OUNIT.h>
#include <OF_INN.h>
#include <OF_CAMP.h>

//--------- Begin of function FirmInn::process_ai ---------//

void FirmInn::process_ai()
{
	if( info.game_date%30==firm_recno%30 )
	{
		if( think_del() )
			return;
	}

	if( info.game_date%30==firm_recno%30 )
	{
		think_hire_spy();
		think_hire_general();
	}
}
//----------- End of function FirmInn::process_ai -----------//


//------- Begin of function FirmInn::think_del -----------//
//
// Think about deleting this firm.
//
int FirmInn::think_del()
{
	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->cash < 500 + 500 * ownNation->pref_cash_reserve / 100 &&
		 ownNation->profit_365days() < 0 )
	{
		ai_del_firm();
		return 1;
	}

	if( ownNation->ai_inn_count > ownNation->ai_supported_inn_count()+2 )		// if the current number of inns is more than the number the nation can support plus 2, then destroy the current one
	{
		ai_del_firm();
		return 1;
	}

	//-------- delete it if it is near no base town ------//

	Town* townPtr;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->nation_recno == nation_recno )
		{
			if( m.points_distance( townPtr->center_x, townPtr->center_y,
				 center_x, center_y ) <= EFFECTIVE_FIRM_TOWN_DISTANCE )
			{
				return 0;
			}
		}
	}

	ai_del_firm();
	return 1;
}
//--------- End of function FirmInn::think_del -----------//


//--------- Begin of function FirmInn::think_hire_spy ---------//

int FirmInn::think_hire_spy()
{
	Nation* ownNation = nation_array[nation_recno];

	if( !ownNation->ai_should_spend(ownNation->pref_spy/2) )
		return 0;

	//--------------------------------------------//

	InnUnit* innUnit = inn_unit_array;

	for( int i=0 ; i<inn_unit_count ; i++, innUnit++ )
	{
		if( innUnit->skill.skill_id != SKILL_SPYING )
			continue;

		int raceId = unit_res[innUnit->unit_id]->race_id;

		if( think_assign_spy_to(raceId, i+1) )
			return 1;
	}

	return 0;
}
//----------- End of function FirmInn::think_hire_spy -----------//


//-------- Begin of function FirmInn::think_assign_spy_to --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int FirmInn::think_assign_spy_to(int raceId, int innUnitRecno)
{
	Town *townPtr;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->majority_race() != raceId )
			continue;

		if( townPtr->region_id != region_id )
			continue;

		//---- think about assign spies to independent town to lower resistance ---//

		if( townPtr->nation_recno == 0 )
		{
			for( int j=0 ; j<MAX_RACE ; j++ )
			{
				//--- current resistance == target resistance if we don't have any spies in the town ---//

				if( townPtr->race_target_resistance_array[j][nation_recno-1] ==
					 townPtr->race_resistance_array[j][nation_recno-1] )
				{
					int unitRecno = hire(innUnitRecno);

					nation_array[nation_recno]->add_action( townPtr->loc_x1, townPtr->loc_y1,
								  -1, -1, ACTION_AI_ASSIGN_SPY, townPtr->nation_recno, 1, unitRecno );

					return 1;
				}
			}
		}
	}

	return 0;
}
//-------- End of function FirmInn::think_assign_spy_to --------//


//--------- Begin of function FirmInn::think_hire_general ---------//

int FirmInn::think_hire_general()
{
	Nation* ownNation = nation_array[nation_recno];

	if( !ownNation->ai_should_spend(ownNation->pref_military_development/2) )
		return 0;

	//--------------------------------------------//

	InnUnit* innUnit = inn_unit_array;

	for( int i=0 ; i<inn_unit_count ; i++, innUnit++ )
	{
		if( innUnit->skill.skill_id != SKILL_LEADING )
			continue;

		int raceId = unit_res[innUnit->unit_id]->race_id;

		if( think_assign_general_to(raceId, i+1) )
			return 1;
	}

	return 0;
}
//----------- End of function FirmInn::think_hire_general -----------//


//-------- Begin of function FirmInn::think_assign_general_to --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int FirmInn::think_assign_general_to(int raceId, int innUnitRecno)
{
	InnUnit* innUnit = inn_unit_array+innUnitRecno-1;
	Nation*  ownNation = nation_array[nation_recno];
	int		curRating, bestRating=10;		// the new one needs to be at least 10 points better than the existing one
	FirmCamp *firmCamp, *bestCamp=NULL;

	//----- think about which camp to move to -----//

	for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmCamp = (FirmCamp*) firm_array[ ownNation->ai_camp_array[i] ];

		if( firmCamp->region_id != region_id )
			continue;

		int curLeadership = firmCamp->cur_commander_leadership();
		int newLeadership = firmCamp->new_commander_leadership(raceId, innUnit->skill.skill_level);

		curRating = newLeadership - curLeadership;

		//-------------------------------------//

		if( curRating > bestRating )
		{
			//--- if there is already somebody being assigned to it ---//

			if( ownNation->is_action_exist(firmCamp->loc_x1, firmCamp->loc_y1,
				 -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP) )
			{
				continue;
			}

			bestRating = curRating;
			bestCamp   = firmCamp;
		}
	}

	if( !bestCamp )
		return 0;

	//--------------------------------------------//

	int unitRecno = hire(innUnitRecno);

	ownNation->add_action(bestCamp->loc_x1, bestCamp->loc_y1, -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP, 1, unitRecno);

	return 1;
}
//-------- End of function FirmInn::think_assign_general_to --------//
