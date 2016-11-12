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

//Filename    : OTALKGER.CPP
//Description : German version of the talk messages

#ifdef GERMAN

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
	//
	// Senden:
	//
	// <King>'s Königreich schlägt Ihnen Freundschafts-/Beistands- Pakt vor.
	// Sie schlagen <King>'s Königreich Freundschafts-/Beistands- Pakt vor.
	//
	// Antwort:
	//
	// <King>'s Königreich akzeptiert/verweigert Ihren Vorschlag für
	// einen Freundschafts-/Beistands- Pakt.
	//
	// Sie akzeptierten/verweigern den von <King>'s Königreich
	// vorgeschlagenen Freundschafts-/Beistands- Pakt.
	//
	//---------------------------------------------//

	String treatyStr;

	treatyStr  = treatyTypeStr;
	treatyStr += " treaty";

	treatyStr = translate.process(treatyStr);

	//--------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie schlagen ";
			str += to_nation_name();
			str += " ";
			str += treatyStr;
			str += " vor.";
		}
		else
		{
			str  = from_nation_name();
			str += " schlägt Ihnen ";
			str += treatyStr;
			str += " vor.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert ";
			else
				str += " verweigert ";

			str += "Ihren Vorschlag für einen ";
			str += treatyStr;
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptierten ";
			else
				str = "Sie verweigern ";

			str += "den von ";
			str += from_nation_name();
			str += " vorgeschlagenen ";
			str += treatyStr;
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
	String treatyStr;

	treatyStr  = treatyTypeStr;
	treatyStr += " treaty";

	treatyStr = translate.process(treatyStr);

	//---------------------------------------------//
	//
	// Send:
	// <King>'s Kingdom terminates its friendly/alliance treaty with you.
	// You terminate your friendly/alliance treaty with <King>'s Kingdom.
	//
	//---------------------------------------------//
	//
	// Senden:
	// <King>'s Königreich kündigt den Freundschafts-/Beistands- Pakt mit Ihnen.
	// Sie kündigen Ihren Freundschafts-/Beistands- Pakt mit <King>'s Königreich.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		str  = "Sie kündigen Ihren ";
		str += treatyStr;
		str += " mit ";
		str += to_nation_name();
		str += ".";
	}
	else
	{
		str  = from_nation_name();
		str += " kündigt den ";
		str += treatyStr;
		str += " mit Ihnen.";
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
	//
	// Senden:
	// <King>'s Königreich erbittet Waffenstillstand.
	// Sie bitten <King>'s Königreich um Waffenstillstand.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert den Waffenstillstand.
	// <King>'s Königreich verweigert den Waffenstillstand.
	// Sie akzeptieren den Waffenstillstand mit <King>'s Königreich. 
	// Sie verweigern Waffenstillstand mit <King>'s Königreich.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bitten ";
			str +=  to_nation_name();
			str += " um Waffenstillstand.";
		}
		else
		{
			str =  from_nation_name();
			str += " erbittet Waffenstillstand.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert";
			else
				str += " verweigert";

			str += " den Waffenstillstand.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren den";
			else
				str = "Sie verweigern";

			str += " Waffenstillstand mit ";
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
	//
	// Senden:
	// <King>'s Königreich erbittet Ihre Kriegserklärung gegen <King B>'s Königreich.
	// Sie bitten <King>'s Königreich um Kriegserklärung gegen <King B>'s Königreich.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert Kriegserklärung gegen <King B>'s Königreich.
	// Sie akzeptieren/verweigern Kriegserklärung gegen <King B>'s Königreich.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bitten ";
			str +=  to_nation_name();
			str += " um";
		}
		else
		{
			str = from_nation_name();
			str += " erbittet Ihre";
		}

		str += " Kriegserklärung gegen ";
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
				str += " akzeptiert";
			else
				str += " verweigert";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren";
			else
				str = "Sie verweigern";
		}

		str += " Kriegserklärung gegen ";
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
	//
	// <King>'s Königreich bietet $10 für 10 Nahrungseinheiten.
	//
	//-------------------------------------------------------//


	if( disp_second_line )
	{
		str =  from_nation_name();
		str += " bietet ";
		str += misc.format(talk_para2,2);
		str += " für 10 Nahrungseinheiten.";

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
	//
	// Senden:
	// <King>'s Königreich erbittet Kauf von <amount>
	// Nahrungs-Einheiten von Ihnen.
	//
	// Sie bitten um Kauf von <amount> Nahrungs-
	// Einheiten von <King>'s Königreich.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert den Verkauf von 
	// <amount> Nahrungs-Einheiten an Sie.
	//
	// Sie akzeptieren/verweigern den Verkauf von <amount>
	// Nahrungs-Einheiten an <King>'s Königreich.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bitten um Kauf von ";
			str += talk_para1;
			str += " Nahrungs-Einheiten von ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();
			str += " erbittet Kauf von ";
			str += talk_para1;
			str += " Nahrungs-Einheiten von Ihnen.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert";
			else
				str += " verweigert";

			str += " den Verkauf von ";
			str += talk_para1;
			str += " Nahrungs-Einheiten an Sie.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren";
			else
				str = "Sie verweigern";

			str += " den Verkauf von ";
			str += talk_para1;
			str += " Nahrungs-Einheiten an ";
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
	//
	// Senden:
	// <King>'s Königreich erklärt Ihnen den Krieg.
	// Sie erklären <King>'s Königreich den Krieg.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		str  = "Sie erklären ";
		str += to_nation_name();
		str += " den Krieg.";
	}
	else
	{
		str  = from_nation_name();
		str += " erklärt Ihnen den Krieg.";
	}
}
//------- End of function TalkMsg::declare_war ------//


//----- Begin of function TalkMsg::give_tribute ------//
//
// <char*> tributeStr - either "tribute" or "aid".
//
// talk_para1 - amount of the tribute.
//
void TalkMsg::give_tribute(char* tributeTypeStr)
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
	//
	// Senden:
	// <King>'s Königreich bietet Ihnen <$999> Hilfe/Tribut an.
	// Sie bieten <King>'s Königreich <$999> Hilfe/Tribut an.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert Hilfe/Tribut von <$999>.
	// Sie akzeptieren/verweigern <$999> Hilfe/Tribut von <King>'s Königreich.
	//
	//---------------------------------------------//

	String tributeStr;

	tributeStr = translate.process(tributeTypeStr);

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bieten ";
			str += to_nation_name();
			str += " ";
			str += misc.format(talk_para1, 2);
			str += " ";
			str += tributeStr;
			str += " an.";
		}
		else
		{
			str  = from_nation_name();
			str += " bietet Ihnen ";
			str += misc.format(talk_para1, 2);
			str += " ";
			str += tributeStr;
			str += " an.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert ";
			else
				str += " verweigert ";

			str += tributeStr;
			str += " von ";
			str += misc.format(talk_para1, 2);
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren ";
			else
				str = "Sie verweigern ";

			str += misc.format(talk_para1, 2);
			str += " ";
			str += tributeStr;
			str += " von ";
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
	//
	// Senden:
	// <King>'s Königreich erbittet/verlangt <tribute amount> Hilfe/Tribut
	// von Ihnen.
	//
	// Sie erbitten/verlangen <tribute amount> Hilfe/Tribut von
	// <King>'s Königreich.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert Zahlung von <tribute amount>
	// Hilfe/Tribut an Sie.	     //CAUTION: No more alternating 'give/pay', just only: 'Zahlung'!
	//
	// Sie akzeptieren/verweigern Zahlung von <tribute amount> Hilfe/Tribut
	// an <King>'s Königreich.   // The same here....
	//
	//---------------------------------------------//

	char* aidStr;

	if( isAid )
		aidStr = "Hilfe";
	else
		aidStr = "Tribut";

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( isAid )
				str = "Sie erbitten ";
			else
				str = "Sie verlangen ";

			str += misc.format(talk_para1,2);
			str += " ";
			str += aidStr;
			str += " von ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();

			if( isAid )
				str += " erbittet ";
			else
				str += " verlangt ";

			str += misc.format(talk_para1,2);
			str += " ";
			str += aidStr;
			str += " von Ihnen.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert";
			else
				str += " verweigert Zahlung von ";
			str += misc.format(talk_para1,2);
			str += " ";
			str += aidStr;
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren";
			else
				str = "Sie verweigern";

			str += " Zahlung von ";
			str += misc.format(talk_para1,2);
			str += " ";
			str += aidStr;
			str += " an ";
			str += from_nation_name();
			str += ".";
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
	//
	// Senden:
	// <King>'s Königreich bietet Ihnen die <tech><version> -Technologie an.
	//
	// Sie bieten <King>'s Königreich die <tech><version> -Technologie an.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert Ihr Angebot der <tech><version>
	// -Technologie.
	//
	// Sie akzeptieren/verweigern das Angebot der <tech><version> -Technologie
	// von <King>'s Königreich.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bieten ";
			str += to_nation_name();
		}
		else
		{
			str  = from_nation_name();
			str += " bietet Ihnen";
		}

		str += " die ";

		// BUGHERE : Is technology description translated?
		str += tech_res[talk_para1]->tech_des();

		if( talk_para2 )		// Ships do not have different versions 
		{
			str += " ";
			str += misc.roman_number(talk_para2);
		}

		str += " -Technologie an.";
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert";
			else
				str += " verweigert";

			str += " Ihr Angebot der ";
			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += misc.roman_number(talk_para2);
			}

			str += " -Technologie.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren";
			else
				str = "Sie verweigern";
			
			str += " das Angebot der ";

			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += misc.roman_number(talk_para2);
			}

			str += " -Technologie von ";
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
	//
	// Senden:
	// <King>'s Königreich verlangt/erbittet die neueste
	// <tech> -Technologie von Ihnen.
	//
	// Sie verlangen/erbitten die neueste <tech> -Technologie von
	// <King>'s Königreich.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert den Transfer seiner neuesten
	// <tech> -Technology an Sie.
	//
	// Sie akzeptieren/verweigern den Transfer der neuesten <tech> -Technologie
	// an <King>'s Königreich.
	//
	//---------------------------------------------//

	char friendRelation = ( nation_array[from_nation_recno]->get_relation_status(to_nation_recno)
		 >= NATION_FRIENDLY );

	//------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( friendRelation )
				str = "Sie erbitten";
			else
				str = "Sie verlangen";
			str += " die neueste ";
			str += tech_res[talk_para1]->tech_des();
			str += " -Technologie von ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = from_nation_name();
			if( friendRelation )
				str = " erbittet";
			else
				str = " verlangt";
			str += " die neueste ";
			str += tech_res[talk_para1]->tech_des();
			str += " -Technologie von Ihnen";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert";
			else
				str += " verweigert";

			str += " den Transfer seiner neuesten ";
			str += tech_res[talk_para1]->tech_des();
			str += " -Technology an Sie.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren";
			else
				str = "Sie verweigern";

			str += " den Transfer der neuesten ";
			str += tech_res[talk_para1]->tech_des();
			str += " -Technologie an ";
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
	//
	// Senden:
	// <King>'s Königreich erbittet militärische Soforthilfe von Ihnen.
	// Sie erbitten militärische Soforthilfe von <King>'s Königreich.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert, sofort die erbetene militärische 
	// Hilfe zu entsenden.
	// <King>'s Königreich verweigert die militärische Soforthilfe.
	//
	// Sie akzeptieren die Entsendung von militärischer Soforthilfe zu <King>'s Königreich.
	// Sie verweigern die Entsendung der militärischen Soforthilfe zu <King>'s Königreich.
	//
	//---------------------------------------------//


	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie erbitten militärische Soforthilfe von ";
			str +=  to_nation_name();
			str += ".";
		}
		else
		{
			str =  from_nation_name();
			str += " erbittet militärische Soforthilfe von Ihnen.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert, sofort die erbetene militärische Hilfe zu entsenden.";
			else
				str += " verweigert die militärische Soforthilfe.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren die Entsendung von militärischer Soforthilfe zu ";
			else
				str = "Sie verweigern die Entsendung der militärischen Soforthilfe zu ";

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
	//
	// Senden:
	// <King>'s Königreich bittet Sie, sich einem Handels-Embargo gegen
	// <King B>'s Königreich anzuschließen.
	//
	// Sie bitten <King>'s Königreich, sich einem Handels-Embargo gegen
	// <King B>'s Königreich anzuschließen.
	//
	// Antwort:
	// <King>'s Königreich akzeptiert/verweigert, sich dem Handels-Embargo gegen
	// <King B>'s Königreich anzuschließen.
	//
	// Sie akzeptieren/verweigern, sich dem Handels-Embargo gegen  <King B>'s 
	// Königreich anzuschließen, das von <King>'s Königreich erbeten wurde.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bitten ";
			str += to_nation_name();
		}
		else
		{
			str  = from_nation_name();
			str += " bittet Sie";
		}

		str += ", sich einem Handels-Embargo gegen ";
		str += nation_array[talk_para1]->nation_name();
		str += nation_color_code_str(talk_para1);
		str += " anzuschließen.";
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert";
			else
				str += " verweigert";

			str += ", sich dem Handels-Embargo gegen ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += " anzuschließen.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Sie akzeptieren";
			else
				str = "Sie verweigern";

			str += ", sich dem Handels-Embargo gegen ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += " anzuschließen, das von ";
			str += from_nation_name();
			str += " erbeten wurde.";
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
	//
	// Senden:
	//
	// Um beide Königreiche zu vereinen, bietet Ihnen 
	// <King name> den Betrag von <amount> für Ihren Thron an.
	//
	// Sie bieten <amount> für den Thron von <King>'s
	// Königreich.
	//
	// Antwort:
	//
	// <king name> verweigert, sich selbst zu entehren, 
	// indem er seinen Thron verkauft!
	//
	// <king name> akzeptiert Ihre Zahlung im Tausch
	// für seinen Thron.
	//
	// Sie verweigen es, sich selbst zu entehren, indem Sie Ihren
	// Thron an <King>'s Königreich verkaufen.
	//
	//---------------------------------------------//


	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Sie bieten ";
			str += talk_para1*10;	// *10 to restore its original value.
			str += " für den Thron von ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "Um beide Königreiche zu vereinen, bietet Ihnen ";
			str += from_king_name();
			str += " den Betrag von ";
			str += talk_para1*10;	// *10 to restore its original value.
			str += " für Ihren Thron an.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = to_king_name();

			if( reply_type == REPLY_ACCEPT )
				str += " akzeptiert Ihre Zahlung im Tausch für seinen Thron.";
			else
				str += " verweigert, sich selbst zu entehren, indem er seinen Thron verkauft!";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				// BUGHERE : not translated, but this message never appears
				str = "You agree to take the money in exchange for your throne.";
			}
			else
			{
				str  = "Sie verweigen es, sich selbst zu entehren, indem Sie Ihren Thron an ";
				str += from_nation_name();
				str += " verkaufen.";
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
	//
	// Senden:
	// <King>'s Königreich kapituliert vor Ihnen.
	// Sie haben vor <King>'s Königreich kapituliert.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		str  = "Sie haben vor ";
		str += to_nation_name();
		str += " kapituliert.";
	}
	else
	{
		str  = from_nation_name();
		str += " kapituliert vor Ihnen.";
	}
}
//------- End of function TalkMsg::surrender ------//

#endif
