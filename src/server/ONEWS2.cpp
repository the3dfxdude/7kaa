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

//Filename    : ONEWS2.CPP
//Description : News adding functions

#include <OTOWN.h>
#include <ONATION.h>
#include <OMONSRES.h>
#include <OTALKRES.h>
#include <OFIRM.h>
#include <OSPY.h>
#include <OUNIT.h>
#include <ONEWS.h>


//------ Begin of function NewsArray::diplomacy -----//
//
// <int> talkMsgRecno = the recno of the TalkMsg in talk_res.talk_msg_array
//
// short_para1 = the recno of the TalkMsg in talk_res.talk_msg_array
//
void NewsArray::diplomacy(int talkMsgRecno)
{
	//----------- add news --------------//

	err_when( talk_res.is_talk_msg_deleted(talkMsgRecno) );

	TalkMsg* talkMsgPtr = talk_res.get_talk_msg(talkMsgRecno);

	News* newsPtr = add_news( NEWS_DIPLOMACY, NEWS_NORMAL, talkMsgPtr->from_nation_recno, talkMsgPtr->to_nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = talkMsgRecno;
}
//------- End of function NewsArray::diplomacy -----//


//------ Begin of function NewsArray::town_rebel -----//
//
// <int> townRecno  = the recno of the town where people there rebel
// <int> rebelCount = no. of rebels
//
// short_para1 = the town name id.
// short_para2 = no. of rebels
//
void NewsArray::town_rebel(int townRecno, int rebelCount)
{
	Town* townPtr = town_array[townRecno];

	//----------- add news --------------//

	News* newsPtr = add_news( NEWS_TOWN_REBEL, NEWS_NORMAL, townPtr->nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = townPtr->town_name_id;
	newsPtr->short_para2 = rebelCount;

	//-------- set location ----------//

	newsPtr->set_loc( townPtr->center_x, townPtr->center_y, NEWS_LOC_TOWN, townRecno );
}
//------- End of function NewsArray::town_rebel -----//


//------ Begin of function NewsArray::migrate -----//
//
// <int> srcTownRecno = the town that the worker migrates from
// <int> desTownRecno = the town that the worker migrates to
// <int> raceId		 = race id. of the migrated worker/peasant
// <int> migratedCount= no. of people migrated.
// [int] firmRecno    = if firm worker migrates,
//					  			>this is the firm id. that the worker works for
//               			if town peasant migrates
//					  			>this is 0
//
// short_para1 = the town name id. that the worker migrates from
// short_para2 = the town name id. that the worker migrates to
// short_para3 = race id. of the migrated worker/peasant
// short_para4 = no. of people migrated
// short_para5 = the firm id. that the worker works for
//
void NewsArray::migrate(int srcTownRecno, int desTownRecno, int raceId, int migratedCount, int firmRecno)
{
	err_when( srcTownRecno==0 || desTownRecno==0 || raceId==0 || migratedCount==0 );

	Town* srcTown = town_array[srcTownRecno];
	Town* desTown = town_array[desTownRecno];

	//----------- add news --------------//

	News* newsPtr = add_news( NEWS_MIGRATE, NEWS_NORMAL, srcTown->nation_recno, desTown->nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = srcTown->town_name_id;
	newsPtr->short_para2 = desTown->town_name_id;
	newsPtr->short_para3 = raceId;
	newsPtr->short_para4 = migratedCount;

	if( firmRecno )
		newsPtr->short_para5 = firm_array[firmRecno]->firm_id;
	else
		newsPtr->short_para5 = 0;

	//-------- set location ----------//

	newsPtr->set_loc( desTown->center_x, desTown->center_y, NEWS_LOC_TOWN, desTownRecno );
}
//------- End of function NewsArray::migrate -----//


//------ Begin of function NewsArray::new_nation -----//
//
// <int> nationRecno - recno of the new nation.
//
// king_name1() - name of the king of the new kingdom.
//
void NewsArray::new_nation(int nationRecno)
{
	News* newsPtr = add_news( NEWS_NEW_NATION, NEWS_NORMAL, nationRecno );

	if( !newsPtr )
		return;

	//---- set the news location to one of its town ----//

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		Town* townPtr = town_array[i];

		if( townPtr->nation_recno == nationRecno )
		{
			newsPtr->set_loc( townPtr->center_x, townPtr->center_y,
									NEWS_LOC_TOWN, i );
			break;
		}
	}
}
//------- End of function NewsArray::new_nation -----//


//------ Begin of function NewsArray::nation_destroyed -----//
//
// <int> nationRecno - recno of the nation that has been destroyed
//
// nation_name1() - the nation that has been destroyed.
//
void NewsArray::nation_destroyed(int nationRecno)
{
	add_news( NEWS_NATION_DESTROYED, NEWS_NORMAL, nationRecno );
}
//------- End of function NewsArray::nation_destroyed -----//


//------ Begin of function NewsArray::nation_surrender -----//
//
// <int> nationRecno   - recno of the surrendering nation.
// <int> toNationRecno - recno of the nation to surrender.
//
void NewsArray::nation_surrender(int nationRecno, int toNationRecno)
{
	add_news( NEWS_NATION_SURRENDER, NEWS_NORMAL, nationRecno, toNationRecno );
}
//------- End of function NewsArray::nation_surrender -----//


//------ Begin of function NewsArray::king_die -----//
//
// <int> nationRecno - recno of the nation with its king died.
//
// king_name1() - the nation whose king has died.
//
void NewsArray::king_die(int nationRecno)
{
	add_news( NEWS_KING_DIE, NEWS_NORMAL, nationRecno );
}
//------- End of function NewsArray::king_die -----//


//------ Begin of function NewsArray::new_king -----//
//
// <int> nationRecno   - recno of the nation
// <int> kingUnitRecno - unit recno of the new king.
//
// This function should be called before set_king() has been called.
//
// nation_name1() - name of the nation where there is a new king.
//
// short_para1 - race id. of the new king.
// short_para2 - name id. of the new king.
//
void NewsArray::new_king(int nationRecno, int kingUnitRecno)
{
	News* newsPtr = add_news( NEWS_NEW_KING, NEWS_NORMAL, nationRecno );

	if( !newsPtr )
		return;

	Unit* unitPtr = unit_array[kingUnitRecno];

	newsPtr->short_para1 = unitPtr->race_id;
	newsPtr->short_para2 = unitPtr->name_id;
}
//------- End of function NewsArray::new_king -----//


//------ Begin of function NewsArray::firm_destroyed -----//
//
// <int>   firmRecno  - recno of the firm destroyed.
// <Unit*> attackUnit - recno to the attacking unit.
//
// short_para1 - id. of the firm destroyed.
// short_para2 - name id of the town where the firm is located.
// short_para3 - destroyer type: 1 - a nation, 2 - rebels, 3 - Fryhtans.
//
void NewsArray::firm_destroyed(int firmRecno, Unit* attackUnit)
{
	Firm* firmPtr = firm_array[firmRecno];

	err_when( firmPtr->nation_recno != nation_array.player_recno );

	int destroyerNationRecno=0;

	if( attackUnit )
		destroyerNationRecno = attackUnit->nation_recno;

	News* newsPtr = add_news( NEWS_FIRM_DESTROYED, NEWS_NORMAL, firmPtr->nation_recno, destroyerNationRecno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = firmPtr->firm_id;

	if( firmPtr->closest_town_name_id )
		newsPtr->short_para2 = firmPtr->closest_town_name_id;
	else
		newsPtr->short_para2 = firmPtr->get_closest_town_name_id();

	//-------- set destroyer type ------//

	newsPtr->short_para3 = DESTROYER_UNKNOWN;

	if( attackUnit )
	{
		if( attackUnit->unit_mode == UNIT_MODE_REBEL )
			newsPtr->short_para3 = DESTROYER_REBEL;

		else if( unit_res[attackUnit->unit_id]->unit_class == UNIT_CLASS_MONSTER )
			newsPtr->short_para3 = DESTROYER_MONSTER;

		else if( attackUnit->nation_recno )
			newsPtr->short_para3 = DESTROYER_NATION;
	}

	//--------- set location ---------//

	newsPtr->set_loc( firmPtr->center_x, firmPtr->center_y, NEWS_LOC_ANY );
}
//------- End of function NewsArray::firm_destroyed -----//


//------ Begin of function NewsArray::firm_captured -----//
//
// <int> firmRecno 	  - recno of the firm destroyed.
// <int> takeoverNationRecno - recno of nation that has taken over the firm.
// <int> spyTakeover	  - whether the capturer of the firm is a spy.
//
// short_para1 - id. of the firm destroyed.
// short_para2 - name id of the town where the firm is located.
// short_para3 - whether the capturer of the firm is a spy.
//
void NewsArray::firm_captured(int firmRecno, int takeoverNationRecno, int spyTakeover)
{
	Firm* firmPtr = firm_array[firmRecno];

	err_when( firmPtr->nation_recno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_FIRM_CAPTURED, NEWS_NORMAL, firmPtr->nation_recno, takeoverNationRecno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = firmPtr->firm_id;

	if( firmPtr->closest_town_name_id )
		newsPtr->short_para2 = firmPtr->closest_town_name_id;
	else
		newsPtr->short_para2 = firmPtr->get_closest_town_name_id();

	newsPtr->short_para3 = spyTakeover;

	//--------- set location ---------//

	newsPtr->set_loc( firmPtr->center_x, firmPtr->center_y, NEWS_LOC_FIRM, firmRecno );
}
//------- End of function NewsArray::firm_captured -----//


//------ Begin of function NewsArray::town_destroyed -----//
//
// <int> townNameId	 - name id. of the town destroyed.
// <int> xLoc, yLoc   - location of the town
// <Unit*> attackUnit - recno to the attacking unit.
//
// short_para1 - name id. of the town destroyed.
// short_para2 - destroyer type: 1 - a nation, 2 - rebels, 3 - Fryhtans.
//
void NewsArray::town_destroyed(int townNameId, int xLoc, int yLoc, Unit* attackUnit)
{
	int destroyerNationRecno=0;

	if( attackUnit )
		destroyerNationRecno = attackUnit->nation_recno;

	News* newsPtr = add_news( NEWS_TOWN_DESTROYED, NEWS_NORMAL, nation_array.player_recno, destroyerNationRecno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = townNameId;

	//-------- set destroyer type ------//

	newsPtr->short_para2 = DESTROYER_UNKNOWN;

	if( attackUnit )
	{
		if( attackUnit->unit_mode == UNIT_MODE_REBEL )
			newsPtr->short_para2 = DESTROYER_REBEL;

		else if( unit_res[attackUnit->unit_id]->unit_class == UNIT_CLASS_MONSTER )
			newsPtr->short_para2 = DESTROYER_MONSTER;

		else if( attackUnit->nation_recno )
			newsPtr->short_para2 = DESTROYER_NATION;
	}

	//-------- set location ----------//

	newsPtr->set_loc( xLoc, yLoc, NEWS_LOC_ANY );
}
//------- End of function NewsArray::town_destroyed -----//


//------ Begin of function NewsArray::town_abandoned -----//
//
// <int> townRecno - recno of the town destroyed.
//
// short_para1 - name id. of the town destroyed.
//
void NewsArray::town_abandoned(int townRecno)
{
	Town* townPtr = town_array[townRecno];

	err_when( townPtr->nation_recno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_TOWN_ABANDONED, NEWS_NORMAL, townPtr->nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = townPtr->town_name_id;

	//-------- set location ----------//

	newsPtr->set_loc( townPtr->center_x, townPtr->center_y, NEWS_LOC_ANY );
}
//------- End of function NewsArray::town_abandoned -----//


//------ Begin of function NewsArray::town_surrendered -----//
//
// <int> townRecno 	  - recno of the town
// <int> toNationRecno - recno of the nation this town surrenders to
//
// short_para1 - name id. of the surrendering town.
//
// nation_name1() - name of the nation the town surrenders to.
// nation_name2() - name of the nation of the surrendering town.
//
// This function should be called before the town surrenders.
//
void NewsArray::town_surrendered(int townRecno, int toNationRecno)
{
	Town* townPtr = town_array[townRecno];

	err_when( townPtr->nation_recno != nation_array.player_recno &&
				 toNationRecno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_TOWN_SURRENDERED, NEWS_NORMAL, toNationRecno, townPtr->nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = townPtr->town_name_id;

	//-------- set location ----------//

	newsPtr->set_loc( townPtr->center_x, townPtr->center_y, NEWS_LOC_TOWN, townRecno );
}
//------- End of function NewsArray::town_surrendered -----//


//------ Begin of function NewsArray::monster_king_killed -----//
//
// <int> monsterId - monster id. of the monster king.
//
// short_para1 - monster id.
//
void NewsArray::monster_king_killed(int monsterId, int xLoc, int yLoc)
{
	err_when( monsterId < 1 || monsterId > monster_res.monster_count );

	News* newsPtr = add_news( NEWS_MONSTER_KING_KILLED, NEWS_NORMAL );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = monsterId;

	//-------- set location ----------//

	newsPtr->set_loc( xLoc, yLoc, NEWS_LOC_ANY );
}
//------- End of function NewsArray::monster_king_killed -----//


//------ Begin of function NewsArray::monster_firm_destroyed -----//
//
// <int> monsterId - monster id. of the monster king.
//
// short_para1 - monster id.
//
void NewsArray::monster_firm_destroyed(int monsterId, int xLoc, int yLoc)
{
	err_when( monsterId < 1 || monsterId > monster_res.monster_count );

	News* newsPtr = add_news( NEWS_MONSTER_FIRM_DESTROYED, NEWS_NORMAL );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = monsterId;

	//-------- set location ----------//

	newsPtr->set_loc( xLoc, yLoc, NEWS_LOC_ANY );
}
//------- End of function NewsArray::monster_firm_destroyed -----//


//------ Begin of function NewsArray::scroll_acquired -----//
//
// <int> acquireNationRecno - recno of the nation that has acquired the scroll
// <int> scrollRaceId	    - race of the scroll
//
// nation_name1() - the nation that has acquired the scroll.
//
// short_para1 = the race id. of the scroll.
//
void NewsArray::scroll_acquired(int acquireNationRecno, int scrollRaceId)
{
	News* newsPtr = add_news( NEWS_SCROLL_ACQUIRED, NEWS_NORMAL, acquireNationRecno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = scrollRaceId;
}
//------- End of function NewsArray::scroll_acquired -----//


//------ Begin of function NewsArray::monster_gold_acquired -----//
//
// When the player recovered treasures from monsters.
//
// <int> goldAmt - the amount of treasure recovered.
//
// short_para1 = amount of gold.
//
void NewsArray::monster_gold_acquired(int goldAmt)
{
	News* newsPtr = add_news( NEWS_MONSTER_GOLD_ACQUIRED, NEWS_NORMAL, nation_array.player_recno );

	if( !newsPtr )
		return;

	newsPtr->short_para1 = goldAmt;
}
//------- End of function NewsArray::monster_gold_acquired -----//


//------ Begin of function NewsArray::spy_killed -----//
//
// <int> spyRecno - recno of the spy.
//
// nation_name1() - your nation.
// nation_name2() - the nation that killed your spy.
//
// short_para1 - firm id. if it's a firm
//					  0 if it's a town
// short_para2 - the town id.
// short_para3 - spy place
//
// This function should be called just right before the spy is killed.
//
void NewsArray::spy_killed(int spyRecno)
{
	Spy*  spyPtr = spy_array[spyRecno];
	News* newsPtr;

	//---------- your spy is killed in an enemy nation ---------//

	if( spyPtr->true_nation_recno == nation_array.player_recno )
	{
		newsPtr = add_news( NEWS_YOUR_SPY_KILLED, NEWS_NORMAL,
					 nation_array.player_recno, spyPtr->cloaked_nation_recno );
	}
	else //----- an enemy spy in your nation is uncovered and executed ----//
	{
		err_when( spyPtr->cloaked_nation_recno != nation_array.player_recno );

		newsPtr = add_news( NEWS_ENEMY_SPY_KILLED, NEWS_NORMAL,
					 nation_array.player_recno, spyPtr->true_nation_recno );
	}

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	//-------------------------------------------//

	newsPtr->short_para3 = spyPtr->spy_place;

	if( spyPtr->spy_place == SPY_FIRM )
	{
		Firm* firmPtr = firm_array[spyPtr->spy_place_para];

		newsPtr->short_para1 = firmPtr->firm_id;
		newsPtr->short_para2 = firmPtr->get_closest_town_name_id();

		newsPtr->set_loc( firmPtr->center_x, firmPtr->center_y, NEWS_LOC_FIRM, firmPtr->firm_recno );
	}
	else if( spyPtr->spy_place == SPY_TOWN )
	{
		Town* townPtr = town_array[spyPtr->spy_place_para];

		newsPtr->short_para1 = 0;
		newsPtr->short_para2 = townPtr->town_name_id;

		newsPtr->set_loc( townPtr->center_x, townPtr->center_y, NEWS_LOC_TOWN, townPtr->town_recno );
	}
	else if( spyPtr->spy_place == SPY_MOBILE )
	{
		Unit* unitPtr = unit_array[spyPtr->spy_place_para];

		newsPtr->short_para1 = unitPtr->race_id;
		newsPtr->short_para2 = unitPtr->name_id;
	}
}
//------- End of function NewsArray::spy_killed -----//


//------ Begin of function NewsArray::unit_betray -----//
//
// <int> unitRecno - recno of the unit.
// <int> betrayToNationRecno - the nation which the unit betray towards
//
// nation_name1() - the nation that the unit originally belongs to.
// nation_name2() - the nation that the unit has turned towards.
//
// short_para1 - race id. of the unit
// short_para2 - name id. of the unit
// short_para3 - rank id. of the unit
//
// This function should be called before the unit betray.
//
void NewsArray::unit_betray(int unitRecno, int betrayToNationRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->nation_recno != nation_array.player_recno &&
				 betrayToNationRecno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_UNIT_BETRAY, NEWS_NORMAL, unitPtr->nation_recno, betrayToNationRecno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = unitPtr->race_id;
	newsPtr->short_para2 = unitPtr->name_id;
	newsPtr->short_para3 = unitPtr->rank_id;

	//------- set location --------//

	if( betrayToNationRecno == nation_array.player_recno )
		newsPtr->set_loc( unitPtr->next_x_loc(), unitPtr->next_y_loc(), NEWS_LOC_UNIT, unitRecno, unitPtr->name_id );
	else
		newsPtr->set_loc( unitPtr->next_x_loc(), unitPtr->next_y_loc(), NEWS_LOC_ANY );
}
//------- End of function NewsArray::unit_betray -----//


//------ Begin of function NewsArray::unit_assassinated -----//
//
// <int> unitRecno - recno of the unit that has been assassinated.
// <int> spyKilled - whether the enemy spy has been killed during his assissination mission.
//
// short_para1 - race id. of the assassinated unit
// short_para2 - name id. of the assassinated unit
// short_para3 - rank id. of the assassinated unit
// short_para4 - whether the enemy spy has been killed or not.
//
void NewsArray::unit_assassinated(int unitRecno, int spyKilled)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->nation_recno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_UNIT_ASSASSINATED, NEWS_NORMAL, unitPtr->nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = unitPtr->race_id;
	newsPtr->short_para2 = unitPtr->name_id;
	newsPtr->short_para3 = unitPtr->rank_id;
	newsPtr->short_para4 = spyKilled;

	//------- set location --------//

	short xLoc, yLoc;

	unitPtr->get_cur_loc(xLoc, yLoc);

	newsPtr->set_loc( xLoc, yLoc, NEWS_LOC_ANY );
}
//------- End of function NewsArray::unit_assassinated -----//


//------ Begin of function NewsArray::assassinator_caught -----//
//
// <int> spyRecno 	 - spy recno of the assassinator.
// <int> targetRankId - the rank id. of the assassinating target
//
// short_para1 - rank id. of the assassinating target.
//
void NewsArray::assassinator_caught(int spyRecno, int targetRankId)
{
	News* newsPtr = add_news( NEWS_ASSASSINATOR_CAUGHT, NEWS_NORMAL );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = targetRankId;

	//------- set location --------//

	int xLoc, yLoc;

	spy_array[spyRecno]->get_loc(xLoc, yLoc);

	newsPtr->set_loc( xLoc, yLoc, NEWS_LOC_ANY );
}
//------- End of function NewsArray::assassinator_caught -----//


//------ Begin of function NewsArray::general_die -----//
//
// <int> unitRecno - recno of the unit.
//
// short_para1 - race id. of your general
// short_para2 - name id. of your general
//
void NewsArray::general_die(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->nation_recno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_GENERAL_DIE, NEWS_NORMAL, unitPtr->nation_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = unitPtr->race_id;
	newsPtr->short_para2 = unitPtr->name_id;

	//------- set location --------//

	newsPtr->set_loc( unitPtr->next_x_loc(), unitPtr->next_y_loc(), NEWS_LOC_ANY );
}
//------- End of function NewsArray::general_die -----//


//------ Begin of function NewsArray::raw_exhaust -----//
//
// short_para1 - raw id.
//
void NewsArray::raw_exhaust(int rawId, int xLoc, int yLoc)
{
	News* newsPtr = add_news( NEWS_RAW_EXHAUST, NEWS_NORMAL );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = rawId;

	//------- set location --------//

	newsPtr->set_loc( xLoc, yLoc, NEWS_LOC_ANY );
}
//------- End of function NewsArray::raw_exhaust -----//


//------ Begin of function NewsArray::tech_researched -----//
//
// short_para1 - tech id.
// short_para2 - tech version.
//
void NewsArray::tech_researched(int techId, int techVersion)
{
	News* newsPtr = add_news( NEWS_TECH_RESEARCHED, NEWS_NORMAL, nation_array.player_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = techId;
	newsPtr->short_para2 = techVersion;
}
//------- End of function NewsArray::tech_researched -----//


//------ Begin of function NewsArray::lightning_damage -----//
//
void NewsArray::lightning_damage(int xLoc, int yLoc, int objectId, int recno, int objectDie)
{
	News* newsPtr = add_news( NEWS_LIGHTNING_DAMAGE, NEWS_NORMAL );

	if( !newsPtr )
		return;

	newsPtr->set_loc( xLoc, yLoc, objectId, recno);

	newsPtr->short_para1 = objectId;
	newsPtr->short_para2 = 0;
	newsPtr->short_para3 = 0;
	newsPtr->short_para4 = 0;
	switch( objectId )
	{
	case NEWS_LOC_UNIT:
		newsPtr->short_para4 = unit_array[recno]->rank_id;
		if( (newsPtr->short_para2 = unit_array[recno]->race_id) > 0)
			newsPtr->short_para3 = (short) unit_array[recno]->name_id;
		else
			newsPtr->short_para3 = unit_array[recno]->unit_id;
		break;
	case NEWS_LOC_FIRM:
		newsPtr->short_para2 = firm_array[recno]->firm_id;
		newsPtr->short_para3 = firm_array[recno]->closest_town_name_id;
		break;
	case NEWS_LOC_TOWN:
		newsPtr->short_para3 = town_array[recno]->town_name_id;
		break;
	default:
		err_here();
	}
	newsPtr->short_para5 = objectDie;
}
//------- End of function NewsArray::lightning_damage -----//


//------ Begin of function NewsArray::earthquake_damage -----//
//
void NewsArray::earthquake_damage(int unitDamage, int unitDie, int townDamage, int firmDamage, int firmDie)
{
	News* newsPtr;
	// ######## begin Gilbert 12/9 #######//
	if( unitDamage > 0 || unitDie > 0)
	{
		newsPtr = add_news( NEWS_EARTHQUAKE_DAMAGE, NEWS_NORMAL );
		if( newsPtr )
		{
			newsPtr->short_para1 = 1;
			newsPtr->short_para2 = unitDamage;
			newsPtr->short_para3 = unitDie;
		}
	}
	if( townDamage > 0)
	{
		newsPtr = add_news( NEWS_EARTHQUAKE_DAMAGE, NEWS_NORMAL );
		if( newsPtr )
		{
			newsPtr->short_para1 = 2;
			newsPtr->short_para2 = townDamage;
		}
	}
	if( firmDamage > 0 || firmDie > 0)
	{
		newsPtr = add_news( NEWS_EARTHQUAKE_DAMAGE, NEWS_NORMAL );
		if( newsPtr )
		{
			newsPtr->short_para1 = 3;
			newsPtr->short_para2 = firmDamage;
			newsPtr->short_para3 = firmDie;
		}
	}
	// ######## end Gilbert 12/9 #######//
}
//------- End of function NewsArray::earthquake_damage -----//


//------ Begin of function NewsArray::goal_deadline -----//
//
// Display a warning message as the deadline of the goals approaches.
//
// short_para1 - years left before the deadline.
// short_para2 - months left before the deadline.
//
void NewsArray::goal_deadline(int yearLeft, int monthLeft)
{
	News* newsPtr = add_news( NEWS_GOAL_DEADLINE, NEWS_NORMAL, nation_array.player_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = yearLeft;
	newsPtr->short_para2 = monthLeft;
}
//------- End of function NewsArray::goal_deadline -----//


//------ Begin of function NewsArray::weapon_ship_worn_out -----//
//
// Your weapon worn out and destroyed due to lack of money for
// maintenance.
//
// short_para1 - unit id. of the weapon
// short_para2 - level of the weapon
//
void NewsArray::weapon_ship_worn_out(int unitId, int weaponLevel)
{
	err_when( unit_res[unitId]->unit_class != UNIT_CLASS_WEAPON &&
				 unit_res[unitId]->unit_class != UNIT_CLASS_SHIP );

	News* newsPtr = add_news( NEWS_WEAPON_SHIP_WORN_OUT, NEWS_NORMAL, nation_array.player_recno );

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = unitId;
	newsPtr->short_para2 = weaponLevel;
}
//------- End of function NewsArray::weapon_ship_worn_out -----//


//------ Begin of function NewsArray::firm_worn_out -----//
//
// <int> firmRecno - recno of the firm destroyed.
//
// short_para1 - id. of the firm destroyed.
// short_para2 - name id of the town where the firm is located.
//
void NewsArray::firm_worn_out(int firmRecno)
{
	Firm* firmPtr = firm_array[firmRecno];

	err_when( firmPtr->nation_recno != nation_array.player_recno );

	News* newsPtr = add_news( NEWS_FIRM_WORN_OUT, NEWS_NORMAL, firmPtr->nation_recno);

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = firmPtr->firm_id;

	if( firmPtr->closest_town_name_id )
		newsPtr->short_para2 = firmPtr->closest_town_name_id;
	else
		newsPtr->short_para2 = firmPtr->get_closest_town_name_id();
}
//------- End of function NewsArray::firm_worn_out -----//


//------ Begin of function NewsArray::chat_msg -----//
//
// <int>   fromNationRecno - recno of the nation from which this chat message is sent.
// <char*> chatStr     	   - pointer to the chat string.
//
// short_para1 - id. of the chat msg in Info::remote_chat_str_array[]
//
// nation_name1() - the nation from which this chat message is sent.
//
void NewsArray::chat_msg(int fromNationRecno, char* chatStr)
{
	//---- add the chat string into Info::remote_chat_str_array[] ----//

	int useChatId=0;
	int minDate=info.game_date+1;

	for( int i=0; i<MAX_REMOTE_CHAT_STR ; i++ )
	{
		if( info.remote_chat_array[i].received_date < minDate )	 // replace the oldest one
		{
			minDate	 = info.remote_chat_array[i].received_date;
			useChatId = i+1;
		}
	}

	if( useChatId )
	{
		ChatInfo* chatInfo = info.remote_chat_array+useChatId-1;

		chatInfo->received_date = info.game_date;
		chatInfo->from_nation_recno = fromNationRecno;

		strncpy( chatInfo->chat_str, chatStr, CHAT_STR_LEN );
		chatInfo->chat_str[CHAT_STR_LEN] = NULL;
	}

	//----------------------------------------------//

	News* newsPtr = add_news( NEWS_CHAT_MSG, NEWS_NORMAL, fromNationRecno);

	if( !newsPtr )		// only news of nations that have contact with the player are added
		return;

	newsPtr->short_para1 = useChatId;
}
//------- End of function NewsArray::chat_msg -----//


//------ Begin of function NewsArray::multi_retire -----//
//
// This function is called when a human player retires.
//
void NewsArray::multi_retire(int nationRecno)
{
	add_news( NEWS_MULTI_RETIRE, NEWS_NORMAL, nationRecno, nation_array.player_recno, 1 );		// add player recno as the 2nd parameter so this message is always displayed even if the player doesn't yet have contact with this nation
}
//------- End of function NewsArray::multi_retire -----//


//------ Begin of function NewsArray::multi_quit_game -----//
//
// This function is called when a human player quits the game.
//
void NewsArray::multi_quit_game(int nationRecno)
{
	add_news( NEWS_MULTI_QUIT_GAME, NEWS_NORMAL, nationRecno, nation_array.player_recno, 1 );		// add player recno as the 2nd parameter so this message is always displayed even if the player doesn't yet have contact with this nation
}
//------- End of function NewsArray::multi_quit_game -----//


//------ Begin of function NewsArray::multi_save_game -----//
//
// This function is called when a human player calls for saving the game.
//
void NewsArray::multi_save_game()
{
	add_news( NEWS_MULTI_SAVE_GAME, NEWS_NORMAL );
}
//------- End of function NewsArray::multi_save_game -----//


//------ Begin of function NewsArray::multi_connection_lost -----//
//
// This function is called when a human player's connection has been lost.
//
void NewsArray::multi_connection_lost(int nationRecno)
{
	add_news( NEWS_MULTI_CONNECTION_LOST, NEWS_NORMAL, nationRecno, nation_array.player_recno, 1 );		// add player recno as the 2nd parameter so this message is always displayed even if the player doesn't yet have contact with this nation
}
//------- End of function NewsArray::multi_connection_lost -----//


//------ Begin of function NewsArray::add_news -----//
//
// Called by news processing function to set news parameters
//
// <int> newsId       = the id. of the news
// <int> newsType     = news type
// [int] nationRecno  = nation recno of the news
// [int] nationRecno2 = recno of the 2nd nation related to the news
// [int] forceAdd		 = add this news anyway, regardless of whether
//								the nation has contact with the player or not
//                      (default: 0)
//
// return : <News*> return the pointer of the News
//						  NULL - the nation of the news does not have contact with the player
//
News* NewsArray::add_news(int newsId, int newsType, int nationRecno, int nationRecno2, int forceAdd)
{
	if( nation_array.player_recno==0 )		// if the player has lost
		return NULL;

	//----- only news of nations that have contact with the player are added ----//

	if( nation_array.player_recno && !forceAdd )
	{
		Nation* playerNation = ~nation_array;

		if( nationRecno && nationRecno != nation_array.player_recno )
		{
			if( !playerNation->get_relation(nationRecno)->has_contact )
				return NULL;
		}

		if( nationRecno2 && nationRecno2 != nation_array.player_recno )
		{
			if( !playerNation->get_relation(nationRecno2)->has_contact )
				return NULL;
		}
	}

	//----------------------------------------------//

	static News news;

	news.id   = newsId;
	news.type = newsType;
	news.news_date = info.game_date;
	news.loc_type = 0;

	Nation* nationPtr;

	if( nationRecno )
	{
		nationPtr = nation_array[nationRecno];

		news.nation_name_id1 = nationPtr->nation_name_id;
		news.nation_race_id1 = (char) nationPtr->race_id;
		news.nation_color1   = nationPtr->color_scheme_id;

		err_when( !news.nation_name_id1 || !news.nation_race_id1 );
	}
	else
	{
		news.nation_name_id1 = 0;
		news.nation_color1   = -1;
	}

	if( nationRecno2 )
	{
		nationPtr = nation_array[nationRecno2];

		news.nation_name_id2 = nationPtr->nation_name_id;
		news.nation_race_id2 = (char) nationPtr->race_id;
		news.nation_color2   = nationPtr->color_scheme_id;
	}
	else
	{
		news.nation_name_id2 = 0;
		news.nation_color2   = -1;
	}

	//--- if the news adding flag is turned off, don't add the news ---//

	if( news_add_flag )
	{
		//--- if no. of news reaches MAX., delete the oldest one ---//

		if( size() >= MAX_NEWS )
		{
			start();
			linkout();

			if( last_clear_recno > 1 )
				last_clear_recno--;
		}

		//--------- link in a new news ---------//

		linkin(&news);

		return (News*) get();
	}
	else
	{
		return &news;
	}
}
//------- End of function NewsArray::add_news -----//


//------ Begin of function News::set_loc ------//
//
void News::set_loc(int xLoc, int yLoc, int locType, int locTypePara, int locTypePara2)
{
	loc_type 	   = locType;
	loc_type_para  = locTypePara;
	loc_type_para2 = locTypePara2;

	err_when( loc_type_para < 0 );
	err_when( loc_type_para2 < 0 );

	loc_x = xLoc;
	loc_y = yLoc;
}
//------- End of function News::set_loc -------//


//------ Begin of function News::is_loc_valid ------//
//
// Whether the location of this news is still valid.
//
int News::is_loc_valid()
{
	if( !loc_type )
		return 0;

	int rc=0;

	if( loc_type == NEWS_LOC_TOWN )
	{
		if( !town_array.is_deleted(loc_type_para) )
		{
			Town* townPtr = town_array[loc_type_para];

			rc = townPtr->center_x == loc_x &&
				  townPtr->center_y == loc_y;
		}
	}
	else if( loc_type == NEWS_LOC_FIRM )
	{
		if( !firm_array.is_deleted(loc_type_para) )
		{
			Firm* firmPtr = firm_array[loc_type_para];

			rc = firmPtr->center_x == loc_x &&
				  firmPtr->center_y == loc_y;
		}
	}
	else if( loc_type == NEWS_LOC_UNIT )
	{
		if( !unit_array.is_deleted(loc_type_para) )
		{
			Unit* unitPtr = unit_array[loc_type_para];

			if( unitPtr->name_id == loc_type_para2 )
			{
				//--- if the unit is no longer belong to our nation ----//
				//--- only keep track of the unit for one month --------//

				if( unitPtr->nation_recno == nation_array.player_recno ||
					 info.game_date < news_date + 30 )
				{
					if( unitPtr->get_cur_loc(loc_x, loc_y) )
					{
						Location* locPtr = world.get_loc(loc_x, loc_y);

						rc = locPtr->visit_level > 0;
					}
				}
			}
		}
	}
	else if( loc_type == NEWS_LOC_ANY )
	{
		rc = 1;
	}

	if( !rc )
		loc_type = 0;

	return rc;
}
//------- End of function News::is_loc_valid -------//

