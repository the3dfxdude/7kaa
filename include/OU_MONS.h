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

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	int 	random_attack();
	int 	assign_to_firm();
	void 	group_order_monster(int destXLoc, int destYLoc, int actionType);
	void 	king_leave_scroll();
};

//--------------------------------------------//

#endif

