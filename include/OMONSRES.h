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

//Filename    : OMONSRES.H
//Description : Header file of object RaceRes

#ifndef __OMONSRES_H
#define __OMONSRES_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//----------- Define constant ------------//

enum { MAX_MONSTER_LEVEL = 9 };
enum { MAX_ACTIVE_MONSTER = 3 };		// No. of monster type in each game

//----------- Define constant ------------//

#define REPUTATION_INCREASE_PER_ATTACK_MONSTER ((float)0.001)

//------------ Define struct RaceRec ---------------//

struct MonsterRec
{
	enum { UNIT_CODE_LEN=8, RANK_LEN=8, NAME_LEN=20, FIRM_BUILD_CODE_LEN=8, UNIT_ID_LEN=3 };

	char 	unit_code[UNIT_CODE_LEN];
	char  name[NAME_LEN];
	char  level;				// the level of the 1-9 monster, the higher the level is, the more powerful the monster is.
	char 	firm_build_code[FIRM_BUILD_CODE_LEN];
	char 	unit_id[UNIT_ID_LEN];
};

//------------- Define struct MonsterInfo --------------//

struct MonsterInfo
{
public:
	//------- constant vars --------//

	enum { NAME_LEN=20, FIRM_BUILD_CODE_LEN=8 };

	short monster_id;

	char	name[NAME_LEN+1];
	short	unit_id;
	char	level;
	char  firm_build_code[FIRM_BUILD_CODE_LEN+1];

public:
	int 	create_firm_monster();
	int	build_firm_monster(int xLoc, int yLoc, int fullHitPoints=1);
};

//----------- Define class MonsterRes ---------------//

class MonsterRes
{
public:
	char        	init_flag;

	short       	monster_count;
	MonsterInfo*   monster_info_array;

	short				active_monster_array[MAX_ACTIVE_MONSTER];

	ResourceDb  	res_bitmap;

public:
	MonsterRes();

	void        init();
	void        deinit();

	void			init_active_monster();
	void 			generate(int generateCount);
	void 			stop_attack_nation(short nationRecno);

	int 			write_file(File* filePtr);
	int			read_file(File* filePtr);

	MonsterInfo* operator[](int monsterId);      // pass monsterId  as recno
	MonsterInfo* get_monster_by_unit_id(int unitId);

private:
	void        load_monster_info();
};

extern MonsterRes monster_res;

//----------------------------------------------------//

#endif
