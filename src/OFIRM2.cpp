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

//Filename    : OFIRM2.CPP
//Description : Class Firm - part 2

#include <OFIRM.h>
#include <OTOWN.h>
#include <OSPY.h>
#include <ONATION.h>
#include <ORACERES.h>
#include <OUNIT.h>
#include <OREMOTE.h>

//--------- Begin of function Firm::kill_overseer ---------//
//
// Kill the overeseer of the firm
//
void Firm::kill_overseer()
{
	if( !overseer_recno )
		return;

	//-------- if the overseer is a spy -------//

	Unit* unitPtr = unit_array[overseer_recno];

	if( unitPtr->spy_recno )
		spy_array[unitPtr->spy_recno]->set_place(SPY_UNDEFINED, 0);

	//-- no need to del the spy here, unit_array.del() will del the spy --//

	//-----------------------------------------//

	if( overseer_town_recno )
		town_array[overseer_town_recno]->dec_pop(unit_array[overseer_recno]->race_id, 1);

	unit_array.del(overseer_recno);

	overseer_recno = 0;
}
//----------- End of function Firm::kill_overseer -----------//


//--------- Begin of function Firm::kill_all_worker ---------//
//
// All the workers in the firm are deleted
//
void Firm::kill_all_worker()
{
	for(int i=worker_count; i>0; i--)
		kill_worker(i);
}
//----------- End of function Firm::kill_all_worker -----------//


//--------- Begin of function Firm::kill_worker ---------//
//
// Kill a specific worker.
//
void Firm::kill_worker(int workerId)
{
	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	err_when( workerId<1 || workerId>worker_count );

	//------- decrease worker no. and create an unit -----//

	Worker* workerPtr = worker_array+workerId-1;
	int	  unitRecno = 0;

	if( workerPtr->race_id && workerPtr->name_id )
		race_res[workerPtr->race_id]->free_name_id(workerPtr->name_id);

	if( workerPtr->town_recno )      // town_recno is 0 if the workers in the firm do not live in towns
		town_array[workerPtr->town_recno]->dec_pop(workerPtr->race_id, 1);		// 1-has job

	//-------- if this worker is a spy ---------//

	if( workerPtr->spy_recno )
	{
		spy_array[workerPtr->spy_recno]->set_place(SPY_UNDEFINED, 0);
		spy_array.del_spy( workerPtr->spy_recno );
	}

	//--- decrease the nation unit count as the Unit has already increased it ----//

	if( !firm_res[firm_id]->live_in_town )		// if the unit does not live in town, increase the unit count now
		unit_res[workerPtr->unit_id]->dec_nation_unit_count(nation_recno);

	//------- delete the record from the worker_array ------//

	err_when( worker_count > MAX_WORKER );
	err_when( selected_worker_id > worker_count );

	misc.del_array_rec(worker_array, worker_count, sizeof(Worker), workerId);

	if( selected_worker_id > workerId || selected_worker_id == worker_count )
		selected_worker_id--;

	worker_count--;

	if( worker_count==0 )
		selected_worker_id = 0;

	err_when( selected_worker_id > worker_count );
}
//----------- End of function Firm::kill_worker -----------//


//--------- Begin of function Firm::kill_builder ---------//

void Firm::kill_builder(short builderRecno)
{
	unit_array.del(builderRecno);
}
//----------- End of function Firm::kill_builder -----------//


//--------- Begin of function Firm::locate_space ---------//
int Firm::locate_space(int removeFirm, int &xLoc, int &yLoc, int xLoc2, int yLoc2, int width, int height, int mobileType, int regionId)
{
	int checkXLoc, checkYLoc;

	if(removeFirm)
	{
		//*** note only for land unit with size 1x1 ***//
		char	mType = UNIT_LAND;

		for(checkYLoc=loc_y1; checkYLoc<=loc_y2; checkYLoc++)
		{
			for(checkXLoc=loc_x1; checkXLoc<=loc_x2; checkXLoc++)
			{
				if(world.get_loc(checkXLoc, checkYLoc)->can_move(mType))
				{
					xLoc = checkXLoc;
					yLoc = checkYLoc;
					return 1;
				}
			}
		}
	}
	else
	{
		checkXLoc = loc_x1;
		checkYLoc = loc_y1;
		if(!world.locate_space(&checkXLoc, &checkYLoc, xLoc2, yLoc2, width, height, mobileType, regionId))
			return 0;
		else
		{
			xLoc = checkXLoc;
			yLoc = checkYLoc;
			return 1;
		}
	}

	return 0;
}
//----------- End of function Firm::locate_space -----------//


//--------- Begin of function Firm::find_idle_builder ---------//

int Firm::find_idle_builder(int nearest)
{
	Unit	*unitPtr;
	short	minDist = 0x1000;
	int	unitRecno = 0;

	for( int i=unit_array.size(); i>0; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno!=nation_recno || !unitPtr->race_id )
			continue;

		if( unitPtr->skill.skill_id != SKILL_CONSTRUCTION )
			continue;

		if( unitPtr->is_visible() && unitPtr->region_id() != region_id )
			continue;

		if( unitPtr->unit_mode == UNIT_MODE_CONSTRUCT )
		{
			Firm *firmPtr = firm_array[unitPtr->unit_mode_para];

			if( firmPtr->under_construction || (firmPtr->hit_points*100/firmPtr->max_hit_points)<=90 || info.game_date <= firmPtr->last_attacked_date+8 )
				continue;
		}
		else if( unitPtr->unit_mode == UNIT_MODE_UNDER_TRAINING )
		{
			continue;
		}
		else if( unitPtr->action_mode == ACTION_ASSIGN_TO_FIRM && unitPtr->action_para2 == firm_recno )
		{
			return i;
		}
		else if( unitPtr->action_mode != ACTION_STOP )
		{
			continue;
		}

		if( !nearest )
			return i;

		short curDist = misc.points_distance(unitPtr->next_x_loc(), unitPtr->next_y_loc(), loc_x1, loc_y1);
		if( curDist < minDist )
		{
			unitRecno = i;
			minDist = curDist;
		}
	}

	return unitRecno;
}
//----------- End of function Firm::find_idle_builder -----------//


//--------- Begin of function Firm::send_idle_builder_here ---------//

void Firm::send_idle_builder_here(char remoteAction)
{
	if( builder_recno )
		return;

	if( remote.is_enable() && !remoteAction )
	{
		// packet structure : <firm recno>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_REQ_BUILDER, sizeof(short));
		*shortPtr = firm_recno;
		return;
	}

	int unitRecno = find_idle_builder(1);
	if( !unitRecno )
		return;

	Unit *unitPtr = unit_array[unitRecno];
	if( unitPtr->unit_mode==UNIT_MODE_CONSTRUCT )
	{
		Firm *firmPtr = firm_array[unitPtr->unit_mode_para];

		// order idle unit out of the building
		if( !firmPtr->set_builder(0) )
		{
			return;
		}
	}

	unitPtr->assign(loc_x1, loc_y1);

	return;
}
//----------- End of function Firm::send_idle_builder_here -----------//
