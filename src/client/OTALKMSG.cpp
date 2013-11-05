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

//Filename    : OTALKMSG.CPP
//Description : Object TalkMsg

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

//------ Define function pointers array ------//

static char talk_msg_reply_needed_array[] =
{
	1,
	1,
	1,
	0,
   0,
	0,
	1,
	1,
	1,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
};

//------- Begin of function TalkMsg::TalkMsg --------//
//
TalkMsg::TalkMsg()
{
	memset( this, 0, sizeof(TalkMsg) );
}
//-------- End of function TalkMsg::TalkMsg ---------//


//------- Begin of function TalkMsg::process_accepted_reply --------//
//
void TalkMsg::process_accepted_reply()
{
	Nation* toNation   = nation_array[to_nation_recno];
	Nation* fromNation = nation_array[from_nation_recno];

	NationRelation* fromRelation = fromNation->get_relation(to_nation_recno);
	NationRelation* toRelation = toNation->get_relation(from_nation_recno);

	int goodRelationDec=0;		// whether the message is for requesting help.

	switch(talk_id)
	{
		case TALK_PROPOSE_TRADE_TREATY:
			toNation->set_trade_treaty(from_nation_recno, 1);
			break;

		case TALK_PROPOSE_FRIENDLY_TREATY:
			toNation->form_friendly_treaty(from_nation_recno);
			break;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			toNation->form_alliance_treaty(from_nation_recno);
			break;

		case TALK_END_TRADE_TREATY:
			toNation->set_trade_treaty(from_nation_recno, 0);

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
			fromNation->end_treaty(to_nation_recno, NATION_NEUTRAL);

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

				TalkMsg talkMsg;

				memset(&talkMsg, 0, sizeof(TalkMsg));

				talkMsg.to_nation_recno   = (char) talk_para1;
				talkMsg.from_nation_recno = to_nation_recno;
				talkMsg.talk_id  			  = TALK_END_TRADE_TREATY;

				talk_res.send_talk_msg( &talkMsg, COMMAND_AUTO );
				goodRelationDec = 4;
			}
			break;

		case TALK_REQUEST_CEASE_WAR:
			unit_array.stop_war_between(to_nation_recno, from_nation_recno);
			toNation->set_relation_status(from_nation_recno, NATION_TENSE);

			fromRelation->last_talk_reject_date_array[TALK_DECLARE_WAR-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_DECLARE_WAR-1]   = info.game_date;
			break;

		case TALK_REQUEST_DECLARE_WAR:
			if( fromNation->get_relation_status(talk_para1) == NATION_HOSTILE )	// the requesting nation must be at war with the enemy
			{

				//-- if we are currently allied or friendly with the nation, we need to terminate the friendly/alliance treaty first --//

				TalkMsg talkMsg;

				if( toNation->get_relation_status(talk_para1) == NATION_ALLIANCE )
				{
					memset(&talkMsg, 0, sizeof(TalkMsg));

					talkMsg.to_nation_recno   = (char) talk_para1;
					talkMsg.from_nation_recno = to_nation_recno;
					talkMsg.talk_id  			  = TALK_END_ALLIANCE_TREATY;

					talk_res.send_talk_msg( &talkMsg, COMMAND_AUTO );
				}

				else if( toNation->get_relation_status(talk_para1) == NATION_FRIENDLY )
				{
					memset(&talkMsg, 0, sizeof(TalkMsg));

					talkMsg.to_nation_recno   = (char) talk_para1;
					talkMsg.from_nation_recno = to_nation_recno;
					talkMsg.talk_id  			  = TALK_END_FRIENDLY_TREATY;

					talk_res.send_talk_msg( &talkMsg, COMMAND_AUTO );
				}

				//--- send a declare war message to the target kingdom ---//

				memset(&talkMsg, 0, sizeof(TalkMsg));

				talkMsg.to_nation_recno   = (char) talk_para1;
				talkMsg.from_nation_recno = to_nation_recno;
				talkMsg.talk_id  			  = TALK_DECLARE_WAR;

				talk_res.send_talk_msg( &talkMsg, COMMAND_AUTO );

				//----------------------------------------------------//

				goodRelationDec = 10;
			}
			break;

		case TALK_REQUEST_BUY_FOOD:
		{
			int buyCost = talk_para1 * talk_para2 / 10;

			fromNation->add_food( (float) talk_para1 );
			fromNation->add_expense( EXPENSE_IMPORTS, (float) buyCost, 0 );
			toNation->consume_food( (float) talk_para1 );
			toNation->add_income( INCOME_EXPORTS, (float) buyCost, 0 );
			break;
		}

		case TALK_DECLARE_WAR:
			toRelation->started_war_on_us_count++;			// how many times this nation has started a war with us, the more the times the worse this nation is.
			toNation->set_relation_status(from_nation_recno, NATION_HOSTILE);

			fromRelation->last_talk_reject_date_array[TALK_REQUEST_CEASE_WAR-1] = info.game_date;
			toRelation->last_talk_reject_date_array[TALK_REQUEST_CEASE_WAR-1]   = info.game_date;

			//--- decrease reputation of the nation which declares war ---//

			if( toNation->reputation > 0 )
				fromNation->change_reputation( -toNation->reputation * 20 / 100 );
			break;

		case TALK_GIVE_TRIBUTE:
		case TALK_GIVE_AID:
			fromNation->give_tribute( to_nation_recno, talk_para1 );
			break;

		case TALK_DEMAND_TRIBUTE:
		case TALK_DEMAND_AID:
			toNation->give_tribute( from_nation_recno, talk_para1 );
			goodRelationDec = talk_para1/200;
			break;

		case TALK_GIVE_TECH:
			fromNation->give_tech( to_nation_recno, talk_para1, talk_para2 );
			break;

		case TALK_DEMAND_TECH:
			talk_para2 = tech_res[talk_para1]->get_nation_tech_level(to_nation_recno);		// get the latest tech version id. of the agreed nation.
			toNation->give_tech( from_nation_recno, talk_para1, talk_para2 );
			goodRelationDec = talk_para2*3;
			break;

		case TALK_REQUEST_SURRENDER:
		{
			float offeredAmt = (float) talk_para1 * 10;		// * 10 is to restore its original value. It has been divided by 10 to cope with the upper limit of <short>

			toNation->add_income(INCOME_TRIBUTE, offeredAmt);
			fromNation->add_expense(EXPENSE_TRIBUTE, offeredAmt);

			toNation->surrender(from_nation_recno);
			break;
		}

		case TALK_SURRENDER:
			fromNation->surrender(to_nation_recno);
			break;

		default:
			err_here();
	}

	//---- if the nation accepts a message that is requesting help, then decrease its good_relation_duration_rating, so it won't accept so easily next time ---//

	if( goodRelationDec )
		toRelation->good_relation_duration_rating -= (float) goodRelationDec;

	//-- remove talk messges that are the same but sent from the opposite nation --//

	TalkMsg talkMsg;

	talkMsg.from_nation_recno = to_nation_recno;
	talkMsg.to_nation_recno	  = from_nation_recno;
	talkMsg.talk_id			  = talk_id;

	int loopCount=0;

	while(1)
	{
		err_when( loopCount++ > 100 );

		int talkMsgRecno = talk_res.is_talk_msg_exist(&talkMsg, 0);		// don't check talk_para1 & talk_para2

		if( talkMsgRecno )
			talk_res.del_talk_msg(talkMsgRecno);
		else
			break;
	}
}
//-------- End of function TalkMsg::process_accepted_reply ---------//


//----- Begin of function TalkMsg::is_reply_needed ------//
//
// Whether a reply is needed for this message.
//
int TalkMsg::is_reply_needed()
{
	return talk_msg_reply_needed_array[talk_id-1];
}
//------- End of function TalkMsg::is_reply_needed ------//


//----- Begin of function TalkMsg::is_valid_to_reply ------//
//
// Return whether this message is still valid. Only
// if this message is valid, the receiver can answer
// this message.
//
int TalkMsg::is_valid_to_reply()
{
	//-----------------------------------------------------//
	// When a diplomatic message is sent, the receiver must
	//	reply the message with a month.
	//-----------------------------------------------------//

	#define TALK_MSG_VALID_DAYS	30

	if( info.game_date > date + TALK_MSG_VALID_DAYS )
		return 0;

	//--- check if the nations are still there -----//

	if( nation_array.is_deleted(from_nation_recno) )
		return 0;

	if( nation_array.is_deleted(to_nation_recno) )
		return 0;

	//--------------------------------------//

	if( !talk_res.can_send_msg(to_nation_recno, from_nation_recno, talk_id) )
		return 0;

	//--------------------------------------//

	Nation* toNation   = nation_array[to_nation_recno];
	Nation* fromNation = nation_array[from_nation_recno];

	switch( talk_id )
	{
		case TALK_REQUEST_TRADE_EMBARGO:
			if( nation_array.is_deleted(talk_para1) )
				return 0;

			//-- if the requesting nation is itself trading with the target nation --//

			if( fromNation->get_relation(talk_para1)->trade_treaty )
				return 0;

			//-- or if the requested nation already doesn't have a trade treaty with the nation --//

			if( !toNation->get_relation(talk_para1)->trade_treaty )
				return 0;

			break;

		case TALK_REQUEST_DECLARE_WAR:
			if( nation_array.is_deleted(talk_para1) )
				return 0;

			//-- if the requesting nation is no longer hostile with the nation --//

			if( fromNation->get_relation_status(talk_para1) != NATION_HOSTILE )
				return 0;

			//-- or if the requested nation has become hostile with the nation --//

			if( toNation->get_relation_status(talk_para1) == NATION_HOSTILE )
				return 0;

			break;

		case TALK_REQUEST_BUY_FOOD:
			return fromNation->cash >= talk_para1 * talk_para2 / 10;

		case TALK_GIVE_TRIBUTE:
		case TALK_GIVE_AID:
			return fromNation->cash >= talk_para1;

		case TALK_GIVE_TECH:
/* 		// still display the message even if the nation already has the technology
			//---- if the nation has acquired the technology itself ----//

			if( tech_res[talk_para1]->get_nation_tech_level(to_nation_recno) >= talk_para2 )
				return 0;
*/
			break;

		case TALK_DEMAND_TECH:
/*
			//---- if the requesting nation has acquired the technology itself ----//

			if( tech_res[talk_para1]->get_nation_tech_level(from_nation_recno) >= talk_para2 )
				return 0;
*/
			break;
	}

	return 1;
}
//------- End of function TalkMsg::is_valid_to_reply ------//


//----- Begin of function TalkMsg::is_valid_to_disp ------//
//
// Return whether this message is still valid. Only
// if this message is valid, the receiver can answer
// this message.
//
int TalkMsg::is_valid_to_disp()
{
	//--- check if the nations are still there -----//

	if( nation_array.is_deleted(from_nation_recno) )
		return 0;

	if( nation_array.is_deleted(to_nation_recno) )
		return 0;

	//--------------------------------------//

	Nation* toNation   = nation_array[to_nation_recno];
	Nation* fromNation = nation_array[from_nation_recno];

	switch( talk_id )
	{
		case TALK_REQUEST_TRADE_EMBARGO:
			if( nation_array.is_deleted(talk_para1) )
				return 0;
			break;

		case TALK_REQUEST_DECLARE_WAR:
			if( nation_array.is_deleted(talk_para1) )
				return 0;
			break;
	}

	return 1;
}
//------- End of function TalkMsg::is_valid_to_disp ------//

//----- Begin of function TalkMsg::can_accept ------//
//
// Return whether the receiver can accept this message
// if he wants to.
//
int TalkMsg::can_accept()
{
	Nation* toNation = nation_array[to_nation_recno];

	switch( talk_id )
	{
		case TALK_REQUEST_BUY_FOOD:
			return toNation->food >= talk_para1;

		case TALK_DEMAND_TRIBUTE:
		case TALK_DEMAND_AID:
			return toNation->cash >= talk_para1;

		case TALK_DEMAND_TECH:		// the requested nation has the technology
			return tech_res[talk_para1]->get_nation_tech_level(to_nation_recno) > 0;
	}

	return 1;
}
//------- End of function TalkMsg::can_accept ------//


//----- Begin of function TalkMsg::from_nation_name ------//
//
char* TalkMsg::from_nation_name()
{
	static String str;

	str = nation_array[from_nation_recno]->nation_name();

	//------ add nation color bar -------//

	if( talk_res.msg_add_nation_color )
	{
		char colorCodeStr[] = " 0";

		colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_array[from_nation_recno]->color_scheme_id;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function TalkMsg::from_nation_name ------//


//----- Begin of function TalkMsg::to_nation_name ------//
//
char* TalkMsg::to_nation_name()
{
	static String str;

	str = nation_array[to_nation_recno]->nation_name();

	//------ add nation color bar -------//

	if( talk_res.msg_add_nation_color )
	{
		char colorCodeStr[] = " 0";

		colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_array[to_nation_recno]->color_scheme_id;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function TalkMsg::to_nation_name ------//


//----- Begin of function TalkMsg::from_king_name ------//
//
char* TalkMsg::from_king_name()
{
	static String str;

	str = nation_array[from_nation_recno]->king_name();

	//------ add nation color bar -------//

	if( talk_res.msg_add_nation_color )
	{
		char colorCodeStr[] = " 0";

		colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_array[from_nation_recno]->color_scheme_id;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function TalkMsg::from_king_name ------//


//----- Begin of function TalkMsg::to_king_name ------//
//
char* TalkMsg::to_king_name()
{
	static String str;

	str = nation_array[to_nation_recno]->king_name();

	//------ add nation color bar -------//

	if( talk_res.msg_add_nation_color )
	{
		char colorCodeStr[] = " 0";

		colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_array[to_nation_recno]->color_scheme_id;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function TalkMsg::to_king_name ------//


//----- Begin of function TalkMsg::nation_color_code_str ------//
//
char* TalkMsg::nation_color_code_str(int nationRecno)
{
	static char colorCodeStr[] = " 0";

	colorCodeStr[1] = FIRST_NATION_COLOR_CODE_IN_TEXT + nation_array[nationRecno]->color_scheme_id;

	return colorCodeStr;
}
//------- End of function TalkMsg::nation_color_code_str ------//

