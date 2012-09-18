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

#if( !defined(GERMAN) && !defined(FRENCH) && !defined(SPANISH) )

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

	err_when( short_para2 < 1 );

	str = short_para2;

	if( short_para2 == 1 )
		str += " Peasant in ";
	else
		str += " Peasants in ";

	str += town_res.get_name(short_para1);
	str += " in ";
	str += nation_name1();

	if( short_para2 > 1 )
		str += " are rebelling.";
	else
		str += " is rebelling.";
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

	if( short_para4 == 1 )
		str = "A";
	else
		str = misc.format(short_para4);

	str += " ";
	str += race_res[short_para3]->name;
	str += " ";

	if( short_para5 )
		str += firm_res[short_para5]->worker_title;
	else
		str += "Peasant";

	if( short_para4 == 1 )
		str += " has";
	else
		str += "s have";

	//------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )		// from player nation to another nation
	{
		str += " emigrated from your village of ";
		str += town_res.get_name(short_para1);
		str += " to ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// only if it is not an independent town
		{
			str += " in ";
			str += nation_name2();
		}

		str += ".";
	}
	else
	{
		str += " immigrated from ";
		str += town_res.get_name(short_para1);

		if( nation_name_id1 )
		{
			str += " in ";
			str += nation_name1();
		}

		str += " to your village of ";
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

	str  = "A new Kingdom has emerged under the leadership of ";
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

	str  = nation_name1();
	str += " has been destroyed.";
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

	str  = nation_name1();
	str += " has surrendered to ";

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str += "you.";
	}
	else
	{
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

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Your King, ";
		str += king_name1();
		str += ", has been slain.";
	}
	else
	{
		str  = "King ";
		str += king_name1();
		str += " of ";
		str += nation_name1();
		str += " has been slain.";
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

	str = race_res[short_para1]->get_name(short_para2);

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str += " has ascended the throne as your new King.";
	}
	else
	{
		str += " has ascended the throne as the new King of ";
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

	str  = "Your ";
	str += firm_res[short_para1]->name;
	str += " near ";
	str += town_res.get_name(short_para2);
	str += " has been destroyed";

	// ##### patch begin Gilbert 10/2 ######//
	switch( short_para3 )
	// ##### patch end Gilbert 10/2 ######//
	{
		case DESTROYER_NATION:
			str += " by ";
			str += nation_name2();
			str += ".";
			break;

		case DESTROYER_REBEL:
			str += " by Rebels.";
			break;

		case DESTROYER_MONSTER:
			str += " by Fryhtans.";
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

	str  = "Your ";
	str += firm_res[short_para1]->name;
	str += " near ";
	str += town_res.get_name(short_para2);
	str += " has been captured by ";

	if( short_para3 )
		str += "a spy from ";

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

	str  = "Your village of ";
	str += town_res.get_name(short_para1);
	str += " has been destroyed";

	switch( short_para2 )
	{
		case DESTROYER_NATION:
			str += " by ";
			str += nation_name2();
			str += ".";
			break;

		case DESTROYER_REBEL:
			str += " by Rebels.";
			break;

		case DESTROYER_MONSTER:
			str += " by Fryhtans.";
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

	str  = "Your village of ";
	str += town_res.get_name(short_para1);
	str += " has been abandoned by its people.";
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

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str  = "Your village of ";
		str += town_res.get_name(short_para1);
		str += " has surrendered to ";
		str += nation_name1();
		str += ".";
	}
	else if( nation_name_id2 )
	{
		str  = "The village of ";
		str += town_res.get_name(short_para1);
		str += " in ";
		str += nation_name2();
		str += " has surrendered to you.";
	}
	else // nation_name_id2 == 0, it's an independent town
	{
		str  = "The independent village of ";
		str += town_res.get_name(short_para1);
		str += " has surrendered to you.";
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

	str  = "An All High ";
	str += monster_res[short_para1]->name;
	str += " has been slain.";
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

	char* nameStr = monster_res[short_para1]->name;

	if( nameStr[0] == 'I' )		// "An Ick Lair"
		str = "An ";
	else
		str = "A ";

	str += nameStr;
	str += " Lair has been destroyed.";
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

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "You have acquired the ";
	}
	else
	{
		str  = nation_name1();
		str += " has acquired ";
	}

	str += race_res[short_para1]->adjective;
	str += " Scroll of Power.";
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

	str  = "You have recovered ";
	str += misc.format(short_para1,2);
	str += " worth of treasure from the Fryhtans.";
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

	if( short_para3 == SPY_FIRM )
	{
		str  = "Your spy has been exposed and executed on his mission to a ";
		str += firm_res[short_para1]->name;
		str += " near ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " in ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_TOWN )
	{
		str  = "Your spy has been exposed and executed on his mission to ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " in ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_MOBILE )
	{
		str  = "Your spy ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " has been exposed and executed on his mission";

		if( nation_name_id2 )		// not for independent town.
		{
			str += " to ";
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

	if( short_para3==SPY_FIRM || short_para3==SPY_TOWN )
	{
		str  = "A spy from ";
		str += nation_name2();
		str += " has been uncovered and executed in your ";

		if( short_para3==SPY_FIRM )
		{
			str += firm_res[short_para1]->name;
			str += " near ";
			str += town_res.get_name(short_para2);
			str += ".";
		}
		else
		{
			str += "village of ";
			str += town_res.get_name(short_para2);
			str += ".";
		}
	}
	else
	{
		err_when( short_para3 != SPY_MOBILE );

		str  = "Spy ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " from ";
		str += nation_name2();
		str += " has been uncovered and executed.";
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

	if( nation_name_id1 == 0 )		// independent unit joining your force
	{
		str  = "Independent unit ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " has joined your force.";

		return;
	}

	//------------------------------------//

	if( short_para3==RANK_GENERAL )
		str = "General ";
	else
		str = "";

	str += race_res[short_para1]->get_name(short_para2);

	//---------------------------------//

	if( nation_name_id2 == 0 )		// became an independent unit
	{
		str += " has renounced you and become independent.";
	}
	else
	{
		if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
		{
			str += " has betrayed you and turned towards ";
			str += nation_name2();
			str += ".";
		}
		else
		{
			str += " of ";
			str += nation_name1();
			str += " has defected to your forces.";
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

	str  = "Your general, ";
	str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	str += ", has been slain.";
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

	if( short_para3 == RANK_KING )
	{
		str  = "Your King, ";
		str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	}
	else
	{
		str  = "Your general, ";
		str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	}

	str += ", has been assassinated by an enemy spy.";

	if( short_para4 )
		str += " The enemy spy has been killed.";
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

	str = "An enemy spy has been killed while attempting to assassinate your ";

	if( short_para1 == RANK_KING )
		str += "King.";
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

	str  = "Your ";
	str += raw_res[short_para1]->name;
	str += " Mine has exhausted its ";
	str += raw_res[short_para1]->name;
	str += " deposit.";
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

	str  = "Your scientists have finished their ";
	str += tech_res[short_para1]->tech_des();

	if( tech_res[short_para1]->max_tech_level > 1 )		// if the tech has more than one level
	{
		str += " Mark ";
		str += misc.roman_number(short_para2);
	}

	str += " research.";
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

	str = "Your ";

	switch( short_para1 )
	{
	case NEWS_LOC_UNIT:
		if( short_para4 == RANK_GENERAL )
			str += "General ";
		else if( short_para4 == RANK_KING )
			str += "King ";
		else
			str += "unit ";

		if( short_para2 > 0 )
			str += race_res[short_para2]->get_name((WORD) short_para3);
		else
			str += unit_res[short_para3]->name;
		break;

	case NEWS_LOC_FIRM:
		str += firm_res[short_para2]->name;
		if( short_para3 )
		{
			str += " near ";
			str += town_res.get_name(short_para3);
		}
		break;

	case NEWS_LOC_TOWN:
		str += "village ";
		str += town_res.get_name(short_para3);
		break;

	default:
		err_here();
	}

	//----------------------------------//

	str += " has been ";

	if( short_para1==NEWS_LOC_UNIT )
	{
		if( short_para5 )
			str += "struck and killed ";
		else
			str += "struck and injured ";
	}
	else
	{
		if( short_para5 )
			str += "destroyed ";
		else
			str += "struck ";
	}

	str += "by lightning.";
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

	int conjunction = 0;
	str = "";

	// ###### begin Gilbert 12/9 ##########//
	if( short_para1 == 1)
	{
		str += short_para2;
		str += " of your unit";
		if( short_para2 == 1)
			str += " has";
		else
			str += "s have";
		str += " been injured";

		if( short_para3 > 0)
		{
			str += " and ";
			str += short_para3;
			str += " killed";
		}

		str += " in an earthquake";
	}
	else if( short_para1 == 2 )
	{
		if( short_para2 > 0)
		{
			str += short_para2;
			str += " of your villager";
			if( short_para2 == 1)
				str += " has";
			else
				str += "s have";
			str += " been killed in an earthquake";
		}
	}
	else if( short_para1 == 3)
	{
		str += short_para2;
		str += " of your building";
		if( short_para2 == 1)
			str += " has";
		else
			str += "s have";
		str += " been damaged";

		if( short_para3 > 0)
		{
			str += " and ";
			str += short_para3;
			str += " destroyed";
		}
		str += " in an earthquake.";
	}
	else
		err_here();
	// ###### end Gilbert 12/9 ##########//
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

	str = "Make haste! You have only ";

	if( short_para1 )
	{
		str += short_para1;

		if( short_para1 > 1 )
			str += " years ";
		else
			str += " year ";
	}

	if( short_para1 && short_para2 )
		str += "and ";

	if( short_para2 )
	{
		str += short_para2;

		if( short_para2 > 1 )
			str += " months ";
		else
			str += " month ";
	}

	str += "left to achieve your goal.";
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

	str  = "A ";
	str += unit_res[short_para1]->name;

	if( short_para2 )
	{
		str += " ";
		str += misc.roman_number(short_para2);
	}

	str += " of yours has broken down due to the lack of maintenance funds.";
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

	str  = "Your ";
	str += firm_res[short_para1]->name;
	str += " near ";
	str += town_res.get_name(short_para2);
	str += " has fallen into disrepair due to the lack of maintenance funds.";
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

	str  = nation_name1();
	str += " has retired and quited the game.";
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

	str  = nation_name1();
	str += " has quited the game.";
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

	str  = "The current game has been saved to ";
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

	str  = "The connection with ";
	str += nation_name1();
	str += " has been lost.";
}
//------- End of function News::multi_connection_lost -----//


//------ Begin of function News::nation_name1 -----//
//
char* News::nation_name1()
{
	static String str;

	if( nation_name_id1 < 0 )		// human player - custom name
		str = nation_array.get_human_name(nation_name_id1, 1);		// 1-first word of the name only
	else
		str = race_res[nation_race_id1]->get_single_name( (WORD)nation_name_id1 );

	str += "'s Kingdom";

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

	if( nation_name_id2 < 0 )		// human player - custom name
		str = nation_array.get_human_name(nation_name_id2, 1);		// 1-first word of the name only
	else
		str = race_res[nation_race_id2]->get_single_name( (WORD)nation_name_id2 );

	str += "'s Kingdom";

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
