//Filename    : OR_NAT.CPP
//Description : Nation Report

#include <stdlib.h>
#include <KEY.H>
#include <OCONFIG.H>
#include <OVGA.H>
#include <OFONT.H>
#include <OSTR.H>
#include <OGETA.H>
#include <ODATE.H>
#include <OMOUSE.H>
#include <OIMGRES.H>
#include <OVBROWIF.H>
#include <OBUTTON.H>
#include <OSYS.H>
#include <ONATION.H>
#include <OTALKRES.H>
#include <OINFO.H>
#include <OREMOTE.H>

//------------- Define coordinations -----------//

enum { REPORT_BROWSE_X1 = ZOOM_X1+6,
		 REPORT_BROWSE_Y1 = ZOOM_Y1+6,
		 REPORT_BROWSE_X2 = ZOOM_X2-6,
		 REPORT_BROWSE_Y2 = REPORT_BROWSE_Y1+206,

		 REPORT_BUTTON_X1 = ZOOM_X1+8,
		 REPORT_BUTTON_Y1 = REPORT_BROWSE_Y2+6,
		 REPORT_BUTTON_WIDTH = 135,
		 REPORT_BUTTON_Y2 = REPORT_BUTTON_Y1+20,
		 REPORT_BUTTON_X_SPACE = 130,

		 REPORT_DET_X1 = ZOOM_X1+6,
		 REPORT_DET_Y1 = REPORT_BUTTON_Y2+4,
		 REPORT_DET_X2 = ZOOM_X2-6,
		 REPORT_DET_Y2 = ZOOM_Y2-6,
		 REPORT_TALK_Y2 = ZOOM_Y2-86,		// use a shorter height for the diplomacy menu as it needs to let the player looks at the messages received.
	 };

//---------- Define report mode strings ---------//

static char* report_mode_str_array[MAX_NATION_REPORT_MODE] =
{
	"Information",
	"Diplomacy",
	"Chat",
	"Diplomatic Log",
	"Debug",
};

//---------- Define chat receiver str strings ---------//

static char* chat_receiver_str_array[MAX_CHAT_RECEIVER_TYPE] =
{
	"Send to the selected kingdom",
	"Send to all allied human controlled kingdoms",
	"Send to all human controlled kingdoms",
};

//----------- Define static variables ----------//

static VBrowseIF 	 browse_nation, browse_talk_msg;
static ButtonGroup button_allow_attack(2);
static ButtonGroup button_chat_receiver(MAX_CHAT_RECEIVER_TYPE);
static init_get_chat = 0;
static GetA 		 get_chat;

//----------- Define static functions ----------//

static void put_nation_rec(int recNo, int x, int y, int refreshFlag);
static void put_talk_msg_rec(int recNo, int x, int y, int refreshFlag);
static int  nation_filter(int recNo=0);
static void disp_button();
static int  detect_button();
static void disp_detail(int refreshFlag);
static void detect_detail();
static void disp_nation_info();
static void detect_nation_info();
static void disp_debug_info();
static void disp_nation_talk();
static void detect_nation_talk();
static void disp_nation_chat(int refreshFlag);
static void detect_nation_chat();
static void disp_talk_msg_sent(int refreshFlag);

static int  sort_talk_msg( const void *a, const void *b );


//--------- Begin of function Info::disp_nation ---------//
//
// <int> refreshFlag - INFO_REPAINT - the user has just switched
//												  to this report.
//							  INFO_UPDATE  - the report is already in this mode,
//												  just keep displaying and updating.
//
void Info::disp_nation(int refreshFlag)
{
	//--------- Create nation income and expenses browsers -------//

	int x=REPORT_BROWSE_X1+9, y=REPORT_BROWSE_Y1+4;

	vga_back.d3_panel_up(REPORT_BROWSE_X1, REPORT_BROWSE_Y1, REPORT_BROWSE_X2, REPORT_BROWSE_Y1+33 );

	font_san.put( x	 , y+7, "Kingdom" );

	font_san.put( x+185, y+7, "Reputation" );
	font_san.put( x+275, y+7, "Status" );


#if(defined(SPANISH))
	font_san.put( x+335, y   , "Allow" );
	font_san.put( x+335, y+13, "Attack" );

	font_san.put( x+395, y,    "Acuerdo" );
	font_san.put( x+395, y+13, "Comerc." );

	font_san.put( x+468, y   , "Cantidad" );
	font_san.put( x+468, y+13, "Comerciada" );
#elif(defined(FRENCH))
	font_san.put( x+335, y   , "Allow" );
	font_san.put( x+335, y+13, "Attack" );

	font_san.put( x+395, y,    "Accord" );
	font_san.put( x+395, y+13, "Commercial" );

	font_san.put( x+473, y   , "Montant" );
	font_san.put( x+473, y+13, "Commercial" );
#elif(defined(GERMAN))
	font_san.put( x+345, y   , "Allow" );
	font_san.put( x+343, y+13, "Attack" );

	font_san.put( x+405, y   , "Handels-" );
	font_san.put( x+405, y+13, "Vertrag" );

	font_san.put( x+465, y   , "Handels-" );
	font_san.put( x+465, y+13, "Betrag" );
#else
	font_san.put( x+345, y   , "Allow" );
	font_san.put( x+343, y+13, "Attack" );

	font_san.put( x+405, y   , "Trade" );
	font_san.put( x+405, y+13, "Treaty" );

	font_san.put( x+465, y   , "Trade" );
	font_san.put( x+465, y+13, "Amount" );
#endif


	if( refreshFlag == INFO_REPAINT )
	{
		browse_nation.init( REPORT_BROWSE_X1, REPORT_BROWSE_Y1+35, REPORT_BROWSE_X2, REPORT_BROWSE_Y2,
								  0, 20, nation_filter(), put_nation_rec, 1 );

		browse_nation.open(browse_nation_recno);
	}
	else
	{
		browse_nation.paint();
		browse_nation.open(browse_nation_recno, nation_filter());
	}

	browse_nation_recno = browse_nation.recno();

	//------- disp report buttons --------//

	disp_button();

	//--------- display detail info --------//

	if( refreshFlag==INFO_REPAINT )
	{
		info.last_talk_nation_recno = 0;
		browse_talk_msg_recno = 1;
	}

	disp_detail(refreshFlag);
}
//----------- End of function Info::disp_nation -----------//


//--------- Begin of function Info::detect_nation ---------//
//
void Info::detect_nation()
{
	//------- detect nation browser ------//

	if( browse_nation.detect() )
	{
		browse_nation_recno = browse_nation.recno();
		return;
	}

	//------- detect report buttons --------//

	if( detect_button() )
		return;

	//--------- detect detail info -------//

	detect_detail();
}
//----------- End of function Info::detect_nation -----------//


//--------- Begin of function Info::init_player_reply ---------//
//
// This function is called by TalkRes::player_reply() when the
// player is going to reply to a diplomatic message from another
// nation.
//
// <int> talkToNationRecno - recno of the nation the player is
//									  going to talk to.
//
void Info::init_player_reply(int talkToNationRecno)
{
	nation_report_mode = NATION_REPORT_TALK;

	info.player_reply_mode = 1;

	//--- go to the nation in the browser which the player is going to talk to ---//

	int nationCount = nation_filter();

	for( int i=1 ; i<=nationCount ; i++ )
	{
		if( nation_filter(i) == talkToNationRecno )
		{
			browse_nation_recno = i;
			break;
		}
	}

	err_when( i>nationCount );

	//----- if currently it's in nation report mode -----//

	if( sys.view_mode == MODE_NATION )
		browse_nation.refresh(browse_nation_recno);
}
//----------- End of function Info::init_player_reply -----------//


//-------- Begin of static function nation_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of nations of this nation
//
// 2. when <int> recNo is given :
//    - return the nation recno in nation_array of the given recno.
//
static int nation_filter(int recNo)
{
	int    	i, nationCount=0;
	Nation*  viewingNation = nation_array[info.viewing_nation_recno];

	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( i==info.viewing_nation_recno ||
			 viewingNation->get_relation(i)->has_contact )
		{
			nationCount++;
		}

		if( recNo && nationCount==recNo )
			return i;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return nationCount;
}
//----------- End of static function nation_filter -----------//


//-------- Begin of static function put_nation_rec --------//
//
static void put_nation_rec(int recNo, int x, int y, int refreshFlag)
{
	int	  nationRecno = nation_filter(recNo);
	Nation* nationPtr   = nation_array[nationRecno];
	Nation* viewingNation = nation_array[info.viewing_nation_recno];
	NationRelation* nationRelation = viewingNation->get_relation(nationRecno);

	x+=3;
	y+=5;

	nationPtr->disp_nation_color(x, y+1);

	font_san.put( x+20 , y, nationPtr->nation_name(), 0, x+215 );
	font_san.put( x+220, y, m.format((int) nationPtr->reputation) );

	//------- display relation parameters --------//

	if( nationRecno != info.viewing_nation_recno )
	{
		font_san.put( x+272, y, nationRelation->status_str() );
		font_san.put( x+355, y, nationRelation->should_attack ? "Yes" : "No" );
		font_san.put( x+412, y, nationRelation->trade_treaty ? "Yes" : "No" );
		font_san.put( x+465, y, m.format( (int) viewingNation->total_year_trade(nationRecno),2) );

		if( config.show_ai_info )
		{
			font_san.put( x+510, y, m.format(nationPtr->get_relation(info.viewing_nation_recno)->ai_relation_level) );
			font_san.put( x+180, y, m.format( (int) nationPtr->get_relation(info.viewing_nation_recno)->good_relation_duration_rating) );
		}
	}
}
//----------- End of static function put_nation_rec -----------//


//--------- Begin of static function disp_button ---------//
//
static void disp_button()
{
	int x=REPORT_BUTTON_X1;

	for( int i=1 ; i<=MAX_NATION_REPORT_MODE ; i++ )
	{
		if( i==NATION_REPORT_DEBUG )
		{
			if( remote.is_enable() ||
				 ( !sys.debug_session && !sys.testing_session ) )
			{
				continue;
			}
		}
		else if( !sys.debug_session && i==NATION_REPORT_CHAT )
		{
			if( !remote.is_enable() )
				continue;
		}

		//-----------------------------------------//

		if( info.nation_report_mode == i )
			vga.d3_panel_down( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 );
		else
			vga.d3_panel_up( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 );

		font_san.center_put( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2, report_mode_str_array[i-1] );

		x+=REPORT_BUTTON_WIDTH;
	}
}
//----------- End of static function disp_button -----------//


//--------- Begin of static function detect_button ---------//
//
static int detect_button()
{
	int x=REPORT_BUTTON_X1;

	for( int i=1 ; i<=MAX_NATION_REPORT_MODE ; i++ )
	{
		if( i==NATION_REPORT_DEBUG )
		{
			if( remote.is_enable() ||
				 ( !sys.debug_session && !sys.testing_session ) )
			{
				continue;
			}
		}
		else if( !sys.debug_session && i==NATION_REPORT_CHAT )
		{
			if( !remote.is_enable() )
				continue;
		}

		//-----------------------------------------//

		if( mouse.single_click( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 ) )
		{
			info.nation_report_mode = i;
			return 1;
		}

		x+=REPORT_BUTTON_WIDTH;
	}

	return 0;
}
//----------- End of static function detect_button -----------//


//--------- Begin of static function disp_detail ---------//
//
static void disp_detail(int refreshFlag)
{
	if( browse_nation.recno()==0 )		// no records in the list
		return;

	switch( info.nation_report_mode )
	{
		case NATION_REPORT_INFO:
			disp_nation_info();
			break;

		case NATION_REPORT_TALK:
			disp_nation_talk();
			break;

		case NATION_REPORT_CHAT:
			disp_nation_chat(refreshFlag);
			break;

		case NATION_REPORT_TALK_LOG:
			disp_talk_msg_sent(refreshFlag);
			break;

		case NATION_REPORT_DEBUG:
			disp_debug_info();
			break;
	}
}
//----------- End of static function disp_detail -----------//


//--------- Begin of static function detect_detail ---------//
//
static void detect_detail()
{
	switch( info.nation_report_mode )
	{
		case NATION_REPORT_INFO:
			detect_nation_info();
			break;

		case NATION_REPORT_TALK:
			detect_nation_talk();
			break;

		case NATION_REPORT_CHAT:
			detect_nation_chat();
			break;

		case NATION_REPORT_TALK_LOG:
			if( browse_talk_msg.detect() )
				info.browse_talk_msg_recno = browse_talk_msg.recno();
			break;
	}
}
//----------- End of static function detect_detail -----------//


//--------- Begin of static function disp_nation_info ---------//
//
static void disp_nation_info()
{
	vga.d3_panel_down( REPORT_DET_X1, REPORT_DET_Y1, REPORT_DET_X2, REPORT_DET_Y2 );

	//----------- display info ------------//

	int	  nationRecno  = nation_filter(browse_nation.recno());
	Nation* nationPtr    = nation_array[nationRecno];
	Nation* viewingNation = nation_array[info.viewing_nation_recno];
	NationRelation* nationRelation = viewingNation->get_relation(nationRecno);

	int x1=REPORT_DET_X1+6, x2=REPORT_DET_X1+340, x3=REPORT_DET_X2-10;
	int y=REPORT_DET_Y1+6;

	//-------- display economic data ----------//

	String str;

	if( nationRecno == info.viewing_nation_recno )
	{
		font_san.put_field( x1, y, "Your Food", x2, nationPtr->food_str() );

		str  = "$";
		str += nationPtr->cash_str();

		font_san.put_field( x1, y+=16, "Your Treasure", x2, str );

		font_san.field( x1, y+=16, "Your Continuous Peace Time", x2, nationPtr->peace_duration_str(), x3, INFO_REPAINT, "PEACE" );
	}
	else
	{
		str  = translate.process("Your Yearly Import from ");
		str += nationPtr->nation_name();

		font_san.field( x1, y    , str, x2, (int) viewingNation->get_relation(nationRecno)->import_365days(IMPORT_TOTAL), 2, x3, INFO_REPAINT, "IMPORT" );

		str  = translate.process("Your Yearly Export to ");
		str += nationPtr->nation_name();

		font_san.field( x1, y+=16, str, x2, (int) nationPtr->get_relation(info.viewing_nation_recno)->import_365days(IMPORT_TOTAL), 2, x3, INFO_REPAINT, "EXPORT" );

		str  = translate.process("Continuous Peace Time of ");
		str += nationPtr->nation_name();

		font_san.field( x1, y+=16, str, x2, nationPtr->peace_duration_str(), x3, INFO_REPAINT, "PEACE" );

		//--------- duration of current status ----------//

#if(defined(SPANISH))
		str  = "Duración del estado de ";
		str += translate.process(nationRelation->status_str());
#elif(defined(FRENCH))
		str  = "Duration of ";
		str += nationRelation->status_str();
		str += " Status";
		str  = translate.process(str);
#else
		// GERMAN and US
		str  = translate.process("Duration of ");
		str += translate.process(nationRelation->status_str());
		str += " Status";
#endif

		font_san.field( x1, y+=16, str, x2, nationRelation->status_duration_str(), x3, INFO_REPAINT, "STATTIME" );

		//------- display the allow_attack field --------//

		#if(defined(SPANISH))
			// str  = "Permitir que tus Unidades ataquen al ";
			str  = "Permitir atacar al ";
			str += nationPtr->nation_name();
		#elif(defined(FRENCH))
			str  = "Autoriser attaque contre le ";
			str += nationPtr->nation_name();
		#elif(defined(GERMAN))
			str  = "Angriffe auf ";
			str += nationPtr->nation_name();
			str += " erlauben";
		#else
			str  = "Allow Your Units to Attack ";
			str += nationPtr->nation_name();
		#endif

		font_san.field( x1, y+=16, str, x2, "", x3, INFO_REPAINT, "ALLOWATK" );

		button_allow_attack[1].create_text( x2+6 , y, x2+50 , y+15, "Yes" );
		button_allow_attack[0].create_text( x2+54, y, x2+100, y+15, "No"  );

		button_allow_attack.paint( viewingNation->get_relation(nationRecno)->should_attack );

		//---- if this nation is our ally, display its cash and food ----//

		if( viewingNation->get_relation(nationRecno)->status == NATION_ALLIANCE )
		{
#if(defined(SPANISH))
			str  = "Tesoro del ";
			str += nationPtr->nation_name();
#elif(defined(FRENCH))
			str  = "Trésor du ";
			str += nationPtr->nation_name();
#else
			// GERMAN and US
			str  = nationPtr->nation_name();
			str += " 's Treasure";
#endif

			font_san.field( x1, y+=16, str, x2, nationPtr->cash, 2, x3, INFO_REPAINT );

#if(defined(SPANISH))
			str = "Alimentos del ";
			str += nationPtr->nation_name();
#elif(defined(FRENCH))
			str = "Réserves de nourriture du ";
			str += nationPtr->nation_name();
#else
			// GERMAN and US
			str  = nationPtr->nation_name();
			str += " 's Food";
#endif

			font_san.field( x1, y+=16, str, x2, nationPtr->food, 2, x3, INFO_REPAINT );
		}
	}

	y+=26;

	//----- display its relation status with other nations -----//

	if( info.viewing_nation_recno != nationRecno )
	{
		String str;

#if(defined(SPANISH))
		str  = "Estado Diplomático del ";
		str += nationPtr->nation_name();
		str += " con otros Reinos:";
#elif(defined(FRENCH))
		str  = "Relations diplomatiques de ";
		str += nationPtr->nation_name();
		str += " avec les autres royaumes:";
#else
		// GERMAN and US
		str  = nationPtr->nation_name();
		str += "'s ";
		str += translate.process( "Diplomatic Status with Other Kingdoms:" );
#endif

		nationPtr->disp_nation_color(x1, y+1);
		font_san.put(x1+20, y, str);
		y+=20;

		for( int i=1 ; i<=nation_array.size() ; i++ )
		{
			if( nation_array.is_deleted(i) ||
				 i == nationRecno ||
				 i == info.viewing_nation_recno )
			{
				continue;
			}

			nation_array[i]->disp_nation_color(x1, y+1);

			font_san.put( x1+20, y, nation_array[i]->nation_name() );
			font_san.put( x1+240, y, nationPtr->get_relation(i)->status_str() );

			if( nationPtr->get_relation(i)->trade_treaty )
			{
				font_san.put( x1+330, y, "Trade Treaty" );
#if(defined(FRENCH))
				font_san.put( x1+460, y, m.format((int)nationPtr->total_year_trade(i),2) );
#else
				// German and US
				font_san.put( x1+450, y, m.format((int)nationPtr->total_year_trade(i),2) );
#endif
			}

			if( config.show_ai_info )
			{
				font_san.put( x1+500, y, nationPtr->get_relation(i)->ai_relation_level );
				font_san.put( x1+530, y, (int) nationPtr->get_relation(i)->good_relation_duration_rating );
			}

			y+=16;
		}
	}
}
//----------- End of static function disp_nation_info -----------//


//--------- Begin of static function detect_nation_info ---------//
//
static void detect_nation_info()
{
	int nationRecno = nation_filter(browse_nation.recno());

	Nation* viewingNation = nation_array[info.viewing_nation_recno];

	if( button_allow_attack.detect() >= 0 )
		viewingNation->set_relation_should_attack(nationRecno, button_allow_attack.button_pressed, COMMAND_PLAYER);
}
//----------- End of static function detect_nation_info -----------//


//--------- Begin of static function disp_debug_info ---------//
//
static void disp_debug_info()
{
	vga.d3_panel_down( REPORT_DET_X1, REPORT_DET_Y1, REPORT_DET_X2, REPORT_DET_Y2 );

	//----------- display info ------------//

	int	  nationRecno  = nation_filter(browse_nation.recno());
	Nation* nationPtr    = nation_array[nationRecno];

	int x1=REPORT_DET_X1+6, x2=REPORT_DET_X1+160;
	int y=REPORT_DET_Y1+6;

	int refreshFlag = INFO_REPAINT;

	//------------ display AI info ----------//

	font_san.put_field( x1, y    , "Food    ", x2, nationPtr->food_str() );
	font_san.put_field( x1, y+=16, "Treasure", x2, nationPtr->cash_str() );
	font_san.put_field( x1, y+=16, "ai_capture_enemy_town_recno", x2, nationPtr->ai_capture_enemy_town_recno, 1 );
	font_san.put_field( x1, y+=16, "Surplus supply rating", x2, nationPtr->surplus_supply_rating() );
	y+=48;

	//--------- display AI preference ----------//

	x2 += 60;

	font_san.put_field( x1, y+=16, "Unit Chase Distance"     , x2, nationPtr->pref_unit_chase_distance     , 1 );
	font_san.put_field( x1, y+=16, "Military Development"	   , x2, nationPtr->pref_military_development	 , 1 );
	font_san.put_field( x1, y+=16, "Economic Development"	   , x2, nationPtr->pref_economic_development	 , 1 );
	font_san.put_field( x1, y+=16, "Increase Pop by Capture" , x2, nationPtr->pref_inc_pop_by_capture      , 1 );
	font_san.put_field( x1, y+=16, "Increase Pop by Growth"  , x2, nationPtr->pref_inc_pop_by_growth   	 , 1 );
	font_san.put_field( x1, y+=16, "Peacefulness" 		      , x2, nationPtr->pref_peacefulness        	 , 1 );
	font_san.put_field( x1, y+=16, "Military Courage"    	   , x2, nationPtr->pref_military_courage    	 , 1 );
	font_san.put_field( x1, y+=16, "Territorial Cohesiveness", x2, nationPtr->pref_territorial_cohesiveness, 1 );
	font_san.put_field( x1, y+=16, "Trading Tendency"        , x2, nationPtr->pref_trading_tendency        , 1 );

	x1 += (REPORT_DET_X2-REPORT_DET_X1)/2;
	x2 += (REPORT_DET_X2-REPORT_DET_X1)/2;

	y=REPORT_DET_Y1+6;

	font_san.put_field( x1, y    , "Allying Tendency"        , x2, nationPtr->pref_allying_tendency        , 1 );
	font_san.put_field( x1, y+=16, "Honesty"                 , x2, nationPtr->pref_honesty                 , 1 );
	font_san.put_field( x1, y+=16, "Town Harmony"         	, x2, nationPtr->pref_town_harmony			    , 1 );
	font_san.put_field( x1, y+=16, "Citizen Loyalty Concern" , x2, nationPtr->pref_loyalty_concern  		 , 1 );
	font_san.put_field( x1, y+=16, "Forgiveness"             , x2, nationPtr->pref_forgiveness      		 , 1 );
	font_san.put_field( x1, y+=16, "Collect Tax Tendency"    , x2, nationPtr->pref_collect_tax      		 , 1 );
	font_san.put_field( x1, y+=16, "Build Inn Tendency"      , x2, nationPtr->pref_hire_unit       		    , 1 );
	font_san.put_field( x1, y+=16, "Use Weapon Tendency"     , x2, nationPtr->pref_use_weapon       		 , 1 );
	font_san.put_field( x1, y+=16, "Keep Generals Tendency"  , x2, nationPtr->pref_keep_general            , 1 );
	font_san.put_field( x1, y+=16, "Keep Skilled Units Tendency", x2, nationPtr->pref_keep_skilled_unit    , 1 );
	font_san.put_field( x1, y+=16, "Attack Monster"          , x2, nationPtr->pref_attack_monster          , 1 );
	font_san.put_field( x1, y+=16, "Use Spies"               , x2, nationPtr->pref_spy                     , 1 );
	font_san.put_field( x1, y+=16, "Use Counter Spies"       , x2, nationPtr->pref_counter_spy             , 1 );
	font_san.put_field( x1, y+=16, "Food Reserve"            , x2, nationPtr->pref_food_reserve            , 1 );
	font_san.put_field( x1, y+=16, "Cash Reserve"            , x2, nationPtr->pref_cash_reserve            , 1 );
	font_san.put_field( x1, y+=16, "Use Marine"              , x2, nationPtr->pref_use_marine              , 1 );
}
//----------- End of static function disp_debug_info -----------//


//--------- Begin of static function disp_nation_talk ---------//
//
static void disp_nation_talk()
{
	int nationRecno = nation_filter(info.browse_nation_recno);

	//----- if the selected nation is the viewing nation ----//

	if( nationRecno != info.viewing_nation_recno &&
		 info.viewing_nation_recno == nation_array.player_recno )
	{
		vga.d3_panel_down( REPORT_DET_X1, REPORT_DET_Y1, REPORT_DET_X2, REPORT_TALK_Y2 );

		if( info.last_talk_nation_recno != nationRecno && !info.player_reply_mode )
			talk_res.init_conversion(nationRecno);

		info.player_reply_mode = 0;
		info.last_talk_nation_recno = nationRecno;

		talk_res.disp_talk();
	}
}
//----------- End of static function disp_nation_talk -----------//


//--------- Begin of static function detect_nation_talk ---------//
//
static void detect_nation_talk()
{
	int nationRecno = nation_filter(info.browse_nation_recno);

	//----- if the selected nation is the viewing nation ----//

	if( nationRecno != info.viewing_nation_recno &&
		 info.viewing_nation_recno == nation_array.player_recno )
	{
		talk_res.detect_talk();
	}
}
//----------- End of static function detect_nation_talk -----------//


//--------- Begin of static function disp_talk_msg_sent ---------//
//
static void disp_talk_msg_sent(int refreshFlag)
{
	//--- filter out talk messages sent by this nation and sort them by date ---//

	TalkMsg* 	talkMsg;
	TalkMsgDisp talkMsgDisp;

	static short lastNationRecno=0;
	int nationRecno = nation_filter(info.browse_nation_recno);

	if( lastNationRecno != nationRecno )
	{
		info.browse_talk_msg_recno = 1;		// reset the browser recno of the viewing nation has been changed
		lastNationRecno = nationRecno;
	}

	//--------------------------------------------//

	info.talk_msg_disp_array.zap();

	for( short i=1 ; i<=talk_res.talk_msg_count() ; i++ )
	{
		if( talk_res.is_talk_msg_deleted(i) )
			continue;

		talkMsg = talk_res.get_talk_msg(i);

		if( !talkMsg->is_valid_to_disp() )		// don't link it out, otherwise it may cause multiplayer sync problem. 
			continue;

		if( talkMsg->from_nation_recno == nationRecno ||
			 talkMsg->to_nation_recno == nationRecno )
		{
			if( talkMsg->from_nation_recno == info.viewing_nation_recno ||
				 talkMsg->to_nation_recno == info.viewing_nation_recno )
			{
				if( !nation_array.is_deleted(talkMsg->from_nation_recno) &&
					 !nation_array.is_deleted(talkMsg->to_nation_recno) )
				{
					talkMsgDisp.recno = i;
					talkMsgDisp.date  = talkMsg->date;
					talkMsgDisp.is_reply = 0;

					info.talk_msg_disp_array.linkin(&talkMsgDisp);

					if( talkMsg->reply_date )
					{
						talkMsgDisp.recno = i;
						talkMsgDisp.date  = talkMsg->reply_date;
						talkMsgDisp.is_reply = 1;

						info.talk_msg_disp_array.linkin(&talkMsgDisp);
					}
				}
			}
		}
	}

	if( info.talk_msg_disp_array.size() > 0 )
		info.talk_msg_disp_array.quick_sort(sort_talk_msg);

	//----- display a browser of the talk msg sent -----//

	if( refreshFlag == INFO_REPAINT || !browse_talk_msg.init_flag )
	{
		browse_talk_msg.init( REPORT_DET_X1, REPORT_DET_Y1, REPORT_DET_X2, REPORT_DET_Y2,
									 0, 30, info.talk_msg_disp_array.size(), put_talk_msg_rec, 1 );

		browse_talk_msg.open(info.browse_talk_msg_recno);
	}
	else
	{
		browse_talk_msg.paint();
		browse_talk_msg.open(info.browse_talk_msg_recno, info.talk_msg_disp_array.size());
	}

	info.browse_talk_msg_recno = browse_talk_msg.recno();
}
//----------- End of static function disp_talk_msg_sent -----------//


//------ Begin of function sort_talk_msg ------//
//
static int sort_talk_msg( const void *a, const void *b )
{
	if( ((TalkMsgDisp*)b)->recno == ((TalkMsgDisp*)a)->recno )
		return ((TalkMsgDisp*)b)->is_reply ? 1 : -1;

	return ((TalkMsgDisp*)b)->date - ((TalkMsgDisp*)a)->date;
}
//------- End of function sort_talk_msg ------//


//-------- Begin of static function put_talk_msg_rec --------//
//
static void put_talk_msg_rec(int recNo, int x1, int y, int refreshFlag)
{
	TalkMsgDisp* talkMsgDisp = (TalkMsgDisp*) info.talk_msg_disp_array.get(recNo);
	TalkMsg*		 talkMsg = talk_res.get_talk_msg(talkMsgDisp->recno);

	int x=x1, isTo;

	x+=3;
	y+=2;

	if( !talkMsgDisp->is_reply )		// if not a replying message
		isTo = talkMsg->from_nation_recno == info.viewing_nation_recno;	// you send a message to a nation
	else
		isTo = talkMsg->from_nation_recno != info.viewing_nation_recno;

	//---------------------------------------//

	char* str1;

	if( isTo )
		str1 = translate.process("To");
	else
		str1 = translate.process("From");

	font_san.put( x , y, str1 );

	x += font_san.text_width(str1)+5;

	//---------------------------------------//

	if( talkMsg->from_nation_recno == info.viewing_nation_recno )
		nation_array[talkMsg->to_nation_recno]->disp_nation_color(x, y+2);
	else
		nation_array[talkMsg->from_nation_recno]->disp_nation_color(x, y+2);

	x+=18;

	String str;

	str = translate.process("on ");

	if( talkMsgDisp->is_reply )
		str += date.date_str(talkMsg->reply_date);
	else
		str += date.date_str(talkMsg->date);

	str += " :";

	font_san.put( x , y, str );

	font_san.put( x1, y+13, talkMsg->msg_str(info.viewing_nation_recno, talkMsgDisp->is_reply), 0, browse_talk_msg.ix2 );
}
//----------- End of static function put_talk_msg_rec -----------//


//--------- Begin of static function disp_nation_chat ---------//
//
static void disp_nation_chat(int refreshFlag)
{
	int nationRecno = nation_filter(info.browse_nation_recno);

	//----- if the selected nation is the viewing nation ----//

	if( !(sys.debug_session || nation_array[nationRecno]->nation_type == NATION_REMOTE &&
		 info.viewing_nation_recno == nation_array.player_recno ) )
	{
		return;
	}

	//-------- display the chat get control -------//

	vga.d3_panel_down( REPORT_DET_X1, REPORT_DET_Y1, REPORT_DET_X2, REPORT_TALK_Y2 );

	font_san.put( REPORT_DET_X1+10, REPORT_DET_Y1+10,
					  "Please enter your chat message and press <Enter> to send." );

	// ###### begin Gilbert 15/10 #######//
	if( !init_get_chat )
	{
		get_chat.init( REPORT_DET_X1+10, REPORT_DET_Y1+30, REPORT_DET_X2-10,
			info.player_chat_str, CHAT_STR_LEN, &font_san, 0, 0 );
		get_chat.clear();
		init_get_chat = 1;
	}
	// ###### end Gilbert 15/10 #######//

	int vgaUseBack = vga.use_back_buf;

	vga.use_back();
	get_chat.paint(1);			// 1-paint cursor

	if( !vgaUseBack )
		vga.use_front();

	//-------- display receiver type buttons ---------//

	int y=REPORT_TALK_Y2-70;

	for( int i=1 ; i<=MAX_CHAT_RECEIVER_TYPE ; i++, y+=22 )
	{
		if( info.chat_receiver_type == i )
			vga.d3_panel_down( REPORT_DET_X1+10, y, REPORT_DET_X2-10, y+20 );
		else
			vga.d3_panel_up( REPORT_DET_X1+10, y, REPORT_DET_X2-10, y+20 );

		font_san.center_put( REPORT_DET_X1+10, y, REPORT_DET_X2-10, y+20, chat_receiver_str_array[i-1] );
	}
}
//----------- End of static function disp_nation_chat -----------//


//--------- Begin of static function detect_nation_chat ---------//
//
static void detect_nation_chat()
{
	int nationRecno = nation_filter(info.browse_nation_recno);

	//----- if the selected nation is the viewing nation ----//

	if( sys.debug_session || nation_array[nationRecno]->nation_type == NATION_REMOTE &&
		 info.viewing_nation_recno == nation_array.player_recno )
	{
		// ###### begin Gilbert 15/10 #######//
		if( !init_get_chat )
		{
			get_chat.init( REPORT_DET_X1+10, REPORT_DET_Y1+30, REPORT_DET_X2-10,
				info.player_chat_str, CHAT_STR_LEN, &font_san, 0, 0 );
			get_chat.clear();
			init_get_chat = 1;
		}
		// ###### end Gilbert 15/10 #######//

		for( int i=0 ; i<5 ; i++ )
		{
			int keyCode = get_chat.detect();

			if( !keyCode )
				break;

			//---- press <Enter> to send the chat message ----//

			if( keyCode == KEY_RETURN && info.player_chat_str[0] != '\0')
			{
				if( remote.is_enable() )
				{
					// packet structure : <to nation recno> <from nation recno> <char[CHAT_STR_LEN+1]>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_CHAT, 2*sizeof(short) + strlen(info.player_chat_str)+1 );

					switch( info.chat_receiver_type )
					{
					case 1:		// selected
						shortPtr[0] = nationRecno;
						break;
					case 2:		// all allies
						shortPtr[0] = -1;
						break;
					case 3:		// all human
						shortPtr[0] = 0;
						break;
					}
					shortPtr[1] = info.viewing_nation_recno;

					strcpy(2*sizeof(short)+(char *)shortPtr, info.player_chat_str);
				}

				get_chat.clear();
				break;
			}

			mouse.get_event();
		}
	}

	//-------- detect receiver type buttons ---------//

	int y=REPORT_TALK_Y2-70;

	for( int i=1 ; i<=MAX_CHAT_RECEIVER_TYPE ; i++, y+=22 )
	{
		if( mouse.single_click( REPORT_DET_X1+10, y, REPORT_DET_X2-50, y+20 ) )
		{
			info.chat_receiver_type = i;
			return;
		}
	}
}
//----------- End of static function detect_nation_chat -----------//


//------- Begin of function Info::player_reply_chat --------//
//
// <int> withNationRecno - player reply a chat message to this nation.
//
void Info::player_reply_chat(int withNationRecno)
{
	nation_report_mode = NATION_REPORT_CHAT;

	//--- go to the nation in the browser which the player is going to talk to ---//

	int nationCount = nation_filter();

	for( int i=1 ; i<=nationCount ; i++ )
	{
		if( nation_filter(i) == withNationRecno )
		{
			browse_nation_recno = i;
			break;
		}
	}

	err_when( i>nationCount );

	sys.set_view_mode(MODE_NATION);
}
//-------- End of function Info::player_reply_chat ---------//

