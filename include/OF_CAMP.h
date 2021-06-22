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

//Filename    : OF_CAMP.H
//Description : Header of FirmCamp

#ifndef __OF_CAMP_H
#define __OF_CAMP_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//------------ define constant -----------//

enum	{	INSIDE_CAMP=0,
			OUTSIDE_CAMP,
		};

//-------- Define struct DefenseUnit ----------//

#pragma pack(1)
struct DefenseUnit
{
	short	unit_recno;
	char	status;	// inside / outside the camp
};
#pragma pack()

//------- Define class FirmCamp --------//

struct FirmCampCrc;
class Town;
class Unit;

#pragma pack(1)
class FirmCamp : public Firm
{
public:
	//-------------------------------------//

	DefenseUnit	defense_array[MAX_WORKER+1];	// plus 1 with the overseer
	char			employ_new_worker;
	short			defend_target_recno; // used in defend mode, store recno of the latest target attacking this firm
	char			defense_flag;

	int			is_worker_full();

	//---------- AI related vars ----------//

	char 			patrol_unit_count;              	// for AI to get the recno of the patrol units
	short			patrol_unit_array[MAX_WORKER+1];

	char 			coming_unit_count;              	// for AI to get the recno of the coming units
	short			coming_unit_array[MAX_WORKER+1];

	short			ai_capture_town_recno;		// >0 if the troop is in the process of attacking the independent village for capturing it.
	char			ai_recruiting_soldier;

	char			is_attack_camp;

	int 			total_combat_level();
	int 			average_combat_level();
	int 			ai_combat_level_needed();

	int	   	ai_has_excess_worker();

public:
	FirmCamp();

	void		init_derived();
	void		deinit();

	void 		put_info(int refreshFlag);
	int		detect_info();

	void		next_day();
	void		patrol();
	int		patrol_all_soldier();

	void 		assign_unit(int unitRecno);
	void 		assign_overseer(int overseerRecno);
	void		assign_worker(int unitRecno);

	void		defense(short targetRecno, int useRangeAttack=0);
	void		defense_inside_camp(short unitRecno, short targetRecno);
	void		defense_outside_camp(short unitRecno, short targetRecno);
	void		clear_defense_mode(int firmRecno);

	int	 	mobilize_worker(int workerId, char remoteAction);
	int  		mobilize_overseer();

	void 		change_nation(int newNationRecno);

	void		update_defense_unit(short unitRecno);
	void		set_employ_worker(char flag);

	virtual	FirmCamp *cast_to_FirmCamp() { return this; };

	//----------- AI functions ----------//

	void		process_ai();
	int		ai_should_close();			// overloaded function 
	void		ai_update_link_status();

	int 		cur_commander_leadership(int bestRaceId=0);
	int 		new_commander_leadership(int newRaceId, int newSkillLevel);

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(FirmCampCrc *c);

private:
	void 		reset_unit_home_camp(int firmRecno);
	void 		reset_attack_camp(int firmRecno);

	void		disp_camp_info(int dispY1, int refreshFlag);

	void		train_unit();
	void 		recover_hit_point();
	void 		pay_weapon_expense();

	void 		update_influence();
	void 		validate_patrol_unit();

	//-------------- AI functions --------------//

	void		ai_reset_defense_mode();
	void		think_recruit();
	int 		think_attack();
	int  		ai_recruit(int combatLevelNeeded);
	void		ai_attack_town_defender(Unit*);
	int 		think_attack_nearby_enemy();
	void 		think_change_town_link();

	void 		think_capture();
	int		think_capture_return();
	int 		think_capture_use_spy(Town* targetTown);
	int 		think_capture_use_spy2(Town* targetTown, int raceId, int curSpyLevel);
	int 		think_assign_better_overseer(Town* targetTown);
	int 		think_assign_better_overseer2(int targetTownRecno, int raceId);
	void 		process_ai_capturing();

	int 		think_capture_target_town();
	int 		ai_capture_independent_town(Town* targetTown, int defenseCombatLevel);
	int 		ai_capture_enemy_town(Town* targetTown, int defenseCombatLevel);

	int		think_use_cash_to_capture();
	void 		think_linked_town_change_nation(int linkedTownRecno, int oldNationRecno, int newNationRecno);

	int 		think_assign_better_commander();
	int 		best_commander_race();
};
#pragma pack()

//--------------------------------------//

#endif
