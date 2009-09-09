//Filename    : OR_SPY.CPP
//Description : Spy Report

#include <OVGA.H>
#include <OFONT.H>
#include <OFIRMRES.H>
#include <ONATION.H>
#include <ORACERES.H>
#include <OIMGRES.H>
#include <OVBROWIF.H>
#include <OSPY.H>
#include <OINFO.H>

//------------- Define coordinations -----------//

enum { SPY_BROWSE_X1 = ZOOM_X1+6,
		 SPY_BROWSE_Y1 = ZOOM_Y1+6,
		 SPY_BROWSE_X2 = ZOOM_X2-6,
		 SPY_BROWSE_Y2 = ZOOM_Y2-6,
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_spy;

//----------- Define static functions ----------//

static void put_spy_rec(int recNo, int x, int y, int refreshFlag);
static int  spy_filter(int recNo=0);
static void	disp_total();

//--------- Begin of function Info::disp_spy ---------//
//
void Info::disp_spy(int refreshFlag)
{
	//------- Display the spy report -------//

	int x=SPY_BROWSE_X1+9;
	int y=SPY_BROWSE_Y1+4;

	vga_back.d3_panel_up(SPY_BROWSE_X1, SPY_BROWSE_Y1, SPY_BROWSE_X2, SPY_BROWSE_Y1+20 );

#if(defined(FRENCH))
	font_san.put( x	 , y, "Spy Name" );
	font_san.put( x+155, y, "Cloak" );
	font_san.put( x+205, y, "Location" );
	font_san.put( x+320, y, "Skill" );
	font_san.put( x+384, y, "Loyalty" );
	font_san.put( x+448, y, "Action" );
#else
	// German and US
	font_san.put( x	 , y, "Spy Name" );
	font_san.put( x+155, y, "Cloak" );
	font_san.put( x+205, y, "Location" );
	font_san.put( x+330, y, "Skill" );
	font_san.put( x+370, y, "Loyalty" );
	font_san.put( x+435, y, "Action" );
#endif

	if( refreshFlag == INFO_REPAINT )
	{
		browse_spy.init( SPY_BROWSE_X1, SPY_BROWSE_Y1+22, SPY_BROWSE_X2, SPY_BROWSE_Y2-20,
								 0, 21, spy_filter(), put_spy_rec, 1 );

		browse_spy.open(browse_spy_recno);
	}
	else
	{
		browse_spy.paint();
		browse_spy.open(browse_spy_recno, spy_filter());
	}

	//--------- Display total ------------//

	disp_total();
}
//----------- End of function Info::disp_spy -----------//


//--------- Begin of function Info::detect_spy ---------//
//
void Info::detect_spy()
{
	//------- detect the spy browser -------//

	if( browse_spy.detect() )
	{
		browse_spy_recno = browse_spy.recno();

		if( browse_spy.double_click )
		{
			Spy* spyPtr = spy_array[ spy_filter(browse_spy_recno) ];
			int  xLoc, yLoc;

			if( spyPtr->get_loc(xLoc, yLoc) )
				world.go_loc( xLoc, yLoc, 1 );
		}
	}
}
//----------- End of function Info::detect_spy -----------//


//-------- Begin of static function disp_total --------//

static void disp_total()
{
	int x = SPY_BROWSE_X1+9;
	int y = SPY_BROWSE_Y2-16;

	vga_back.d3_panel_up(SPY_BROWSE_X1, SPY_BROWSE_Y2-18, SPY_BROWSE_X2, SPY_BROWSE_Y2 );

	String str;

	if( browse_spy.total_rec() > 1 )
		str = "Total Spies";
	else
		str = "Total Spy";

	str  = translate.process(str);
	str += ": ";
	str += browse_spy.total_rec();

	font_san.put( x, y, str );
}
//----------- End of static function disp_total -----------//


//-------- Begin of static function spy_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int spy_filter(int recNo)
{
	int  totalSpy = spy_array.size();
	int  spyCount=0;
	Spy* spyPtr;

	for( int spyRecno=1 ; spyRecno<=totalSpy ; spyRecno++ )
	{
		if( spy_array.is_deleted(spyRecno) )
			continue;

		spyPtr = spy_array[spyRecno];

		if( spyPtr->true_nation_recno==info.viewing_nation_recno )
			spyCount++;

		if( recNo && spyCount==recNo )
			return spyRecno;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return spyCount;
}
//----------- End of static function spy_filter -----------//


//-------- Begin of static function put_spy_rec --------//
//
static void put_spy_rec(int recNo, int x, int y, int refreshFlag)
{
	int  spyRecno = spy_filter(recNo);
	Spy* spyPtr   = spy_array[spyRecno];

	x+=3;
	y+=5;

	//------ display rank/skill icon -------//

	int 	 cloakedRankId  = spyPtr->cloaked_rank_id();
	int 	 cloakedSkillId = spyPtr->cloaked_skill_id();
	String str;

	switch( cloakedRankId )
	{
		case RANK_KING:
			str = "U_KING";
			break;

		case RANK_GENERAL:
			str = "U_GENE";
			break;

		case RANK_SOLDIER:
			if( cloakedSkillId )
			{
				str  = "U_";
				str += Skill::skill_code_array[cloakedSkillId-1];
			}
			else
			{
				str = "";
			}
			break;
	}

	if( str.len() > 0 )
		image_icon.put_back(x, y+1, str);

	//------ display race icon -------------//

	vga_back.put_bitmap( x+13, y-4, race_res[spyPtr->race_id]->icon_bitmap_ptr );

	//----------- display name -----------//

	font_san.put( x+39, y, race_res[spyPtr->race_id]->get_name(spyPtr->name_id), 0, 185 );

	//------- display cloaked nation color ------//

	int tx = x+170;

	if( spyPtr->cloaked_nation_recno==0 )		// independent nation
	{
		vga_back.bar( tx, y, tx+12, y+12, V_WHITE );
		vga_back.rect( tx, y, tx+12, y+12, 1, VGA_GRAY+8 );
	}
	else
	{
		nation_array[spyPtr->cloaked_nation_recno]->disp_nation_color(tx, y+2);
	}

	//---------- display other info ----------//

	switch( spyPtr->spy_place )
	{
		case SPY_FIRM:
			str = firm_res[firm_array[spyPtr->spy_place_para]->firm_id]->name;
			break;

		case SPY_TOWN:
			str = town_array[spyPtr->spy_place_para]->town_name();
			break;

		case SPY_MOBILE:
		{
			Unit* unitPtr = unit_array[spyPtr->spy_place_para];

			switch( unitPtr->unit_mode )
			{
				case UNIT_MODE_CONSTRUCT:
					str = firm_res[firm_array[unitPtr->unit_mode_para]->firm_id]->name;
					break;

				case UNIT_MODE_ON_SHIP:
					str = "On Ship";
					break;

				default:
					str = "Mobile";
			}
			break;
		}

		default:
			str = "";
	}

	font_san.put( x+205, y, str );

	font_san.put( x+335, y, spyPtr->spy_skill );
	font_san.put( x+385, y, spyPtr->spy_loyalty );
	font_san.put( x+435, y, spyPtr->action_str() );
}
//----------- End of static function put_spy_rec -----------//


