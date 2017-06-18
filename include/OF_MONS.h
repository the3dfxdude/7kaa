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

//Filename    : OF_MONS.H
//Description : Header of Firm Monster

#ifndef __OF_MONS_H
#define __OF_MONS_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//---------- Define constant ---------//

#define MAX_MONSTER_GENERAL_IN_FIRM 	 8		// maximum no. of monster generals in a firm
#define MAX_SOLDIER_PER_GENERAL			 8		// maximum soldier can be led by a general
#define MAX_MONSTER_IN_FIRM			    ( 1 + MAX_MONSTER_GENERAL_IN_FIRM * (1+MAX_SOLDIER_PER_GENERAL) )

#define MONSTER_ATTACK_NEIGHBOR_RANGE   6  	// it will attack any firms/towns within 3 location away from it
#define MONSTER_EXPAND_RANGE 			    8	 	// the new firm built should be within this distance with the current firm


//------ Define struct MonsterGeneral -------//
//
// Monster generals who live in monster firms.
//
#pragma pack(1)
struct MonsterInFirm
{
public:
	BYTE  monster_id;
	BYTE  unit_id;
	short mobile_unit_recno;		// unit recno of this monster when it is a mobile unit
											// this is only used as a reference for soldiers to find their leaders
	char  combat_level;
	short hit_points;
	short	max_hit_points;

	BYTE  soldier_monster_id;     // monster id. of the soldiers led by this monster general
	char	soldier_count;				// no. of soldiers commaned by this monster general/king

public:
	void	set_combat_level(int combatLevel);
};
#pragma pack()

//------- Define class FirmMonster --------//

#pragma pack(1)
class FirmMonster : public Firm
{
public:
	enum { MAX_WAITING_SOLDIER = 30 };		// maximum no. of soldiers queued in the monster firm waiting for their generals

	short monster_id;					// the monster type id. of the firm.
	short monster_general_count;

	char  monster_aggressiveness;  // 0-100

	char  defending_king_count;
	char  defending_general_count;
	char  defending_soldier_count;

	int	total_defender()	{ return defending_king_count + defending_general_count + defending_soldier_count; }

	MonsterInFirm monster_king;
	MonsterInFirm monster_general_array[MAX_MONSTER_GENERAL_IN_FIRM];

	char	waiting_soldier_count;
	short waiting_soldier_array[MAX_WAITING_SOLDIER];	// the unit recno of their generals are kept here

	char  monster_nation_relation;	// each bit n is high representing this independent town will attack nation n.
	short	defend_target_recno; 		// used in defend mode, store recno of the latest unit attacking this firm

	char 	patrol_unit_count;              	// for AI to get the recno of the patrol units
	short	patrol_unit_array[MAX_SOLDIER_PER_GENERAL+1];

public:
	void	init_derived();
	void  deinit_derived();

	~FirmMonster();

	char*	firm_name();

	void 	put_info(int refreshFlag);
	void 	detect_info();

	void	next_day();

	void	assign_unit(int unitRecno);
	int 	can_assign_monster(int unitRecno);

	void 	set_king(int monsterId, int combatLevel);
	void 	add_general(int generalUnitRecno);
	void 	add_soldier(int generalUnitRecno);

	int 	recruit_general(int soldierCount= -1);
	void 	recruit_soldier();

	int	mobilize_king();
	int	mobilize_general(int generalId, int mobilizeSoldier=1);

	int	total_combat_level();

	void 	being_attacked(int attackerUnitRecno);
	void	clear_defense_mode();
	void	reduce_defender_count(int rankId);

	void	set_hostile_nation(int nationRecno);
	void	reset_hostile_nation(int nationRecno);
	int	is_hostile_nation(int nationRecno);

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();

private:
	void	disp_monster_info(int dispY1, int refreshFlag);

	void 	validate_patrol_unit();
	void 	recover_hit_points();

	int	mobilize_monster(int monsterId, int rankId, int combatLevel, int hitPoints=0);

	int 	think_attack_neighbor();
	int	think_attack_human();
	int 	think_expansion();
};
#pragma pack()

//--------------------------------------//

#endif
