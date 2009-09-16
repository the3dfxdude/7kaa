//Filename    : OF_WAR.CPP
//Description : Firm War Factory

#include <OINFO.h>
#include <OVGA.h>
#include <ODATE.h>
#include <OSTR.h>
#include <OCONFIG.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OBUTT3D.h>
#include <OPOWER.h>
#include <OUNIT.h>
#include <OGAME.h>
#include <OSYS.h>
#include <ONATION.h>
#include <OF_WAR.h>
#include <OREMOTE.h>
#include <OIMGRES.h>
#include <OSE.h>
#include <OSERES.h>
#include <OBUTTCUS.h>


//------------- Define constant ------------//

#ifdef AMPLUS
	#define BUILD_BUTTON_WIDTH 	201
	#define BUILD_BUTTON_HEIGHT 	46
	#define COUNT_BUTTON_OFFSET_X	156
	#define COUNT_BUTTON_OFFSET_Y 7
	#define COUNT_BUTTON_WIDTH		32
	#define COUNT_BUTTON_HEIGHT	30
	#define BUILD_UNIT_ICON_OFFSET_X 6
	#define BUILD_UNIT_ICON_OFFSET_Y 4
	#define BUILD_UNIT_NAME_OFFSET_X 60
	#define BUILD_UNIT_NAME_OFFSET_Y 12
#else
	#define BUILD_BUTTON_WIDTH 	201
	#define BUILD_BUTTON_HEIGHT 	56
	#define COUNT_BUTTON_OFFSET_X	156
	#define COUNT_BUTTON_OFFSET_Y 12
	#define COUNT_BUTTON_WIDTH		32
	#define COUNT_BUTTON_HEIGHT	32
	#define BUILD_UNIT_ICON_OFFSET_X 6
	#define BUILD_UNIT_ICON_OFFSET_Y 8
	#define BUILD_UNIT_NAME_OFFSET_X 60
	#define BUILD_UNIT_NAME_OFFSET_Y 18
#endif

//---------- Define constant ------------//

enum { WAR_MENU_MAIN,
		 WAR_MENU_BUILD,
	  };

//--------- define static variables ----------//

static Button3D button_select_build;
static Button3D	button_cancel_build;
static char     war_menu_mode;
static char     disable_refresh=0;
static short	 added_count;
// ###### begin Gilbert 10/9 #######//
// static short	 button_unit_id[MAX_WEAPON_TYPE];
static ButtonCustom button_weapon[MAX_WEAPON_TYPE];
static ButtonCustom button_queue_weapon[MAX_WEAPON_TYPE];
// ###### end Gilbert 10/9 #######//
static ButtonCustom button_cancel;


// --------- declare static function ----------//

static void i_disp_build_button(ButtonCustom *button, int);
static void i_disp_queue_button(ButtonCustom *button, int);

//--------- Begin of function FirmWar::FirmWar ---------//
//
FirmWar::FirmWar()
{
	firm_skill_id = SKILL_MFT;
	build_unit_id = 0;
	build_queue_count = 0;
}
//----------- End of function FirmWar::FirmWar -----------//


//--------- Begin of function FirmWar::~FirmWar ---------//
//
FirmWar::~FirmWar()
{
}
//----------- End of function FirmWar::~FirmWar -----------//


//--------- Begin of function FirmWar::put_info ---------//
//
void FirmWar::put_info(int refreshFlag)
{
	if( refreshFlag==INFO_REPAINT && !disable_refresh )
		war_menu_mode = WAR_MENU_MAIN;

	switch( war_menu_mode )
	{
		case WAR_MENU_MAIN:
			disp_main_menu(refreshFlag);
			break;

		case WAR_MENU_BUILD:
			disp_build_menu(refreshFlag);
			break;
	}
}
//----------- End of function FirmWar::put_info -----------//


//--------- Begin of function FirmWar::detect_info ---------//
//
void FirmWar::detect_info()
{
	switch( war_menu_mode )
	{
		case WAR_MENU_MAIN:
			detect_main_menu();
			break;

		case WAR_MENU_BUILD:
			detect_build_menu();
			break;
	}
}
//----------- End of function FirmWar::detect_info -----------//


//--------- Begin of function FirmWar::disp_main_menu ---------//
//
void FirmWar::disp_main_menu(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	disp_war_info(INFO_Y1+54, refreshFlag);

	disp_worker_list(INFO_Y1+107, refreshFlag);

	disp_worker_info(INFO_Y1+171, refreshFlag);

	if( own_firm() )
	{
		if( refreshFlag==INFO_REPAINT )
			button_select_build.paint( INFO_X1, INFO_Y1+235, 'A', "MAKEWEAP" );
	}

	disp_spy_button( INFO_X1+BUTTON_ACTION_WIDTH, INFO_Y1+235, refreshFlag );
}
//----------- End of function FirmWar::disp_main_menu -----------//


//--------- Begin of function FirmWar::detect_main_menu ---------//
//
void FirmWar::detect_main_menu()
{
	//-------- detect basic info -----------//

	if( detect_basic_info() )
		return;

	//---------- detect cancel build ------------//
	if(build_unit_id)
	{
		if(button_cancel_build.detect())
		{
			if( !remote.is_enable() )
				cancel_build_unit();
			else
			{
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_WAR_SKIP_WEAPON, sizeof(short) );
				shortPtr[0] = firm_recno;
			}
		}
	}

	//----------- detect worker -----------//

	if( detect_worker_list() )
	{
		disp_war_info(INFO_Y1+54, INFO_UPDATE);
		disp_worker_info(INFO_Y1+171, INFO_UPDATE);
	}

	//-------- detect spy button ----------//

	detect_spy_button();

	if( !own_firm() )
		return;

	//------ detect the select research button -------//

	if( button_select_build.detect() )
	{
		war_menu_mode = WAR_MENU_BUILD;
		disable_refresh = 1;    // static var for disp_info() only
		info.disp();
		disable_refresh = 0;
	}
}
//----------- End of function FirmWar::detect_main_menu -----------//


//--------- Begin of function FirmWar::disp_build_menu ---------//
//
void FirmWar::disp_build_menu(int refreshFlag)
{
	// ###### begin Gilbert 10/9 ########//
	if( refreshFlag == INFO_UPDATE )
	{
		for( int b=0; b<added_count; ++b )
		{
			button_weapon[b].paint(-1, 0);
			// button_queue_weapon[b] is called by button_weapon[b].paint();
		}
	}
	else if( refreshFlag == INFO_REPAINT )
	{
		added_count=0;
		int unitId, x=INFO_X1+2, y=INFO_Y1;
		UnitInfo* unitInfo;

		for( unitId=1; unitId<=MAX_UNIT_TYPE ; unitId++ )
		{
			unitInfo = unit_res[unitId];

			if( unitInfo->unit_class == UNIT_CLASS_WEAPON &&
				 unitInfo->get_nation_tech_level(nation_recno) > 0 )
			{
				// disp_build_button( y, unitId, 1);
				button_queue_weapon[added_count].create(x+COUNT_BUTTON_OFFSET_X, y+COUNT_BUTTON_OFFSET_Y,
					x+COUNT_BUTTON_OFFSET_X+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_OFFSET_Y+COUNT_BUTTON_HEIGHT-1,
					i_disp_queue_button, ButtonCustomPara(this, unitId) );
				button_weapon[added_count].paint(x, y, x+BUILD_BUTTON_WIDTH-1, y+BUILD_BUTTON_HEIGHT-1,
					i_disp_build_button, ButtonCustomPara(&button_queue_weapon[added_count], unitId) );

				err_when(added_count >= MAX_UNIT_TYPE);
				//button_unit_id[added_count++] = unitId;
				added_count++;
				y += BUILD_BUTTON_HEIGHT;
			}
		}

		button_cancel.paint(x, y, x+BUILD_BUTTON_WIDTH-1, y+BUILD_BUTTON_HEIGHT*3/4,
		ButtonCustom::disp_text_button_func, ButtonCustomPara("Done",0) );
	}
	// ###### end Gilbert 10/9 ########//
}
//----------- End of function FirmWar::disp_build_menu -----------//


//--------- Begin of function FirmWar::detect_build_menu ---------//
//
void FirmWar::detect_build_menu()
{

	int 	 	 unitId, x=INFO_X1+2, y=INFO_Y1, rc, quitFlag;
	UnitInfo* unitInfo;

	for( int b = 0; b < added_count; ++b)
	{
		// ##### begin Gilbert 10/9 ######//
		unitId = button_weapon[b].custom_para.value;
		// ##### end Gilbert 10/9 ######//
		unitInfo = unit_res[unitId];

		//------ detect pressing on the small queue count button -------//

		// ######## begin Gilbert 10/9 #########//
		rc = 0;

		if( (rc = button_queue_weapon[b].detect(0,0,2)) != 0 )		// both button
		{
			quitFlag = 0;		// don't quit the menu right after pressing the button
		}

		//------ detect pressing on the big button -------//

		else if( (rc = button_weapon[b].detect(0,0,2)) != 0 )
		{
			quitFlag = 1;		// quit the menu right after pressing the button
		}
		// ######## end Gilbert 10/9 #########//

		//------- process the action --------//

		if( rc > 0 )
		{
			if( rc==1 )		// left button
			{
				if( remote.is_enable() )
				{
					// packet structure : <firm recno> <unit Id>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_WAR_BUILD_WEAPON, 2*sizeof(short) );
					shortPtr[0] = firm_recno;
					shortPtr[1] = unitId;
				}
				else
					add_queue(unitId);
				// ##### begin Gilbert 25/9 ######//
				se_ctrl.immediate_sound("TURN_ON");
				// ##### end Gilbert 25/9 ######//
			}
			else 				// right button - remove queue
			{
				if( remote.is_enable() )
				{
					// packet structure : <firm recno> <unit Id>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_WAR_CANCEL_WEAPON, 2*sizeof(short) );
					shortPtr[0] = firm_recno;
					shortPtr[1] = unitId;
				}
				else
					remove_queue(unitId);
				// ##### begin Gilbert 25/9 ######//
				se_ctrl.immediate_sound("TURN_OFF");
				// ##### end Gilbert 25/9 ######//
			}

			if( quitFlag )
				info.disp();		// info.disp() will call put_info() which will switch mode back to the main menu mode
			// ###### begin Gilbert 10/9 ########//
			else
				//disp_queue_button(y+COUNT_BUTTON_OFFSET_Y, unitId, 1);
				info.update();
			// ###### end Gilbert 10/9 ########//

			return;
		}

		y += BUILD_BUTTON_HEIGHT;
	}
	//------ detect the cancel button --------//

	if( button_cancel.detect() )
	{
		// ##### begin Gilbert 25/9 ######//
		se_ctrl.immediate_sound("TURN_OFF");
		// ##### end Gilbert 25/9 ######//
		war_menu_mode = WAR_MENU_MAIN;
		info.disp();
	}
}
//----------- End of function FirmWar::detect_build_menu -----------//


// ####### begin Gilbert 10/9 ########//
/*
//-------- Begin of function FirmWar::disp_build_button --------//
//
void FirmWar::disp_build_button(int y, int unitId, int buttonUp)
{
	int x;
	int y0 = y;
	if( buttonUp )
	{
		vga.d3_panel_up( INFO_X1, y, INFO_X2-1, y+BUILD_BUTTON_HEIGHT-2 );
		x = INFO_X1;
	}
	else
	{
		vga.d3_panel_down( INFO_X1, y, INFO_X2-1, y+BUILD_BUTTON_HEIGHT-2 );
		x = INFO_X1+1;
		y++;
	}

	// display unit large icon
	UnitInfo* unitInfo = unit_res[unitId];
	vga_front.put_bitmap(x+6, y+8, unitInfo->get_large_icon_ptr(0));

	//-------- display unit name --------//

	String str;
	str = unitInfo->name;

	if( unitInfo->unit_class == UNIT_CLASS_WEAPON )		// add version no.
	{
		int techLevel = unitInfo->get_nation_tech_level(nation_recno);

		if( techLevel > 1 )
		{
			str += " ";
			str += m.roman_number(techLevel);
		}
	}
	
	font_bible.put( x+60, y+18, str );

	disp_queue_button(y0+COUNT_BUTTON_OFFSET_Y, unitId, 1);
}
//--------- End of function FirmWar::disp_build_button ---------//
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
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_up( x1, y1, x2, y2, 1 );
		}
		x2--;
		y2--;
	}
	else
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_down( x1, y1, x2, y2, 1 );
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
		vga_front.put_bitmap(x1+BUILD_UNIT_ICON_OFFSET_X, y1+BUILD_UNIT_ICON_OFFSET_Y, 
			unitInfo->get_large_icon_ptr(0));

		err_when( button->custom_para.value != queueButton->custom_para.value);

		//-------- display unit name --------//

		String str;
		str = unitInfo->name;

		if( unitInfo->unit_class == UNIT_CLASS_WEAPON )		// add version no.
		{
			FirmWar *warFactory = (FirmWar *)queueButton->custom_para.ptr;
			int techLevel = unitInfo->get_nation_tech_level(warFactory->nation_recno);

			if( techLevel > 1 )
			{
				str += " ";
				str += m.roman_number(techLevel);
			}
		}
		
		font_bible.put( x1+BUILD_UNIT_NAME_OFFSET_X, y1+BUILD_UNIT_NAME_OFFSET_Y, str );
	}

	// display small button
	queueButton->paint(-1, repaintBody);
}
//--------- End of static function i_disp_build_button ---------//



/*
//-------- Begin of function FirmWar::disp_queue_button --------//
//
void FirmWar::disp_queue_button(int y, int unitId, int buttonUp)
{
	//----- count the no. of units queued for this weapon ------//

	int x=INFO_X1+2+COUNT_BUTTON_OFFSET_X;
	int queuedCount=0;

	for( int i=0 ; i<build_queue_count ; i++ )
	{
		if( build_queue_array[i] == unitId )
			queuedCount++;
	}

	if(build_unit_id==unitId)
		queuedCount++;

	if( buttonUp )
	{
		vga.d3_panel_up( x, y, x+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_HEIGHT-1);
	}
	else
	{
		vga.d3_panel_down( x, y, x+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_HEIGHT-1);
		x++;
		y++;
	}

	font_san.center_put( x, y, x+COUNT_BUTTON_WIDTH-1 , y+COUNT_BUTTON_HEIGHT-1, m.format(queuedCount) );
}
//--------- End of function FirmWar::disp_queue_button ---------//
*/

//-------- Begin of static function i_disp_queue_button --------//
//
static void i_disp_queue_button(ButtonCustom *button, int repaintBody)
{
	FirmWar *warFactory = (FirmWar *)button->custom_para.ptr;

	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	if( !button->pushed_flag )
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_up( x1, y1, x2, y2, 1, 1);
		}
		x2--;
		y2--;
	}
	else
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_down( x1, y1, x2, y2, 1, 1);
		}
		x1++;
		y1++;
	}

	//----- count the no. of units queued for this weapon ------//

	short unitId = button->custom_para.value;
	int queuedCount=0;
	for( int i=0 ; i<warFactory->build_queue_count ; i++ )
	{
		if( warFactory->build_queue_array[i] == unitId )
			queuedCount++;
	}
	if( warFactory->build_unit_id == unitId)
		queuedCount++;

	font_mid.center_put( x1+3, y1+3, x2-3, y2-3, m.format(queuedCount), 1);
}
//--------- End of static function i_disp_queue_button ---------//

// ####### end Gilbert 10/9 ########//


//--------- Begin of function FirmWar::add_queue ---------//
//
void FirmWar::add_queue(int unitId)
{
	if( build_queue_count+(build_unit_id>0)==MAX_BUILD_QUEUE )
		return;

	build_queue_array[build_queue_count++] = unitId;
}
//----------- End of function FirmWar::add_queue -----------//


//--------- Begin of function FirmWar::remove_queue ---------//
//
void FirmWar::remove_queue(int unitId)
{
	for( int i=build_queue_count-1 ; i>=0 ; i-- )
	{
		if( build_queue_array[i] == unitId )
		{
			err_when( build_queue_count > MAX_BUILD_QUEUE );

			m.del_array_rec( build_queue_array, build_queue_count, sizeof(build_queue_array[0]), i+1 );

			build_queue_count--;
			return;
		}
	}

	if(build_unit_id==unitId)
		cancel_build_unit();
}
//----------- End of function FirmWar::remove_queue -----------//


//--------- Begin of function FirmWar::disp_war_info ---------//
//
void FirmWar::disp_war_info(int dispY1, int refreshFlag)
{
	static short lastUnitId=0;

	if( refreshFlag==INFO_UPDATE && lastUnitId != build_unit_id )
	{
		lastUnitId = build_unit_id;
		info.disp();
	}

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+50 );

	if( !build_unit_id )
		return;

	int x=INFO_X1+4, y=dispY1+4;

	//-------- display the icon of the researching item ---------//

	UnitInfo* unitInfo = unit_res[build_unit_id];

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_down( x, y, x+UNIT_LARGE_ICON_WIDTH+3, y+UNIT_LARGE_ICON_HEIGHT+3, 2 );
		vga_front.put_bitmap( x+2, y+2, unitInfo->get_large_icon_ptr(0) );

		//----------- display text ------------//

		x += UNIT_LARGE_ICON_WIDTH+10;

		String str = unitInfo->name;

		if( unitInfo->unit_class == UNIT_CLASS_WEAPON )		// add version no.
		{
			int techLevel = unitInfo->get_nation_tech_level(nation_recno);

			if( techLevel > 1 )
			{
				str += " ";
				str += m.roman_number(techLevel);
			}
		}

		font_san.put( x, y+4, str );
	}
	else
	{
		x += UNIT_LARGE_ICON_WIDTH+10;
	}

	//-- unitInfo->build_days is the no. of days take to build the firm when productivity is 100, the actually no. of days will be longer if productivity is less than 100 --//

	float buildProgressDays = build_progress_days
									  + (float) (sys.frame_count-last_process_build_frame_no) / FRAMES_PER_DAY
									  * (float) (worker_count*6+productivity/2) / 100;

	vga_front.indicator( 0, x-2, y+21, buildProgressDays, unitInfo->build_days, VGA_GRAY );

	button_cancel_build.paint(MSG_X2-27, dispY1, "V_X-U", "V_X-D");
	button_cancel_build.set_help_code( "CANCELWP" );
}
//----------- End of function FirmWar::disp_war_info -----------//


//--------- Begin of function FirmWar::next_day ---------//
//
void FirmWar::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//----------- update population -------------//

	recruit_worker();

	//-------- train up the skill ------------//

	update_worker();

	//--------- calculate productivity ----------//

	calc_productivity();

	//--------- process building weapon -------//

	if( build_unit_id )
		process_build();
	else
		process_queue();
}
//----------- End of function FirmWar::next_day -----------//


//--------- Begin of function FirmWar::process_queue ---------//
//
void FirmWar::process_queue()
{
	if( build_queue_count==0 )
		return;

	//--- first check if the nation has enough money to build the weapon ---//

	Nation* nationPtr = nation_array[nation_recno];
	build_unit_id = build_queue_array[0];

	if( nationPtr->cash < unit_res[build_unit_id]->build_cost )
	{
		build_unit_id = 0;
		return;
	}

	nationPtr->add_expense( EXPENSE_WEAPON, unit_res[build_unit_id]->build_cost, 1);

	err_when( build_queue_count > MAX_BUILD_QUEUE );

	m.del_array_rec( build_queue_array, build_queue_count, sizeof(build_queue_array[0]), 1 );

	build_queue_count--;

	//------- set building parameters -------//

	last_process_build_frame_no = sys.frame_count;
	build_progress_days = (float) 0;

	if( firm_array.selected_recno == firm_recno )
	{
		disable_refresh = 1;
		info.disp();
		disable_refresh = 0;
	}
}
//----------- End of function FirmWar::process_queue -----------//


//--------- Begin of function FirmWar::process_build ---------//
//
void FirmWar::process_build()
{
	err_when( !build_unit_id );

	UnitInfo* unitInfo = unit_res[build_unit_id];
	int   totalBuildDays = unitInfo->build_days;

	build_progress_days += (float) (worker_count*6+productivity/2) / 100;

	last_process_build_frame_no = sys.frame_count;

	if( config.fast_build && nation_recno==nation_array.player_recno )
		build_progress_days += (float) 2;

	if( build_progress_days > totalBuildDays )
	{
		SpriteInfo*	spriteInfo = sprite_res[ unitInfo->sprite_id ];
		int 			xLoc=loc_x1; // xLoc & yLoc are used for returning results
		int 			yLoc=loc_y1;

		if( !world.locate_space(xLoc, yLoc, loc_x2, loc_y2,
			 spriteInfo->loc_width, spriteInfo->loc_height, unitInfo->mobile_type) )
		{
			build_progress_days = (float)(totalBuildDays + 1);
			return;
		}

		unit_array.add_unit( build_unit_id, nation_recno, 0, 0, xLoc, yLoc );

		if( firm_array.selected_recno == firm_recno )
		{
			disable_refresh = 1;
			info.disp();
			disable_refresh = 0;
		}

		if( own_firm() )
			se_res.far_sound(center_x, center_y, 1, 'F', firm_id, "FINS", 'S', unit_res[build_unit_id]->sprite_id);

		build_unit_id = 0;
	}
}
//----------- End of function FirmWar::process_build -----------//


//--------- Begin of function FirmWar::cancel_build_unit ---------//
void FirmWar::cancel_build_unit()
{
	build_unit_id = 0;

	if( firm_array.selected_recno == firm_recno )
	{
		disable_refresh = 1;
		info.disp();
		disable_refresh = 0;
	}
}
//----------- End of function FirmWar::cancel_build_unitpt -----------//
