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

//Filename    : OTERRAIN.H
//Description : Header file of object TerrainRes
//Ownership   : Gilbert

#ifndef __OTERRAIN_H
#define __OTERRAIN_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//---------- Define terrain type id. ----------//

enum { TOTAL_TERRAIN_TYPE=4 };
enum { MAX_GEN_TERRAIN_TYPE=4 };		// map generator will not generate light dirt or above

enum TerrainTypeCode
{
	TERRAIN_OCEAN = 1,					// abbrev = 'S'
	TERRAIN_DARK_GRASS,					// 'G'
	TERRAIN_LIGHT_GRASS,					// 'F'
	TERRAIN_DARK_DIRT,					// 'D' - hill
};

enum SubTerrainMask
{
	BOTTOM_MASK = 1,											// abbrev = '-'
	MIDDLE_MASK = 2,											// '0'
	TOP_MASK = 4,												// '+'
	NOT_TOP_MASK = BOTTOM_MASK | MIDDLE_MASK,			// 'B'
	NOT_BOTTOM_MASK = MIDDLE_MASK | TOP_MASK,			// 'A'
	ALL_MASK = BOTTOM_MASK | MIDDLE_MASK | TOP_MASK	// '*'
};

enum TerrainFlag
{
	TERRAIN_FLAG_SNOW = 1,
};


enum { TERRAIN_TILE_WIDTH  = 64,
		 TERRAIN_TILE_HEIGHT = 64,
		 TERRAIN_TILE_X_MASK = 63,
		 TERRAIN_TILE_Y_MASK = 63,
	  };

//------------ Define struct TerrainRec ---------------//

struct TerrainRec
{
	enum { TYPE_CODE_LEN=2, FILE_NAME_LEN=8, BITMAP_PTR_LEN=4, PATTERN_ID_LEN=2 };

	char nw_type_code[TYPE_CODE_LEN];
	char ne_type_code[TYPE_CODE_LEN];
	char sw_type_code[TYPE_CODE_LEN];
	char se_type_code[TYPE_CODE_LEN];

	char extra_flag;
	char special_flag;
	char represent_type;
	char secondary_type;
	char pattern_id[PATTERN_ID_LEN];

	char file_name[FILE_NAME_LEN];
	char bitmap_ptr[BITMAP_PTR_LEN];
};

//------------- Define struct TerrainInfo --------------//

struct TerrainInfo
{
	char 	nw_type, nw_subtype;
	char 	ne_type, ne_subtype;
	char 	sw_type, sw_subtype;
	char 	se_type, se_subtype;

	char	average_type;
	char 	extra_flag;
	char	special_flag;
	char	secondary_type;
	char	pattern_id;

	int 	is_coast() { return average_type==TERRAIN_OCEAN && secondary_type>TERRAIN_OCEAN ||
							 average_type> TERRAIN_OCEAN && secondary_type==TERRAIN_OCEAN;  }

	char  alternative_count_with_extra;  	  // no. of alternative bitmaps of this terrain
	char  alternative_count_without_extra;   // no. of alternative bitmaps of this terrain
	unsigned char	flags;

	char* bitmap_ptr;
	int	anim_frames;
	char** anim_bitmap_ptr;

	char *get_bitmap(unsigned frameNo);
	int	can_snow()						{ return flags & TERRAIN_FLAG_SNOW; }
};

//------------ Define struct TerrainType ---------------//

struct TerrainType
{
	short	first_terrain_id;
	short	last_terrain_id;
	short min_height;
};

// field related to terrain pattern substitution
//----------- Define struct TerrainSubRec --------//

struct TerrainSubRec
{
	enum { SUB_NO_LEN=4, PATTERN_ID_LEN=2, DIRECTION_LEN=2, STEP_ID_LEN=2, SEC_ADJ_LEN=2};
	char sub_no[SUB_NO_LEN];
	char step_id[STEP_ID_LEN];
	char old_pattern_id[PATTERN_ID_LEN];
	char new_pattern_id[PATTERN_ID_LEN];
	char sec_adj[SEC_ADJ_LEN];
	char post_move[DIRECTION_LEN];
};

//---------- Define struct TerrainSubInfo --------//

struct TerrainSubInfo
{
	short sub_no;
	short	step_id;
	char	old_pattern_id;
	char	new_pattern_id;
	char	sec_adj;
	char	post_move;
	TerrainSubInfo *next_step;
};

//----------- Define struct TerrainAnimRec -------//

struct TerrainAnimRec
{
	enum { FILE_NAME_LEN=8, FRAME_NO_LEN=2, BITMAP_PTR_LEN=4 };
	char	base_file[FILE_NAME_LEN];
	char	frame_no[FRAME_NO_LEN];
	char	next_frame[FRAME_NO_LEN];
	char	filename[FILE_NAME_LEN];
	char	bitmap_ptr[BITMAP_PTR_LEN];
};

//----------- Define class TerrainRes ---------------//

class TerrainRes
{
public:
	short    	 terrain_count;
	TerrainInfo* terrain_info_array;
	char* 		 file_name_array;

	TerrainType	 terrain_type_array[TOTAL_TERRAIN_TYPE];
	short			 nw_type_min[TOTAL_TERRAIN_TYPE];
	short			 nw_type_max[TOTAL_TERRAIN_TYPE];

	//----- field related to terrain pattern substitution -----//
	short			 ter_sub_rec_count;
	short			 ter_sub_index_count;
	TerrainSubInfo* ter_sub_array;
	TerrainSubInfo** ter_sub_index;

	char	   	 init_flag;
	ResourceDb	 res_bitmap;
	ResourceDb   anm_bitmap;

public:
	TerrainRes();

	void 		 	 init();
	void 		 	 deinit();

	int 			 get_tera_type_id(char* teraTypeCode);
	char*			 get_map_tile(int terrainId);

	int 			 scan(int nwType, int nwSubType, int neType, int neSubType,
							int swType, int swSubType, int seType, int seSubType,
							int firstInstance=0, int includeExtra=0, int special=0);

	int			 scan(int primaryType, int secondaryType, int patternId,
							int firstInstance=0, int includeExtra=0, int special=0);

	//---- function related to terrain pattern substitution ----//

	int			 search_pattern(int nwPatternId, TerrainSubInfo **resultArray,
										 int maxResult);

	//---------- define code conversion function -------//

	static TerrainTypeCode terrain_code(char);
	static SubTerrainMask terrain_mask(char);
	static int terrain_height(int height, int* =NULL);
	static short min_height(TerrainTypeCode, SubTerrainMask = BOTTOM_MASK);
	static short max_height(TerrainTypeCode, SubTerrainMask = TOP_MASK);

	//-------------------------------------------------//

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		TerrainInfo* operator[](int terrainId);
	#else
		TerrainInfo* operator[](int terrainId) 	{ return terrain_info_array+terrainId-1; }
	#endif

private:
	void 		    load_info();
	void			 load_sub_info();
	void			 load_anim_info();
};

extern TerrainRes terrain_res;

//----------------------------------------------------//

#endif
