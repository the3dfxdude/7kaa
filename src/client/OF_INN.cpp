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

//Filename    : OF_INN.CPP
//Description : Firm Military Inn

#include <OINFO.h>
#include <OBOX.h>
#include <OVGA.h>
#include <OSTR.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OHELP.h>
#include <OVBROWIF.h>
#include <OGAME.h>
#include <OTOWN.h>
#include <OBUTT3D.h>
#include <ONATION.h>
#include <ORAWRES.h>
#include <ORACERES.h>
#include <OWORLD.h>
#include <OSPY.h>
#include <OF_INN.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OSERES.h>

//------------- Define coordinations -----------//

enum { HIRE_BROWSE_X1 = INFO_X1,
		 HIRE_BROWSE_Y1 = INFO_Y1+52,
		 HIRE_BROWSE_X2 = INFO_X2,
		 HIRE_BROWSE_Y2 = HIRE_BROWSE_Y1+144
	  };

enum { HIRE_DET_X1 = INFO_X1,
		 HIRE_DET_Y1 = HIRE_BROWSE_Y2+5,
		 HIRE_DET_X2 = INFO_X2,
		 HIRE_DET_Y2 = HIRE_DET_Y1+54
	  };

//----------- Define static variables ----------//

static VBrowseIF 	browse_hire;
static Button3D	button_hire;
static int			last_hire_count;
static FirmInn* 	firm_inn_ptr;

//----------- Define static functions ----------//

static void put_hire_rec(int recNo, int x, int y, int refreshFlag);

//--------- Begin of function FirmInn::FirmInn ---------//
//
FirmInn::FirmInn()
{
}
//----------- End of function FirmInn::FirmInn -----------//


//--------- Begin of function FirmInn::~FirmInn ---------//
//
FirmInn::~FirmInn()
{
}
//----------- End of function FirmInn::~FirmInn -----------//


//--------- Begin of function FirmInn::init_derived ---------//
//
void FirmInn::init_derived()
{
	inn_unit_count = 0;
	next_skill_id   = m.random(MAX_TRAINABLE_SKILL)+1;
}
//----------- End of function FirmInn::init_derived -----------//


//--------- Begin of function FirmInn::assign_unit ---------//
//
void FirmInn::assign_unit(int unitRecno)
{
	//------- if this is a construction worker -------//

	if( unit_array[unitRecno]->skill.skill_id == SKILL_CONSTRUCTION )
	{
		set_builder(unitRecno);
		return;
	}
/*
	//--- can only assign if the unit is a spy cloaked into this firm's nation color ---//

	if( !unitPtr->spy_recno || unitPtr->nation_recno != nation_recno )
		return;

	//---------------------------------------------//

	if( inn_unit_count == MAX_INN_UNIT )
		return;

	err_when( inn_unit_count > MAX_INN_UNIT );

	err_when( !unitRecno );

	Unit* unitPtr  = unit_array[unitRecno];

	//--------- add the InnUnit --------//

	InnUnit *innUnit = inn_unit_array+inn_unit_count;

	inn_unit_count++;

	//--------- set InnUnit vars -----------------//

	innUnit->unit_id   = unitPtr->unit_id;
	innUnit->skill     = unitPtr->skill;
	innUnit->spy_recno = unitPtr->spy_recno;

	innUnit->set_hire_cost();
	unitPtr->deinit_sprite();
*/
}
//----------- End of function FirmInn::assign_unit -----------//


//--------- Begin of function FirmInn::put_info ---------//
//
void FirmInn::put_info(int refreshFlag)
{
	firm_inn_ptr = this;

	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	//----------- display browser -----------//

	if( refreshFlag == INFO_REPAINT )
	{
		browse_hire.init( HIRE_BROWSE_X1, HIRE_BROWSE_Y1, HIRE_BROWSE_X2, HIRE_BROWSE_Y2,
								0, 25, inn_unit_count, put_hire_rec );

		browse_hire.open(1);
	}
	else
	{
		if( last_hire_count != inn_unit_count )
		{
			if( last_hire_count==0 || inn_unit_count==0 )		// repaint the whole area as the detail area needs to disappear
			{
				last_hire_count = inn_unit_count;
				info.disp();
				return;
			}

			last_hire_count = inn_unit_count;

			browse_hire.refresh(-1, inn_unit_count);

			if( last_hire_count==0 || inn_unit_count==0 )
				refreshFlag = INFO_REPAINT;
		}
		else
			browse_hire.update();          // update only
	}

	last_hire_count = inn_unit_count;

	put_det(refreshFlag);

	//---------- display spy button ----------//

	disp_spy_button(INFO_X1+BUTTON_ACTION_WIDTH, HIRE_DET_Y2+4, refreshFlag);
}
//----------- End of function FirmInn::put_info -----------//


//--------- Begin of function FirmInn::detect_info ---------//
//
void FirmInn::detect_info()
{
	firm_inn_ptr = this;

	if( detect_basic_info() )
		return;

	//-------- detect spy button ----------//

	if( !own_firm() )
	{
		detect_spy_button();
		return;
	}

	//-------------------------------------//

	if( browse_hire.detect() )
	{
		put_det(INFO_UPDATE);
	}

	if( button_hire.detect() && inn_unit_count > 0 )
	{
		// ###### begin Gilbert 31/7 #######//
		se_res.far_sound(center_x, center_y, 1, 'S', 
			unit_res[inn_unit_array[browse_hire.recno()-1].unit_id]->sprite_id,
			"RDY" );
		// ###### end Gilbert 31/7 #######//
#ifdef USE_DPLAY
		if(remote.is_enable())
		{
			// packet structure : <firm recno>, <hire Id> <nation no>
			short *shortPtr=(short *)remote.new_send_queue_msg(MSG_F_INN_HIRE, 3*sizeof(short));
			shortPtr[0] = firm_recno;
			shortPtr[1] = browse_hire.recno();
			shortPtr[2] = nation_recno;
		}
		else
#endif
		{
			hire(browse_hire.recno());
		}
	}
}
//----------- End of function FirmInn::detect_info -----------//


//--------- Begin of function FirmInn::hire ---------//
//
int FirmInn::hire(short recNo)
{
	err_when( recNo < 1 );

	if( recNo > inn_unit_count )		// this may happen in a multiplayer game
		return 0;

	//--------- first check if you have enough money to hire ------//

	InnUnit* innUnit;
	Nation* 	nationPtr		= nation_array[nation_recno];

	innUnit = inn_unit_array+recNo-1;

	if( nationPtr->cash < innUnit->hire_cost )
		return 0;

	//---------- add the unit now -----------//

	int unitRecno = create_unit( innUnit->unit_id );
	if(!unitRecno)
		return 0; // no space for creating the unit

	nationPtr->add_expense(EXPENSE_HIRE_UNIT, innUnit->hire_cost);

	//-------- set skills of the unit --------//

	Unit* unitPtr = unit_array[unitRecno];

	memcpy( &(unitPtr->skill), &(innUnit->skill), sizeof(Skill) );

	err_when( innUnit->skill.combat_level<=0 || innUnit->skill.combat_level>100 );

	unitPtr->set_combat_level( innUnit->skill.combat_level );

	//-------- if the unit's skill is spying -----//

	if( unitPtr->skill.skill_id == SKILL_SPYING )
	{
		unitPtr->spy_recno = spy_array.add_spy(unitRecno, unitPtr->skill.skill_level);
		unitPtr->skill.skill_id = 0;		// reset its primary skill, its spying skill has been recorded in spy_array
	}

	//----------------------------------------//
	//
	// Loyalty of the hired unit
	//
	// = 30 + the nation's reputation / 2 + 30 if racially homogenous
	//
	//----------------------------------------//

	int unitLoyalty = 30 + (int)nationPtr->reputation/2;

	if( race_res.is_same_race(unitPtr->race_id, nationPtr->race_id) )
		unitLoyalty += 20;

	unitLoyalty = MAX( 40, unitLoyalty );
	unitLoyalty = MIN( 100, unitLoyalty );

	if( unitPtr->spy_recno )
		spy_array[unitPtr->spy_recno]->spy_loyalty = unitLoyalty;
	else
		unitPtr->loyalty = unitLoyalty; 

	//---- remove the record from the hire list ----//

	del_inn_unit(recNo);

	if( firm_recno == firm_array.selected_recno &&
		 nation_recno == nation_array.player_recno )
	{
		put_info(INFO_UPDATE);
	}

	return unitRecno;
}
//----------- End of function FirmInn::hire -----------//


//--------- Begin of function FirmInn::put_det ---------//
//
void FirmInn::put_det(int refreshFlag)
{
	if( browse_hire.none_record )
	{
		button_hire.reset();
		return;
	}

	//--------- display details ----------//

	InnUnit* innUnit = inn_unit_array+browse_hire.recno()-1;

	disp_unit_info(HIRE_DET_Y1, innUnit, refreshFlag );

	//------- paint buttons --------//

	if( own_firm() )
	{
		if( refreshFlag == INFO_REPAINT )
		{
			button_hire.paint( HIRE_DET_X1, HIRE_DET_Y2+4, 'A', "HIREUNIT" );
		}
		else
		{
			if( inn_unit_count > 0 &&
				 (~nation_array)->cash >= inn_unit_array[browse_hire.recno()-1].hire_cost )
			{
				button_hire.enable();
			}
			else
			{
				button_hire.disable();
			}
		}
	}
}
//----------- End of function FirmInn::put_det -----------//


//--------- Begin of function FirmInn::disp_unit_info ---------//
//
// Display the skill information of the people in the town.
//
// <int> 			 dispY1		 - the top y coordination of the info area
// <InnUnit*> hireInfoPtr - pointer to a HireInfo structure
// <int>    		 refreshFlag - refresh flag
//
void FirmInn::disp_unit_info(int dispY1, InnUnit* hireInfoPtr, int refreshFlag)
{
	Skill* skillPtr = &(hireInfoPtr->skill);

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+54 );
	}

	//------ display population composition of this resident town -----//

	int x=INFO_X1+4, y=dispY1+4, x1=x+100;
	String str;

	font_san.field( x, y, "Combat", x1, skillPtr->combat_level, 1, INFO_X2-10, refreshFlag );

	if( refreshFlag == INFO_REPAINT )
	{
		font_san.field( x, y+16, skillPtr->skill_des(), x1, skillPtr->skill_level , 1, INFO_X2-10, refreshFlag );
	}
	else
	{
		font_san.use_max_height();
		font_san.put( x+2, y+18, skillPtr->skill_des(), 1, x1-2 );
		font_san.use_std_height();

		font_san.update_field( x1, y+16, skillPtr->skill_level, 1, INFO_X2-10);
	}

	font_san.field( x, y+32, "Hiring Cost", x1, hireInfoPtr->hire_cost, 2, INFO_X2-10, refreshFlag);
}
//----------- End of function FirmInn::disp_unit_info -----------//


//-------- Begin of static function put_hire_rec --------//
//
static void put_hire_rec(int recNo, int x, int y, int refreshFlag)
{
	InnUnit* innUnit = firm_inn_ptr->inn_unit_array+recNo-1;

	//-------- display unit icon -------//

	vga_front.d3_panel_down(x+1, y+1, x+UNIT_SMALL_ICON_WIDTH+4, y+UNIT_SMALL_ICON_HEIGHT+4, 2, 0 );
	// ##### begin Gilbert 17/10 #######//
	vga_front.put_bitmap(x+3, y+3, unit_res[innUnit->unit_id]->get_small_icon_ptr(RANK_SOLDIER) );
	// ##### end Gilbert 17/10 #######//

	//--------- set help parameters --------//

	if( mouse.in_area(x+1, y+1, x+UNIT_SMALL_ICON_WIDTH+4, y+UNIT_SMALL_ICON_HEIGHT+4) )
		help.set_unit_help( innUnit->unit_id, 0, x+1, y+1, x+UNIT_SMALL_ICON_WIDTH+4, y+UNIT_SMALL_ICON_HEIGHT+4 );

	//---------- display info ----------//

	y+=6;

   font_san.use_max_height();

	if( innUnit->skill.skill_level > 0 )
	{
		font_san.put( x+32 , y, innUnit->skill.skill_des(1), 1, x+122 );		// 1-use short words
		font_san.put( x+116, y, innUnit->skill.skill_level, 1, x+146 );
	}
	else
	{
		font_san.put( x+32 , y, "Combat", 1, x+122 );		// 1-use short words
		font_san.put( x+116, y, innUnit->skill.combat_level, 1, x+146 );
	}

	font_san.use_std_height();

	font_san.put( x+140, y, m.format(innUnit->hire_cost,2), 1, x+browse_hire.rec_width-3 );
}
//----------- End of static function put_hire_rec -----------//


//--------- Begin of function FirmInn::next_day ---------//
//
void FirmInn::next_day()
{
	//----------- run next_day() of the base class -------------//

	Firm::next_day();

	//------------ update the hire list ------------//

	int updateInterveal = 10 + info.year_passed*2;		// there will be less and less units to hire as the game passes

	if( info.game_date % updateInterveal == firm_recno % updateInterveal )
		update_add_hire_list();

	if( info.game_date % 10 == firm_recno % 10 )
		update_del_hire_list();
}
//----------- End of function FirmInn::next_day -----------//


//---------- Begin of function FirmInn::update_add_hire_list --------//
//
void FirmInn::update_add_hire_list()
{
	//-------- new units come by --------//

	if( inn_unit_count < MAX_INN_UNIT )
	{
		if( should_add_inn_unit() )
		{
			int unitId = race_res[m.random(MAX_RACE)+1]->basic_unit_id;

			if( unitId )
				add_inn_unit(unitId);
		}
	}
}
//----------- End of function FirmInn::update_add_hire_list ---------//


//--------- Begin of function FirmInn::update_del_hire_list --------//
//
void FirmInn::update_del_hire_list()
{
	//------- existing units leave -------//

	for( int i=inn_unit_count ; i>0 && inn_unit_count>0 ; i-- )
	{
		if( !inn_unit_array[i-1].spy_recno && --inn_unit_array[i-1].stay_count==0 )
		{
			del_inn_unit(i);

			if( firm_recno == firm_array.selected_recno )
			{
				if( browse_hire.recno() > i && browse_hire.recno() > 1 )
					browse_hire.refresh( browse_hire.recno()-1, inn_unit_count );
			}
		}
	}
}
//--------- End of function FirmInn::update_del_hire_list ---------//


//-------- Begin of function FirmInn::should_add_inn_unit --------//
//
int FirmInn::should_add_inn_unit()
{
	#define MAX_INN_UNIT_PER_REGION 	10		// maximum no. of inn units in any single region

	FirmInn* firmInn;
	int 		totalInnUnit = inn_unit_count;

	for(int i=0; i<linked_firm_count; i++)
	{
		firmInn = (FirmInn*) firm_array[linked_firm_array[i]];   // links between inns are stored in linked_firm_array[] for quick scan only

		err_when( firmInn->firm_id != FIRM_INN );

		totalInnUnit += firmInn->inn_unit_count;
	}

	return totalInnUnit < MAX_INN_UNIT_PER_REGION;
}
//--------- End of function FirmInn::should_add_inn_unit ---------//


//--------- Begin of function FirmInn::add_inn_unit ---------//

void FirmInn::add_inn_unit(int unitId)
{
	err_when( inn_unit_count >= MAX_INN_UNIT );

	InnUnit *innUnit = inn_unit_array+inn_unit_count;

	inn_unit_count++;

	innUnit->unit_id = unitId;

	//--------- set the skill now -----------------//

	char skillId = (char) next_skill_id;

	if( ++next_skill_id > MAX_TRAINABLE_SKILL )
		next_skill_id = 1;

	innUnit->skill.skill_id = skillId;

	if( skillId > 0 )
		innUnit->skill.skill_level = 30+m.random(70);
	else
		innUnit->skill.skill_level = 0;

	if( skillId==0 || skillId==SKILL_LEADING )
		innUnit->skill.combat_level = 30+m.random(70);
	else
		innUnit->skill.combat_level = 10;

	innUnit->set_hire_cost();

	innUnit->stay_count = 5 + m.random(5);

	innUnit->spy_recno = 0;
}
//----------- End of function FirmInn::add_inn_unit -----------//


//--------- Begin of function InnUnit::set_hire_cost ---------//
//
void InnUnit::set_hire_cost()
{
	hire_cost = skill.combat_level + (int)skill.skill_level*2;

	if( skill.skill_id==SKILL_LEADING )		// the cost of a leader unit is higher
	{
		hire_cost += skill.skill_level*2;

		for( int i=10 ; i<=100 ; i+=20 )		// increase the hiring cost with bigger steps when the skill level gets higher
		{
			if( i > skill.skill_level )
				hire_cost += i/5;
		}
	}
	else if( skill.skill_id==SKILL_SPYING )		// the cost of a leader unit is higher
	{
		hire_cost += skill.skill_level;
	}

	hire_cost *= 2;
}
//----------- End of function InnUnit::set_hire_cost -----------//


//--------- Begin of function FirmInn::del_inn_unit ---------//

void FirmInn::del_inn_unit(int recNo)
{
	err_when( recNo<1 || recNo>MAX_INN_UNIT );

	err_when( inn_unit_count < 1 );
	err_when( inn_unit_count > MAX_INN_UNIT );

	m.del_array_rec(inn_unit_array, inn_unit_count, sizeof(InnUnit), recNo);

	inn_unit_count--;
}
//----------- End of function FirmInn::del_inn_unit -----------//


//--------- Begin of function FirmInn::auto_defense ---------//
void FirmInn::auto_defense(short targetRecno)
{
	//---------- the area to check -----------//
	int xLoc1 = center_x - EFFECTIVE_FIRM_TOWN_DISTANCE;
	int yLoc1 = center_y - EFFECTIVE_FIRM_TOWN_DISTANCE;
	int xLoc2 = center_x + EFFECTIVE_FIRM_TOWN_DISTANCE;
	int yLoc2 = center_y + EFFECTIVE_FIRM_TOWN_DISTANCE;

	//----------- boundary checking ----------//
	if(xLoc1<0)	xLoc1 = 0;
	if(yLoc1<0) yLoc1 = 0;
	if(xLoc2>=MAX_WORLD_X_LOC) xLoc2 = MAX_WORLD_X_LOC-1;
	if(yLoc2>=MAX_WORLD_Y_LOC) yLoc2 = MAX_WORLD_Y_LOC-1;

	int skipWidthDist = STD_TOWN_LOC_WIDTH;
	int skipHeightDist = STD_TOWN_LOC_HEIGHT;
	int xEnd, yEnd, xLimit, yLimit;
	int i, j, dist;
	Location *locPtr;
	Town *townPtr;

	//--------------------------------------------------//
	// check for our town in the effective area
	//--------------------------------------------------//
	xLimit = xLoc2+skipWidthDist-1;
	yLimit = yLoc2+skipHeightDist-1;
	for(xEnd=0, i=xLoc1; i<=xLimit && !xEnd; i+=skipWidthDist)
	{
		if(i>=xLoc2)
		{
			xEnd++; // final
			i = xLoc2;
		}

		for(yEnd=0, j=yLoc1; j<=yLimit && !yEnd; j+=skipHeightDist)
		{
			if(j>=yLoc2)
			{
				yEnd++; // final
				j = yLoc2;
			}

			err_when(i<xLoc1 || i>xLoc2 || j<yLoc1 || j>yLoc2);
			locPtr = world.get_loc(i, j);
			if(!locPtr->is_town())
				continue;

			err_when(!locPtr->town_recno() || town_array.is_deleted(locPtr->town_recno()));
			townPtr = town_array[locPtr->town_recno()];

			if(townPtr->nation_recno!=nation_recno)
				continue;

			dist = m.points_distance(center_x, center_y, townPtr->center_x, townPtr->center_y);
			if(dist <= EFFECTIVE_FIRM_TOWN_DISTANCE)
				townPtr->auto_defense(targetRecno);
		}
	}
}
//----------- End of function FirmInn::auto_defense -----------//

