//Filename    : OR_TOWN.CPP
//Description : Town Report

#include <OVGA.h>
#include <OFONT.h>
#include <OIMGRES.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <ORACERES.h>
#include <OFIRM.h>
#include <OFIRM.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OINFO.h>

//------------- Define coordinations -----------//

enum { TOWN_BROWSE_X1 = ZOOM_X1+6,
		 TOWN_BROWSE_Y1 = ZOOM_Y1+6,
		 TOWN_BROWSE_X2 = ZOOM_X2-6,
		 TOWN_BROWSE_Y2 = TOWN_BROWSE_Y1+280
	  };

enum { POP_TOTAL_X1 = ZOOM_X1+6,
		 POP_TOTAL_Y1 = TOWN_BROWSE_Y2+2,
		 POP_TOTAL_X2 = ZOOM_X2-6,
		 POP_TOTAL_Y2 = POP_TOTAL_Y1+18,
	  };

enum { FIRM_BROWSE_X1 = ZOOM_X1+6,
		 FIRM_BROWSE_Y1 = POP_TOTAL_Y2+6,
		 FIRM_BROWSE_X2 = ZOOM_X2-6,
		 FIRM_BROWSE_Y2 = ZOOM_Y2-6,
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_town, browse_firm;
static int		  total_population, total_peasant;
static int  	  firm_income_array[MAX_FIRM_TYPE];
static int  	  total_firm_cost, total_firm_income, total_firm_count;
static float	  total_expense;

//----------- Define static functions ----------//

static void put_town_rec(int recNo, int x, int y, int refreshFlag);
static void put_firm_rec(int recNo, int x, int y, int refreshFlag);
static int  town_filter(int recNo=0);
static int  firm_filter(int recNo=0);
static void	disp_total();
static void calc_firm_total();

//--------- Begin of function Info::disp_town ---------//
//
void Info::disp_town(int refreshFlag)
{
	int x=TOWN_BROWSE_X1+9;
	int y=TOWN_BROWSE_Y1+4;

	vga_back.d3_panel_up(TOWN_BROWSE_X1, TOWN_BROWSE_Y1, TOWN_BROWSE_X2, TOWN_BROWSE_Y1+20 );

	font_san.put( x	 , y, "Village" );
	font_san.put( x+150, y, "Villagers" );
	font_san.put( x+225, y, "Peasants" );
	font_san.put( x+295, y, "Loyalty" );
	font_san.put( x+355, y, "Races" );

	if( refreshFlag == INFO_REPAINT )
	{
		browse_town.init( TOWN_BROWSE_X1, TOWN_BROWSE_Y1+22, TOWN_BROWSE_X2, TOWN_BROWSE_Y2-20,
								0, 21, town_filter(), put_town_rec, 1 );

		browse_town.open(browse_town_recno);
	}
	else
	{
		browse_town.paint();
		browse_town.open(browse_town_recno, town_filter());
	}

	//------- Display the firm report -------//

	calc_firm_total();

	x=FIRM_BROWSE_X1+9;
	y=FIRM_BROWSE_Y1+4;

	vga_back.d3_panel_up(FIRM_BROWSE_X1, FIRM_BROWSE_Y1, FIRM_BROWSE_X2, FIRM_BROWSE_Y1+20 );

	font_san.put( x	 , y, "Structure" );
	font_san.put( x+140, y, "Unit Cost" );
#if(defined(FRENCH))
	font_san.put( x+237, y, "No. of Structures" );
#else
	font_san.put( x+217, y, "No. of Structures" );
#endif
	font_san.put( x+340, y, "Yearly Expense" );
	font_san.put( x+450, y, "Yearly Income" );

	if( refreshFlag == INFO_REPAINT )
	{
		browse_firm.init( FIRM_BROWSE_X1, FIRM_BROWSE_Y1+22, FIRM_BROWSE_X2, FIRM_BROWSE_Y2-20,
								0, 16, firm_filter(), put_firm_rec, 1 );

		browse_firm.open(browse_firm_recno);
	}
	else
	{
		browse_firm.paint();
		browse_firm.open(browse_firm_recno, firm_filter());
	}

	//--------- Display total ------------//

	disp_total();
}
//----------- End of function Info::disp_town -----------//


//--------- Begin of function Info::detect_town ---------//
//
void Info::detect_town()
{
   //------- detect the town browser -------//

	if( browse_town.detect() )
	{
		browse_town_recno = browse_town.recno();

		if( browse_town.double_click )
		{
			Town* townPtr = town_array[ town_filter(browse_town_recno) ];

			world.go_loc(townPtr->center_x, townPtr->center_y, 1);
		}
	}

	//------- detect the firm browser -------//

	if( browse_firm.detect() )
		browse_firm_recno = browse_firm.recno();
}
//----------- End of function Info::detect_town -----------//


//-------- Begin of static function disp_total --------//

static void disp_total()
{
	//------- calculate total --------//

	total_population = 0;
	total_peasant	  = 0;

	Town* townPtr;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->nation_recno==info.viewing_nation_recno )
		{
			total_population += townPtr->population;
			total_peasant	  += townPtr->jobless_population;
		}
	}

	//-------- display town total --------//

	int x=TOWN_BROWSE_X1+9;
	int y=TOWN_BROWSE_Y2-16;

	vga_back.d3_panel_up(TOWN_BROWSE_X1, TOWN_BROWSE_Y2-18, TOWN_BROWSE_X2, TOWN_BROWSE_Y2 );

	String str;

	if( browse_town.total_rec() > 1 )
		str = "Total Villages";
	else
		str = "Total Village";

	str  = translate.process(str);
	str += ": ";
	str += browse_town.total_rec();

	font_san.put( x, y, str );

   //-------------------------------//

	str  = "Total Villagers";

	str  = translate.process(str);
	str += ": ";
	str += total_population;

	font_san.put( x+180, y, str );

	//-------------------------------//

	if( total_peasant > 1 )
		str = "Total Peasants";
	else
		str = "Total Peasant";

	str  = translate.process(str);
	str += ": ";
	str += total_peasant;

	font_san.put( x+360, y, str );

	//------- display other totals --------//

	Nation* viewNation = nation_array[info.viewing_nation_recno];

	x=POP_TOTAL_X1+9;
	y=POP_TOTAL_Y1+2;

	vga_back.d3_panel_up(POP_TOTAL_X1, POP_TOTAL_Y1, POP_TOTAL_X2, POP_TOTAL_Y2 );

	str  = "Total Other Human Units";

	str  = translate.process(str);
	str += ": ";
	str += viewNation->total_human_count;

	font_san.put( x, y, str );

	str  = "Total Population";

	str  = translate.process(str);
	str += ": ";
	str += viewNation->total_population + viewNation->total_human_count;

	font_san.put( x+360, y, str );

	//-------- display firm total ---------//

	x=FIRM_BROWSE_X1+9;
	y=FIRM_BROWSE_Y2-16;

	vga_back.d3_panel_up(FIRM_BROWSE_X1, FIRM_BROWSE_Y2-18, FIRM_BROWSE_X2, FIRM_BROWSE_Y2 );

	font_san.put( x	 , y, "Total" );
	font_san.put( x+265, y, total_firm_count );
	font_san.put( x+370, y, m.format(total_firm_cost,2) );
	font_san.put( x+470, y, m.format(total_firm_income,2) );
}
//----------- End of static function disp_total -----------//


//-------- Begin of static function calc_firm_total --------//

static void calc_firm_total()
{
	//-------- calculate firm incomes --------//

	total_firm_income = 0;

	memset( firm_income_array, 0, sizeof(firm_income_array) );

	int   thisIncome;
	Firm* firmPtr;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->nation_recno == info.viewing_nation_recno )
		{
			thisIncome = (int) firmPtr->income_365days();

			if( thisIncome > 0 )
			{
				firm_income_array[firmPtr->firm_id-1] += thisIncome;
				total_firm_income += thisIncome;
			}
		}
	}

	//------ calculate total firm cost --------//

	total_firm_count = 0;
	total_firm_cost  = 0;

	FirmInfo* firmInfo;

	for( i=1 ; i<=MAX_FIRM_TYPE ; i++ )
	{
		firmInfo = firm_res[i];

		total_firm_cost += firmInfo->year_cost *
								 firmInfo->nation_firm_count_array[info.viewing_nation_recno-1];

		total_firm_count += firmInfo->nation_firm_count_array[info.viewing_nation_recno-1];
	}
}
//----------- End of static function calc_firm_total -----------//


//-------- Begin of static function town_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int town_filter(int recNo)
{
	int   totalTown = town_array.size();
	int   townCount=0;
	Town* townPtr;

	for( int townRecno=1 ; townRecno<=totalTown ; townRecno++ )
	{
		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( townPtr->nation_recno==info.viewing_nation_recno )
			townCount++;

		if( recNo && townCount==recNo )
			return townRecno;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return townCount;
}
//----------- End of static function town_filter -----------//


//-------- Begin of static function put_town_rec --------//
//
static void put_town_rec(int recNo, int x, int y, int refreshFlag)
{
	int   townRecno = town_filter(recNo);
	Town* townPtr   = town_array[townRecno];

	//---------- display info ----------//

	x+=3;
	y+=3;

	font_san.put( x    , y, townPtr->town_name() );
	font_san.put( x+175, y, townPtr->population );
	font_san.put( x+241, y, townPtr->jobless_population );
	font_san.put( x+309, y, townPtr->average_loyalty() );

	//------- display race icons -------//

	x += 350;

	int i;
	int iconSpacing = RACE_ICON_WIDTH+2;
#if(MAX_RACE > 7)
	int raceCount = 0;
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( townPtr->race_pop_array[i] > 0 )
		{
			++raceCount;
		}
	}
	if( raceCount > 7 )
	{
		iconSpacing = 7 * iconSpacing / raceCount;
	}
#endif
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( townPtr->race_pop_array[i] > 0 )
		{
			vga_back.put_bitmap( x, y-2, race_res[i+1]->icon_bitmap_ptr );
			x += iconSpacing;
		}
	}
}
//----------- End of static function put_town_rec -----------//


//-------- Begin of static function firm_filter --------//
//
// This function has dual purpose :
//
// 1. when <int> recNo is not given :
//    - return the total no. of firms of this nation
//
// 2. when <int> recNo is given :
//    - return the firm recno in firm_array of the given recno.
//
static int firm_filter(int recNo)
{
	int 		 firmTypeCount=0;
	FirmInfo* firmInfo;

	for( int firmId=1 ; firmId<=MAX_FIRM_TYPE ; firmId++ )
	{
		firmInfo = firm_res[firmId];

		if( firmInfo->nation_firm_count_array[info.viewing_nation_recno-1] )
			firmTypeCount++;

		if( recNo && firmTypeCount==recNo )
			return firmId;
	}

	err_when( recNo );   // the recNo is not found, it is out of range

	return firmTypeCount;
}
//----------- End of static function firm_filter -----------//


//----------- Begin of static function put_firm_rec -----------//
//
static void put_firm_rec(int recNo, int x, int y, int refreshFlag)
{
	int   	 firmId = firm_filter(recNo);
	FirmInfo* firmInfo = firm_res[firmId];

	x+=3;
	y+=3;

	int firmCount = firmInfo->nation_firm_count_array[info.viewing_nation_recno-1];

	font_san.put( x    , y, firmInfo->name );
	font_san.put( x+155, y, m.format(firmInfo->year_cost,2) );
	font_san.put( x+265, y, firmCount );
	font_san.put( x+370, y, m.format(firmInfo->year_cost*firmCount,2) );
	font_san.put( x+470, y, m.format(firm_income_array[firmId-1], 2) );
}
//----------- End of static function put_firm_rec -----------//

