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

//Filename    : ONEWS.CPP
//Description : Object News

#include <OVGA.h>
#include <OINFO.h>
#include <OHELP.h>
#include <OGAME.h>
#include <OCONFIG.h>
#include <OSYS.h>
#include <OMOUSE.h>
#include <OFONT.h>
#include <ODATE.h>
#include <OIMGRES.h>
#include <ONATION.h>
#include <OTALKRES.h>
#include <ONEWS.h>

//------- define screen coordinations -------//

enum { NEWS_HEIGHT = 20,
		 NEWS_X1 = ZOOM_X1+8,
		 NEWS_X2 = ZOOM_X2-16,
		 NEWS_Y2 = ZOOM_Y2-5,
	  };

//--------- define static vars --------------//

static Font* news_font_ptr = &font_news;

//------ Begin of function NewsArray::NewsArray -----//

NewsArray::NewsArray() : DynArray(sizeof(News), 100)
{
}
//-------- End of function NewsArray::NewsArray -----//


//------ Begin of function NewsArray::init -----//
//
void NewsArray::init()
{
	reset();

	set_font(&font_san);		// use black font
}
//------- End of function NewsArray::init -----//


//------ Begin of function NewsArray::deinit -----//
//
void NewsArray::deinit()
{
	reset();
}
//------- End of function NewsArray::deinit -----//


//------ Begin of function NewsArray::reset -----//
//
// Reset all news display options and clear all news in the log
//
void NewsArray::reset()
{
	zap();               // clear all news in the log

	last_clear_recno = 0;
	news_add_flag    = 1;

	default_setting();
}
//------- End of function NewsArray::reset -----//


//------ Begin of function NewsArray::default_setting -----//
//
// Reset all news display options
//
void NewsArray::default_setting()
{
	news_type_option[NEWS_NORMAL]=1;

	news_who_option = NEWS_DISP_PLAYER;  // default display news of groups controlled by the player
}
//------- End of function NewsArray::default_setting -----//


//------ Begin of function NewsArray::set_font -----//
//
void NewsArray::set_font(Font* fontPtr)
{
	news_font_ptr = fontPtr;
}
//------- End of function NewsArray::set_font -----//


//------ Begin of function NewsArray::disp -----//
//
void NewsArray::disp()
{
	//-- only display news in normal view and diplomatic/chat option screen ---//

	int rc=0;

	switch( sys.view_mode )
	{
		case MODE_NORMAL:
			rc = 1;
			break;

		case MODE_NATION:
			rc = info.nation_report_mode==NATION_REPORT_TALK ||
				  info.nation_report_mode==NATION_REPORT_CHAT;
			break;
	}

	if( rc )
	{
		put(0);     // 0-not a detect action
	}
}
//------- End of function NewsArray::disp -----//


//------ Begin of function NewsArray::detect -----//
//
int NewsArray::detect()
{
	//-- only display news in normal view and diplomatic option screen ---//

	if( sys.view_mode == MODE_NORMAL ||
		 (sys.view_mode == MODE_NATION &&
		 (info.nation_report_mode==NATION_REPORT_TALK ||
		  info.nation_report_mode==NATION_REPORT_CHAT) ) )
	{
		return put(1);     // 1-is a detect action
	}

	return 0;
}
//------- End of function NewsArray::detect -----//


//------ Begin of function NewsArray::put -----//
//
// Display or detect news messages.
//
// <int> detectAction - 1 if this is a detect action.
//
int NewsArray::put(int detectAction)
{
	int 		i, y;
	News*	newsPtr;

	vga.use_back();

	//--- count the no. of recent news which should be displayed ---//

	int dispCount=0;

	for( i=size() ; i>=MAX(1,last_clear_recno+1) ; i-- )
	{
		newsPtr = (News*) get(i);

		//----- if display major news only ------//

		if( config.disp_news_flag == OPTION_DISPLAY_MAJOR_NEWS ||
			 game.game_mode == GAME_TUTORIAL )                    		// only display major news in the tutorial
		{
			if( !newsPtr->is_major() )
				continue;
		}

		//---------------------------------------//

		if( info.game_date < newsPtr->news_date + DISP_NEWS_DAYS )
		{
			if( ++dispCount >= DISP_NEWS_COUNT )
				break;
		}
		else
			break;
	}

	//----- display news messages from the bottom up ------//

	y = NEWS_Y2 - NEWS_HEIGHT;

	int newsRecno, newsHeight;

	for( i=dispCount ; i>0 ; i-- )
	{
		newsRecno = size()-i+1;

		newsPtr = (News*) get(newsRecno);

		//----- if display major news only ------//

		if( config.disp_news_flag == OPTION_DISPLAY_MAJOR_NEWS ||
			 game.game_mode == GAME_TUTORIAL )								// only display major news in the tutorial
		{
			if( !newsPtr->is_major() )
				continue;
		}

		//---- if it's a diplomatic message, check if it's still valid ----//

		if( newsPtr->id == NEWS_DIPLOMACY )
		{
			err_when( talk_res.is_talk_msg_deleted(newsPtr->short_para1) ); // it shouldn't have been deleted as when it's deleted in TalkRes(), it will be removed from news_array immediately 

			TalkMsg* talkMsgPtr = talk_res.get_talk_msg(newsPtr->short_para1); 

			//--- if it is no longer valid, delete it ---//

			if( talkMsgPtr->reply_type==REPLY_WAITING && !talkMsgPtr->is_valid_to_reply() )
			{
				news_array.linkout(newsRecno);
				continue;
			}
		}

		//----- display the new message now -----//

		if( newsPtr->put( y, detectAction, newsHeight ) )		// return 1 if reply diplomatic message
			return 1;

		y -= newsHeight;
	}

	//------- display buttons -------//

	if( !detectAction )
	{
		image_icon.put_back(NEWS_X2+3, NEWS_Y2-12, "NEWS_LOG");	// news log report

		if( dispCount > 0 )			// clear news button
			image_icon.put_back(NEWS_X2+3, NEWS_Y2-26, "NEWS_CLR");
	}

	//------- detect buttons -------//

	else
	{
		//--- detect news log report ---//

		help.set_help( NEWS_X2+3, NEWS_Y2-12, NEWS_X2+12, NEWS_Y2-3, "NEWSLOG" );

		if( mouse.single_click( NEWS_X2+3, NEWS_Y2-12, NEWS_X2+12, NEWS_Y2-3 ) )
			sys.set_view_mode(MODE_NEWS_LOG);

		if( dispCount > 0 )			// clear news button
		{
			help.set_help( NEWS_X2+3, NEWS_Y2-26, NEWS_X2+12, NEWS_Y2-17, "CLRNEWS" );

			if( mouse.single_click( NEWS_X2+3, NEWS_Y2-26, NEWS_X2+12, NEWS_Y2-17 ) )
				clear_news_disp();
		}
	}

	//-------------------------------//

	vga.use_front();

	return 0;
}
//------- End of function NewsArray::put -----//


//------ Begin of function NewsArray::clear_news_disp -----//
//
// Clear all news on the screen.
//
void NewsArray::clear_news_disp()
{
	last_clear_recno = size();
}
//------- End of function NewsArray::clear_news_disp -----//


//------ Begin of function News::put -----//
//
// <int> y 				 - the y display location of the news message
// <int> detectAction - whether it's a detect action
// <int&> newsHeight	 - for returning the height of the news message displayed
//
// return: <int> 1 - if detect pressing on the news
//					  0 - not detected or it's a display action
//
int News::put(int y, int detectAction, int& newsHeight)
{
	//------- get the height of the message -------//

	#define NEWS_LINE_SPACE				2
	#define NEWS_SECOND_LINE_HEIGHT 16
	#define NEWS_ICON_WIDTH			  17

	String msgStr(msg());
	char*  dateStr = date.date_str(news_date,1);

	int dateWidth = news_font_ptr->text_width(dateStr) + 5;		// 5 is the space between the date and the text

	newsHeight = NEWS_HEIGHT;

	if( NEWS_ICON_WIDTH + dateWidth + news_font_ptr->text_width(msgStr) > NEWS_X2-NEWS_X1+1 )
	{
		y 			  -= NEWS_SECOND_LINE_HEIGHT;		// two line messages.
		newsHeight += NEWS_SECOND_LINE_HEIGHT;
	}

	//---- brighten the area for display in non-standard terrain set ---//

	vga.active_buf->adjust_brightness(ZOOM_X1, y-3, NEWS_X2+1, y+newsHeight-4, 6);

	//------ if it's a detect action --------//

	if( detectAction )
	{
		help.set_help( NEWS_X1, y, NEWS_X1+10, y+10, "REPLYDIP" );

		if( mouse.single_click( NEWS_X1, y, NEWS_X1+10, y+10 ) )
		{
			if( id == NEWS_DIPLOMACY )
			{
				err_when( talk_res.is_talk_msg_deleted(short_para1) );	// it shouldn't have been deleted as when it's deleted in TalkRes(), it will be removed from news_array immediately 

				TalkMsg* talkMsgPtr = talk_res.get_talk_msg(short_para1);

				if( talkMsgPtr->reply_type == REPLY_WAITING ) 	// currently waiting a reply from the player
				{
					vga.use_front();

					talk_res.player_reply(short_para1);
					return 1;
				}
			}
			else if( id == NEWS_CHAT_MSG )
			{
				int fromNationRecno = info.remote_chat_array[short_para1-1].from_nation_recno;

				if( !nation_array.is_deleted(fromNationRecno) )
				{
					vga.use_front();
					info.player_reply_chat(fromNationRecno);
					return 1;
				}
			}
			else if( is_loc_valid() )
			{
				vga.use_front();
				world.go_loc( loc_x, loc_y, 1 );		// 1-select object on the location if there is any
				return 1;
			}
		}

		return 0;
	}

	//---- display the color of the sender nation ----//

	if( id==NEWS_DIPLOMACY )
	{
		err_when( talk_res.is_talk_msg_deleted(short_para1) );	// it shouldn't have been deleted as when it's deleted in TalkRes(), it will be removed from news_array immediately

		TalkMsg* talkMsgPtr = talk_res.get_talk_msg(short_para1);
		int		nationRecno;

		if( talkMsgPtr->reply_type == REPLY_WAITING ||
			 talkMsgPtr->reply_type == REPLY_NOT_NEEDED )
		{
			nationRecno = talkMsgPtr->from_nation_recno;
		}
		else
			nationRecno = talkMsgPtr->to_nation_recno;

		nation_array[nationRecno]->disp_nation_color(NEWS_X1, y+2);
	}
	else
	{
		if( is_loc_valid() )
			image_icon.put_back( NEWS_X1, y+2, "NEWS_GO" );

		if( id==NEWS_CHAT_MSG )
		{
			char colorCode = game.color_remap_array[nation_color1].main_color;
			nation_array.disp_nation_color(NEWS_X1, y+2, colorCode );
		}
	}

	//------- display the message content -------//

	news_font_ptr->put( NEWS_X1+NEWS_ICON_WIDTH, y, dateStr );

	news_font_ptr->put_paragraph( NEWS_X1+NEWS_ICON_WIDTH+dateWidth, y, NEWS_X2,
		y+newsHeight, msgStr, NEWS_LINE_SPACE );

	return 0;
}
//------- End of function News::put -----//


//------ Begin of function NewsArray::remove -----//
//
// Remove a specific news from the news_array.
//
// <int> newsId     = id. of the news
// <int> shortPara1 = short_para1 of the news
//
void NewsArray::remove(int newsId, int shortPara1)
{
	News* newsPtr;

	for( int i=size() ; i>0 ; i-- )
	{
		newsPtr = news_array[i];

		if( newsPtr->id == newsId &&
			 newsPtr->short_para1 == shortPara1 )
		{
			news_array.linkout(i);

			//### begin alex 31/3 ###//
			//if( i<last_clear_recno && last_clear_recno > 1 )
			if( i<=last_clear_recno && last_clear_recno > 0 )
				last_clear_recno--;
			//#### end alex 31/3 ####//

			break;
		}
	}
}
//------- End of function NewsArray::remove -----//


//------- Begin of function NewsArray::operator[] -----//

News* NewsArray::operator[](int recNo)
{
	#ifdef DEBUG

	News* msgPtr;

	if( recNo )
		msgPtr = (News*) get(recNo);
	else
		msgPtr = NULL;   // NULL will then cause error

	err_if( !msgPtr || msgPtr->news_date==0 )
		err_now( "NewsArray[] is deleted" );

	return msgPtr;

	#else

	return (News*) get(recNo);

	#endif
}
//--------- End of function NewsArray::operator[] ----//

