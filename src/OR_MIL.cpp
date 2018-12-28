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

//Filename    : OINFOMIL.CPP
//Description : Economy information screen

#include <OVGA.h>
#include <OFONT.h>
#include <OSPY.h>
#include <OIMGRES.h>
#include <OUNITRES.h>
#include <ORACERES.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <OF_CAMP.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OINFO.h>
#include "gettext.h"

//------------- Define coordinations -----------//

enum { TROOP_BROWSE_X1 = ZOOM_X1+6,
		 TROOP_BROWSE_Y1 = ZOOM_Y1+6,
		 TROOP_BROWSE_X2 = ZOOM_X2-6,
		 TROOP_BROWSE_Y2 = TROOP_BROWSE_Y1+240
	  };

enum { UNIT_BROWSE_X1 = ZOOM_X1+6,
		 UNIT_BROWSE_Y1 = TROOP_BROWSE_Y2+6,
		 UNIT_BROWSE_X2 = ZOOM_X2-6,
		 UNIT_BROWSE_Y2 = ZOOM_Y2-6,
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_troop;
static VBrowseIF browse_unit;

//----------- Define static functions ----------//

static void put_troop_rec(int recNo, int x, int y, int refreshFlag);
static void put_unit_rec(int recNo, int x, int y, int refreshFlag);
static int  troop_filter(int recNo=0);
static int  unit_filter(int recNo=0);
static void	disp_troop_total();
static void disp_unit_total();
static void put_heading(char justify, int x1, int y1, int x2, int y2, const char *textPtr);

#define J_L Font::LEFT_JUSTIFY
#define J_C Font::CENTER_JUSTIFY
#define J_R Font::RIGHT_JUSTIFY

//--------- Begin of function Info::disp_military ---------//
//
void Info::disp_military(int refreshFlag)
{
	//------- Display the Troop report -------//

	int x=TROOP_BROWSE_X1+9;
	int y=TROOP_BROWSE_Y1+4;

	vga_back.d3_panel_up(TROOP_BROWSE_X1, TROOP_BROWSE_Y1, TROOP_BROWSE_X2, TROOP_BROWSE_Y1+33 );

	#if(defined(FRENCH))
		font_san.put( x	 , y+7 , "Commander" );
		font_san.put( x+170, y+7,  "Leadership" );
		font_san.put( x+275, y+7,  "Loyalty" );
		font_san.put( x+350, y,    "Points" );
		font_san.put( x+350, y+13, "de vie" );
		font_san.put( x+410, y+7,  "Garnison" );
		font_san.put( x+480, y+7, "Status" );
	#elif(defined(GERMAN))
		font_san.put( x	 , y+7 , "Commander" );
		font_san.put( x+210, y+7 , "Leadership" );
		font_san.put( x+275, y+7 , "Loyalty" );
		font_san.put( x+342, y+7 , "Hit Points" );
		font_san.put( x+406, y   , "Commanded" );
		font_san.put( x+415, y+13, "Soldiers" );
		font_san.put( x+485, y+7 , "Status" );
	#else
		put_heading( J_L, x    , y, x+200, y+29, _("Commander") );
		put_heading( J_C, x+200, y, x+285, y+29, _("Leadership") );
		put_heading( J_L, x+285, y, x+342, y+29, _("Loyalty") );
		put_heading( J_C, x+342, y, x+406, y+29, _("Hit Points") );
		put_heading( J_C, x+406, y, x+490, y+29, _("Commanded Soldiers") );
		put_heading( J_R, x+490, y, TROOP_BROWSE_X2-20, y+29, _("Status") );
	#endif

	if( refreshFlag == INFO_REPAINT )
	{
		browse_troop.init( TROOP_BROWSE_X1, TROOP_BROWSE_Y1+35, TROOP_BROWSE_X2, TROOP_BROWSE_Y2-20,
								 0, 22, troop_filter(), put_troop_rec, 1 );

		browse_troop.open(browse_troop_recno);
	}
	else
	{
		browse_troop.paint();
		browse_troop.open(browse_troop_recno, troop_filter());
	}

	disp_troop_total();

	//------- Display the unit report -------//

	x=UNIT_BROWSE_X1+9;
	y=UNIT_BROWSE_Y1+4;

	vga_back.d3_panel_up(UNIT_BROWSE_X1, UNIT_BROWSE_Y1, UNIT_BROWSE_X2, UNIT_BROWSE_Y1+20 );

	font_san.put( x	 , y, _("Unit Type") );
	font_san.put( x+300, y, _("No. of Units") );

	if( refreshFlag == INFO_REPAINT )
	{
		browse_unit.init( UNIT_BROWSE_X1, UNIT_BROWSE_Y1+22, UNIT_BROWSE_X2, UNIT_BROWSE_Y2-20,
								0, 16, unit_filter(), put_unit_rec, 1 );

		browse_unit.open(browse_unit_recno);
	}
	else
	{
		browse_unit.paint();
		browse_unit.open(browse_unit_recno, unit_filter());
	}

	disp_unit_total();
}
//----------- End of function Info::disp_military -----------//


//--------- Begin of function Info::detect_military ---------//
//
void Info::detect_military()
{
	//------- detect the troop report -------//

	if( browse_troop.detect() )
	{
		browse_troop_recno = browse_troop.recno();

		if( browse_troop.double_click )
		{
			Unit* unitPtr = unit_array[ troop_filter(browse_troop_recno) ];
			short xLoc, yLoc;

			if( unitPtr->get_cur_loc(xLoc, yLoc) )
			{
				world.go_loc(xLoc, yLoc, 1);
			}
		}
	}

	//------- detect the unit report -------//

	if( browse_unit.detect() )
	{
		browse_unit_recno = browse_unit.recno();
	}
}
//----------- End of function Info::detect_military -----------//


//-------- Begin of static function disp_troop_total --------//

static void disp_troop_total()
{
	//---- count the total no. of soldiers ----//

	Unit* unitPtr;
	int   totalSoldier=0, totalCommandedSoldier=0;

	int i;
	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno != info.viewing_nation_recno )
			continue;

		if( unitPtr->rank_id == RANK_SOLDIER )
			totalSoldier++;
		else
			totalCommandedSoldier += unitPtr->commanded_soldier_count();
	}

	Nation* viewingNation = nation_array[info.viewing_nation_recno];

	for( i=viewingNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		totalSoldier += firm_array[ viewingNation->ai_camp_array[i] ]->worker_count;
	}

	//--------- paint the area ----------//

	int x=TROOP_BROWSE_X1+9;
	int y=TROOP_BROWSE_Y2-16;

	vga_back.d3_panel_up(TROOP_BROWSE_X1, TROOP_BROWSE_Y2-18, TROOP_BROWSE_X2, TROOP_BROWSE_Y2 );

	//------ display commander count ------//

	String str;

	snprintf( str, MAX_STR_LEN+1, _("Total Commanders: %s"), misc.format(browse_troop.total_rec()) );

	font_san.put( x, y, str );

	//------ display soldiers under command count ------//

	snprintf( str, MAX_STR_LEN+1, _("Total Soldiers Under Command: %s"), misc.format(totalCommandedSoldier) );

	font_san.put( x+160, y, str );

	//------ display soldiers count ------//

	snprintf( str, MAX_STR_LEN+1, _("Total Soldiers: %s"), misc.format(totalSoldier) );

#if(defined(FRENCH))
	font_san.put( x+396, y, str );
#else
	font_san.put( x+410, y, str );
#endif
}
//----------- End of static function disp_troop_total -----------//


//-------- Begin of static function disp_unit_total --------//

static void disp_unit_total()
{
	int totalUnitCount=0, totalUnitCost=0;

	UnitInfo* unitInfo;

	for( int unitId=1 ; unitId<=MAX_UNIT_TYPE ; unitId++ )
	{
		unitInfo = unit_res[unitId];

		int unitCount    = unitInfo->nation_unit_count_array[info.viewing_nation_recno-1];
		int generalCount = unitInfo->nation_general_count_array[info.viewing_nation_recno-1];

		totalUnitCount += unitCount;

		if( unitInfo->race_id )
		{
			totalUnitCost += generalCount * GENERAL_YEAR_SALARY;
			totalUnitCost += (unitCount-generalCount) * SOLDIER_YEAR_SALARY;
		}
		else
			totalUnitCost += unitCount * unitInfo->year_cost;
	}


	//--------- paint the area ----------//

	int x=UNIT_BROWSE_X1+9;
	int y=UNIT_BROWSE_Y2-16;

	vga_back.d3_panel_up(UNIT_BROWSE_X1, UNIT_BROWSE_Y2-18, UNIT_BROWSE_X2, UNIT_BROWSE_Y2 );

	String str;

	snprintf( str, MAX_STR_LEN+1, _("Total Units: %s"), misc.format(totalUnitCount) );

	font_san.put( x, y, str );
}
//----------- End of static function disp_unit_total -----------//


//-------- Begin of static function troop_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int troop_filter(int recNo)
{
	int   i, unitCount=0, totalUnit=unit_array.size();
	Unit* unitPtr;

	for( i=1 ; i<=totalUnit ; i++ )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno == info.viewing_nation_recno &&
			 (unitPtr->rank_id==RANK_GENERAL || unitPtr->rank_id==RANK_KING) )
		{
			unitCount++;
		}

		if( recNo && unitCount==recNo )
			return i;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return unitCount;
}
//----------- End of static function troop_filter -----------//


//-------- Begin of static function put_troop_rec --------//
//
static void put_troop_rec(int recNo, int x, int y, int refreshFlag)
{
	int   unitRecno = troop_filter(recNo);
	Unit* unitPtr   = unit_array[unitRecno];

	//---------- display bitmap ----------//

	x+=3;
	y+=3;

	vga_back.put_bitmap( x, y-2, unit_res[unitPtr->unit_id]->get_small_icon_ptr(unitPtr->rank_id) );

	//-------- display name & team size -------//

	y+=2;

	font_san.put( x+28 , y, unitPtr->unit_name(), 0, 238 );
	font_san.put( x+240, y, unitPtr->skill.get_skill(SKILL_LEADING) );

	if( unitPtr->rank_id != RANK_KING )
		font_san.put( x+295, y, unitPtr->loyalty );

	//--------- display hit points -----------//

	String str;

	str  = (int) unitPtr->hit_points;
	str += "/";
	str += unitPtr->max_hit_points;

	font_san.put( x+352, y, str );

	//---- display the no. of soldiers led by this general ---//

	font_san.put( x+445, y, unitPtr->commanded_soldier_count() );

	//---- display the status of the general ----//

	const char* statusStr;

	if( unitPtr->unit_mode == UNIT_MODE_OVERSEE )
	{
		int firmId = firm_array[ unitPtr->unit_mode_para ]->firm_id;

		if( firmId == FIRM_CAMP )
			statusStr = _("In Fort");

		else if( firmId == FIRM_BASE )
			statusStr = _("In Seat");

		else
			err_here();
	}
	else if( unitPtr->unit_mode == UNIT_MODE_ON_SHIP )
	{
		statusStr = _("On Ship");
	}
	else
	{
		statusStr = _("Mobile");
	}

#if(defined(FRENCH))
	font_san.put( x+470, y, statusStr );
#else
	// German and US
	font_san.put( x+486, y, statusStr );
#endif
}
//----------- End of static function put_troop_rec -----------//


//-------- Begin of static function unit_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int unit_filter(int recNo)
{
	int 		 unitTypeCount=0;
	UnitInfo* unitInfo;

	//------- count human units -------//

	Nation* nationPtr = nation_array[info.viewing_nation_recno];

	unitTypeCount++;

	if( recNo && unitTypeCount==recNo )
		return -1;

	//------- count non-human unit types -------//

	for( int unitId=1 ; unitId<=MAX_UNIT_TYPE ; unitId++ )
	{
		unitInfo = unit_res[unitId];

		//---- if this is not a human unit -----//

		if( !unitInfo->race_id &&
			 unitInfo->nation_unit_count_array[info.viewing_nation_recno-1] > 0 )
		{
			unitTypeCount++;
		}

		if( recNo && unitTypeCount==recNo )
			return unitId;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return unitTypeCount;
}
//----------- End of static function unit_filter -----------//


//-------- Begin of static function put_unit_rec --------//
//
static void put_unit_rec(int recNo, int x, int y, int refreshFlag)
{
	int   	 unitId = unit_filter(recNo);
	int   	 unitCount;
	const char* 	 str;
	UnitInfo* unitInfo;
	Nation* nationPtr = nation_array[info.viewing_nation_recno];
	int rc = unit_filter(recNo);

	switch( rc )
	{
		//------- count human units -------//

		case -1:
			str		  = _("Human General");
			unitCount  = nationPtr->total_general_count;
			break;

		default:
			unitInfo   = unit_res[rc];
			str 		  = _(unitInfo->name);
			unitCount  = unitInfo->nation_unit_count_array[ info.viewing_nation_recno-1 ];
	}

	//---------- display info --------//

	x+=3;
	y+=3;

	font_san.put( x   , y, str );
	font_san.put( x+320, y, misc.format(unitCount,1) );
}
//----------- End of static function put_unit_rec -----------//


//-------- Begin of static function put_heading --------//
//
static void put_heading(char justify, int x1, int y1, int x2, int y2, const char *textPtr)
{
	int dispLines=0;
	int totalLines=0;
	font_san.count_line(x1,y1,x2,y2,textPtr,0,dispLines,totalLines);
	if( dispLines > 1 )
		font_san.put_paragraph(x1,y1,x2,y2,textPtr,0,1,1,justify);
	else if( y1+7<y2 )
		font_san.put_paragraph(x1,y1+7,x2,y2,textPtr,0,1,1,justify);
}
//----------- End of static function put_heading -----------//
