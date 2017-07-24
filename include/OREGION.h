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

// Filename    : OREGION.H
// Description : Header file of RegionArray
// Owner       : Gilbert

#ifndef __OREGION_H
#define __OREGION_H

#ifndef DEBUG
#include <OREGIONS.h>
#endif

#ifndef __ALL_H
#include <ALL.h>
#endif

//---------- define constant ---------//

#define MAX_REGION 	255

//------- Define enum RegionType -------//

enum RegionType
{
	REGION_INPASSABLE,
	REGION_LAND,
	REGION_SEA,
};

//------- Define struct RegionInfo ----------//

#pragma pack(1)
struct RegionInfo
{
	uint8_t				region_id;
	uint8_t				region_stat_id;

	RegionType		region_type;
	int				adj_offset_bit;
	int				region_size;

	short				center_x, center_y;		// the center locatino of the region
};
#pragma pack()

//------- Define class RegionArray ----------//

class RegionStat;

#pragma pack(1)
class RegionArray
{
public:
	int				init_flag;

	RegionInfo*		region_info_array;
	int				region_info_count;

	RegionStat*		region_stat_array;
	int				region_stat_count;

	unsigned char *connect_bits;
	uint8_t		 		region_sorted_array[MAX_REGION]; 	// an array of region id. sorted by the region size

public:
	RegionArray();
	~RegionArray();

	void	init(int maxRegion);
	void	deinit();

	void	next_day();

	void	inc_size(int reg);
	void	set_region(int reg, RegionType);
	void	set_adjacent(int reg1, int reg2);
	int	is_adjacent(int reg1, int reg2);
	void	sort_region();

	void 	init_region_stat();
	void 	update_region_stat();

	int 	get_sea_path_region_id(int regionId1, int regionId2);
	int	nation_has_base_town(int regionId, int nationRecno);

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

	//--------------------------------------//

	#ifdef DEBUG

	RegionStat* get_region_stat(int regionId);
	RegionStat* get_region_stat2(int regionStatId);
	RegionInfo* get_sorted_region(int recNo);

	RegionType region_type(int region);
	RegionInfo *operator[](int region);

	#else

	RegionInfo *operator[](int region)
					{ return region_info_array+region-1; }

	RegionStat* get_region_stat(int regionId)
					{ return region_stat_array + region_info_array[regionId-1].region_stat_id - 1; }

	RegionStat* get_region_stat2(int regionStatId)
					{ return region_stat_array+regionStatId-1; }

	RegionType  region_type(int region)
					{ return region_info_array[region-1].region_type; }

	RegionInfo* get_sorted_region(int recNo)
					{ return operator[]( region_sorted_array[recNo-1] ); }
	#endif
};
#pragma pack()

extern RegionArray region_array;

//------------------------------------------//

#endif
