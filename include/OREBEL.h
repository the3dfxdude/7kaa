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

//Filename    : OREBEL.H
//Description : class Rebel

#ifndef __OREBEL_H
#define __OREBEL_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.h>
#endif


//------- action mode definitions --------//

enum { REBEL_IDLE=1,
		 REBEL_ATTACK_TOWN,		// Attack town without capturing
		 REBEL_ATTACK_FIRM,		// Attack firm without capturing
		 REBEL_SETTLE_NEW,		// Settle to a new town
       REBEL_SETTLE_TO,			// Settle to an existing town 
	  };

//---------- Define class Rebel ---------//

#pragma pack(1)
class Rebel
{
public:
	short	rebel_recno;				// recno of this rebel in rebel_array
	short leader_unit_recno;
	char 	action_mode;
	short	action_para;
	short action_para2;
	short	mobile_rebel_count;				// no. of units in this rebel group
	short town_recno;					// the town controlled by the rebel, one rebel can only control one town
	char	hostile_nation_bits;

public:
	Rebel();
	~Rebel();

	void	next_day();

	void 	join(int unitRecno);

	void	set_action(int actionMode, int actionPara=0, int actionPara2=0)
			{ action_mode = actionMode, action_para = actionPara; action_para2 = actionPara2; }

	void 	town_being_attacked(int attackerUnitRecno);
	void	set_hostile_nation(short nationRecno);
	void	reset_hostile_nation(short nationRecno);
	int	is_hostile_nation(short nationRecno);

public:
	void	think_new_action();
	void	think_cur_action();
	void	think_town_action();

	int 	think_settle_new();
	int 	think_settle_to();
	int 	think_capture_attack_town();
	int 	think_attack_firm();

	void 	execute_new_action();
	void	stop_all_rebel_unit();

	void	turn_indepedent();

	int 	select_new_leader();
	void 	process_leader_quit();

	// #### patch begin Gilbert 20/1 ######//
	uint8_t crc8();
	void	clear_ptr();
	// #### patch end Gilbert 20/1 ######//
};
#pragma pack()

//-------- Define class RebelArray -------//

class RebelArray : public DynArrayB
{
public:
	short		rebel_count;

public:
	RebelArray();
	~RebelArray();

	void 		init();
	void 		deinit();

	int 		create_rebel(int unitRecno, int hostileNationRecno, int actionMode=REBEL_IDLE, int actionPara=0);
	void		del_rebel(int rebelRecno);

	void 		drop_rebel_identity(int unitRecno);
	void 		settle_town(int unitRecno, int townRecno);

	void 		next_day();

	void		stop_attack_town(short townRecno);
	void		stop_attack_firm(short firmRecno);
	void		stop_attack_nation(short nationRecno);

	//--------- file functions -----------//

	int 		write_file(File* filePtr);
	int		read_file(File* filePtr);

	//--------------------------------------//

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		Rebel* operator[](int recNo);
	#else
		Rebel* operator[](int recNo)	{ return (Rebel*) get_ptr(recNo); }
	#endif

	int   	is_deleted(int recNo)   { return get_ptr(recNo) == NULL; }
};


extern RebelArray rebel_array;

//----------------------------------------//

#endif
