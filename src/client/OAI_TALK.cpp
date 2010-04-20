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

//Filename   : OAI_TALK.CPP
//Description: AI routines on diplomacy.

#include <OCONFIG.h>
#include <OTALKRES.h>
#include <OTECHRES.h>
#include <OF_MARK.h>
#include <ONATION.h>


//-------- Declare static functions ---------//

static int has_sent_same_msg(TalkMsg* talkMsg);


//----- Begin of function Nation::ai_process_talk_msg -----//
//
// action_para - recno of the message in talk_res.talk_msg_array.
//
int Nation::ai_process_talk_msg(ActionNode* actionNode)
{
	if( talk_res.is_talk_msg_deleted(actionNode->action_para) )		// if the talk message has been deleted
		return -1;

	TalkMsg* talkMsg = talk_res.get_talk_msg(actionNode->action_para);

	err_when( talkMsg->talk_id < 1 || talkMsg->talk_id > MAX_TALK_TYPE );

	err_when( talkMsg->from_nation_recno == nation_recno );
	err_when( talkMsg->to_nation_recno   != nation_recno );

	if( !talkMsg->is_valid_to_reply() )		// if it is no longer valid
		return -1;

	//----- call the consider function -------//

	if( talkMsg->reply_type == REPLY_WAITING )
	{
		int rc = consider_talk_msg(talkMsg);

		if( rc==1 )		// if rc is not 1 or 0, than the consider function have processed itself, no need to call reply_talk_msg() here
			talk_res.reply_talk_msg( actionNode->action_para, REPLY_ACCEPT, COMMAND_AI );

		else if( rc==0 )
			talk_res.reply_talk_msg( actionNode->action_para, REPLY_REJECT, COMMAND_AI );

		// don't reply if rc is neither 0 or 1
	}
	else
		err_here();

	return -1;		// always return -1 to remove the action from action_array. 
}
//------ End of function Nation::ai_process_talk_msg ------//


//----- Begin of function Nation::consider_talk_msg -----//
//
int Nation::consider_talk_msg(TalkMsg* talkMsg)
{
	//--------------------------------------------//
	// Whether the nation has already sent out a
	// message that is the same as the one it received.
	// If so, accept the message right now.
	//--------------------------------------------//

	switch( talkMsg->talk_id )
	{
		case TALK_PROPOSE_TRADE_TREATY:
		case TALK_PROPOSE_FRIENDLY_TREATY:
		case TALK_PROPOSE_ALLIANCE_TREATY:
		case TALK_REQUEST_TRADE_EMBARGO:
		case TALK_REQUEST_CEASE_WAR:
		case TALK_REQUEST_DECLARE_WAR:
			if( has_sent_same_msg(talkMsg) )
				return 1;
	};

	//-------------------------------//

	switch( talkMsg->talk_id )
	{
		case TALK_PROPOSE_TRADE_TREATY:
			return consider_trade_treaty(talkMsg->from_nation_recno) >= 0;		// the returned value is the curRating - acceptRating, if >=0, means it accepts

		case TALK_PROPOSE_FRIENDLY_TREATY:
			return consider_friendly_treaty(talkMsg->from_nation_recno) >= 0;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			return consider_alliance_treaty(talkMsg->from_nation_recno) >= 0;

		case TALK_REQUEST_MILITARY_AID:
			return consider_military_aid(talkMsg);

		case TALK_REQUEST_TRADE_EMBARGO:
			return consider_trade_embargo(talkMsg);

		case TALK_REQUEST_CEASE_WAR:
			return consider_cease_war(talkMsg->from_nation_recno) >= 0;

		case TALK_REQUEST_DECLARE_WAR:
			return consider_declare_war(talkMsg);

		case TALK_REQUEST_BUY_FOOD:
			return consider_sell_food(talkMsg);

		case TALK_GIVE_TRIBUTE:
			return consider_take_tribute(talkMsg);

		case TALK_DEMAND_TRIBUTE:
			return consider_give_tribute(talkMsg);

		case TALK_GIVE_AID:
			return consider_take_aid(talkMsg);

		case TALK_DEMAND_AID:
			return consider_give_aid(talkMsg);

		case TALK_GIVE_TECH:
			return consider_take_tech(talkMsg);

		case TALK_DEMAND_TECH:
			return consider_give_tech(talkMsg);

		case TALK_REQUEST_SURRENDER:
			return consider_accept_surrender_request(talkMsg);

		default:
			err_here();
			return 0;
	}
}
//------ End of function Nation::consider_talk_msg ------//


//----- Begin of function Nation::notify_talk_msg -----//
//
// Notify the AI for a notification only message (reply not needed.)
//
// This function is called directly from TalkRes::send_talk_msg_now()
// when the message is sent.
//
void Nation::notify_talk_msg(TalkMsg* talkMsg)
{
	int relationChange=0;
	NationRelation* nationRelation = get_relation(talkMsg->from_nation_recno);

	switch( talkMsg->talk_id )
	{
		case TALK_END_TRADE_TREATY:			// it's a notification message only, no accept or reject
			relationChange = -5;
			nationRelation->last_talk_reject_date_array[TALK_PROPOSE_TRADE_TREATY-1] = info.game_date;
			break;

		case TALK_END_FRIENDLY_TREATY:			// it's a notification message only, no accept or reject
		case TALK_END_ALLIANCE_TREATY:
			relationChange = -5;
			nationRelation->last_talk_reject_date_array[TALK_PROPOSE_FRIENDLY_TREATY-1] = info.game_date;
			nationRelation->last_talk_reject_date_array[TALK_PROPOSE_ALLIANCE_TREATY-1] = info.game_date;
			break;

		case TALK_DECLARE_WAR:			// it already drops to zero when the status is set to hostile
			break;

		case TALK_GIVE_TRIBUTE:
		case TALK_GIVE_AID:

			//--------------------------------------------------------------//
			// The less cash the nation, the more it will appreciate the
			// tribute.
			//
			// $1000 for 100 ai relation increase if the nation's cash is 1000.
			//--------------------------------------------------------------//

			relationChange = 100 * talkMsg->talk_para1 / MAX(1000, (int) cash);
			break;

		case TALK_GIVE_TECH:

			//--------------------------------------------------------------//
			// The lower tech the nation has, the more it will appreciate the
			// tech giveaway.
			//
			// Giving a level 2 weapon which the nation is unknown of
			// increase the ai relation by 60 if its pref_use_weapon is 100.
			// (by 30 if its pref_use_weapon is 0).
			//--------------------------------------------------------------//
		{
			int ownLevel = tech_res[talkMsg->talk_para1]->get_nation_tech_level(nation_recno);

			if( talkMsg->talk_para2 > ownLevel )
				relationChange = 30 * (talkMsg->talk_para2-ownLevel)
									  * (100+pref_use_weapon) / 200;
			break;
		}

		case TALK_SURRENDER:
			break;

		default:
			err_here();
	}

	//------- chance relationship now -------//

	if( relationChange < 0 )
		relationChange -= relationChange * pref_forgiveness / 100;

	if( relationChange != 0 )
		change_ai_relation_level( talkMsg->from_nation_recno, relationChange );
}
//------ End of function Nation::notify_talk_msg ------//


//----- Begin of function Nation::consider_trade_treaty -----//
//
// Consider agreeing to open up trade with the given nation.
//
int Nation::consider_trade_treaty(int withNationRecno)
{
	NationRelation* nationRelation = get_relation(withNationRecno);

	//---- don't accept new trade treaty soon when the trade treaty was terminated not too long ago ----//

	if( info.game_date < nationRelation->last_talk_reject_date_array[TALK_END_TRADE_TREATY-1] + 365 - pref_forgiveness )
		return 0;

	//-- if we look forward to have a trade treaty with this nation ourselves --//

	if( nationRelation->ai_demand_trade_treaty )
		return 1;

	return ai_trade_with_rating(withNationRecno) > 0;
}
//------ End of function Nation::consider_trade_treaty ------//


//----- Begin of function Nation::ai_trade_with_rating -----//
//
// Return a rating from 0 to 100 indicating how important
// will be for us to trade with the given nation.
//
int Nation::ai_trade_with_rating(int withNationRecno)
{
	Nation* nationPtr = nation_array[withNationRecno];
	int 	  tradeRating=0;

	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		//--------------------------------------------------------------//
		//
		// If we have the raw material and it doesn't have, then we
		// can export to it. And it is more favorite if the nation's
		// population is high, so we can export more.
		//
		//--------------------------------------------------------------//

		if( raw_count_array[i] && !nationPtr->raw_count_array[i] )
			tradeRating += MIN(30, nationPtr->total_population/3);

		//--------------------------------------------------------------//
		//
		// If the nation has the supply a raw material that we don't
		// have, then we can import it.
		//
		//--------------------------------------------------------------//

		else if( nationPtr->raw_count_array[i] && !raw_count_array[i] )
			tradeRating += 30;
	}

	return tradeRating;
}
//------ End of function Nation::ai_trade_with_rating ------//


//----- Begin of function Nation::consider_friendly_treaty -----//
//
int Nation::consider_friendly_treaty(int withNationRecno)
{
	NationRelation* nationRelation = get_relation(withNationRecno);

	if( nationRelation->status >= NATION_FRIENDLY )		// already has a friendly relationship
		return -1;			// -1 means don't reply

	if( nationRelation->ai_relation_level < 20 )
		return -1;

	//------- some consideration first -------//

	if( !should_consider_friendly(withNationRecno) )
		return -1;

	//------ total import and export amounts --------//

	int curRating = consider_alliance_rating(withNationRecno);

	int acceptRating = 60 - pref_allying_tendency/8 - pref_peacefulness/4;  // range of acceptRating: 23 to 60

	return curRating - acceptRating;
}
//------ End of function Nation::consider_friendly_treaty ------//


//----- Begin of function Nation::consider_alliance_treaty -----//
//
int Nation::consider_alliance_treaty(int withNationRecno)
{
	NationRelation* nationRelation = get_relation(withNationRecno);

	if( nationRelation->status >= NATION_ALLIANCE )		// already has a friendly relationship
		return -1;			// -1 means don't reply

	if( nationRelation->ai_relation_level < 40 )
		return -1;

	//------- some consideration first -------//

	if( !should_consider_friendly(withNationRecno) )
		return -1;

	//------ total import and export amounts --------//

	int curRating = consider_alliance_rating(withNationRecno);

	int acceptRating = 80 - pref_allying_tendency/4 - pref_peacefulness/8;  // range of acceptRating: 43 to 80

	return curRating - acceptRating;
}
//------ End of function Nation::consider_alliance_treaty ------//


//----- Begin of function Nation::consider_cease_war -----//
//
// This function is shared by think_request_cease_war().
//
int Nation::consider_cease_war(int withNationRecno)
{
	NationRelation* nationRelation = get_relation(withNationRecno);

	if( nationRelation->status != NATION_HOSTILE )
		return -1;			// -1 means don't reply

   //---- if we are attacking the nation, don't cease fire ----//

	if( ai_attack_target_nation_recno == withNationRecno )
		return -1;

	//---- if we are planning to capture the enemy's town ---//

	if( ai_capture_enemy_town_recno &&
		 !town_array.is_deleted(ai_capture_enemy_town_recno) &&
		 town_array[ai_capture_enemy_town_recno]->nation_recno == withNationRecno )
	{
		return -1;
	}

	//--- don't cease fire too soon after a war is declared ---//

	if( info.game_date < nationRelation->last_change_status_date + 60 + (100-pref_peacefulness) )		// more peaceful nation may cease fire sooner (but the minimum is 60 days).
		return -1;

	//------ if we're run short of money for war -----//

	Nation* withNation = nation_array[withNationRecno];

	if( !ai_should_spend_war(withNation->military_rank_rating(), 1) )		// if we shouldn't spend any more on war, then return 1
		return 1;

	//------------------------------------------------//

	int curRating = consider_alliance_rating(withNationRecno);

	//------------------------------------//
	//
	// Tend to be easilier to accept cease-fire if this nation's
	// military strength is weak.
	//
	// If the nation's peacefulness concern is high, it will
	// also be more likely to accept cease-fire.
	//
	//-------------------------------------//

	//--- if the enemy is more power than us, tend more to request cease-fire ---//
	
	curRating += total_enemy_military() - military_rank_rating();
	
	curRating += ai_trade_with_rating(withNationRecno) * (100+pref_trading_tendency) / 300;				// when we have excessive supply, we may want to cease-fire with our enemy

	curRating -= (military_rank_rating()-50)/2;					// if our military ranking is high, we may like to continue the war, otherwise the nation should try to cease-fire

	curRating -= nationRelation->started_war_on_us_count*10;		// the number of times this nation has started a war with us, the higher the number, the more unlikely we will accept cease-fire

	int acceptRating = pref_peacefulness/4;

	return curRating - acceptRating;
}
//------ End of function Nation::consider_cease_war ------//


//----- Begin of function Nation::consider_sell_food -----//
//
// talkMsg->talk_para1 - qty of food wanted to buy.
// talkMsg->talk_para2 - buying price offered for 10 food.
//
int Nation::consider_sell_food(TalkMsg* talkMsg)
{
	int relationStatus = get_relation_status(talkMsg->from_nation_recno);

	if( relationStatus == NATION_HOSTILE )
		return 0;

	//--- if after selling the food, the remaining is not enough for its own consumption for ? years ---//

	float newFood = food-talkMsg->talk_para1;
	float yearConsumption = (float) yearly_food_consumption();
	int offeredAmount = talkMsg->talk_para2;
	int relationLevel = get_relation(talkMsg->from_nation_recno)->ai_relation_level;

	if( newFood < 1000 + 1000 * pref_food_reserve / 100 )
		return 0;

	if( relationLevel >= 50 )
		offeredAmount += 5;				// increase the chance of selling food

	else if( relationLevel < 30 )		// decrease the chance of selling food
		offeredAmount -=5 ;

	//---- if we run short of cash, we tend to accept the offer ---//

	float fixedExpense = fixed_expense_365days();

	if( cash < fixedExpense )
		offeredAmount += (int) (20 * (fixedExpense-cash) / fixedExpense);

	//---------------------------------//

	float reserveYears = (float) (100+pref_food_reserve) / 100;			// 1 to 2 years

	if( yearly_food_change() > 0 &&
		 newFood > yearConsumption * reserveYears )
	{
		if( offeredAmount >= 10 )		// offered >= $10
		{
			return 1;
		}
		else         	// < $10, only if we have plenty of reserve
		{
			if( newFood > yearConsumption * reserveYears * 2 )
				return 1;
		}
	}
	else
	{
		if( offeredAmount >= 20 )
		{
			if( yearly_food_change() > 0 &&
				 newFood > yearConsumption * reserveYears / 2 )
			{
				return 1;
			}
		}

		if( offeredAmount >= 30 )
		{
			return yearly_food_change() > 0 ||
					 newFood > yearConsumption * reserveYears;
		}
	}

	return 0;
}
//------ End of function Nation::consider_sell_food ------//


//----- Begin of function Nation::should_consider_friendly -----//
//
int Nation::should_consider_friendly(int withNationRecno)
{
	Nation* withNation = nation_array[withNationRecno];

	//------- if this is a larger nation -------//

	if( overall_rank_rating() / 100 > 50 )
	{
		//--- big nations don't ally with their biggest opponents ---//

		int maxOverallRating=0;
		int biggestOpponentNationRecno=0;

		for( int i=nation_array.size() ; i>0 ; i-- )
		{
			if( nation_array.is_deleted(i) || i==nation_recno )
				continue;

			int overallRating = nation_array[i]->overall_rating;

			if( overallRating > maxOverallRating )
			{
				maxOverallRating = overallRating;
				biggestOpponentNationRecno = i;
			}
		}

		if( biggestOpponentNationRecno == withNationRecno )
			return 0;
	}

	//--- don't ally with nations with too low reputation ---//

	return withNation->reputation >= MIN(20, reputation) - 20;
}
//------ End of function Nation::should_consider_friendly -----//


//----- Begin of function Nation::consider_alliance_rating -----//
//
// Return a rating from 0 to 100 for whether this nation should ally
// with the given nation.
//
int Nation::consider_alliance_rating(int nationRecno)
{
	Nation* nationPtr = nation_array[nationRecno];

	//---- the current relation affect the alliance tendency ---//

	NationRelation* nationRelation = get_relation(nationRecno);

	int allianceRating = nationRelation->ai_relation_level-20;

	//--- if the nation has a bad record of starting wars with us before, decrease the rating ---//

	allianceRating -= nationRelation->started_war_on_us_count * 20;

	//------ add the trade rating -------//

	int tradeRating = trade_rating(nationRecno) +				// existing trade amount
							ai_trade_with_rating(nationRecno)/2;	// possible trade

	allianceRating += tradeRating;

	//---- if the nation's power is larger than us, it's a plus ----//

	int powerRating = nationPtr->military_rank_rating() - military_rank_rating();		// if the nation's power is larger than ours, it's good to form treaty with them

	if( powerRating > 0 )
		allianceRating += powerRating;

	return allianceRating;
}
//------ End of function Nation::consider_alliance_rating -----//


//----- Begin of function Nation::consider_take_tribute -----//
//
// talkMsg->talk_para1 - amount of the tribute.
//
int Nation::consider_take_tribute(TalkMsg* talkMsg)
{
	int cashSignificance = 100 * talkMsg->talk_para1 / MAX(1000, (int) cash);		

	//--- It does not necessarily want the tribute ---//

	int aiRelationLevel = get_relation(talkMsg->from_nation_recno)->ai_relation_level;

	if( true_profit_365days() > 0 &&
		 cashSignificance < (100-aiRelationLevel)/5 )
	{
		return 0;
	}

	//----------- take the tribute ------------//

	int relationChange = cashSignificance * (100+pref_cash_reserve) / 200; 

	change_ai_relation_level( talkMsg->from_nation_recno, relationChange );

	return 1;
}
//------ End of function Nation::consider_take_tribute ------//


//----- Begin of function Nation::consider_take_aid -----//
//
// talkMsg->talk_para1 - amount of the tribute.
//
int Nation::consider_take_aid(TalkMsg* talkMsg)
{
	int cashSignificance = 100 * talkMsg->talk_para1 / MAX(1000, (int) cash);		

	//--- It does not necessarily want the tribute ---//

	int aiRelationLevel = get_relation(talkMsg->from_nation_recno)->ai_relation_level;

	if( true_profit_365days() > 0 &&
		 cashSignificance < (100-aiRelationLevel)/5 )
	{
		return 0;
	}

	//----------- take the tribute ------------//

	int relationChange = cashSignificance * (100+pref_cash_reserve) / 200; 

	change_ai_relation_level( talkMsg->from_nation_recno, relationChange );

	return 1;
}
//------ End of function Nation::consider_take_aid ------//


//-------- Begin of static function has_sent_same_msg --------//
//
// Whether the nation has already sent out a message that is
// the same as the one it received.
//
static int has_sent_same_msg(TalkMsg* talkMsgPtr)
{
	TalkMsg talkMsg;

	memcpy( &talkMsg, talkMsgPtr, sizeof(TalkMsg) );

	talkMsg.from_nation_recno = talkMsg.to_nation_recno;
	talkMsg.to_nation_recno	  = talkMsg.from_nation_recno;

	return talk_res.is_talk_msg_exist(&talkMsg, 1);		// 1-check talk_para1 & talk_para2
}
//------ End of static function has_sent_same_msg ------//


//----- Begin of function Nation::consider_take_tech -----//
//
// talkMsg->talk_para1 - id. of the technology.
// talkMsg->talk_para2 - level of the technology. 
//
int Nation::consider_take_tech(TalkMsg* talkMsg)
{
	int ourTechLevel = tech_res[talkMsg->talk_para1]->get_nation_tech_level(nation_recno);

	if( ourTechLevel >= talkMsg->talk_para2 )
		return 0;

	int relationChange = (talkMsg->talk_para2-ourTechLevel) * (15+pref_use_weapon/10);

	change_ai_relation_level( talkMsg->from_nation_recno, relationChange );

	return 1;
}
//------ End of function Nation::consider_take_tech ------//


//----- Begin of function Nation::surplus_supply_rating -----//
//
// Return a rating from 0 to 100 indicating how much surplus
// of supply this nation has in markets.
//
int Nation::surplus_supply_rating()
{
	FirmMarket* firmMarket;
	int			stockQty, totalStockQty=0, totalStockSlot=0;

	for( int i=ai_market_count-1; i>=0 ; i-- )
	{
		firmMarket = (FirmMarket*) firm_array[ ai_market_array[i] ];

		err_when( firmMarket->firm_id != FIRM_MARKET );

		MarketGoods* marketGoods = firmMarket->market_goods_array;

		for( int j=0 ; j<MAX_MARKET_GOODS ; j++, marketGoods++ )
		{
			if( marketGoods->raw_id || marketGoods->product_raw_id )
			{
				stockQty = (int) marketGoods->stock_qty;

				totalStockQty += stockQty;
				totalStockSlot++;
			}
		}
	}

	if( totalStockSlot==0 )
		return 0;

	int avgStockQty = totalStockQty / totalStockSlot;

	return 100 * avgStockQty / MAX_MARKET_STOCK;
}
//------ End of function Nation::surplus_supply_rating ------//


//----- Begin of function Nation::consider_give_aid -----//
//
// talkMsg->talk_para1 - amount of the tribute.
//
int Nation::consider_give_aid(TalkMsg* talkMsg)
{
	//-------- don't give tribute too frequently -------//

	NationRelation* nationRelation = get_relation(talkMsg->from_nation_recno);

	if( info.game_date <
		 nationRelation->last_talk_reject_date_array[TALK_GIVE_AID-1]
		 + 365 - pref_allying_tendency )
	{
		return 0;
	}

	//--------------------------------------------------//

	int importanceRating = (int) nationRelation->good_relation_duration_rating;

	if( nationRelation->status >= NATION_FRIENDLY &&
		 ai_should_spend( importanceRating, talkMsg->talk_para1 ) )        // 0-importance is 0
	{
		if( info.game_date > nationRelation->last_change_status_date
			 + 720 - pref_allying_tendency )						// we have allied with this nation for quite some while
		{
			nationRelation->last_talk_reject_date_array[TALK_GIVE_AID-1] = info.game_date;
			return 1;
		}
	}

	return 0;
}
//------ End of function Nation::consider_give_aid ------//


//----- Begin of function Nation::consider_give_tribute -----//
//
// talkMsg->talk_para1 - amount of the tribute.
//
int Nation::consider_give_tribute(TalkMsg* talkMsg)
{
	//-------- don't give tribute too frequently -------//

	NationRelation* nationRelation = get_relation(talkMsg->from_nation_recno);

	if( info.game_date <
		 nationRelation->last_talk_reject_date_array[TALK_GIVE_TRIBUTE-1] + 365 - pref_allying_tendency )
	{
		return 0;
	}

	//---------------------------------------------//

	int relationStatus = get_relation_status(talkMsg->from_nation_recno);
	Nation* fromNation = nation_array[talkMsg->from_nation_recno];

	if( true_profit_365days() < 0 )		// don't give tribute if we are losing money
		return 0;

	int reserveYears = 1 + 3 * pref_cash_reserve / 100;			// 1 to 4 years

	if( cash-talkMsg->talk_para1 < fixed_expense_365days() * reserveYears )
		return 0;

	int militaryDiff = fromNation->military_rank_rating() - military_rank_rating();

	if( militaryDiff > 10+pref_military_courage/2 )
	{
		nationRelation->last_talk_reject_date_array[TALK_GIVE_TRIBUTE-1] = info.game_date;
		return 1;
	}

	return 0;
}
//------ End of function Nation::consider_give_tribute ------//


//----- Begin of function Nation::consider_give_tech -----//
//
// Consider giving the latest level of the technology to the nation.
//
// talkMsg->talk_para1 - id. of the technology.
//
int Nation::consider_give_tech(TalkMsg* talkMsg)
{
	//-------- don't give tribute too frequently -------//

	NationRelation* nationRelation = get_relation(talkMsg->from_nation_recno);

	if( info.game_date <
		 nationRelation->last_talk_reject_date_array[TALK_GIVE_TECH-1] + 365 - pref_allying_tendency )
	{
		return 0;
	}

	//----------------------------------------------------//

	int importanceRating = (int) nationRelation->good_relation_duration_rating;

	if( nationRelation->status == NATION_ALLIANCE &&
		 importanceRating + pref_allying_tendency/10 > 30 )
	{
		nationRelation->last_talk_reject_date_array[TALK_GIVE_TECH-1] = info.game_date;
		return 1;
	}

	return 0;
}
//------ End of function Nation::consider_give_tech ------//


//----- Begin of function Nation::consider_declare_war -----//
//
// Consider the request of declaring war on the target nation.
//
// talk_para1 - the recno nation to declare war with.
//
int Nation::consider_declare_war(TalkMsg* talkMsg)
{
	//--- if it even won't consider trade embargo, there is no reason that it will consider declaring war ---//

	if( !consider_trade_embargo(talkMsg) )
		return 0;

	//---------------------------------------//

	int fromRelationRating    = ai_overall_relation_rating(talkMsg->from_nation_recno);
	int againstRelationRating = ai_overall_relation_rating(talkMsg->talk_para1);

	Nation* againstNation = nation_array[talkMsg->talk_para1];

	NationRelation* fromRelation 	  = get_relation(talkMsg->from_nation_recno);
	NationRelation* againstRelation = get_relation(talkMsg->talk_para1);

	//--- if we don't have a good enough relation with the requesting nation, turn down the request ---//

	if( fromRelation->good_relation_duration_rating < 10 )
		return 0;

	//--- if we are more friendly with the against nation than the requesting nation, turn down the request ---//

	if( againstRelation->good_relation_duration_rating >
		 fromRelation->good_relation_duration_rating )
	{
		return 0;
	}

	//--- if the nation is having a financial difficulty, it won't agree ---//

	if( cash < 2000 * pref_cash_reserve / 100  )
		return 0;

	//--------------------------------------------//

	int acceptRating = 100 + againstNation->total_enemy_military() -
							 military_rank_rating();

	//--- it won't declare war with a friendly or allied nation easily ---//

	if( againstRelation->status >= NATION_FRIENDLY )		// no need to handle NATION_ALLIANCE separately as ai_overall_relation_relation() has already taken it into account
		acceptRating += 100;

	return fromRelationRating - againstRelationRating > acceptRating;
}
//------ End of function Nation::consider_declare_war ------//


//----- Begin of function Nation::consider_trade_embargo -----//
//
int Nation::consider_trade_embargo(TalkMsg* talkMsg)
{
	int fromRelationRating    = ai_overall_relation_rating(talkMsg->from_nation_recno);
	int againstRelationRating = ai_overall_relation_rating(talkMsg->talk_para1);

	NationRelation* fromRelation 	  = get_relation(talkMsg->from_nation_recno);
	NationRelation* againstRelation = get_relation(talkMsg->talk_para1);

	//--- if we don't have a good enough relation with the requesting nation, turn down the request ---//

	if( fromRelation->good_relation_duration_rating < 5 )
		return 0;

	//--- if we are more friendly with the against nation than the requesting nation, turn down the request ---//

	if( againstRelation->good_relation_duration_rating >
		 fromRelation->good_relation_duration_rating )
	{
		return 0;
	}

	//--- if we have a large trade with the against nation or have a larger trade with the against nation than the requesting nation ---//

	int fromTrade 	  = trade_rating(talkMsg->from_nation_recno);
	int againstTrade = trade_rating(talkMsg->talk_para1);

	if( againstTrade > 40 ||
		 ( againstTrade > 10 && againstTrade - fromTrade > 15 ) )
	{
		return 0;
	}

	//--- if the nation is having a financial difficulty, it won't agree ---//

	if( cash < 2000 * pref_cash_reserve / 100  )
		return 0;

	//--------------------------------------------//

	int acceptRating = 75;

	//--- it won't declare war with a friendly or allied nation easily ---//

	if( againstRelation->status >= NATION_FRIENDLY )		// no need to handle NATION_ALLIANCE separately as ai_overall_relation_relation() has already taken it into account
		acceptRating += 100;

	return fromRelationRating - againstRelationRating > acceptRating;
}
//------ End of function Nation::consider_trade_embargo ------//


//----- Begin of function Nation::consider_military_aid -----//
//
int Nation::consider_military_aid(TalkMsg* talkMsg)
{
	Nation*			 fromNation   = nation_array[talkMsg->from_nation_recno];
	NationRelation* fromRelation = get_relation(talkMsg->from_nation_recno);

	//----- don't aid too frequently ------//

	if( info.game_date < fromRelation->last_military_aid_date + 200 - pref_allying_tendency )
		return 0;

	//------- only when the AI relation >= 60 --------//

	if( fromRelation->ai_relation_level < 60 )
		return 0;

	//--- if the requesting nation is not at war now ----//

	if( !fromNation->is_at_war() )
		return 0;

	//---- can't aid if we are at war ourselves -----//

	if( is_at_war() )
		return 0;

	//--- if the nation is having a financial difficulty, it won't agree ---//

	if( cash < 2000 * pref_cash_reserve / 100  )
		return 0;

	//----- can't aid if we are too weak ourselves ---//

	if( ai_general_count*10 + total_human_count < 100-pref_military_courage/2 )
		return 0;

	//----- see what units are attacking the nation -----//

	if( unit_array.is_deleted(fromNation->last_attacker_unit_recno) )
		return 0;

	Unit* unitPtr = unit_array[ fromNation->last_attacker_unit_recno ];

	if( unitPtr->nation_recno == nation_recno )		// if it's our own units
		return 0;

	if( unitPtr->nation_recno == 0 )
		return 0;

	if( !unitPtr->is_visible() )
		return 0;

	//------ only attack if it's a common enemy to us and our ally -----//

	if( get_relation(unitPtr->nation_recno)->status != NATION_HOSTILE )
		return 0;

	//------- calculate the combat level of the target units there ------//

	int hasWar;

	int targetCombatLevel = mobile_defense_combat_level( unitPtr->next_x_loc(), unitPtr->next_y_loc(),
									unitPtr->nation_recno, 0, hasWar );

	if( ai_attack_target(unitPtr->next_x_loc(), unitPtr->next_y_loc(), targetCombatLevel, 0, 1 ) )		//0-not defense mode, 1-just move to flag 
	{
		fromRelation->last_military_aid_date = info.game_date;
		return 1;
	}

	return 0;
}
//------ End of function Nation::consider_military_aid ------//


//----- Begin of function Nation::consider_accept_surrender_request -----//
//
// Consider accepting the cash offer and sell the throne to another kingdom.
//
// talkMsg->talk_para1 - the amount offered.
//
int Nation::consider_accept_surrender_request(TalkMsg* talkMsg)
{
	Nation* nationPtr = nation_array[talkMsg->from_nation_recno];
	int    offeredAmt = talkMsg->talk_para1 * 10;			// *10 to restore its original value which has been divided by 10 to cope with <short> upper limit

	//---- don't surrender to the player if the player is already the most powerful nation ---//

	if( !nationPtr->is_ai() && config.ai_aggressiveness >= OPTION_HIGH )
	{
		if( nation_array.max_overall_nation_recno == nationPtr->nation_recno )
			return 0;
	}

	//--- if we are running out of cash, ignore all normal thinking ---//

	if( !(cash < 100 && profit_365days() < 0) )
	{
		//----- never surrender to a weaker nation ------//

		if( nationPtr->overall_rank_rating() < overall_rank_rating() )
			return 0;

		//------ don't surrender if we are still strong -----//

		if( overall_rank_rating() > 30 + pref_peacefulness/4 )		// 30 to 55
			return 0;

		//---- don't surrender if our cash is more than the amount they offered ----//

		if( offeredAmt < cash * (75+pref_cash_reserve/2) / 100 )		// 75% to 125%
			return 0;

		//-- if there are only two nations left, don't surrender if we still have some power --//

		if( nation_array.nation_count == 2 )
		{
			if( overall_rank_rating() > 20 - 10 * pref_military_courage / 100 )
				return 0;
		}
	}

	//-------------------------------------//

	int surrenderToRating = ai_surrender_to_rating(talkMsg->from_nation_recno);

	surrenderToRating += 100 * offeredAmt / 13000;

	int acceptRating = overall_rank_rating()*13 + 100;

	//------ AI aggressiveness effects -------//

	switch( config.ai_aggressiveness )
	{
		case OPTION_HIGH:
			if( nationPtr->is_ai() )		// tend to accept AI kingdom offer easier
				acceptRating -= 75;
			else
				acceptRating += 75;
			break;

		case OPTION_VERY_HIGH:
			if( nationPtr->is_ai() )		// tend to accept AI kingdom offer easier
				acceptRating -= 150;
			else
				acceptRating += 150;
			break;
	}

	return surrenderToRating > acceptRating;
}
//------ End of function Nation::consider_accept_surrender_request ------//


//----- Begin of function Nation::ai_overall_relation_rating -----//
//
// Return the overall relation rating of this nation with the
// specific nation.
//
int Nation::ai_overall_relation_rating(int withNationRecno)
{
	NationRelation* nationRelation = get_relation(withNationRecno);
	Nation* 			 nationPtr = nation_array[withNationRecno];

	int overallRating = nationRelation->ai_relation_level +
							  (int) nationRelation->good_relation_duration_rating +
							  (int) nationPtr->reputation +
							  nationPtr->military_rank_rating() +
							  trade_rating(withNationRecno) +
							  ai_trade_with_rating(withNationRecno)/2 +
							  nationPtr->total_alliance_military();

	return overallRating;
}
//------ End of function Nation::ai_overall_relation_rating ------//
