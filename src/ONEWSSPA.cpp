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

//Filename    : ONEWS3.CPP
//Description : Object news msg generating routines

#if(defined(SPANISH))

#include <OSTR.h>
#include <OTOWN.h>
#include <OFIRMRES.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OSPY.h>
#include <OFONT.h>
#include <OTECHRES.h>
#include <OMONSRES.h>
#include <OREMOTE.h>
#include <OGODRES.h>
#include <ORACERES.h>
#include <OTALKRES.h>
#include <ONEWS.h>

//--------- Define variable type ----------//

typedef void (News::*NewsFP)();

//-------- Define struct NewsInfo -------//

struct NewsInfo
{
	NewsFP news_function_ptr;
	char	 is_major;					// whether this is a major news or not
};

//------ Define function pointers array ------//

static NewsInfo news_info_array[] =
{
	{ &News::diplomacy, 1 },
	{ &News::town_rebel, 1 },
	{ &News::migrate, 0 },
	{ &News::new_nation, 1 },
	{ &News::nation_destroyed, 1 },
	{ &News::nation_surrender, 1 },
	{ &News::king_die, 1 },
	{ &News::new_king, 1 },
	{ &News::firm_destroyed, 0 },
	{ &News::firm_captured, 0 },
	{ &News::town_destroyed, 0 },
	{ &News::town_abandoned, 0 },
	{ &News::town_surrendered, 0 },
	{ &News::monster_king_killed, 0 },
	{ &News::monster_firm_destroyed, 0 },
	{ &News::scroll_acquired, 1 },
	{ &News::monster_gold_acquired, 0 },
	{ &News::your_spy_killed, 1 },
	{ &News::enemy_spy_killed, 1 },
	{ &News::unit_betray, 1 },
	{ &News::unit_assassinated, 1 },
	{ &News::assassinator_caught, 1 }, 
	{ &News::general_die, 1 },
	{ &News::raw_exhaust, 1 },
	{ &News::tech_researched, 1 },
	{ &News::lightning_damage, 1 },
	{ &News::earthquake_damage, 1 },
	{ &News::goal_deadline, 1 },
	{ &News::weapon_ship_worn_out, 0 },
	{ &News::firm_worn_out, 0 },
	{ &News::chat_msg, 1 },
	{ &News::multi_retire, 1 },
	{ &News::multi_quit_game, 1 },
	{ &News::multi_save_game, 1 },
	{ &News::multi_connection_lost, 1 },
};

//------- Define static variables --------//

static String str;

//------ Begin of function News::msg -----//
//
// Return the msg string of current news.
//
char* News::msg()
{
	NewsFP newsFP = news_info_array[id-1].news_function_ptr;

	(this->*newsFP)();   // call the corrsponding function to return the news msg

	return str;
}
//------- End of function News::msg -----//


//------ Begin of function News::is_major -----//
//
// Whether this is a major news or not.
//
int News::is_major()
{
	if( id==NEWS_TOWN_REBEL )
	{
		//-- only rebellions happening in the player's village are considered as major news --//

		return nation_array.player_recno &&
				 nation_name_id1 == (~nation_array)->nation_name_id;
	}
	else
	{
		return news_info_array[id-1].is_major;
	}
}
//------- End of function News::is_major -----//


//------ Begin of function News::diplomacy -----//
//
// Diplomatic messages from other nations.
//
// short_para1 = the recno of TalkMsg in talk_res.talk_msg_array.
//
void News::diplomacy()
{
	err_when( talk_res.is_talk_msg_deleted(short_para1) );

	TalkMsg* talkMsgPtr = talk_res.get_talk_msg(short_para1);

	str = talkMsgPtr->msg_str(nation_array.player_recno);
}
//------- End of function News::diplomacy -----//


//------ Begin of function News::town_rebel -----//
//
// Generate a news that a firm is built
//
// nation_name1() - the name of the nation the rebel took place.
//
// short_para1 = the town name id.
// short_para2 = no. of rebels
//
void News::town_rebel()
{
	//---------------- Text Format -----------------//
	//
	// <no. of rebel> Peasants in <town name> in
	// <nation name> is/are rebelling.
	//
	//----------------------------------------------//
	//
	// <no. of rebel> Campesino(s) de <town name> del
	// <nation name> se está(n) revelando.
	//
	//----------------------------------------------//

	err_when( short_para2 < 1 );

	str = short_para2;

	if( short_para2 > 1 )
		str += " Campesinos de ";
	else
		str += " Campesino de ";

	str += town_res.get_name(short_para1);
	str += " del ";
	str += nation_name1();

	if( short_para2 > 1 )
		str += " se están revelando.";
	else
		str += " se está revelando.";
}
//------- End of function News::town_rebel -----//


//------ Begin of function News::migrate -------//
//
// nation_name1() - from nation
// nation_name2() - to nation
//
// short_para1 = the town name id. that the worker migrates from
// short_para2 = the town name id. that the worker migrates to
// short_para3 = race id. of the migrated worker/peasant
// short_para4 = no. of people migrated
// short_para5 = the firm id. that the worker works for
//
void News::migrate()
{
	//---------------- Text Format -----------------//
	//
	// A <racial> <worker name>/Peasant has emigrated from your
	// village of <town name> to <town name> in <nation name>.
	//
	// A <racial> <worker name>/Peasant has immigrated from
	// <town name> in <nation name> to your village of <town name>.
	//
	//----------------------------------------------//
	//
	// Un <worker name>/Campesino <racial> ha emigrado de tu
	// aldea de <town name> a <town name> del <nation name>.
	//
	// Un <worker name>/Campesino <racial> ha emigrado de
	// <town name> del <nation name> a tu aldea de <town name>.
	//
	//----------------------------------------------//

	if( short_para4 == 1 )
		str = "Un";
	else
		str = misc.format(short_para4);

	str += " ";
	if( short_para5 )
		str += firm_res[short_para5]->worker_title;
	else
		str += "Campesino";

	str += " ";
	str += race_res[short_para3]->name;

	if( short_para4 == 1 )
		str += " ha";
	else
		str += " ha";

	//------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )		// from player nation to another nation
	{
		str += "emigrado de tu aldea de ";
		str += town_res.get_name(short_para1);
		str += " a ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// only if it is not an independent town
		{
			str += " del ";
			str += nation_name2();
		}

		str += ".";
	}
	else
	{
		str += " emigrado de ";
		str += town_res.get_name(short_para1);

		if( nation_name_id1 )
		{
			str += " del ";
			str += nation_name1();
		}

		str += " a tu aldea de ";
		str += town_res.get_name(short_para2);
		str += ".";
	}
}
//------- End of function News::migrate --------//


//------ Begin of function News::new_nation -----//
//
// king_name1() - name of the king of the new kingdom.
//
void News::new_nation()
{
	//---------------- Text Format -----------------//
	//
	// A new Kingdom has emerged under the leadership of <king Name>.
	//
	//----------------------------------------------//
	//
	// Ha surgido un nuevo Reino bajo el mando de <king Name>.
	//
	//----------------------------------------------//

	str  = "Ha surgido un nuevo Reino bajo el mando de ";
	str += king_name1(1);
	str += ".";
}
//------- End of function News::new_nation -----//


//------ Begin of function News::nation_destroyed -----//
//
// nation_name1() - name of the destroyed nation.
//
void News::nation_destroyed()
{
	//---------------- Text Format -----------------//
	//
	// <King>'s Kingdom has been destroyed.
	//
	//----------------------------------------------//
	//
	// El Reino de <King> ha sido destruido.
	//
	//----------------------------------------------//

	str  = "El ";
	str += nation_name1();
	str += " ha sido destruido.";
}
//------- End of function News::nation_destroyed -----//


//------ Begin of function News::nation_surrender -----//
//
// nation_name1() - name of the surrendering nation.
// nation_name2() - name of the nation to surrender.
//
void News::nation_surrender()
{
	//---------------- Text Format -----------------//
	//
	// <King>'s Kingdom has surrendered to <King B>'s Kingdom.
	//
	// <King>'s Kingdom has surrendered to you.
	//
	//----------------------------------------------//
	//
	// El Reino de <King> se ha rendido al Reino de <King B>.
	//
	// El Reino de <King> se te ha rendido.
	//
	//----------------------------------------------//

	str  = "El ";
	str += nation_name1();

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str += " se te ha rendido.";
	}
	else
	{
		str += " se ha rendido al ";
		str += nation_name2();
		str += ".";
	}
}
//------- End of function News::nation_surrender -----//


//------ Begin of function News::king_die -----//
//
// king_name1() - the nation whose king has died.
//
void News::king_die()
{
	//---------------- Text Format -----------------//
	//
	// Your King, <king name>, has been slain.
	//
	// OR
	//
	// King <king name> of <nation name> has been slain.
	//
	//----------------------------------------------//
	//
	// Tu Rey, <king name>, ha sido liquidado.
	//
	// O
	//
	// El Rey <king name> del <nation name> ha sido liquidado.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Tu Rey, ";
		str += king_name1();
		str += ", ha sido liquidado.";
	}
	else
	{
		str  = "El Rey ";
		str += king_name1();
		str += " del ";
		str += nation_name1();
		str += " ha sido liquidado.";
	}
}
//------- End of function News::king_die -----//


//------ Begin of function News::new_king -----//
//
// nation_name1() - name of the nation where there is a new king.
//
// short_para1 - race id. of the new king.
// short_para2 - name id. of the new king.
//
void News::new_king()
{
	//---------------- Text Format -----------------//
	//
	// <unit name> has ascended the throne as your new King.
	//
	// OR
	//
	// <unit name> has ascended the throne as the new King of
	// <nation name>.
	//
	//----------------------------------------------//
	//
	// <unit name> ha subido al trono como tu nuevo Rey.
	//
	// O
	//
	// <unit name> ha subido al trono como nuevo Rey del
	// <nation name>.
	//
	//----------------------------------------------//

	str = race_res[short_para1]->get_name(short_para2);

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str += " ha subido al trono como tu nuevo Rey.";
	}
	else
	{
		str += " ha subido al trono como nuevo Rey del ";
		str += nation_name1();
		str += ".";
	}
}
//------- End of function News::new_king -----//


//------ Begin of function News::firm_destroyed -----//
//
// short_para1 - name id. of the firm destroyed.
// short_para2 - id of the town where the firm is located.
// short_para3 - destroyer type: 1 - a nation, 2 - rebels, 3 - Fryhtans.
//
void News::firm_destroyed()
{
	//---------------- Text Format -----------------//
	//
	// Your <firm type> near <town name> has been destroyed by <kingdom name>.
	// Your <firm type> near <town name> has been destroyed by Rebels.
	// Your <firm type> near <town name> has been destroyed by Fryhtans.
	//
	//----------------------------------------------//
	//
	// El <kingdom name> ha destruido tu <firm type> cerca de <town name>.
	// Los Rebeldes han destruido tu <firm type> cerca de <town name>.
	// Los Fryhtans han destruido tu <firm type> cerca de <town name>.
	//
	//----------------------------------------------//

	str = "";
	switch( short_para3 )
	{
		case DESTROYER_NATION:
			str += "El ";
			str += nation_name2();
			str += " ha";
			break;

		case DESTROYER_REBEL:
			str += "Los Rebeldes han";
			break;

		case DESTROYER_MONSTER:
			str += "Los Fryhtans han";
			break;

		case DESTROYER_UNKNOWN:
			str += "";
			break;
	}

	str += " destruido tu ";
	str += firm_res[short_para1]->name;
	str += " cerca de ";
	str += town_res.get_name(short_para2);
	str += ".";

}
//------- End of function News::firm_destroyed -----//


//------ Begin of function News::firm_captured -----//
//
// nation_name2() - name of the nation that took over your firm.
//
// short_para1 - id. of the firm took over
// short_para2 - id of the town where the firm is located.
// short_para3 - whether the capturer of the firm is a spy.
//
void News::firm_captured()
{
	//---------------- Text Format -----------------//
	//
	// Your <firm type> near <town name> has been
	// captured by <kingdom name>.
	//
	// If the capturer is a spy:
	//
	// Your <firm type> near <town name> has been
	// captured by a spy from <kingdom name>.
	//
	//----------------------------------------------//
	//
	// El <kingdom name> ha capturado tu <firm type>
	// cerca de <town name>.
	//
	// Si el capturador es un espía:
	//
	// Un espía del <kingdom name> ha capturado tu <firm type>
	// cerca de <town name>.
	//
	//----------------------------------------------//

	str = "";
	if( short_para3 )
		str += "Un espía del ";

	else
		str += "El ";

	str += nation_name2();
	str += " ha capturado tu ";
	str += firm_res[short_para1]->name;
	str += " cerca de ";
	str += town_res.get_name(short_para2);
	str += ".";
}
//------- End of function News::firm_captured -----//


//------ Begin of function News::town_destroyed -----//
//
// short_para1 - name id. of the town
// short_para2 - destroyer type
//
void News::town_destroyed()
{
	//---------------- Text Format -----------------//
	//
	// Your village of <name name> has been destroyed by <kingdom name>.
	// Your village of <name name> has been destroyed by Rebels.
	// Your village of <name name> has been destroyed by Fryhtans.
	//
	//----------------------------------------------//
	//
	// El <kingdom name> ha destruido tu aldea de <name name>.
	// Los Rebeldes han destruido tu aldea de <name name>.
	// Los Fryhtans han destruido tu aldea de <name name>.
	//
	//----------------------------------------------//

	str = "";
	switch( short_para2 )
	{
		case DESTROYER_NATION:
			str += "El ";
			str += nation_name2();
			str += " ha";
			break;

		case DESTROYER_REBEL:
			str += "Los Rebeldes han";
			break;

		case DESTROYER_MONSTER:
			str += "Los Fryhtans han";
			break;

		case DESTROYER_UNKNOWN:
			str += "";
			break;
	}

	str += " destruido tu aldea de ";
	str += town_res.get_name(short_para1);
	str += ".";
}
//------- End of function News::town_destroyed -----//


//------ Begin of function News::town_abandoned -----//
//
// short_para1 - name id of the town
//
void News::town_abandoned()
{
	//---------------- Text Format -----------------//
	//
	// Your village of <village name> has been abandoned by
	// its people.
	//
	//----------------------------------------------//
	//
	// Tu aldea de <village name> ha sido abandonada por
	// su gente.
	//
	//----------------------------------------------//

	str  = "Tu aldea de ";
	str += town_res.get_name(short_para1);
	str += " ha sido abandonada por su gente.";
}
//------- End of function News::town_abandoned -----//


//------ Begin of function News::town_surrendered -----//
//
// short_para1 - name id. of the surrendering town.
//
// nation_name1() - name of the nation the town surrenders to.
// nation_name2() - name of the nation of the surrendering town.
//
void News::town_surrendered()
{
	//---------------- Text Format -----------------//
	//
	// The village of <village name> in <King's Kingdom> has
	// surrendered to you.
	//
	// The independent village of <village name> has
	// surrendered to you.
	//
	// Your village of <village name> has surrendered
	// to <King's Kingdom>.
	//
	//----------------------------------------------//
	//
	// La aldea de <village name> del <King's Kingdom> se
	// te ha rendido.
	//
	// La aldea independiente de <village name> se
	// te ha rendido.
	//
	// Tu aldea de <village name> se ha rendido
	// al <King's Kingdom>.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str  = "Tu aldea de ";
		str += town_res.get_name(short_para1);
		str += " se ha rendido al ";
		str += nation_name1();
		str += ".";
	}
	else if( nation_name_id2 )
	{
		str  = "La aldea de ";
		str += town_res.get_name(short_para1);
		str += " del ";
		str += nation_name2();
		str += " se te ha rendido.";
	}
	else // nation_name_id2 == 0, it's an independent town
	{
		str  = "La aldea independiente de ";
		str += town_res.get_name(short_para1);
		str += " se te ha rendido.";
	}
}
//------- End of function News::town_surrendered -----//


//------ Begin of function News::monster_king_killed -----//
//
// short_para1 - monster id.
//
void News::monster_king_killed()
{
	//---------------- Text Format -----------------//
	//
	// An All High <monster type name> has been slain.
	//
	//----------------------------------------------//
	//
	// El Rey <monster type name> ha sido aniquilado.
	//
	//----------------------------------------------//

	str  = "El Rey ";
	str += monster_res[short_para1]->name;
	str += " ha sido aniquilado.";
}
//------- End of function News::monster_king_killed -----//


//------ Begin of function News::monster_firm_destroyed -----//
//
// short_para1 - monster id.
//
void News::monster_firm_destroyed()
{
	//---------------- Text Format -----------------//
	//
	// A <monster type name> Lair has been destroyed.
	//
	//----------------------------------------------//
	//
	// Una Guarida <monster type name> ha sido destruida.
	//
	//----------------------------------------------//

	char* nameStr = monster_res[short_para1]->name;

	str = "Una Guarida ";
	str += nameStr;
	str += " ha sido destruida.";
}
//------- End of function News::monster_firm_destroyed -----//


//------ Begin of function News::scroll_acquired -----//
//
// nation_name1() - the nation that has acquired the scroll.
//
// short_para1 = the race id. of the scroll.
//
void News::scroll_acquired()
{
	//---------------- Text Format -----------------//
	//
	// You have acquired the <race name> Scroll of Power.
	// <nation name> has acquired the <race name> Scroll of Power.
	//
	//----------------------------------------------//
	//
	// Has conseguido el Pergamino del Poder <race name>.
	// El <nation name> ha conseguido el Pergamino del Poder <race name>.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Has";
	}
	else
	{
		str  = "El ";
		str += nation_name1();
		str += " ha";
	}

	str += " conseguido el Pergamino del Poder ";
	str += race_res[short_para1]->adjective;
}
//------- End of function News::scroll_acquired -----//


//------ Begin of function News::monster_gold_acquired -----//
//
// nation_name1() - the nation that has acquired the monster treasure.
//
// short_para1 = amount of gold.
//
void News::monster_gold_acquired()
{
	//---------------- Text Format -----------------//
	//
	// Your have recovered <treasure amount> worth of treasure from the Fryhtans.
	//
	//----------------------------------------------//
	//
	// Has recuperado <treasure amount> del tesoro de los Fryhtans.
	//
	//----------------------------------------------//

	str  = "Has recuperado ";
	str += misc.format(short_para1,2);
	str += " del tesoro de los Fryhtans.";
}
//------- End of function News::monster_gold_acquired -----//


//------ Begin of function News::your_spy_killed -----//
//
// nation_name1() - your nation.
// nation_name2() - the nation that killed your spy.
//
// short_para1 - firm id. if it's a firm
//					  0 if it's a town
//					  race id if spy place == SPY_MOBILE 
// short_para2 - the town id.
//					  name id if spy place == SPY_MOBILE
// short_para3 - spy place
//
void News::your_spy_killed()
{
	//---------------- Text Format -----------------//
	//
	// Your spy has been exposed and executed on his mission to
	// <village name> [ in <nation name> ].  (no nation name for independent town.)
	//
	// Your spy has been exposed and executed on his mission to
	// a <firm type name> near <village name> in <nation name>.
	//
	// Your spy <spy name> has been exposed and executed on his mission to
	// <nation name>.
	//
	//----------------------------------------------//
	//
	// Tu espía ha sido descubierto y ejecutado en
	// <village name> [ del <nation name> ].  (no nation name for independent town.)
	//
	// Tu espía ha sido descubierto y ejecutado en
	// el/la <firm type name> cerca de <village name> del <nation name>.
	//
	// Tu espía <spy name> ha sido descubierto y ejecutado en
	// el <nation name>.
	//
	//----------------------------------------------//

	if( short_para3 == SPY_FIRM )
	{
		str  = "Tu espía ha sido descubierto y ejecutado en la ";
		str += firm_res[short_para1]->name;
		str += " cerca de ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " del ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_TOWN )
	{
		str  = "Tu espía ha sido descubierto y ejecutado en ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " del ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_MOBILE )
	{
		str  = "Tu espía ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " ha sido descubierto y ejecutado";

		if( nation_name_id2 )		// not for independent town.
		{
			str += " en el ";
			str += nation_name2();
		}
	}

	str += ".";
}
//------- End of function News::your_spy_killed -----//


//------ Begin of function News::enemy_spy_killed -----//
//
// nation_name1() - your nation.
// nation_name2() - the nation that the spy belongs to.
//
// short_para1 - firm id. if it's a firm
//					  0 if it's a town
// short_para2 - town name id. if it's a town
//					  0 if it's a firm
// short_para3 - id of the town where the firm is located.
//
void News::enemy_spy_killed()
{
	//---------------- Text Format -----------------//
	//
	// A spy from <kingdom> has been uncovered and executed in your
	// <firm type> near <town name>.
	//
	// A spy from <kingdom> has been uncovered and executed in your
	// village of <town name>.
	//
	// Spy <spy name> from <kingdom> has been uncovered and executed.
	//
	//----------------------------------------------//
	//
	// Un espía del <kingdom> ha sido descubierto y ejecutado en tu
	// <firm type> cerca de <town name>.
	//
	// Un espía del <kingdom> ha sido descubierto y ejecutado en tu
	// aldea de <town name>.
	//
	// El espía <spy name> del <kingdom> ha sido descubierto y ejecutado.
	//
	//----------------------------------------------//

	if( short_para3==SPY_FIRM || short_para3==SPY_TOWN )
	{
		str  = "Un espía del ";
		str += nation_name2();
		str += " ha sido descubierto y ejecutado en tu ";

		if( short_para3==SPY_FIRM )
		{
			str += firm_res[short_para1]->name;
			str += " cerca de ";
			str += town_res.get_name(short_para2);
			str += ".";
		}
		else
		{
			str += "aldea de ";
			str += town_res.get_name(short_para2);
			str += ".";
		}
	}
	else
	{
		err_when( short_para3 != SPY_MOBILE );

		str  = "El espía ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " del ";
		str += nation_name2();
		str += " ha sido descubierto y ejecutado.";
	}
}
//------- End of function News::enemy_spy_killed -----//


//------ Begin of function News::unit_betray -----//
//
// Only for mobile units or generals in command base.
//
// nation_name1() - the nation that the unit originally belongs to.
// nation_name2() - the nation that the unit has turned towards.
//
// short_para1 - race id. of the unit
// short_para2 - name id. of the unit
// short_para3 - rank id. of the unit
//
void News::unit_betray()
{
	//---------------- Text Format -----------------//
	//
	// [General] <unit name> has betrayed you and turned towards
	// <nation name>.
	//
	// [General] <unit name> has renounced you and become independent.
	//
	// [General] <unit name> of <nation name> has defected to your
	// forces.
	//
	// Independent unit <unit name> has joined your forces.
	//
	//----------------------------------------------//
	//
	// [General] <unit name> te ha traicionado y se ha unido
	// al <nation name>.
	//
	// [General] <unit name> te ha abandonado y se ha hecho independiente.
	//
	// [General] <unit name> del <nation name> ha desertado a tu
	// ejército.
	//
	// <unit name>, unidad independiente, se ha unido a tu ejército.
	//
	//----------------------------------------------//

	if( nation_name_id1 == 0 )		// independent unit joining your force
	{
		str  = race_res[short_para1]->get_name(short_para2);
		str += ", unidad independiente, se ha unido a tu ejército.";
		return;
	}

	//------------------------------------//

	str = "";
	if( short_para3==RANK_GENERAL )
		str += "General ";

	str += race_res[short_para1]->get_name(short_para2);

	//---------------------------------//

	if( nation_name_id2 == 0 )		// became an independent unit
	{
		str += " te ha abandonado y se ha hecho independiente.";
	}
	else
	{
		if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
		{
			str += " te ha traicionado y se ha unido al ";
			str += nation_name2();
			str += ".";
		}
		else
		{
			str += " del ";
			str += nation_name1();
			str += " ha desertado a tu ejército.";
		}
	}
}
//------- End of function News::unit_betray -----//


//------ Begin of function News::general_die -----//
//
// short_para1 - race id. of your general
// short_para2 - name id. of your general
//
void News::general_die()
{
	//---------------- Text Format -----------------//
	//
	// Your general, <general name>, has been slain.
	//
	//----------------------------------------------//
	//
	// Tu General, <general name>, ha sido liquidado.
	//
	//----------------------------------------------//

	str  = "Tu General, ";
	str += race_res[short_para1]->get_single_name( static_cast<uint16_t>(short_para2) );
	str += ", ha sido liquidado.";
}
//------- End of function News::general_die -----//


//------ Begin of function News::unit_assassinated -----//
//
// short_para1 - race id. of assassinated unit
// short_para2 - name id. of assassinated unit
// short_para3 - rank id. of assassinated unit
// short_para4 - whether the enemy spy has been killed or not.
//
void News::unit_assassinated()
{
	//---------------- Text Format -----------------//
	//
	// Your King, <king name>, has been assassinated by an enemy spy.
	// Your general, <general name>, has been assassinated by an enemy spy.
	//
	// [The enemy spy has been killed.]
	//
	//----------------------------------------------//
	//
	// Tu Rey, <king name>, ha sido asesinado por un espía enemigo.
	// Tu General, <general name>, ha sido asesinado por un espía enemigo.
	//
	// [El espía enemigo ha sido liquidado.]
	//
	//----------------------------------------------//

	if( short_para3 == RANK_KING )
	{
		str  = "Tu Rey, ";
		str += race_res[short_para1]->get_single_name( static_cast<uint16_t>(short_para2) );
	}
	else
	{
		str  = "Tu General, ";
		str += race_res[short_para1]->get_single_name( static_cast<uint16_t>(short_para2) );
	}

	str += ", ha sido asesinado por un espía enemigo.";

	if( short_para4 )
		str += " El espía enemigo ha sido liquidado.";
}
//------- End of function News::unit_assassinated -----//


//------ Begin of function News::assassinator_caught -----//
//
// short_para1 - rank id. of the assassinating target.
//
void News::assassinator_caught()
{
	//---------------- Text Format -----------------//
	//
	// An enemy spy has been killed while attempting
	// to assassinate your King/General.
	//
	//----------------------------------------------//
	//
	// Un espía enemigo ha sido liquidado mientras intentaba
	// asesinar a tu Rey/General.
	//
	//----------------------------------------------//

	str = "Un espía enemigo ha sido liquidado mientras intentaba asesinar a tu ";

	if( short_para1 == RANK_KING )
		str += "Rey.";
	else
		str += "General.";
}
//------- End of function News::assassinator_caught -----//


//------ Begin of function News::raw_exhaust -----//
//
// short_para1 - raw id.
//
void News::raw_exhaust()
{
	//---------------- Text Format -----------------//
	//
	// Your <raw type> Mine has exhausted its <raw type> deposit.
	//
	//----------------------------------------------//
	//
	// Tu Mina de <raw type> ha agotado sus depósitos de <raw type>.
	//
	//----------------------------------------------//

	str  = "Tu Mina de ";
	str += raw_res[short_para1]->name;
	str += " ha agotado sus depósitos de ";
	str += raw_res[short_para1]->name;
	str += ".";
}
//------- End of function News::raw_exhaust -----//


//------ Begin of function News::tech_researched -----//
//
// short_para1 - tech id.
// short_para2 - tech version.
//
void News::tech_researched()
{
	//---------------- Text Format -----------------//
	//
	// Your scientists have finished their <technology>
	// [Mark <version>] research.
	//
	//----------------------------------------------//
	//
	// Tus científicos han terminado de investigar su <technology>
	// [Marca <version>].
	//
	//----------------------------------------------//

	str  = "Tus científicos han terminado de investigar su ";
	str += tech_res[short_para1]->tech_des();

	if( tech_res[short_para1]->max_tech_level > 1 )		// if the tech has more than one level
	{
		str += " Marca ";
		str += misc.roman_number(short_para2);
	}

	str += ".";
}
//------- End of function News::tech_researched -----//


//------ Begin of function News::lightning_damage -----//
//
void News::lightning_damage()
{
	//---------------- Text Format -----------------//
	//
	// Your <unit name> has been struck and injured/killed by lightning.
	//
	// Your <firm name> near <village name> has been struck/destroyed by lightning.
	//
	// Your village <village name> has been struck/destroyed by lightning.
	//
	//----------------------------------------------//
	//
	// Un rayo ha alcanzado a tu <unit name>, causando heridas/muerte.
	//
	// Un rayo ha alcanzado/destruido tu <firm name> cerca de <village name>.
	//
	// Un rayo ha alcanzado/destruido a tu aldea de <village name>.
	//
	//----------------------------------------------//

	str = "";

	switch( short_para1 )
	{
	case NEWS_LOC_UNIT:
		str += "Un rayo ha alcanzado a tu ";
		if( short_para4 == RANK_GENERAL )
			str += "General ";
		else if( short_para4 == RANK_KING )
			str += "King ";
		else
			str += "unidad ";

		if( short_para2 > 0 )
			str += race_res[short_para2]->get_name(short_para3);
		else
			str += unit_res[short_para3]->name;
		if( short_para5 )
			str += ", causando muerte.";
		else
			str += ", causando heridas.";
		break;

	case NEWS_LOC_FIRM:
	case NEWS_LOC_TOWN:
		str += "Un rayo ha ";
		if( short_para5 )
			str += "destruido";
		else
			str += "alcanzado";
		if( short_para1 == NEWS_LOC_FIRM )
		{
			str += " tu ";
			str += firm_res[short_para2]->name;
			if( short_para3 )
			{
				str += " cerca de ";
				str += town_res.get_name(short_para3);
			}
		}
		else if( short_para1 == NEWS_LOC_TOWN )
		{
			str += " a tu aldea de ";
			str += town_res.get_name(short_para3);
		}
		str += ".";

	default:
		err_here();
	}
}
//------- End of function News::lightning_damage -----//


//------ Begin of function News::earthquake_damage -----//
//
void News::earthquake_damage()
{
	//---------------- Text Format -----------------//
	//
	// <number> of your units has/have been injured and <number> killed
	// in an earthquake.
	//
	// <number> of your villagers has/have been killed in an earthquake.
	//
	// <number> of your buildings has/have been damaged and <number> destroyed
	// in an earthquake.
	//
	//----------------------------------------------//
	//
	// <number> de tus unidades ha/han sido heridas y <number> muertas
	// en un terremoto.
	//
	// <number> de tus aldeanos ha/han muerto en un terremoto.
	//
	// <number> de tus edificios ha/han sido dañados y <number> destruidos
	// en un terremoto.
	//
	//----------------------------------------------//

	int conjunction = 0;
	str = "";

	if( short_para1 == 1)
	{
		str += short_para2;
		str += " de tus unidad";
		if( short_para2 == 1)
			str += " ha";
		else
			str += "es han";
		str += " sido heridas";

		if( short_para3 > 0)
		{
			str += " y ";
			str += short_para3;
			str += " muertas";
		}

		str += " en un terremoto.";
	}
	else if( short_para1 == 2 )
	{
		if( short_para2 > 0)
		{
			str += short_para2;
			str += " de tus aldean";
			if( short_para2 == 1)
				str += " ha";
			else
				str += "os han";
			str += " muerto en un terremoto.";
		}
	}
	else if( short_para1 == 3)
	{
		str += short_para2;
		str += " de tus edificio";
		if( short_para2 == 1)
			str += " ha";
		else
			str += "s han";
		str += " sido dañados ";

		if( short_para3 > 0)
		{
			str += " y ";
			str += short_para3;
			str += " destruidos";
		}
		str += " en un terremoto.";
	}
	else
		err_here();
}
//------- End of function News::earthquake_damage -----//


//------ Begin of function News::goal_deadline -----//
//
// Display a warning message as the deadline of the goals approaches.
//
// short_para1 - years left before the deadline.
// short_para2 - months left before the deadline.
//
void News::goal_deadline()
{
	//---------------- Text Format -----------------//
	//
	// Make haste! You have only <year> year[s] and <month> month[s]
	// left to achieve your goal.
	//
	//----------------------------------------------//
	//
	// ¡Rápido! Sólo tienes <year> año[s] y <month> mes[es]
	// para alcanzar tu objetivo.
	//
	//----------------------------------------------//

	str = "¡Rápido! Sólo tienes ";

	if( short_para1 )
	{
		str += short_para1;

		if( short_para1 > 1 )
			str += " años ";
		else
			str += " año ";
	}

	if( short_para1 && short_para2 )
		str += "y ";

	if( short_para2 )
	{
		str += short_para2;

		if( short_para2 > 1 )
			str += " meses ";
		else
			str += " mes ";
	}

	str += "para alcanzar tu objetivo.";
}
//------- End of function News::goal_deadline -----//


//------ Begin of function News::weapon_ship_worn_out -----//
//
// Your weapon or ship worn out and destroyed due to lack of money for
// maintenance.
//
// short_para1 - unit id. of the weapon
// short_para2 - level of the weapon
//
void News::weapon_ship_worn_out()
{
	//---------------- Text Format -----------------//
	//
	// A <weapon name> <weapon level> of yours has broken
	// down due to the lack of maintenance funds.
	//
	//----------------------------------------------//
	//
	// Una de tus armas, <weapon name> <weapon level>, se ha desmoronado
	// por falta de fondos de mantenimiento.
	//
	//----------------------------------------------//

	str  = "Una de tus armas, ";
	str += unit_res[short_para1]->name;

	if( short_para2 )
	{
		str += " ";
		str += misc.roman_number(short_para2);
	}

	str += ", se ha desmoronado por falta de fondos de mantenimiento.";
}
//------- End of function News::weapon_ship_worn_out -----//


//------ Begin of function News::firm_worn_out -----//
//
// short_para1 - id. of the firm destroyed.
// short_para2 - id of the town where the firm is located.
//
void News::firm_worn_out()
{
	//---------------- Text Format -----------------//
	//
	// Your <firm type> near <town name> has fallen into
	// disrepair due to the lack of maintenance funds.
	//
	//----------------------------------------------//
	//
	// Tu <firm type> cerca de <town name> está en ruinas
	// por falta de fondos de mantenimiento.
	//
	//----------------------------------------------//

	str  = "Tu ";
	str += firm_res[short_para1]->name;
	str += " cerca de ";
	str += town_res.get_name(short_para2);
	str += " está en ruinas por falta de fondos de mantenimiento.";
}
//------- End of function News::firm_worn_out -----//


//------ Begin of function News::chat_msg -----//
//
// short_para1 - id. of the chat msg in Info::remote_chat_str_array[]
//
// nation_name1() - the nation from which this chat message is sent.
//
void News::chat_msg()
{
	str = info.remote_chat_array[short_para1-1].chat_str;
}
//------- End of function News::chat_msg -----//


//------ Begin of function News::multi_retire -----//
//
// This function is called when a human player retires.
//
// nation_name1() - the nation that has retired.
//
void News::multi_retire()
{
	//---------------- Text Format -----------------//
	//
	// <Kingdom name> has retired and quited the game.
	//
	//----------------------------------------------//
	//
	// El <Kingdom name> se ha retirado y abandonado el juego.
	//
	//----------------------------------------------//

	str  = "El ";
	str += nation_name1();
	str += " se ha retirado y abandonado el juego.";
}
//------- End of function News::multi_retire -----//


//------ Begin of function News::multi_quit_game -----//
//
// This function is called when a human player quits the game.
//
void News::multi_quit_game()
{
	//---------------- Text Format -----------------//
	//
	// <Kingdom name> has quited the game.
	//
	//----------------------------------------------//
	//
	// El <Kingdom name> ha abandonado el juego.
	//
	//----------------------------------------------//

	str  = "El ";
	str += nation_name1();
	str += " ha abandonado el juego.";
}
//------- End of function News::multi_quit_game -----//


//------ Begin of function News::multi_save_game -----//
//
// This function is called when a human player calls for saving the game.
//
void News::multi_save_game()
{
	//---------------- Text Format -----------------//
	//
	// The current game has been saved to <file name>.
	//
	//----------------------------------------------//
	//
	// El juego actual se ha guardado en <file name>.
	//
	//----------------------------------------------//

	str  = "El juego actual se ha guardado en ";
	str += remote.save_file_name;
	str += ".";
}
//------- End of function News::multi_save_game -----//


//------ Begin of function News::multi_connection_lost -----//
//
// This function is called when a human player's connection has been lost.
//
void News::multi_connection_lost()
{
	//---------------- Text Format -----------------//
	//
	// The connectino with <kingdom name> has been lost.
	//
	//----------------------------------------------//
	//
	// Se ha perdido la conexión con el <kingdom name>.
	//
	//----------------------------------------------//

	str  = "Se ha perdido la conexión con el ";
	str += nation_name1();
	str += ".";
}
//------- End of function News::multi_connection_lost -----//


//------ Begin of function News::nation_name1 -----//
//
char* News::nation_name1()
{
	static String str;

	str = "Reino de "; 
	if( nation_name_id1 < 0 )		// human player - custom name
		str += nation_array.get_human_name(nation_name_id1, 1);		// 1-first word of the name only
	else
		str += race_res[nation_race_id1]->get_single_name( static_cast<uint16_t>(nation_name_id1) );

	//------ add color bar -------//

	char colorCodeStr[] = " 0";

	colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_color1;

	str += colorCodeStr;

	return str;
}
//------- End of function News::nation_name1 -----//


//------ Begin of function News::nation_name2 -----//
//
char* News::nation_name2()
{
	static String str;

	str = "Reino de ";
	if( nation_name_id2 < 0 )		// human player - custom name
		str += nation_array.get_human_name(nation_name_id2, 1);		// 1-first word of the name only
	else
		str += race_res[nation_race_id2]->get_single_name( static_cast<uint16_t>(nation_name_id2) );

	//------ add color bar -------//

	char colorCodeStr[] = " 0";

	colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_color2;

	str += colorCodeStr;

	return str;
}
//------- End of function News::nation_name2 -----//


//------ Begin of function News::king_name1 -----//
//
// [int] addColor - add color bar at the end of the king name
//						  (default: 0)
//
char* News::king_name1(int addColor)
{
	static String str;

	if( nation_name_id1 < 0 )		// human player - custom name
		str = nation_array.get_human_name(nation_name_id1);
	else
		str = race_res[nation_race_id1]->get_name( static_cast<uint16_t>(nation_name_id1) );

	//------ add color bar -------//

	if( addColor )
	{
		char colorCodeStr[] = " 0";
		colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_color1;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function News::king_name1 -----//


//------ Begin of function News::king_name2 -----//
//
// [int] addColor - add color bar at the end of the king name
//						  (default: 0)
//
char* News::king_name2(int addColor)
{
	static String str;

	if( nation_name_id2 < 0 )		// human player - custom name
		str = nation_array.get_human_name(nation_name_id2);
	else
		str = race_res[nation_race_id2]->get_name( static_cast<uint16_t>(nation_name_id2) );

	//------ add color bar -------//

	if( addColor )
	{
		char colorCodeStr[] = " 0";
		colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_color2;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function News::king_name2 -----//

#endif
