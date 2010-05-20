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

//Filename    : OR_TECH.CPP
//Description : Technology Report

#include <OVGA.h>
#include <vga_util.h>
#include <OFONT.h>
#include <OIMGRES.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <OTECHRES.h>
#include <OGODRES.h>
#include <ORACERES.h>
#include <ONATION.h>
#include <OU_GOD.h>
#include <OINFO.h>

//------------- Define coordinations -----------//

enum { TECH_BROWSE_X1 = ZOOM_X1+6,
		 TECH_BROWSE_Y1 = ZOOM_Y1+6,
		 TECH_BROWSE_X2 = ZOOM_X2-6,
		 TECH_BROWSE_Y2 = TECH_BROWSE_Y1+220
	  };

enum { SCROLL_X1 = ZOOM_X1+6,
		 SCROLL_Y1 = TECH_BROWSE_Y2+6,
		 SCROLL_X2 = ZOOM_X2-6,
		 SCROLL_Y2 = SCROLL_Y1+80
	  };

enum { GOD_BROWSE_X1 = ZOOM_X1+6,
		 GOD_BROWSE_Y1 = SCROLL_Y2+6,
		 GOD_BROWSE_X2 = ZOOM_X2-6,
		 GOD_BROWSE_Y2 = ZOOM_Y2-6
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_tech, browse_god;

//----------- Define static functions ----------//

static int  tech_filter(int recNo=0);
static int  god_filter(int recNo=0);
static void put_tech_rec(int recNo, int x, int y, int refreshFlag);
static void put_god_rec(int recNo, int x, int y, int refreshFlag);
static void disp_owned_scroll();
static void disp_scroll(int x, int y, int raceId);

//--------- Begin of function Info::disp_tech ---------//
//
void Info::disp_tech(int refreshFlag)
{
	//-------- display the technology browser ---------//

	int x=TECH_BROWSE_X1+9;
	int y=TECH_BROWSE_Y1+4;

	vga_back.d3_panel_up(TECH_BROWSE_X1, TECH_BROWSE_Y1, TECH_BROWSE_X2, TECH_BROWSE_Y1+32 );

	font_san.put( x	 , y+7, "Technology" );

#if(defined(SPANISH))
	font_san.put( x+160, y   , "Version" );
	font_san.put( x+160, y+13, "Present");

	font_san.put( x+245, y   , "Version" );
	font_san.put( x+230, y+13, "Researching");

	font_san.put( x+320, y+7, "Research Progress" );

	font_san.put( x+460, y   , "Tower of" );
	font_san.put( x+468, y+13, "Science" );
#elif(defined(FRENCH))
	font_san.put( x+160, y   , "Catégorie" );
	font_san.put( x+160, y+13, "Actuelle" );

	font_san.put( x+230, y   , "Catégorie" );
	font_san.put( x+230, y+13, "Recherchée" );

	font_san.put( x+320, y,    "Etat de la" );
	font_san.put( x+320, y+13, "Recherche");

	font_san.put( x+460, y   , "Tour du" );
	font_san.put( x+462, y+13, "Savoir" );
#else
	// German and US
	font_san.put( x+160, y   , "Present" );
	font_san.put( x+160, y+13, "Version" );

	font_san.put( x+230, y   , "Researching" );
	font_san.put( x+245, y+13, "Version" );

	font_san.put( x+320, y+7, "Research Progress" );

	font_san.put( x+460, y   , "Tower of" );
	font_san.put( x+462, y+13, "Science" );
#endif

	if( refreshFlag == INFO_REPAINT )
	{
		browse_tech.init( TECH_BROWSE_X1, TECH_BROWSE_Y1+34, TECH_BROWSE_X2, TECH_BROWSE_Y2,
								0, 22, tech_filter(), put_tech_rec, 1 );

		browse_tech.open(browse_tech_recno);
	}
	else
	{
		browse_tech.paint();
		browse_tech.open(browse_tech_recno, tech_filter());
	}

	//----- display the list of acquired scrolls of power ----//

	disp_owned_scroll();

	//-------- display the god unit browser ---------//

	x=GOD_BROWSE_X1+9;
	y=GOD_BROWSE_Y1+4;

	vga_back.d3_panel_up(GOD_BROWSE_X1, GOD_BROWSE_Y1, GOD_BROWSE_X2, GOD_BROWSE_Y1+20 );

	font_san.put( x	 , y, "Greater Being" );
	font_san.put( x+300, y, "Hit Points" );

	if( refreshFlag == INFO_REPAINT )
	{
		browse_god.init( GOD_BROWSE_X1, GOD_BROWSE_Y1+22, GOD_BROWSE_X2, GOD_BROWSE_Y2,
							  0, 22, god_filter(), put_god_rec, 1 );

		browse_god.open(browse_god_recno);
	}
	else
	{
		browse_god.paint();
		browse_god.open(browse_god_recno, god_filter());
	}
}
//----------- End of function Info::disp_tech -----------//


//--------- Begin of function Info::detect_tech ---------//
//
void Info::detect_tech()
{
	if( browse_tech.detect() )
		browse_tech_recno = browse_tech.recno();

	if( browse_god.detect() )
	{
		browse_god_recno = browse_god.recno();

		if( browse_god.double_click )
		{
			Unit* unitPtr = unit_array[ god_filter(browse_god.recno()) ];

			world.go_loc( unitPtr->next_x_loc(), unitPtr->next_y_loc(), 1 );
		}
	}
}
//----------- End of function Info::detect_tech -----------//


//-------- Begin of static function tech_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int tech_filter(int recNo)
{
	int 		 techCount=0;
	TechInfo* techInfo;

	for( int techId=tech_res.tech_count ; techId>0 ; techId-- )
	{
		techInfo = tech_res[techId];

		if( techInfo->is_nation_researching(info.viewing_nation_recno) ||
			 techInfo->get_nation_tech_level(info.viewing_nation_recno) > 0 )
		{
			techCount++;
		}

		if( recNo && techCount==recNo )
			return techId;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return techCount;
}
//----------- End of static function tech_filter -----------//


//-------- Begin of static function god_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int god_filter(int recNo)
{
	int 	godCount=0, totalUnit=unit_array.size();
	Unit* unitPtr;

	for( int i=1 ; i<=totalUnit ; i++ )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno == info.viewing_nation_recno &&
			 unit_res[ unitPtr->unit_id ]->unit_class == UNIT_CLASS_GOD )
		{
			godCount++;
		}

		if( recNo && godCount==recNo )
			return i;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return godCount;
}
//----------- End of static function god_filter -----------//


//-------- Begin of static function put_tech_rec --------//
//
static void put_tech_rec(int recNo, int x, int y, int refreshFlag)
{
	int   	 techId   = tech_filter(recNo);
	TechInfo* techInfo = tech_res[techId];

	//---------- display bitmap ----------//

	x+=3;
	y+=3;

	vga_back.put_bitmap( x, y-2, techInfo->tech_small_icon() );

	//----------- display info ----------//

	y+=2;

	int curLevel = techInfo->get_nation_tech_level(info.viewing_nation_recno);

	font_san.put( x+28 , y, techInfo->tech_des() );

	if( curLevel > 0 )
		font_san.put( x+180, y, m.roman_number(curLevel) );

	//----- if the nation is researching this technology -----//

	int isResearching = techInfo->is_nation_researching(info.viewing_nation_recno);

	if( isResearching )
	{
		err_when( curLevel >= techInfo->max_tech_level );

		font_san.put( x+260, y, m.roman_number(curLevel+1) );
		font_san.put( x+480, y, isResearching );		//isResearching tells the no. of towers of science researching this technology

		//----- display the research progress bar -----//

		vga_util.d3_panel_down( x+320, y-2, x+440, y+14 );
		vga_back.indicator( x+321, y-1, x+439, y+13, techInfo->get_progress(info.viewing_nation_recno), (float)100, VGA_GRAY );
	}
}
//----------- End of static function put_tech_rec -----------//


//-------- Begin of static function put_god_rec --------//
//
static void put_god_rec(int recNo, int x, int y, int refreshFlag)
{
	UnitGod* unitGod = (UnitGod*) unit_array[ god_filter(recNo) ];

	err_when( unit_res[unitGod->unit_id]->unit_class != UNIT_CLASS_GOD );

	//----------- display info ----------//

	x+=3;
	y+=5;

	font_san.put( x, y, unit_res[unitGod->unit_id]->name );

	//--------- display hit points -----------//

	String str;

	str  = (int) unitGod->hit_points;
	str += "/";
	str += unitGod->max_hit_points;

	font_san.put( x+300, y, str );
}
//----------- End of static function put_god_rec -----------//


//-------- Begin of static function disp_scroll --------//
//
static void disp_scroll(int x, int y, int raceId)
{
	char iconName[]="SCROLL-0";

	iconName[7] = race_res[raceId]->code[0];

	image_spict.put_back( x, y, iconName );
}
//----------- End of static function disp_scroll -----------//


//-------- Begin of static function disp_owned_scroll --------//
//
static void disp_owned_scroll()
{
	vga_util.d3_panel_down( SCROLL_X1, SCROLL_Y1, SCROLL_X2, SCROLL_Y2 );

	//------ count the number of acquired scrolls ------//

	Nation* nationPtr = nation_array[info.viewing_nation_recno];
	int scrollCount=0;

	int i;
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( nationPtr->know_base_array[i] )
			scrollCount++;
	}

	//------- display words -------//

	if( scrollCount > 1 )
		font_san.put( SCROLL_X1+6, SCROLL_Y1+5, "Acquired Scrolls:" );
	else
		font_san.put( SCROLL_X1+6, SCROLL_Y1+5, "Acquired Scroll:" );

	//------- display scrolls ----------//

	int x=SCROLL_X1+6, y=SCROLL_Y1+18;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( !nationPtr->know_base_array[i] )
			continue;

		disp_scroll(x, y, i+1 );

		font_san.put( x+36, y+6, race_res[i+1]->name );

		x+=105;

		if( x+95 > SCROLL_X2 )
		{
			x  = SCROLL_X1+6;
			y += 30;
		}
	}
}
//----------- End of static function disp_owned_scroll -----------//

