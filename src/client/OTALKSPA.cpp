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

//Filename    : OTALKSPA.CPP
//Description : Spanish version of the talk messages

#if(defined(SPANISH))

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
			propose_treaty("comercial");
			break;

		case TALK_PROPOSE_FRIENDLY_TREATY:
			propose_treaty("amistoso");
			break;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			propose_treaty("de alianza");
			break;

		case TALK_END_TRADE_TREATY:
			end_treaty("comercial");
			break;

		case TALK_END_FRIENDLY_TREATY:
			end_treaty("amistoso");
			break;

		case TALK_END_ALLIANCE_TREATY:
			end_treaty("de alianza");
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
			give_tribute("tributo");
			break;

		case TALK_DEMAND_TRIBUTE:
			demand_tribute(0);		// 1-is tribute, not aid
			break;

		case TALK_GIVE_AID:
			give_tribute("ayuda");
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
	// Envío:
	//
	// El Reino de <King> te propone un acuerdo amistoso/de alianza.
	// Propones un acuerdo amistoso/de alianza al Reino de <King>.
	//
	// Respuesta:
	//
	// El Reino de <King> acepta/rechaza tu propuesta de
	// acuerdo amistoso/de alianza.
	//
	// Aceptas/Rechazas el acuerdo amistoso/de alianza
	// propuesto por el Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Propones un acuerdo ";
			str += treatyTypeStr;
			str += " al ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str +=  from_nation_name();
			str += " te propone un acuerdo ";
			str += treatyTypeStr;
			str += ".";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta tu propuesta de acuerdo ";
			else
				str += " rechaza tu propuesta de acuerdo ";

			str += treatyTypeStr;
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas el acuerdo ";
			else
				str = "Rechazas el acuerdo ";

			str += treatyTypeStr;

			str += " propuesto por el ";
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
	//
	// Envío:
	// El Reino de <King> finaliza su acuerdo amistoso/de alianza contigo.
	// Finalizas el acuerdo amistoso/de alianza con el Reino de <King>.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		str  = "Finalizas el acuerdo ";
		str += treatyTypeStr;
		str += " con el ";
		str +=  to_nation_name();
		str += ".";
	}
	else
	{
		str  = "El ";
		str += from_nation_name();
		str += " finaliza su acuerdo ";
		str += treatyTypeStr;
		str += " contigo.";
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
	// Envío:
	// El Reino de <King> solicita un cese el fuego.
	// Solicitas un cese el fuego al Reino de <King>.
	//
	// Respuesta:
	// El Reino de <King> acepta el cese el fuego.
	// El Reino de <King> rechaza el cese el fuego.
	// Aceptas el cese el fuego con el Reino de <King>.
	// Rechazas el cese el fuego con el Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Solicitas un cese el fuego al ";
			str +=  to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " solicita un cese el fuego.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";

			str += " el cese el fuego.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";

			str += " el cese el fuego con el ";
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
	// Envío:
	// El Reino de <King> solicita que declares la guerra al Reino de <King B>.
	// Solicitas al Reino de <King> que declare la guerra al Reino de <King B>.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza declarar la guerra al Reino de <King B>.
	// Aceptas/Rechazas declarar la guerra al Reino de <King B>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Solicitas al ";
			str +=  to_nation_name();
			str += " que declare la guerra al ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " solicita que declares la guerra al ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += ".";
		}

	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";
		}

		str += " declarar la guerra al ";
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
	// El Reino de <King> ofrece $10 por 10 unidades de alimento.
	//
	//-------------------------------------------------------//

	if( disp_second_line )
	{
		str  = "El ";
		str +=  from_nation_name();
		str += " ofrece ";
		str += misc.format(talk_para2,2);
		str += " por 10 unidades de alimento.";

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
	// Envío:
	// El Reino de <King> solicita la compra de <amount>
	// unidades de alimento.
	//
	// Solicitas la compra de <amount> unidades de alimento
	// al Reino de <King>.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza la venta de <amount> unidades
	// de alimento.
	//
	// Aceptas/Rechazas la venta de <amount> unidades de alimento al
	// Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Solicitas la compra de ";
			str += talk_para1;
			str += " unidades de alimento al ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str +=  from_nation_name();
			str += " solicita la compra de ";
			str += talk_para1;
			str += " unidades de alimento.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";

			str += " la venta de ";
			str += talk_para1;
			str += " unidades de alimento.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";

			str += " la venta de ";
			str += talk_para1;
			str += " unidades de alimento al ";
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
	// Envío:
	// El Reino de <King> te declara la guerra.
	// Declaras la guerra al Reino de <King>.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		str  = "Declaras la guerra al ";
		str += to_nation_name();
		str += ".";
	}
	else
	{
		str  = "El ";
		str += from_nation_name();
		str += " te declara la guerra.";
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
	//
	// Envío:
	// El Reino de <King> te ofrece <$999> de ayuda/tributo.
	// Ofreces al Reino de <King> <$999> de ayuda/tributo.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza tu ayuda/tributo de <$999>.
	// Aceptas/Rechazas los <$999> de ayuda/tributo del Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Ofreces al ";
			str += to_nation_name();
			str += " ";
			str += misc.format(talk_para1, 2);
			str += " de ";
			str += tributeStr;
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " te ofrece ";
			str += misc.format(talk_para1, 2);
			str += " de ";
			str += tributeStr;
			str += ".";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta tu ";
			else
				str += " rechaza tu ";

			str += tributeStr;
			str += " de ";
			str += misc.format(talk_para1, 2);
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas los ";
			else
				str = "Rechazas los ";

			str += misc.format(talk_para1, 2);
			str += " de ";
			str += tributeStr;
			str += " del ";
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
	// Envío:
	// El Reino de <King> solicita/exige <tribute amount> de ayuda/tributo
	// a tu Reino.
	//
	// Solicitas/Exiges <tribute amount> de ayuda/tributo al
	// Reino de <King>.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza cederte/pagarte <tribute amount>
	// de ayuda/tributo.
	//
	// Aceptas/Rechazas ceder/pagar al Reino de <King> <tribute amount>
	// de ayuda/tributo.
	//
	//---------------------------------------------//

	char* aidStr;

	if( isAid )
		aidStr = "ayuda";
	else
		aidStr = "tributo";

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( isAid )
				str = "Solicitas ";
			else
				str = "Exiges ";

			str += misc.format(talk_para1,2);
			str += " de ";
			str += aidStr;
			str += " al ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();

			if( isAid )
				str += " solicita ";
			else
				str += " exige ";

			str += misc.format(talk_para1,2);
			str += " de ";
			str += aidStr;
			str += " a tu Reino.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{

			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";

			if( isAid )
				str += " cederte ";
			else
				str += " pagarte ";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";

			if( isAid )
				str += " ceder al ";
			else
				str += " pagar al ";

			str += from_nation_name();
			str += " ";
		}

		str += misc.format(talk_para1,2);
		str += " de ";
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
	//
	// Envío:
	// El Reino de <King> te ofrece la tecnología de <tech><version>.
	//
	// Ofreces la tecnología de <tech><version> al Reino de <King>.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza tu obsequio de tecnología
	// de <tech><version>.
	//
	// Aceptas/Rechazas el obsequio de tecnología de <tech><version>
	// del Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Ofreces la tecnología de ";
			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions 
			{
				str += " ";
				str += misc.roman_number(talk_para2);
			}

			str += " al ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " te ofrece la tecnología de ";
			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += misc.roman_number(talk_para2);
			}
			str += ".";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";
			str += " tu obsequio de tecnología de ";
			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += misc.roman_number(talk_para2);
			}

			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";
			str += " el obsequio de tecnología de ";

			str += tech_res[talk_para1]->tech_des();

			if( talk_para2 )		// Ships do not have different versions
			{
				str += " ";
				str += misc.roman_number(talk_para2);
			}

			str += " del ";
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
	// Envío:
	// El Reino de <King> te exige/solicita la última
	// tecnología de <tech>.
	//
	// Exiges/Solicitas la última tecnología de <tech> al
	// Reino de <King>.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza transmitirte la última tecnología
	// de <tech>.
	//
	// Aceptas/Rechazas transmitir tu tecnología de <tech> al
	// Reino de <King>.
	//
	//---------------------------------------------//

	char* requestStr;
	char* requestStr2;

	if( nation_array[from_nation_recno]->get_relation_status(to_nation_recno)
		 >= NATION_FRIENDLY )
	{
		requestStr = "solicita";
		requestStr2 = "Solicitas";
	}
	else
	{
		requestStr = "exige";
		requestStr2 = "Exiges";
	}

	//------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = requestStr2;
			str += " la última tecnología de ";
			str += tech_res[talk_para1]->tech_des();
			str += " al ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " te ";
			str += requestStr;
			str += " la última tecnología de ";
			str += tech_res[talk_para1]->tech_des();
			str += ".";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";

			str += " transmitirte la última tecnología de ";
			str += tech_res[talk_para1]->tech_des();
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";

			str += " transmitir tu tecnología de ";
			str += tech_res[talk_para1]->tech_des();
			str += " al ";
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
	// Envío:
	// El Reino de <King> te solicita ayuda militar inmediata.
	// Solicitas ayuda militar inmediata al Reino de <King>.
	//
	// Respuesta:
	// El Reino de <King> acepta enviarte inmediatamente la ayuda
	// militar solicitada.
	// El Reino de <King> te niega la ayuda militar solicitada.
	//
	// Aceptas el envío inmediato de ayuda militar al Reino de <King>.
	// Rechazas el envío de ayuda militar al Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Solicitas ayuda militar inmediata al ";
			str +=  to_nation_name();
			str += ".";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " te solicita ayuda militar inmediata.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta enviarte inmediatamente la ayuda militar solicitada.";
			else
				str += " te niega la ayuda militar solicitada.";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas el envío inmediato de ayuda militar al ";
			else
				str = "Rechazas el envío de ayuda militar al ";

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
	// Envío:
	// El Reino de <King> solicita que te unas al embargo comercial del
	// Reino de <King B>.
	//
	// Solicitas al Reino de <King> que se una al embargo comercial del
	// Reino de <King B>.
	//
	// Respuesta:
	// El Reino de <King> acepta/rechaza unirse al embargo comercial
	// del Reino de <King B>.
	//
	// Aceptas/Rechazas unirte al embargo comercial del Reino de <King B>
	// solicitado por el Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Solicitas al ";
			str += to_nation_name();
			str += " que se una ";
		}
		else
		{
			str  = "El ";
			str += from_nation_name();
			str += " solicita que te unas ";
		}

		str += " al embargo comercial del ";
		str += nation_array[talk_para1]->nation_name();
		str += nation_color_code_str(talk_para1);
		str += ".";
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str = "El ";
			str += to_nation_name();

			if( reply_type == REPLY_ACCEPT )
				str += " acepta";
			else
				str += " rechaza";

			str += " unirse al embargo comercial del ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += ".";
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
				str = "Aceptas";
			else
				str = "Rechazas";

			str += " unirte al embargo comercial del ";
			str += nation_array[talk_para1]->nation_name();
			str += nation_color_code_str(talk_para1);
			str += " solicitado por el ";
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
	//
	// Envío:
	//
	// Para unificar nuestros dos Reinos bajo su reinado, el Rey
	// <King name> ofrece <amount> por tu trono.
	//
	// Ofreces <amount> por el trono del Reino de
	// <King>.
	//
	// Respuesta:
	//
	// ¡El Rey <king name> no acepta la deshonra de
	// vender su trono!
	//
	// El Rey <king name> acepta tu dinero a
	// cambio de su trono.
	//
	// No aceptas la deshonra de vender tu
	// trono al Reino de <King>.
	//
	//---------------------------------------------//

	if( reply_type == REPLY_WAITING || !should_disp_reply )
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			str  = "Ofreces ";
			str += talk_para1*10;	// *10 to restore its original value.
			str += " por el trono del ";
			str += to_nation_name();
			str += ".";
		}
		else
		{
			str  = "Para unificar nuestros dos Reinos bajo su reinado, el ";
			str += from_king_name();
			str += " ofrece ";
			str += talk_para1*10;	// *10 to restore its original value.
			str += " por tu trono.";
		}
	}
	else
	{
		if( viewing_nation_recno == from_nation_recno )
		{
			if( reply_type == REPLY_ACCEPT )
			{
				str  = "El ";
				str += to_king_name();
				str += " acepta tu dinero a cambio de su trono.";
			}
			else
			{
				str = "¡El ";
				str += to_king_name();
				str += " no acepta la deshonra de vender su trono!";
			}
		}
		else
		{
			if( reply_type == REPLY_ACCEPT )
			{
				str = "You agree to take the money in exchange for your throne.";
			}
			else
			{
				str  = "No aceptas la deshonra de vender tu trono al ";
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
	//
	// Envío:
	//  (incorrect) El Reino de <King> te ofrece su rendición. 
	// El Reino de <King> se te ha rendido.
	// Te has rendido al Reino de <King>.
	//
	//---------------------------------------------//

	if( viewing_nation_recno == from_nation_recno )
	{
		str  = "Te has rendido al ";
		str += to_nation_name();
		str += ".";
	}
	else
	{
		str  = "El ";
		str += from_nation_name();
		str += " se te ha rendido.";
	}
}
//------- End of function TalkMsg::surrender ------//

#endif
