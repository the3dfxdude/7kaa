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

//Filename    : OTALKENG.CPP
//Description : English version of the talk messages

#if( !defined(GERMAN) && !defined(FRENCH) && !defined(SPANISH) )

#include <OMOUSE.h>
#include <OSYS.h>
#include <OVGA.h>
#include <OINFO.h>
#include <OFONT.h>
#include <OTECHRES.h>
#include <OGAMESET.h>
#include <ONEWS.h>
#include <ONATION.h>
#include <OTALKRES.h>
#include "gettext.h"

//-------- define static vars --------//

static String str;						// these vars don't have to be saved as their are only used by msg_str() for passing vars to other functions
static short  viewing_nation_recno;
static char   should_disp_reply;
static char   disp_second_line;

//-------- macros --------//
// used in printf commands in pairs for "%'s Kingdom%s"
#define NATION_PARA1 para1_nation_name()
#define COLOR_PARA1 nation_color_code_str(talk_para1)

#define FROM_NATION from_nation_name()
#define FROM_COLOR nation_color_code_str2(from_nation_recno)

#define TO_NATION to_nation_name()
#define TO_COLOR nation_color_code_str2(to_nation_recno)

//------- Begin of function TalkMsg::msg_str --------//
//
// Return the text of this message.
//
// <int> viewingNationRecno = the recno of the nation viewing this message
// [int] dispReply 			 = whether display the reply if there is one
//										(default: 1)
// [int] dispSecondLine		 = whether should display the second line of the
//										message (default: 0)
//
char* TalkMsg::msg_str(int viewingNationRecno, int dispReply, int dispSecondLine)
{
	viewing_nation_recno = viewingNationRecno;
	should_disp_reply		= dispReply;
	disp_second_line		= dispSecondLine;

	//-------- compose the message str -------//

	switch(talk_id)
	{
		case TALK_PROPOSE_TRADE_TREATY:
			propose_treaty(TALK_PROPOSE_TRADE_TREATY);
			break;

		case TALK_PROPOSE_FRIENDLY_TREATY:
			propose_treaty(TALK_PROPOSE_FRIENDLY_TREATY);
			break;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			propose_treaty(TALK_PROPOSE_ALLIANCE_TREATY);
			break;

		case TALK_END_TRADE_TREATY:
			end_treaty(TALK_END_TRADE_TREATY);
			break;

		case TALK_END_FRIENDLY_TREATY:
			end_treaty(TALK_END_FRIENDLY_TREATY);
			break;

		case TALK_END_ALLIANCE_TREATY:
			end_treaty(TALK_END_FRIENDLY_TREATY);
			break;

		case TALK_REQUEST_MILITARY_AID:
			request_military_aid();
			break;

		case TALK_REQUEST_TRADE_EMBARGO:
			request_trade_embargo();
			break;

		case TALK_REQUEST_CEASE_WAR:
			request_cease_war();
			break;

		case TALK_REQUEST_DECLARE_WAR:
			request_declare_war();
			break;

		case TALK_REQUEST_BUY_FOOD:
			request_buy_food();
			break;

		case TALK_DECLARE_WAR:
			declare_war();
			break;

		case TALK_GIVE_TRIBUTE:
			give_tribute("tribute");
			break;

		case TALK_DEMAND_TRIBUTE:
			demand_tribute(0);		// 1-is tribute, not aid
			break;

		case TALK_GIVE_AID:
			give_tribute("aid");
			break;

		case TALK_DEMAND_AID:
			demand_tribute(1);		// 1-is aid, not tribute
			break;

		case TALK_GIVE_TECH:
			give_tech();
			break;

		case TALK_DEMAND_TECH:
			demand_tech();
			break;

		case TALK_REQUEST_SURRENDER:
			request_surrender();
			break;

		case TALK_SURRENDER:
			surrender();
			break;

		default:
			err_here();
	}

	return str;
}
//-------- End of function TalkMsg::msg_str ---------//


//----- Begin of function TalkMsg::propose_treaty ------//
//
// talk_para1 - duration of the treaty (no. of years).
//
void TalkMsg::propose_treaty(short treatyType)
{
	//---------------------------------------------//
	//
	// Send:
	//
	// <King>'s Kingdom proposes a/an friendly/alliance treaty to you.
	// You propose a/an friendly/alliance treaty to <King>'s Kingdom.
	//
	// Reply:
	//
	// <King>'s Kingdom accepts/rejects your proposed
	// friendly/alliance treaty.
	//
	// You accept/reject the friendly/alliance treaty
	// proposed by <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( treatyType == TALK_PROPOSE_TRADE_TREATY )
			{
				// TRANSLATORS: You propose a trade treaty to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You propose a trade treaty to %s's Kingdom%s."), TO_NATION, TO_COLOR);
			}
			else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
			{
				// TRANSLATORS: You propose a friendly treaty to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You propose a friendly treaty to %s's Kingdom%s."), TO_NATION, TO_COLOR);
			}
			else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
			{
				// TRANSLATORS: You propose an alliance treaty to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You propose an alliance treaty to %s's Kingdom%s."), TO_NATION, TO_COLOR);
			}
		}
		else
		{
			if( treatyType == TALK_PROPOSE_TRADE_TREATY )
			{
				// TRANSLATORS: <King>'s Kingdom<Color> proposes a trade treaty to you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s proposes a trade treaty to you."), FROM_NATION, FROM_COLOR);
			}
			else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
			{
				// TRANSLATORS: <King>'s Kingdom<Color> proposes a friendly treaty to you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s proposes a friendly treaty to you."), FROM_NATION, FROM_COLOR);
			}
			else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
			{
				// TRANSLATORS: <King>'s Kingdom<Color> proposes an alliance treaty to you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s proposes an alliance treaty to you."), FROM_NATION, FROM_COLOR);
			}
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> accepts your proposed trade treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s accepts your proposed trade treaty."), TO_NATION, TO_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> accepts your proposed friendly treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s accepts your proposed friendly treaty."), TO_NATION, TO_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> accepts your proposed alliance treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s accepts your proposed alliance treaty."), TO_NATION, TO_COLOR);
				}
			}
			else
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> rejects your proposed trade treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s rejects your proposed trade treaty."), TO_NATION, TO_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> rejects your proposed friendly treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s rejects your proposed friendly treaty."), TO_NATION, TO_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> rejects your proposed alliance treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s rejects your proposed alliance treaty."), TO_NATION, TO_COLOR);
				}
			}
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: You accept the trade treaty proposed by <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the trade treaty proposed by %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: You accept the friendly treaty proposed by <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the friendly treaty proposed by %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: You accept the alliance treaty proposed by <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the alliance treaty proposed by %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
				}
			}
			else
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: You reject the trade treaty proposed by <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the trade treaty proposed by %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: You reject the friendly treaty proposed by <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the friendly treaty proposed by %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: You reject the alliance treaty proposed by <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the alliance treaty proposed by %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
				}
			}
		}
	}
}
//------- End of function TalkMsg::propose_treaty ------//


//----- Begin of function TalkMsg::end_treaty ------//
//
// talk_para1 - treaty type, NATION_FRIENDLY or NATION_ALLIANCE.
//
void TalkMsg::end_treaty(short treatyType)
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom terminates its friendly/alliance treaty with you.
	// You terminate your friendly/alliance treaty with <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		if( treatyType == TALK_END_TRADE_TREATY )
		{
			// TRANSLATORS: You terminate your trade treaty with <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You terminate your trade treaty with %s's Kingdom%s."), TO_NATION, TO_COLOR);
		}
		else if( treatyType == TALK_END_FRIENDLY_TREATY )
		{
			// TRANSLATORS: You terminate your friendly treaty with <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You terminate your friendly treaty with %s's Kingdom%s."), TO_NATION, TO_COLOR);
		}
		else if( treatyType == TALK_END_ALLIANCE_TREATY )
		{
			// TRANSLATORS: You terminate your alliance treaty with <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You terminate your alliance treaty with %s's Kingdom%s."), TO_NATION, TO_COLOR);
		}
	}
	else
	{
		if( treatyType == TALK_END_TRADE_TREATY )
		{
			// TRANSLATORS: <King>'s Kingdom<Color> terminates its trade treaty with you.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s terminates its trade treaty with you."), FROM_NATION, FROM_COLOR);
		}
		else if( treatyType == TALK_END_FRIENDLY_TREATY )
		{
			// TRANSLATORS: <King>'s Kingdom<Color> terminates its friendly treaty with you.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s terminates its friendly treaty with you."), FROM_NATION, FROM_COLOR);
		}
		else if( treatyType == TALK_END_ALLIANCE_TREATY )
		{
			// TRANSLATORS: <King>'s Kingdom<Color> terminates its alliance treaty with you.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s terminates its alliance treaty with you."), FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::end_treaty ------//


//----- Begin of function TalkMsg::request_cease_war ------//
//
void TalkMsg::request_cease_war()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom requests a cease-fire.
	// You request a cease-fire with <King>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees to a cease-fire.
	// <King>'s Kingdom refuses a cease-fire.
	// You agree to a cease-fire with <King>'s Kingdom.
	// You refuse a cease-fire with <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You request a cease-fire with <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You request a cease-fire with %s's Kingdom%s."), TO_NATION, TO_COLOR);
		}
		else
		{
			// TRANSLATORS: <King>'s Kingdom<Color> requests a cease-fire.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s requests a cease-fire."), FROM_NATION, FROM_COLOR);
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King>'s Kingdom<Color> agrees to a cease-fire.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s agrees to a cease-fire."), TO_NATION, TO_COLOR);
			else
				// TRANSLATORS: <King>'s Kingdom<Color> refuses a cease-fire.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s refuses a cease-fire."), TO_NATION, TO_COLOR);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to a cease-fire with <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to a cease-fire with %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
			else
				// TRANSLATORS: You refuse a cease-fire with <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse a cease-fire with %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::request_cease_war ------//


//----- Begin of function TalkMsg::request_declare_war ------//
//
// talk_para1 - the recno of the nation to declare war with.
//
void TalkMsg::request_declare_war()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom requests that you declare war on <King B>'s Kingdom.
	// You request <King>'s Kingdom to declare war on <King B>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees/refuses to declare war on <King B>'s Kingdom.
	// You agree/refuse to declare war on <King B>'s Kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You request <King 1>'s Kingdom<Color> to declare war on <King 2>'s Kingdom<Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("You request %s's Kingdom%s to declare war on %s's Kingdom%s."), TO_NATION, TO_COLOR, NATION_PARA1, COLOR_PARA1);
		}
		else
		{
			// TRANSLATORS: <King 1>'s Kingdom<Color> requests that you declare war on <King 2>'s Kingdom<Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s requests that you declare war on %s's Kingdom%s."), FROM_NATION, FROM_COLOR, NATION_PARA1, COLOR_PARA1);
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King 1>'s Kingdom<Color> agrees to declare war on <King 2>'s Kingdom<Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s agrees to declare war on %s's Kingdom%s."), TO_NATION, TO_COLOR, NATION_PARA1, COLOR_PARA1);
			else
				// TRANSLATORS: <King 1>'s Kingdom<Color> refuses to declare war on <King 2>'s Kingdom<Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s refuses to declare war on %s's Kingdom%s."), TO_NATION, TO_COLOR, NATION_PARA1, COLOR_PARA1);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to declare war on <King>'s Kingdom<Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to declare war on %s's Kingdom%s."), NATION_PARA1, COLOR_PARA1);
			else
				// TRANSLATORS: You refuse to declare war on <King>'s Kingdom<Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to declare war on %s's Kingdom%s."), NATION_PARA1, COLOR_PARA1);
		}
	}
}
//------- End of function TalkMsg::request_declare_war ------//


//----- Begin of function TalkMsg::request_buy_food ------//
//
// talk_para1 - the qty of food the nation wants to buy.
// talk_para2 - price offered for 10 qty of food.
//
void TalkMsg::request_buy_food()
{
	//---- display the second line in the reply question ----//
	//
	// <King>'s Kingdom offers $10 for 10 units of food.
	//
	//-------------------------------------------------------//

	if( disp_second_line )
	{
		// TRANSLATORS: <King>'s Kingdom<Color> offers <Amount> for 10 units of food.
		snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s offers %s for 10 units of food."), FROM_NATION, FROM_COLOR, misc.format(talk_para2,2));

		return;
	}

	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom requests to purchase <amount>
	// units of food from you.
	//
	// You request to purchase <amount> units of food
	// from <King>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees/refuses to sell <amount> units
	// of food to you.
	//
	// You agree/refuse to sell <amount> units of food to
	// <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You request to purchase <Number> units of food from <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You request to purchase %s units of food from %s's Kingdom%s."), misc.format(talk_para1), TO_NATION, TO_COLOR);
		}
		else
		{
			// TRANSLATORS: <King>'s Kingdom<Color> requests to purchase <Number> units of food from you.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s requests to purchase %s units of food from you."), FROM_NATION, FROM_COLOR, misc.format(talk_para1));
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King>'s Kingdom<Color> agrees to sell <Number> units of food to you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s agrees to sell %s units of food to you."), TO_NATION, TO_COLOR, misc.format(talk_para1));
			else
				// TRANSLATORS: <King>'s Kingdom<Color> refuses to sell <Number> units of food to you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s refuses to sell %s units of food to you."), TO_NATION, TO_COLOR, misc.format(talk_para1));
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to sell <Number> units of food to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to sell %s units of food to %s's Kingdom%s."), misc.format(talk_para1), FROM_NATION, FROM_COLOR);
			else
				// TRANSLATORS: You refuse to sell <Number> units of food to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to sell %s units of food to %s's Kingdom%s."), misc.format(talk_para1), FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::request_buy_food ------//


//----- Begin of function TalkMsg::declare_war ------//
//
void TalkMsg::declare_war()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom declares war on you.
	// You declare war on <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		// TRANSLATORS: You declare war on <King>'s Kingdom<Color>.
		snprintf(str, MAX_STR_LEN+1, _("You declare war on %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
	}
	else
	{
		// TRANSLATORS: <King>'s Kingdom<Color> declares war on you.
		snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s declares war on you."), FROM_NATION, FROM_COLOR);
	}
}
//------- End of function TalkMsg::declare_war ------//


//----- Begin of function TalkMsg::give_tribute ------//
//
// <char*> tributeStr - either "tribute" or "aid".
//
// talk_para1 - amount of the tribute.
//
void TalkMsg::give_tribute(const char* tributeStr)
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom offers you <$999> in aid/tribute.
	// You offer <King>'s Kingdom <$999> in aid/tribute.
	//
	// Reply:
	// <King>'s Kingdom accepts/rejects your aid/tribute of <$999>.
	// You accept/reject the <$999> in aid/tribute from <King>'s Kingdom.
	//
	//---------------------------------------------//

	bool isAid = false;

	if( strncmp(tributeStr, "aid", 7) == 0 )
	{
		isAid = true;
	}

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( isAid )
			{
				// TRANSLATORS: You offer <King>'s Kingdom<Color> <Amount> in aid.
				snprintf(str, MAX_STR_LEN+1, _("You offer %s's Kingdom%s %s in aid."), TO_NATION, TO_COLOR, misc.format(talk_para1, 2));
			}
			else
			{
				// TRANSLATORS: You offer <King>'s Kingdom<Color> <Amount> in tribute.
				snprintf(str, MAX_STR_LEN+1, _("You offer %s's Kingdom%s %s in tribute."), TO_NATION, TO_COLOR, misc.format(talk_para1, 2));
			}
		}
		else
		{
			if( isAid )
			{
				// TRANSLATORS: <King>'s Kingdom<Color> offers you <Amount> in aid.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s offers you %s in aid."), FROM_NATION, FROM_COLOR, misc.format(talk_para1, 2));
			}
			else
			{
				// TRANSLATORS: <King>'s Kingdom<Color> offers you <Amount> in tribute.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s offers you %s in tribute."), FROM_NATION, FROM_COLOR, misc.format(talk_para1, 2));
			}
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
			{
				if( isAid )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> accepts your aid of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s accepts your aid of %s."), TO_NATION, TO_COLOR, misc.format(talk_para1, 2));
				}
				else
				{
					// TRANSLATORS: <King>'s Kingdom<Color> accepts your tribute of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s accepts your tribute of %s."), TO_NATION, TO_COLOR, misc.format(talk_para1, 2));
				}
			}
			else
			{
				if( isAid )
				{
					// TRANSLATORS: <King>'s Kingdom<Color> rejects your aid of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s rejects your aid of %s."), TO_NATION, TO_COLOR, misc.format(talk_para1, 2));
				}
				else
				{
					// TRANSLATORS: <King>'s Kingdom<Color> rejects your tribute of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s rejects your tribute of %s."), TO_NATION, TO_COLOR, misc.format(talk_para1, 2));
				}
			}
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				if( isAid )
				{
					// TRANSLATORS: You accept the <Amount> in aid from <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the %s's Kingdom%s in aid from %s."), misc.format(talk_para1, 2), FROM_NATION, FROM_COLOR);
				}
				else
				{
					// TRANSLATORS: You accept the <Amount> in tribute from <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the %s's Kingdom%s in tribute from %s."), misc.format(talk_para1, 2), FROM_NATION, FROM_COLOR);
				}
			}
			else
			{
				if( isAid )
				{
					// TRANSLATORS: You reject the <Amount> in aid from <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the %s's Kingdom%s in aid from %s."), misc.format(talk_para1, 2), FROM_NATION, FROM_COLOR);
				}
				else
				{
					// TRANSLATORS: You reject the <Amount> in tribute from <King>'s Kingdom<Color>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the %s's Kingdom%s in tribute from %s."), misc.format(talk_para1, 2), FROM_NATION, FROM_COLOR);
				}
			}
		}
	}
}
//------- End of function TalkMsg::give_tribute ------//


//----- Begin of function TalkMsg::demand_tribute ------//
//
// <int> isAid - 1 if it's a aid, 0 if it's a tribute.
//
// talk_para1 - the amount of the tribute.
//
void TalkMsg::demand_tribute(int isAid)
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom requests/demands <tribute amount> in aid/tribute
	// from you.
	//
	// You request/demand <tribute amount> in aid/tribute from
	// <King>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees/refuses to give/pay you <tribute amount>
	// in aid/tribute.
	//
	// You agree/refuse to give/pay <King>'s Kingdom <tribute amount>
	// in aid/tribute.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( isAid )
				// TRANSLATORS: You request <Amount> in aid from <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You request %s in aid from %s's Kingdom%s."), misc.format(talk_para1,2), TO_NATION, TO_COLOR);
			else
				// TRANSLATORS: You demand <Amount> in tribute from <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You demand %s in tribute from %s's Kingdom%s."), misc.format(talk_para1,2), TO_NATION, TO_COLOR);
		}
		else
		{
			if( isAid )
				// TRANSLATORS: <King>'s Kingdom<Color> requests <Amount> in aid from you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s requests %s in aid from you."), FROM_NATION, FROM_COLOR, misc.format(talk_para1,2));
			else
				// TRANSLATORS: <King>'s Kingdom<Color> demands <Amount> in tribute from you.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s demands %s in tribute from you."), FROM_NATION, FROM_COLOR, misc.format(talk_para1,2));
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: <King>'s Kingdom<Color> agrees to give you <Amount> in aid.
						_("%s's Kingdom%s agrees to give you %s in aid.") :
					 // TRANSLATORS: <King>'s Kingdom<Color> agrees to pay you <Amount> in tribute.
						_("%s's Kingdom%s agrees to pay you %s in tribute."),
					 TO_NATION, TO_COLOR,
					 misc.format(talk_para1,2));
			else
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: <King>'s Kingdom<Color> refuses to give you <Amount> in aid.
						_("%s's Kingdom%s refuses to give you %s in aid.") :
					 // TRANSLATORS: <King>'s Kingdom<Color> refuses to pay you <Amount> in tribute.
						_("%s's Kingdom%s refuses to pay you %s in tribute."),
					 TO_NATION, TO_COLOR,
					 misc.format(talk_para1,2));
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: You agree to give <King>'s Kingdom<Color> <Amount> in aid.
						_("You agree to give %s's Kingdom%s %s in aid.") :
					 // TRANSLATORS: You agree to pay <King>'s Kingdom<Color> <Amount> in tribute.
						_("You agree to pay %s's Kingdom%s %s in tribute."),
					 FROM_NATION, FROM_COLOR,
					 misc.format(talk_para1,2));
			else
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: You refuse to give <King>'s Kingdom<Color> <Amount> in aid.
						_("You refuse to give %s's Kingdom%s %s in aid.") :
					 // TRANSLATORS: You refuse to pay <King>'s Kingdom<Color> <Amount> in tribute.
						_("You refuse to pay %s's Kingdom%s %s in tribute."),
					 FROM_NATION, FROM_COLOR,
					 misc.format(talk_para1,2));
		}
	}
}
//------- End of function TalkMsg::demand_tribute ------//


const char *give_tech_you_offer[] =
{
	// TRANSLATORS: You offer <Tech><Level> technology to <King>'s Kingdom<Color>.
	N_("You offer Catapult%s technology to %s's Kingdom%s."),
	N_("You offer Porcupine%s technology to %s's Kingdom%s."),
	N_("You offer Ballista%s technology to %s's Kingdom%s."),
	N_("You offer Cannon%s technology to %s's Kingdom%s."),
	N_("You offer Spitfire%s technology to %s's Kingdom%s."),
	N_("You offer Caravel%s technology to %s's Kingdom%s."),
	N_("You offer Galleon%s technology to %s's Kingdom%s."),
	N_("You offer Unicorn%s technology to %s's Kingdom%s."),
};
const char *give_tech_kingdom_offers[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> offers <Tech><Level> technology to you.
	N_("%s's Kingdom%s offers Catapult%s technology to you."),
	N_("%s's Kingdom%s offers Porcupine%s technology to you."),
	N_("%s's Kingdom%s offers Ballista%s technology to you."),
	N_("%s's Kingdom%s offers Cannon%s technology to you."),
	N_("%s's Kingdom%s offers Spitfire%s technology to you."),
	N_("%s's Kingdom%s offers Caravel%s technology to you."),
	N_("%s's Kingdom%s offers Galleon%s technology to you."),
	N_("%s's Kingdom%s offers Unicorn%s technology to you."),
};
const char *give_tech_accepts_your_offer[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> accepts your gift of <Tech> technology.
	N_("%s's Kingdom%s accepts your gift of Catapult%s technology."),
	N_("%s's Kingdom%s accepts your gift of Porcupine%s technology."),
	N_("%s's Kingdom%s accepts your gift of Ballista%s technology."),
	N_("%s's Kingdom%s accepts your gift of Cannon%s technology."),
	N_("%s's Kingdom%s accepts your gift of Spitfire%s technology."),
	N_("%s's Kingdom%s accepts your gift of Caravel%s technology."),
	N_("%s's Kingdom%s accepts your gift of Galleon%s technology."),
	N_("%s's Kingdom%s accepts your gift of Unicorn%s technology."),
};
const char *give_tech_rejects_your_offer[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> rejects your gift of <Tech><Level> technology.
	N_("%s's Kingdom%s rejects your gift of Catapult%s technology."),
	N_("%s's Kingdom%s rejects your gift of Porcupine%s technology."),
	N_("%s's Kingdom%s rejects your gift of Ballista%s technology."),
	N_("%s's Kingdom%s rejects your gift of Cannon%s technology."),
	N_("%s's Kingdom%s rejects your gift of Spitfire%s technology."),
	N_("%s's Kingdom%s rejects your gift of Caravel%s technology."),
	N_("%s's Kingdom%s rejects your gift of Galleon%s technology."),
	N_("%s's Kingdom%s rejects your gift of Unicorn%s technology."),
};
const char *give_tech_you_accept[] =
{
	// TRANSLATORS: You accept the gift of <Tech><Level> technology from <King>'s Kingdom<Color>.
	N_("You accept the gift of Catapult%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Porcupine%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Ballista%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Cannon%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Spitfire%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Caravel%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Galleon%s technology from %s's Kingdom%s."),
	N_("You accept the gift of Unicorn%s technology from %s's Kingdom%s."),
};
const char *give_tech_you_reject[] =
{
	// TRANSLATORS: You reject the gift of <Tech><Level> technology from <King>'s Kingdom<Color>.
	N_("You reject the gift of Catapult%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Porcupine%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Ballista%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Cannon%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Spitfire%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Caravel%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Galleon%s technology from %s's Kingdom%s."),
	N_("You reject the gift of Unicorn%s technology from %s's Kingdom%s."),
};
//----- Begin of function TalkMsg::give_tech ------//
//
// talk_para1 - id. of the tech given.
// talk_para2 - version of the tech.
//
void TalkMsg::give_tech()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom offers <tech><version> technology to you.
	//
	// You offer <tech><version> technology to <King>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom accepts/rejects your gift of <tech><version>
	// technology.
	//
	// You accept/reject the gift of <tech><version> technology
	// from <King>'s Kingdom.
	//
	//---------------------------------------------//

	String tech;

	if( talk_para2 )		// Ships do not have different versions
	{
		tech += " ";
		tech += misc.roman_number(talk_para2);
	}

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			snprintf(str, MAX_STR_LEN+1, _(give_tech_you_offer[talk_para1-1]), tech, TO_NATION, TO_COLOR);
		}
		else
		{
			snprintf(str, MAX_STR_LEN+1, _(give_tech_kingdom_offers[talk_para1-1]), FROM_NATION, FROM_COLOR, tech);
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str, MAX_STR_LEN+1, _(give_tech_accepts_your_offer[talk_para1-1]), TO_NATION, TO_COLOR, tech);
			else
				snprintf(str, MAX_STR_LEN+1, _(give_tech_rejects_your_offer[talk_para1-1]), TO_NATION, TO_COLOR, tech);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str, MAX_STR_LEN+1, _(give_tech_you_accept[talk_para1-1]), tech, FROM_NATION, FROM_COLOR);
			else
				snprintf(str, MAX_STR_LEN+1, _(give_tech_you_reject[talk_para1-1]), tech, FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::give_tech ------//


const char *demand_tech_you_request[] =
{
	// TRANSLATORS: You request the latest <Tech> technology from <King>'s Kingdom<Color>.
	N_("You request the latest Catapult technology from %s's Kingdom%s."),
	N_("You request the latest Porcupine technology from %s's Kingdom%s."),
	N_("You request the latest Ballista technology from %s's Kingdom%s."),
	N_("You request the latest Cannon technology from %s's Kingdom%s."),
	N_("You request the latest Spitfire technology from %s's Kingdom%s."),
	N_("You request the latest Caravel technology from %s's Kingdom%s."),
	N_("You request the latest Galleon technology from %s's Kingdom%s."),
	N_("You request the latest Unicorn technology from %s's Kingdom%s."),
};
const char *demand_tech_you_demand[] =
{
	// TRANSLATORS: You demand the latest <Tech> technology from <King>'s Kingdom<Color>.
	N_("You demand the latest Catapult technology from %s's Kingdom%s."),
	N_("You demand the latest Porcupine technology from %s's Kingdom%s."),
	N_("You demand the latest Ballista technology from %s's Kingdom%s."),
	N_("You demand the latest Cannon technology from %s's Kingdom%s."),
	N_("You demand the latest Spitfire technology from %s's Kingdom%s."),
	N_("You demand the latest Caravel technology from %s's Kingdom%s."),
	N_("You demand the latest Galleon technology from %s's Kingdom%s."),
	N_("You demand the latest Unicorn technology from %s's Kingdom%s."),
};
const char *demand_tech_kingdom_requests[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> requests the latest <Tech> technology from you.
	N_("%s's Kingdom%s requests the latest Catapult technology from you."),
	N_("%s's Kingdom%s requests the latest Porcupine technology from you."),
	N_("%s's Kingdom%s requests the latest Ballista technology from you."),
	N_("%s's Kingdom%s requests the latest Cannon technology from you."),
	N_("%s's Kingdom%s requests the latest Spitfire technology from you."),
	N_("%s's Kingdom%s requests the latest Caravel technology from you."),
	N_("%s's Kingdom%s requests the latest Galleon technology from you."),
	N_("%s's Kingdom%s requests the latest Unicorn technology from you."),
};
const char *demand_tech_kingdom_demands[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> demands the latest <Tech> technology from you.
	N_("%s's Kingdom%s demands the latest Catapult technology from you."),
	N_("%s's Kingdom%s demands the latest Porcupine technology from you."),
	N_("%s's Kingdom%s demands the latest Ballista technology from you."),
	N_("%s's Kingdom%s demands the latest Cannon technology from you."),
	N_("%s's Kingdom%s demands the latest Spitfire technology from you."),
	N_("%s's Kingdom%s demands the latest Caravel technology from you."),
	N_("%s's Kingdom%s demands the latest Galleon technology from you."),
	N_("%s's Kingdom%s demands the latest Unicorn technology from you."),
};
const char *demand_tech_kingdom_agrees[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> agrees to transfer its latest <Tech> technology to you.
	N_("%s's Kingdom%s agrees to transfer its latest Catapult technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Porcupine technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Ballista technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Cannon technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Spitfire technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Caravel technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Galleon technology to you."),
	N_("%s's Kingdom%s agrees to transfer its latest Unicorn technology to you."),
};
const char *demand_tech_kingdom_refuses[] =
{
	// TRANSLATORS: <King>'s Kingdom<Color> refuses to transfer its latest <Tech> technology to you.
	N_("%s's Kingdom%s refuses to transfer its latest Catapult technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Porcupine technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Ballista technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Cannon technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Spitfire technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Caravel technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Galleon technology to you."),
	N_("%s's Kingdom%s refuses to transfer its latest Unicorn technology to you."),
};
const char *demand_tech_you_agree[] =
{
	// TRANSLATORS: You agree to transfer your latest <Tech> technology to <King>'s Kingdom<Color>.
	N_("You agree to transfer your latest Catapult technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Porcupine technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Ballista technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Cannon technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Spitfire technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Caravel technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Galleon technology to %s's Kingdom%s."),
	N_("You agree to transfer your latest Unicorn technology to %s's Kingdom%s."),
};
const char *demand_tech_you_refuse[] =
{
	// TRANSLATORS: You refuse to transfer your latest <Tech> technology to <King>'s Kingdom<Color>.
	N_("You refuse to transfer your latest Catapult technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Porcupine technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Ballista technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Cannon technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Spitfire technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Caravel technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Galleon technology to %s's Kingdom%s."),
	N_("You refuse to transfer your latest Unicorn technology to %s's Kingdom%s."),
};
//----- Begin of function TalkMsg::demand_tech ------//
//
// Demand for the latest version of the technology.
//
// talk_para1 - id. of the tech demanded.
// talk_para2 - version of the tech if the nation agrees to transfer
//					 technology.
//
void TalkMsg::demand_tech()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom demands/requests the latest
	// <tech> technology from you.
	//
	// You demand/request the latest <tech> technology from
	// <King>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees/refuses to transfer its latest <tech>
	// technology to you.
	//
	// You agree/refuse to transfer your <tech> technology to
	// <King>'s Kingdom.
	//
	//---------------------------------------------//

	bool friendlyRequest = false;

	if( nation_array[from_nation_recno]->get_relation_status(to_nation_recno)
		 >= NATION_FRIENDLY )
	{
		friendlyRequest = true;
	}

	//------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( friendlyRequest )
			{
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_you_request[talk_para1-1]), TO_NATION, TO_COLOR);
			}
			else
			{
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_you_demand[talk_para1-1]), TO_NATION, TO_COLOR);
			}
		}
		else
		{
			if( friendlyRequest )
			{
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_kingdom_requests[talk_para1-1]), FROM_NATION, FROM_COLOR);
			}
			else
			{
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_kingdom_demands[talk_para1-1]), FROM_NATION, FROM_COLOR);
			}
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_kingdom_agrees[talk_para1-1]), TO_NATION, TO_COLOR);
			else
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_kingdom_refuses[talk_para1-1]), TO_NATION, TO_COLOR);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_you_agree[talk_para1-1]), FROM_NATION, FROM_COLOR);
			else
				snprintf(str, MAX_STR_LEN+1, _(demand_tech_you_refuse[talk_para1-1]), FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::demand_tech ------//


//----- Begin of function TalkMsg::request_military_aid ------//
//
void TalkMsg::request_military_aid()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom requests immediate military aid from you.
	// You request immediate military aid from <King>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees to immediately send your requested
	// military aid.
	// <King>'s Kingdom denies you your requested military aid.
	//
	// You agree to immediately send military aid to <King>'s Kingdom.
	// You refuse to send military aid to <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You request immediate military aid from <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You request immediate military aid from %s's Kingdom%s."), TO_NATION, TO_COLOR);
		}
		else
		{
			// TRANSLATORS: <King>'s Kingdom<Color> requests immediate military aid from you.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s requests immediate military aid from you."), FROM_NATION, FROM_COLOR);
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King>'s Kingdom<Color> agrees to immediately send your requested military aid.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s agrees to immediately send your requested military aid."), TO_NATION, TO_COLOR);
			else
				// TRANSLATORS: <King>'s Kingdom<Color> denies you your requested military aid.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s denies you your requested military aid."), TO_NATION, TO_COLOR);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to immediately send military aid to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to immediately send military aid to %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
			else
				// TRANSLATORS: You refuse to send military aid to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to send military aid to %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::request_military_aid ------//


//----- Begin of function TalkMsg::request_trade_embargo ------//
//
// talk_para1 - the nation to have a trade embargo on.
//
void TalkMsg::request_trade_embargo()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom requests you to join an embargo on trade with
	// <King B>'s Kingdom.
	//
	// You request <King>'s Kingdom to join an embargo on trade with
	// <King B>'s Kingdom.
	//
	// Reply:
	// <King>'s Kingdom agrees/refuses to join an embargo on trade
	// with <King B>'s Kingdom.
	//
	// You agree/refuse to join an embargo on trade with <King B>'s Kingdom
	// as requested by <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You request <King 1>'s Kingdom<Color> to join an embargo on trade with <King 2>'s Kingdom<Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("You request %s's Kingdom%s to join an embargo on trade with %s's Kingdom%s."), TO_NATION, TO_COLOR, NATION_PARA1, COLOR_PARA1);
		}
		else
		{
			// TRANSLATORS: <King 1>'s Kingdom<Color> requests you to join an embargo on trade with <King 2>'s Kingdom<Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s requests you to join an embargo on trade with %s's Kingdom%s."), FROM_NATION, FROM_COLOR, NATION_PARA1, COLOR_PARA1);
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King 1>'s Kingdom<Color> agrees to join an embargo on trade with <King 2>'s Kingdom<Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s agrees to join an embargo on trade with %s's Kingdom%s."), TO_NATION, TO_COLOR, NATION_PARA1, COLOR_PARA1);
			else
				// TRANSLATORS: <King 1>'s Kingdom<Color> refuses to join an embargo on trade with <King 2>'s Kingdom<Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s refuses to join an embargo on trade with %s's Kingdom%s."), TO_NATION, TO_COLOR, NATION_PARA1, COLOR_PARA1);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to join an embargo on trade with <King 1>'s Kingdom<Kingdom color> as requested by <King 2>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to join an embargo on trade with %s's Kingdom%s as requested by %s's Kingdom%s."), NATION_PARA1, COLOR_PARA1, FROM_NATION, FROM_COLOR);
			else
				// TRANSLATORS: You refuse to join an embargo on trade with <King 1>'s Kingdom<Kingdom color> as requested by <King 2>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to join an embargo on trade with %s's Kingdom%s as requested by %s's Kingdom%s."), NATION_PARA1, COLOR_PARA1, FROM_NATION, FROM_COLOR);
		}
	}
}
//------- End of function TalkMsg::request_trade_embargo ------//


//----- Begin of function TalkMsg::request_surrender ------//
//
void TalkMsg::request_surrender()
{
	//---------------------------------------------//
	//
	// Send:
	//
	// To unite our two Kingdoms under his rule, King
	// <King name> offers <amount> for your throne.
	//
	// You offer <amount> for the throne of <King>'s
	// Kingdom.
	//
	// Reply:
	//
	// King <king name> refuses to dishonor himself by
	// selling his throne!
	//
	// King <king name> agrees to take your money in
	// exchange for his throne.
	//
	// You refuse to dishonor yourself by selling your
	// throne to <King>'s kingdom.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You offer <Amount> for the throne of <King>'s Kingdom<Color>.
			snprintf(str, MAX_STR_LEN+1, _("You offer %s for the throne of %s's Kingdom%s."), misc.format(talk_para1*10), TO_NATION, TO_COLOR);
		}
		else
		{
			// TRANSLATORS: To unite our two Kingdoms under his rule, King <Name> offers <Amount> for your throne.
			snprintf(str, MAX_STR_LEN+1, _("To unite our two Kingdoms under his rule, King %s offers %s for your throne."), from_king_name(), misc.format(talk_para1*10));
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: King <Name> agrees to take your money in exchange for his throne.
				snprintf(str, MAX_STR_LEN+1, _("King %s agrees to take your money in exchange for his throne."), to_king_name());
			else
				// TRANSLATORS: King <Name> refuses to dishonor himself by selling his throne!
				snprintf(str, MAX_STR_LEN+1, _("King %s refuses to dishonor himself by selling his throne!"), to_king_name());
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				str = _("You agree to take the money in exchange for your throne.");
			}
			else
			{
				// TRANSLATORS: You refuse to dishonor yourself by selling your throne to <King>'s Kingdom<Color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to dishonor yourself by selling your throne to %s's Kingdom%s."), FROM_NATION, FROM_COLOR);
			}
		}
	}
}
//------- End of function TalkMsg::request_surrender ------//


//----- Begin of function TalkMsg::surrender ------//
//
void TalkMsg::surrender()
{
	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom offerrrendered to you.
	// You have surrendered to <King>'s Kingdom.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		// TRANSLATORS: You have surrendered to <King>'s Kingdom<Color>.
		snprintf(str, MAX_STR_LEN+1, _("You have surrendered to %s's Kingdom%s."), TO_NATION, TO_COLOR);
	}
	else
	{
		// TRANSLATORS: <King>'s Kingdom<Color> has surrendered to you.
		snprintf(str, MAX_STR_LEN+1, _("%s's Kingdom%s has surrendered to you."), FROM_NATION, FROM_COLOR);
	}
}
//------- End of function TalkMsg::surrender ------//

#endif
