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

//Filename    : OTERRAIN.CPP
//Description : Terrain resource object
//Ownership   : Gilbert

#include <OIMGRES.h>
#include <COLOR.h>
#include <OSYS.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OTERRAIN.h>
#include <OCONFIG.h>

//---------- #define constant ------------//

//#define TERRAIN_DB   "TERRAIN"
//#define TERRAIN_SUB_DB "TERSUB"
//#define TERRAIN_ANIM_DB "TERANM"
#define MAX_TERRAIN_ANIM_FRAME 16

//-------- define base terrain colors --------//
//
// Base terrain types:
//
// -Ocean
// -Dark Grass
// -Light Grass
// -Dark Dirt
// -Light Dirt
//
//--------------------------------------------//

static int terrain_type_color_array[TOTAL_TERRAIN_TYPE] =		// the color of each terrain type on the small map
{
	0x20,
	0x0A,
	0x0D,
	0x2D,
};

static const char* map_tile_name_array[TOTAL_TERRAIN_TYPE] =		// the color of each terrain type on the small map
{
	"TERA_S",
	"TERA_DG",
	"TERA_LG",
	"TERA_D",
};

static char* map_tile_ptr_array[TOTAL_TERRAIN_TYPE];

static int terrain_type_min_height_array[TOTAL_TERRAIN_TYPE][3] =		// the color of each terrain type on the small map
{
//	{   0,   5,  55 },             // S-, S0, S+
//	{  60,  65, 125 },             // G-, G0, G+
//	{ 130, 135, 195 },             // F-, F0, F+
//	{ 200, 205, 250 },             // D-, D0, D+

	{   0,  20,  40 },             // S-, S0, S+
	{  60,  80, 110 },             // G-, G0, G+
	{ 130, 150, 180 },             // F-, F0, F+
	{ 200, 215, 240 },             // D-, D0, D+
};


// ------ Begin of function TerrainInfo::get_bitmap -------//
char *TerrainInfo::get_bitmap(unsigned frameNo)
{
	return frameNo && anim_frames ? anim_bitmap_ptr[frameNo%(unsigned)anim_frames] : NULL;
}
// ------ end of function TerrainRes::terrain_code -------//

// ------ Begin of function TerrainRes::terrain_code -------//
TerrainTypeCode TerrainRes::terrain_code(char tcCode)
{
	switch(tcCode)
	{
	case 'S':
		return TERRAIN_OCEAN;
	case 'G':
		return TERRAIN_DARK_GRASS;
	case 'F':
		return TERRAIN_LIGHT_GRASS;
	case 'D':
		return TERRAIN_DARK_DIRT;
	default:
		err_here();
		return TERRAIN_OCEAN;
	}
}
// ------ End of static function TerrainRes::terrain_code -------//

// ------ Begin of static function TerrainRes::terrain_mask -------//
SubTerrainMask TerrainRes::terrain_mask(char subtc)
{
	switch(subtc)
	{
	case '0':
		return MIDDLE_MASK;
	case '+':
		return TOP_MASK;
	case '-':
		return BOTTOM_MASK;
	case 'A':
		return NOT_BOTTOM_MASK;
	case 'B':
		return NOT_TOP_MASK;
	case '*':
		return ALL_MASK;
	default:
		err_here();
		return MIDDLE_MASK;
	}
}
// ------ End of function TerrainRes::terrain_mask -------//


// ------ Begin of function TerrainRes::terrain_height -------//
int TerrainRes::terrain_height(int height, int *subPtr)
{
	char tc, subtc;
	for( tc = TOTAL_TERRAIN_TYPE -1; tc >= 0; --tc)
	{
		if( height >= terrain_type_min_height_array[tc][0])
		{
			for( subtc = 2; subPtr && subtc >= 0; --subtc)
			{
				if(height >= terrain_type_min_height_array[tc][subtc])
				{
					*subPtr = 1 << subtc;
					break;
				}
			}
			err_when(subtc < 0);
			return tc+1;
		}
	}
	err_here();
	return 0;
}
// ------ End of function TerrainRes::terrain_height -------//

// ------ Begin of function TerrainRes::min_height ------//
short TerrainRes::min_height(TerrainTypeCode tc, SubTerrainMask subtc)
{
	int s, j;
	for(s=0,j=1 ; s < 3 && !(subtc & j); ++s, j+=j);
	return terrain_type_min_height_array[tc-1][s];
}
// ------ End of function TerrainRes::min_height ------//


// ------ Begin of function TerrainRes::max_height ------//
short TerrainRes::max_height(TerrainTypeCode tc, SubTerrainMask subtc)
{
	if( subtc & TOP_MASK )
	{
		if( tc < (TerrainTypeCode)TOTAL_TERRAIN_TYPE )
			return terrain_type_min_height_array[tc-1+1][0]-1;
		else
			return 255;
	}

	int s, j;
	for(s=2,j=2 ; s >= 0 && !(subtc & j); --s, j>>=1);
	return terrain_type_min_height_array[tc-1][s]-1;
}
// ------ End of function TerrainRes::max_height ------//

//------- Begin of function TerrainRes::TerrainRes -----------//

TerrainRes::TerrainRes()
{
	init_flag=0;
}
//--------- End of function TerrainRes::TerrainRes -----------//


//---------- Begin of function TerrainRes::init -----------//
//
// This function must be called after a map is generated.
//
void TerrainRes::init()
{
	deinit();

	//----- open firm material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	// str += "I_TERAIN.RES";
	str += "I_TERN";
	str += config.terrain_set;
	str += ".RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_info();
	load_sub_info();

	str = DIR_RES;
	// str += "I_TERANM.RES";
	str += "I_TERA";
	str += config.terrain_set;
	str += ".RES";

	anm_bitmap.init_imported(str,1);

	load_anim_info();

	//-------- init map_tile_ptr_array --------//

	int i, terrainId;

	for( i=0 ; i<TOTAL_TERRAIN_TYPE ; i++ )
	{
		if( map_tile_name_array[i] )
			// ###### begin Gilbert 9/9 ########//
			// map_tile_ptr_array[i] = image_spict.get_ptr(map_tile_name_array[i]) + sizeof(short)*2;
			map_tile_ptr_array[i] = image_tpict.get_ptr(map_tile_name_array[i]) + sizeof(short)*2;
			// ###### end Gilbert 9/9 ########//
	}

	//-------- init terrain_type_array ---------//

	for( i=1 ; i<=TOTAL_TERRAIN_TYPE ; i++ )
	{
		terrainId = scan( i, ALL_MASK, i, ALL_MASK, i, ALL_MASK, i, ALL_MASK, 1 );	// scan for terrain of all four types. 1-take the first instance of the terrain bitmap that matches the given terrain types

		if(terrainId)
		{
			terrain_type_array[i-1].first_terrain_id = terrainId;
			terrain_type_array[i-1].last_terrain_id  = terrainId + terrain_res[terrainId]->alternative_count_with_extra;
			terrain_type_array[i-1].min_height		  = terrain_type_min_height_array[i-1][0];
		}
		else
		{
			// BUGHERE: should not reach this point after all terrain type are complete
			terrain_type_array[i-1].first_terrain_id = 0;
			terrain_type_array[i-1].last_terrain_id  = 0;
			terrain_type_array[i-1].min_height		  = terrain_type_min_height_array[i-1][0];
		}
	}

	init_flag=1;
}
//---------- End of function TerrainRes::init -----------//


//---------- Begin of function TerrainRes::deinit -----------//
void TerrainRes::deinit()
{
	if( init_flag )
	{
		for( int i = terrain_count-1; i >= 0; --i)
		{
			if( terrain_info_array[i].anim_frames > 0 )
				mem_del(terrain_info_array[i].anim_bitmap_ptr);
		}
		mem_del(terrain_info_array);
		mem_del(ter_sub_array);
		mem_del(ter_sub_index);
		init_flag=0;
	}
}
//---------- End of function TerrainRes::deinit -----------//


//------- Begin of function TerrainRes::load_info -------//
//
// Read in information of TERRAIN.DBF into memory array
//
void TerrainRes::load_info()
{
	TerrainRec  	*terrainRec;
	TerrainInfo 	*terrainInfo = NULL;
	int      	 	i;
	uint32_t			bitmapOffset;

	//---- read in terrain count and initialize terrain info array ----//

	// Database *dbTerrain = game_set.open_db(TERRAIN_DB);
	String terrainDbName;
	terrainDbName = DIR_RES;
	terrainDbName += "TERRAIN";
	terrainDbName += config.terrain_set;
	terrainDbName += ".RES";

	Database terrainDbObj(terrainDbName, 1);
	Database *dbTerrain = &terrainDbObj;

	terrain_count      = (short) dbTerrain->rec_count();
	terrain_info_array = (TerrainInfo*) mem_add( sizeof(TerrainInfo)*terrain_count );

	memset( terrain_info_array, 0, sizeof(TerrainInfo) * terrain_count );
	file_name_array = (char *) mem_add(terrainRec->FILE_NAME_LEN*terrain_count);

	// ------ initial nw_type_index -------//
	for(i = 0; i < TOTAL_TERRAIN_TYPE; ++i)
	{
		nw_type_min[i] = 0;
		nw_type_max[i] = 0;
	}

	//---------- read in TERRAIN.DBF ---------//

	char	firstNw=0, firstNe, firstSw, firstSe;
	char	firstNwSub=0, firstNeSub, firstSwSub, firstSeSub, firstSpFlag;
	int	firstId;

	for( i=0 ; i<terrain_count ; i++ )
	{
		terrainRec  = (TerrainRec*) dbTerrain->read(i+1);
		terrainInfo = terrain_info_array+i;

		memcpy(&file_name_array[i*terrainRec->FILE_NAME_LEN], terrainRec->file_name,
			terrainRec->FILE_NAME_LEN);
		terrainInfo->nw_type = terrain_code(terrainRec->nw_type_code[0]);
		terrainInfo->nw_subtype = terrain_mask(terrainRec->nw_type_code[1]);
		terrainInfo->ne_type = terrain_code(terrainRec->ne_type_code[0]);
		terrainInfo->ne_subtype = terrain_mask(terrainRec->ne_type_code[1]);
		terrainInfo->sw_type = terrain_code(terrainRec->sw_type_code[0]);
		terrainInfo->sw_subtype = terrain_mask(terrainRec->sw_type_code[1]);
		terrainInfo->se_type = terrain_code(terrainRec->se_type_code[0]);
		terrainInfo->se_subtype = terrain_mask(terrainRec->se_type_code[1]);

		terrainInfo->average_type = terrain_code(terrainRec->represent_type);
		// ######## begin Gilbert 12/2 #######//
		switch(terrainRec->extra_flag)
		{
		case 0:			// fall through
		case ' ':		// fall through
		case 'N':		// fall through
		case 'n':
			terrainInfo->extra_flag = 0;
			break;
		default:
			terrainInfo->extra_flag = 1;
		}
		terrainInfo->special_flag = terrainRec->special_flag==' ' ? 0 : terrainRec->special_flag;
		// ######## end Gilbert 12/2 #######//

		terrainInfo->secondary_type = terrain_code(terrainRec->secondary_type);
		terrainInfo->pattern_id = misc.atoi(terrainRec->pattern_id, terrainRec->PATTERN_ID_LEN);

		//------ set alternative_count ---------//

      if( firstNw == terrainInfo->nw_type &&
			 firstNwSub == terrainInfo->nw_subtype &&
          firstNe == terrainInfo->ne_type &&
			 firstNeSub == terrainInfo->ne_subtype &&
          firstSw == terrainInfo->sw_type &&
			 firstSwSub == terrainInfo->sw_subtype &&
          firstSe == terrainInfo->se_type &&
			 firstSeSub == terrainInfo->se_subtype &&
			 firstSpFlag == terrainInfo->special_flag )
		{
			terrain_info_array[firstId-1].alternative_count_with_extra++;

			if( !terrainInfo->extra_flag )
				terrain_info_array[firstId-1].alternative_count_without_extra++;

			// ----- record firstId - terrain_id -------//
			terrainInfo->alternative_count_with_extra = firstId -1 -i;
			terrainInfo->alternative_count_without_extra = firstId -1 -i;
		}
      else
      {
   		// build index on nw_type
			if( firstNw != terrainInfo->nw_type)
			{
				// --------- mark end of previous nw_type group ---------//
				if(firstNw > 0)
					nw_type_max[firstNw-1] = i;

				// --------- mark start of new nw_type group -----------//
				nw_type_min[terrainInfo->nw_type-1] = i+1;
			}

	      firstNw = terrainInfo->nw_type;
         firstNe = terrainInfo->ne_type;
         firstSw = terrainInfo->sw_type;
         firstSe = terrainInfo->se_type;
         firstNwSub = terrainInfo->nw_subtype;
         firstNeSub = terrainInfo->ne_subtype;
         firstSwSub = terrainInfo->sw_subtype;
         firstSeSub = terrainInfo->se_subtype;
			firstSpFlag = terrainInfo->special_flag;
         firstId = i+1;
      }

		//---- get the bitmap pointer of the terrain icon in res_icon ----//

		memcpy( &bitmapOffset, terrainRec->bitmap_ptr, sizeof(uint32_t) );

		terrainInfo->bitmap_ptr = res_bitmap.read_imported(bitmapOffset);

		terrainInfo->anim_frames = 0;
		terrainInfo->anim_bitmap_ptr = NULL;

		terrainInfo->flags = 0;
		if( terrainInfo->average_type != TERRAIN_OCEAN &&
			terrainInfo->secondary_type != TERRAIN_OCEAN)
		{
			terrainInfo->flags |= TERRAIN_FLAG_SNOW;
		}

	}

	// ------- mark end of last nw_type group -------//
	if(terrainInfo && terrainInfo->nw_type > 0)
		nw_type_max[terrainInfo->nw_type-1] = i;

}
//--------- End of function TerrainRes::load_info ---------//


//---- Begin of function TerrainRes::scan ------//
//
// Scan for a terrain with the specific base terrain types.
//
// <int> nwType, neType, swType, seType - the base terrain types to look for
// <int> nwSubType, neSubType, swSubType, seSubType - subTerrainMask
// [int] firstInstance  - whether return the first instance of the terrain instead of a random instance
//								  (default: 0)
// [int] includeExtra   - whether include extra terrain type in the scanning
//								  (default: 0)
// [int] special			- whether search special_flag
//								  (default: 0)
//
// return : <int> >0 - the id. of the terrain
//					  ==0 - no terrain of the specific base types found
//
int TerrainRes::scan(int nwType, int nwSubType, int neType, int neSubType,
	int swType, int swSubType, int seType, int seSubType, 
	int firstInstance, int includeExtra, int special)
{
	int terrainId = nw_type_min[nwType-1];
	int terrainIdMax = nw_type_max[nwType-1];
	if(terrainId <= 0 || terrainIdMax <= 0)
		return 0;

	TerrainInfo* terrainInfo = terrain_info_array+terrainId-1;
	//	int firstTerrainId=0, instanceCount=0;

	for( ; terrainId<=terrainIdMax; terrainId++, terrainInfo++ )
	{
		if( terrainInfo->nw_type == nwType &&
			 terrainInfo->nw_subtype & nwSubType &&
			 terrainInfo->ne_type == neType &&
			 terrainInfo->ne_subtype & neSubType &&
			 terrainInfo->sw_type == swType &&
			 terrainInfo->sw_subtype & swSubType &&
			 terrainInfo->se_type == seType &&
			 terrainInfo->se_subtype & seSubType &&
			 terrainInfo->special_flag == special)
		{
			if( firstInstance )
				return terrainId;
			else
			{
				if( includeExtra )
				{
					err_when(terrainInfo->alternative_count_with_extra < 0);
					return terrainId + misc.random(terrainInfo->alternative_count_with_extra+1);
				}
				else
				{
					err_when(terrainInfo->alternative_count_without_extra < 0);
					return terrainId + misc.random(terrainInfo->alternative_count_without_extra+1);
				}
			}
		}
		else
		{
			//------ skip tiles of the same type and special_flag
			err_when(terrainInfo->alternative_count_with_extra < 0);
			terrainId += terrainInfo->alternative_count_with_extra;
			terrainInfo += terrainInfo->alternative_count_with_extra;
		}
	}

	return 0;
}
//------ End of function TerrainRes::scan ------//

//---- Begin of function TerrainRes::scan ------//
//
// Scan for a terrain with the specific base terrain types.
//
// <int> primaryType    - TerrainTypeCode
// <int> secondaryType  - TerrainTypeCode
// <int>	patternId      - pattern_id in TerrainInfo
// [int] firstInstance  - whether return the first instance of the terrain instead of a random instance
//								  (default: 0)
// [int] includeExtra   - whether include extra terrain type in the scanning
//								  (default: 0)
// [int] special			- whether search special_flag
//								  (default: 0)
//
// return : <int> >0 - the id. of the terrain
//					  ==0 - no terrain of the specific base types found
//
int TerrainRes::scan(int primaryType, int secondaryType,	int patternId, 
							int firstInstance, int includeExtra, int special)
{
	// if patternId is zero, that means it requires a pure terrain
	if( patternId == 0)
		secondaryType = primaryType;

	// ----------- search 1, search the lower type -------//
	int terrainId = nw_type_min[MIN(primaryType, secondaryType)-1];
	int terrainIdMax = nw_type_max[MIN(primaryType, secondaryType)-1];
	TerrainInfo* terrainInfo = terrain_info_array+terrainId-1;
	// int firstTerrainId=0, instanceCount=0;

	for( ; terrainId<=terrainIdMax; terrainId++, terrainInfo++ )
	{
		if( ( (terrainInfo->average_type == primaryType &&
			    terrainInfo->secondary_type == secondaryType) ||
				(terrainInfo->average_type == secondaryType &&
				 terrainInfo->secondary_type == primaryType)) &&
			 terrainInfo->pattern_id == patternId &&
			 terrainInfo->special_flag == special)
		{
			if( firstInstance )
				return terrainId;
			else
			{
				if( includeExtra )
				{
					err_when(terrainInfo->alternative_count_with_extra < 0);
					return terrainId + misc.random(terrainInfo->alternative_count_with_extra+1);
				}
				else
				{
					err_when(terrainInfo->alternative_count_without_extra < 0);
					return terrainId + misc.random(terrainInfo->alternative_count_without_extra+1);
				}
			}
		}
		else
		{
			//------ skip tiles of the same type and special_flag
			err_when(terrainInfo->alternative_count_with_extra < 0);
			terrainId += terrainInfo->alternative_count_with_extra;
			terrainInfo += terrainInfo->alternative_count_with_extra;
		}
	}

	// ----------- search 2, search the higher type ---------//
	if( primaryType != secondaryType)
	{
		terrainId = nw_type_min[MAX(primaryType, secondaryType)-1];
		terrainIdMax = nw_type_max[MAX(primaryType, secondaryType)-1];
		terrainInfo = terrain_info_array+terrainId-1;
		// int firstTerrainId=0, instanceCount=0;

		for( ; terrainId<=terrainIdMax; terrainId++, terrainInfo++ )
		{
			if( ( (terrainInfo->average_type == primaryType &&
					 terrainInfo->secondary_type == secondaryType) ||
					(terrainInfo->average_type == secondaryType &&
					 terrainInfo->secondary_type == primaryType)) &&
				 terrainInfo->pattern_id == patternId &&
				 terrainInfo->special_flag == special)
			{
				if( firstInstance )
					return terrainId;
				else
				{
					if( includeExtra )
					{
						err_when(terrainInfo->alternative_count_with_extra < 0);
						return terrainId + misc.random(terrainInfo->alternative_count_with_extra+1);
					}
					else
					{
						err_when(terrainInfo->alternative_count_without_extra < 0);
						return terrainId + misc.random(terrainInfo->alternative_count_without_extra+1);
					}
				}
			}
			else
			{
				//------ skip tiles of the same type and special_flag
				err_when(terrainInfo->alternative_count_with_extra < 0);
				terrainId += terrainInfo->alternative_count_with_extra;
				terrainInfo += terrainInfo->alternative_count_with_extra;
			}
		}
	}

	return 0;

}
//------ End of function TerrainRes::scan ------//


//------------ Begin of function TerrainRes::search_pattern -----------//
//
// search first (northwest pattern id) to match a set of
// substitution pattern, to be stored in resultArray
// return the no. of matches
//
// <int>						nwPatternId		pattern_id of a terrain, 
//													obtained by terrain_res[terrain_id]->pattern_id
// <TerrainSubInfo *>	resultArray		an array to be filled with matches
// <int>						maxResult		the no. of elements of the resultArray
// <int>						return			no. of matches	
//
int TerrainRes::search_pattern(int nwPatternId, TerrainSubInfo **resultArray,
	int maxResult)
{
	err_when(!init_flag);

	if(!resultArray || !maxResult)
		return 0;

	int occur = 0;
	for( int i = 0; i < ter_sub_index_count; ++i)
	{
		if( ter_sub_index[i] && nwPatternId == ter_sub_index[i]->old_pattern_id)
		{
			resultArray[occur++] = ter_sub_index[i];
			if( occur >= maxResult)
				break;
		}
	}
	return occur;
}
//------------ End of function TerrainRes::search_pattern -----------//


//------------ Begin of function TerrainRes::load_sub_info -----------//
void TerrainRes::load_sub_info()
{
	TerrainSubRec  	*terrainSubRec;
	TerrainSubInfo 	*terrainSubInfo = NULL;
	int      	 	i;

	//---- read in terrain count and initialize terrain info array ----//

	Database terSubDbObj(DIR_RES"TERSUB.RES", 1);
	//Database *dbTerrain = game_set.open_db(TERRAIN_SUB_DB);
	Database *dbTerrain = &terSubDbObj;

	ter_sub_rec_count = (short) dbTerrain->rec_count();
	ter_sub_array = (TerrainSubInfo*) mem_add( sizeof(TerrainSubInfo)*ter_sub_rec_count );

	memset( ter_sub_array, 0, sizeof(TerrainSubInfo) * ter_sub_rec_count );

	//---------- read in TERSUB.DBF ---------//
	short	maxSubNo = 0;

	for( i=0 ; i < ter_sub_rec_count ; i++ )
	{
		terrainSubRec  = (TerrainSubRec*) dbTerrain->read(i+1);
		terrainSubInfo = ter_sub_array+i;

		terrainSubInfo->sub_no = misc.atoi(terrainSubRec->sub_no, terrainSubRec->SUB_NO_LEN);
		terrainSubInfo->step_id = misc.atoi(terrainSubRec->step_id, terrainSubRec->STEP_ID_LEN);
		terrainSubInfo->old_pattern_id = misc.atoi(terrainSubRec->old_pattern_id, terrainSubRec->PATTERN_ID_LEN);
		terrainSubInfo->new_pattern_id = misc.atoi(terrainSubRec->new_pattern_id, terrainSubRec->PATTERN_ID_LEN);

		// sec_adj is useful when a pure type is changing to boundary type.
		// eg. a GG square can be changed to SG or GS sqare
		terrainSubInfo->sec_adj = misc.atoi(terrainSubRec->sec_adj, terrainSubRec->SEC_ADJ_LEN);
		switch(terrainSubRec->post_move[0])
		{
		case 'N':
			switch(terrainSubRec->post_move[1])
			{
			case 'E':
				terrainSubInfo->post_move = 2;
				break;
			case 'W':
				terrainSubInfo->post_move = 8;
				break;
			default:
				terrainSubInfo->post_move = 1;
			}
			break;
		case 'S':
			switch(terrainSubRec->post_move[1])
			{
			case 'E':
				terrainSubInfo->post_move = 4;
				break;
			case 'W':
				terrainSubInfo->post_move = 6;
				break;
			default:
				terrainSubInfo->post_move = 5;
			}
			break;
		case 'E':
			terrainSubInfo->post_move = 3;
			break;
		case 'W':
			terrainSubInfo->post_move = 7;
			break;
		case 'X':
			terrainSubInfo->post_move = 0;
			break;
		default:
			err_here();
		}

		terrainSubInfo->next_step = NULL;

		if( terrainSubInfo->sub_no > maxSubNo )
			maxSubNo = terrainSubInfo->sub_no;
	}

	// ------- build ter_sub_index ------//
	ter_sub_index_count = maxSubNo;
	ter_sub_index = (TerrainSubInfo **) mem_add(sizeof(TerrainSubInfo *) * ter_sub_index_count);
	memset(ter_sub_index, 0, sizeof(TerrainSubInfo *) * ter_sub_index_count);

	TerrainSubInfo *lastTerrainSubInfo = NULL;
	for( i=0 ; i < ter_sub_rec_count ; i++ )
	{
		terrainSubInfo = ter_sub_array+i;
		if( terrainSubInfo->step_id == 1)
		{
			err_when(ter_sub_index[terrainSubInfo->sub_no-1] != NULL);
			ter_sub_index[terrainSubInfo->sub_no-1] = terrainSubInfo;
		}
		else
		{
			// link from the next_step pointer of previous step
			// search the previous record first
			if( lastTerrainSubInfo && lastTerrainSubInfo->sub_no == terrainSubInfo->sub_no
				&& lastTerrainSubInfo->step_id == terrainSubInfo->step_id -1)
			{
				lastTerrainSubInfo->next_step = terrainSubInfo;
			}
			else
			{
				// search from the array
				int j;
				for(j = 0; j < ter_sub_rec_count; j++)
				{
					if(ter_sub_array[j].sub_no == terrainSubInfo->sub_no &&
						ter_sub_array[j].step_id == terrainSubInfo->step_id-1)
					{
						ter_sub_array[j].next_step = terrainSubInfo;
						break;
					}
				}
				err_when(j >= ter_sub_rec_count);	// not found
			}
		}
		lastTerrainSubInfo = terrainSubInfo;
	}
}
//------------ End of function TerrainRes::load_sub_info -----------//


//------------ Begin of function TerrainRes::load_anim_info -------//
void TerrainRes::load_anim_info()
{
	TerrainAnimRec	terrainAnimRec, lastAnimRec;
	int i;
	uint32_t bitmapOffset;

	String terAnimDbName(DIR_RES);
	terAnimDbName += "TERANM";
	terAnimDbName += config.terrain_set;
	terAnimDbName += ".RES";
	Database terAnimDbObj(terAnimDbName, 1);
	// Database *dbTerAnim = game_set.open_db(TERRAIN_ANIM_DB);
	Database *dbTerAnim = &terAnimDbObj;

	int count = dbTerAnim->rec_count();

	int	animFrameCount = 0;
	char *animFrameBitmap[MAX_TERRAIN_ANIM_FRAME];
	memset(lastAnimRec.filename, ' ', lastAnimRec.FILE_NAME_LEN);
	memset(lastAnimRec.base_file, ' ', lastAnimRec.FILE_NAME_LEN);
	
	//---------- read in TERANM.DBF -------//
//	int j,k,l;
	int l;
	for( i = 0; i < count; i++)
	{
		char* bitmapPtr;
		terrainAnimRec = *(TerrainAnimRec *)(dbTerAnim->read(i+1));
		
		memcpy( &bitmapOffset, terrainAnimRec.bitmap_ptr, sizeof(uint32_t) );
		bitmapPtr = anm_bitmap.read_imported(bitmapOffset);

		if( memcmp(terrainAnimRec.base_file, lastAnimRec.base_file, terrainAnimRec.FILE_NAME_LEN))
		{
			// string not equal
			if( lastAnimRec.filename[0] != ' ' && animFrameCount > 0)
			{
				// replace terrainInfo->anim_frames and anim_frame_ptr
				// where the bitmap filename are the same
				int replaceCount = 0;
				for(l = 0; l < terrain_count; ++l)
				{
					if( memcmp(&file_name_array[terrainAnimRec.FILE_NAME_LEN *l],
						lastAnimRec.base_file, terrainAnimRec.FILE_NAME_LEN) == 0)
					{
						TerrainInfo *terrainInfo = terrain_info_array+l;
						err_when(terrainInfo->anim_frames > 0);
						terrainInfo->anim_frames = animFrameCount;
						terrainInfo->anim_bitmap_ptr = (char **) mem_add(
							sizeof(char *)*animFrameCount);
						memcpy(terrainInfo->anim_bitmap_ptr, animFrameBitmap,
							sizeof(char *)*animFrameCount);
						replaceCount++;
					}
				}
				/*
				for(int special = 0; special <= 1; ++special)
				{
					j = scan( terrain_code(lastAnimRec.average_type),
						terrain_code(lastAnimRec.secondary_type),
						misc.atoi(lastAnimRec.pattern_id, lastAnimRec.PATTERN_ID_LEN), 1,0,special);
					if( j > 0)
					{
						k = terrain_info_array[j-1].alternative_count_with_extra;
						for(l = j; l <= j+k; ++l)
						{
							if( memcmp(&file_name_array[terrainAnimRec.FILE_NAME_LEN *(l-1)],
								lastAnimRec.base_file, terrainAnimRec.FILE_NAME_LEN) == 0)
							{
								TerrainInfo *terrainInfo = terrain_info_array+j-1;
								err_when(terrainInfo->anim_frames > 0);
								terrainInfo->anim_frames = animFrameCount;
								terrainInfo->anim_bitmap_ptr = (char **) mem_add(
									sizeof(char *)*animFrameCount);
								memcpy(terrainInfo->anim_bitmap_ptr, animFrameBitmap,
									sizeof(char *)*animFrameCount);
								replaceCount++;
							}
						}
					}
				}
				*/

				err_when( replaceCount == 0);
			}

			lastAnimRec = terrainAnimRec;
			animFrameCount = 0;
			memset(animFrameBitmap, 0, sizeof(animFrameBitmap));
		}
		animFrameCount++;
		animFrameBitmap[misc.atoi(terrainAnimRec.frame_no, terrainAnimRec.FRAME_NO_LEN)-1]
			= bitmapPtr;
	}

	if( lastAnimRec.filename[0] != ' ' && animFrameCount > 0)
	{
		// replace terrainInfo->anim_frames and anim_frame_ptr
		// where the bitmap filename are the same
		int replaceCount = 0;
		for(l = 0; l < terrain_count; ++l)
		{
			if( memcmp(&file_name_array[terrainAnimRec.FILE_NAME_LEN *(l)],
				lastAnimRec.base_file, terrainAnimRec.FILE_NAME_LEN) == 0)
			{
				TerrainInfo *terrainInfo = terrain_info_array+l;
				err_when(terrainInfo->anim_frames > 0);
				terrainInfo->anim_frames = animFrameCount;
				terrainInfo->anim_bitmap_ptr = (char **) mem_add(
					sizeof(char *)*animFrameCount);
				memcpy(terrainInfo->anim_bitmap_ptr, animFrameBitmap,
					sizeof(char *)*animFrameCount);
				replaceCount++;
			}
		}
		/*
		for(int special = 0; special <= 1; ++special)
		{
			j = scan( terrain_code(lastAnimRec.average_type),
				terrain_code(lastAnimRec.secondary_type),
				misc.atoi(lastAnimRec.pattern_id, lastAnimRec.PATTERN_ID_LEN), 1,0,special);
			if( j > 0)
			{
				k = terrain_info_array[j-1].alternative_count_with_extra;
				for(l = j; l <= j+k; ++l)
				{
					if( memcmp(&file_name_array[terrainAnimRec.FILE_NAME_LEN *(l-1)],
						lastAnimRec.base_file, terrainAnimRec.FILE_NAME_LEN) == 0)
					{
						TerrainInfo *terrainInfo = terrain_info_array+j-1;
						err_when(terrainInfo->anim_frames > 0);
						terrainInfo->anim_frames = animFrameCount;
						terrainInfo->anim_bitmap_ptr = (char **) mem_add(
							sizeof(char *)*animFrameCount);
						memcpy(terrainInfo->anim_bitmap_ptr, animFrameBitmap,
							sizeof(char *)*animFrameCount);
						replaceCount++;
					}
				}
			}
		}
		*/

		err_when( replaceCount == 0);
	}

	mem_del(file_name_array);
	file_name_array = NULL;
}
//------------ End of function TerrainRes::load_anim_info -------//


//----- Begin of function TerrainRes::get_tera_type_id ------//

int TerrainRes::get_tera_type_id(char* teraTypeCode)
{
	return terrain_code(teraTypeCode[0]);
}
//----- End of function TerrainRes::get_tera_type_id ------//


//----- Begin of function TerrainRes::get_map_tile ------//

char* TerrainRes::get_map_tile(int terrainId)
{
	return map_tile_ptr_array[terrain_res[terrainId]->average_type-1];
}
//----- End of function TerrainRes::get_map_tile ------//


#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS

//---------- Begin of function TerrainRes::operator[] -----------//

TerrainInfo* TerrainRes::operator[](int terrainId)
{
	err_if( terrainId<1 || terrainId>terrain_count )
		err_now( "TerrainRes::operator[]" );

	return terrain_info_array+terrainId-1;
}
//------------ End of function TerrainRes::operator[] -----------//

#endif
