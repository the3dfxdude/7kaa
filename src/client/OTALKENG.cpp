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
				// TRANSLATORS: You propose a trade treaty to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You propose a trade treaty to %s."), to_nation_name());
			}
			else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
			{
				// TRANSLATORS: You propose a friendly treaty to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You propose a friendly treaty to %s."), to_nation_name());
			}
			else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
			{
				// TRANSLATORS: You propose an alliance treaty to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You propose an alliance treaty to %s."), to_nation_name());
			}
		}
		else
		{
			if( treatyType == TALK_PROPOSE_TRADE_TREATY )
			{
				// TRANSLATORS: <King's Kingdom> proposes a trade treaty to you.
				snprintf(str, MAX_STR_LEN+1, _("%s proposes a trade treaty to you."), from_nation_name());
			}
			else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
			{
				// TRANSLATORS: <King's Kingdom> proposes a friendly treaty to you.
				snprintf(str, MAX_STR_LEN+1, _("%s proposes a friendly treaty to you."), from_nation_name());
			}
			else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
			{
				// TRANSLATORS: <King's Kingdom> proposes an alliance treaty to you.
				snprintf(str, MAX_STR_LEN+1, _("%s proposes an alliance treaty to you."), from_nation_name());
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
					// TRANSLATORS: <King's Kingdom> accepts your proposed trade treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s accepts your proposed trade treaty."), to_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: <King's Kingdom> accepts your proposed friendly treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s accepts your proposed friendly treaty."), to_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: <King's Kingdom> accepts your proposed alliance treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s accepts your proposed alliance treaty."), to_nation_name());
				}
			}
			else
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: <King's Kingdom> rejects your proposed trade treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s rejects your proposed trade treaty."), to_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: <King's Kingdom> rejects your proposed friendly treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s rejects your proposed friendly treaty."), to_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: <King's Kingdom> rejects your proposed alliance treaty.
					snprintf(str, MAX_STR_LEN+1, _("%s rejects your proposed alliance treaty."), to_nation_name());
				}
			}
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: You accept the trade treaty proposed by <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the trade treaty proposed by %s."), from_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: You accept the friendly treaty proposed by <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the friendly treaty proposed by %s."), from_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: You accept the alliance treaty proposed by <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the alliance treaty proposed by %s."), from_nation_name());
				}
			}
			else
			{
				if( treatyType == TALK_PROPOSE_TRADE_TREATY )
				{
					// TRANSLATORS: You reject the trade treaty proposed by <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the trade treaty proposed by %s."), from_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_FRIENDLY_TREATY )
				{
					// TRANSLATORS: You reject the friendly treaty proposed by <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the friendly treaty proposed by %s."), from_nation_name());
				}
				else if( treatyType == TALK_PROPOSE_ALLIANCE_TREATY )
				{
					// TRANSLATORS: You reject the alliance treaty proposed by <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the alliance treaty proposed by %s."), from_nation_name());
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
			// TRANSLATORS: You terminate your trade treaty with <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You terminate your trade treaty with %s."), to_nation_name());
		}
		else if( treatyType == TALK_END_FRIENDLY_TREATY )
		{
			// TRANSLATORS: You terminate your friendly treaty with <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You terminate your friendly treaty with %s."), to_nation_name());
		}
		else if( treatyType == TALK_END_ALLIANCE_TREATY )
		{
			// TRANSLATORS: You terminate your alliance treaty with <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You terminate your alliance treaty with %s."), to_nation_name());
		}
	}
	else
	{
		if( treatyType == TALK_END_TRADE_TREATY )
		{
			// TRANSLATORS: <King's Kingdom> terminates its trade treaty with you.
			snprintf(str, MAX_STR_LEN+1, _("%s terminates its trade treaty with you."), from_nation_name());
		}
		else if( treatyType == TALK_END_FRIENDLY_TREATY )
		{
			// TRANSLATORS: <King's Kingdom> terminates its friendly treaty with you.
			snprintf(str, MAX_STR_LEN+1, _("%s terminates its friendly treaty with you."), from_nation_name());
		}
		else if( treatyType == TALK_END_ALLIANCE_TREATY )
		{
			// TRANSLATORS: <King's Kingdom> terminates its alliance treaty with you.
			snprintf(str, MAX_STR_LEN+1, _("%s terminates its alliance treaty with you."), from_nation_name());
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
			// TRANSLATORS: You request a cease-fire with <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You request a cease-fire with %s."), to_nation_name());
		}
		else
		{
			// TRANSLATORS: <King's Kingdom> requests a cease-fire.
			snprintf(str, MAX_STR_LEN+1, _("%s requests a cease-fire."), from_nation_name());
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King's Kingdom> agrees to a cease-fire.
				snprintf(str, MAX_STR_LEN+1, _("%s agrees to a cease-fire."), to_nation_name());
			else
				// TRANSLATORS: <King's Kingdom> refuses a cease-fire.
				snprintf(str, MAX_STR_LEN+1, _("%s refuses a cease-fire."), to_nation_name());
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to a cease-fire with <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to a cease-fire with %s."), from_nation_name());
			else
				// TRANSLATORS: You refuse a cease-fire with <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse a cease-fire with %s."), from_nation_name());
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
			// TRANSLATORS: You request <King 1's Kingdom> to declare war on <King 2's Kingdom><Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("You request %1$s to declare war on %2$s%3$s."), to_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
		}
		else
		{
			// TRANSLATORS: <King 1's Kingdom> requests that you declare war on <King 2's Kingdom><Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("%1$s requests that you declare war on %2$s%3$s."), from_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King 1's Kingdom> agrees to declare war on <King 2's Kingdom><Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%1$s agrees to declare war on %2$s%3$s."), to_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
			else
				// TRANSLATORS: <King 1's Kingdom> refuses to declare war on <King 2's Kingdom><Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%1$s refuses to declare war on %2$s%3$s."), to_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to declare war on <King's Kingdom><Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to declare war on %1$s%2$s."), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
			else
				// TRANSLATORS: You refuse to declare war on <King's Kingdom><Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to declare war on %1$s%2$s."), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
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
		// TRANSLATORS: <King's Kingdom> offers <Amount> for 10 units of food.
		snprintf(str, MAX_STR_LEN+1, _("%1$s offers %2$s for 10 units of food."), from_nation_name(), misc.format(talk_para2,2));

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
			// TRANSLATORS: You request to purchase <Number> units of food from <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You request to purchase %1$s units of food from %2$s."), misc.format(talk_para1), to_nation_name());
		}
		else
		{
			// TRANSLATORS: <King's Kingdom> requests to purchase <Number> units of food from you.
			snprintf(str, MAX_STR_LEN+1, _("%1$s requests to purchase %2$s units of food from you."), from_nation_name(), misc.format(talk_para1));
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King's Kingdom> agrees to sell <Number> units of food to you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s agrees to sell %2$s units of food to you."), to_nation_name(), misc.format(talk_para1));
			else
				// TRANSLATORS: <King's Kingdom> refuses to sell <Number> units of food to you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s refuses to sell %2$s units of food to you."), to_nation_name(), misc.format(talk_para1));
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to sell <Number> units of food to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to sell %1$s units of food to %2$s."), misc.format(talk_para1), from_nation_name());
			else
				// TRANSLATORS: You refuse to sell <Number> units of food to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to sell %1$s units of food to %2$s."), misc.format(talk_para1), from_nation_name());
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
		// TRANSLATORS: You declare war on <King's Kingdom>.
		snprintf(str, MAX_STR_LEN+1, _("You declare war on %s."), to_nation_name());
	}
	else
	{
		// TRANSLATORS: <King's Kingdom> declares war on you.
		snprintf(str, MAX_STR_LEN+1, _("%s declares war on you."), from_nation_name());
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
				// TRANSLATORS: You offer <King's Kingdom> <Amount> in aid.
				snprintf(str, MAX_STR_LEN+1, _("You offer %1$s %2$s in aid."), to_nation_name(), misc.format(talk_para1, 2));
			}
			else
			{
				// TRANSLATORS: You offer <King's Kingdom> <Amount> in tribute.
				snprintf(str, MAX_STR_LEN+1, _("You offer %1$s %2$s in tribute."), to_nation_name(), misc.format(talk_para1, 2));
			}
		}
		else
		{
			if( isAid )
			{
				// TRANSLATORS: <King's Kingdom> offers you <Amount> in aid.
				snprintf(str, MAX_STR_LEN+1, _("%1$s offers you %2$s in aid."), from_nation_name(), misc.format(talk_para1, 2));
			}
			else
			{
				// TRANSLATORS: <King's Kingdom> offers you <Amount> in tribute.
				snprintf(str, MAX_STR_LEN+1, _("%1$s offers you %2$s in tribute."), from_nation_name(), misc.format(talk_para1, 2));
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
					// TRANSLATORS: <King's Kingdom> accepts your aid of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%1$s accepts your aid of %2$s."), to_nation_name(), misc.format(talk_para1, 2));
				}
				else
				{
					// TRANSLATORS: <King's Kingdom> accepts your tribute of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%1$s accepts your tribute of %2$s."), to_nation_name(), misc.format(talk_para1, 2));
				}
			}
			else
			{
				if( isAid )
				{
					// TRANSLATORS: <King's Kingdom> rejects your aid of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%1$s rejects your aid of %2$s."), to_nation_name(), misc.format(talk_para1, 2));
				}
				else
				{
					// TRANSLATORS: <King's Kingdom> rejects your tribute of <Amount>.
					snprintf(str, MAX_STR_LEN+1, _("%1$s rejects your tribute of %2$s."), to_nation_name(), misc.format(talk_para1, 2));
				}
			}
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				if( isAid )
				{
					// TRANSLATORS: You accept the <Amount> in aid from <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the %1$s in aid from %2$s."), misc.format(talk_para1, 2), from_nation_name());
				}
				else
				{
					// TRANSLATORS: You accept the <Amount> in tribute from <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You accept the %1$s in tribute from %2$s."), misc.format(talk_para1, 2), from_nation_name());
				}
			}
			else
			{
				if( isAid )
				{
					// TRANSLATORS: You reject the <Amount> in aid from <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the %1$s in aid from %2$s."), misc.format(talk_para1, 2), from_nation_name());
				}
				else
				{
					// TRANSLATORS: You reject the <Amount> in tribute from <King's Kingdom>.
					snprintf(str, MAX_STR_LEN+1, _("You reject the %1$s in tribute from %2$s."), misc.format(talk_para1, 2), from_nation_name());
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
				// TRANSLATORS: You request <Amount> in aid from <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You request %1$s in aid from %2$s."), misc.format(talk_para1,2), to_nation_name());
			else
				// TRANSLATORS: You demand <Amount> in tribute from <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You demand %1$s in tribute from %2$s."), misc.format(talk_para1,2), to_nation_name());
		}
		else
		{
			if( isAid )
				// TRANSLATORS: <King's Kingdom> requests <Amount> in aid from you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s requests %2$s in aid from you."), from_nation_name(), misc.format(talk_para1,2));
			else
				// TRANSLATORS: <King's Kingdom> demands <Amount> in tribute from you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s demands %2$s in tribute from you."), from_nation_name(), misc.format(talk_para1,2));
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
					 // TRANSLATORS: <King's Kingdom> agrees to give you <Amount> in aid.
						_("%1$s agrees to give you %2$s in aid.") :
					 // TRANSLATORS: <King's Kingdom> agrees to pay you <Amount> in tribute.
						_("%1$s agrees to pay you %2$s in tribute."),
					 to_nation_name(),
					 misc.format(talk_para1,2));
			else
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: <King's Kingdom> refuses to give you <Amount> in aid.
						_("%1$s refuses to give you %2$s in aid.") :
					 // TRANSLATORS: <King's Kingdom> refuses to pay you <Amount> in tribute.
						_("%1$s refuses to pay you %2$s in tribute."),
					 to_nation_name(),
					 misc.format(talk_para1,2));
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: You agree to give <King's Kingdom> <Amount> in aid.
						_("You agree to give %1$s %2$s in aid.") :
					 // TRANSLATORS: You agree to pay <King's Kingdom> <Amount> in tribute.
						_("You agree to pay %1$s %2$s in tribute."),
					 from_nation_name(),
					 misc.format(talk_para1,2));
			else
				snprintf(str,
					 MAX_STR_LEN+1,
					 isAid ?
					 // TRANSLATORS: You refuse to give <King's Kingdom> <Amount> in aid.
						_("You refuse to give %1$s %2$s in aid.") :
					 // TRANSLATORS: You refuse to pay <King's Kingdom> <Amount> in tribute.
						_("You refuse to pay %1$s %2$s in tribute."),
					 from_nation_name(),
					 misc.format(talk_para1,2));
		}
	}
}
//------- End of function TalkMsg::demand_tribute ------//


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

	String tech(tech_res[talk_para1]->tech_des());

	if( talk_para2 )		// Ships do not have different versions
	{
		tech += " ";
		tech += misc.roman_number(talk_para2);
	}

	const char *techStr = tech;

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			// TRANSLATORS: You offer <Tech> technology to <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You offer %1$s technology to %2$s."), techStr, to_nation_name());
		}
		else
		{
			// TRANSLATORS: <King's Kingdom> offers <Tech> technology to you.
			snprintf(str, MAX_STR_LEN+1, _("%1$s offers %2$s technology to you."), from_nation_name(), techStr);
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King's Kingdom> accepts your gift of <Tech> technology.
				snprintf(str, MAX_STR_LEN+1, _("%1$s accepts your gift of %2$s technology."), to_nation_name(), techStr);
			else
				// TRANSLATORS: <King's Kingdom> rejects your gift of <Tech> technology.
				snprintf(str, MAX_STR_LEN+1, _("%1$s rejects your gift of %2$s technology."), to_nation_name(), techStr);
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You accept the gift of <Tech> technology from <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You accept the gift of %1$s technology from %2$s."), techStr, from_nation_name());
			else
				// TRANSLATORS: You reject the gift of <Tech> technology from <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You reject the gift of %1$s technology from %2$s."), techStr, from_nation_name());
		}
	}

}
//------- End of function TalkMsg::give_tech ------//


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
				// TRANSLATORS: You request the latest <Tech> technology from <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You request the latest %1$s technology from %2$s."), tech_res[talk_para1]->tech_des(), to_nation_name());
			}
			else
			{
				// TRANSLATORS: You demand the latest <Tech> technology from <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You demand the latest %1$s technology from %2$s."), tech_res[talk_para1]->tech_des(), to_nation_name());
			}
		}
		else
		{
			if( friendlyRequest )
			{
				// TRANSLATORS: <King's Kingdom> requests the latest <Tech> technology from you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s requests the latest %2$s technology from you."), from_nation_name(), tech_res[talk_para1]->tech_des());
			}
			else
			{
				// TRANSLATORS: <King's Kingdom> demands the latest <Tech> technology from you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s demands the latest %2$s technology from you."), from_nation_name(), tech_res[talk_para1]->tech_des());
			}
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King's Kingdom> agrees to transfer its latest <Tech> technology to you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s agrees to transfer its latest %2$s technology to you."), to_nation_name(), tech_res[talk_para1]->tech_des());
			else
				// TRANSLATORS: <King's Kingdom> refuses to transfer its latest <Tech> technology to you.
				snprintf(str, MAX_STR_LEN+1, _("%1$s refuses to transfer its latest %2$s technology to you."), to_nation_name(), tech_res[talk_para1]->tech_des());
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to transfer your latest <Tech> technology to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to transfer your latest %1$s technology to %2$s."), tech_res[talk_para1]->tech_des(), from_nation_name());
			else
				// TRANSLATORS: You refuse to transfer your latest <Tech> technology to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to transfer your latest %1$s technology to %2$s."), tech_res[talk_para1]->tech_des(), from_nation_name());
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
			// TRANSLATORS: You request immediate military aid from <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You request immediate military aid from %s."), to_nation_name());
		}
		else
		{
			// TRANSLATORS: <King's Kingdom> requests immediate military aid from you.
			snprintf(str, MAX_STR_LEN+1, _("%s requests immediate military aid from you."), from_nation_name());
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King's Kingdom> agrees to immediately send your requested military aid.
				snprintf(str, MAX_STR_LEN+1, _("%s agrees to immediately send your requested military aid."), to_nation_name());
			else
				// TRANSLATORS: <King's Kingdom> denies you your requested military aid.
				snprintf(str, MAX_STR_LEN+1, _("%s denies you your requested military aid."), to_nation_name());
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to immediately send military aid to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to immediately send military aid to %s."), from_nation_name());
			else
				// TRANSLATORS: You refuse to send military aid to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to send military aid to %s."), from_nation_name());
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
			// TRANSLATORS: You request <King 1's Kingdom> to join an embargo on trade with <King 2's Kingdom><Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("You request %1$s to join an embargo on trade with %2$s%3$s."), to_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
		}
		else
		{
			// TRANSLATORS: <King 1's Kingdom> requests you to join an embargo on trade with <King 2's Kingdom><Kingdom color>.
			snprintf(str, MAX_STR_LEN+1, _("%1$s requests you to join an embargo on trade with %2$s%3$s."), from_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: <King 1's Kingdom> agrees to join an embargo on trade with <King 2's Kingdom><Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%1$s agrees to join an embargo on trade with %2$s%3$s."), to_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
			else
				// TRANSLATORS: <King 1's Kingdom> refuses to join an embargo on trade with <King 2's Kingdom><Kingdom color>.
				snprintf(str, MAX_STR_LEN+1, _("%1$s refuses to join an embargo on trade with %2$s%3$s."), to_nation_name(), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1));
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				// TRANSLATORS: You agree to join an embargo on trade with <King 1's Kingdom><Kingdom color> as requested by <King 2's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You agree to join an embargo on trade with %1$s%2$s as requested by %3$s."), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1), from_nation_name());
			else
				// TRANSLATORS: You refuse to join an embargo on trade with <King 1's Kingdom><Kingdom color> as requested by <King 2's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to join an embargo on trade with %1$s%2$s as requested by %3$s."), nation_array[talk_para1]->nation_name(), nation_color_code_str(talk_para1), from_nation_name());
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
			// TRANSLATORS: You offer <Amount> for the throne of <King's Kingdom>.
			snprintf(str, MAX_STR_LEN+1, _("You offer %1$s for the throne of %2$s."), misc.format(talk_para1*10), to_nation_name());
		}
		else
		{
			// TRANSLATORS: To unite our two Kingdoms under his rule, King <Name> offers <Amount> for your throne.
			snprintf(str, MAX_STR_LEN+1, _("To unite our two Kingdoms under his rule, King %1$s offers %2$s for your throne."), from_king_name(), misc.format(talk_para1*10));
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
				// TRANSLATORS: You refuse to dishonor yourself by selling your throne to <King's Kingdom>.
				snprintf(str, MAX_STR_LEN+1, _("You refuse to dishonor yourself by selling your throne to %s."), from_nation_name());
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
		// TRANSLATORS: You have surrendered to <King's Kingdom>.
		snprintf(str, MAX_STR_LEN+1, _("You have surrendered to %s."), to_nation_name());
	}
	else
	{
		// TRANSLATORS: <King's Kingdom> has surrendered to you.
		snprintf(str, MAX_STR_LEN+1, _("%s has surrendered to you."), from_nation_name());
	}
}
//------- End of function TalkMsg::surrender ------//

#endif
