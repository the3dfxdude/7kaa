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

//Filename    : OSPY.H
//Description : class Spy

#ifndef __OSPY_H
#define __OSPY_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.h>
#endif

//------- define constant -------//

#define MAX_BRIBE_AMOUNT	 4000

#define SPY_ENEMY_RANGE  	 5    	// only allow changing spy cloak if there are no enemy units within 5 locations from the unit

#define MIN_VIEW_SECRET_SPYING_SKILL	20		// the minimum spying skill required for viewing secret reports

//------- action mode definitions --------//

enum { SPY_UNDEFINED,
		 SPY_MOBILE,
		 SPY_TOWN,
		 SPY_FIRM,
	  };

enum { SPY_IDLE,
		 SPY_SOW_DISSENT,
		 SPY_SABOTAGE,
	  };

//---------- Define class Spy ---------//

#pragma pack(1)
class Spy
{
public:
	short spy_recno;
	char  spy_place;   			// either SPY_TOWN or SPY_FIRM
	short	spy_place_para;		// it can be town_recno, firm_recno or unit_recno depending on what spy_place is

	char  spy_skill;
	char  spy_loyalty;			// the spy's loyalty to his true home nation

	char  true_nation_recno;
	char	cloaked_nation_recno;

	char	notify_cloaked_nation_flag;		// whether the spy will send a surrendering message to the cloaked nation when it changes its cloak to the nation
	char  exposed_flag;							// this is set to 1 when the spy finished stealing the secret of a nation.

	char  race_id;
	uint16_t name_id;
	char  action_mode;
	const char* action_str();

	int 	cloaked_rank_id();
	int 	cloaked_skill_id();

public:
	Spy();

	void  deinit();

	void	next_day();

	void	process_town_action();
	void	process_firm_action();
	void	pay_expense();

	int 	think_betray();
	void	drop_spy_identity();

	void  change_true_nation(int newNationRecno);
	void  change_cloaked_nation(int newNationRecno);
	int   can_change_cloaked_nation(int newNationRecno);

	int	capture_firm();

	void	reward(int remoteAction);
	void 	set_exposed(int remoteAction);

	void 	think_become_king();

	int	mobilize_spy();
	int 	mobilize_town_spy(int decPop=1);
	int 	mobilize_firm_spy();

	void	set_action_mode(int actionMode);
	void	set_next_action_mode();
	void  change_loyalty(int loyaltyChange);
	void  update_loyalty();

	int 	can_sabotage();

	void	set_place(int spyPlace, int spyPlacePara);
	int 	spy_place_nation_recno();
	int 	get_loc(int& xLoc, int& yLoc);

	int 	assassinate(int targetUnitRecno, int remoteAction);
	int 	get_assassinate_rating(int targetUnitRecno, int& attackRating, int& defenseRating, int& defenderCount);

	void	get_killed(int dispNews=1);

	//------- AI functions -------//

	void	process_ai();
	void	think_town_spy();
	void	think_firm_spy();
	int 	think_mobile_spy();

	int 	think_bribe();
	int 	think_assassinate();
	int	think_reward();

	int 	think_mobile_spy_new_action();
	int 	add_assign_spy_action(int destXLoc, int destYLoc, int cloakedNationRecno);

	int 	ai_spy_being_attacked(int attackerUnitRecno);

	// #### patch begin Gilbert 20/1 ######//
	uint8_t crc8();
	void	clear_ptr();
	// #### patch end Gilbert 20/1 ######//
};
#pragma pack()

//-------- Define class SpyArray -------//

class SpyArray : public DynArrayB
{
public:
	SpyArray();
	~SpyArray();

	void		init();
	void		deinit();

	int 		add_spy(int unitRecno, int spySkill);
	int		add_spy();
	void 		del_spy(int spyRecno);

	void		next_day();

	int 		find_town_spy(int townRecno, int raceId, int spySeq);
	void		process_sabotage();
	void 		mobilize_all_spy(int spyPlace, int spyPlacePara, int nationRecno);

	void 		update_firm_spy_count(int firmRecno);
	void 		change_cloaked_nation(int spyPlace, int spyPlacePara, int fromNationRecno, int toNationRecno);
	void 		set_action_mode(int spyPlace, int spyPlacePara, int actionMode);

	int 		catch_spy(int spyPlace, int spyPlacePara);
	int 		total_spy_skill_level(int spyPlace, int spyPlacePara, int spyNationRecno, int& spyCount);

	void 		disp_view_secret_menu(int spyRecno, int refreshFlag);
	int  		detect_view_secret_menu(int spyRecno, int nationRecno);

	int 		needed_view_secret_skill(int viewMode);

	void 		ai_spy_town_rebel(int townRecno);

	//--------- file functions -----------//

	int 		write_file(File* filePtr);
	int		read_file(File* filePtr);

	//--------------------------------------//

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		Spy* operator[](int recNo);
	#else
		Spy* operator[](int recNo)	  { return (Spy*) get_ptr(recNo); }
	#endif

	int     is_deleted(int recNo);
};

inline int SpyArray::is_deleted(int recNo)
{
	Spy* spyPtr = (Spy*) get_ptr(recNo);
	return !spyPtr || !spyPtr->spy_recno;
}

extern SpyArray spy_array;

//----------------------------------------//

#endif
