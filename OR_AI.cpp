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

//Filename    : OR_AI.CPP
//Description : Report for displaying AI actions

#include <OVGA.h>
#include <ODATE.h>
#include <OFONT.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <OFIRMRES.h>
#include <ONATION.h>
#include <OINFO.h>

//------------- Define coordinations -----------//

enum { ACTION_BROWSE_X1 = ZOOM_X1+6,
		 ACTION_BROWSE_Y1 = ZOOM_Y1+6,
		 ACTION_BROWSE_X2 = ZOOM_X2-6,
		 ACTION_BROWSE_Y2 = ACTION_BROWSE_Y1+200
	  };

enum { ATTACK_BROWSE_X1 = ZOOM_X1+6,
		 ATTACK_BROWSE_Y1 = ACTION_BROWSE_Y2+6,
		 ATTACK_BROWSE_X2 = ZOOM_X2-6,
		 ATTACK_BROWSE_Y2 = ATTACK_BROWSE_Y1+160
	  };

enum { OTHER_INFO_X1 = ZOOM_X1+6,
		 OTHER_INFO_Y1 = ATTACK_BROWSE_Y2+6,
		 OTHER_INFO_X2 = ZOOM_X2-6,
		 OTHER_INFO_Y2 = ZOOM_Y2-6,
	  };

//----------- Define action mode string --------//

static char* action_mode_str_array[] =
{
	"Build firm",
	"Assign overseer",
	"Assign constructor",
	"Assign worker",
	"Assign spy",
	"Scout",
	"Settle to another town",
	"Process diplomatic message",
	"Defend",
	"Sea Travel 1",
	"Sea Travel 2",
	"Sea Travel 3",
};

//----------- Define static variables ----------//

static VBrowseIF browse_action, browse_attack;
static int		  total_population, total_peasant;
static Nation*	  cur_nation_ptr;

//----------- Define static functions ----------//

static void put_action_rec(int recNo, int x, int y, int refreshFlag);
static void put_attack_rec(int recNo, int x, int y, int refreshFlag);
static void disp_other_info();

//--------- Begin of function Info::disp_ai_action ---------//
//
void Info::disp_ai_action(int refreshFlag)
{
	cur_nation_ptr = nation_array[info.viewing_nation_recno];

	if( cur_nation_ptr->nation_type != NATION_AI )
		return;

   //------ display the queued action list -------//

	int x=ACTION_BROWSE_X1+9;
	int y=ACTION_BROWSE_Y1+4;

	vga_back.d3_panel_up(ACTION_BROWSE_X1, ACTION_BROWSE_Y1, ACTION_BROWSE_X2, ACTION_BROWSE_Y1+18 );

	font_san.put( x	 , y, "Action Mode" );
	font_san.put( x+120, y, "Action Para" );
	font_san.put( x+220, y, "Add Date" );
	font_san.put( x+320, y, "Instance" );
	font_san.put( x+390, y, "Processing" );
	font_san.put( x+470, y, "Processed" );

	browse_action.init( ACTION_BROWSE_X1, ACTION_BROWSE_Y1+20, ACTION_BROWSE_X2, ACTION_BROWSE_Y2,
							  0, 22, cur_nation_ptr->action_count(), put_action_rec, 1 );

	browse_action.open(browse_ai_action_recno );		// if refreshFlag is INFO_UPDATE, keep the original top_rec_no of the browser

	//------ display the queued action list -------//

	x=ATTACK_BROWSE_X1+9;
	y=ATTACK_BROWSE_Y1+4;

	vga_back.d3_panel_up(ATTACK_BROWSE_X1, ATTACK_BROWSE_Y1, ATTACK_BROWSE_X2, ATTACK_BROWSE_Y1+18 );

	font_san.put( x	 , y, "Firm recno" );
	font_san.put( x+120, y, "Combat level" );
	font_san.put( x+220, y, "Distance" );
	font_san.put( x+320, y, "Patrol Date" );

	browse_attack.init( ATTACK_BROWSE_X1, ATTACK_BROWSE_Y1+20, ATTACK_BROWSE_X2, ATTACK_BROWSE_Y2,
							  0, 22, cur_nation_ptr->attack_camp_count, put_attack_rec, 1 );

	browse_attack.open(browse_ai_attack_recno);		// if refreshFlag is INFO_UPDATE, keep the original top_rec_no of the browser

	//-------------------------------//

	disp_other_info();
}
//----------- End of function Info::disp_ai_action -----------//


//--------- Begin of function Info::detect_ai_action ---------//
//
void Info::detect_ai_action()
{
	if( browse_action.detect() )
		browse_ai_action_recno = browse_action.recno();

	if( browse_attack.detect() )
		browse_ai_attack_recno = browse_attack.recno();
}
//----------- End of function Info::detect_ai_action -----------//


//-------- Begin of static function put_action_rec --------//
//
static void put_action_rec(int recNo, int x, int y, int refreshFlag)
{
	ActionNode* actionNode = cur_nation_ptr->get_action(recNo);

	//---------- display info ----------//

	x+=3;
	y+=3;

	font_san.put( x, y, action_mode_str_array[actionNode->action_mode-1] );

	if( actionNode->action_mode >= ACTION_AI_BUILD_FIRM &&
		 actionNode->action_mode <= ACTION_AI_ASSIGN_WORKER )
	{
		font_san.put( x+120, y, firm_res[actionNode->action_para]->name, 0, x+216 );
	}

	font_san.put( x+220, y, date.date_str(actionNode->add_date, 1) );		// short month string
	font_san.put( x+340, y, actionNode->instance_count );
	font_san.put( x+410, y, actionNode->processing_instance_count );
	font_san.put( x+490, y, actionNode->processed_instance_count );
/*
   font_san.put( x+220, y, actionNode->action_x_loc );		// short month string
	font_san.put( x+340, y, actionNode->action_y_loc );
	font_san.put( x+410, y, actionNode->ref_x_loc );
	font_san.put( x+490, y, actionNode->ref_y_loc );
*/
}
//----------- End of static function put_action_rec -----------//


//-------- Begin of static function put_attack_rec --------//
//
static void put_attack_rec(int recNo, int x, int y, int refreshFlag)
{
	AttackCamp* attackCamp = cur_nation_ptr->attack_camp_array+recNo-1;

	//---------- display info ----------//

	x+=3;
	y+=3;

	font_san.put( x	 , y, attackCamp->firm_recno );
	font_san.put( x+120, y, attackCamp->combat_level );
	font_san.put( x+220, y, attackCamp->distance );
	font_san.put( x+340, y, date.date_str(attackCamp->patrol_date) );
}
//----------- End of static function put_attack_rec -----------//


//--------- Begin of static function disp_other_info ---------//
//
static void disp_other_info()
{
	//---------- display total income ----------//

	vga_back.d3_panel_up(OTHER_INFO_X1, OTHER_INFO_Y1, OTHER_INFO_X2, OTHER_INFO_Y2 );

	int x=OTHER_INFO_X1+9;
	int y=OTHER_INFO_Y1+6;
	int x2=x+300;

	font_san.put_field( x, y    , "ai_capture_enemy_town_recno", x2, cur_nation_ptr->ai_capture_enemy_town_recno, 1 );
	font_san.put_field( x, y+=16, "ai_capture_enemy_town_plan_date", x2, date.date_str(cur_nation_ptr->ai_capture_enemy_town_plan_date) );
	font_san.put_field( x, y+=16, "ai_capture_enemy_town_start_attack_date", x2, date.date_str(cur_nation_ptr->ai_capture_enemy_town_start_attack_date) );

	font_san.put_field( x, y+=16, "ai_attack_target_nation_recno", x2, cur_nation_ptr->ai_attack_target_nation_recno, 1 );
	font_san.put_field( x, y+=16, "ai_attack_target_x_loc", x2, cur_nation_ptr->ai_attack_target_x_loc, 1 );
	font_san.put_field( x, y+=16, "ai_attack_target_y_loc", x2, cur_nation_ptr->ai_attack_target_y_loc, 1 );
}
//----------- End of static function disp_other_info -----------//

