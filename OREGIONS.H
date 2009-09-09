// Filename    : OREGIONS.H
// Description : Header file of RegionStat

#ifndef __OREGIONS_H
#define __OREGIONS_H

#ifndef __OFIRMID_H
#include <OFIRMID.H>
#endif

//-------- Define constant ---------//

#define MIN_STAT_REGION_SIZE 	100		// only regions with size >= 100 locations are included in the region_stat_array

#define MAX_REACHABLE_REGION_PER_STAT	10		// maximum reachable regions to be kept in RegionStat

//------- Define struct RegionPath -------//

struct RegionPath
{
	BYTE		sea_region_id;				// region id. of the sea route
	BYTE		land_region_stat_id;
};

//------- Define class RegionStat --------//

class RegionStat
{
public:
	BYTE		region_id;				// sorted in the order of region size

	char		nation_is_present_array[MAX_NATION];
	char		nation_presence_count;

	short		firm_type_count_array[MAX_FIRM_TYPE];
	short		firm_nation_count_array[MAX_NATION];
	short		camp_nation_count_array[MAX_NATION];
	short		mine_nation_count_array[MAX_NATION];
	short		harbor_nation_count_array[MAX_NATION];
	short		total_firm_count;

	short		town_nation_count_array[MAX_NATION];
	short		base_town_nation_count_array[MAX_NATION];
	short		independent_town_count;
	short		total_town_count;

	short		nation_population_array[MAX_NATION];
	short		nation_jobless_population_array[MAX_NATION];

	short		unit_nation_count_array[MAX_NATION];
	short		independent_unit_count;		// either rebels or monsters
	short		total_unit_count;

	short		site_count;
	short    raw_count;

	RegionPath  reachable_region_array[MAX_REACHABLE_REGION_PER_STAT];
	char			reachable_region_count;

public:
	void		init();
	void		update_stat();
};

//--------------------------------------------//

#endif