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

// Filename    : OREGIONS.H
// Description : Header file of RegionStat

#ifndef __OREGIONS_H
#define __OREGIONS_H

#ifndef __OFIRMID_H
#include <OFIRMID.h>
#endif

//-------- Define constant ---------//

#define MIN_STAT_REGION_SIZE 	100		// only regions with size >= 100 locations are included in the region_stat_array

#define MAX_REACHABLE_REGION_PER_STAT	10		// maximum reachable regions to be kept in RegionStat

//------- Define struct RegionPath -------//

#pragma pack(1)
struct RegionPath
{
	uint8_t		sea_region_id;				// region id. of the sea route
	uint8_t		land_region_stat_id;
};
#pragma pack()

//------- Define class RegionStat --------//

#pragma pack(1)
class RegionStat
{
public:
	uint8_t		region_id;				// sorted in the order of region size

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
#pragma pack()

//--------------------------------------------//

#endif
