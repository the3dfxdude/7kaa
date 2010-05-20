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

//Filename    : OF_HARB.CPP
//Description : Firm Harbor

#include <OINFO.h>
#include <OVGA.h>
#include <vga_util.h>
#include <ODATE.h>
#include <OHELP.h>
#include <OMOUSE.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <OVBROWIF.h>
#include <OTERRAIN.h>
#include <OPOWER.h>
#include <OREGIONS.h>
#include <OU_MARI.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OF_HARB.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OBUTTCUS.h>
#include <OSE.h>
#include <OSERES.h>


//------------- Define coordinations -----------//

enum { SHIP_BROWSE_X1 = INFO_X1,
		 SHIP_BROWSE_Y1 = INFO_Y1+54,
		 SHIP_BROWSE_X2 = INFO_X2,
		 SHIP_BROWSE_Y2 = SHIP_BROWSE_Y1+100
	  };

enum { SHIP_DET_X1 = INFO_X1,
		 SHIP_DET_Y1 = SHIP_BROWSE_Y2+5,
		 SHIP_DET_X2 = INFO_X2,
		 SHIP_DET_Y2 = SHIP_DET_Y1+92
	  };

//---------- Define constant ------------//

enum { HARBOR_MENU_MAIN,
		 HARBOR_MENU_BUILD,
	  };

// ######## begin Gilbert 22/9 #######//
#define BUILD_BUTTON_WIDTH      205
#define BUILD_BUTTON_HEIGHT     48
#define COUNT_BUTTON_OFFSET_X   160
#define COUNT_BUTTON_OFFSET_Y 8
// ######## end Gilbert 22/9 #######//
#define COUNT_BUTTON_WIDTH		32
#define COUNT_BUTTON_HEIGHT	32

//----------- Define static variables ----------//

static VBrowseIF 		browse_ship;
static Button3D		button_build, button_sail;
static Button3D		button_cancel_build;
static ButtonGroup	button_mode(2);
// ###### begin Gilbert 20/9 #######//
// static short                 button_unit_id[MAX_SHIP_TYPE];
static ButtonCustom     button_ship[MAX_SHIP_TYPE];
static ButtonCustom     button_queue_ship[MAX_SHIP_TYPE];
// ###### end Gilbert 20/9 #######//
static int				added_count;
static ButtonCustom	button_cancel;
static int				last_ship_count;
static FirmHarbor* 	firm_harbor_ptr;
static char       	harbor_menu_mode;
static char       	disable_refresh=0;
static char				ship_disp_mode=SHIP_MENU_UNIT;

//----------- Define static functions ----------//

static void put_ship_rec(int recNo, int x, int y, int refreshFlag);
static void i_disp_build_button(ButtonCustom *button, int);
static void i_disp_queue_button(ButtonCustom *button, int);

//--------- Begin of function FirmHarbor::FirmHarbor ---------//
//
FirmHarbor::FirmHarbor()
{
	ship_count = 0;
	build_unit_id = 0;
	build_queue_count = 0;

	land_region_id = 0;
	sea_region_id = 0;
	link_checked = 0;
	linked_mine_num = linked_factory_num = linked_market_num = 0;
	memset(linked_mine_array, 0, sizeof(short)*MAX_LINKED_FIRM_FIRM);
	memset(linked_factory_array, 0, sizeof(short)*MAX_LINKED_FIRM_FIRM);
	memset(linked_market_array, 0, sizeof(short)*MAX_LINKED_FIRM_FIRM);
}
//----------- End of function FirmHarbor::FirmHarbor -----------//


//--------- Begin of function FirmHarbor::deinit_derived ---------//
//
void FirmHarbor::deinit_derived()
{
	int shipUnitRecno;

	for( int i=ship_count ; i>0 ; i-- )
	{
		shipUnitRecno = ship_recno_array[i-1];

		//---- mobilize ships in the harbor ----//

		sail_ship( shipUnitRecno, COMMAND_AUTO );

		//--- if the ship does not sailed out due to no limit sea space, delete it  ---//

		if( ship_count==i )		
		{
			del_hosted_ship(shipUnitRecno);			
			unit_array.del(shipUnitRecno);
		}
	}
}
//----------- End of function FirmHarbor::deinit_derived() -----------//


//--------- Begin of function FirmHarbor::init ---------//
//
void FirmHarbor::init(int xLoc, int yLoc, int nationRecno, int firmId, const char* buildCode, short builderRecno)
{
	// ignore raceId and find north, south, west or east harbor

	if( world.get_loc(xLoc+1, yLoc+2)->can_build_harbor(1) )
	{
		// check north harbour
		Firm::init(xLoc, yLoc, nationRecno, firmId, "N", builderRecno);
		land_region_id = world.get_loc(xLoc+1, yLoc+2)->region_id;
		sea_region_id = world.get_loc(xLoc+1, yLoc)->region_id;
	}
	else if( world.get_loc(xLoc+1, yLoc)->can_build_harbor(1) )
	{
		// check south harbour
		Firm::init(xLoc, yLoc, nationRecno, firmId, "S", builderRecno);
		land_region_id = world.get_loc(xLoc+1, yLoc)->region_id;
		sea_region_id = world.get_loc(xLoc+1, yLoc+2)->region_id;
	}
	else if( world.get_loc(xLoc+2, yLoc+1)->can_build_harbor(1) )
	{
		// check west harbour
		Firm::init(xLoc, yLoc, nationRecno, firmId, "W", builderRecno);
		land_region_id = world.get_loc(xLoc+2, yLoc+1)->region_id;
		sea_region_id = world.get_loc(xLoc, yLoc+1)->region_id;
	}
	else if( world.get_loc(xLoc, yLoc+1)->can_build_harbor(1) )
	{
		// check east harbour
		Firm::init(xLoc, yLoc, nationRecno, firmId, "E", builderRecno);
		land_region_id = world.get_loc(xLoc, yLoc+1)->region_id;
		sea_region_id = world.get_loc(xLoc+2, yLoc+1)->region_id;
	}
	else
	{
		err_here();
	}

	region_id = land_region_id;		// set region_id to land_region_id

	//------- update the harbor count of the regions ------//

	region_array.update_region_stat();
}
//----------- End of function FirmHarbor::init -----------//


//------- Begin of function FirmHarbor::assign_unit -----------//
//
void FirmHarbor::assign_unit(int unitRecno)
{
	//------- if this is a construction worker -------//

	if( unit_array[unitRecno]->skill.skill_id == SKILL_CONSTRUCTION )
	{
		set_builder(unitRecno);
		return;
	}

	//---------------------------------------//

	//if( ship_count==MAX_SHIP_IN_HARBOR )
	//	return;
	if(ship_count + (build_unit_id>0) == MAX_SHIP_IN_HARBOR)
		return; // leave a room for the building unit

	add_hosted_ship(unitRecno);

	unit_array[unitRecno]->deinit_sprite();

	UnitMarine *unitPtr = (UnitMarine*) unit_array[unitRecno];
	unitPtr->extra_move_in_beach = NO_EXTRA_MOVE;
	unitPtr->deinit_sprite();

	//------------------------------------------------//

	if( firm_array.selected_recno == firm_recno )
		info.disp();
}
//--------- End of function FirmHarbor::assign_unit -----------//


//--------- Begin of function FirmHarbor::next_day ---------//
//
void FirmHarbor::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//------- process building -------//

	if( build_unit_id )
		process_build();
	else
		process_queue();

	//-*********** simulate ship movement ************-//
	//if(build_unit_id==0)
	//	build_ship(UNIT_CARAVEL, 0);
	//-*********** simulate ship movement ************-//
}
//----------- End of function FirmHarbor::next_day -----------//


//--------- Begin of function FirmHarbor::put_info ---------//
//
void FirmHarbor::put_info(int refreshFlag)
{
	if( refreshFlag==INFO_REPAINT && !disable_refresh )
		harbor_menu_mode = HARBOR_MENU_MAIN;

	switch( harbor_menu_mode )
	{
		case HARBOR_MENU_MAIN:
			disp_main_menu(refreshFlag);
			break;

		case HARBOR_MENU_BUILD:
			disp_build_menu(refreshFlag);
			break;
	}
}
//----------- End of function FirmHarbor::put_info -----------//


//--------- Begin of function FirmHarbor::detect_info ---------//
//
void FirmHarbor::detect_info()
{
	switch( harbor_menu_mode )
	{
		case HARBOR_MENU_MAIN:
			detect_main_menu();
			break;

		case HARBOR_MENU_BUILD:
			detect_build_menu();
			break;
	}
}
//----------- End of function FirmHarbor::detect_info -----------//


//--------- Begin of function FirmHarbor::disp_main_menu ---------//
//
void FirmHarbor::disp_main_menu(int refreshFlag)
{
	firm_harbor_ptr = this;

	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_harbor_info() )
		return;

	//-------- display browser -----------//

	if( refreshFlag == INFO_REPAINT )
	{
		browse_ship.init( SHIP_BROWSE_X1, SHIP_BROWSE_Y1, SHIP_BROWSE_X2, SHIP_BROWSE_Y2,
								0, 25, ship_count, put_ship_rec );

		browse_ship.open(1);

		put_det(INFO_REPAINT);
	}
	else
	{
		if( last_ship_count != ship_count )
		{
			last_ship_count = ship_count;
			info.disp();
		}
		else
			browse_ship.update();          // update only
	}

	last_ship_count = ship_count;

   //-------------------------------//

	if( !own_firm() )
		return;

	//-------- display buttons ---------//

	if( refreshFlag == INFO_REPAINT )
	{
		button_build.paint( SHIP_DET_X1, SHIP_DET_Y2+4, 'A', "MAKESHIP" );
		button_sail.paint ( SHIP_DET_X1+BUTTON_ACTION_WIDTH, SHIP_DET_Y2+4, 'A', "SAILOUT" );
	}

	if( ship_count > 0 )
		button_sail.enable();
	else
		button_sail.disable();

	//------ display the info of the ship under construction ------//

	disp_build_info(refreshFlag);
}
//----------- End of function FirmHarbor::disp_main_menu -----------//


//--------- Begin of function FirmHarbor::detect_main_menu ---------//
//
void FirmHarbor::detect_main_menu()
{
	firm_harbor_ptr = this;

	if( detect_basic_info() )
		return;

	if( !own_firm() )
		return;

	if( browse_ship.detect() )
		put_det(INFO_UPDATE);

	if( detect_det() )
		return;

	//------- detect the build button ---------//

	if( button_build.detect() )
	{
		harbor_menu_mode = HARBOR_MENU_BUILD;
		disable_refresh = 1;    // static var for disp_info() only
		info.disp();
		disable_refresh = 0;
	}

	//-------- detect the sail button ---------//

	if( button_sail.detect() && browse_ship.recno() > 0 )
		sail_ship( ship_recno_array[browse_ship.recno()-1], COMMAND_PLAYER );

	//---------- detect cancel build button -----------//
	if(build_unit_id)
	{
		if(button_cancel_build.detect())
		{
#ifdef USE_DPLAY
			if( !remote.is_enable() )
#endif
				cancel_build_unit();
#ifdef USE_DPLAY
			else
			{
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_HARBOR_SKIP_SHIP, sizeof(short));
				shortPtr[0] = firm_recno;
			}
#endif
		}
	}
}
//----------- End of function FirmHarbor::detect_main_menu -----------//


//------- Begin of function FirmHarbor::should_show_harbor_info -------//
//
int FirmHarbor::should_show_harbor_info()
{
	if( should_show_info() )
		return 1;

	//--- if any of the ships in the harbor has the spies of the player ---//

	UnitMarine* unitMarine;

	for( int i=0 ; i<ship_count ; i++ )
	{
		unitMarine = (UnitMarine*) unit_array[ ship_recno_array[i] ];

		if( unitMarine->should_show_info() )
			return 1;
	}

	return 0;
}
//------- End of function FirmHarbor::should_show_harbor_info ---------//


//--------- Begin of function FirmHarbor::put_det ---------//
//
void FirmHarbor::put_det(int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( SHIP_DET_X1, SHIP_DET_Y1, SHIP_DET_X2, SHIP_DET_Y2 );
	else
		vga_util.blt_buf( SHIP_DET_X1, SHIP_DET_Y1, SHIP_DET_X2, SHIP_DET_Y2, 0 );

	if( browse_ship.none_record )
		return;

	refreshFlag = INFO_REPAINT;		// for calling disp_ship_units() and disp_ship_goods()

	//------- display info of the ship --------//

	UnitMarine* shipUnitPtr  = (UnitMarine*) unit_array[ ship_recno_array[browse_ship.recno()-1] ];
	UnitInfo*   shipUnitInfo = unit_res[shipUnitPtr->unit_id];

	//----- if the ship can carry both goods and units, display mode selection button ---//

	if( shipUnitInfo->carry_goods_capacity > 0 &&
		 shipUnitInfo->carry_unit_capacity > 0 )
	{
		button_mode[0].create_text( SHIP_DET_X1+10, SHIP_DET_Y1+3, SHIP_DET_X1+80, SHIP_DET_Y1+19, "Units" );
		button_mode[1].create_text( SHIP_DET_X1+90, SHIP_DET_Y1+3, SHIP_DET_X2-10, SHIP_DET_Y1+19, "Goods" );
		button_mode.paint(ship_disp_mode);

		if( ship_disp_mode == SHIP_MENU_UNIT )
			disp_ship_units(shipUnitPtr, SHIP_DET_Y1+22, refreshFlag);
		else
			disp_ship_goods(shipUnitPtr, SHIP_DET_Y1+22, refreshFlag);
	}
	else if( shipUnitInfo->carry_goods_capacity > 0 )
	{
		disp_ship_goods(shipUnitPtr, SHIP_DET_Y1+22, refreshFlag);
	}
	else if( shipUnitInfo->carry_unit_capacity > 0 )
	{
		disp_ship_units(shipUnitPtr, SHIP_DET_Y1+22, refreshFlag);
	}
}
//----------- End of function FirmHarbor::put_det -----------//


//--------- Begin of function FirmHarbor::detect_det ---------//
//
int FirmHarbor::detect_det()
{
	if( browse_ship.none_record )
		return 0;

	UnitMarine* shipUnitPtr  = (UnitMarine*) unit_array[ ship_recno_array[browse_ship.recno()-1] ];
	UnitInfo*   shipUnitInfo = unit_res[shipUnitPtr->unit_id];

	if( shipUnitInfo->carry_goods_capacity > 0 &&
		 shipUnitInfo->carry_unit_capacity > 0 )
	{
		if( button_mode.detect() >= 0 )
		{
			ship_disp_mode = button_mode.button_pressed;
			vga_util.blt_buf( SHIP_DET_X1, SHIP_DET_Y1+22, SHIP_DET_X2, SHIP_DET_Y2, 0 );
			put_det(INFO_UPDATE);
			return 1;
		}
	}

	return 0;
}
//----------- End of function FirmHarbor::detect_det -----------//


//--------- Begin of function FirmHarbor::disp_ship_goods ---------//
//
// Display goods carried by the selected ship.
//
void FirmHarbor::disp_ship_goods(UnitMarine* shipUnit, int dispY1, int refreshFlag)
{
	int	x=INFO_X1+20, y=dispY1+5;
	String str;

	int i;
	for(i=0; i<MAX_RAW; i++, x+=60)
	{
		vga_front.d3_panel_up( x, y, x+RAW_SMALL_ICON_WIDTH+5, y+RAW_SMALL_ICON_HEIGHT+5 );

		raw_res.put_small_raw_icon( x+3, y+3, i+1 );

		font_san.disp( x+25, y+2, shipUnit->raw_qty_array[i], 1, x+59 );
	}

	x =INFO_X1+20;
	y+=19;

	for( i=0; i<MAX_PRODUCT; i++, x+=60)
	{
		vga_front.d3_panel_up( x, y, x+RAW_SMALL_ICON_WIDTH+5, y+RAW_SMALL_ICON_HEIGHT+5 );

		raw_res.put_small_product_icon( x+3, y+3, i+1 );

		font_san.disp( x+25, y+2, shipUnit->product_raw_qty_array[i], 1, x+59 );
	}
}
//---------- End of function FirmHarbor::disp_ship_goods ----------//


//--------- Begin of function FirmHarbor::disp_ship_units ---------//
//
void FirmHarbor::disp_ship_units(UnitMarine* shipUnit, int dispY1, int refreshFlag)
{
	//------ display population composition -------//

	int	  x, y;
	Unit*   unitPtr;
	static  short last_unit_race_array[MAX_UNIT_IN_SHIP];

	dispY1+=5;

	for( int i=0 ; i<MAX_UNIT_IN_SHIP ; i++ )
	{
		x = INFO_X1+10+i%5*37;
		y = dispY1+i/5*29;

		if( i<shipUnit->unit_count )
		{
			unitPtr = unit_array[ shipUnit->unit_recno_array[i] ];

			if( refreshFlag==INFO_REPAINT || last_unit_race_array[i] != unitPtr->race_id )
			{
				vga_front.d3_panel_up( x, y, x+27, y+23, 1 );
				// ####### begin Gilbert 17/10 ##########//
				vga_front.put_bitmap(x+2, y+2, unit_res[unitPtr->unit_id]->get_small_icon_ptr(unitPtr->rank_id));
				// ####### end Gilbert 17/10 ##########//
			}

			//------- set help parameters ---------//

			if( mouse.in_area(x, y, x+27, y+23) )
				help.set_unit_help( unitPtr->unit_id, 0, x, y, x+27, y+23 );

			//------------------------------------//

			last_unit_race_array[i] = unitPtr->race_id;
		}
		else
		{
			if( last_unit_race_array[i] != 0 )
			{
				vga_util.blt_buf( x-2, y-2, x+36, y+25, 0 );
				last_unit_race_array[i] = 0;
			}
		}
	}
}
//----------- End of function FirmHarbor::disp_ship_units -----------//


//--------- Begin of function FirmHarbor::disp_build_menu ---------//
//
void FirmHarbor::disp_build_menu(int refreshFlag)
{
	// ###### begin Gilbert 20/9 ######//
	if( refreshFlag == INFO_UPDATE )
	{
		for( int b=0; b<added_count; ++b )
		{
			button_ship[b].paint(-1, 0);
			// button_queue_ship[b] is called by button_ship[b].paint();
		}
	}
	else if( refreshFlag == INFO_REPAINT )
	{
		added_count=0;
		int 	 unitId, x=INFO_X1, y=INFO_Y1;
		for( unitId=1; unitId<=MAX_UNIT_TYPE ; unitId++ )
		{
			if( unit_res[unitId]->unit_class == UNIT_CLASS_SHIP )
			{
				if( unit_res[unitId]->get_nation_tech_level(nation_recno) > 0 )
				{
					// disp_build_button( y, unitId, 1);
					button_queue_ship[added_count].create(x+COUNT_BUTTON_OFFSET_X, y+COUNT_BUTTON_OFFSET_Y,
						x+COUNT_BUTTON_OFFSET_X+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_OFFSET_Y+COUNT_BUTTON_HEIGHT-1,
						i_disp_queue_button, ButtonCustomPara(this, unitId) );
					button_ship[added_count].paint(x, y, x+BUILD_BUTTON_WIDTH-1, y+BUILD_BUTTON_HEIGHT-1,
						i_disp_build_button, ButtonCustomPara(&button_queue_ship[added_count], unitId) );

					err_when(added_count >= MAX_SHIP_TYPE);
					// button_unit_id[added_count++] = unitId;
					added_count++;
					y += BUILD_BUTTON_HEIGHT;
				}
			}
		}
		button_cancel.paint(x, y, x+BUILD_BUTTON_WIDTH-1, y+BUILD_BUTTON_HEIGHT*3/4,
			ButtonCustom::disp_text_button_func, ButtonCustomPara((void*)"Done",0) );
	}
	// ###### end Gilbert 20/9 ######//
}
//----------- End of function FirmHarbor::disp_build_menu -----------//


// ####### begin Gilbert 20/9 ########//
/*
//--------- Begin of function FirmHarbor::disp_build_button ---------//
void FirmHarbor::disp_build_button(int y, int unitId, int buttonUp)
{
	int x;
	int y0 = y;
	if(buttonUp)
	{
		vga_util.d3_panel_up(INFO_X1, y, INFO_X2-1, y+BUILD_BUTTON_HEIGHT-2);
		x = INFO_X1;
	}
	else
	{
		vga_util.d3_panel_down(INFO_X1, y, INFO_X2-1, y+BUILD_BUTTON_HEIGHT-2);
		x = INFO_X1+1;
		y++;
	}

	// display unit large icon
	UnitInfo* unitInfo = unit_res[unitId];
	vga_front.put_bitmap(x+6, y+4, unitInfo->get_large_icon_ptr(0));

	//-------- display unit name --------//

	String str;
	str = unitInfo->name;
	font_bible.put( x+60, y+14, str );
	disp_queue_button(y0+COUNT_BUTTON_OFFSET_Y, unitId, 1);
}
//----------- End of function FirmHarbor::disp_build_button -----------//
*/


//-------- Begin of function i_disp_build_button --------//
//
static void i_disp_build_button(ButtonCustom *button, int repaintBody)
{
	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	if( !button->pushed_flag )
	{
		if( repaintBody )
		{
			vga_util.blt_buf(x1, y1, x2, y2, 0);
			vga_util.d3_panel2_up( x1, y1, x2, y2, 1 );
		}
		x2--;
		y2--;
	}
	else
	{
		if( repaintBody )
		{
			vga_util.blt_buf(x1, y1, x2, y2, 0);
			vga_util.d3_panel2_down( x1, y1, x2, y2, 1 );
		}
		x1++;
		y1++;
	}

	ButtonCustom *queueButton = (ButtonCustom *)button->custom_para.ptr;
	if( repaintBody)
	{
		// display unit large icon
		short unitId = button->custom_para.value;
		UnitInfo* unitInfo = unit_res[unitId];
		vga_front.put_bitmap(x1+6, y1+4, unitInfo->get_large_icon_ptr(0));

		err_when( button->custom_para.value != queueButton->custom_para.value);

		//-------- display unit name --------//

		String str;
		str = unitInfo->name;

		if( unitInfo->unit_class == UNIT_CLASS_WEAPON )		// add version no.
		{
			FirmHarbor *harbor = (FirmHarbor *)queueButton->custom_para.ptr;
			int techLevel = unitInfo->get_nation_tech_level(harbor->nation_recno);

			if( techLevel > 1 )
			{
				str += " ";
				str += m.roman_number(techLevel);
			}
		}
		
		font_bible.put( x1+60, y1+13, str );
	}

	// display small button
	queueButton->paint(-1, repaintBody);
}
//--------- End of static function i_disp_build_button ---------//


/*
//--------- Begin of function FirmHarbor::disp_queue_button ---------//
void FirmHarbor::disp_queue_button(int y, int unitId, int buttonUp)
{
	//----- count the no. of units queued for this ship ------//
	int x=INFO_X1+2+COUNT_BUTTON_OFFSET_X;
	int queuedCount=0;

	for(int i=0; i<build_queue_count; i++)
	{
		if(build_queue_array[i] == unitId)
			queuedCount++;
	}

	if(build_unit_id==unitId)
		queuedCount++;

	if(buttonUp)
		vga_util.d3_panel_up(x, y, x+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_HEIGHT-1);
	else
	{
		vga_util.d3_panel_down(x, y, x+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_HEIGHT-1);
		x++;
		y++;
	}

	font_san.center_put(x, y, x+COUNT_BUTTON_WIDTH-1 , y+COUNT_BUTTON_HEIGHT-1, m.format(queuedCount));
}
//----------- End of function FirmHarbor::disp_queue_button -----------//
*/


//-------- Begin of static function i_disp_queue_button --------//
//
static void i_disp_queue_button(ButtonCustom *button, int repaintBody)
{
	FirmHarbor *harbor= (FirmHarbor *)button->custom_para.ptr;

	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	if( !button->pushed_flag )
	{
		if( repaintBody )
		{
			vga_util.blt_buf(x1, y1, x2, y2, 0);
			vga_util.d3_panel2_up( x1, y1, x2, y2, 1, 1);
		}
		x2--;
		y2--;
	}
	else
	{
		if( repaintBody )
		{
			vga_util.blt_buf(x1, y1, x2, y2, 0);
			vga_util.d3_panel2_down( x1, y1, x2, y2, 1, 1);
		}
		x1++;
		y1++;
	}

	//----- count the no. of units queued for this weapon ------//

	short unitId = button->custom_para.value;
	int queuedCount=0;
	for( int i=0 ; i<harbor->build_queue_count ; i++ )
	{
		if( harbor->build_queue_array[i] == unitId )
			queuedCount++;
	}
	if( harbor->build_unit_id == unitId)
		queuedCount++;

	font_mid.center_put( x1+3, y1+3, x2-3, y2-3, m.format(queuedCount), 1);
}
//--------- End of static function i_disp_queue_button ---------//
// ####### end Gilbert 20/9 #######//


//--------- Begin of function FirmHarbor::detect_build_menu ---------//
//
void FirmHarbor::detect_build_menu()
{
	int 	 	 unitId, x=INFO_X1+2, y=INFO_Y1, rc, quitFlag;
	UnitInfo* unitInfo;

	for(int b=0; b<added_count; ++b)
	{
		// ###### begin Gilbert 20/9 #########//
		unitId = button_ship[b].custom_para.value;
		// ###### end Gilbert 20/9 #########//
		unitInfo = unit_res[unitId];

		//------ detect pressing on the small queue count button -------//

		// ####### begin Gilbert 20/9 ########//
		rc = 0;
		if( (rc = button_queue_ship[b].detect(0,0,2)) != 0 )		// both button
		{
			quitFlag = 0;		// don't quit the menu right after pressing the button
		}

		//------ detect pressing on the big button -------//
		else if( (rc = button_ship[b].detect(0,0,2)) != 0 )
		{
			quitFlag = 1;		// quit the menu right after pressing the button
		}
		// ####### end Gilbert 20/9 ########//

		//------- process the action --------//

		if( rc > 0 )
		{
			if( rc==1 )		// left button
			{
#ifdef USE_DPLAY
				if( remote.is_enable() )
				{
					// packet structure : <firm recno> <unit Id>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_HARBOR_BUILD_SHIP, 2*sizeof(short) );
					shortPtr[0] = firm_recno;
					shortPtr[1] = unitId;
				}
				else
#endif
					add_queue(unitId);
				// ##### begin Gilbert 25/9 ######//
				se_ctrl.immediate_sound("TURN_ON");
				// ##### end Gilbert 25/9 ######//
			}
			else 				// right button - remove queue
			{
#ifdef USE_DPLAY
				if( remote.is_enable() )
				{
					// packet structure : <firm recno> <unit Id>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_HARBOR_BUILD_SHIP, 2*sizeof(short) );
					shortPtr[0] = firm_recno;
					shortPtr[1] = -unitId;
				}
				else
#endif
					remove_queue(unitId);

				// ##### begin Gilbert 25/9 ######//
				se_ctrl.immediate_sound("TURN_OFF");
				// ##### end Gilbert 25/9 ######//
			}

			if( quitFlag )
				info.disp();		// info.disp() will call put_info() which will switch mode back to the main menu mode
			else
			// ######## begin Gilbert 20/9 ########//
				// disp_queue_button(y+COUNT_BUTTON_OFFSET_Y, unitId, 1);
				info.update();
			// ######## end Gilbert 20/9 ########//

			return;
		}

		y += BUILD_BUTTON_HEIGHT;
	}

	//------ detect the cancel button --------//

	if( button_cancel.detect() || mouse.any_click(1) )		// press the cancel button or right click
	{
		harbor_menu_mode = HARBOR_MENU_MAIN;
		info.disp();
		// ###### begin Gilbert 26/9 #######//
		se_ctrl.immediate_sound("TURN_OFF");
		// ###### end Gilbert 26/9 #######//
	}
}
//----------- End of function FirmHarbor::detect_build_menu -----------//


//--------- Begin of function FirmHarbor::build_ship ---------//
//

void FirmHarbor::build_ship(int unitId, char)
{
	if(ship_count>=MAX_SHIP_IN_HARBOR)
		return;

	Nation* nationPtr = nation_array[nation_recno];

	if( nationPtr->cash < unit_res[unitId]->build_cost )
		return;

	nationPtr->add_expense( EXPENSE_SHIP, unit_res[unitId]->build_cost);

	build_unit_id  = unitId;
	start_build_frame_no = sys.frame_count;
}

//----------- End of function FirmHarbor::build_ship -----------//


//--------- Begin of function FirmHarbor::sail_ship ---------//
//
void FirmHarbor::sail_ship(int unitRecno, char remoteAction)
{
#ifdef USE_DPLAY
	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <firm recno> <browseRecno>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_HARBOR_SAIL_SHIP, 2*sizeof(short) );
		shortPtr[0] = firm_recno;
		shortPtr[1] = unitRecno;
		return;
	}
#endif

	//----- get the browse recno of the ship in the harbor's ship_recno_array[] ----//

	int browseRecno=0;

	for( int i=0 ; i<ship_count ; i++ )
	{
		if( ship_recno_array[i] == unitRecno )
		{
			browseRecno = i+1;
			break;
		}
	}

	err_when( !browseRecno );
	// ###### begin Gilbert 10/11 ########//
	if( !browseRecno )
		return;
	// ###### end Gilbert 10/11 ########//

	//------------------------------------------------------------//

	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->unit_mode != UNIT_MODE_IN_HARBOR || unitPtr->unit_mode_para != firm_recno );

	SpriteInfo*	spriteInfo = unitPtr->sprite_info;
	int 			xLoc=loc_x1; // xLoc & yLoc are used for returning results
	int 			yLoc=loc_y1;

	if(!world.locate_space(xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height, UNIT_SEA, sea_region_id))
		return;

	unitPtr->init_sprite(xLoc, yLoc);

	del_hosted_ship(ship_recno_array[browseRecno-1]);

	//-------- selected the ship --------//

	if( firm_array.selected_recno == firm_recno &&
		 nation_recno == nation_array.player_recno )
	{
		power.reset_selection();
		unitPtr->selected_flag = 1;
		unit_array.selected_recno = unitPtr->sprite_recno;
		unit_array.selected_count = 1;

		info.disp();
	}
}
//----------- End of function FirmHarbor::sail_ship -----------//


//--------- Begin of function FirmHarbor::disp_build_info ---------//
//
void FirmHarbor::disp_build_info(int refreshFlag)
{
	if( !build_unit_id || nation_recno!=nation_array.player_recno )
		return;

	int dispY1 = INFO_Y1+26;
	int x=MSG_X1+3, y=MSG_Y1+3;

	if( refreshFlag == INFO_REPAINT )
	{
		vga_util.d3_panel_up( MSG_X1, MSG_Y1, MSG_X2, MSG_Y2 );

		vga_util.d3_panel_down(x, y, x+UNIT_SMALL_ICON_WIDTH+3, y+UNIT_SMALL_ICON_HEIGHT+3 );
		vga_front.put_bitmap(x+2, y+2, unit_res[build_unit_id]->get_small_icon_ptr(RANK_SOLDIER) );
	}

	vga_front.indicator( 0, x+UNIT_SMALL_ICON_WIDTH+6, y,
		(float)(sys.frame_count-start_build_frame_no),
		(float)unit_res[build_unit_id]->build_days * FRAMES_PER_DAY, VGA_GRAY );

	button_cancel_build.paint(MSG_X2-27, MSG_Y1+2, "V_X-U", "V_X-D");
	button_cancel_build.set_help_code( "CANCELSHP" );
}
//----------- End of function FirmHarbor::disp_build_info -----------//


//--------- Begin of function FirmHarbor::process_build ---------//
//
void FirmHarbor::process_build()
{
	int totalBuildDays = unit_res[build_unit_id]->build_days;

	err_when( !build_unit_id );

	if( (int)(sys.frame_count-start_build_frame_no) / FRAMES_PER_DAY >= totalBuildDays )
	{
		int unitRecno = unit_array.add_unit( build_unit_id, nation_recno );

		add_hosted_ship(unitRecno);

		if( own_firm() )
			se_res.far_sound(center_x, center_y, 1, 'F', firm_id, "FINS", 'S', unit_res[build_unit_id]->sprite_id);

		build_unit_id = 0;

		// ##### begin Gilbert 20/9 ########//
		if( firm_array.selected_recno == firm_recno )
		{
			disable_refresh = 1;
			info.disp();
			disable_refresh = 0;
		}
		// ##### end Gilbert 20/9 ########//

		//-*********** simulate ship movement ************-//
		//sail_ship(ship_recno_array[0], 0);
		//-*********** simulate ship movement ************-//
	}
}
//----------- End of function FirmHarbor::process_build -----------//


//--------- Begin of function FirmHarbor::cancel_build_unit ---------//
void FirmHarbor::cancel_build_unit()
{
	build_unit_id = 0;

	if( firm_array.selected_recno == firm_recno )
	{
		disable_refresh = 1;
		info.disp();
		disable_refresh = 0;
	}
}
//----------- End of function FirmHarbor::cancel_build_unit -----------//


//--------- Begin of function FirmHarbor::add_hosted_ship ---------//
//
void FirmHarbor::add_hosted_ship(int shipRecno)
{
	err_when( ship_count == MAX_SHIP_IN_HARBOR );

	ship_recno_array[ship_count++] = shipRecno;

	//---- set the unit_mode of the ship ----//

	err_when( firm_id != FIRM_HARBOR );

	unit_array[shipRecno]->set_mode(UNIT_MODE_IN_HARBOR, firm_recno);

	//---------------------------------------//

	if( firm_recno == firm_array.selected_recno )
		put_info(INFO_UPDATE);
}
//----------- End of function FirmHarbor::add_hosted_ship -----------//


//--------- Begin of function FirmHarbor::del_hosted_ship ---------//
//
void FirmHarbor::del_hosted_ship(int delUnitRecno)
{
	//---- reset the unit_mode of the ship ----//

	unit_array[delUnitRecno]->set_mode(0);

	//-----------------------------------------//

	int i;
	for( i=0 ; i<ship_count ; i++ )
	{
		if( ship_recno_array[i] == delUnitRecno )
		{
			err_when( ship_count > MAX_SHIP_IN_HARBOR );

			m.del_array_rec( ship_recno_array, ship_count, sizeof(ship_recno_array[0]), i+1 );
			break;
		}
	}

	err_when( i==ship_count );

	ship_count--;

	if( firm_recno == firm_array.selected_recno )
		put_info(INFO_UPDATE);
}
//----------- End of function FirmHarbor::del_hosted_ship -----------//


//--------- Begin of function FirmHarbor::update_linked_firm_info ---------//
// Note: when ai is finished, ai_link_checked may be used as am indicator
//			for link_checked.  Performance should be improved in this way.
//
void FirmHarbor::update_linked_firm_info()
{
	if(link_checked)
		return; // no need to check again
	
	linked_mine_num = linked_factory_num = linked_market_num = 0;
	memset(linked_mine_array, 0, sizeof(short)*MAX_LINKED_FIRM_FIRM);
	memset(linked_factory_array, 0, sizeof(short)*MAX_LINKED_FIRM_FIRM);
	memset(linked_market_array, 0, sizeof(short)*MAX_LINKED_FIRM_FIRM);

	Firm *firmPtr;
	Nation *nationPtr;
	for(int i=linked_firm_count-1; i>=0; i--)
	{
		if(linked_firm_enable_array[i]!=LINK_EE)
			continue;

		firmPtr = firm_array[linked_firm_array[i]];
		if(!nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty)
			continue;

		switch(firmPtr->firm_id)
		{
			case FIRM_MINE:
					nationPtr = nation_array[firmPtr->nation_recno];
					//if(nationPtr->get_relation_status(nation_recno)<=NATION_TENSE)
					//	continue;

					linked_mine_array[linked_mine_num] = firmPtr->firm_recno;
					linked_mine_num++;
					break;

			case FIRM_FACTORY:
					nationPtr = nation_array[firmPtr->nation_recno];
					//if(nationPtr->get_relation_status(nation_recno)<=NATION_TENSE)
					//	continue;

					linked_factory_array[linked_factory_num] = firmPtr->firm_recno;
					linked_factory_num++;
					break;

			case FIRM_MARKET:
					nationPtr = nation_array[firmPtr->nation_recno];
					//if(nationPtr->get_relation_status(nation_recno)<=NATION_TENSE)
					//	continue;

					linked_market_array[linked_market_num] = firmPtr->firm_recno;
					linked_market_num++;
					break;
		}
	}
}
//----------- End of function FirmHarbor::update_linked_firm_info -----------//


//--------- Begin of function FirmHarbor::process_queue ---------//
void FirmHarbor::process_queue()
{
	if(build_queue_count>0)
	{
		build_ship(build_queue_array[0], COMMAND_AUTO);

		// remove the queue no matter build_ship success or not

		m.del_array_rec( build_queue_array, build_queue_count, sizeof(build_queue_array[0]), 1 );
		build_queue_count--;

		if( firm_array.selected_recno == firm_recno )
		{
			disable_refresh = 1;
			info.disp();
			disable_refresh = 0;
		}
	}
}
//----------- End of function FirmHarbor::process_queue -----------//


//--------- Begin of function FirmHarbor::add_queue ---------//
void FirmHarbor::add_queue(int unitId)
{
	if( build_queue_count+(build_unit_id>0)==MAX_BUILD_SHIP_QUEUE )
		return;

	build_queue_array[build_queue_count++] = unitId;
}
//----------- End of function FirmHarbor::add_queue -----------//


//--------- Begin of function FirmHarbor::remove_queue ---------//
void FirmHarbor::remove_queue(int unitId)
{
	for( int i=build_queue_count-1 ; i>=0 ; i-- )
	{
		if( build_queue_array[i] == unitId )
		{
			err_when( build_queue_count > MAX_BUILD_SHIP_QUEUE );

			m.del_array_rec( build_queue_array, build_queue_count, sizeof(build_queue_array[0]), i+1 );

			build_queue_count--;
			return;
		}
	}

	if(build_unit_id==unitId)
		cancel_build_unit();
}
//----------- End of function FirmHarbor::remove_queue -----------//


//-------- Begin of static function put_ship_rec --------//
//
static void put_ship_rec(int recNo, int x, int y, int refreshFlag)
{
	UnitMarine* unitPtr  = (UnitMarine*) unit_array[ firm_harbor_ptr->ship_recno_array[recNo-1] ];
	UnitInfo*   unitInfo = unit_res[unitPtr->unit_id];

	//-------- display unit icon -------//

	vga_front.d3_panel_down(x+1, y+1, x+UNIT_SMALL_ICON_WIDTH+4, y+UNIT_SMALL_ICON_HEIGHT+4, 2, 0 );
	// ###### begin Gilbert 17/10 #######//
	vga_front.put_bitmap(x+3, y+3, unit_res[unitPtr->unit_id]->get_small_icon_ptr(unitPtr->rank_id) );
	// ###### end Gilbert 17/10 #######//

	y+=6;

	//---------- display unit name ----------//

	font_san.put( x+32 , y, unitInfo->name, 1, x+119 );		// 1-use short words

	//------- display unit hit points -------//

	String str;

	str  = (int) unitPtr->hit_points;
	str += "/";
	str += unitPtr->max_hit_points;

	font_san.put( x+125, y, str, 1, x+browse_ship.rec_width-3 );
}
//----------- End of static function put_ship_rec -----------//

