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
#include <OCONFIG.h>
#include <OINFO.h>
#include <OU_MARI.h>
#include <ONATION.h>

//------- Begin of function Nation::process_action --------//
//
// [int] priorityActionRecno - if this is given, this specific action will
//								 	    be processed first. Otherwise it will process
//							  		    actions in the array in a sequential order.
//
// [int] processActionMode   - if this is given, only message of this type
//										 will be processed and all messages in the queued of this 
//										 type will be processed. 
//
// Note: priorityActionRecno and processActionMode couldn't be used at
//		   the same time.
//
// return: <int> 1 - all messages of the specific action mode are processed or all messages are processed
//							or the priority message has been processed.
//
int Nation::process_action(int priorityActionRecno, int processActionMode)
{
	err_when( priorityActionRecno && processActionMode );

	// #define MAX_PROCESS_ACTION_TIME	 0.01		// maximum time given to processing actions (second)
	// unsigned expireTime = misc.get_time() + (unsigned)(MAX_PROCESS_ACTION_TIME*1000);

	int 			actionRecno, rc, delFlag, doneFlag=0;
	int			thisSessionProcessCount=0;		// actions processed in this call session
	ActionNode* actionNode;

	int divider = 4-config.ai_aggressiveness;		// the more nations there, the less process count
	int nationRecno = nation_recno;
	int maxSessionProcessCount = 70 / MAX(nation_array.nation_count,1) / MAX(divider,1);

	for( actionRecno=1 ; actionRecno<=action_count() &&
		  (thisSessionProcessCount < maxSessionProcessCount || processActionMode) && !doneFlag ;		// if processActionMode has been specific, then all messages in the queue of this type will be processed
		  actionRecno++ )
	{
		//----- priority action ------//

		if( priorityActionRecno )
		{
			actionRecno = priorityActionRecno;
			doneFlag 	= 1;			// mark it done, so if the function "continue" to the next loop, the function will end
		}

		actionNode = get_action(actionRecno);

		//----- if only process specific action mode -----//

		if( processActionMode && actionNode->action_mode != processActionMode )
			continue;

		//--- if the AI action is about processing diplomatic message ---//

		if( actionNode->action_mode == ACTION_AI_PROCESS_TALK_MSG &&
			 processActionMode != ACTION_AI_PROCESS_TALK_MSG )
		{
			if( misc.random(10) > 0 )		// 1/10 chance of processing the diplomatic messages
				continue;
		}

		//----------------------------------------------//

		if( actionNode->processing_instance_count == actionNode->instance_count )
		{
			//---------------------------------------------//
			//
			// If this action has been marked processing for over 6 months
			// and we still haven't received finishing notifications,
			// then there may be some accidents (or bugs) happened, and
			// we will need to delete the action.
			//
			//---------------------------------------------//

			if( info.game_date > actionNode->add_date + 30 * 6 )
			{
				del_action(actionRecno);
				actionRecno--;					// stay in this array position as the current one has been deleted, the following one replace the current one's position
			}

			continue;
		}

		err_when( actionNode->processing_instance_count > actionNode->instance_count );

		if( info.game_date < actionNode->next_retry_date && !priorityActionRecno )		// priorityAction bypass retry date checking 
			continue;

		if( actionNode->retry_count==0 )		// the actionNode may still exist even when retry_count==0, waiting for processed_count to reach processing_count
			continue;

		//-- there is an unprocessing action in this waiting node --//

		switch( actionNode->action_mode )
		{
			case ACTION_AI_BUILD_FIRM:
				rc = ai_build_firm(actionNode);
				break;

			case ACTION_AI_ASSIGN_OVERSEER:
				rc = ai_assign_overseer(actionNode);
				break;

			case ACTION_AI_ASSIGN_CONSTRUCTION_WORKER:
				rc = ai_assign_construction_worker(actionNode);
				break;

			case ACTION_AI_ASSIGN_WORKER:
				rc = ai_assign_worker(actionNode);
				break;

			case ACTION_AI_ASSIGN_SPY:
				rc = ai_assign_spy(actionNode);
				break;

			case ACTION_AI_SCOUT:
				rc = ai_scout(actionNode);
				break;

			case ACTION_AI_SETTLE_TO_OTHER_TOWN:
				rc = ai_settle_to_other_town(actionNode);
				break;

			case ACTION_AI_PROCESS_TALK_MSG:
				rc = ai_process_talk_msg(actionNode);
				break;

			case ACTION_AI_SEA_TRAVEL:
				rc = ai_sea_travel(actionNode);
				break;

			case ACTION_AI_SEA_TRAVEL2:
				rc = ai_sea_travel2(actionNode);
				break;

			case ACTION_AI_SEA_TRAVEL3:
				rc = ai_sea_travel3(actionNode);
				break;
		}

		if( nation_array.is_deleted(nationRecno) )		// diplomatic option can result in surrendering 
			return 0;

		actionNode = get_action(actionRecno);  // in case an action_array resize invalidated the prior ptr copy

		thisSessionProcessCount++;

		//------ check the return result -------//

		delFlag = 0;

		if( rc==1 )		// the action has been processed, but not sure whether it is complete or not
		{
			actionNode->processing_instance_count++;

			//---------------------------------------------------//
			// for ACTION_DYNAMIC, the action is immediately
			// deleted when processing_instance_count == instance_count.
			//---------------------------------------------------//

			if( actionNode->action_type == ACTION_DYNAMIC )
			{
				if( actionNode->processing_instance_count > actionNode->instance_count )
					delFlag = 1;
			}
		}
		else if( rc==0 )			// action failed, retry
		{
			actionNode->next_retry_date = info.game_date + 7;		// try again one week later

			if( --actionNode->retry_count==0 )
				delFlag = 1;

			err_when( actionNode->retry_count < 0 );
		}
		else if( rc== -1 )		// action failed, remove immediately if return -1
		{
			actionNode->retry_count = 0;
			delFlag = 1;
		}

		//-----------------------------------------//

		if( delFlag && actionNode->processing_instance_count == actionNode->processed_instance_count )		// if processing_count > processed_count, do not remove this ActionNode, as there are some unit using this actionNode, when they finish or fail the action, processed_count will increase and processing_count will reach processed_count
		{
			del_action(actionRecno);
			actionRecno--;					// stay in this array position as the current one has been deleted, the following one replace the current one's position
		}
	}

	return actionRecno > action_count() || doneFlag;
}
//---------- End of function Nation::process_action --------//


//------- Begin of function Nation::process_action_id --------//
//
// Process a specific action.
//
int Nation::process_action_id(int actionId)
{
	for( int i=action_count() ; i>0 ; i-- )
	{
		if( get_action(i)->action_id == actionId )
		{
			process_action(i);
			return 1;
		}
	}

	return 0;
}
//---------- End of function Nation::process_action_id --------//


//------- Begin of function Nation::get_action_based_on_id --------//
//
// Return ActionNode for the given actionId.
//
ActionNode* Nation::get_action_based_on_id(int actionId)
{
	for( int i=action_count() ; i>0 ; i-- )
	{
		if( get_action(i)->action_id == actionId )
		{
			return get_action(i);
		}
	}

	return 0;
}
//---------- End of function Nation::get_action_based_on_id --------//


//--------- Begin of function Nation::add_action --------//
//
// <short> xLoc, yLoc		 - location of the action
// <short> refXLoc, refYLoc - reference location (optional, not all action types need this)
// <int>   actionMode		 - action mode
// <int>   actionPara		 - action parameter
// [int]	  instanceCount	 - no. of instances of this action should be carried out
//									   (default: 1)
// [int]   unitRecno			 - recno of the unit responsible for this action
//										if not given, an appropriate unit will be found.
// [int]    actionPara2		 - action para2
// [short*] groupUnitArray  - array of unit recno in the group for the action
//										the no. of units in the array is stored in instance_count
//										(default: NULL)
//
// return: <int> recno of the action added in action_array
//					  0 - if the action is not added as it is already in action_array.
//
int Nation::add_action(short xLoc, short yLoc, short refXLoc, short refYLoc,
							  int actionMode, int actionPara, int instanceCount,
							  int unitRecno, int actionPara2, short* groupUnitArray)
{
	err_when( instanceCount < 1 );

	//--- check if the action has been added already or not ---//

	if( is_action_exist(xLoc, yLoc, refXLoc, refYLoc, actionMode, actionPara, unitRecno) )
		return 0;

	//---------- queue the action ----------//

	ActionNode actionNode;

	memset( &actionNode, 0, sizeof(ActionNode) );

	actionNode.action_mode		= actionMode;			// what kind of action
	actionNode.action_para		= actionPara;			// parameter of the action
	actionNode.action_para2		= actionPara2;			// parameter of the action
	actionNode.action_x_loc		= xLoc;					// location to act to
	actionNode.action_y_loc		= yLoc;
	actionNode.ref_x_loc			= refXLoc;				// the refective location of this action make to
	actionNode.ref_y_loc			= refYLoc;
	actionNode.retry_count		= STD_ACTION_RETRY_COUNT;  // number of term to wait before discarding this action
	actionNode.instance_count 	= instanceCount;				// num of this action being processed in the waiting queue

	int immediateProcess=0;

	if( groupUnitArray )
	{
		// the no. of units in the array is stored in instance_count

		err_when( instanceCount < 1 );
		err_when( instanceCount > ActionNode::MAX_ACTION_GROUP_UNIT );

		memcpy( actionNode.group_unit_array, groupUnitArray, instanceCount * sizeof(groupUnitArray[0]) );

		immediateProcess = 1;				// have to execute this command immediately as the unit in unit_array[] may change
		actionNode.retry_count = 1;		// only try once as the unit in unit_array[] may change
	}

	if( unitRecno )
	{
		//-- this may happen when the unit is a spy and has just changed cloak --//

		if( !nation_array[unit_array[unitRecno]->true_nation_recno()]->is_ai() &&
			 !nation_array[unit_array[unitRecno]->nation_recno]->is_ai() )
		{
			return 0;
		}

		//-------------------------------------//

		actionNode.unit_recno = unitRecno;

		if( unit_array[unitRecno]->is_visible() )
		{
			immediateProcess = 1;				// have to execute this command immediately as the unit in unit_array[] may change
			actionNode.retry_count = 1;		// only try once as the unit in unit_array[] may change
		}
		else		//--- the unit is still being trained ---//
		{
			actionNode.next_retry_date = info.game_date + TOTAL_TRAIN_DAYS + 1;
		}
	}

	//-------- set action type ---------//

	actionNode.action_type = ACTION_FIXED;		// default action type

	//------- link into action_array --------//

	return add_action( &actionNode, immediateProcess );
}
//---------- End of function Nation::add_action --------//


//--------- Begin of function Nation::add_action --------//
//
// <ActionNode*> actionNode - the action node to be added.
// [int] immediateProcess   - process this action immediately
//
int Nation::add_action(ActionNode* actionNode, int immediateProcess)
{
	//----------- reset some vars -----------//

	actionNode->add_date    = info.game_date;
	actionNode->action_id   = ++last_action_id;
	actionNode->retry_count = STD_ACTION_RETRY_COUNT;

	actionNode->processing_instance_count = 0;
	actionNode->processed_instance_count  = 0;

	//------- link into action_array --------//

	action_array.linkin( actionNode );

	if( immediateProcess )
		process_action( action_array.recno() );

	return action_array.recno();
}
//---------- End of function Nation::add_action --------//


//--------- Begin of function Nation::del_action --------//
//
void Nation::del_action(int actionRecno)
{
	action_array.linkout(actionRecno);
}
//---------- End of function Nation::del_action --------//


//--------- Begin of function Nation::is_action_exist --------//
//
// <short> actionXLoc, actionYLoc - action_?_loc in ActionNode to match with
// <short> refXLoc, refYLoc       - ref_?_loc in ActionNode to match with
// <int>	  actionMode             - action mode
// <int>   actionPara				 - parameter of the action
// [int]	  unitRecno              - unit recno to match with, only useful for actions under processing.
// [int]   checkMode					 - 1-check actionXLoc & actionYLoc only
//												2-check refXLoc & refYLoc only
//												0-check both
//												(default: 0)
//
// return: <int> >0 if the action recno of the existing action
//					 ==0 if not exist
//
int Nation::is_action_exist(short actionXLoc, short actionYLoc, short refXLoc, short refYLoc, int actionMode, int actionPara, int unitRecno, int checkMode)
{
	int 			i;
	ActionNode* actionNode;

	for( i=action_count() ; i>0 ; i-- )
	{
		actionNode = get_action(i);

		if( actionNode->action_mode == actionMode &&
			 actionNode->action_para == actionPara )
		{
			if( unitRecno && unitRecno != actionNode->unit_recno )	// it requests to match the unit recno and it is not matched here
				continue;

			if( refXLoc>=0 )
			{
				if( checkMode==0 || checkMode==2 )
				{
					if( actionNode->ref_x_loc==refXLoc && actionNode->ref_y_loc==refYLoc)
						return i;
				}
			}
			else
			{
				if( checkMode==0 || checkMode==1 )
				{
					if( actionNode->action_x_loc==actionXLoc && actionNode->action_y_loc==actionYLoc )
						return i;
				}
			}
		}
	}

	return 0;
}
//---------- End of function Nation::is_action_exist --------//


//--------- Begin of function Nation::is_action_exist --------//
//
// Check if the an action of the specific mode and para
// exists in the action_array.
//
// <int>	  actionMode - action mode
// <int>   actionPara - parameter of the action
// [int]   regionId	 - if this parameter is given, only
//								action with destination in this
//							   region will be checked.
//
int Nation::is_action_exist(int actionMode, int actionPara, int regionId)
{
	int 			i;
	ActionNode* actionNode;

	for( i=action_count() ; i>0 ; i-- )
	{
		actionNode = get_action(i);

		if( actionNode->action_mode == actionMode &&
			 actionNode->action_para == actionPara )
		{
			if( !regionId )
				return 1;

			err_when( actionNode->action_x_loc < 0 ||
						 actionNode->action_y_loc < 0 ||
						 actionNode->action_x_loc >= MAX_WORLD_X_LOC ||
						 actionNode->action_y_loc >= MAX_WORLD_Y_LOC );

			if( world.get_region_id(actionNode->action_x_loc,
				 actionNode->action_y_loc) == regionId )
			{
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function Nation::is_action_exist --------//


//--------- Begin of function Nation::is_build_action_exist --------//
//
// Return 1 if there is already a firm queued for building with
// a building location that is within the effective range
// of the given position.
//
int Nation::is_build_action_exist(int firmId, int xLoc, int yLoc)
{
	int 			i;
	ActionNode* actionNode;

	for( i=action_count() ; i>0 ; i-- )
	{
		actionNode = get_action(i);

		if( actionNode->action_mode == ACTION_AI_BUILD_FIRM &&
			 actionNode->action_para == firmId )
		{
			if( misc.points_distance( actionNode->action_x_loc, actionNode->action_y_loc,
				 xLoc, yLoc) <= EFFECTIVE_FIRM_TOWN_DISTANCE )
			{
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function Nation::is_build_action_exist --------//


//--------- Begin of function Nation::action_finished --------//
//
// The action under processing is finished successfully.
//
// <uint16_t>  aiActionId   - the id. of the action to be marked finished.
// [short] unitRecno    - if this is given, the the unit's all action
//								  will be stopped.
// [int]   actionFailure - whether the action is failed and called by
//                        action_failure(). (default: 0)
//
void Nation::action_finished(uint16_t aiActionId, short unitRecno, int actionFailure)
{
	//----- locate the ActionNode of this action ------//

	int 			actionRecno;
	ActionNode* actionNode;

	//----- try to match actions by unitRecno first ----//

	for( actionRecno=action_count() ; actionRecno>0 ; actionRecno-- )
	{
		actionNode = get_action(actionRecno);

		if( aiActionId == actionNode->action_id )
			break;
	}

	if( actionRecno==0 )		// not found
	{
//		if( sys.debug_session && !sys.signal_exit_flag )
//			box.msg( "Error: action_finished() - entry not found." );

		stop_unit_action(unitRecno);
		return;
	}

	//------------------------------------------------//
	//
	// In the above condition is true, that means this ship
	// unit has called this function once and the current
	// calling is a duplicated calling.
	//
	//------------------------------------------------//

	int shouldStop=1;

	if( actionNode->action_mode == ACTION_AI_SEA_TRAVEL )		// don't reset the unit's ai_action_id in ACTION_AI_SEA_TRAVEL mode as if we reset it, the ship will take new action and won't wait for the units to go aboard.
	{
		 if( !unit_array.is_deleted(unitRecno) &&
			  unit_res[ unit_array[unitRecno]->unit_id ]->unit_class == UNIT_CLASS_SHIP )
		 {
			 if( actionNode->action_para2 )
			 {
				 return;
			 }
			 else
			 {
				 actionNode->action_para2 = unitRecno;
				 shouldStop = 0;
			 }
		 }
	}

	//---------------------------------------------//
	//
	// Only handle ACTION_FIXED, for ACTION_DYNAMIC,
	// the action is immediately deleted when
	// processing_instance_count == instance_count.
	//
	//---------------------------------------------//

	if( actionNode->action_type != ACTION_FIXED )
	{
		stop_unit_action(unitRecno);
		return;
	}

	//-------------------------------------------------//

	actionNode->processed_instance_count++;

	err_when( actionNode->processed_instance_count > actionNode->processing_instance_count );
	err_when( actionNode->processed_instance_count > actionNode->instance_count );

	//---- if all requested instances are processed ----//

	int allDoneFlag=0;

	if( actionNode->processed_instance_count >= actionNode->instance_count )
		allDoneFlag = 1;

	//---- if the action is failed and all the outstanding units are finished, del the action ---//

	else if( actionNode->retry_count==0 &&
		 actionNode->processed_instance_count >= actionNode->processing_instance_count )
	{
		allDoneFlag = 1;
	}

	//------- stop the AI actions of the unit -----//

	if( shouldStop )
		stop_unit_action(unitRecno);

	//---- if the action is done, see if there needs to be a following action ----//

	if( allDoneFlag )
	{
		auto_next_action(actionNode);
		del_action(actionRecno);
	}
}
//---------- End of function Nation::action_finished --------//


//--------- Begin of function Nation::action_failure --------//
//
// It's basically the same as action_finish(). Now there isn't any
// difference.
//
// <uint16_t>  aiActionId - the id. of the action to be marked finished.
// [short] unitRecno  - if this is given, the the unit's all action
//							 	 will be stopped.
//
void Nation::action_failure(uint16_t aiActionId, short unitRecno)
{
	//-- if the unit is a ship, ignore it as it will be called by stop2() when it stops, but hasn't yet finished its action --//
/*
	Unit* unitPtr = unit_array[unitRecno];

	if( unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_SHIP )
	{
		int actionMode = get_action_based_on_id(aiActionId)->action_mode;

		if( actionMode == ACTION_AI_SEA_TRAVEL ||
			 actionMode == ACTION_AI_SEA_TRAVEL2 )
		{
			return;
		}
	}
*/
	//------------------------------------------------//

	action_finished(aiActionId, unitRecno, 1);		// 1 - action failure
}
//---------- End of function Nation::action_failure ---------//


//--------- Begin of function Nation::stop_unit_action --------//

void Nation::stop_unit_action(short unitRecno)
{
	if( !unitRecno )
		return;

	//------- stop the AI actions of the unit -----//
	//
	// It is possible that this is not an AI unit as
	// when a player spy cloaked as an enemy unit,
	// the AI will control it.
	//
	//---------------------------------------------//

	if( !unit_array.is_deleted(unitRecno) )
	{
		Unit* unitPtr = unit_array[unitRecno];

		unitPtr->ai_action_id = 0;

		//---- if the unit is a ship on the beach and it's mode isn't NO_EXTRA_MOVE, we couldn't call stop2() as that will cause bug ---//

		if(unitPtr->action_mode2==ACTION_SHIP_TO_BEACH)
		{
			UnitMarine *shipPtr = (UnitMarine*) unitPtr;

			err_when( unit_res[shipPtr->unit_id]->unit_class != UNIT_CLASS_SHIP );

			if( shipPtr->extra_move_in_beach != NO_EXTRA_MOVE )
				return;
		}

		//--------------------------------------------------//

		unitPtr->stop2();
		unitPtr->reset_action_misc_para();

		err_when(unitPtr->in_auto_defense_mode());
		err_when(unitPtr->action_x_loc!=-1 || unitPtr->action_y_loc!=-1);
		err_when(unitPtr->action_mode!=ACTION_STOP);
	}
}
//---------- End of function Nation::stop_unit_action ---------//


//--------- Begin of function Nation::auto_next_action --------//
//
// Automatically create a follow-up action to this action
// if this action needs one.
//
void Nation::auto_next_action(ActionNode* actionNode)
{
	int actionRecno=0;

	switch( actionNode->action_mode )
	{
		case ACTION_AI_SEA_TRAVEL:
			{
				ActionNode nextAction = *actionNode;
				nextAction.action_mode = ACTION_AI_SEA_TRAVEL2;
				nextAction.instance_count = 1;	// only move one ship, it was previously set to the no. units to aboard the ship
				actionRecno = add_action(&nextAction, 1);			// 1-immediate process flag
			}
			break;

		case ACTION_AI_SEA_TRAVEL2:
			{
				ActionNode nextAction = *actionNode;
				nextAction.action_mode = ACTION_AI_SEA_TRAVEL3;
				actionRecno = add_action(&nextAction, 1);			// 1-immediate process flag
			}
			break;
	}

	if( actionRecno )
		process_action(actionRecno);
}
//---------- End of function Nation::auto_next_action ---------//


