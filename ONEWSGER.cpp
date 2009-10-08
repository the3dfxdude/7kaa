//Filename    : ONEWS3.CPP
//Description : Object news msg generating routines

#ifdef GERMAN

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

static char firm_gender(short firmId)
{
	switch(firmId)
	{
	case FIRM_FACTORY:
	case FIRM_CAMP:
	case FIRM_MINE:
	case FIRM_WAR_FACTORY:
		return 1;
	}

	return 0;
}

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
	// <no. of rebel> Bauern aus <town name> in
	// <nation name> rebellieren.			// No more 'is/are', just only: 'rebellieren'.
	//
	//----------------------------------------------//

	err_when( short_para2 < 1 );

	str = short_para2;

	str += " Bauern aus ";

	str += town_res.get_name(short_para1);
	str += " in ";
	str += nation_name1();

	str += " rebellieren.";
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
	// Der <racial> <worker name>/Bauer übersiedelte aus Ihrem 
	// Dorf <town name> nach <town name> in <nation name>.
	//
	// Der <racial> <worker name>/Bauer übersiedelte aus 
	// <town name> in <nation name> in Ihr Dorf <town name>.
	//
	//----------------------------------------------//

	// BUGHERE : Is <n> <racial> correct?
	if( short_para4 == 1 )
		str = "Der";
	else
		str = m.format(short_para4);

	str += " ";
	str += race_res[short_para3]->name;
	str += " ";

	if( short_para5 )
		str += firm_res[short_para5]->worker_title;
	else
		str += "Bauer";

	str += " übersiedelte aus ";

	//------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )		// from player nation to another nation
	{
		str += "Ihrem Dorf ";
		str += town_res.get_name(short_para1);
		str += " nach ";
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
		str += town_res.get_name(short_para1);

		if( nation_name_id1 )
		{
			str += " in ";
			str += nation_name1();
		}

		str += " in Ihr Dorf ";
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
	// Ein neues Königreich unter der Führung von <king Name> 
	// wurde ausgerufen.
	//
	//----------------------------------------------//

	str  = "Ein neues Königreich unter der Führung von ";
	str += king_name1(1);
	str += " wurde ausgerufen.";
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
	// <King>'s Königreich wurde vernichtet.
	//
	//----------------------------------------------//

	str  = nation_name1();
	str += " wurde vernichtet.";
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
	// <King>'s Königreich kapitulierte vor <King B>'s Königreich.
	//
	// <King>'s Königreich kapitulierte vor Ihnen.
	//
	//----------------------------------------------//

	str  = nation_name1();
	str += " kapitulierte vor ";

	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str += "Ihnen.";
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
	//
	// Ihr König <king name> wurde getötet.
	//
	// OR
	//
	// König <king name> von <nation name> wurde getötet.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Ihr König ";
		str += king_name1();
		str += " wurde getötet.";
	}
	else
	{
		str  = "König ";
		str += king_name1();
		str += " von ";
		str += nation_name1();
		str += " wurde getötet.";
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
	// <unit name> bestieg als Ihr neuer König den Thron.
	//
	// OR
	//
	// <unit name> bestieg als neuer König von
	// <nation name> den Thron.
	//
	//----------------------------------------------//

	str = race_res[short_para1]->get_name(short_para2);

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str += " bestieg als Ihr neuer König den Thron.";
	}
	else
	{
		str += " bestieg als neuer König von ";
		str += nation_name1();
		str += " den Thron.";
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
	// Ihr(e) <firm type> nahe <town name> wurde von <kingdom name> zerstört.
	// Ihr(e) <firm type> nahe <town name> wurde von Rebellen zerstört.
	// Ihr(e) <firm type> nahe <town name> wurde von Morghouls zerstört.
	//
	//----------------------------------------------//

	str  = firm_gender(short_para1) ? "Ihre " : "Ihr ";
	str += firm_res[short_para1]->name;
	str += " nahe ";
	str += town_res.get_name(short_para2);
	str += " wurde";

	switch( short_para2 )
	{
		case DESTROYER_NATION:
			str += " von ";
			str += nation_name2();
			break;

		case DESTROYER_REBEL:
			str += " von Rebellen";
			break;

		case DESTROYER_MONSTER:
			str += " von Morghouls";
			break;

		case DESTROYER_UNKNOWN:
			break;
	}
	str += " zerstört.";
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
	// Ihr/e <firm type> nahe <town name> wurde von
	// <kingdom name> erobert.
	//
	// If the capturer is a spy:               //  Falls der Eroberer ein Spion ist: (need to be  translated?)
	//
	// Ihr/e <firm type> nahe <town name> wurde von
	// einem Spion aus <kingdom name> übernommen.
	//
	//----------------------------------------------//


	str  = firm_gender(short_para1) ? "Ihre " : "Ihr ";
	str += firm_res[short_para1]->name;
	str += " nahe ";
	str += town_res.get_name(short_para2);
	str += " wurde von ";

	if( short_para3 )
	{
		str += "einem Spion aus ";
		str += nation_name2();
		str += " übernommen.";
	}
	else
	{
		str += nation_name2();
		str += " erobert.";
	}
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
	// Ihr Dorf <name name> wurde von <kingdom name> zerstört.
	// Ihr Dorf <name name> wurde von Rebellen zerstört.
	// Ihr Dorf <name name> wurde von Morghouls zerstört.
	//
	//----------------------------------------------//

	str  = "Ihr Dorf ";
	str += town_res.get_name(short_para1);
	str += " wurde";

	switch( short_para2 )
	{
		case DESTROYER_NATION:
			str += " von ";
			str += nation_name2();
			break;

		case DESTROYER_REBEL:
			str += " von Rebellen";
			break;

		case DESTROYER_MONSTER:
			str += " von Morghouls";
			break;

		case DESTROYER_UNKNOWN:
			break;
	}
	str += " zerstört.";
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
	// Ihr Dorf <village name> wurde von seinen Bewohnern
	// verlassen.
	//
	//----------------------------------------------//

	str  = "Ihr Dort ";
	str += town_res.get_name(short_para1);
	str += "  wurde von seinen Bewohnern verlassen.";
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
	// Das Dorf <village name> in <King's Kingdom> hat
	// vor Ihnen kapituliert.
	//
	// Das unabhängige Dorf <village name> hat vor Ihnen
	// kapituliert.
	//
	// Ihr Dorf <village name> hat vor <King's Kingdom>
	// kapituliert.
	//
	//----------------------------------------------//


	if( nation_array.player_recno && nation_name_id2 == (~nation_array)->nation_name_id )
	{
		str  = "Ihr Dorf ";
		str += town_res.get_name(short_para1);
		str += " hat vor ";
		str += nation_name1();
		str += " kapituliert.";
	}
	else if( nation_name_id2 )
	{
		str  = "Das Dorf ";
		str += town_res.get_name(short_para1);
		str += " in ";
		str += nation_name2();
		str += " hat vor Ihnen kapituliert.";
	}
	else // nation_name_id2 == 0, it's an independent town
	{
		str  = "Das unabhängige Dorf ";
		str += town_res.get_name(short_para1);
		str += " hat vor Ihnen kapituliert.";
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
	// Der <monster type name> König wurde getötet.
	//
	//----------------------------------------------//


	str  = "Der ";
	str += monster_res[short_para1]->name;
	str += " König wurde getötet.";
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
	// Ein <monster type name> Nest wurde zerstört.
	//
	//----------------------------------------------//

	char* nameStr = monster_res[short_para1]->name;

	str = "Ein ";
	str += nameStr;
	str += " Nest wurde zerstört.";
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
	// Sie haben die <race name> Schriftrolle der Macht erlangt.
	// <nation name> hat die <race name> Schriftrolle der Macht erlangt.
	//
	//----------------------------------------------//

	if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
	{
		str  = "Sie haben die ";
	}
	else
	{
		str  = nation_name1();
		str += " hat die ";
	}

	str += race_res[short_para1]->adjective;
	str += " Schriftrolle der Macht erlangt.";
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
	// Sie haben einen Schatz in Höhe von <treasure amount> von den Morghouls erobert.
	//
	//----------------------------------------------//

	str  = "Sie haben einen Schatz in Höhe von ";
	str += m.format(short_para1,2);
	str += " von den Morghouls erobert.";
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
	// Ihr Spion wurde in <village name> [ in <nation name> ].  
	// ertappt und hingerichtet. (no nation name for independent town.)    // <<<  ... your own remark!
	//
	// Ihr Spion wurde in <firm type name> nahe <village name> in 
	// <nation name> ertappt und hingerichtet.
	//
	// Ihr Spion <spy name> wurde auf seiner Mission in <nation name>
	// ertappt und hingerichtet.
	//
	//----------------------------------------------//

	if( short_para3 == SPY_FIRM )
	{
		str  = "Ihr Spion wurde in ";
		str += firm_res[short_para1]->name;
		str += " nahe ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " in ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_TOWN )
	{
		str  = "Ihr Spion wurde in ";
		str += town_res.get_name(short_para2);

		if( nation_name_id2 )		// not for independent town.
		{
			str += " in ";
			str += nation_name2();
		}
	}
	else if( short_para3 == SPY_MOBILE )
	{
		str  = "Ihr Spion ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " wurde auf seiner Mission";

		if( nation_name_id2 )		// not for independent town.
		{
			str += " to ";
			str += nation_name2();
		}
	}

	str += " ertappt und hingerichtet.";
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
	// Ein Spion aus <kingdom> wurde entlarvt und in 
	// <firm type> nahe <town name> hingerichtet.
	//
	// Ein Spion aus <kingdom> wurde in Ihrem Dorf <town name>
	// entlarvt und hingerichtet. Friede seiner Asche.
	//
	// Spion <spy name> aus <kingdom> wurde entlarvt und hingerichtet.
	//
	//----------------------------------------------//

	if( short_para3==SPY_FIRM || short_para3==SPY_TOWN )
	{
		str  = "Ein Spion aus ";
		str += nation_name2();
		str += " wurde";

		if( short_para3==SPY_FIRM )
		{
			str += " entlarvt und in ";
			str += firm_res[short_para1]->name;
			str += " nahe ";
			str += town_res.get_name(short_para2);
			str += " hingerichtet.";
		}
		else
		{
			str += " in Ihrem Dorf ";
			str += town_res.get_name(short_para2);
			str += " entlarvt und hingerichtet. Friede seiner Asche.";
		}
	}
	else
	{
		err_when( short_para3 != SPY_MOBILE );

		str  = "Spion ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " aus ";
		str += nation_name2();
		str += " wurde entlarvt und hingerichtet.";
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
	// [General] <unit name> hat Sie verraten und flüchtete nach
	// <nation name>.
	//
	// [General] <unit name> hat Sie verlassen und wurde zum
	// Freibürger.
	//
	// [General] <unit name> aus <nation name> ist zu Ihrer Armee
	// übergelaufen.
	//
	// Der Freibürger <unit name> ist Ihrer Armee beigetreten.
	//
	//----------------------------------------------//

	// BUGHERE : General is German?
	if( nation_name_id1 == 0 )		// independent unit joining your force
	{
		str  = "Der Freibürger ";
		str += race_res[short_para1]->get_name(short_para2);
		str += " ist Ihrer Armee beigetreten.";

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
		str += " hat Sie verlassen und wurde zum Freibürger.";
	}
	else
	{
		if( nation_array.player_recno && nation_name_id1 == (~nation_array)->nation_name_id )
		{
			str += " hat Sie verraten und flüchtete nach ";
			str += nation_name2();
			str += ".";
		}
		else
		{
			str += " aus ";
			str += nation_name1();
			str += " ist zu Ihrer Armee übergelaufen.";
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
	// Ihr General <general name> wurde getötet. Friede seiner Asche.
	//
	//----------------------------------------------//

	str  = "Ihr General ";
	str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	str += " wurde getötet. Friede seiner Asche.";
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
	// Ihr König <king name> wurde von einem feindlichen Spion ermordet.
	// Ihr General <general name> wurde von einem feindlichen Spion ermordet.
	//
	// [Der feindliche Spion wurde liquidiert.]
	//
	//----------------------------------------------//

	if( short_para3 == RANK_KING )
	{
		str  = "Ihr König ";
		str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	}
	else
	{
		str  = "Ihr General ";
		str += race_res[short_para1]->get_single_name( (WORD)short_para2 );
	}

	str += " wurde von einem feindlichen Spion ermordet.";

	if( short_para4 )
		str += " Der feindliche Spion wurde liquidiert.";
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
	// Ein feindlicher Spion wurde beim Attentatsversuch 
	// auf Ihren König/General getötet.
	//
	//----------------------------------------------//

	str = "Ein feindlicher Spion wurde beim Attentatsversuch auf Ihren ";

	if( short_para1 == RANK_KING )
		str += "König getötet.";
	else
		str += "General getötet.";
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
	// Der <raw type>-Vorrat Ihrer <raw type>-Mine ist erschöpft.
	//
	//----------------------------------------------//

	str  = "Der ";
	str += raw_res[short_para1]->name;
	str += "-Vorrat Ihrer ";
	str += raw_res[short_para1]->name;
	str += "-Mine ist erschöpft.";
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
	// Ihre Forscher beendeten erfolgreich die <technology>
	// [Mark <version>] -Erforschung.
	//
	//----------------------------------------------//

	str  = "Ihre Forscher beendeten erfolgreich die ";
	str += tech_res[short_para1]->tech_des();

	if( tech_res[short_para1]->max_tech_level > 1 )		// if the tech has more than one level
	{
		str += " Mark ";
		str += m.roman_number(short_para2);
	}

	str += " -Erforschung.";
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
	// Ihre Einheit <unit name> wurde vom Blitz getroffen und verletzt/getötet.
	//
	// Ihr(e) <firm name> nahe <village name> wurde vom Blitz getroffen/zerstört.
	//
	// Ihr Dorf <village name> wurde vom Blitz getroffen/zerstört.
	//
	//----------------------------------------------//

//	str = "Your ";

	switch( short_para1 )
	{
	case NEWS_LOC_UNIT:
		// BUGHERE : German translation may not take care of General/King case
		str = "Ihre Einheit ";
		if( short_para4 == RANK_GENERAL )
			str += "General ";
		else if( short_para4 == RANK_KING )
			str += "König ";
		else
			str += "";

		if( short_para2 > 0 )
			str += race_res[short_para2]->get_name((WORD) short_para3);
		else
			str += unit_res[short_para3]->name;

		str += "wurde vom Blitz getroffen und ";
		if( short_para5 )
			str += "getötet.";
		else
			str += "verletzt";
		break;

	case NEWS_LOC_FIRM:
		str  = firm_gender(short_para2) ? "Ihre " : "Ihr ";
		str += firm_res[short_para2]->name;
		if( short_para3 )
		{
			str += " nahe ";
			str += town_res.get_name(short_para3);
		}
		str += " wurde vom Blitz ";
		if( short_para5 )
			str += "getroffen.";
		else
			str += "zerstört.";
		break;

	case NEWS_LOC_TOWN:
		str = "Ihr Dorf ";
		str += town_res.get_name(short_para3);
		str += " wurde vom Blitz ";
		if( short_para5 )
			str += "getroffen.";
		else
			str += "zerstört.";
		break;

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
	// Bei einem Erdbeben wurde/wurden <number> Ihrer Einheiten verletzt und 
	// <number> getötet.		                            // no more 'has/have' in the second time!
	//
	// <number> Ihrer Dorfbewohner wurde/wurden bei einem Erdbeben getötet.
	//
	// Bei einem Erdbeben wurde/wurden <number> Ihrer Gebäude beschädigt 
	// und <number> zerstört.
	//
	//----------------------------------------------//

	int conjunction = 0;
	str = "";

	if( short_para1 == 1)
	{
		str += "Bei einem Erdbeben";
		if( short_para2 <= 1)
			str += " wurde ";
		else
			str += " wurden ";
		str += short_para2;
		str += " Ihrer Einheiten verletzt";

		if( short_para3 > 0)
		{
			str += " und ";
			str += short_para3;
			str += " getötet";
		}
		str += ".";
	}
	else if( short_para1 == 2 )
	{
		if( short_para2 > 0)
		{
			str += short_para2;
			str += " Ihrer Dorfbewohner";
			if( short_para2 <= 1)
				str += " wurde ";
			else
				str += " wurden ";
			str += "bei einem Erdbeben getötet.";
		}
	}
	else if( short_para1 == 3)
	{
		str += "Bei einem Erdbeben ";
		if( short_para2 == 1)
			str += " wurde ";
		else
			str += " wurden ";
		str += short_para2;
		str += " Ihrer Gebäude beschädigt";

		if( short_para3 > 0)
		{
			str += " und ";
			str += short_para3;
			str += " zerstört";
		}
		str += ".";
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
	// Sputen Sie sich! Sie haben nur noch <year> Jahr(e) und <month> Monat(e)
	// übrig, um Ihr Ziel zu erreichen!
	//
	//----------------------------------------------//

	str = "Sputen Sie sich! Sie haben nur noch ";

	if( short_para1 )
	{
		str += short_para1;

		if( short_para1 > 1 )
			str += " Jahre";
		else
			str += " Jahr";
	}

	if( short_para1 && short_para2 )
		str += " und ";

	if( short_para2 )
	{
		str += short_para2;

		if( short_para2 > 1 )
			str += " Monate";
		else
			str += " Monat";
	}

	str += " übrig, um Ihr Ziel zu erreichen!";
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
	// Ihre <weapon name> <weapon level> ist wegen Mangel an 
	// Instandhaltungs-Budget kaputt gegangen.
	//
	//----------------------------------------------//

	str  = "Ihre ";
	str += unit_res[short_para1]->name;

	if( short_para2 )
	{
		str += " ";
		str += m.roman_number(short_para2);
	}

	str += " ist wegen Mangel an Instandhaltungs-Budget kaputt gegangen.";
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
	// Ihr(e) <firm type> nahe <town name> ist wegen Mangel an
	// Instandhaltungs-Budget außer Betrieb.
	//
	//----------------------------------------------//

	str  = firm_gender(short_para1) ? "Ihre " : "Ihr ";
	str += firm_res[short_para1]->name;
	str += " nahe ";
	str += town_res.get_name(short_para2);
	str += " ist wegen Mangel an Instandhaltungs-Budget außer Betrieb.";
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
	// <Kingdom name> hat aufgegeben und das Spiel verlassen.
	//
	//----------------------------------------------//

	str  = nation_name1();
	str += " hat aufgegeben und das Spiel verlassen.";
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
	// <Kingdom name> hat das Spiel verlassen.
	//
	//----------------------------------------------//

	str  = nation_name1();
	str += " hat das Spiel verlassen.";
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
	// Das aktuelle Spiel wurde unter <file name> gespeichert.
	//
	//----------------------------------------------//

	str  = "Das aktuelle Spiel wurde unter ";
	str += remote.save_file_name;
	str += " gespeichert.";
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
	// Die Verbindung zu <kingdom name> ist verlorengegangen.
	//
	//----------------------------------------------//

	str  = "Die Verbindung zu ";
	str += nation_name1();
	str += " ist verlorengegangen.";
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

	str += "'s Königreich";

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

	str += "'s Königreich";

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
