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

//Filename    : ONEWSFRE.CPP
//Description : Object news msg generating routines, french version

#if(defined(FRENCH))

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
	// <no. of rebel> paysans sont entrés en rébellion à <town name> dans
	// <nation name>.
	//
	//----------------------------------------------//

	err_when( short_para2 < 1 );

	str = short_para2;

	if( short_para2 == 1 )
		str += " paysan est entré en rébellion à ";
	else
		str += " paysans sont entrés en rébellion à ";

	str += town_res.get_name(short_para1);
	str += " dans ";
	str += nation_name1();
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
	// Un <worker name>/paysan <racial> a quitté votre village de <town name> 
	// pour le village de <town name> dans <nation name>.
	//
	// Un <worker name>/paysan <racial> a quitté le village de <town name> 
	// dans <nation name> pour votre village de <town name>.
	//
	//----------------------------------------------//

	if( short_para4 == 1 )
		str = "Un ";
	else
		str = misc.format(short_para4);

	str += " ";

	if( short_para5 )
		str += firm_res[short_para5]->worker_title;
	else
		str += "paysan";

	if( short_para4 > 1 )
		str += "s";

	str += " ";
	str += race_res[short_para3]->name;

	//------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )		// from player nation to another nation
	{
		str += " a quitté votre village de ";
		str += town_res.get_name(short_para1);
		str += " pour le village de ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// only if it is not an independent town
		{
			str += " dans ";
			str += nation_name2();
		}

		str += ".";
	}
	else
	{
		str += " a quitté le village de ";
		str += town_res.get_name(short_para1);

		if( nation_name_id1 )
		{
			str += " dans ";
			str += nation_name1();
		}

		str += " pour votre village de ";
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
	// Un nouveau Royaume est né sous la souveraineté de <king Name>.
	//
	//----------------------------------------------//

	str  = "Un nouveau Royaume est né sous la souveraineté de ";
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
	// Le Royaume de <King> a été anéanti.
	//
	//----------------------------------------------//

	str  = "Le ";
	str += nation_name1();
	str += " a été anéanti.";
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
	// Le Royaume de <King> s'est rendu au Royaume de <King B>.
	//
	// Le Royaume de <King> s'est rendu à vous.
	//
	//----------------------------------------------//

	str  = "Le ";
	str += nation_name1();
	str += " s'est rendu ";

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str += "à vous.";
	}
	else
	{
		str += "au ";
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
	// Votre Roi <king name> est mort.
	//
	// OU
	//
	// Le Roi <king name> de <nation name> est mort.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Votre Roi ";
		str += king_name1();
		str += " est mort.";
	}
	else
	{
		str  = "Le Roi ";
		str += king_name1();
		str += " de ";
		str += nation_name1();
		str += " est mort.";
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
	// Vive votre nouveau Roi <unit name>.
	//
	// OU
	//
	// <unit name> a été couronné Roi de <nation name>.
	//
	//----------------------------------------------//



	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Vive votre nouveau Roi ";
		str += race_res[short_para1]->get_name(short_para2);
		str += ".";
	}
	else
	{
		str = race_res[short_para1]->get_name(short_para2);
		str += " a été couronné Roi de ";
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
	// Votre <firm type> près de <town name> a été détruit(e) par
	// <kingdom name>.
	//
	// Votre <firm type> près de <town name> a été détruit(e) par des
	// rebelles.
	//
	// Votre <firm type> près de <town name> a été détuit(e) par des Fryhtans.
	//
	//----------------------------------------------//

	str  = "Votre ";
	str += firm_res[short_para1]->name;
	str += " près de ";
	str += town_res.get_name(short_para2);
	str += " a été détruit";

	switch( short_para2 )
	{
		case DESTROYER_NATION:
			str += " par ";
			str += nation_name2();
			str += ".";
			break;

		case DESTROYER_REBEL:
			str += " par des rebelles.";
			break;

		case DESTROYER_MONSTER:
			str += " par des Fryhtans.";
			break;

		case DESTROYER_UNKNOWN:
			str += ".";
			break;
	}
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
	// Votre <firm type> près de <town name> a été
	// capturé(e) par <kingdom name>.
	//
	// Si l'auteur de la capture est un espion:
	//
	// Votre <firm type> près de <town name> a été
	// capturé(e) par un espion de <kingdom name>.
	//
	//----------------------------------------------//

	str  = "Votre ";
	str += firm_res[short_para1]->name;
	str += " près de ";
	str += town_res.get_name(short_para2);
	str += " a été capturé par ";

	if( short_para3 )
		str += "un espion de ";

	str += nation_name2();
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
	// Votre village de <name name> a été détruit par <kingdom name>.
	// Votre village de <name name> a été détruit par des rebelles.
	// Votre village de <name name> a été détruit par des Fryhtans.
	//
	//----------------------------------------------//

	str  = "Votre village de ";
	str += town_res.get_name(short_para1);
	str += " a été détruit";

	switch( short_para2 )
	{
		case DESTROYER_NATION:
			str += " par ";
			str += nation_name2();
			str += ".";
			break;

		case DESTROYER_REBEL:
			str += " par des rebelles.";
			break;

		case DESTROYER_MONSTER:
			str += " par des Fryhtans.";
			break;

		case DESTROYER_UNKNOWN:
			str += ".";
			break;
	}
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
	// Votre village de <village name> a été déserté par
	// tous ses habitants.
	//
	//----------------------------------------------//

	str  = "Votre village de ";
	str += town_res.get_name(short_para1);
	str += " a été déserté par tous ses habitants.";
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
	// Le village de <village name> dans <King's Kingdom> s'est soumis
	// à vous.
	//
	// Le village indépendant de <village name> s'est soumis
	// à vous.
	//
	// Votre village de <village name> s'est soumis
	// à <King's Kingdom>.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str  = "Votre village de ";
		str += town_res.get_name(short_para1);
		str += " s'est soumis à ";
		str += nation_name1();
		str += ".";
	}
	else if( nation_name_id2 )
	{
		str  = "Le village de ";
		str += town_res.get_name(short_para1);
		str += " dans ";
		str += nation_name2();
		str += " s'est soumis à vous.";
	}
	else // nation_name_id2 == 0, it's an independent town
	{
		str  = "Le village indépendant de ";
		str += town_res.get_name(short_para1);
		str += " s'est soumis à vous.";
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
	// Le Roi des <monster type name> est mort.
	//
	//----------------------------------------------//

	str  = "Le Roi des ";
	str += monster_res[short_para1]->name;
	str += " est mort.";
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
	// Un antre <monster type name> a été détruit.
	//
	//----------------------------------------------//

	char* nameStr = monster_res[short_para1]->name;

	str  = "Un antre ";
	str += nameStr;
	str += " a été détruit.";
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
	// Vous avez pris possession du Parchemin du Pouvoir <race name>.
	// <nation name> a pris possession du Parchemin du Pouvoir <race name>. 
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Vous avez pris possession du Parchemin du Pouvoir ";
	}
	else
	{
		str  = nation_name1();
		str += " a pris possession du Parchemin du Pouvoir ";
	}

	str += race_res[short_para1]->adjective;
	str += ".";
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
	// Vous vous êtes emparé d'un trésor Fryhtan d'une valeur de
	// <treasure amount>.
	//
	//----------------------------------------------//

	str  = "Vous vous êtes emparé d'un trésor Fryhtan d'une valeur de";
	str += misc.format(short_para1,2);
	str += ".";
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
	// Votre espion a été découvert et exécuté en mission à
	// <village name> [ dans le <nation name> ].  (pas de nation name pour des
	// villages indépendants)
	//
	// Votre espion a été découvert et exécuté en mission dans une 
	// <firm type name> près de <village name> dans le <nation name>.
	//
	// Votre espion <spy name> a été découvert et exécuté en mission dans le
	// <nation name>.
	//
	//----------------------------------------------//


	if( short_para3 == SPY_FIRM )
	{
		str  = "Votre espion a été découvert et exécuté en mission dans une ";
		str += firm_res[short_para1]->name;
		str += " près de ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " dans le ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_TOWN )
	{
		str  = "Votre espion a été découvert et exécuté en mission à ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " dans le ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_MOBILE )
	{
		str  = "Votre espion ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " a été découvert et exécuté en mission";

		if( nation_name_id2 )		// not for independent town.
		{
			str += " dans le ";
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
	// Un espion de <kingdom> a été découvert et exécuté dans votre
	// <firm type> près de <town name>.
	//
	// Un espion de <kingdom> a été découvert et exécuté dans votre village de
	// <town name>.
	//
	// L'espion <spy name> de <kingdom> a été découvert et exécuté.
	//
	//----------------------------------------------//

	if( short_para3==SPY_FIRM || short_para3==SPY_TOWN )
	{
		str  = "Un espion de ";
		str += nation_name2();
		str += " a été découvert et exécuté dans votre ";

		if( short_para3==SPY_FIRM )
		{
			str += firm_res[short_para1]->name;
			str += " près de ";
			str += town_res.get_name(short_para2);
			str += ".";
		}
		else
		{
			str += "village de ";
			str += town_res.get_name(short_para2);
			str += ".";
		}
	}
	else
	{
		err_when( short_para3 != SPY_MOBILE );

		str  = "L'espion ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " de ";
		str += nation_name2();
		str += " a été découvert et exécuté.";
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
	// [Général] <unit name> vous a trahi et a prêté allégeance à
	// <nation name>.
	//
	// [Général] <unit name> vous a quitté pour devenir indépendant.
	//
	// [Général] <unit name> de <nation name> a fait défection en votre
	// faveur.
	//
	// L'unité indépendante <unit name> vous a prêté allégeance.
	//
	//----------------------------------------------//

	if( nation_name_id1 == 0 )		// independent unit joining your force
	{
		str  = "L'unité indépendante ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " vous a prêté allégeance.";

		return;
	}

	//------------------------------------//

	if( short_para3==RANK_GENERAL )
		str = "Général ";
	else
		str = "";

	str += race_res[short_para1]->get_name(short_para2);

	//---------------------------------//

	if( nation_name_id2 == 0 )		// became an independent unit
	{
		str += " vous a quitté pour devenir indépendant.";
	}
	else
	{
		if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
		{
			str += " vous a trahi et a prêté allégeance à ";
			str += nation_name2();
			str += ".";
		}
		else
		{
			str += " de ";
			str += nation_name1();
			str += " a fait défection en votre faveur.";
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
	// Votre général <general name> est mort.
	//
	//----------------------------------------------//


	str  = "Votre général ";
	str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	str += " est mort.";
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
	// Votre Roi <king name> a été assassiné par un espion ennemi.
	// Votre général <general name> a été assassiné par un espion ennemi.
	//
	// [L'espion ennemi a été tué.]
	//
	//----------------------------------------------//

	if( short_para3 == RANK_KING )
	{
		str  = "Votre Roi ";
		str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	}
	else
	{
		str  = "Votre général ";
		str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	}

	str += " a été assassiné par un espion ennemi.";

	if( short_para4 )
		str += " L'espion ennemi a été tué.";
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
	// Un espion ennemi a été tué alors qu'il tentait
	// d'assassiner votre Roi/Général.
	//
	//----------------------------------------------//

	str = "Un espion ennemi a été tué alors qu'il tentait d'assassiner votre ";

	if( short_para1 == RANK_KING )
		str += "Roi.";
	else
		str += "Général.";
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
	// Votre mine de <raw type> est épuisée.
	//
	//----------------------------------------------//

	str  = "Votre mine de ";
	str += raw_res[short_para1]->name;
	str += " est épuisée.";
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
	// Vos chercheurs ont fait une découverte : <technology>
	// [Version <version>].
	//
	//----------------------------------------------//


	str  = "Vos chercheurs ont fait une découverte : ";
	str += tech_res[short_para1]->tech_des();

	if( tech_res[short_para1]->max_tech_level > 1 )		// if the tech has more than one level
	{
		str += " Version ";
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
	// Votre <unit name> a été frappé(e) et blessé(e)/tué(e) par la foudre.
	//
	// Votre <firm name> près de <village name> a été frappé(e)/détruit(e) par
	// la foudre.
	//
	// Votre village de <village name> a été frappé/détruit par la foudre.
	//
	//----------------------------------------------//

	str = "Votre ";

	switch( short_para1 )
	{
	case NEWS_LOC_UNIT:
		if( short_para4 == RANK_GENERAL )
			str += "Général ";
		else if( short_para4 == RANK_KING )
			str += "Roi ";
		else
			str += "unités ";

		if( short_para2 > 0 )
			str += race_res[short_para2]->get_name((WORD) short_para3);
		else
			str += unit_res[short_para3]->name;
		break;

	case NEWS_LOC_FIRM:
		str += firm_res[short_para2]->name;
		if( short_para3 )
		{
			str += " près de ";
			str += town_res.get_name(short_para3);
		}
		break;

	case NEWS_LOC_TOWN:
		str += "village de ";
		str += town_res.get_name(short_para3);
		break;

	default:
		err_here();
	}

	//----------------------------------//

	if( short_para1==NEWS_LOC_UNIT )
	{
		if( short_para5 )
			str += "a été frappé et tué par la foudre.";
		else
			str += "a été frappé et blessé par la foudre.";
	}
	else
	{
		if( short_para5 )
			str += "a été détruit par la foudre.";
		else
			str += "a été détruit par la foudre.";
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
	// <number> de vos unités ont été blessées et <number> tuées
	// dans un tremblement de terre.
	//
	// <number> de vos villageois ont été tués dans un tremblement de terre.
	//
	// <number> de vos bâtiments ont été endommagés et <number> détruits
	// dans un tremblement de terre.
	//
	//----------------------------------------------//

	int conjunction = 0;
	str = "";

	if( short_para1 == 1)
	{
		str += short_para2;
		str += " de vos unités ont été blessées";

		if( short_para3 > 0)
		{
			str += " et ";
			str += short_para3;
			str += " tuées";
		}

		str += " dans un tremblement de terre.";
	}
	else if( short_para1 == 2 )
	{
		if( short_para2 > 0)
		{
			str += short_para2;
			str += " de vos villageois ont été tués dans un tremblement de terre.";
		}
	}
	else if( short_para1 == 3)
	{
		str += short_para2;
		str += " de vos bâtiments ont été endommagés";

		if( short_para3 > 0)
		{
			str += " et ";
			str += short_para3;
			str += " détruits";
		}
		str += " dans un tremblement de terre.";
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
	// Hâtez-vous! Il ne vous reste que <year> an[s] et <month> mois
	// pour atteindre votre objectif.
	//
	//----------------------------------------------//


	str = "Hâtez-vous! Il ne vous reste que ";

	if( short_para1 )
	{
		str += short_para1;

		if( short_para1 > 1 )
			str += " ans ";
		else
			str += " an ";
	}

	if( short_para1 && short_para2 )
		str += "et ";

	if( short_para2 )
	{
		str += short_para2;
		str += " mois ";
	}

	str += "pour atteindre votre objectif.";
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
	// Un(e) de vos <weapon name> <weapon level> est inutilisable en raison du
	// manque de fonds pour la maintenance.
	//
	//----------------------------------------------//


	str  = "Un de vos ";
	str += unit_res[short_para1]->name;

	if( short_para2 )
	{
		str += " ";
		str += misc.roman_number(short_para2);
	}

	str += " est inutilisable en raison du manque de fonds pour la maintenance.";
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
	// Votre <firm type> près de <town name> n'est plus en état de marche en
	// raison du manque de fonds pour la maintenance.
	//
	//----------------------------------------------//

	str  = "Votre ";
	str += firm_res[short_para1]->name;
	str += " près de ";
	str += town_res.get_name(short_para2);
	str += " n'est plus en état de marche en raison du manque de fonds pour la maintenance.";
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
	// <Kingdom name> s'est retiré et a abandonné la partie.
	//
	//----------------------------------------------//

	str  = nation_name1();
	str += " s'est retiré et a abandonné la partie.";
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
	// <Kingdom name> a abandonné la partie.
	//
	//----------------------------------------------//

	str  = nation_name1();
	str += " a abandonné la partie.";
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
	// La partie en cours a été enregistrée sous le nom <file name>.
	//
	//----------------------------------------------//

	str  = "La partie en cours a été enregistrée sous le nom ";
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
	// La connexion avec <kingdom name> s'est interrompue.
	//
	//----------------------------------------------//


	str  = "La connexion avec ";
	str += nation_name1();
	str += " s'est interrompue.";
}
//------- End of function News::multi_connection_lost -----//


//------ Begin of function News::nation_name1 -----//
//
char* News::nation_name1()
{
	static String str;

	str = "Royaume de "; 
	if( nation_name_id1 < 0 )		// human player - custom name
		str += nation_array.get_human_name(nation_name_id1, 1);		// 1-first word of the name only
	else
		str += race_res[nation_race_id1]->get_single_name( (WORD)nation_name_id1 );

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

	str = "Royaume de ";
	if( nation_name_id2 < 0 )		// human player - custom name
		str += nation_array.get_human_name(nation_name_id2, 1);		// 1-first word of the name only
	else
		str += race_res[nation_race_id2]->get_single_name( (WORD)nation_name_id2 );

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
		str = race_res[nation_race_id1]->get_name( (WORD)nation_name_id1 );

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
		str = race_res[nation_race_id2]->get_name( (WORD)nation_name_id2 );

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
