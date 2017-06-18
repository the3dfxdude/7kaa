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

//Filename    : OUNITRES.H
//Description : Header file of Object Unit resource

#ifndef __OUNITRES_H
#define __OUNITRES_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

#include <win32_compat.h>

#ifdef NO_DEBUG_UNIT
#undef DEBUG
#endif

//---------- Define constant ------------//

enum { UNIT_NORMAN=1,
		 UNIT_MAYA,
		 UNIT_GREEK,
		 UNIT_VIKING,
		 UNIT_PERSIAN,
		 UNIT_CHINESE,
		 UNIT_JAPANESE,
		 UNIT_CARAVAN,
		 UNIT_CATAPULT,
		 UNIT_BALLISTA,
		 UNIT_FLAMETHROWER,
		 UNIT_CANNON,
		 UNIT_EXPLOSIVE_CART,
		 UNIT_VESSEL,
		 UNIT_TRANSPORT,
		 UNIT_CARAVEL,
		 UNIT_GALLEON,
		 UNIT_DRAGON,
		 UNIT_CHINESE_DRAGON,
		 UNIT_PERSIAN_HEALER,
		 UNIT_VIKING_GOD,
		 UNIT_PHOENIX,
		 UNIT_KUKULCAN,
		 UNIT_JAPANESE_GOD,
		 UNIT_SKELETON,
		 UNIT_LYW,
		 UNIT_HOBGLOBLIN,
		 UNIT_GIANT_ETTIN,
		 UNIT_GITH,
		 UNIT_ROCKMAN,
		 UNIT_GREMJERM,
		 UNIT_FIREKIN,
		 UNIT_GNOLL,
		 UNIT_GOBLIN,
		 UNIT_LIZARDMAN,
		 UNIT_MAN,
		 UNIT_HEADLESS,
		 UNIT_EGYPTIAN,
		 UNIT_INDIAN,
		 UNIT_ZULU,
		 UNIT_EGYPTIAN_GOD,
		 UNIT_INDIAN_GOD,
		 UNIT_ZULU_GOD,
		 UNIT_F_BALLISTA,
		 UNIT_LAST					// keep this line after the last unit
	  };

enum { MAX_UNIT_TYPE = UNIT_LAST-1,
		 MAX_WEAPON_TYPE = 6,		// no. of types of weapons
		 MAX_SHIP_TYPE = 4, 			// no. of types of ships
	  };

//--------- Define Unit Classes --------//

enum { UNIT_CLASS_HUMAN = 'H',
		 UNIT_CLASS_CARAVAN = 'C',
		 UNIT_CLASS_WEAPON = 'W',
		 UNIT_CLASS_SHIP = 'S',
		 UNIT_CLASS_MONSTER = 'M',
		 UNIT_CLASS_GOD = 'G',
	  };

//------------ Mobile Types -------------//

enum { UNIT_AIR ='A',
		 UNIT_LAND='L',
		 UNIT_SEA ='S',
	  };

//--------- Define constant ------------//

#define STD_UNIT_HIT_POINTS  200       // the hit points for all standard units

//--------- Define constant -----------//

enum { UNIT_SMALL_ICON_WIDTH=24,
		 UNIT_SMALL_ICON_HEIGHT=20,
		 UNIT_LARGE_ICON_WIDTH=46,
		 UNIT_LARGE_ICON_HEIGHT=38
	  };

//-------- Define struct UnitRec ----------//

struct UnitRec
{
	enum { NAME_LEN=15, SPRITE_CODE_LEN=8, RACE_CODE_LEN=8, UNIT_CLASS_LEN=8, UNIT_PARA_LEN=3,
			 BUILD_DAYS_LEN=3, YEAR_COST_LEN=3, CARRY_CAPACITY_LEN=3, FREE_WEAPON_COUNT_LEN=1,
			 FILE_NAME_LEN=8, BITMAP_PTR_LEN=4, SPRITE_ID_LEN=3, RACE_ID_LEN=3 };

	char name[NAME_LEN];
	char sprite_code[SPRITE_CODE_LEN];
	char race_code[RACE_CODE_LEN];
	char unit_class[UNIT_CLASS_LEN];

	char mobile_type;
	char all_know;

	char visual_range[UNIT_PARA_LEN];
   char visual_extend[UNIT_PARA_LEN];
   char shealth[UNIT_PARA_LEN];
	char hit_points[UNIT_PARA_LEN];
	char armor[UNIT_PARA_LEN];

	char build_days[BUILD_DAYS_LEN];
	char year_cost[YEAR_COST_LEN];

   char weapon_power;		// an index from 1 to 9 indicating the powerfulness of the weapon 

	char carry_unit_capacity[CARRY_CAPACITY_LEN];
	char carry_goods_capacity[CARRY_CAPACITY_LEN];
	char free_weapon_count[FREE_WEAPON_COUNT_LEN];

   char vehicle_code[SPRITE_CODE_LEN];
	char vehicle_unit_code[SPRITE_CODE_LEN];

	char transform_unit[SPRITE_CODE_LEN];
	char transform_combat_level[UNIT_PARA_LEN];
	char guard_combat_level[UNIT_PARA_LEN];

	char large_icon_file_name[FILE_NAME_LEN];
	char large_icon_ptr[BITMAP_PTR_LEN];
	char general_icon_file_name[FILE_NAME_LEN];
	char general_icon_ptr[BITMAP_PTR_LEN];
	char king_icon_file_name[FILE_NAME_LEN];
	char king_icon_ptr[BITMAP_PTR_LEN];

	char small_icon_file_name[FILE_NAME_LEN];
	char small_icon_ptr[BITMAP_PTR_LEN];
	// ###### begin Gilbert 17/10 #######//
	char general_small_icon_file_name[FILE_NAME_LEN];
	char general_small_icon_ptr[BITMAP_PTR_LEN];
	char king_small_icon_file_name[FILE_NAME_LEN];
	char king_small_icon_ptr[BITMAP_PTR_LEN];
	// ###### end Gilbert 17/10 #######//

	char die_effect_sprite[SPRITE_CODE_LEN];

	char sprite_id[SPRITE_ID_LEN];
	char dll_sprite_id[SPRITE_ID_LEN];
	char race_id[RACE_ID_LEN];

	char vehicle_id[SPRITE_ID_LEN];
	char vehicle_unit_id[SPRITE_ID_LEN];

	char transform_unit_id[SPRITE_ID_LEN];
	char die_effect_id[UNIT_PARA_LEN];

	char first_attack[UNIT_PARA_LEN];
	char attack_count[UNIT_PARA_LEN];
};

//-------- Define struct UnitAttackRec ----------//

struct UnitAttackRec
{
	enum { SPRITE_CODE_LEN=8, UNIT_PARA_LEN=3, COMBAT_LEVEL_LEN=3 };

	char sprite_code[SPRITE_CODE_LEN];

	char attack_id[UNIT_PARA_LEN];
	char combat_level[COMBAT_LEVEL_LEN];

	char attack_delay[UNIT_PARA_LEN];
	char attack_range[UNIT_PARA_LEN];

	char attack_damage[UNIT_PARA_LEN];
	char pierce_damage[UNIT_PARA_LEN];

	char bullet_out_frame[UNIT_PARA_LEN];
	char bullet_speed[UNIT_PARA_LEN];
	char bullet_radius[UNIT_PARA_LEN];
	char bullet_sprite_code[SPRITE_CODE_LEN];
	char bullet_sprite_id[UNIT_PARA_LEN];
	char dll_bullet_sprite_id[UNIT_PARA_LEN];
	char eqv_attack_next[UNIT_PARA_LEN];
	char min_power[UNIT_PARA_LEN];
	char consume_power[UNIT_PARA_LEN];
	char fire_radius[UNIT_PARA_LEN];
	char effect_code[SPRITE_CODE_LEN];
	char effect_id[UNIT_PARA_LEN];
};

//-------- Define struct UnitInfo ----------//

struct UnitInfo
{
	//-------- define constant ---------//

	enum { NAME_LEN=15 };

	char  name[NAME_LEN+1];

	short unit_id;
	short sprite_id;
	short dll_sprite_id;
	char  race_id;
	char  unit_class;
	char  is_monster;

	char  mobile_type;

	BYTE  visual_range;
	BYTE	visual_extend;
	BYTE	shealth;
	BYTE  armor;

	short hit_points;

	char  build_days;
	short build_cost;
	short year_cost;

	char  weapon_power;		// an index from 1 to 9 indicating the powerfulness of the weapon

	char  carry_unit_capacity;
	short carry_goods_capacity;
	char  free_weapon_count;			// only for ships. It's the no. of free weapons can be loaded onto the ship

	char  vehicle_id;
	char  vehicle_unit_id;
	char  solider_id;

	char  transform_unit_id;
	char  transform_combat_level;
	char  guard_combat_level;

	short first_attack;
	char  attack_count;
	short	die_effect_id;

	// char* large_icon_ptr;
	char* soldier_icon_ptr;
	char* general_icon_ptr;
	char* king_icon_ptr;
	// ######### begin Gilbert 17/10 #######//
	// char* small_icon_ptr;
	char* soldier_small_icon_ptr;
	char* general_small_icon_ptr;
	char* king_small_icon_ptr;
	// ######### end Gilbert 17/10 #######//

	//------- game vars -----------//

	char  nation_tech_level_array[MAX_NATION];			// each nation's tech level on this unit
	int   get_nation_tech_level(int nationRecno) 						{ return nation_tech_level_array[nationRecno-1]; }
	void  set_nation_tech_level(int nationRecno, char techLevel) 	{ nation_tech_level_array[nationRecno-1] = techLevel; }

	short nation_unit_count_array[MAX_NATION];			// mobile units + soldiers in camps, not including workers and prayers in bases
	short nation_general_count_array[MAX_NATION];

public:
	int   is_loaded();	// whether the sprite data of this unit is in the memory or not
	char* get_large_icon_ptr(char rankId);
	// ###### begin Gilbert 17/10 ######//
	char*	get_small_icon_ptr(char rankId);
	// ###### end Gilbert 17/10 ######//

	void	inc_nation_unit_count(int nationRecno);
	void	dec_nation_unit_count(int nationRecno);

	void	inc_nation_general_count(int nationRecno);
	void	dec_nation_general_count(int nationRecno);

	void 	unit_change_nation(int newNationRecno, int oldNationRecno, int rankId);
};

//--------- Define struct AttackInfo ----------//

#pragma pack(1)
struct AttackInfo
{
	BYTE  combat_level;

	BYTE  attack_delay;
	BYTE  attack_range;

	BYTE  attack_damage;
   BYTE  pierce_damage;

	short bullet_out_frame;    // on which attacking frames the bullet should be out
	char  bullet_speed;
	char	bullet_radius;
	char  bullet_sprite_id;
	char  dll_bullet_sprite_id;
	char  eqv_attack_next;
	// cur_attack of the next equivalent attack
	// so as to cycle through several similar attacks
	short	min_power;
	short	consume_power;
	char	fire_radius;
	short	effect_id;
};
#pragma pack()

//---------- Define class UnitRes ------------//

class UnitRes
{
public:
	char           init_flag;
	int            unit_info_count;
	int            attack_info_count;

	UnitInfo*      unit_info_array;
	AttackInfo*    attack_info_array;

	ResourceDb     res_large_icon;
	ResourceDb     res_general_icon;
	ResourceDb     res_king_icon;
	ResourceDb     res_small_icon;
	// ###### begin Gilbert 17/10 #######//
	ResourceDb     res_general_small_icon;
	ResourceDb     res_king_small_icon;
	// ###### end Gilbert 17/10 #######//

	short				mobile_monster_count;

public:
	UnitRes()   { init_flag=0; }

	void  init();
	void  deinit();

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		UnitInfo*   operator[](int unitId);
		AttackInfo* get_attack_info(int attackId);
	#else
		UnitInfo*   operator[](int unitId)        { return unit_info_array+unitId-1; }
		AttackInfo* get_attack_info(int attackId) { return attack_info_array+attackId-1; }
	#endif

	static char	mobile_type_to_mask(int mobileType);

private:
	void  load_info();
	void  load_attack_info();
};

extern UnitRes unit_res;

//----------------------------------------------//

#endif

