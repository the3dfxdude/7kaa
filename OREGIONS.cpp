// Filename    : OREGIONS.CPP
// Description : functions on RegionStat

#include <stdlib.h>
#include <ONATION.h>
#include <OSITE.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OUNIT.h>
#include <OREGIONS.h>


//-------- Begin of function RegionArray::init_region_stat -------//

void RegionArray::init_region_stat()
{
	//------ count the no. of regions with statistic -----//
	//
	// Only include land regions that are big enough.
	//
	//----------------------------------------------------//

	region_stat_count=0;

	RegionInfo* regionInfo;

	for( int i=1 ; i<=region_info_count ; i++ )
	{
		regionInfo = region_array[i];

		if( regionInfo->region_size >= MIN_STAT_REGION_SIZE &&		// regions are sorted by their sizes
			 regionInfo->region_type == REGION_LAND )
		{
			region_stat_count++;
		}
	}

	err_when( region_stat_count==0 );

	//-------- init the region_stat_array ---------//

	region_stat_array = (RegionStat*) mem_add( region_stat_count * sizeof(RegionStat) );

	memset( region_stat_array, 0, region_stat_count * sizeof(RegionStat) );

	int regionStatId=1;

	for( i=1 ; i<=region_info_count ; i++ )
	{
		regionInfo = get_sorted_region(i);

		if( regionInfo->region_type != REGION_LAND )
			continue;

		err_when( regionStatId<1 || regionStatId>region_stat_count );

		region_stat_array[regionStatId-1].region_id = regionInfo->region_id;
		regionInfo->region_stat_id	= regionStatId;

		if( ++regionStatId > region_stat_count )
			break;
	}

	err_when( regionStatId != region_stat_count+1 );		// no all regionStat get their region_id

	for( i=0 ; i<region_stat_count ; i++ )
		region_stat_array[i].init();

	update_region_stat();
}
//--------- End of function RegionArray::init_region_stat -------//


//--------- Begin of function RegionArray::update_region_stat -------//

void RegionArray::update_region_stat()
{
	for( int i=0 ; i<region_stat_count ; i++ )
		region_stat_array[i].update_stat();
}
//--------- End of function RegionArray::update_region_stat -------//


//--------- Begin of function RegionStat::init -------//

void RegionStat::init()
{
	//------- init reachable region array ------//

	reachable_region_count=0;

	for( int seaRegionId=1 ; seaRegionId<=region_array.region_info_count ; seaRegionId++ )
	{
		if( region_array[seaRegionId]->region_type != REGION_SEA )
			continue;

		if( !region_array.is_adjacent(region_id, seaRegionId) )
			continue;

		//--- scan thru all big regions (regions in region_stat_array) ---//

		RegionStat* regionStat = region_array.region_stat_array;

		for( int i=1 ; i<=region_array.region_stat_count ; i++, regionStat++ )
		{
			if( regionStat->region_id==region_id )
				continue;

			if( region_array.is_adjacent(seaRegionId, regionStat->region_id) )
			{
				reachable_region_array[reachable_region_count].sea_region_id  = seaRegionId;
				reachable_region_array[reachable_region_count].land_region_stat_id = i;

				if( ++reachable_region_count == MAX_REACHABLE_REGION_PER_STAT )
					return;
			}
		}
	}
}
//--------- End of function RegionStat::init -------//


//--------- Begin of function RegionStat::update_stat -------//

void RegionStat::update_stat()
{
	//------ save useful constant info ------//

	int 			regionId = region_id;
	RegionPath  reachableRegionArray[MAX_REACHABLE_REGION_PER_STAT];
	char			reachableRegionCount = reachable_region_count;

	err_when( sizeof(reachable_region_array) != sizeof(reachableRegionArray) );

	memcpy( reachableRegionArray, reachable_region_array, sizeof(reachable_region_array) );

	memset( this, 0, sizeof(RegionStat) );		// reset all data

	region_id = regionId;
	reachable_region_count = reachableRegionCount;

	memcpy( reachable_region_array, reachableRegionArray, sizeof(reachable_region_array) );

	//--------- update firm stat ---------//

	Firm* firmPtr;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->region_id != region_id )
			continue;

		if( firmPtr->nation_recno==0 )		// monster firms
			continue;

      err_when( firmPtr->firm_id < 1 || firmPtr->firm_id > MAX_FIRM_TYPE );
		err_when( firmPtr->nation_recno < 1 || firmPtr->nation_recno > MAX_NATION );

		firm_type_count_array[firmPtr->firm_id-1]++;
		firm_nation_count_array[firmPtr->nation_recno-1]++;

		total_firm_count++;

		if( firmPtr->firm_id == FIRM_CAMP )
			camp_nation_count_array[firmPtr->nation_recno-1]++;

		if( firmPtr->firm_id == FIRM_HARBOR )
			harbor_nation_count_array[firmPtr->nation_recno-1]++;

		if( firmPtr->firm_id == FIRM_MINE )
			mine_nation_count_array[firmPtr->nation_recno-1]++;
	}

	//--------- update town stat ---------//

	Town* townPtr;

	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->region_id != region_id )
			continue;

		if( townPtr->nation_recno )
		{
			err_when( townPtr->nation_recno < 1 || townPtr->nation_recno > MAX_NATION );

			town_nation_count_array[townPtr->nation_recno-1]++;

			if( townPtr->is_base_town )
				base_town_nation_count_array[townPtr->nation_recno-1]++;

			nation_population_array[townPtr->nation_recno-1] += townPtr->population;
			nation_jobless_population_array[townPtr->nation_recno-1] += townPtr->jobless_population;
		}
		else
			independent_town_count++;

		total_town_count++;
	}

	//--------- update unit stat ---------//

	Unit* unitPtr;

	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->region_id() != region_id )
			continue;

		if( unitPtr->nation_recno )
			unit_nation_count_array[unitPtr->nation_recno-1]++;
		else
			independent_unit_count++;

		total_unit_count++;
	}

	//--------- update site count --------//

	Site* sitePtr;

	for( i=site_array.size() ; i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( sitePtr->region_id != region_id )
			continue;

		if( sitePtr->site_type == SITE_RAW )
			raw_count++;

		site_count++;
	}

	//----- update each nation's presence on the region -----//

	for( i=0 ; i<nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i+1) )
			continue;

		if( firm_nation_count_array[i] > 0 ||
			 town_nation_count_array[i] > 0 ||
			 unit_nation_count_array[i] > 0 )
		{
			nation_is_present_array[i] = 1;

			nation_presence_count++;
		}
	}
}
//--------- End of function RegionStat::update_stat -------//


//--------- Begin of function RegionArray::get_sea_path_region_id -------//
//
// Return the region id. of the sea path between the two given regions.
//
int RegionArray::get_sea_path_region_id(int regionId1, int regionId2)
{
	RegionStat* regionStat = region_array.get_region_stat(regionId1);
	int			regionStatId2 = region_array[regionId2]->region_stat_id;

	RegionPath* regionPath = regionStat->reachable_region_array;

	for( int i=0 ; i<regionStat->reachable_region_count ; i++, regionPath++ )
	{
		if( regionPath->land_region_stat_id == regionStatId2 )
			return regionPath->sea_region_id;
	}

	return 0;
}
//--------- End of function RegionArray::get_sea_path_region_id -------//


//--------- Begin of function RegionArray::nation_has_base_town -------//
//
// Return whether the given nation has a base town in the given region.
//
int RegionArray::nation_has_base_town(int regionId, int nationRecno)
{
	RegionStat* regionStat = region_array.region_stat_array;

	for( int i=1 ; i<=region_array.region_stat_count ; i++, regionStat++ )
	{
		if( regionStat->region_id != regionId )
			continue;

		return regionStat->base_town_nation_count_array[nationRecno-1] > 0;
	}

	return 0;
}
//--------- End of function RegionArray::nation_has_base_town -------//



