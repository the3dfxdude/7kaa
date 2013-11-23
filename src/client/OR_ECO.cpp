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

//Filename    : OINFOECO.CPP
//Description : Economy information screen

#include <OVGA.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OIMGRES.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <OFIRM.h>
#include <OFIRMRES.h>
#include <ORACERES.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OINFO.h>
#include "gettext.h"

//------------- Define coordinations -----------//

enum { INCOME_BROWSE_X1 = ZOOM_X1+6,
		 INCOME_BROWSE_Y1 = ZOOM_Y1+6,
		 INCOME_BROWSE_X2 = ZOOM_X2-6,
		 INCOME_BROWSE_Y2 = INCOME_BROWSE_Y1+186
	  };

enum { EXPENSE_BROWSE_X1 = ZOOM_X1+6,
		 EXPENSE_BROWSE_Y1 = INCOME_BROWSE_Y2+6,
		 EXPENSE_BROWSE_X2 = ZOOM_X2-6,
		 EXPENSE_BROWSE_Y2 = ZOOM_Y2-30,
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_income, browse_expense;

//----------- Define static functions ----------//

static void put_income_rec(int recNo, int x, int y, int refreshFlag);
static void put_expense_rec(int recNo, int x, int y, int refreshFlag);

static void disp_total();

//--------- Begin of function Info::disp_economy ---------//
//
void Info::disp_economy(int refreshFlag)
{
	//------- display the income report -------//

	int x=INCOME_BROWSE_X1+9;
	int y=INCOME_BROWSE_Y1+4;

	vga_back.d3_panel_up(INCOME_BROWSE_X1, INCOME_BROWSE_Y1, INCOME_BROWSE_X2, INCOME_BROWSE_Y1+20 );

	font_san.put( x	 , y, _("Income Item") );
	font_san.put( x+350, y, _("Yearly Income") );

	int incomeCount;		// only display the cheat income if it amount is > 0

	if( nation_array[info.viewing_nation_recno]->income_365days(INCOME_CHEAT) > 0 &&
		 (sys.testing_session || info.viewing_nation_recno == nation_array.player_recno) )		// only display cheat amount in debug mode or cheat amount of the player's kingdom, do not display cheat amount on AI kingdoms
	{
		incomeCount = INCOME_TYPE_COUNT;
	}
	else
		incomeCount = INCOME_TYPE_COUNT-1;

	if( refreshFlag == INFO_REPAINT )
	{
		browse_income.init( INCOME_BROWSE_X1, INCOME_BROWSE_Y1+22, INCOME_BROWSE_X2, INCOME_BROWSE_Y2-20,
								0, 16, incomeCount, put_income_rec, 1 );

		browse_income.open(browse_income_recno);		// if refreshFlag is INFO_UPDATE, keep the original top_rec_no of the browser
	}
	else
	{
		browse_income.paint();
		browse_income.open(browse_income_recno, incomeCount);
	}

	//------- display the expense report -------//

	x=EXPENSE_BROWSE_X1+9;
	y=EXPENSE_BROWSE_Y1+4;

	vga_back.d3_panel_up(EXPENSE_BROWSE_X1, EXPENSE_BROWSE_Y1, EXPENSE_BROWSE_X2, EXPENSE_BROWSE_Y1+20 );

	font_san.put( x	 , y, _("Expense Item") );
	font_san.put( x+350, y, _("Yearly Expense") );

	if( refreshFlag == INFO_REPAINT )
	{
		browse_expense.init( EXPENSE_BROWSE_X1, EXPENSE_BROWSE_Y1+22, EXPENSE_BROWSE_X2, EXPENSE_BROWSE_Y2-20,
							0, 16, EXPENSE_TYPE_COUNT, put_expense_rec, 1 );

		browse_expense.open(browse_expense_recno);		// if refreshFlag is INFO_UPDATE, keep the original top_rec_no of the browser
	}
	else
	{
		browse_expense.paint();
		browse_expense.open(browse_expense_recno, EXPENSE_TYPE_COUNT);		// if refreshFlag is INFO_UPDATE, keep the original top_rec_no of the browser
	}

	//--------- display total ----------//

	disp_total();
}
//----------- End of function Info::disp_economy -----------//


//--------- Begin of function Info::detect_economy ---------//
//
void Info::detect_economy()
{
	if( browse_income.detect() )
		browse_income_recno = browse_income.recno();

	if( browse_expense.detect() )
		browse_expense_recno = browse_expense.recno();
}
//----------- End of function Info::detect_economy -----------//


//--------- Begin of static function disp_total ---------//
//
static void disp_total()
{
	//--- calculate the total income and expense ----//

	float totalIncome  = (float) 0;
	float totalExpense = (float) 0;

	Nation* nationPtr = nation_array[info.viewing_nation_recno];

	int i;
	for( i=0 ; i<INCOME_TYPE_COUNT ; i++ )
		totalIncome  += nationPtr->income_365days(i);

	for( i=0 ; i<EXPENSE_TYPE_COUNT ; i++ )
		totalExpense += nationPtr->expense_365days(i);

	//---------- display total income ----------//

	vga_back.d3_panel_up(INCOME_BROWSE_X1, INCOME_BROWSE_Y2-18, INCOME_BROWSE_X2, INCOME_BROWSE_Y2 );

	int x=INCOME_BROWSE_X1+9;
	int y=INCOME_BROWSE_Y2-16;

	font_san.put( x, y, _("Total Yearly Income") );
	font_san.put( x+370, y, misc.format( (int) totalIncome, 2 ) );

	//---------- display total expense ----------//

	vga_back.d3_panel_up(EXPENSE_BROWSE_X1, EXPENSE_BROWSE_Y2-18, EXPENSE_BROWSE_X2, EXPENSE_BROWSE_Y2 );

	x=EXPENSE_BROWSE_X1+9;
	y=EXPENSE_BROWSE_Y2-16;

	font_san.put( x, y, _("Total Yearly Expenses") );
	font_san.put( x+370, y, misc.format( (int) totalExpense, 2 ) );

	//----------- display the balance --------//

	y=EXPENSE_BROWSE_Y2+7;

	vga_back.d3_panel_up(EXPENSE_BROWSE_X1, EXPENSE_BROWSE_Y2+4, EXPENSE_BROWSE_X2, ZOOM_Y2-6 );

	font_san.put( x, y, _("Yearly Balance") );
	font_san.put( x+370, y, misc.format( (int)(totalIncome-totalExpense), 2 ) );
}
//----------- End of static function disp_total -----------//


//-------- Begin of static function put_income_rec --------//
//
static void put_income_rec(int recNo, int x, int y, int refreshFlag)
{
	//----- define income descriptions ------//

	static const char* income_des_array[INCOME_TYPE_COUNT] =
	{
		N_("Sale of Goods"),
		N_("Exports"),
		N_("Taxes"),
		N_("Recovered Treasure"),
		N_("Worker Income"),
		N_("Sale of Buildings"),
		N_("Aid/Tribute from Other Kingdoms"),
		N_("Cheating"),
	};

	//---------------------------------//

	x+=3;
	y+=3;

	Nation* nationPtr = nation_array[info.viewing_nation_recno];

	font_san.put( x    , y, _(income_des_array[recNo-1]) );
	font_san.put( x+370, y, misc.format( (int) nationPtr->income_365days(recNo-1), 2 ) );
}
//----------- End of static function put_income_rec -----------//


//-------- Begin of static function put_expense_rec --------//
//
static void put_expense_rec(int recNo, int x, int y, int refreshFlag)
{
	//----- define expense descriptions -------//

	static const char* expense_des_array[EXPENSE_TYPE_COUNT] =
	{
		N_("General Costs"),
		N_("Spy Costs"),
		N_("Other Mobile Human Unit Costs"),
		N_("Caravan Costs"),
		N_("Weapons Costs"),
		N_("Ship Costs"),
		N_("Buildings Costs"),
		N_("Training Units"),
		N_("Hiring Units"),
		N_("Honoring Units"),
		N_("Foreign Worker Salaries"),
		N_("Grants to Your Villagers"),
		N_("Grants to Other Villagers"),
		N_("Imports"),
		N_("Aid/Tribute to Other Kingdoms"),
		N_("Bribes"),
	};

	//---------------------------------//

	x+=3;
	y+=3;

	Nation* nationPtr = nation_array[info.viewing_nation_recno];

	font_san.put( x    , y, _(expense_des_array[recNo-1]) );
	font_san.put( x+370, y, misc.format( (int) nationPtr->expense_365days(recNo-1), 2 ) );
}
//----------- End of static function put_expense_rec -----------//


