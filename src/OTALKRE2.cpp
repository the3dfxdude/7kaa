/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2022 Jesse Allen
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

//Filename    : OTALKRE2.CPP
//Description : Object TalkRes Message Processing

#include <ONATION.h>
#include <OTALKRES.h>
#include <OTECHRES.h>

// ----------- Define static functions ----------//

static void delete_msg_in_reverse(TalkMsg *talkMsg);

//------- Begin of function TalkRes::process_accepted_reply --------//
//
//###### begin jesse 2022/10/2 #######//
// talkMsg may be invalidated after calling send_msg_now. If calling
// send_msg_now, do it after processing the talkMsg.
//###### end jesse 2022/10/2 #######//
//
void TalkRes::process_accepted_reply(TalkMsg *talkMsg)
{
	//---- delete duplicate message in reverse now this has been accepted ----//
	delete_msg_in_reverse(talkMsg); // do this now because talkMsg may be invalid after sending a reply

	Nation* toNation   = nation_array[talkMsg->to_nation_recno];
	Nation* fromNation = nation_array[talkMsg->from_nation_recno];

	NationRelation* fromRelation = fromNation->get_relation(talkMsg->to_nation_recno);
	NationRelation* toRelation = toNation->get_relation(talkMsg->from_nation_recno);

	int goodRelationDec=0;		// whether the message is for requesting help.

	switch(talkMsg->talk_id)
	{
		case TALK_PROPOSE_TRADE_TREATY:
			toNation->set_trade_treaty(talkMsg->from_nation_recno, 1);
			break;

		case TALK_PROPOSE_FRIENDLY_TREATY:
			toNation->form_friendly_treaty(talkMsg->from_nation_recno);
			break;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			toNation->form_alliance_treaty(talkMsg->from_nation_recno);
			break;

		case TALK_END_TRADE_TREATY:
			toNation->set_trade_treaty(talkMsg->from_nation_recno, 0);

			//---- set reject date on proposing treaty ----//

			fromRelation->last_talk_reject_date_array[TALK_PROPOSE_TRADE_TREATY-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_PROPOSE_TRADE_TREATY-1]   = info.game_date;

			fromRelation->last_talk_reject_date_array[TALK_END_TRADE_TREATY-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_END_TRADE_TREATY-1]   = info.game_date;

			//----- decrease reputation -----//

			if( toNation->reputation > 0 )
				fromNation->change_reputation( -toNation->reputation * 5 / 100 );
			break;

		case TALK_END_FRIENDLY_TREATY:
		case TALK_END_ALLIANCE_TREATY:
			fromNation->end_treaty(talkMsg->to_nation_recno, NATION_NEUTRAL);

			//---- set reject date on proposing treaty ----//
			//
			// If a friendly treaty is rejected, assuming an alliance treaty
			// is even more impossible. (thus set reject date on both friendly
			// and alliance treaties.)
			//
			// If a alliance treaty is rejected, only set reject date on
			// alliance treaty, it may still try proposing friendly treaty later.
			//
			//---------------------------------------------//

			fromRelation->last_talk_reject_date_array[TALK_PROPOSE_FRIENDLY_TREATY-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_PROPOSE_FRIENDLY_TREATY-1]   = info.game_date;

			fromRelation->last_talk_reject_date_array[TALK_PROPOSE_ALLIANCE_TREATY-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_PROPOSE_ALLIANCE_TREATY-1]   = info.game_date;
			break;

		case TALK_REQUEST_MILITARY_AID:
			goodRelationDec = 10;
			break;

		case TALK_REQUEST_TRADE_EMBARGO:
			{
				//--- send an end treaty message to the target kingdom ---//

				TalkMsg talkMsgReply;

				memset(&talkMsgReply, 0, sizeof(TalkMsg));

				talkMsgReply.to_nation_recno   = (char) talkMsg->talk_para1;
				talkMsgReply.from_nation_recno = talkMsg->to_nation_recno;
				talkMsgReply.talk_id           = TALK_END_TRADE_TREATY;

				send_talk_msg( &talkMsgReply, COMMAND_AUTO );
				goodRelationDec = 4;
			}
			break;

		case TALK_REQUEST_CEASE_WAR:
			unit_array.stop_war_between(talkMsg->to_nation_recno, talkMsg->from_nation_recno);
			toNation->set_relation_status(talkMsg->from_nation_recno, NATION_TENSE);

			fromRelation->last_talk_reject_date_array[TALK_DECLARE_WAR-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_DECLARE_WAR-1]   = info.game_date;
			break;

		case TALK_REQUEST_DECLARE_WAR:
			if( fromNation->get_relation_status(talkMsg->talk_para1) == NATION_HOSTILE )	// the requesting nation must be at war with the enemy
			{
				TalkMsg talkMsgReply;
				memset(&talkMsgReply, 0, sizeof(TalkMsg));

				talkMsgReply.to_nation_recno   = (char) talkMsg->talk_para1;
				talkMsgReply.from_nation_recno = talkMsg->to_nation_recno;
				talkMsgReply.talk_id           = TALK_DECLARE_WAR;

				//-- if we are currently allied or friendly with the nation, we need to terminate the friendly/alliance treaty first --//

				if( toNation->get_relation_status(talkMsg->talk_para1) == NATION_ALLIANCE )
				{
					TalkMsg breakTreatyMsg;
					memset(&breakTreatyMsg, 0, sizeof(TalkMsg));

					breakTreatyMsg.to_nation_recno   = (char) talkMsg->talk_para1;
					breakTreatyMsg.from_nation_recno = talkMsg->to_nation_recno;
					breakTreatyMsg.talk_id           = TALK_END_ALLIANCE_TREATY;

					send_talk_msg( &breakTreatyMsg, COMMAND_AUTO );
				}

				else if( toNation->get_relation_status(talkMsg->talk_para1) == NATION_FRIENDLY )
				{
					TalkMsg breakTreatyMsg;
					memset(&breakTreatyMsg, 0, sizeof(TalkMsg));

					breakTreatyMsg.to_nation_recno   = (char) talkMsg->talk_para1;
					breakTreatyMsg.from_nation_recno = talkMsg->to_nation_recno;
					breakTreatyMsg.talk_id           = TALK_END_FRIENDLY_TREATY;
					send_talk_msg( &breakTreatyMsg, COMMAND_AUTO );
				}

				//--- send a declare war message to the target kingdom ---//

				send_talk_msg( &talkMsgReply, COMMAND_AUTO );

				//----------------------------------------------------//

				goodRelationDec = 10;
			}
			break;

		case TALK_REQUEST_BUY_FOOD:
		{
			int buyCost = talkMsg->talk_para1 * talkMsg->talk_para2 / 10;

			fromNation->add_food( (float) talkMsg->talk_para1 );
			fromNation->add_expense( EXPENSE_IMPORTS, (float) buyCost, 0 );
			toNation->consume_food( (float) talkMsg->talk_para1 );
			toNation->add_income( INCOME_EXPORTS, (float) buyCost, 0 );
			break;
		}

		case TALK_DECLARE_WAR:
			toRelation->started_war_on_us_count++;			// how many times this nation has started a war with us, the more the times the worse this nation is.
			toNation->set_relation_status(talkMsg->from_nation_recno, NATION_HOSTILE);

			fromRelation->last_talk_reject_date_array[TALK_REQUEST_CEASE_WAR-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_REQUEST_CEASE_WAR-1]   = info.game_date;

			//--- decrease reputation of the nation which declares war ---//

			if( toNation->reputation > 0 )
				fromNation->change_reputation( -toNation->reputation * 20 / 100 );
			break;

		case TALK_GIVE_TRIBUTE:
		case TALK_GIVE_AID:
			fromNation->give_tribute( talkMsg->to_nation_recno, talkMsg->talk_para1 );
			break;

		case TALK_DEMAND_TRIBUTE:
		case TALK_DEMAND_AID:
			toNation->give_tribute( talkMsg->from_nation_recno, talkMsg->talk_para1 );
			goodRelationDec = talkMsg->talk_para1/200;
			break;

		case TALK_GIVE_TECH:
			fromNation->give_tech( talkMsg->to_nation_recno, talkMsg->talk_para1, talkMsg->talk_para2 );
			break;

		case TALK_DEMAND_TECH:
			talkMsg->talk_para2 = tech_res[talkMsg->talk_para1]->get_nation_tech_level(talkMsg->to_nation_recno);		// get the latest tech version id. of the agreed nation.
			toNation->give_tech( talkMsg->from_nation_recno, talkMsg->talk_para1, talkMsg->talk_para2 );
			goodRelationDec = talkMsg->talk_para2*3;
			break;

		case TALK_REQUEST_SURRENDER:
		{
			float offeredAmt = (float) talkMsg->talk_para1 * 10;		// * 10 is to restore its original value. It has been divided by 10 to cope with the upper limit of <short>

			toNation->add_income(INCOME_TRIBUTE, offeredAmt);
			fromNation->add_expense(EXPENSE_TRIBUTE, offeredAmt);

			toNation->surrender(talkMsg->from_nation_recno);
			break;
		}

		case TALK_SURRENDER:
			fromNation->surrender(talkMsg->to_nation_recno);
			break;

		default:
			err_here();
	}

	//---- if the nation accepts a message that is requesting help, then decrease its good_relation_duration_rating, so it won't accept so easily next time ---//

	if( goodRelationDec )
		toRelation->good_relation_duration_rating -= (float) goodRelationDec;
}
//-------- End of function TalkRes::process_accepted_reply ---------//


//------ Begin of static function delete_msg_in_reverse ------//
//
// remove same messages that are sent in reverse
//
static void delete_msg_in_reverse(TalkMsg *talkMsg)
{
	TalkMsg talkMsgReverse;

	talkMsgReverse.from_nation_recno = talkMsg->to_nation_recno;
	talkMsgReverse.to_nation_recno   = talkMsg->from_nation_recno;
	talkMsgReverse.talk_id           = talkMsg->talk_id;

	int loopCount=0;

	while(1)
	{
		err_when( loopCount++ > 100 );

		int talkMsgRecno = talk_res.is_talk_msg_exist(&talkMsgReverse, 0);		// don't check talk_para1 & talk_para2

		if( talkMsgRecno )
			talk_res.del_talk_msg(talkMsgRecno);
		else
			break;
	}
}
//------ End of static function delete_msg_in_reverse ------//

