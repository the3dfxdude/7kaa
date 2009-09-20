//Filename    : OINFO.CPP
//Description : Info class

#include <time.h>
#include <OVGA.h>
#include <OIMGRES.h>
#include <OSPY.h>
#include <OSTR.h>
#include <OBUTTON.h>
#include <OMOUSE.h>
#include <OFIRM.h>
#include <OSTR.h>
#include <ONEWS.h>
#include <ONATION.h>
#include <OFONT.h>
#include <ODATE.h>
#include <OTOWN.h>
#include <ODATE.h>
#include <OGAME.h>
#include <OGFILE.h>
#include <OPOWER.h>
#include <OSITE.h>
#include <OWALLRES.h>
#include <OSYS.h>
#include <OUNIT.h>
#include <OCOLTBL.h>
#include <OINFO.h>
#include <OOPTMENU.h>

//------------ Define static vars ----------------//

static char* skill_name_array[] =
{
	"Combat Skill",
	"Construction Skill",
	"Production Skill"
};

//------ define static vars --------//

static	char *save_buf_1 = NULL;
static	char *save_buf_1b = NULL;
static	char *save_buf_2 = NULL;
static	char *save_buf_3 = NULL;
static	char *save_buf_4 = NULL;


//-------- Begin of function Info::Info --------//
//
Info::Info() : report_array(sizeof(short), 50),
					report_array2(sizeof(short), 50),
					talk_msg_disp_array(sizeof(TalkMsgDisp), 50)
{
	info_background_bitmap = NULL;
}
//--------- End of function Info::Info ---------//


//-------- Begin of function Info::~Info --------//
//
Info::~Info()
{
	deinit();

	err_when( save_buf_1 );
}
//--------- End of function Info::~Info ---------//


//-------- Begin of function Info::init --------//
//
void Info::init()
{
	deinit();

	game_day   = 1;
	game_month = 6;
	game_year  = 1000;

	game_start_date = date.julian( game_year, game_month, game_day );

	game_date  = game_start_date;

	year_day   = date.day_year( game_year, game_month, game_day );

	year_passed= 0;      // no. of years has been passed since the game begins

	goal_deadline = date.julian( date.year(info.game_start_date)+config.goal_year_limit,
						 date.month(info.game_start_date),
						 date.day(info.game_start_date) );

	goal_difficulty  = 0;
	goal_score_bonus = 0;

	//------ reset report browsers recno -----//

	browse_nation_recno  = 0;
	browse_race_recno    = 0;
	browse_firm_recno    = 0;
	browse_income_recno	= 0;
	browse_expense_recno	= 0;
	browse_troop_recno   = 0;
	browse_unit_recno    = 0;
	browse_tech_recno    = 0;
	browse_god_recno		= 0;
	browse_town_recno    = 0;
	browse_spy_recno     = 0;
	browse_caravan_recno = 0;
	browse_ship_recno	   = 0;
	browse_talk_msg_recno= 0;
	browse_news_recno	   = 0;
	browse_ai_action_recno = 0;
	browse_ai_attack_recno = 0;

	//------ vars of the nation report ------//

	nation_report_mode = NATION_REPORT_INFO;
	player_reply_mode  = 0;
	chat_receiver_type = CHAT_RECEIVER_CURRENT;

	//----------------------------------//

	start_play_time = m.get_time();	// the time player start playing the game
	total_play_time = 0;             // total time the player has played in all saved games

	// these will be updated during loading game and saving game

	err_when( MAX_REMOTE_CHAT_STR < DISP_NEWS_COUNT );			// it must not fewer then the maximum number of news that can be displayed on the screen at a time.
}
//--------- End of function Info::init ---------//


//-------- Begin of function Info::deinit --------//
//
void Info::deinit()
{
	if( info_background_bitmap )
	{
		mem_del( info_background_bitmap );
		info_background_bitmap = NULL;
	}
}
//--------- End of function Info::deinit ---------//


//------- Begin of function Info::init_random_seed ------//
//
// [unsigned] randomSeed - if given, it will be the random seed of the game.
//									random seed. otherwise a random seed will be
//								   picked.
//								   (default:0)
//
void Info::init_random_seed(int randomSeed)
{
	if( randomSeed )
		random_seed = randomSeed;
	else
	{
		randomSeed = time(NULL);
		randomSeed = (int) _rotr( randomSeed, 4 );
		if( randomSeed < 0 )
			randomSeed = ~randomSeed;
		if( randomSeed == 0 )
			randomSeed = 1;
		random_seed = randomSeed;
	}

	m.set_random_seed(random_seed);

	//------ write random seed --------//

	if( sys.testing_session )
	{
		File fileMapSeed;

		fileMapSeed.file_create( "MAP.RS" );

		String str(m.format(random_seed,1));

		fileMapSeed.file_write(str, str.len());
	}
}
//------- End of function Info::init_random_seed ------//


//-------- Begin of function Info::next_day ---------//
//
void Info::next_day()
{
	if( ++game_day > 30 )
		game_day = 30;            // game_day is limited to 1-30 for
										  // calculation of e.g. revenue_30days()
	game_date++;

	week_day=game_date%7;

	//-----------------------------------------//

	if( date.month(game_date) != game_month )
	{
		game_day   = 1;
		game_month = date.month(game_date);

		firm_array.next_month();
		nation_array.next_month();
	}

	if( date.year(game_date) != game_year )
	{
		game_month = 1;
		game_year  = date.year(game_date);
		year_passed++;

		firm_array.next_year();
		nation_array.next_year();
	}

	//-------- set year_day ----------//

	year_day = date.day_year( game_year, game_month, game_day );

	//--- if a spy is viewing secret reports of other nations ---//

	if( viewing_spy_recno )
		process_viewing_spy();

	//-------- deadline approaching message -------//

	if( !game.game_has_ended && config.goal_year_limit_flag )
	{
		int dayLeft = goal_deadline-game_date;
		int yearLeft = dayLeft/365;

		if( dayLeft%365==0 && yearLeft>=1 && yearLeft<=5 )
			news_array.goal_deadline( yearLeft, 0 );

		if( dayLeft==0 )		// deadline arrives, everybody loses the game
			game.game_end(0, 0);
	}
}
//---------- End of function Info::next_day --------//


//-------- Begin of function Info::disp_panel --------//
//
void Info::disp_panel()
{
	image_interface.put_to_buf( &vga_back, "MAINSCR" );

	//------ keep a copy of bitmap of the panel texture -----//

	if( !info_background_bitmap ) 
		info_background_bitmap = mem_add( 4 + (INFO_X2-INFO_X1+1)*(INFO_Y2-INFO_Y1+1) );

	vga_back.read_bitmap( INFO_X1, INFO_Y1, INFO_X2, INFO_Y2, info_background_bitmap );
}
//--------- End of function Info::disp_panel ---------//


//-------- Begin of function Info::disp --------//
//
// Display the side info area.
//
void Info::disp()
{
//	mouse.handle_flicking = 0;		// since it will be called by Firm detect functions directly which may have set mouse.handle_flicking to 1 first, so we need to cancel it here

	if( !power.enable_flag )
		return;

	if( sys.signal_exit_flag )
		return;

	if( option_menu.is_active() )
		return;

	vga_back.put_bitmap( INFO_X1, INFO_Y1, info_background_bitmap );
	vga_front.put_bitmap( INFO_X1, INFO_Y1, info_background_bitmap );

	//------- use front buffer -------//

	int saveUseBackBuf = vga.use_back_buf;

	vga.use_front();

	//------ if units/firm selected, display info --------//

	if( firm_array.selected_recno )
	{
		firm_array[firm_array.selected_recno]->disp_info_both(INFO_REPAINT);
	}
	else if( town_array.selected_recno )
	{
		town_array[town_array.selected_recno]->disp_info(INFO_REPAINT);
	}
	else if( site_array.selected_recno )
	{
		site_array[site_array.selected_recno]->disp_info(INFO_REPAINT);
	}
	else if( unit_array.selected_recno )
	{
		unit_array[unit_array.selected_recno]->disp_info(INFO_REPAINT);
	}
	else if( wall_res.selected_x_loc >= 0 )
	{
		wall_res.disp_info(INFO_REPAINT);
	}

	//----- restore use back buffer if it was ----//

	if( saveUseBackBuf )
		vga.use_back();
}
//-------- End of function Info::disp --------//


//-------- Begin of function Info::update --------//

void Info::update()
{
	if( !power.enable_flag )
		return;

	if( option_menu.is_active() )
		return;

	//-------------------------------------------//

	disp_heading();

	//------- use front buffer -------//

	int saveUseBackBuf = vga.use_back_buf;

	vga.use_front();

	//-------------------------------------------//

	if( firm_array.selected_recno )
	{
		firm_array[firm_array.selected_recno]->disp_info_both(INFO_UPDATE);
	}
	else if( town_array.selected_recno )
	{
		town_array[town_array.selected_recno]->disp_info(INFO_UPDATE);
	}
	else if( site_array.selected_recno )
	{
		site_array[site_array.selected_recno]->disp_info(INFO_UPDATE);
	}
	else if( unit_array.selected_recno )
	{
		unit_array[unit_array.selected_recno]->disp_info(INFO_UPDATE);
	}
	else if( wall_res.selected_x_loc >= 0 )
	{
		wall_res.disp_info(INFO_UPDATE);
	}

	//----- restore use back buffer if it was ----//

	if( saveUseBackBuf )
		vga.use_back();
}
//-------- End of function Info::update --------//


//-------- Begin of function Info::disp_heading --------//

void Info::disp_heading()
{
	//---- display info on the top menu area ----//

	int x=TOP_MENU_X2-250;

	//---------- display date -----------//

   font_mid.use_max_height();
	font_mid.disp( 460, 10, date.date_str(game_date,1), 575);
	font_mid.use_std_height();

	if( !nation_array.player_recno )		// the player has lost the game
	{
		font_mid.disp( 307, 10, "", 445);		// clear the display 
		font_mid.disp( 305, 30, "", 445);
		// ##### begin Gilbert 4/11 #######//
		image_icon.put_front(447,26, "REPU_DW" );
		// ##### end Gilbert 4/11 #######//
		font_mid.disp( 476, 30, "", 575);
		return;
	}

	String  str;
	Nation* nationPtr = ~nation_array;

	//------- display food and net food change --------//

	err_when( vga.use_back_buf );

	char* strPtr = nationPtr->food_str();

	font_mid.disp( 307, 10, strPtr, 445);

	//------- display cash and profit --------//

	strPtr = nationPtr->cash_str();

	font_mid.disp( 305, 30, strPtr, 445);

	//------- display reputation ---------//

	if( nationPtr->reputation >= 0 )
	{
		str = m.format( (int)nationPtr->reputation, 4 );			// format type 4 - no thousand separators
	}
	else
	{
		str  = "-";
		str += m.format( (int)-nationPtr->reputation, 4 );		// format type 4 - no thousand separators
	}

	int reputationChange = (int) nationPtr->reputation_change_365days();

	if( reputationChange )
	{
		str += " (";

		if( reputationChange > 0 )
			str += "+";
		else
			str += "-";

		str += abs(reputationChange);
		str += ")";
	}

	image_icon.put_front(447,26, nationPtr->reputation_change_365days() >= (float)0.0 ? (char*)"REPU_UP" : (char*)"REPU_DW" );
	font_mid.disp( 476, 30, str, 575);
}
//-------- End of function Info::disp_heading --------//


//-------- Begin of function Info::detect --------//

int Info::detect()
{
	//------------ detect objects ------------//

	if( firm_array.selected_recno )
	{
		firm_array[firm_array.selected_recno]->detect_info_both();
	}
	else if( town_array.selected_recno )
	{
		town_array[town_array.selected_recno]->detect_info();
	}
	else if( site_array.selected_recno )
	{
		site_array[site_array.selected_recno]->detect_info();
	}
	else if( unit_array.selected_recno )
	{
		unit_array[unit_array.selected_recno]->detect_info();
	}

	return 0;
}
//-------- End of function Info::detect --------//


//-------- Begin of function Info::draw_selected --------//

void Info::draw_selected()
{
	if( firm_array.selected_recno )
		firm_array[firm_array.selected_recno]->draw_selected();

	else if( town_array.selected_recno )
		town_array[town_array.selected_recno]->draw_selected();

	else if( site_array.selected_recno )
		site_array[site_array.selected_recno]->draw_selected();

	else if( wall_res.selected_x_loc >= 0 )
		wall_res.draw_selected();
}
//-------- End of function Info::draw_selected --------//


//-------- Begin of function Info::get_report_data --------//

short Info::get_report_data(int recNo)
{
	err_when( recNo<1 || recNo>report_array.size() );

	return *((short*)report_array.get(recNo));
}
//-------- End of function Info::get_report_data --------//


//-------- Begin of function Info::get_report_data2 --------//

short Info::get_report_data2(int recNo)
{
	err_when( recNo<1 || recNo>report_array2.size() );

	return *((short*)report_array2.get(recNo));
}
//-------- End of function Info::get_report_data2 --------//


//-------- Begin of function Info::process_viewing_spy --------//

void Info::process_viewing_spy()
{
	//---- check if the viewing spy is still valid ----//

	int isValid=1;

	if( spy_array.is_deleted(viewing_spy_recno) )	// the spy is dead
	{
		isValid = 0;
	}
	else
	{
		Spy* spyPtr = spy_array[viewing_spy_recno];

		//-- check if the spy still stay in the same place --//

		if( spyPtr->spy_place_nation_recno() != info.viewing_nation_recno )
		{
			isValid = 0;
		}
		else
		{
		/*
			//--- on average, a spy will get caught in 5 to 15 days when viewing the secret of its enemy ---//

			if( m2.random(5+spyPtr->spy_skill/5) )		// use m2 to avoid multiplayer sync problem
			{
				spyPtr->set_exposed(COMMAND_PLAYER);
				isValid = 0;
			}
		*/
		}
	}

	if( !isValid )		//-- if not valid, set the mode back to normal viewing mode
		sys.set_view_mode(MODE_NORMAL);
}
//-------- End of function Info::process_viewing_spy --------//


//------- Begin of function Info::play_time_str --------//
//
char* Info::play_time_str()
{
	int totalMin = total_play_time / (60*1000);
	int playHour = totalMin / 60;
	int playMin  = totalMin - playHour*60;

	static String str;

	str = "";

	if( playHour > 0 )
	{
		str += playHour;
		str += translate.process( playHour>1 ? (char*)" hours" : (char*)" hour" );
		str += translate.process( " and " );
	}

	str += playMin;
	str += translate.process( playMin>1  ? (char*)" minutes" : (char*)" minute" );

	return str;
}
//---------- End of function Info::play_time_str ---------//


//------- Begin of function Info::game_duration_str --------//
//
char* Info::game_duration_str()
{
	//------- get the true game start date --------//

	int gameStartDate;

	//-- For scenarios (whose goal_difficulty are > 0 ), the actual game start date is no info.game_start_date, we must calculate it

	if( info.goal_difficulty )
	{
		gameStartDate =  date.julian( date.year(info.goal_deadline)-config.goal_year_limit,
							  date.month(info.goal_deadline),
							  date.day(info.goal_deadline) );
	}
	else
	{
		gameStartDate = info.game_start_date;
	}

	//---------------------------------------------//

	int totalDay = info.game_date - gameStartDate;
	int playYear = totalDay / 365;
	int playDay  = totalDay - playYear*365;

	static String str;

	str = "";

	if( playYear > 0 )
	{
		str += playYear;
		str += translate.process( playYear>1 ? (char*)" years" : (char*)" year" );
		str += translate.process( (char*)" and " );
	}

	str += playDay;
	str += translate.process( playDay>1  ? (char*)" days" : (char*)" day" );

	return str;
}
//---------- End of function Info::game_duration_str ---------//


//------- Begin of function Info::save_game_scr --------//
//
void Info::save_game_scr()
{
	err_when( save_buf_1 );

	// top and buttom
	if( 0 < ZOOM_Y1 )
	{
		save_buf_1  = vga_front.save_area(0, 0, VGA_WIDTH-1, ZOOM_Y1-1);
		save_buf_1b = vga_back.save_area(0, 0, VGA_WIDTH-1, ZOOM_Y1-1);		// save the back buffer also as the top area of the back buf is used for font display 
	}

	if( ZOOM_Y2 < VGA_HEIGHT-1 )
		save_buf_2 = vga_front.save_area(0, ZOOM_Y2+1, VGA_WIDTH-1, VGA_HEIGHT-1);

	// left and right
	if( 0 < ZOOM_X1 )
		save_buf_3 = vga_front.save_area(0, ZOOM_Y1, ZOOM_X1-1, ZOOM_Y2);

	if( ZOOM_X2 < VGA_WIDTH-1 )
		save_buf_4 = vga_front.save_area(ZOOM_X2+1, ZOOM_Y1, VGA_WIDTH-1, ZOOM_Y2);
}
//---------- End of function Info::save_game_scr ---------//


//------- Begin of function Info::rest_game_scr --------//
//
void Info::rest_game_scr()
{
	// restore area outside front buffer
	if(save_buf_4)
		vga_front.rest_area(save_buf_4, 1);
	if(save_buf_3)
		vga_front.rest_area(save_buf_3, 1);
	if(save_buf_2)
		vga_front.rest_area(save_buf_2, 1);
	if(save_buf_1)
		vga_front.rest_area(save_buf_1, 1);
	if(save_buf_1b)
		vga_back.rest_area(save_buf_1b, 1);

	save_buf_1 = NULL;

	info.disp();
	sys.blt_virtual_buf();		// blt the virtual front buffer to the screen
}
//---------- End of function Info::rest_game_scr ---------//


//------- Begin of function Info::free_game_scr --------//
//
void Info::free_game_scr()
{
	if(save_buf_4)
		mem_del(save_buf_4);

	if(save_buf_3)
		mem_del(save_buf_3);

	if(save_buf_2)
		mem_del(save_buf_2);

	if(save_buf_1)
		mem_del(save_buf_1);

	if(save_buf_1b)
		mem_del(save_buf_1b);

	save_buf_1 = NULL;
}
//---------- End of function Info::free_game_scr ---------//


//------- Begin of function Info::disp_loyalty --------//
//
// return: <int> x2 - the ending x position of the loyalty string.
//
int Info::disp_loyalty(int x, int y, int x2, int curLoyalty, int targetLoyalty, int nationRecno, int refreshFlag)
{
	int endX;

	if( x != x2 )		// if x==x2, don't display the field name.
	{
		font_san.field( x, y, "Loyalty", x2, curLoyalty, 1, INFO_X2-2, refreshFlag);
		endX = x2 + 4 + font_san.text_width( m.format(curLoyalty) );
	}
	else
	{
		endX = font_san.put( x2+4, y+2, curLoyalty, 1 );
	}

	if( nation_array[nationRecno]->cash <= 0 )		// if the nation no longer has money to pay the unit
		targetLoyalty = 0;

	if( curLoyalty != targetLoyalty )		// only increase, no decrease. Decrease are caused by events. Increases are made gradually
	{
		int tx = x2+6+font_san.text_width( m.format(curLoyalty) );

		if( targetLoyalty > curLoyalty )
			image_icon.put_front( tx, y+2, "ARROWUP" );

		else if( targetLoyalty < curLoyalty )
			image_icon.put_front( tx, y+2, "ARROWDWN" );

		endX = font_san.put( tx+10, y+2, targetLoyalty, 1 );
	}

	return endX;
}
//---------- End of function Info::disp_loyalty ---------//


