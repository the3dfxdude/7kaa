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

//Filename    : OF_INN.H
//Description : Header of FirmInn

#ifndef __OF_INN_H
#define __OF_INN_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//----------- Define constant --------------//

#define MAX_INN_UNIT 		6  		// maximum no. of units allowed in the hire waiting list

//------- define struct InnUnit ---------//

#pragma pack(1)
struct InnUnit
{
public:
	char  unit_id;
	Skill skill;
	short hire_cost;
	short stay_count;		// how long this unit is going to stay until it leaves the inn if you do not hire him.
	short spy_recno;		// >0 if this unit is a spy

public:
	void	set_hire_cost();
};
#pragma pack()

struct FirmInnCrc;

//------- Define class FirmInn --------//

#pragma pack(1)
class FirmInn : public Firm
{
public:
	short			 next_skill_id;		// the skill id. of the next available unit

	InnUnit 		 inn_unit_array[MAX_INN_UNIT];
	short			 inn_unit_count;

public:
	FirmInn();
	~FirmInn();

	void 		init_derived();

	void 		put_info(int);
	int		detect_info();
	void		put_det(int);

	void		next_day();
	void		assign_unit(int unitRecno);

	int		hire(short recNo);
	int		hire_remote(short unitId, short combat_level, short skill_id, short skill_level, short hire_cost, short spy_recno);

	virtual	void auto_defense(short targetRecno);
	virtual	FirmInn* cast_to_FirmInn() { return this; };

	void		process_ai();

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(FirmInnCrc *c);

private:
	int 		should_add_inn_unit();

	void 		add_inn_unit(int unitId);
	void 		del_inn_unit(int recNo);

	void 		update_add_hire_list();
	void 		update_del_hire_list();

	void 		disp_unit_info(int dispY1, InnUnit* hireInfoPtr, int refreshFlag);

	//-------- AI actions ---------//

	int		think_del();
	int 		think_hire_spy();
	int 		think_assign_spy_to(int raceId, int innUnitRecno);
	int 		think_hire_general();
	int 		think_assign_general_to(int raceId, int innUnitRecno);
};
#pragma pack()

//--------------------------------------//

#endif
