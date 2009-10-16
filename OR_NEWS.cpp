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

//Filename    : OR_NEWS.CPP
//Description : Report for displaying AI actions

#include <OVGA.h>
#include <ODATE.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <OTALKRES.h>
#include <ONEWS.h>
#include <OINFO.h>

//------------- Define coordinations -----------//

enum { NEWS_BROWSE_X1 = ZOOM_X1+6,
		 NEWS_BROWSE_Y1 = ZOOM_Y1+6,
		 NEWS_BROWSE_X2 = ZOOM_X2-6,
		 NEWS_BROWSE_Y2 = ZOOM_Y2-25,
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_news;

//----------- Define static functions ----------//

static void put_news_rec(int recNo, int x, int y, int refreshFlag);

//--------- Begin of function Info::disp_news_log ---------//
//
void Info::disp_news_log(int refreshFlag)
{
	int x=NEWS_BROWSE_X1+9;
	int y=NEWS_BROWSE_Y1+4;

	if( refreshFlag == INFO_REPAINT )
	{
		browse_news.init( NEWS_BROWSE_X1, NEWS_BROWSE_Y1, NEWS_BROWSE_X2, NEWS_BROWSE_Y2,
								0, 32, news_array.size(), put_news_rec, 1 );

		browse_news.open(browse_news_recno);
	}
	else
	{
		browse_news.paint();
		browse_news.open(browse_news_recno, news_array.size());
	}

	//------- display button ---------//

	image_icon.put_back(ZOOM_X2-20, ZOOM_Y2-18, "NEWS_LOG");	// news log report
}
//----------- End of function Info::disp_news_log -----------//


//--------- Begin of function Info::detect_news_log ---------//
//
void Info::detect_news_log()
{
	if( browse_news.detect() )
		browse_news_recno = browse_news.recno();

	//--------- detect button --------//

	if( mouse.single_click( ZOOM_X2-20, ZOOM_Y2-18, ZOOM_X2-11, ZOOM_Y2-9 ) )
		sys.set_view_mode(MODE_NORMAL);
}
//----------- End of function Info::detect_news_log -----------//


//-------- Begin of static function put_news_rec --------//
//
static void put_news_rec(int recNo, int x, int y, int refreshFlag)
{
	News* newsPtr = news_array[ news_array.size()-recNo+1 ]; 	// display in reversed order

	font_san.put( x+20, y, date.date_str(newsPtr->news_date, 1) );

   talk_res.msg_add_nation_color = 1; 
	font_san.put_paragraph( x+110, y, browse_news.ix2-30, y+30, newsPtr->msg() );
	talk_res.msg_add_nation_color = 0;

	//----- display and detect the go icon -----//

	if( newsPtr->is_loc_valid() )
	{
		image_icon.put_back( x+2, y+1, "NEWS_GO" );

		if( mouse.single_click(x+2, y+1, x+12, y+11) )
		{
			vga.use_front();
			world.go_loc( newsPtr->loc_x, newsPtr->loc_y, 1 );		// 1-select object on the location if there is any
			vga.use_back();
		}
	}
}
//----------- End of static function put_news_rec -----------//


