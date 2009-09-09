//Filename    : OFIRM2.CPP
//Description : Class Firm - part 2

#include <OFIRM.H>
#include <OTOWN.H>
#include <OSPY.H>
#include <ONATION.H>
#include <ORACERES.H>
#include <OUNIT.H>

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

	m.del_array_rec(worker_array, worker_count, sizeof(Worker), workerId);

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
		if(!world.locate_space(checkXLoc, checkYLoc, xLoc2, yLoc2, width, height, mobileType, regionId))
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

