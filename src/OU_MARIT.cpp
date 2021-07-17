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

//Filename   : OU_MARI3.CPP
//Description: UnitMarine - functions for loading/unloading goods, trading

#include <ALL.h>
#include <OREMOTE.h>
#include <OU_CARA.h>
#include <OU_MARI.h>
#include <OF_HARB.h>
#include <ONATION.h>
#include <OF_FACT.h>
#include <OF_MINE.h>
#include <OCONFIG.h>

static char		mprocessed_raw_qty_array[MAX_LINKED_FIRM_FIRM][MAX_RAW];	// 1 for not unload but can up load, 2 for unload but not up load
static char		mprocessed_product_raw_qty_array[MAX_LINKED_FIRM_FIRM][MAX_PRODUCT];	// ditto
static char		linked_mine_num;
static char		linked_factory_num;
static char		linked_market_num;
static short	linked_mine_array[MAX_LINKED_FIRM_FIRM];
static short	linked_factory_array[MAX_LINKED_FIRM_FIRM];
static short	linked_market_array[MAX_LINKED_FIRM_FIRM];
static char		empty_slot_position_array[MAX_LINKED_FIRM_FIRM];
static char		firm_selected_array[MAX_LINKED_FIRM_FIRM];

//--------- Begin of function UnitMarine::del_stop ---------//
void UnitMarine::del_stop(int stopId, char remoteAction)
{
	// ####### begin Gilbert 30/7 #######//
	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <unit recno> <stop id>
		short *shortPtr = (short *) remote.new_send_queue_msg(MSG_U_SHIP_DEL_STOP, 2*sizeof(short));
		*shortPtr = sprite_recno;
		shortPtr[1] = stopId;
		return;
	}
	// ####### end Gilbert 30/7 #######//

	if(remote.is_enable() && stop_array[stopId-1].firm_recno==0)
		return;

	stop_array[stopId-1].firm_recno = 0;
	stop_defined_num--;
	update_stop_list();

	if(unit_array.selected_recno==sprite_recno)
	{
		if(!remote.is_enable() || nation_recno==nation_array.player_recno || config.show_ai_info)
			info.disp();
	}
}
//---------- End of function UnitMarine::del_stop ----------//


//--------- Begin of function UnitMarine::update_stop_list ---------//
void UnitMarine::update_stop_list()
{
	//------------- used to debug for multiplayer game ------------------//
	#ifdef DEBUG
		misc.random(100);
	#endif

	//-------------------------------------------------------//
	// backup original destination stop firm recno
	//-------------------------------------------------------//
	err_when(stop_defined_num<0 || stop_defined_num>MAX_STOP_FOR_SHIP);
	short nextStopRecno = dest_stop_id ? stop_array[dest_stop_id-1].firm_recno : 0;

	//----------------------------------------------------------------------//
	// check stop existence and the relationship between firm's nation
	//----------------------------------------------------------------------//
	ShipStop *nodePtr = stop_array;
	Firm		*firmPtr;
	int i;
	for(i=0; i<MAX_STOP_FOR_SHIP; i++, nodePtr++)
	{
		if(!nodePtr->firm_recno)
			continue;

		if(firm_array.is_deleted(nodePtr->firm_recno))
		{
			nodePtr->firm_recno = 0;	// clear the recno
			stop_defined_num--;
			continue;
		}

		firmPtr = firm_array[nodePtr->firm_recno];

		if( !can_set_stop(firmPtr->firm_recno) ||
			 firmPtr->loc_x1 != nodePtr->firm_loc_x1 ||
			 firmPtr->loc_y1 != nodePtr->firm_loc_y1 )
		{
			nodePtr->firm_recno = 0;
			stop_defined_num--;
			continue;
		}
	}

	//-------------------------------------------------------//
	// remove duplicate node
	//-------------------------------------------------------//
	ShipStop *insertNodePtr = stop_array;

	if(stop_defined_num<1)
	{
		memset(stop_array, 0, sizeof(ShipStop)*MAX_STOP_FOR_SHIP);
		dest_stop_id = 0;
		return;	// no stop
	}

	//-------------------------------------------------------//
	// move the only firm_recno to the beginning of the array
	//-------------------------------------------------------//
	short compareRecno;
	for(i=0, nodePtr=stop_array; i<MAX_STOP_FOR_SHIP; i++, nodePtr++)
	{
		if(nodePtr->firm_recno)
		{
			compareRecno = nodePtr->firm_recno;
			break;
		}
	}

	if(i++) // else, the first record is already in the beginning of the array
		memcpy(insertNodePtr, nodePtr, sizeof(ShipStop));

	if(stop_defined_num==1)
	{
		memset(insertNodePtr+1, 0, sizeof(ShipStop)*(MAX_STOP_FOR_SHIP-1));
		dest_stop_id = 1;
		return;
	}

	short unprocessed = stop_defined_num-1;
	err_when(i==MAX_STOP_FOR_SHIP); // error if only one record
	err_when(!unprocessed);
	insertNodePtr++;
	nodePtr++;

	for(; i<MAX_STOP_FOR_SHIP && unprocessed; i++, nodePtr++)
	{
		if(!nodePtr->firm_recno)
			continue; // empty

		err_when(!nodePtr->firm_recno);
		if(nodePtr->firm_recno==compareRecno)
		{
			nodePtr->firm_recno = 0;
			stop_defined_num--;
		}
		else
		{
			compareRecno = nodePtr->firm_recno;

			if(insertNodePtr!=nodePtr)
				memcpy(insertNodePtr++, nodePtr, sizeof(ShipStop));
			else
				insertNodePtr++;
		}
		unprocessed--;
	}

	if(stop_defined_num>2)
	{
		//-------- compare the first and the end record -------//
		nodePtr = stop_array + stop_defined_num - 1; // point to the end
		if(nodePtr->firm_recno == stop_array[0].firm_recno)
		{
			nodePtr->firm_recno = 0;	// remove the end record
			stop_defined_num--;
		}
	}

	if(stop_defined_num<MAX_STOP_FOR_SHIP)
		memset(stop_array+stop_defined_num, 0, sizeof(ShipStop)*(MAX_STOP_FOR_SHIP-stop_defined_num));

	#ifdef DEBUG
		int debugCount;
		for(debugCount=0; debugCount<stop_defined_num; debugCount++)
			err_when(!stop_array[debugCount].firm_recno);

		for(; debugCount<MAX_STOP_FOR_SHIP; debugCount++)
			err_when(stop_array[debugCount].firm_recno);

		for(debugCount=0; debugCount<stop_defined_num; debugCount++)
			err_when(stop_array[debugCount].firm_recno &&
						stop_array[debugCount].firm_recno==stop_array[(debugCount+1)%MAX_STOP_FOR_SHIP].firm_recno);
	#endif

	//-----------------------------------------------------------------------------------------//
	// There should be at least one stop in the list.  Otherwise, clear all the stops
	//-----------------------------------------------------------------------------------------//
	int ourFirmExist = 0;
	for(i=0, nodePtr=stop_array; i<stop_defined_num; i++, nodePtr++)
	{
		err_when(firm_array.is_deleted(nodePtr->firm_recno));
		firmPtr = firm_array[nodePtr->firm_recno];
		if(firmPtr->nation_recno==nation_recno)
		{
			ourFirmExist++;
			break;
		}
	}

	if(!ourFirmExist) // none of the markets belong to our nation
	{
		memset(stop_array, 0, MAX_STOP_FOR_SHIP * sizeof(ShipStop));
		if(journey_status != INSIDE_FIRM)
			journey_status = ON_WAY_TO_FIRM;
		dest_stop_id		= 0;
		stop_defined_num	= 0;
		return;
	}

	//-----------------------------------------------------------------------------------------//
	// reset dest_stop_id since the order of the stop may be changed
	//-----------------------------------------------------------------------------------------//
	int xLoc = next_x_loc();
	int yLoc = next_y_loc();
	int dist, minDist=0x7FFF;

	for(i=0, dest_stop_id=0, nodePtr=stop_array; i<stop_defined_num; i++, nodePtr++)
	{
		if(nodePtr->firm_recno==nextStopRecno)
		{
			dest_stop_id = i+1;
			break;
		}
		else
		{
			firmPtr = firm_array[nodePtr->firm_recno];
			dist = misc.points_distance(xLoc, yLoc, firmPtr->center_x, firmPtr->center_y);

			if(dist<minDist)
			{
				dist = minDist;
				dest_stop_id = i+1;
			}
		}
	}

	err_when(dest_stop_id<0 || dest_stop_id>MAX_STOP_FOR_SHIP);
}
//----------- End of function UnitMarine::update_stop_list -----------//


//--------- Begin of function UnitMarine::get_next_stop_id ---------//
// Get the id. of the next defined stop.
//
// [int] curStopId - the id. of the current stop.
//							if it is MAX_STOP_FOR_SHIP, this function will return
//							the id. of the first valid stop.
//
//      					(default: MAX_STOP_FOR_SHIP)
// return :	0 ~ MAX_STOP_FOR_SHIP, where 0 for no valid stop
//
int UnitMarine::get_next_stop_id(int curStopId)
{
	int nextStopId = (curStopId>=stop_defined_num) ? 1 : curStopId+1;

	ShipStop *stopPtr = stop_array+nextStopId-1;

	int needUpdate = 0;
	if(firm_array.is_deleted(stopPtr->firm_recno))
		needUpdate++;
	else
	{
		Firm *firmPtr = firm_array[stopPtr->firm_recno];

		if( !can_set_stop(firmPtr->firm_recno) ||
			 firmPtr->loc_x1 != stopPtr->firm_loc_x1 ||
			 firmPtr->loc_y1 != stopPtr->firm_loc_y1 )
		{
			needUpdate++;
		}
	}

	if(needUpdate)
	{
		short preStopRecno = stop_array[curStopId-1].firm_recno;
		update_stop_list();

		if(!stop_defined_num)
			return 0;	// no stop is valid

		int i;
		for(i=1, stopPtr=stop_array; i<=stop_defined_num; i++, stopPtr++)
		{
			if(stopPtr->firm_recno==preStopRecno)
				nextStopId = (i>=stop_defined_num) ? 1 : i+1;
		}
	}

	return nextStopId;
}
//----------- End of function UnitMarine::get_next_stop_id -----------//


//--------- Begin of function UnitMarine::can_set_stop ---------//
//
// Whether can set a caravan's stop on the given firm.
//
int UnitMarine::can_set_stop(int firmRecno)
{
	Firm* firmPtr = firm_array[firmRecno];

	if( firmPtr->under_construction )
		return 0;

	if( firmPtr->firm_id != FIRM_HARBOR )
		return 0;

	return nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty;
}
//----------- End of function UnitMarine::can_set_stop -----------//


//--------- Begin of function UnitMarine::pre_process ---------//
void UnitMarine::pre_process()
{
	#define SURROUND_FIRM_WAIT_FACTOR	10

	Unit::pre_process();
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	err_when(action_mode==ACTION_DIE || cur_action==SPRITE_DIE || hit_points<=0);

	if(action_mode2>=ACTION_ATTACK_UNIT && action_mode2<=ACTION_ATTACK_WALL)
		return; // don't process trading if unit is attacking

	if(auto_mode) // process trading automatically, same as caravan
	{
		if(journey_status==INSIDE_FIRM)
		{
			ship_in_firm();
			return;
		}

		if(!stop_defined_num)
			return;

		//----------------- if there is only one defined stop --------------------//
		if(stop_defined_num==1)
		{
			if( firm_array.is_deleted(stop_array[0].firm_recno) )
			{
				update_stop_list();
				return;
			}

			Firm *firmPtr = firm_array[stop_array[0].firm_recno];
			if(firmPtr->loc_x1!=stop_array[0].firm_loc_x1 || firmPtr->loc_y1!=stop_array[0].firm_loc_y1)
			{
				update_stop_list();
				return;
			}

			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();
			int moveStep = move_step_magn();
			if(curXLoc<firmPtr->loc_x1-moveStep || curXLoc>firmPtr->loc_x2+moveStep ||
				curYLoc<firmPtr->loc_y1-moveStep || curYLoc>firmPtr->loc_y2+moveStep)
			{
				//### begin alex 6/10 ###//
				/*if((move_to_x_loc>=firmPtr->loc_x1-moveStep || move_to_x_loc<=firmPtr->loc_x2+moveStep) &&
					(move_to_y_loc>=firmPtr->loc_y1-moveStep || move_to_y_loc<=firmPtr->loc_y2+moveStep))
					return;

				move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
				journey_status = ON_WAY_TO_FIRM;*/
				if(cur_action==SPRITE_IDLE)
					move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
				else
					journey_status = ON_WAY_TO_FIRM;
				//#### end alex 6/10 ####//
			}
			else
			{
				if(cur_x==next_x && cur_y==next_y && cur_action==SPRITE_IDLE)
				{
					journey_status = SURROUND_FIRM;
					if(nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty)
					{
						if(wait_count<=0)
						{
							//---------- unloading goods -------------//
							cur_firm_recno = stop_array[0].firm_recno;
							get_harbor_linked_firm_info();
							harbor_unload_goods();
							wait_count = MAX_SHIP_WAIT_TERM*SURROUND_FIRM_WAIT_FACTOR;
							cur_firm_recno = 0;
						}
						else
							wait_count--;
					}
				}
			}
			return;
		}

		//------------ if there are more than one defined stop ---------------//
		err_when(stop_defined_num<=1);
		ship_on_way();
	}
	else if(journey_status==INSIDE_FIRM)
		ship_in_firm(0); // autoMode is off
}
//---------- End of function UnitMarine::pre_process ----------//


//--------- Begin of function UnitMarine::ship_in_firm ---------//
// journey_status : INSIDE_FIRM -->	ON_WAY_TO_FIRM
//												NO_STOP_DEFINED if no valid stop
//												SURROUND_FIRM if only one stop
//
// <int> autoMode		- 1 if autoMode is on
//							- 0 otherwise
//
void UnitMarine::ship_in_firm(int autoMode)
{
	//-----------------------------------------------------------------------------//
	// the harbor is deleted while the ship is in harbor
	//-----------------------------------------------------------------------------//
	if(cur_firm_recno && firm_array.is_deleted(cur_firm_recno))
	{
		hit_points = (float) 0;	// ship also die if the harbor is deleted
		unit_array.disappear_in_firm(sprite_recno); // ship also die if the harnor is deleted
		return;
	}

	//-----------------------------------------------------------------------------//
	// waiting (time to upload/download cargo)
	//-----------------------------------------------------------------------------//
	if(wait_count>0)
	{
		wait_count--;
		return;
	}

	//-----------------------------------------------------------------------------//
	// leave the harbor and go to another harbor if possible
	//-----------------------------------------------------------------------------//
	ShipStop *stopPtr = stop_array + dest_stop_id - 1;
	int xLoc = stop_x_loc;
	int yLoc = stop_y_loc;
	Location *locPtr = world.get_loc(xLoc, yLoc);
	Firm		*firmPtr;

	if(xLoc%2==0 && yLoc%2==0 && locPtr->can_move(mobile_type))
		init_sprite(xLoc, yLoc); // appear in the location the unit disappeared before
	else
	{
		//---- the entering location is blocked, select another location to leave ----//
		err_when(cur_firm_recno==0);
		firmPtr = firm_array[cur_firm_recno];

		if(appear_in_firm_surround(xLoc, yLoc, firmPtr))
		{
			init_sprite(xLoc, yLoc);
			stop();
		}
		else
		{
			wait_count = MAX_SHIP_WAIT_TERM*10; //********* BUGHERE, continue to wait or ....
			return;
		}
	}

	//-------------- get next stop id. ----------------//
	int nextStopId = get_next_stop_id(dest_stop_id);

	if(!nextStopId || dest_stop_id==nextStopId)
	{
		dest_stop_id = nextStopId;
		journey_status = (!nextStopId) ? NO_STOP_DEFINED : SURROUND_FIRM;
		return;	// no stop or only one stop is valid
	}

	dest_stop_id = nextStopId;
	firmPtr = firm_array[stop_array[dest_stop_id-1].firm_recno];

	cur_firm_recno = 0;
	journey_status = ON_WAY_TO_FIRM;

	if(autoMode) // move to next firm only if autoMode is on
		move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, FIRM_HARBOR);
}
//---------- End of function UnitMarine::ship_in_firm ----------//


//--------- Begin of function UnitMarine::ship_on_way ---------//
void UnitMarine::ship_on_way()
{
	//### begin alex 6/10 ###//
	//FirmInfo *firmInfo = firm_res[FIRM_HARBOR];
	//#### end alex 6/10 ####//

	if(cur_action==SPRITE_IDLE && journey_status!=SURROUND_FIRM)
	{
		if(!firm_array.is_deleted(stop_array[dest_stop_id-1].firm_recno))
		{
			Firm *firmPtr = firm_array[stop_array[dest_stop_id-1].firm_recno];
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, FIRM_HARBOR);
			int nextXLoc = next_x_loc();
			int nextYLoc = next_y_loc();
			int moveStep = move_step_magn();			
			if(nextXLoc>=firmPtr->loc_x1-moveStep && nextXLoc<=firmPtr->loc_x2+moveStep &&
				nextYLoc>=firmPtr->loc_y1-moveStep && nextYLoc<=firmPtr->loc_y2+moveStep)
				journey_status = SURROUND_FIRM;

			return;
		}
	}

	err_when(cur_action==SPRITE_ATTACK || action_mode==ACTION_ATTACK_UNIT || action_mode==ACTION_ATTACK_FIRM ||
				action_mode==ACTION_ATTACK_TOWN || action_mode==ACTION_ATTACK_WALL);

	if(unit_array.is_deleted(sprite_recno))
		return; //-***************** BUGHERE ***************//

	if(firm_array.is_deleted(stop_array[dest_stop_id-1].firm_recno))
	{
		update_stop_list();

		if(stop_defined_num) // move to next stop
		{
			Firm *firmPtr = firm_array[stop_array[dest_stop_id-1].firm_recno];
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
		}
		return;
	}

	ShipStop *stopPtr = stop_array + dest_stop_id - 1;
	Firm	*firmPtr = firm_array[stopPtr->firm_recno];

	int nextXLoc = next_x_loc();
	int nextYLoc = next_y_loc();
	int moveStep = move_step_magn();
	if(journey_status==SURROUND_FIRM ||
		( nextXLoc==move_to_x_loc && nextYLoc==move_to_y_loc && cur_x==next_x && cur_y==next_y && // move in a tile exactly
		  (nextXLoc>=firmPtr->loc_x1-moveStep && nextXLoc<=firmPtr->loc_x2+moveStep &&
			nextYLoc>=firmPtr->loc_y1-moveStep && nextYLoc<=firmPtr->loc_y2+moveStep) ))
	{
		extra_move_in_beach = NO_EXTRA_MOVE; // since the ship may enter the firm in odd location		

		stopPtr->update_pick_up();

		//-------------------------------------------------------//
		// load/unload goods
		//-------------------------------------------------------//
		cur_firm_recno = stopPtr->firm_recno;

		if(nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty)
		{
			get_harbor_linked_firm_info();
			harbor_unload_goods();
			if(stopPtr->pick_up_type == AUTO_PICK_UP)
				harbor_auto_load_goods();
			else if(stopPtr->pick_up_type!=NO_PICK_UP)
				harbor_load_goods();
		}

		//-------------------------------------------------------//
		//-------------------------------------------------------//
		stop_x_loc = move_to_x_loc; // store entering location
		stop_y_loc = move_to_y_loc;
		wait_count = MAX_SHIP_WAIT_TERM;		// set waiting term

		reset_path();
		deinit_sprite(1);	// the ship enters the harbor now. 1-keep it selected if it is currently selected

		err_when(cur_x!=-1);
		cur_x--;	// set cur_x to -2, such that invisible but still process pre_process()

		journey_status = INSIDE_FIRM;
	}
	else
	{
		if(cur_action!=SPRITE_MOVE)
		{
			//----------------------------------------------------//
			// blocked by something, go to the destination again
			// note: if return value is 0, cannot reach the firm.		//*********BUGHERE
			//----------------------------------------------------//
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, FIRM_HARBOR);
			journey_status = ON_WAY_TO_FIRM;
		}
	}
}
//---------- End of function UnitMarine::ship_on_way ----------//


//--------- Begin of function UnitMarine::appear_in_firm_surround ---------//
int UnitMarine::appear_in_firm_surround(int& xLoc, int& yLoc, Firm* firmPtr)
{
	FirmInfo *firmInfo = firm_res[FIRM_HARBOR];
	int firmWidth = firmInfo->loc_width;
	int firmHeight = firmInfo->loc_height;
	int smallestCount = firmWidth * firmHeight + 1;
	int largestCount = (firmWidth+2) * (firmHeight+2);
	int countLimit = largestCount - smallestCount;
	int count = misc.random(countLimit)+smallestCount;
	int checkXLoc, checkYLoc, xOffset, yOffset, found=0;
	Location *locPtr;

	//-----------------------------------------------------------------//
	for(int i=0; i<countLimit; i++)
	{
		misc.cal_move_around_a_point(count, firmWidth, firmHeight, xOffset, yOffset);
		checkXLoc = firmPtr->loc_x1 + xOffset;
		checkYLoc = firmPtr->loc_y1 + yOffset;

		if(checkXLoc%2 || checkYLoc%2|| 
			checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
		{
			count++;
			continue;
		}

		locPtr = world.get_loc(checkXLoc, checkYLoc);
		if(locPtr->can_move(mobile_type))
		{
			found++;
			break;
		}

		count++;
		if(count > largestCount)
			count = smallestCount;
	}

	//-----------------------------------------------------------------//
	if(found)
	{
		xLoc = checkXLoc;
		yLoc = checkYLoc;
		return 1;
	}

	return 0;
}
//---------- End of function UnitMarine::appear_in_firm_surround ----------//


//--------- Begin of function UnitMarine::get_harbor_linked_firm_info ---------//
void UnitMarine::get_harbor_linked_firm_info()
{
	FirmHarbor *firmHarborPtr = (FirmHarbor*) firm_array[cur_firm_recno];
	err_when(firmHarborPtr->firm_id!=FIRM_HARBOR);

	firmHarborPtr->update_linked_firm_info();
	linked_mine_num = firmHarborPtr->get_linked_mine_num();
	linked_factory_num = firmHarborPtr->get_linked_factory_num();
	linked_market_num = firmHarborPtr->get_linked_market_num();
	memcpy(linked_mine_array, firmHarborPtr->linked_mine_array, sizeof(short)*MAX_LINKED_FIRM_FIRM);
	memcpy(linked_factory_array, firmHarborPtr->linked_factory_array, sizeof(short)*MAX_LINKED_FIRM_FIRM);
	memcpy(linked_market_array, firmHarborPtr->linked_market_array, sizeof(short)*MAX_LINKED_FIRM_FIRM);
}
//---------- End of function UnitMarine::get_harbor_linked_firm_info ----------//


//--------- Begin of function UnitMarine::harbor_unload_goods ---------//
void UnitMarine::harbor_unload_goods()
{
	if(!linked_mine_num && !linked_factory_num && !linked_market_num)
		return; // no linked firm requires or supplys raw and product
	
	for(int i=0; i<MAX_LINKED_FIRM_FIRM; i++)
	{
		memset(mprocessed_raw_qty_array[i], 0, sizeof(char)*MAX_RAW);
		memset(mprocessed_product_raw_qty_array[i], 0, sizeof(char)*MAX_PRODUCT);
	}

	harbor_unload_product();
	harbor_unload_raw();
}
//----------- End of function UnitMarine::harbor_unload_goods -----------//


//--------- Begin of function UnitMarine::harbor_unload_product ---------//
// Product --> market
//
void UnitMarine::harbor_unload_product()
{
	if(!linked_market_num)
		return;

	err_when(linked_market_num>MAX_LINKED_FIRM_FIRM);

	int			i, j, k;
	short			totalDemand;
	short			*marketNodePtr; // point to linked_market_array
	char			*firmSelectedPtr; // mark which firm is used
	FirmMarket	*marketPtr;
	MarketGoods	*marketProductPtr;
	MarketGoods	*marketGoodsPtr; // used to find empty slot
	short			curStock, unloadQty, useEmptySlot;

	for(i=0; i<MAX_PRODUCT; i++)
	{
		if(!product_raw_qty_array[i])
			continue; // without this goods

		//----------------------------------------------------------------------//
		// calculate the demand of this goods in market
		//----------------------------------------------------------------------//
		totalDemand = 0;
		memset(empty_slot_position_array, 0, sizeof(char)*MAX_LINKED_FIRM_FIRM);
		memset(firm_selected_array, 0, sizeof(char)*MAX_LINKED_FIRM_FIRM);
		marketNodePtr = linked_market_array;
		firmSelectedPtr = firm_selected_array;
		for(j=0; j<linked_market_num; j++, marketNodePtr++, firmSelectedPtr++)
		{
			err_when(firm_array.is_deleted(*marketNodePtr));
			marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
			err_when(marketPtr->firm_id != FIRM_MARKET);

			if(marketPtr->nation_recno!=nation_recno)
				continue; // don't unload goods to market of other nation

			if(marketPtr->ai_status==MARKET_FOR_SELL)
				continue; // clearing the market stock, so no unloading

			//---------- check the demand of this goods in the market ---------//
			marketProductPtr = marketPtr->market_product_array[i];
			if(marketProductPtr)
			{
				err_when(marketProductPtr->product_raw_id && marketProductPtr->raw_id);
				totalDemand += (short)(marketPtr->max_stock_qty - marketProductPtr->stock_qty);
				++*firmSelectedPtr;
			}
			else // don't have this product, clear for empty slot
			{
				marketGoodsPtr = marketPtr->market_goods_array;
				for(k=0; k<MAX_MARKET_GOODS; k++, marketGoodsPtr++)
				{
					err_when(marketGoodsPtr->product_raw_id && marketGoodsPtr->raw_id);
					if(!marketGoodsPtr->stock_qty && !marketGoodsPtr->supply_30days())
					{
						empty_slot_position_array[j] = k;
						totalDemand += (short)marketPtr->max_stock_qty;
						++*firmSelectedPtr;
						break;
					}
				}
			}
		}

		//----------------------------------------------------------------------//
		// distribute the stock into each market
		//----------------------------------------------------------------------//
		curStock = product_raw_qty_array[i];
		marketNodePtr = linked_market_array;
		firmSelectedPtr = firm_selected_array;
		for(j=0; j<linked_market_num; j++, marketNodePtr++, firmSelectedPtr++)
		{
			if(!(*firmSelectedPtr))
				continue;

			err_when(firm_array.is_deleted(*marketNodePtr));
			marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
			err_when(marketPtr->firm_id != FIRM_MARKET);

			marketProductPtr = marketPtr->market_product_array[i];
			if(!marketProductPtr) // using empty slot, don't set the pointer to the market_goods_array until unloadQty>0
			{
				useEmptySlot = 1;
				marketProductPtr = marketPtr->market_goods_array + empty_slot_position_array[j];
			}
			else
				useEmptySlot = 0;

			unloadQty = totalDemand ? (short)((marketPtr->max_stock_qty-marketProductPtr->stock_qty)*curStock/totalDemand + 0.5) : 0;
			unloadQty = MIN((short)(marketPtr->max_stock_qty-marketProductPtr->stock_qty), unloadQty);
			unloadQty = MIN(product_raw_qty_array[i], unloadQty);

			if(unloadQty)
			{
				if(useEmptySlot)
					marketPtr->set_goods(0, i+1, empty_slot_position_array[j]);

 				marketProductPtr->stock_qty += unloadQty;
				err_when(marketProductPtr->stock_qty > marketPtr->max_stock_qty);
				product_raw_qty_array[i] -= unloadQty;
				err_when(product_raw_qty_array[i] < 0);
				mprocessed_product_raw_qty_array[linked_mine_num+linked_factory_num+j][i] += 2;
			}
		}
	}
}
//----------- End of function UnitMarine::harbor_unload_product -----------//


//--------- Begin of function UnitMarine::harbor_unload_raw ---------//	
// Raw material --> market and factory
//
void UnitMarine::harbor_unload_raw()
{
	if(!linked_factory_num && !linked_market_num)
		return;

	err_when(linked_factory_num+linked_market_num > MAX_LINKED_FIRM_FIRM);

	int			i, j, k;
	short			totalDemand;
	short			*factoryNodePtr; // point to linked_factory_array
	short			*marketNodePtr; // point to linked_market_array
	char			*firmSelectedPtr; // mark which firm is used (for factory and market)
	FirmFactory *factoryPtr;
	FirmMarket	*marketPtr;
	MarketGoods	*marketRawPtr;
	MarketGoods	*marketGoodsPtr; // used to find empty slot
	short			curStock, unloadQty, useEmptySlot;

	for(i=0; i<MAX_RAW; i++)
	{
		if(!raw_qty_array[i])
			continue; // without this goods

		totalDemand = 0;
		memset(empty_slot_position_array, 0, sizeof(char)*MAX_LINKED_FIRM_FIRM);
		memset(firm_selected_array, 0, sizeof(char)*MAX_LINKED_FIRM_FIRM);
		firmSelectedPtr = firm_selected_array;
		//----------------------------------------------------------------------//
		// calculate the demand of this goods in factory
		//----------------------------------------------------------------------//
		factoryNodePtr = linked_factory_array;
		for(j=0; j<linked_factory_num; j++, factoryNodePtr++, firmSelectedPtr++)
		{
			err_when(firm_array.is_deleted(*factoryNodePtr));
			factoryPtr = (FirmFactory*) firm_array[*factoryNodePtr];
			err_when(factoryPtr->firm_id != FIRM_FACTORY);

			if(factoryPtr->nation_recno!=nation_recno)
				continue; // don't unload goods to factory of other nation

			if(factoryPtr->ai_status==FACTORY_RELOCATE)
				continue; // clearing the factory stock, so no unloading

			if(factoryPtr->product_raw_id-1 == i)
			{
				totalDemand = (short) (factoryPtr->max_raw_stock_qty - factoryPtr->raw_stock_qty);
				++*firmSelectedPtr;
			}
		}

		//----------------------------------------------------------------------//
		// calculate the demand of this goods in market
		//----------------------------------------------------------------------//
		marketNodePtr = linked_market_array;
		for(j=0; j<linked_market_num; j++, marketNodePtr++, firmSelectedPtr++)
		{
			err_when(firm_array.is_deleted(*marketNodePtr));
			marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
			err_when(marketPtr->firm_id != FIRM_MARKET);

			if(marketPtr->nation_recno!=nation_recno)
				continue; // don't unload goods to market of other nation

			if(marketPtr->ai_status==MARKET_FOR_SELL)
				continue; // clearing the market stock, so no unloading

			//---------- check the demand of this goods in the market ---------//
			marketRawPtr = marketPtr->market_raw_array[i];
			if(marketRawPtr)
			{
				err_when(marketRawPtr->product_raw_id && marketRawPtr->raw_id);
				totalDemand += (short)(marketPtr->max_stock_qty - marketRawPtr->stock_qty);
				++*firmSelectedPtr;
			}
			else // don't have this raw, clear for empty slot
			{
				marketGoodsPtr = marketPtr->market_goods_array;
				for(k=0; k<MAX_MARKET_GOODS; k++, marketGoodsPtr++)
				{
					err_when(marketGoodsPtr->product_raw_id && marketGoodsPtr->raw_id);
					if(!marketGoodsPtr->stock_qty && !marketGoodsPtr->supply_30days())
					{
						empty_slot_position_array[j] = k;
						totalDemand += (short)marketPtr->max_stock_qty;
						++*firmSelectedPtr;
						break;
					}
				}
			}
		}

		//----------------------------------------------------------------------//
		// distribute the stock into each factory
		//----------------------------------------------------------------------//
		curStock = raw_qty_array[i];
		factoryNodePtr = linked_factory_array;
		firmSelectedPtr = firm_selected_array;
		for(j=0; j<linked_factory_num; j++, factoryNodePtr++, firmSelectedPtr++)
		{
			if(!(*firmSelectedPtr))
				continue;

			err_when(firm_array.is_deleted(*factoryNodePtr));
			factoryPtr = (FirmFactory*) firm_array[*factoryNodePtr];
			err_when(factoryPtr->firm_id != FIRM_FACTORY);
			err_when(factoryPtr->product_raw_id-1!=i);
			
			unloadQty = totalDemand ? (short)((factoryPtr->max_raw_stock_qty-factoryPtr->raw_stock_qty)*curStock/totalDemand + 0.5) : 0;
			unloadQty = MIN((short)(factoryPtr->max_raw_stock_qty-factoryPtr->raw_stock_qty), unloadQty);
			unloadQty = MIN(raw_qty_array[i], unloadQty);

 			factoryPtr->raw_stock_qty += unloadQty;
			err_when(factoryPtr->raw_stock_qty > factoryPtr->max_raw_stock_qty);
			raw_qty_array[i] -= unloadQty;
			err_when(raw_qty_array[i] < 0);
			mprocessed_raw_qty_array[linked_mine_num+j][i] += 2;
		}

		//----------------------------------------------------------------------//
		// distribute the stock into each market
		//----------------------------------------------------------------------//
		marketNodePtr = linked_market_array;
		for(j=0; j<linked_market_num; j++, marketNodePtr, firmSelectedPtr++)
		{
			if(!(*firmSelectedPtr))
				continue;

			err_when(firm_array.is_deleted(*marketNodePtr));
			marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
			err_when(marketPtr->firm_id != FIRM_MARKET);

			marketRawPtr = marketPtr->market_raw_array[i];
			if(!marketRawPtr) // using empty slot, don't set the pointer to the market_goods_array until unloadQty>0
			{
				useEmptySlot = 1;
				marketRawPtr = marketPtr->market_goods_array + empty_slot_position_array[j];
			}
			else
				useEmptySlot = 0;

			unloadQty = totalDemand ? (short)((marketPtr->max_stock_qty-marketRawPtr->stock_qty)*curStock/totalDemand + 0.5) : 0;
			unloadQty = MIN((short)(marketPtr->max_stock_qty-marketRawPtr->stock_qty), unloadQty);
			unloadQty = MIN(raw_qty_array[i], unloadQty);

			if(unloadQty)
			{
				if(useEmptySlot)
					marketPtr->set_goods(1, i+1, empty_slot_position_array[j]);

 				marketRawPtr->stock_qty += unloadQty;
				err_when(marketRawPtr->stock_qty > marketPtr->max_stock_qty);
				raw_qty_array[i] -= unloadQty;
				err_when(raw_qty_array[i] < 0);
				mprocessed_raw_qty_array[linked_mine_num+linked_factory_num+j][i] += 2;
			}
		}
	}
}
//----------- End of function UnitMarine::harbor_unload_raw -----------//


//--------- Begin of function UnitMarine::harbor_load_goods ---------//
void UnitMarine::harbor_load_goods()
{
	if(!linked_mine_num && !linked_factory_num && !linked_market_num)
		return;

	err_when(linked_mine_num+linked_factory_num+linked_market_num>MAX_LINKED_FIRM_FIRM);

	ShipStop *stopPtr = stop_array+dest_stop_id-1;
	if(stopPtr->pick_up_type == NO_PICK_UP)
		return; // return if not allowed to load any goods

	int pickUpType, goodsId;
	for(int i=0; i<MAX_PICK_UP_GOODS; i++)
	{
		if(!stopPtr->pick_up_array[i])
			continue;

		pickUpType = i+1;
		if(pickUpType>=PICK_UP_RAW_FIRST && pickUpType<=PICK_UP_RAW_LAST)
		{
			goodsId = pickUpType-PICK_UP_RAW_FIRST;
			
			if(raw_qty_array[goodsId]<carry_goods_capacity)
				harbor_load_raw(goodsId, 0, 1); // 1 -- only consider our firm
			
			if(raw_qty_array[goodsId]<carry_goods_capacity)
				harbor_load_raw(goodsId, 0, 0); // 0 -- only consider firm of other nation
		}
		else if(pickUpType>=PICK_UP_PRODUCT_FIRST && pickUpType<=PICK_UP_PRODUCT_LAST)
		{
			goodsId = pickUpType-PICK_UP_PRODUCT_FIRST;

			if(product_raw_qty_array[goodsId]<carry_goods_capacity) // 1 -- only consider our firm
				harbor_load_product(goodsId, 0, 1);
			
			if(product_raw_qty_array[goodsId]<carry_goods_capacity) // 0 -- only consider firm of other nation
				harbor_load_product(goodsId, 0, 0);
		}
		else
			err_here();
	}
}
//----------- End of function UnitMarine::harbor_load_goods -----------//


//--------- Begin of function UnitMarine::harbor_auto_load_goods ---------//
void UnitMarine::harbor_auto_load_goods()
{
	if(!linked_mine_num && !linked_factory_num && !linked_market_num)
		return;

	err_when(linked_mine_num+linked_factory_num+linked_market_num>MAX_LINKED_FIRM_FIRM);

	int i;
	for(i=0; i<MAX_PRODUCT; i++)
	{
		if(product_raw_qty_array[i]<carry_goods_capacity)
			harbor_load_product(i, 1, 1); // 1 -- only consider our market
	}

	for(i=0; i<MAX_RAW; i++)
	{
		if(raw_qty_array[i]<carry_goods_capacity)
			harbor_load_raw(i, 1, 1); // 1 -- only consider our market
	}
}
//----------- End of function UnitMarine::harbor_auto_load_goods -----------//


//--------- Begin of function UnitMarine::harbor_load_product ---------//
// <int> considerMode	- 1 only consider our market
//								- 0 only consider markets of other nation
//
void UnitMarine::harbor_load_product(int goodsId, int autoPickUp, int considerMode)
{
	if(!linked_factory_num && !linked_market_num)
		return;

	if(product_raw_qty_array[goodsId]==carry_goods_capacity)
		return;

	int			i;
	short			totalSupply;
	short			*factoryNodePtr; // point to linked_factory_array
	short			*marketNodePtr; // point to linked_market_array
	char			*firmSelectedPtr; // mark which firm is used (for factory and market)
	FirmFactory *factoryPtr;
	FirmMarket	*marketPtr;
	MarketGoods	*marketProductPtr;
	short			loadQty, keepStockQty;

	totalSupply = 0;
	memset(firm_selected_array, 0, sizeof(char)*MAX_LINKED_FIRM_FIRM);
	firmSelectedPtr = firm_selected_array;
	//----------------------------------------------------------------------//
	// calculate the supply of this goods in factory
	//----------------------------------------------------------------------//
	if(linked_factory_num)
	{
		factoryPtr = (FirmFactory*) firm_array[linked_factory_array[0]];
		keepStockQty = autoPickUp ? (short) (factoryPtr->max_stock_qty/5) : 0;
	}

	factoryNodePtr = linked_factory_array;
	for(i=0; i<linked_factory_num; i++, factoryNodePtr++, firmSelectedPtr++)
	{
		if(mprocessed_product_raw_qty_array[linked_mine_num+i][goodsId]==2)
			continue;

		err_when(firm_array.is_deleted(*factoryNodePtr));
		factoryPtr = (FirmFactory*) firm_array[*factoryNodePtr];
		err_when(factoryPtr->firm_id != FIRM_FACTORY);

		if(considerMode)
		{
			if(factoryPtr->nation_recno!=nation_recno)
				continue; // not our market
		}
		else
		{
			if(factoryPtr->nation_recno==nation_recno)
				continue; // not consider our market for this mode
		}

		//---------- check the supply of this goods in the factory ---------//
		if(factoryPtr->product_raw_id!=goodsId+1)
			continue; // incorrect product

		totalSupply += MAX((short)(factoryPtr->stock_qty-keepStockQty), 0);
		++*firmSelectedPtr;
	}

	//----------------------------------------------------------------------//
	// calculate the supply of this goods in market
	//----------------------------------------------------------------------//
	if(linked_market_num)
	{
		marketPtr = (FirmMarket*) firm_array[linked_market_array[0]];
		keepStockQty = autoPickUp ? (short) (marketPtr->max_stock_qty/5) : 0;
	}

	marketNodePtr = linked_market_array;
	for(i=0; i<linked_market_num; i++, marketNodePtr++, firmSelectedPtr++)
	{
		if(mprocessed_product_raw_qty_array[linked_mine_num+linked_factory_num+i][goodsId]==2)
			continue;
		
		err_when(firm_array.is_deleted(*marketNodePtr));
		marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
		err_when(marketPtr->firm_id != FIRM_MARKET);

		if(considerMode)
		{
			if(marketPtr->nation_recno!=nation_recno)
				continue; // not our market
		}
		else
		{
			if(marketPtr->nation_recno==nation_recno)
				continue; // not consider our market for this mode
		}
		
		//---------- check the supply of this goods in the market ---------//
		marketProductPtr = marketPtr->market_product_array[goodsId];
		if(marketProductPtr)
		{
			totalSupply += MAX((short)(marketProductPtr->stock_qty-keepStockQty), 0);
			++*firmSelectedPtr;
		}
	}

	Nation *nationPtr = nation_array[nation_recno];
	int curDemand = carry_goods_capacity-product_raw_qty_array[goodsId];
	firmSelectedPtr = firm_selected_array;
	//----------------------------------------------------------------------//
	// get the stock from each factory
	//----------------------------------------------------------------------//
	if(linked_factory_num)
	{
		factoryPtr = (FirmFactory*) firm_array[linked_factory_array[0]];
		keepStockQty = autoPickUp ? (short) (factoryPtr->max_stock_qty/5) : 0;
	}

	factoryNodePtr = linked_factory_array;
	for(i=0; i<linked_factory_num; i++, factoryNodePtr++, firmSelectedPtr++)
	{
		if(!(*firmSelectedPtr))
			continue;

		err_when(firm_array.is_deleted(*factoryNodePtr));
		factoryPtr = (FirmFactory*) firm_array[*factoryNodePtr];
		err_when(factoryPtr->firm_id != FIRM_FACTORY);
		err_when(factoryPtr->product_raw_id-1!=goodsId);

		loadQty = MAX((short) (factoryPtr->stock_qty-keepStockQty), 0);
		loadQty = totalSupply ? MIN((short) ((float)loadQty*curDemand/totalSupply), loadQty) : 0;

		if(factoryPtr->nation_recno!=nation_recno)
		{
			loadQty = (nationPtr->cash>0) ? (short) MIN(nationPtr->cash/PRODUCT_PRICE, loadQty) : 0;
			if(loadQty)
				nationPtr->import_goods(IMPORT_PRODUCT, factoryPtr->nation_recno, (float)loadQty*PRODUCT_PRICE);
		}

		factoryPtr->stock_qty -= loadQty;
		err_when(factoryPtr->stock_qty < 0);
		product_raw_qty_array[goodsId] += loadQty;
		err_when(product_raw_qty_array[goodsId] > carry_goods_capacity);
	}

	//----------------------------------------------------------------------//
	// get the stock from each market
	//----------------------------------------------------------------------//
	if(linked_market_num)
	{
		marketPtr = (FirmMarket*) firm_array[linked_market_array[0]];
		keepStockQty = autoPickUp ? (short) (marketPtr->max_stock_qty/5) : 0;
	}

	marketNodePtr = linked_market_array;
	for(i=0; i<linked_market_num; i++, marketNodePtr++, firmSelectedPtr++)
	{
		if(!(*firmSelectedPtr))
			continue;

		err_when(firm_array.is_deleted(*marketNodePtr));
		marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
		err_when(marketPtr->firm_id != FIRM_MARKET);

		marketProductPtr = marketPtr->market_product_array[goodsId];

		loadQty = MAX((short) marketProductPtr->stock_qty-keepStockQty, 0);
		loadQty = totalSupply ? MIN((short) ((float)loadQty*curDemand/totalSupply), loadQty) : 0;

		if(marketPtr->nation_recno!=nation_recno)
		{
			loadQty = (nationPtr->cash>0) ? (short) MIN(nationPtr->cash/PRODUCT_PRICE, loadQty) : 0;
			if(loadQty)
				nationPtr->import_goods(IMPORT_PRODUCT, marketPtr->nation_recno, (float)loadQty * PRODUCT_PRICE);
		}

 		marketProductPtr->stock_qty -= loadQty;
		err_when(marketProductPtr->stock_qty < 0);
		product_raw_qty_array[goodsId] += loadQty;
		err_when(product_raw_qty_array[goodsId] > carry_goods_capacity);
	}
}
//----------- End of function UnitMarine::harbor_load_product -----------//


//--------- Begin of function UnitMarine::harbor_load_raw ---------//
// <int> considerMode	- 1 only consider our market
//								- 0 only consider markets of other nation
//
void UnitMarine::harbor_load_raw(int goodsId, int autoPickUp, int considerMode)
{
	if(!linked_mine_num && !linked_market_num)
		return;

	if(raw_qty_array[goodsId]==carry_goods_capacity)
		return;

	int			i;
	short			totalSupply;
	short			*mineNodePtr; // point to linked_factory_array
	short			*marketNodePtr; // point to linked_market_array
	char			*firmSelectedPtr; // mark which firm is used (for factory and market)
	FirmMine		*minePtr;
	FirmMarket	*marketPtr;
	MarketGoods	*marketRawPtr;
	short			loadQty, keepStockQty;

	totalSupply = 0;
	memset(firm_selected_array, 0, sizeof(char)*MAX_LINKED_FIRM_FIRM);
	firmSelectedPtr = firm_selected_array;
	//----------------------------------------------------------------------//
	// calculate the supply of this goods in mine
	//----------------------------------------------------------------------//
	if(linked_mine_num)
	{
		minePtr = (FirmMine*) firm_array[linked_mine_array[0]];
		keepStockQty = autoPickUp ? (short) (minePtr->max_stock_qty/5) : 0;
	}

	mineNodePtr = linked_mine_array;
	for(i=0; i<linked_mine_num; i++, mineNodePtr++, firmSelectedPtr++)
	{
		if(mprocessed_raw_qty_array[i][goodsId]==2)
			continue;

		err_when(firm_array.is_deleted(*mineNodePtr));
		minePtr = (FirmMine*) firm_array[*mineNodePtr];
		err_when(minePtr->firm_id != FIRM_MINE);

		if(considerMode)
		{
			if(minePtr->nation_recno!=nation_recno)
				continue; // not our market
		}
		else
		{
			if(minePtr->nation_recno==nation_recno)
				continue; // not consider our market for this mode
		}

		//---------- check the supply of this goods in the mine ---------//
		if(minePtr->raw_id!=goodsId+1)
			continue; // incorrect goods

		totalSupply += MAX((short)(minePtr->stock_qty-keepStockQty), 0);
		++*firmSelectedPtr;
	}

	//----------------------------------------------------------------------//
	// calculate the supply of this goods in market
	//----------------------------------------------------------------------//
	if(linked_market_num)
	{
		marketPtr = (FirmMarket*) firm_array[linked_market_array[0]];
		keepStockQty = autoPickUp ? (short) (marketPtr->max_stock_qty/5) : 0;
	}

	marketNodePtr = linked_market_array;
	for(i=0; i<linked_market_num; i++, marketNodePtr++, firmSelectedPtr++)
	{
		if(mprocessed_raw_qty_array[linked_mine_num+linked_factory_num+i][goodsId]==2)
			continue;
		
		err_when(firm_array.is_deleted(*marketNodePtr));
		marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
		err_when(marketPtr->firm_id != FIRM_MARKET);

		if(considerMode)
		{
			if(marketPtr->nation_recno!=nation_recno)
				continue; // not our market
		}
		else
		{
			if(marketPtr->nation_recno==nation_recno)
				continue; // not consider our market for this mode
		}
		
		//---------- check the supply of this goods in the market ---------//
		marketRawPtr = marketPtr->market_raw_array[goodsId];
		if(marketRawPtr)
		{
			totalSupply += MAX((short)(marketRawPtr->stock_qty-keepStockQty), 0);
			++*firmSelectedPtr;
		}
	}

	Nation *nationPtr = nation_array[nation_recno];
	int curDemand = carry_goods_capacity-raw_qty_array[goodsId];
	firmSelectedPtr = firm_selected_array;
	//----------------------------------------------------------------------//
	// get the stock from each mine
	//----------------------------------------------------------------------//
	if(linked_mine_num)
	{
		minePtr = (FirmMine*) firm_array[linked_mine_array[0]];
		keepStockQty = autoPickUp ? (short) (minePtr->max_stock_qty/5) : 0;
	}

	mineNodePtr = linked_mine_array;
	for(i=0; i<linked_mine_num; i++, mineNodePtr++, firmSelectedPtr++)
	{
		if(!(*firmSelectedPtr))
			continue;

		err_when(firm_array.is_deleted(*mineNodePtr));
		minePtr = (FirmMine*) firm_array[*mineNodePtr];
		err_when(minePtr->firm_id != FIRM_MINE);
		err_when(minePtr->raw_id-1!=goodsId);

		loadQty = MAX((short) (minePtr->stock_qty-keepStockQty), 0);
		loadQty = totalSupply ? MIN((short) ((float)loadQty*curDemand/totalSupply), loadQty) : 0;

		if(minePtr->nation_recno!=nation_recno)
		{
			loadQty = (nationPtr->cash>0) ? (short) MIN(nationPtr->cash/RAW_PRICE, loadQty) : 0;
			if(loadQty)
				nationPtr->import_goods(IMPORT_RAW, minePtr->nation_recno, (float)loadQty*RAW_PRICE);
		}

		minePtr->stock_qty -= loadQty;
		err_when(minePtr->stock_qty < 0);
		raw_qty_array[goodsId] += loadQty;
		err_when(raw_qty_array[goodsId] > carry_goods_capacity);
	}

	//----------------------------------------------------------------------//
	// get the stock from each market
	//----------------------------------------------------------------------//
	if(linked_market_num)
	{
		marketPtr = (FirmMarket*) firm_array[linked_market_array[0]];
		keepStockQty = autoPickUp ? (short) (marketPtr->max_stock_qty/5) : 0;
	}

	marketNodePtr = linked_market_array;
	for(i=0; i<linked_market_num; i++, marketNodePtr++, firmSelectedPtr++)
	{
		if(!(*firmSelectedPtr))
			continue;

		err_when(firm_array.is_deleted(*marketNodePtr));
		marketPtr = (FirmMarket*) firm_array[*marketNodePtr];
		err_when(marketPtr->firm_id != FIRM_MARKET);

		marketRawPtr = marketPtr->market_raw_array[goodsId];

		loadQty = MAX((short) marketRawPtr->stock_qty-keepStockQty, 0);
		loadQty = totalSupply ? MIN((short) ((float)loadQty*curDemand/totalSupply), loadQty) : 0;

		if(marketPtr->nation_recno!=nation_recno)
		{
			loadQty = (nationPtr->cash>0) ? (short) MIN(nationPtr->cash/RAW_PRICE, loadQty) : 0;
			if(loadQty)
				nationPtr->import_goods(IMPORT_RAW, marketPtr->nation_recno, (float)loadQty * RAW_PRICE);
		}

 		marketRawPtr->stock_qty -= loadQty;
		err_when(marketRawPtr->stock_qty < 0);
		raw_qty_array[goodsId] += loadQty;
		err_when(raw_qty_array[goodsId] > carry_goods_capacity);
	}
}
//----------- End of function UnitMarine::harbor_load_raw -----------//
