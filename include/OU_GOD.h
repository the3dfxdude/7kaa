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

//Filename   : OU_GOD.CPP
//Description: Unit God header file

#ifndef __OU_GOD_H
#define __OU_GOD_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//----------- Define class God -----------//

#pragma pack(1)
class UnitGod : public Unit
{
public:
	short god_id;
	short base_firm_recno;		// recno of the seat of power which creates and supports this god unit
	char	cast_power_type;
	short	cast_origin_x, cast_origin_y;
	short cast_target_x, cast_target_y;

public:
	virtual void init_derived();
	void pre_process();
	int  process_attack();

	void disp_info(int refreshFlag);
	void detect_info();

	void cast_power(int castXLoc, int castYLoc);

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	void consume_power_pray_points();

	void cast_on_loc(int castXLoc, int castYLoc);
	void cast_on_unit(int unitRecno, int divider);
	void cast_on_worker(Worker* workerPtr, int nationRecno, int divider);

	void viking_summon_rain();
	void viking_summon_tornado();

	void persian_cast_power(int unitRecno, int divider);
	void japanese_cast_power(int unitRecno, int divider);
	void maya_cast_power(int unitRecno, int divider);
	void egyptian_cast_power(int unitRecno, int divider);
	void indian_cast_power(int unitRecno, int divider);
	void zulu_cast_power(int unitRecno, int divider);

	void persian_cast_power(Worker* workerPtr, int nationRecno, int divider);
	void japanese_cast_power(Worker* workerPtr, int nationRecno, int divider);
	void maya_cast_power(Worker* workerPtr, int nationRecno, int divider);
	void egyptian_cast_power(Worker *workerPtr, int nationRecno, int divider);
	void indian_cast_power(Worker *workerPtr, int nationRecno, int divider);
	void zulu_cast_power(Worker *workerPtr, int nationRecno, int divider);

	//--------- AI functions ----------//

	void process_ai();
	void think_dragon();
	void think_maya_god();
	void think_phoenix();
	void think_viking_god();
	void think_persian_god();
	void think_chinese_dragon();
	void think_japanese_god();
	int  think_god_attack_target(int& targetXLoc, int& targetYLoc);

	void think_egyptian_god();
	void think_indian_god();
	void think_zulu_god();
};
#pragma pack()

//--------------------------------------------//

#endif

