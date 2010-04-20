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

//Filename   : OAI_DIPL.CPP
//Description: AI functions on diplomacy

#include <OTALKRES.h>
#include <OCONFIG.h>
#include <OTECHRES.h>
#include <ONATION.h>


//----- Begin of function Nation::think_diplomacy -----//
//
void Nation::think_diplomacy()
{
	//--- process incoming messages first, so we won't send out the same request to nation which has already proposed the same thing ---//

	int nationRecno = nation_recno;

	process_action(0, ACTION_AI_PROCESS_TALK_MSG);

	if( nation_array.is_deleted(nationRecno) )		// the nation may have been deleted, if the nation accepts a purchase kingdom offer
		return;

	//---- thinking about war first -----//

	if( think_declare_war() )
		return;

	//----- think buy food first -------//

	think_request_buy_food();   // don't return even if this request is sent

	//----- think request cease fire ----//

	if( think_request_cease_war() )
		return;

	//------ thinking about treaty ---------//

	if( think_trade_treaty() )
		return;

	if( think_propose_alliance_treaty() )		// try proposing alliance treaty first, then try proposing friendly treaty
		return;

	if( think_propose_friendly_treaty() )
		return;

	if( think_end_treaty() )
		return;

	//-------- think about other matters --------//

	if( think_demand_tribute_aid() )
		return;

	if( think_give_tech() )
		return;

	if( think_demand_tech() )
		return;

	//---- think about offering to purchase throne ----//

	if( think_request_surrender() )
		return;
}
//------ End of function Nation::think_diplomacy ------//


//----- Begin of function Nation::should_diplomacy_retry -----//
//
int Nation::should_diplomacy_retry(int talkId, int nationRecno)
{
	if( !talk_res.can_send_msg(nationRecno, nation_recno, talkId ) )
		return 0;

	int retryInterval;

	//--- shorter retry interval for demand talk message ----//

	if( talkId == TALK_DEMAND_TRIBUTE ||
		 talkId == TALK_DEMAND_AID ||
		 talkId == TALK_DEMAND_TECH )
	{
		retryInterval = 60 + 60 * (100-pref_diplomacy_retry) / 100;		// 2-4 months
	}
	else
	{
		retryInterval = 90 + 270 * (100-pref_diplomacy_retry) / 100;		// 3 months to 12 months before next try
	}

	return info.game_date >
			 get_relation(nationRecno)->last_talk_reject_date_array[talkId-1] + retryInterval;
}
//------ End of function Nation::should_diplomacy_retry ------//


//----- Begin of function Nation::ai_notify_reply -----//
//
// Notify this AI nation that there is a reply to one
// of the diplomatic messages that it has sent out.
//
void Nation::ai_notify_reply(int talkMsgRecno)
{
	err_when( talk_res.is_talk_msg_deleted(talkMsgRecno) );

	TalkMsg* 		 talkMsg = talk_res.get_talk_msg(talkMsgRecno);
	int 				 relationChange = 0;
	NationRelation* nationRelation = get_relation(talkMsg->to_nation_recno);

	if( talkMsg->reply_type == REPLY_REJECT )
		nationRelation->last_talk_reject_date_array[talkMsg->talk_id-1] = info.game_date;
	else
		nationRelation->last_talk_reject_date_array[talkMsg->talk_id-1] = 0;

	switch( talkMsg->talk_id )
	{
		case TALK_PROPOSE_TRADE_TREATY:
			if( talkMsg->reply_type == REPLY_ACCEPT )
				relationChange = pref_trading_tendency/10;
			else
				relationChange = -pref_trading_tendency/10;
			break;

		case TALK_PROPOSE_FRIENDLY_TREATY:
		case TALK_PROPOSE_ALLIANCE_TREATY:
			if( talkMsg->reply_type == REPLY_REJECT )
				relationChange = -5;
			break;

		case TALK_REQUEST_MILITARY_AID:
			if( talkMsg->reply_type == REPLY_ACCEPT )
				relationChange = 0;		// the AI never knows whether the player has really aided him in the war
			else
				relationChange = -(20-pref_military_courage/10);	// -10 to -20
			break;

		case TALK_REQUEST_TRADE_EMBARGO:
			if( talkMsg->reply_type == REPLY_ACCEPT )
				relationChange = (10+pref_trading_tendency/10);		// +10 to +20
			else
				relationChange = -(10+pref_trading_tendency/20);	// -10 to -15
			break;

		case TALK_REQUEST_CEASE_WAR:
			if( talkMsg->reply_type == REPLY_REJECT )
				relationChange = -5;
			break;

		case TALK_REQUEST_DECLARE_WAR:
			if( talkMsg->reply_type == REPLY_ACCEPT )
				relationChange = pref_allying_tendency/10;
			else
				relationChange = -30;
			break;

		case TALK_REQUEST_BUY_FOOD:
			if( talkMsg->reply_type == REPLY_ACCEPT )
				relationChange = pref_food_reserve/10;
			else
				relationChange = -pref_food_reserve/10;
			break;

		case TALK_DEMAND_TRIBUTE:
		case TALK_DEMAND_AID:
			if( talkMsg->reply_type == REPLY_ACCEPT )
			{
				//-- the less cash the nation, the more it will appreciate the tribute --//

				relationChange = 100 * talkMsg->talk_para1 / MAX(1000, (int) cash);
			}
			else
			{
				relationChange = -(400-pref_peacefulness)/10;	// -30 to 40 points depending the peacefulness preference
			}
			break;

		case TALK_DEMAND_TECH:
			if( talkMsg->reply_type == REPLY_ACCEPT )
				relationChange = 10+pref_use_weapon/5; 		// +10 to +30
			else
				relationChange = -(10+pref_use_weapon/10);		// -10 to -20
			break;

		case TALK_GIVE_TRIBUTE:
		case TALK_GIVE_AID:
		case TALK_GIVE_TECH:
			if( talkMsg->reply_type == REPLY_REJECT )		// reject your gift
				relationChange = -5;
			break;

		case TALK_REQUEST_SURRENDER:		// no relation change on this request 
			break;

		default:
			err_here();
	}

	//------- chance relationship now -------//

	if( relationChange < 0 )
		relationChange -= relationChange * (200-pref_forgiveness) / 200;

	if( relationChange != 0 )
		change_ai_relation_level( talkMsg->to_nation_recno, relationChange );

	//---- think about giving tribute to become more friendly with the nation so it will accept our request next time ---//

	if( talkMsg->reply_type == REPLY_REJECT )
	{
		if( think_give_tribute_aid( talkMsg ) )
			return;

		//--- if our request was rejected, end treaty if the ai_nation_relation is low enough ---//

		if( talkMsg->talk_id != TALK_PROPOSE_ALLIANCE_TREATY && 		// the rejected request is not alliance treaty
			 nationRelation->status >= NATION_FRIENDLY &&
			 nationRelation->ai_relation_level < 40-pref_allying_tendency/5 )		// 20 to 40
		{
			int talkId;

			if( nationRelation->status == NATION_FRIENDLY )
				talkId = TALK_END_FRIENDLY_TREATY;
			else
				talkId = TALK_END_ALLIANCE_TREATY;

			talk_res.ai_send_talk_msg(talkMsg->to_nation_recno, nation_recno, talkId);
		}

		//----- declare war if ai_relation_level==0 -----//

		else if( nationRelation->ai_relation_level == 0 )
		{
			//--------- declare war ---------//

			if( config.ai_aggressiveness >= OPTION_HIGH || pref_peacefulness < 50 )
			{
				talk_res.ai_send_talk_msg(talkMsg->to_nation_recno, nation_recno, TALK_DECLARE_WAR);

				//------- attack immediately --------//

				if( config.ai_aggressiveness >= OPTION_VERY_HIGH ||
					 ( config.ai_aggressiveness >= OPTION_HIGH && pref_peacefulness < 50 ) )
				{
					if( largest_town_recno )
					{
						think_capture_new_enemy_town( town_array[largest_town_recno], 1 );		// 1-use forces from all camps to attack the target 
					}
				}
			}
		}
	}
}
//------ End of function Nation::ai_notify_reply ------//


//----- Begin of function Nation::think_propose_friendly_treaty -----//
//
int Nation::think_propose_friendly_treaty()
{
	//--- think about which nation this nation should propose treaty to ---//

	int				 curRating, bestRating=0, bestNationRecno=0;
	NationRelation* nationRelation;

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationRelation = get_relation(i);

		if( !nationRelation->has_contact || nationRelation->status >= NATION_FRIENDLY )
			continue;

		if( !should_diplomacy_retry(TALK_PROPOSE_FRIENDLY_TREATY, i) )
			continue;

		curRating = consider_friendly_treaty(i);

		if( curRating > bestRating )
		{
			bestRating 	 	 = curRating;
			bestNationRecno = i;
		}
	}

	if( bestNationRecno )
	{
		talk_res.ai_send_talk_msg(bestNationRecno, nation_recno, TALK_PROPOSE_FRIENDLY_TREATY );
		return 1;
	}

	return 0;
}
//------ End of function Nation::think_propose_friendly_treaty ------//


//----- Begin of function Nation::think_propose_alliance_treaty -----//
//
int Nation::think_propose_alliance_treaty()
{
	//--- think about which nation this nation should propose treaty to ---//

	int				 curRating, bestRating=0, bestNationRecno=0;
	NationRelation* nationRelation;

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationRelation = get_relation(i);

		if( !nationRelation->has_contact || nationRelation->status == NATION_ALLIANCE )
			continue;

		if( !should_diplomacy_retry(TALK_PROPOSE_ALLIANCE_TREATY, i) )
			continue;

		curRating = consider_alliance_treaty(i);

		if( curRating > bestRating )
		{
			bestRating 	 	 = curRating;
			bestNationRecno = i;
		}
	}

	if( bestNationRecno )
	{
		talk_res.ai_send_talk_msg(bestNationRecno, nation_recno, TALK_PROPOSE_ALLIANCE_TREATY );
		return 1;
	}

	return 0;
}
//------ End of function Nation::think_propose_alliance_treaty ------//


//----- Begin of function Nation::think_request_cease_war -----//
//
int Nation::think_request_cease_war()
{
	Nation* 			 nationPtr;
	NationRelation* nationRelation;

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationPtr = nation_array[i];

		nationRelation = get_relation(i);

		if( nationRelation->status != NATION_HOSTILE )
			continue;

		if( !should_diplomacy_retry(TALK_REQUEST_CEASE_WAR, i) )
			continue;

		//----- think about if it should cease war with the nation ------//

		if( consider_cease_war(i) > 0 )
		{
			talk_res.ai_send_talk_msg(i, nation_recno, TALK_REQUEST_CEASE_WAR);
		}

		//--------------------------------------------//
		// The relation improves slowly if there is
		// no attack. However, if there is any battles
		// started between the two nations, the status will be
		// set to hostile and ai_relation_level will be set to 0 again.
		//--------------------------------------------//

		else
		{
			change_ai_relation_level(i, 1);
		}
	}

	return 0;
}
//------ End of function Nation::think_request_cease_war ------//


//----- Begin of function Nation::think_end_treaty -----//
//
int Nation::think_end_treaty()
{
	if( pref_honesty < 30 )		// never formally end a treaty if the honesty is < 30
		return 0;

	Nation* nationPtr;
	NationRelation* nationRelation;

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationRelation = get_relation(i);

		if( nationRelation->status < NATION_FRIENDLY )
			continue;

		if( nationRelation->ai_secret_attack ||
			 ( nationRelation->ai_relation_level < 30 && trade_rating(i) < 50 ) )
		{
			//--- don't change terminate treaty too soon ---//

			if( info.game_date < nationRelation->last_change_status_date+60+pref_honesty/2 )		// only after 60 to 110 days
				continue;

			//----------------------------------------//

			if( !talk_res.can_send_msg(i, nation_recno, nationRelation->status==NATION_FRIENDLY ? TALK_END_FRIENDLY_TREATY : TALK_END_ALLIANCE_TREATY) )
				continue;

			nationPtr = nation_array[i];

			//-----------------------------------------//
			// What makes it tend to end treaty:
			// -higher honesty
			// -a larger overall power over the target nation.
			//
			// If honesty is > 50, if will end treaty
			// if its power is equal to the enemy.
			//
			// If honesty is < 50, if will end treaty
			// if its power is larger than the enemy.
			//
			// If honesty is > 50, if will end treaty
			// even if its power is lower than the enemy.
			//-----------------------------------------//

			if( pref_honesty-50 > nationPtr->overall_rating - overall_rating )
			{
				if( nationRelation->status == NATION_FRIENDLY )
					talk_res.ai_send_talk_msg(i, nation_recno, TALK_END_FRIENDLY_TREATY);
				else
					talk_res.ai_send_talk_msg(i, nation_recno, TALK_END_ALLIANCE_TREATY);

				return 1;
			}
		}
	}

	return 0;
}
//------ End of function Nation::think_end_treaty ------//


//----- Begin of function Nation::think_trade_treaty -----//
//
int Nation::think_trade_treaty()
{
	Nation* 			nationPtr;
	NationRelation *ourRelation;

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationPtr = nation_array[i];

		ourRelation = get_relation(i);

		if( !ourRelation->has_contact )
			continue;

		//------- propose a trade treaty --------//

		if( !ourRelation->trade_treaty )
		{
			if( consider_trade_treaty(i) > 0 )
			{
				if( should_diplomacy_retry(TALK_PROPOSE_TRADE_TREATY, i) )
				{
					talk_res.ai_send_talk_msg(i, nation_recno, TALK_PROPOSE_TRADE_TREATY);
					ourRelation->ai_demand_trade_treaty = 0;
					return 1;
				}
			}
		}
	}

	return 0;
}
//------ End of function Nation::think_trade_treaty ------//


//----- Begin of function Nation::think_request_buy_food -----//
//
int Nation::think_request_buy_food()
{
	//------ first see if we need to buy food ------//

	int yearFoodChange = yearly_food_change();
	int neededFoodLevel;

	if( yearFoodChange > 0 )
	{
		if( food > 0 )
			return 0;
		else
			neededFoodLevel = (int) -food;		// if the food is negative
	}
	else
	{
		neededFoodLevel = -yearFoodChange * (100+pref_food_reserve) / 50;

		if( food > neededFoodLevel )		// one to three times (based on pref_food_reserve) of the food needed in a year,
			return 0;
	}

	//----- think about which nation to buy food from -----//

	Nation  *nationPtr, *bestNation=NULL;
	int	  curRating, bestRating=0;
	int	  relationStatus;

	int i;
	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationPtr = nation_array[i];

		if( nationPtr->food < 500 )		// if the nation is short of food itself. The minimum request purchase qty is 500
			continue;

		relationStatus = get_relation_status(i);

		if( relationStatus == NATION_HOSTILE || !get_relation(i)->has_contact )
			continue;

		if( nationPtr->yearly_food_change() < 0 &&
			 nationPtr->food < 1500 )
		{
			continue;
		}

		if( !should_diplomacy_retry(TALK_REQUEST_BUY_FOOD, i) )
			continue;

		//-----------------------------------//

		curRating = relationStatus*20 +
						(int)nationPtr->food / 100 +
						(int)nationPtr->yearly_food_change() / 10;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestNation = nationPtr;
		}
	}

	if( !bestNation )
		return 0;

	//------------------------------------//

	static short buyQtyArray[] = { 500, 1000, 2000, 4000 };

	int buyQty=0, buyPrice;

	for( i=3 ; i>=0 ; i-- )
	{
		if( bestNation->food/2 > buyQtyArray[i] )
		{
			buyQty = buyQtyArray[i];
			break;
		}
	}

	if( buyQty == 0 )
		return 0;

	//------- set the offering price ------//

	if( food < neededFoodLevel/4 )		// if we need the food badly
	{
		buyPrice = 30;
	}
	else if( food < neededFoodLevel/3 )
	{
		buyPrice = 20;
	}
	else
	{
		if( bestNation->food > bestNation->all_population() * PERSON_FOOD_YEAR_CONSUMPTION * 5 &&		// if the nation has plenty of food
			 bestNation->cash < bestNation->fixed_expense_365days() / 2 )										// if the nation runs short of cash
		{
			buyPrice = 5;
		}
		else
			buyPrice = 10;
	}

	talk_res.ai_send_talk_msg(bestNation->nation_recno, nation_recno, TALK_REQUEST_BUY_FOOD, buyQty, buyPrice);
	return 1;
}
//------ End of function Nation::think_request_buy_food ------//


//----- Begin of function Nation::think_declare_war -----//
//
int Nation::think_declare_war()
{
	NationRelation* nationRelation;
	int rc=0;

	//---- don't declare a new war if we already has enemies ---//

	int i;
	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		if( get_relation(i)->status == NATION_HOSTILE )
			return 0;
	}

	//------------------------------------------------//

	int targetStrength, minStrength=0x1000, bestTargetNation=0;

	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationRelation = get_relation(i);

		if( !nationRelation->has_contact )
			continue;

		if( nationRelation->status == NATION_HOSTILE )		// already at war
			continue;

		if( nationRelation->ai_relation_level >= 10 )
			continue;

		if( !ai_should_spend( 100-trade_rating(i) ) )		// if trade_rating is 0, importanceRating will be 100, if trade_rating is 100, importanceRating will be 0
			continue;

		//----------------------------------------//

		Nation* targetNation = nation_array[i];

		targetStrength = targetNation->military_rank_rating() +
							  targetNation->population_rank_rating()/2 +
							  targetNation->economic_rank_rating()/3;

		if( targetStrength < minStrength )
		{
			minStrength = targetStrength;
			bestTargetNation = i;
		}
	}

	//------------------------------------------//

	if( bestTargetNation )
	{
		if( should_diplomacy_retry(TALK_DECLARE_WAR, bestTargetNation) )
		{
			talk_res.ai_send_talk_msg(bestTargetNation, nation_recno, TALK_DECLARE_WAR);
			return 1;
		}
	}

	return 0;
}
//------ End of function Nation::think_declare_war ------//


//----- Begin of function Nation::think_give_tribute_aid -----//
//
// This function is called when a nation rejected our request
// which is important to us and we want to give tribute to the
// nation so it may accept next time.
//
// <TalkMsg*> rejectedMsg - the TalkMsg that has been rejected.
//
int Nation::think_give_tribute_aid(TalkMsg* rejectedMsg)
{
	//-----------get the talk id. ------------//

	int talkId;
	int talkNationRecno = rejectedMsg->to_nation_recno;
	int rejectedTalkId  = rejectedMsg->talk_id;
	NationRelation* nationRelation = get_relation(talkNationRecno);

	if( nationRelation->status >= NATION_FRIENDLY )
		talkId = TALK_GIVE_AID;
	else
		talkId = TALK_GIVE_TRIBUTE;

	//-------- don't give tribute too frequently -------//

	if( info.game_date <
		 nationRelation->last_talk_reject_date_array[talkId-1] + 365 - pref_allying_tendency )
	{
		return 0;
	}

	//---- think if the nation should spend money now ----//

	static short tributeAmountArray[] = { 500, 1000 };
	int tributeAmount = tributeAmountArray[m.random(2)];

	if( !ai_should_spend(0, (float) tributeAmount) )		// importance rating is 0
		return 0;

	//--------------------------------------//

	Nation* talkNation = nation_array[talkNationRecno];
	int	  rc;

	if( rejectedTalkId == TALK_PROPOSE_TRADE_TREATY )
	{
		rc = ai_trade_with_rating(talkNationRecno) > 100-pref_trading_tendency/2;
	}

	else if ( rejectedTalkId == TALK_PROPOSE_FRIENDLY_TREATY ||
				 rejectedTalkId == TALK_PROPOSE_ALLIANCE_TREATY )
	{
		int curRating = talkNation->trade_rating(talkNationRecno) +
							 ai_trade_with_rating(talkNationRecno) +
							 talkNation->overall_rating - overall_rating;

		int acceptRating = 200-pref_trading_tendency/4
									 -pref_allying_tendency/4;

		rc = curRating >= acceptRating;
	}

	//--------------------------------------//

	else if( rejectedTalkId == TALK_REQUEST_CEASE_WAR )
	{
		rc = talkNation->military_rank_rating() >
			  military_rank_rating() + (100-pref_peacefulness)/2;
	}

	//--------------------------------------//

	if( rc )
	{
		//------ give tribute --------//

		talk_res.ai_send_talk_msg(talkNationRecno, nation_recno, talkId, tributeAmount);

		nationRelation->last_talk_reject_date_array[talkId-1] = info.game_date;

		//------ request again after giving tribute ----//

		nationRelation->last_talk_reject_date_array[rejectedTalkId-1] = 0;		// reset the rejected talk id.

		talk_res.ai_send_talk_msg(talkNationRecno, nation_recno, rejectedTalkId, rejectedMsg->talk_para1, rejectedMsg->talk_para2 );
	}

	return rc;
}
//------ End of function Nation::think_give_tribute_aid ------//


//----- Begin of function Nation::think_demand_tribute_aid -----//
//
// Demand tribute when the nation's economy is good and its
// military is weak.
//
int Nation::think_demand_tribute_aid()
{
	if( info.game_date < info.game_start_date + 180 + nation_recno*50 )		// don't ask for tribute too soon, as in the beginning, the ranking are all the same for all nations
		return 0;

	//--------------------------------------//

	Nation* nationPtr;
	int	  totalNation=nation_array.size();
	int	  nationRecno=m.random(totalNation)+1;
	int	  curRating, requestRating;
	int	  talkId;
	int	  ourMilitary = military_rank_rating();
	int	  ourEconomy  = economic_rank_rating();

	for( int i=totalNation ; i>0 ; i-- )
	{
		if( ++nationRecno > totalNation )
			nationRecno = 1;

		if( nation_array.is_deleted(nationRecno) || nationRecno==nation_recno )
			continue;

		nationPtr = nation_array[nationRecno];

		//-- only demand tribute from non-friendly nations --//

		if( get_relation(nationRecno)->status <= NATION_NEUTRAL )
			talkId = TALK_DEMAND_TRIBUTE;
		else
			talkId = TALK_DEMAND_AID;

		//-----------------------------------------------//

		float fixedExpense = fixed_expense_365days();

		if( talkId == TALK_DEMAND_TRIBUTE )
		{
			if( !should_diplomacy_retry(talkId, nationRecno) )
				continue;

			curRating = ourMilitary - nationPtr->military_rank_rating();

			if( curRating < 0 )
				continue;

			//----------------------------------------------//
			//
			// Some nation will actually consider the ability
			// of the target nation to pay tribute, so nation
			// will not and just ask anyway.
			//
			//----------------------------------------------//

			if( pref_economic_development > 50 )
			{
				int addRating = nationPtr->economic_rank_rating()-ourEconomy;

				if( addRating > 0 )
					curRating += addRating;
			}

			requestRating = 20 + trade_rating(nationRecno)/2 +
								(100-pref_peacefulness)/3;

			if( cash < fixedExpense && fixedExpense != 0 )
				requestRating -= int( (float) requestRating * cash / fixedExpense);

		}
		else
		{
			if( cash >= fixedExpense )
				continue;

			if( cash > fixedExpense * (50+pref_cash_reserve) / 300 &&		// if the nation is runing short of cash, don't wait a while until next retry, retry immediately
				 !should_diplomacy_retry(talkId, nationRecno) )
			{
				continue;
			}

			//----- only ask for aid when the nation is short of cash ----//

			curRating = (ourMilitary - nationPtr->military_rank_rating())/2 +
							( nationPtr->economic_rank_rating()-ourEconomy );

			requestRating = 20 + 50 * (int)(cash / fixedExpense);
		}

		//----- if this is a human player's nation -----//

		if( !nationPtr->is_ai() )
		{
			switch( config.ai_aggressiveness )
			{
				case OPTION_LOW:
					requestRating += 40;		// don't go against the player too easily
					break;

				case OPTION_HIGH:
					requestRating -= 20;
					break;

				case OPTION_VERY_HIGH:
					requestRating -= 40;
					break;
			}

			//--- if the nation has plenty of cash, demand from it ----//

			if( nationPtr->cash > cash && config.ai_aggressiveness >= OPTION_HIGH )
			{
				requestRating -= (int) (nationPtr->cash - cash)/500;
			}
		}

		//--------------------------------------//

		if( curRating > requestRating )
		{
			int tributeAmount;

			if( curRating - requestRating > 120 )
				tributeAmount = 4000;

			else if( curRating - requestRating > 80 )
				tributeAmount = 3000;

			else if( curRating - requestRating > 40 )
				tributeAmount = 2000;

			else if( curRating - requestRating > 20 )
				tributeAmount = 1000;

			else
				tributeAmount = 500;

			talk_res.ai_send_talk_msg(nationRecno, nation_recno, talkId, tributeAmount);

			return 1;
		}
	}

	return 0;
}
//------ End of function Nation::think_demand_tribute_aid ------//


//----- Begin of function Nation::think_demand_tech -----//
//
int Nation::think_demand_tech()
{
	if( m.random(10) > 0 )		// only 1/10 chance of calling this function
		return 0;

	Nation* nationPtr;
	int	  totalNation=nation_array.size();
	int	  nationRecno=m.random(totalNation)+1;

	for( int i=totalNation ; i>0 ; i-- )
	{
		if( ++nationRecno > totalNation )
			nationRecno = 1;

		if( nation_array.is_deleted(nationRecno) || nationRecno==nation_recno )
			continue;

		nationPtr = nation_array[nationRecno];

		if( nationPtr->total_tech_level() == 0 )
			continue;

		if( !should_diplomacy_retry(TALK_DEMAND_TECH, nationRecno) )
			continue;

		//--- don't request from hostile or tense nations -----//

		if( get_relation(nationRecno)->status < NATION_NEUTRAL )
			continue;

		//---- scan which tech that the nation has but we don't have ----//

		int techId;
		for( techId=1 ; techId<=tech_res.tech_count ; techId++ )
		{
			TechInfo *techInfo = tech_res[techId];

			if( techInfo->get_nation_tech_level(nation_recno)==0 &&
				 techInfo->get_nation_tech_level(nationRecno) > 0 )
			{
				break;
			}
		}

		if( techId > tech_res.tech_count )
			continue;

		//-------- send the message now ---------//

		talk_res.ai_send_talk_msg(nationRecno, nation_recno, TALK_DEMAND_TECH, techId);
		return 1;
	}

	return 0;
}
//------ End of function Nation::think_demand_tech ------//


//----- Begin of function Nation::think_give_tech -----//
//
int Nation::think_give_tech()
{
	return 0;
}
//------ End of function Nation::think_give_tech ------//


//----- Begin of function Nation::think_request_surrender -----//
//
int Nation::think_request_surrender()
{
	if( m.random(5) != 0 )		// don't do this too often
		return 0;

	//---- only do so when we have enough cash ----//

	if( cash < fixed_expense_365days() + 5000 + 10000 * pref_cash_reserve / 100 )
		return 0;

	if( profit_365days() < 0 && cash < 20000 )		// don't ask if we are losing money and the cash isn't plenty
		return 0;

	//----- calculate the amount this nation can offer ----//

	int offerAmount = (int)cash - MIN(5000, (int)fixed_expense_365days());

	static int amtArray[] = { 5000, 7500, 10000, 15000, 20000, 30000, 40000, 50000 };

	int i;
	for( i=7 ; i>=0 ; i-- )
	{
		if( offerAmount >= amtArray[i] )
		{
			offerAmount = amtArray[i];
			break;
		}
	}

	if( i<0 )
		return 0;

	//---------------------------------------------//

	Nation* nationPtr;
	int     ourOverallRankRating = overall_rank_rating();
	int	  totalNation = nation_array.size();

	int nationRecno = m.random(totalNation)+1;

	for( i=0 ; i<totalNation ; i++ )
	{
		if( ++nationRecno > totalNation )
			nationRecno = 1;

		if( nation_array.is_deleted(nationRecno) || nation_recno==nationRecno )
			continue;

		nationPtr = nation_array[nationRecno];

		//--- don't ask for a kingdom that is more powerful to surrender to us ---//

		if( nationPtr->cash > 100 )	// unless it is running short of cash
		{
			if( nationPtr->overall_rank_rating() > ourOverallRankRating )
				continue;
		}

		//-------------------------------------------//

		if( !should_diplomacy_retry(TALK_REQUEST_SURRENDER, nationRecno) )
			continue;

		//-------------------------------------------//

		talk_res.ai_send_talk_msg(nationRecno, nation_recno,
			TALK_REQUEST_SURRENDER, offerAmount/10 );			// divide by 10 to cope with <short>'s upper limit

		return 1;
	}

	return 0;
}
//------ End of function Nation::think_request_surrender ------//

