
//Filename		: OAI_INFO.CPP
//Description	: AI - A.I. info structure

#include <OSYS.h>
#include <OTOWN.h>
#include <OF_MARK.h>
#include <OU_CARA.h>
#include <ONATION.h>


//--------- Begin of function Nation::update_ai_firm_array --------//
//
// <int> firmId - determine which firm array to be returned
//
// <int> actionType: 1  - add a record to the array
//							0  - no addition or deletion, just return
//							-1 - del a record from the array
//
// <int> actionRecno : the recno to be deleted, if actionType is -1.
//							   the recno to be added, if actionType is 1.
//	<int> arrayCount:	for return the count of the AI info array 
//
short* Nation::update_ai_firm_array(int firmId, int actionType, int actionRecno, int& arrayCount)
{
	short* rc;

	switch(firmId)
	{
		case FIRM_BASE:
			rc = update_ai_array(ai_base_count, ai_base_size, &ai_base_array,
					 AI_BASE_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_base_count;
			break;

		case FIRM_CAMP:
			rc = update_ai_array(ai_camp_count, ai_camp_size, &ai_camp_array,
					 AI_CAMP_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_camp_count;
			break;

		case FIRM_FACTORY:
			rc = update_ai_array(ai_factory_count, ai_factory_size, &ai_factory_array,
					 AI_FACTORY_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_factory_count;
			break;

		case FIRM_MARKET:
			rc = update_ai_array(ai_market_count, ai_market_size, &ai_market_array,
					 AI_MARKET_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_market_count;
			break;

		case FIRM_INN:
			rc = update_ai_array(ai_inn_count, ai_inn_size, &ai_inn_array,
					 AI_INN_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_inn_count;
			break;

		case FIRM_MINE:
			rc = update_ai_array(ai_mine_count, ai_mine_size, &ai_mine_array,
				  AI_MINE_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_mine_count;
			break;

		case FIRM_RESEARCH:
			rc = update_ai_array(ai_research_count, ai_research_size, &ai_research_array,
					 AI_RESEARCH_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_research_count;
			break;

		case FIRM_WAR_FACTORY:
			rc = update_ai_array(ai_war_count, ai_war_size, &ai_war_array,
					 AI_WAR_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_war_count;
			break;

		case FIRM_HARBOR:
			rc = update_ai_array(ai_harbor_count, ai_harbor_size, &ai_harbor_array,
					 AI_HARBOR_INC_SIZE, actionType, actionRecno);
			arrayCount = ai_harbor_count;
			break;

		default:
			err_here();
			return 0;
	}

	return rc;
}
//---------- End of function Nation::update_ai_firm_array --------//


//--------- Begin of function Nation::update_ai_array --------//
//
// <short&> aiInfoCount - the count of the AI info array.
// <short&> aiInfoSize  - the size of the AI info array.
// <short**> aiInfoArray - poniter to the AI info array.
// <int>    arrayIncSize - the increment size of the array.
//
// <int> actionType: 1  - add a record to the array
//							0  - no addition or deletion, just return
//							-1 - del a record from the array
//
// [int] actionRecno : the recno to be deleted, if actionType is -1.
//							  the recno to be added, if actionType is 1.
//
short* Nation::update_ai_array(short& aiInfoCount, short& aiInfoSize,
					short** aiInfoArrayPtr, int arrayIncSize, int actionType, int actionRecno)
{
	err_when( aiInfoCount<0 );
	err_when( aiInfoCount > aiInfoSize );
	err_when( actionType<-1 || actionType>1 );

	short* aiInfoArray = *aiInfoArrayPtr;

	if( actionType == -1 )
	{
		short* aiInfoPtr = aiInfoArray;

		for( int i=0 ; i<aiInfoCount ; i++, aiInfoPtr++ )
		{
			if( *aiInfoPtr == actionRecno )
			{
				if( i+1==aiInfoCount )		// the record to be deleted is the last record
				{
					*aiInfoPtr = 0;
				}
				else	// the record to be deleted is not the last record, somewhere in the array
				{
					//---- copy the last record to this slot which has been marked for deletion

					*aiInfoPtr = aiInfoArray[aiInfoCount-1];
					aiInfoArray[aiInfoCount-1] = 0;
				}

				aiInfoCount--;
				return aiInfoArray;
			}
		}

		err_here();		// not found, this shouldn't happen.
	}
	else if( actionType == 1 )
	{
		if( aiInfoCount == aiInfoSize )
		{
			#ifdef DEBUG
			short saveDate1 = aiInfoArray[0];		// for vertification of resizing that old data are kept
			short saveDate2 = aiInfoArray[aiInfoCount-1];		
			#endif

			aiInfoSize += arrayIncSize;

			*aiInfoArrayPtr = (short*) mem_resize( aiInfoArray, aiInfoSize*sizeof(short) );

			aiInfoArray = *aiInfoArrayPtr;
			
			err_when( saveDate1 != aiInfoArray[0] );		// for vertification of resizing that old data are kept
			err_when( saveDate2 != aiInfoArray[aiInfoCount-1] );		
		}

		aiInfoArray[aiInfoCount++] = actionRecno;
	}
	
	return aiInfoArray;
}
//---------- End of function Nation::update_ai_array --------//


//--------- Begin of function Nation::add_firm_info --------//
//
void Nation::add_firm_info(char firmId, short firmRecno)
{
	err_when( !firmId || !firmRecno );

	int aiFirmCount;

	update_ai_firm_array(firmId, 1, firmRecno, aiFirmCount);
}
//---------- End of function Nation::add_firm_info --------//


//--------- Begin of function Nation::del_firm_info --------//

void Nation::del_firm_info(char firmId, short firmRecno)
{
	err_when( !firmId || !firmRecno );

	int aiFirmCount;

	update_ai_firm_array(firmId, -1, firmRecno, aiFirmCount);
}
//---------- End of function Nation::del_firm_info --------//


//--------- Begin of function Nation::update_ai_region --------//

void Nation::update_ai_region()
{
	Town* townPtr;
	int	regionRecno;

	memset( ai_region_array, 0, sizeof(ai_region_array) );
	ai_region_count = 0;

	for( int i=0 ; i<ai_town_count ; i++ )
	{
		townPtr = town_array[ ai_town_array[i] ];

		//---- see if this region has been included -------//

		regionRecno=0;

		for( int j=0 ; j<ai_region_count ; j++ )
		{
			if( ai_region_array[j].region_id == townPtr->region_id )
			{
				regionRecno = j+1;
				break;
			}
		}

		if( !regionRecno )		// not included yet
		{
			if( ai_region_count == MAX_AI_REGION )		// no space for adding new region
				continue;

			err_when( ai_region_count > MAX_AI_REGION );

			ai_region_array[ai_region_count++].region_id = townPtr->region_id;

			regionRecno = ai_region_count;
		}

		//--- increase the town and base_town_count of the nation ---//

		ai_region_array[regionRecno-1].town_count++;

		if( townPtr->is_base_town )
			ai_region_array[regionRecno-1].base_town_count++;
	}
}
//---------- End of function Nation::update_ai_region --------//


//--------- Begin of function Nation::add_town_info --------//

void Nation::add_town_info(short townRecno)
{
	update_ai_array(ai_town_count, ai_town_size, &ai_town_array,
						 AI_TOWN_INC_SIZE, 1, townRecno);

	update_ai_region();
}
//---------- End of function Nation::add_town_info --------//


//--------- Begin of function Nation::del_town_info --------//

void Nation::del_town_info(short townRecno)
{
	err_when( ai_base_town_count<0 );

	//--- if this is a base town, decrease the base town counter ---//

	if( town_array[townRecno]->is_base_town )
	{
		ai_base_town_count--;
		err_when( ai_base_town_count<0 );
	}

	//------- delete the record from ai_town_array ------//

	update_ai_array(ai_town_count, ai_town_size, &ai_town_array,
						 AI_TOWN_INC_SIZE, -1, townRecno);

	update_ai_region();
}
//---------- End of function Nation::del_town_info --------//


//--------- Begin of function Nation::add_general_info --------//

void Nation::add_general_info(short unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->rank_id != RANK_KING && unitPtr->rank_id != RANK_GENERAL );

	update_ai_array(ai_general_count, ai_general_size,
						 &ai_general_array, AI_GENERAL_INC_SIZE, 1, unitRecno);
}
//---------- End of function Nation::add_general_info --------//


//--------- Begin of function Nation::del_general_info --------//

void Nation::del_general_info(short unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->rank_id != RANK_KING && unitPtr->rank_id != RANK_GENERAL );

	update_ai_array(ai_general_count, ai_general_size,
						 &ai_general_array, AI_GENERAL_INC_SIZE, -1, unitRecno);
}
//---------- End of function Nation::del_general_info --------//


//--------- Begin of function Nation::add_caravan_info --------//

void Nation::add_caravan_info(short unitRecno)
{
	update_ai_array(ai_caravan_count, ai_caravan_size, &ai_caravan_array,
						 AI_CARAVAN_INC_SIZE, 1, unitRecno);
}
//---------- End of function Nation::add_caravan_info --------//


//--------- Begin of function Nation::del_caravan_info --------//

void Nation::del_caravan_info(short unitRecno)
{
	update_ai_array(ai_caravan_count, ai_caravan_size, &ai_caravan_array,
						 AI_CARAVAN_INC_SIZE, -1, unitRecno);
}
//---------- End of function Nation::del_caravan_info --------//


//--------- Begin of function Nation::is_caravan_exist --------//
//
// Check whether there is an existing caravan travelling along
// the specific route.
//
// <int> firstStop, secondStop - firm recno of the first and second stops.
// [int] setStopInterval 		 - if this is given, then only caravans
//											that have been set stop within the given
//											days will be counted as existing ones.
//
int Nation::is_caravan_exist(int firstStop, int secondStop, int setStopInterval)
{
	UnitCaravan* unitCaravan;

	for( int i=0; i<ai_caravan_count; i++ )
	{
		unitCaravan = (UnitCaravan*) unit_array[ ai_caravan_array[i] ];

		if( ( unitCaravan->stop_array[0].firm_recno == firstStop &&
				unitCaravan->stop_array[1].firm_recno == secondStop ) ||
			 ( unitCaravan->stop_array[1].firm_recno == firstStop &&
				unitCaravan->stop_array[0].firm_recno == secondStop ) )
		{
			if( setStopInterval )
			{
				if( info.game_date - unitCaravan->last_set_stop_date < setStopInterval )
					return unitCaravan->sprite_recno;
			}
			else
				return unitCaravan->sprite_recno;
		}
	}

	return 0;
}
//---------- End of function Nation::is_caravan_exist --------//


//--------- Begin of function Nation::add_ship_info --------//

void Nation::add_ship_info(short unitRecno)
{
	update_ai_array(ai_ship_count, ai_ship_size, &ai_ship_array,
						 AI_SHIP_INC_SIZE, 1, unitRecno);
}
//---------- End of function Nation::add_ship_info --------//


//--------- Begin of function Nation::del_ship_info --------//

void Nation::del_ship_info(short unitRecno)
{
	update_ai_array(ai_ship_count, ai_ship_size, &ai_ship_array,
						 AI_SHIP_INC_SIZE, -1, unitRecno);
}
//---------- End of function Nation::del_ship_info --------//


//--------- Begin of function Nation::has_base_town_in_region --------//
//
// Return whether this nation has any base town in the given region.
//
int Nation::has_base_town_in_region(int regionId)
{
	for( int i=0 ; i<ai_region_count ; i++ )
	{
		if( ai_region_array[i].region_id == regionId )
			return ai_region_array[i].base_town_count > 0;
	}

	return 0;
}
//---------- End of function Nation::has_base_town_in_region --------//


//--------- Begin of function Nation::get_ai_region --------//
//
// Return the AIRegion of the given region id. 
//
AIRegion* Nation::get_ai_region(int regionId)
{
	for( int i=0 ; i<ai_region_count ; i++ )
	{
		if( ai_region_array[i].region_id == regionId )
			return ai_region_array+i;
	}

	return 0;
}
//---------- End of function Nation::get_ai_region --------//

