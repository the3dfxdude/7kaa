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

//Filename   : OAI_ACT.CPP
//Description: AI - action progressing functions

#include <ALL.h>
#include <OBOX.h>
#include <OSYS.h>
#include <OSPY.h>
#include <OINFO.h>
#include <OUNIT.h>
#include <OFIRM.h>
#include <ONATION.h>


//-------- Begin of function Nation::ai_build_firm -------//
//
// action_x_loc, action_y_loc - location of the building site.
// ref_x_loc,    ref_y_loc    - location of the town which the base
//										  should be built next to.
// action_para					   - firm id.
// action_para2				   - firm race id. (for FirmBase only)
//
int Nation::ai_build_firm(ActionNode* actionNode)
{
	if(!seek_path.total_node_avail)
		return 0;

	err_when( actionNode->action_x_loc < 0 || actionNode->action_y_loc < 0 );			// they must be given when this function is called

	//-------- determine the skill id. needed -------//

	int firmId  = actionNode->action_para;
	int raceId  = actionNode->action_para2;
	int skillId = firm_res[firmId]->firm_skill_id;

	if( !skillId )			// if the firm does not have a specific skill (e.g. Inn), use the general SKILL_CONSTRUCTION
		skillId = SKILL_CONSTRUCTION;						// for military camps, use construction workers instead of generals for facilitating migration of military camps

	//---- if there are camps that should be closed available now, transfer soldiers there to the new camp, ask a construction worker to build the camp so we can transfer the whole troop to the new camp ---//

	if( firmId==FIRM_CAMP &&
		 ai_has_should_close_camp( world.get_region_id( actionNode->action_x_loc, actionNode->action_y_loc ) ) )
	{
		skillId = SKILL_CONSTRUCTION;						// for military camps, use construction workers instead of generals for facilitating migration of military camps
	}

	//------------- get a skilled unit --------------//

	Unit* skilledUnit = get_skilled_unit(skillId, raceId, actionNode);

	if( !skilledUnit )
		return 0;

	//--- a unit with the specific skill is not found, try to find a construction unit instead ---//

	if( skillId != SKILL_CONSTRUCTION )
	{
		Unit* skilledUnit = get_skilled_unit(skillId, raceId, actionNode);

		if( !skilledUnit )
			return 0;
	}

	//------- build the firm now ---------//

	skilledUnit->build_firm(actionNode->action_x_loc, actionNode->action_y_loc, firmId, COMMAND_AI);

	if(skilledUnit->action_x_loc==actionNode->action_x_loc && skilledUnit->action_y_loc==actionNode->action_y_loc)
	{
		err_when( skilledUnit->nation_recno==0 );

		skilledUnit->ai_action_id = actionNode->action_id;
		actionNode->unit_recno = skilledUnit->sprite_recno;

		return 1;
	}
	else
	{
		skilledUnit->stop2();
		return 0;
	}
}
//-------- End of function Nation::ai_build_firm -------//


//----- Begin of function Nation::ai_assign_overseer -----//
//
// action_x_loc, action_y_loc - location of the firm to which the overseer should be assigned.
// ref_x_loc,    ref_y_loc    - not used
// action_para					   - firm id.
// [action_para2]				   - race id of the overseer
//										  (if not given, the majority race will be used.)
//
int Nation::ai_assign_overseer(ActionNode* actionNode)
{
	//---------------------------------------------------------------------------//
	// cancel action if the firm is deleted, has incorrect firm_id or nation is
	// changed
	//---------------------------------------------------------------------------//

	int firmId = actionNode->action_para;

   err_when( !firmId );

	if(!check_firm_ready(actionNode->action_x_loc, actionNode->action_y_loc, firmId))	// return 0 to cancel action
		return -1;		// -1 means remove the current action immediately

	if(!seek_path.total_node_avail)
		return 0;

	//-------- get the poisnter to the firm -------//

	Location* locPtr = world.get_loc(actionNode->action_x_loc, actionNode->action_y_loc);

	err_when(!locPtr->is_firm());

	Firm* firmPtr = firm_array[ locPtr->firm_recno() ];

	//-------- get a skilled unit --------//

	int raceId;		// the race of the needed unit

	if( actionNode->action_para2 )
	{
		raceId = actionNode->action_para2;
	}
	else
	{
		if( firmPtr->firm_id == FIRM_BASE )          // for seat of power, the race must be specific
			raceId = firm_res.get_build(firmPtr->firm_build_id)->race_id;
		else
			raceId = firmPtr->majority_race();
	}

	Unit* skilledUnit = get_skilled_unit(SKILL_LEADING, raceId, actionNode);

	if( !skilledUnit )
		return 0;

	//---------------------------------------------------------------------------//

	FirmInfo* firmInfo = firm_res[firmId];

	err_when( !firmInfo->need_overseer );

	if(skilledUnit->rank_id==RANK_SOLDIER)
		skilledUnit->set_rank(RANK_GENERAL);

	skilledUnit->assign(actionNode->action_x_loc, actionNode->action_y_loc);
	skilledUnit->ai_action_id = actionNode->action_id;

	err_when( skilledUnit->nation_recno != nation_recno );

	actionNode->unit_recno = skilledUnit->sprite_recno;

	return 1;
}
//----- End of function Nation::ai_assign_overseer -----//


//----- Begin of function Nation::ai_assign_construction_worker -----//
//
// action_x_loc, action_y_loc - location of the firm to which the overseer should be assigned.
// ref_x_loc,    ref_y_loc    - not used
//
int Nation::ai_assign_construction_worker(ActionNode* actionNode)
{
	//---------------------------------------------------------------------------//
	// cancel action if the firm is deleted, has incorrect firm_id or nation is
	// changed
	//---------------------------------------------------------------------------//

	if(!check_firm_ready(actionNode->action_x_loc, actionNode->action_y_loc))	// return 0 to cancel action
		return -1;		// -1 means remove the current action immediately

	if(!seek_path.total_node_avail)
		return 0;

	//-------- get the poisnter to the firm -------//

	Location* locPtr = world.get_loc(actionNode->action_x_loc, actionNode->action_y_loc);

	err_when(!locPtr->is_firm());

	Firm* firmPtr = firm_array[ locPtr->firm_recno() ];

	if( firmPtr->builder_recno )		// if the firm already has a construction worker
		return -1;

	//-------- get a skilled unit --------//

	Unit* skilledUnit = get_skilled_unit(SKILL_CONSTRUCTION, 0, actionNode);

	if( !skilledUnit )
		return 0;

	//------------------------------------------------------------------//

	skilledUnit->assign(actionNode->action_x_loc, actionNode->action_y_loc);
	skilledUnit->ai_action_id = actionNode->action_id;

	err_when( skilledUnit->nation_recno != nation_recno );

	actionNode->unit_recno = skilledUnit->sprite_recno;

	return 1;
}
//----- End of function Nation::ai_assign_construction_worker -----//


//----- Begin of function Nation::ai_assign_worker -----//
//
// action_x_loc, action_y_loc - location of the firm to which the overseer should be assigned.
// ref_x_loc,    ref_y_loc    - not used
// action_para					   - firm id.
// [action_para2]				   - race id of the overseer
//										  (if not given, the majority race will be used.)
//
int Nation::ai_assign_worker(ActionNode* actionNode)
{
	//---------------------------------------------------------------------------//
	// cancel action if the firm is deleted, has incorrect firm_id or nation is
	// changed
	//---------------------------------------------------------------------------//

	int firmId = actionNode->action_para;

	err_when( !firmId );

	if(!check_firm_ready(actionNode->action_x_loc, actionNode->action_y_loc, firmId))	// return 0 to cancel action
		return -1;		// -1 means remove the current action immediately

	if(!seek_path.total_node_avail)
		return 0;

	//---------------------------------------------------------------------------//
	// cancel this action if the firm already has enough workers
	//---------------------------------------------------------------------------//

	Location* locPtr = world.get_loc(actionNode->action_x_loc, actionNode->action_y_loc);

	err_when(!locPtr->is_firm());

	Firm* firmPtr = firm_array[locPtr->firm_recno()];

	err_when( firmPtr->firm_id != firmId );
	err_when( !firm_res[firmPtr->firm_id]->need_worker );

	if(firmPtr->worker_count>=MAX_WORKER)
		return -1;

	if( MAX_WORKER - firmPtr->worker_count < actionNode->instance_count )		// if the firm now has more workers, reduce the number needed to be assigned to the firm 
	{
		err_when( actionNode->processing_instance_count >= actionNode->instance_count );
		actionNode->instance_count = MAX( actionNode->processing_instance_count+1, MAX_WORKER - firmPtr->worker_count ); 
	}

	//---------------------------------------------------------------------------//
	// firm exists and belongs to our nation. Assign worker to firm
	//---------------------------------------------------------------------------//

	int   unitRecno=0;
	Unit* unitPtr = NULL;

	//----------- use a trained unit --------//

	if( actionNode->unit_recno )
		unitPtr = unit_array[actionNode->unit_recno];

	//------ recruit on job worker ----------//

	if( !unitPtr && firmPtr->firm_id != FIRM_BASE )   	// seat of power shouldn't call this function at all, as it doesn't handle the racial issue.
	{
		unitRecno = recruit_on_job_worker(firmPtr, actionNode->action_para2);

		if( unitRecno )
			unitPtr = unit_array[unitRecno];
	}

	//------- train a unit --------------//

	if( !unitPtr &&
		 firmPtr->firm_id == FIRM_CAMP &&
		 ai_should_spend( 20+pref_military_development/2 ) )		// 50 to 70
	{
		int trainTownRecno;

		if( train_unit( firmPtr->firm_skill_id, firmPtr->majority_race(),
							 actionNode->action_x_loc, actionNode->action_y_loc, 
							 trainTownRecno, actionNode->action_id ) )
		{
			actionNode->next_retry_date = info.game_date + TOTAL_TRAIN_DAYS + 1;
			actionNode->retry_count++;
			return 0;		// training in process
		}
	}

	//-------- recruit a unit ----------//

	if( !unitPtr )
	{
		unitRecno = recruit_jobless_worker(firmPtr, actionNode->action_para2);

		if( unitRecno )
			unitPtr = unit_array[unitRecno];
	}

	if( !unitPtr )
		return 0;

	//---------------------------------------------------------------------------//

	FirmInfo* firmInfo = firm_res[firmId];

	if( !world.get_loc(actionNode->action_x_loc, actionNode->action_y_loc)->is_firm() ) // firm exists, so assign
		return -1;

	err_when( unitPtr->rank_id != RANK_SOLDIER );

	unitPtr->assign(actionNode->action_x_loc, actionNode->action_y_loc);
	unitPtr->ai_action_id = actionNode->action_id;

	err_when( unitPtr->nation_recno != nation_recno );

	return 1;
}
//----- End of function Nation::ai_assign_worker -----//


//----- Begin of function Nation::ai_settle_to_other_town -----//
//
// action_x_loc, action_y_loc - location of the destination town.
// ref_x_loc, ref_y_loc 		- location of the origin town.
//
int Nation::ai_settle_to_other_town(ActionNode* actionNode)
{
	if(!seek_path.total_node_avail)
		return 0;

	//------- check if both towns are ready first --------//

	if(!check_town_ready(actionNode->action_x_loc, actionNode->action_y_loc) ||
		!check_town_ready(actionNode->ref_x_loc, actionNode->ref_y_loc))
	{
		return -1;
	}

	//----------------------------------------------------//
	// stop if no jobless population
	//----------------------------------------------------//

	Location* locPtr = world.get_loc(actionNode->ref_x_loc, actionNode->ref_y_loc);

	err_when(!locPtr->is_town() || town_array.is_deleted(locPtr->town_recno()));

	Town* townPtr = town_array[locPtr->town_recno()]; // point to the old town

	int raceId = townPtr->pick_random_race(0, 1);		// 0-don't pick has job unit, 1-pick spies

	if( !raceId )
		return -1;

	//---- if cannot recruit because the loyalty is too low ---//

	if( !townPtr->can_recruit(raceId) && townPtr->has_linked_own_camp )
	{
		int minRecruitLoyalty = MIN_RECRUIT_LOYALTY + townPtr->recruit_dec_loyalty(raceId, 0);

		//--- if cannot recruit because of low loyalty, reward the town people now ---//

		if( townPtr->race_loyalty_array[raceId-1] < minRecruitLoyalty )
		{
			if( cash > 0 && townPtr->accumulated_reward_penalty==0 )
			{
				townPtr->reward(COMMAND_AI);
			}

			if( !townPtr->can_recruit(raceId) )	// if still cannot be recruited, return 0 now
				return 0;
		}

		return 0;
	}

	//------------------------------------------------------//
	// recruit
	//------------------------------------------------------//

	int unitRecno = townPtr->recruit(-1, raceId, COMMAND_AI);

	if( !unitRecno )
		return 0;

	//---------------------------------------------------------------------------//
	// since it is not an important action, no need to add processing action
	//---------------------------------------------------------------------------//

	Unit* unitPtr = unit_array[unitRecno];

	unitPtr->assign(actionNode->action_x_loc, actionNode->action_y_loc);			// assign to the town
	unitPtr->ai_action_id = actionNode->action_id;

	err_when( unitPtr->nation_recno==0 );

	return 1;
}
//----- End of function Nation::ai_settle_to_other_town -----//


//--------- Begin of function Nation::ai_scout ----------//
//
// action_x_loc, action_y_loc - location of the destination town.
// ref_x_loc, ref_y_loc 		- location of the origin town.
//
int Nation::ai_scout(ActionNode* actionNode)
{
	if(!seek_path.total_node_avail)
		return 0;

	//------- check if both towns are ready first --------//

	if(!check_town_ready(actionNode->action_x_loc, actionNode->action_y_loc) ||
		!check_town_ready(actionNode->ref_x_loc, actionNode->ref_y_loc))
	{
		return -1;
	}

	//----------------------------------------------------//
	// stop if no jobless population
	//----------------------------------------------------//

	Location* locPtr = world.get_loc(actionNode->ref_x_loc, actionNode->ref_y_loc);

	err_when(!locPtr->is_town() || town_array.is_deleted(locPtr->town_recno()));

	Town* townPtr = town_array[locPtr->town_recno()]; // point to the old town

	int raceId = townPtr->pick_random_race(0, 1);		// 0-don't pick has job unit, 1-pick spies

	if( !raceId )
		return -1;

	//---- if cannot recruit because the loyalty is too low ---//

	if( !townPtr->can_recruit(raceId) && townPtr->has_linked_own_camp )
		return 0;

	//------------------------------------------------------//
	// recruit
	//------------------------------------------------------//

	int unitRecno = townPtr->recruit(-1, raceId, COMMAND_AI);

	if( !unitRecno )
		return 0;

	//---------------------------------------------------------------------------//
	// since it is not an important action, no need to add processing action
	//---------------------------------------------------------------------------//

	Unit* unitPtr = unit_array[unitRecno];

	short selectedArray[1];
	selectedArray[0] = unitRecno;

	//-- must use unit_array.move_to() instead of unit.move_to() because the destination may be reachable, it can be in a different region --//

	unit_array.move_to( actionNode->action_x_loc, actionNode->action_y_loc, 0, selectedArray, 1, COMMAND_AI );

	unitPtr->ai_action_id = actionNode->action_id;

	err_when( unitPtr->nation_recno==0 );

	return 1;
}
//-------- End of function Nation::ai_scout ----------//

