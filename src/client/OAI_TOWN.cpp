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

//Filename   : OAI_TOWN.CPP
//Description: AI - processing town

#include <ALL.h>
#include <OUNIT.h>
#include <OF_INN.h>
#include <OTOWN.h>
#include <OREGIONS.h>
#include <ONATION.h>


//--------- Begin of function Nation::think_town --------//
//
void Nation::think_town()
{
	optimize_town_race();
}
//---------- End of function Nation::think_town --------//


//--------- Begin of function Nation::optimize_town_race --------//
//
// Optimize the distribution of different races in different towns.
//
void Nation::optimize_town_race()
{
	RegionStat* regionStat = region_array.region_stat_array;

	for( int i=0 ; i<region_array.region_stat_count ; i++, regionStat++ )
	{
		if( regionStat->town_nation_count_array[nation_recno-1] > 0 )
			optimize_town_race_region( regionStat->region_id );
	}
}
//---------- End of function Nation::optimize_town_race --------//


//--------- Begin of function Nation::optimize_town_race_region --------//
//
// Optimize the distribution of different races in different towns in
// a single region.
//
void Nation::optimize_town_race_region(int regionId)
{
	//---- reckon the minority jobless pop of each race ----//

	int racePopArray[MAX_RACE];

	memset( racePopArray, 0, sizeof(racePopArray) );

	int 	i, j, majorityRace;
	Town* townPtr;

	for( i=0 ; i<ai_town_count ; i++ )
	{
		townPtr = town_array[ ai_town_array[i] ];

		if( townPtr->region_id != regionId )
			continue;

		majorityRace = townPtr->majority_race();

		for( j=0 ; j<MAX_RACE ; j++ )
		{
			if( j+1 != majorityRace )
				racePopArray[j] += townPtr->jobless_race_pop_array[j];
		}
	}

	//--- locate for towns with minority being majority and those minority race can move to ---//

	Town* destTown;

	for( int raceId=0 ; raceId<MAX_RACE ; raceId++ )
	{
		if( racePopArray[raceId-1] == 0 )		// we don't have any minority of this race
			continue;

		destTown = NULL;

		for( i=0 ; i<ai_town_count ; i++ )
		{
			townPtr = town_array[ ai_town_array[i] ];

			if( townPtr->region_id != regionId )
				continue;

			if( !townPtr->is_base_town )
				continue;

			if( townPtr->majority_race() == raceId &&
				 townPtr->population < MAX_TOWN_POPULATION )
			{
				destTown = townPtr;
				break;
			}
		}

		if( !destTown )
			continue;

		//---- if there is a suitable town for minority to move to ---//

		for( i=0 ; i<ai_town_count ; i++ )
		{
			townPtr = town_array[ ai_town_array[i] ];

			if( townPtr->region_id != regionId )
				continue;

			//---- move minority units from towns -----//

			int joblessCount = townPtr->jobless_race_pop_array[raceId-1];

			if( joblessCount > 0 &&
				 townPtr->majority_race() != raceId )
			{
				int migrateCount = MIN(8, joblessCount);		// migrate a maximum of 8 units at a time

				add_action( destTown->loc_x1, destTown->loc_y1,
					townPtr->loc_x1, townPtr->loc_y1, ACTION_AI_SETTLE_TO_OTHER_TOWN, 0, migrateCount);
			}
		}
	}
}
//---------- End of function Nation::optimize_town_race_region --------//
