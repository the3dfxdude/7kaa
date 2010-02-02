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
			propose_treaty("trade");
			break;

		case TALK_PROPOSE_FRIENDLY_TREATY:
			propose_treaty("friendly");
			break;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			propose_treaty("alliance");
			break;

		case TALK_END_TRADE_TREATY:
			end_treaty("trade");
			break;

		case TALK_END_FRIENDLY_TREATY:
			end_treaty("friendly");
			break;

		case TALK_END_ALLIANCE_TREATY:
			end_treaty("alliance");
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
void TalkMsg::propose_treaty(char* treatyTypeStr)
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

	String treatyStr;
	char*  articleStr;

	if( treatyTypeStr[0] == 'a' )
		articleStr = "an ";
	else
		articleStr = "a ";

	treatyStr  = treatyTypeStr;
	treatyStr += " treaty ";

	//--------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "You propose ";
			str += articleStr;
			str += treatyStr;
			str += " to ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();
			str += " proposes ";
			str += articleStr;
			str += treatyStr;
			str += " to you.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " accepts your proposed ";
			else
				str += " rejects your proposed ";

			str += treatyStr;
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You accept the ";
			else
				str = "You reject the ";

			str += treatyStr;

			str += " proposed by ";
			str += from_nation_name();
			str += ".";
		}
	}
}
//------- End of function TalkMsg::propose_treaty ------//


//----- Begin of function TalkMsg::end_treaty ------//
//
// talk_para1 - treaty type, NATION_FRIENDLY or NATION_ALLIANCE.
//
void TalkMsg::end_treaty(char* treatyTypeStr)
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
		str  = "You terminate your ";
		str += treatyTypeStr;
		str += " treaty with ";
		str +=  to_nation_name();
		str += ".";
	}
	else
	{
		str  = from_nation_name();
		str += " terminates its ";
		str += treatyTypeStr;
		str += " treaty with you.";
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
			str  = "You request a cease-fire with ";
			str +=  to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();
			str += " requests a cease-fire.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees to";
			else
				str += " refuses";

			str += " a cease-fire.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree to";
			else
				str = "You refuse";

			str += " a cease-fire with ";
			str += from_nation_name();
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
			str  = "You request ";
			str +=  to_nation_name();
			str += " to";
		}
		else
		{
			str = from_nation_name();
			str += " requests that you";
		}

		str += " declare war on ";
		str += nation_array[talk_para1]->nation_name();
		str += nation_color_code_str(talk_para1);
		str += ".";
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees";
			else
				str += " refuses";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree";
			else
				str = "You refuse";
		}

		str += " to declare war on ";
		str += nation_array[talk_para1]->nation_name();
		str += nation_color_code_str(talk_para1);
		str += ".";
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
		str =  from_nation_name();
		str += " offers ";
		str += m.format(talk_para2,2);
		str += " for 10 units of food.";

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
			str  = "You request to purchase ";
			str += talk_para1;
			str += " units of food from ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();
			str += " requests to purchase ";
			str += talk_para1;
			str += " units of food from you.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees";
			else
				str += " refuses";

			str += " to sell ";
			str += talk_para1;
			str += " units of food to you.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree";
			else
				str = "You refuse";

			str += " to sell ";
			str += talk_para1;
			str += " units of food to ";
			str += from_nation_name();
			str += ".";
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
		str  = "You declare war on ";
		str += to_nation_name();
		str += ".";
	}
	else
	{
		str  = from_nation_name();
		str += " declares war on you.";
	}
}
//------- End of function TalkMsg::declare_war ------//


//----- Begin of function TalkMsg::give_tribute ------//
//
// <char*> tributeStr - either "tribute" or "aid".
//
// talk_para1 - amount of the tribute.
//
void TalkMsg::give_tribute(char* tributeStr)
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

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "You offer ";
			str += to_nation_name();
			str += " ";
			str += m.format(talk_para1, 2);
			str += " in ";
			str += tributeStr;
			str += ".";
		}
		else
		{
			str  = from_nation_name();
			str += " offers you ";
			str += m.format(talk_para1, 2);
			str += " in ";
			str += tributeStr;
			str += ".";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " accepts your ";
			else
				str += " rejects your ";

			str += tributeStr;
			str += " of ";
			str += m.format(talk_para1, 2);
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You accept the ";
			else
				str = "You reject the ";

			str += m.format(talk_para1, 2);
			str += " in ";
			str += tributeStr;
			str += " from ";
			str += from_nation_name();
			str += ".";
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

	char* aidStr;

	if( isAid )
		aidStr = "aid";
	else
		aidStr = "tribute";

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( isAid )
				str = "You request ";
			else
				str = "You demand ";

			str += m.format(talk_para1,2);
			str += " in ";
			str += aidStr;
			str += " from ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();

			if( isAid )
				str += " requests ";
			else
				str += " demands ";

			str += m.format(talk_para1,2);
			str += " in ";
			str += aidStr;
			str += " from you.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees";
			else
				str += " refuses";

			if( isAid )
				str += " to give you ";
			else
				str += " to pay you ";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree";
			else
				str = "You refuse";

			if( isAid )
				str += " to give ";
			else
				str += " to pay ";

			str += from_nation_name();
			str += " ";
		}

		str += m.format(talk_para1,2);
		str += " in ";
		str += aidStr;
		str += ".";
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

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "You offer ";
			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions 
			{
				str += " ";
				str += m.roman_number(talk_para2);
			}

			str += " technology to ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = from_nation_name();
			str += " offers ";
			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += m.roman_number(talk_para2);
			}

			str += " technology to you.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " accepts your gift of ";
			else
				str += " rejects your gift of ";

			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += m.roman_number(talk_para2);
			}

			str += " technology.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You accept the gift of ";
			else
				str = "You reject the gift of ";

			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += m.roman_number(talk_para2);
			}

			str += " technology from ";
			str += from_nation_name();
			str += ".";
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

	char* requestStr;

	if( nation_array[from_nation_recno]->get_relation_status(to_nation_recno)
		 >= NATION_FRIENDLY )
	{
		requestStr = "request";
	}
	else
	{
		requestStr = "demand";
	}

	//------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "You ";
			str += requestStr;
			str += " the latest ";
			str += tech_res[talk_para1]->tech_des();
			str += " technology from ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = from_nation_name();
			str += " ";
			str += requestStr;
			str += "s the latest ";
			str += tech_res[talk_para1]->tech_des();
			str += " technology from you.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees";
			else
				str += " refuses";

			str += " to transfer its latest ";
			str += tech_res[talk_para1]->tech_des();
			str += " technology to you.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree";
			else
				str = "You refuse";

			str += " to transfer your latest ";
			str += tech_res[talk_para1]->tech_des();
			str += " technology to ";
			str += from_nation_name();
			str += ".";
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
			str  = "You request immediate military aid from ";
			str +=  to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();
			str += " requests immediate military aid from you.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees to immediately send your requested military aid.";
			else
				str += " denies you your requested military aid.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree to immediately send military aid to ";
			else
				str = "You refuse to send military aid to ";

			str += from_nation_name();
			str += ".";
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
			str  = "You request ";
			str += to_nation_name();
		}
		else
		{
			str  = from_nation_name();
			str += " requests you";
		}

		str += " to join an embargo on trade with ";
		str += nation_array[talk_para1]->nation_name();
		str += nation_color_code_str(talk_para1);
		str += ".";
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees";
			else
				str += " refuses";

			str += " to join an embargo on trade with ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "You agree";
			else
				str = "You refuse";

			str += " to join an embargo on trade with ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += " as requested by ";
			str += from_nation_name();
			str += ".";
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
			str  = "You offer ";
			str += talk_para1*10;	// *10 to restore its original value.
			str += " for the throne of ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "To unite our two Kingdoms under his rule, ";
			str += from_king_name();
			str += " offers ";
			str += talk_para1*10;	// *10 to restore its original value.
			str += " for your throne.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_king_name();

			if( reply_type == REPLY_ACCEPT )
				str += " agrees to take your money in exchange for his throne.";
			else
				str += " refuses to dishonor himself by selling his throne!";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				str = "You agree to take the money in exchange for your throne.";
			}
			else
			{
				str  = "You refuse to dishonor yourself by selling your throne to ";
				str += from_nation_name();
				str += ".";
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
		str  = "You have surrendered to ";
		str += to_nation_name();
		str += ".";
	}
	else
	{
		str  = from_nation_name();
		str += " has surrendered to you.";
	}
}
//------- End of function TalkMsg::surrender ------//

#endif