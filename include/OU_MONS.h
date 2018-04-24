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

//Filename   : OU_MONS.CPP
//Description: Unit Monster header file

#ifndef __OU_MONS_H
#define __OU_MONS_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

enum	{	MONSTER_ACTION_STOP = 0,
			MONSTER_ACTION_ATTACK,
			MONSTER_ACTION_DEFENSE,
			MONSTER_ACTION_EXPAND,
		};

//----------- Define class Monster -----------//

class UnitMonster : public Unit
{
public:
	char	monster_action_mode;

public:
	UnitMonster();

	char* unit_name(int withTitle=1);

	void	set_monster_action_mode(char monsterActionMode);

	void	process_ai();
	void 	die();

	virtual void accept_file_visitor(FileReaderVisitor* v) override;
	virtual void accept_file_visitor(FileWriterVisitor* v) override;

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();

private:
	int 	random_attack();
	int 	assign_to_firm();
	void 	group_order_monster(int destXLoc, int destYLoc, int actionType);
	void 	king_leave_scroll();
};

//--------------------------------------------//

#endif

