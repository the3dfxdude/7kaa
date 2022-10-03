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

//Filename    : OTALKRES.CPP
//Description : Object Talk

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
#include <OREMOTE.h>
// #### begin Gilbert 9/10 ######//
#include <OSE.h>
// #### end Gilbert 9/10 ######//
#include "gettext.h"

//---------- the subtitle area ----------//

enum { TALK_X1 = ZOOM_X1+16,
		 TALK_Y1 = ZOOM_Y1+252,
		 TALK_X2 = ZOOM_X2-16,
		 TALK_Y2 = ZOOM_Y2-86,
	  };

enum { TALK_LINE_HEIGHT = 18 };

//------- define constant --------//

#define MESSAGE_SENT_STR		_("The message has been sent.")

//------- define static vars -------//

static String nation_name_str_array[MAX_NATION];

// ----------- Define static function ----------//

static char* select_nation_color(char nation_color);

//---------- Begin of function TalkRes::TalkRes -----------//
//
TalkRes::TalkRes() : talk_msg_array( sizeof(TalkMsg), 100 )
{
}
//---------- End of function TalkRes::TalkRes -----------//


//---------- Begin of function TalkRes::init -----------//
//
void TalkRes::init()
{
	memset( this, 0, sizeof(TalkRes) - sizeof(talk_msg_array) );

	talk_msg_array.zap();
}
//---------- End of function TalkRes::init -----------//


//---------- Begin of function TalkRes::deinit -----------//
//
void TalkRes::deinit()
{
}
//---------- End of function TalkRes::deinit -----------//


//------- Begin of function TalkRes::init_conversion --------//
//
// <int> toNationRecno - recno of the nation which the player
//								 is going to talk to.
//
void TalkRes::init_conversion(int toNationRecno)
{
	memset( &cur_talk_msg, 0, sizeof(TalkMsg) );

	cur_talk_msg.from_nation_recno = (char) nation_array.player_recno;
	cur_talk_msg.to_nation_recno = toNationRecno;

	reply_talk_msg_recno = 0;

	set_talk_choices();
}
//-------- End of function TalkRes::init_conversion ---------//


//---- Begin of function TalkRes::set_talk_choices ------//
//
// Set the choices of the next talk.
//
// return : <int> 1 - choices are set
//                0 - if there is no additional talk segment for this talk
//
int TalkRes::set_talk_choices()
{
	//------------------------------------//

	Nation* playerNation = ~nation_array;

	talk_choice_count = 0;
	choice_question = NULL;
	choice_question_second_line = NULL;
	cur_choice_id = 0;

	//------------------------------------//

	int rc=0;

	memset( available_talk_id_array, 0, sizeof(available_talk_id_array) );

	switch( cur_talk_msg.talk_id )
	{
		//--- add the main option choices ---//

		case 0:
			add_main_choices();
			return 1;

		case TALK_PROPOSE_TRADE_TREATY:
		case TALK_PROPOSE_FRIENDLY_TREATY:
		case TALK_PROPOSE_ALLIANCE_TREATY:
		case TALK_END_TRADE_TREATY:
		case TALK_END_FRIENDLY_TREATY:
		case TALK_END_ALLIANCE_TREATY:
		case TALK_REQUEST_CEASE_WAR:
		case TALK_DECLARE_WAR:
		case TALK_REQUEST_MILITARY_AID:
			return 0;

		case TALK_REQUEST_TRADE_EMBARGO:
			rc = add_trade_embargo_choices();
			break;

		case TALK_REQUEST_DECLARE_WAR:
			rc = add_declare_war_choices();
			break;

		case TALK_REQUEST_BUY_FOOD:
			rc = add_buy_food_choices();
			break;

		case TALK_GIVE_TRIBUTE:
		case TALK_DEMAND_TRIBUTE:
			rc = add_tribute_choices();
			if( rc )
				choice_question = _("How much tribute?");	// add the choice question here because we use the same function for both tribute and aid
			break;

		case TALK_GIVE_AID:
		case TALK_DEMAND_AID:
			rc = add_tribute_choices();
			if( rc )
				choice_question = _("How much aid?");
			break;

		case TALK_GIVE_TECH:
		case TALK_DEMAND_TECH:
			rc = add_give_tech_choices();
			break;

		case TALK_REQUEST_SURRENDER:
			rc = add_request_surrender_choices();
			break;

		case TALK_SURRENDER:
			rc = add_surrender_choices();
			break;

		default:
			err_here();
	}

	if( rc )
		add_talk_choice( _("Cancel."), 0 );

	return rc;
}
//----- End of function TalkRes::set_talk_choices -------//


//---- Begin of function TalkRes::add_main_choices ------//
//
void TalkRes::add_main_choices()
{
	static const char* talkMsgArray[] =
	{
		N_("Propose a trade treaty."),
		N_("Propose a friendly treaty."),
		N_("Propose an alliance treaty."),
		N_("Terminate our trade treaty."),
		N_("Terminate our friendly treaty."),
		N_("Terminate our alliance treaty."),
		N_("Request immediate military aid."),
		N_("Request a trade embargo."),
		N_("Request a cease-fire."),
		N_("Request a declaration of war against a foe."),
		N_("Request to purchase food."),
		N_("Declare war."),
		N_("Offer to pay tribute."),
		N_("Demand tribute."),
		N_("Offer aid."),
		N_("Request aid."),
		N_("Offer to transfer technology."),
		N_("Request technology."),
		N_("Offer to purchase throne and unite kingdoms."),
		N_("Surrender."),
	};

	//-----------------------------------------//

	int rc;
	int relationStatus = (~nation_array)->get_relation_status(cur_talk_msg.to_nation_recno);

	for( int i=1 ; i<=MAX_TALK_TYPE ; i++ )	
	{
		if( !can_send_msg(cur_talk_msg.to_nation_recno, nation_array.player_recno, i) )
			continue;

		rc=0;

		if( !rc )
			add_talk_choice( _(talkMsgArray[i-1]), i );

		available_talk_id_array[i-1] = 1;
	}
}
//----- End of function TalkRes::add_main_choices -------//


//---- Begin of function TalkRes::add_trade_embargo_choices ------//
//
int TalkRes::add_trade_embargo_choices()
{
	if( cur_talk_msg.talk_para1 )
		return 0;

	choice_question = _("Request an embargo on trade with which kingdom?");

	Nation* fromNation = nation_array[cur_talk_msg.from_nation_recno];
	Nation* toNation   = nation_array[cur_talk_msg.to_nation_recno];

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( i==cur_talk_msg.from_nation_recno ||
			 i==cur_talk_msg.to_nation_recno )
		{
			continue;
		}

		if( fromNation->get_relation(i)->trade_treaty==0 &&
			 toNation->get_relation(i)->trade_treaty==1 )
		{
			//------ add color bar -------//

			nation_name_str_array[i-1] = select_nation_color(nation_array[i]->color_scheme_id);

			//------ add natino name ------//

			nation_name_str_array[i-1] += nation_array[i]->nation_name();

			//---- add talk choice string ------//

			add_talk_choice( nation_name_str_array[i-1], i );
		}
	}

	return 1;
}
//----- End of function TalkRes::add_trade_embargo_choices -------//


//---- Begin of function TalkRes::add_declare_war_choices ------//
//
int TalkRes::add_declare_war_choices()
{
	if( cur_talk_msg.talk_para1 )
		return 0;

	choice_question = _("Declare war on which kingdom?");

	Nation* fromNation = nation_array[cur_talk_msg.from_nation_recno];
	Nation* toNation   = nation_array[cur_talk_msg.to_nation_recno];

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		//--- can only ask another nation to declare war with a nation that is currently at war with our nation ---//

		if( fromNation->get_relation_status(i) == NATION_HOSTILE &&
			 toNation->get_relation_status(i) != NATION_HOSTILE )
		{
			//------ add color bar -------//

			nation_name_str_array[i-1] = select_nation_color(nation_array[i]->color_scheme_id);

			//------ add natino name ------//

			nation_name_str_array[i-1] += nation_array[i]->nation_name();

			//---- add talk choice string ------//

			add_talk_choice( nation_name_str_array[i-1], i );
		}
	}

	return 1;
}
//----- End of function TalkRes::add_declare_war_choices -------//


//---- Begin of function TalkRes::add_buy_food_choices ------//
//
int TalkRes::add_buy_food_choices()
{
	#define MIN_FOOD_PURCHASE_PRICE	5

	if( !cur_talk_msg.talk_para1 )
	{
		choice_question = _("How much food do you want to purchase?");

		static const char* qtyStrArray[] = { "500.", "1000.", "2000.", "4000." };
		static short qtyArray[] = { 500, 1000, 2000, 4000 };

		for( int i=0 ; i<4 ; i++ )
		{
			if( (~nation_array)->cash >= qtyArray[i] * MIN_FOOD_PURCHASE_PRICE / 10 )
				add_talk_choice( qtyStrArray[i], qtyArray[i] );
		}

		return 1;
	}
	else if( !cur_talk_msg.talk_para2 )
	{
		choice_question = _("How much do you offer for 10 units of food?");

		static const char* priceStrArray[] = { "$5.", "$10.", "$15.", "$20." };
		static short priceArray[] = { 5, 10, 15, 20 };

		for( int i=0 ; i<4 ; i++ )
		{
			if( i==0 || (~nation_array)->cash >= cur_talk_msg.talk_para1 * priceArray[i] / 10 )		// i==0 to at least add one option
				add_talk_choice( priceStrArray[i], priceArray[i] );
		}

		return 1;
	}
	else
		return 0;
}
//----- End of function TalkRes::add_buy_food_choices -------//


//---- Begin of function TalkRes::add_tribute_choices ------//
//
int TalkRes::add_tribute_choices()
{
	if( cur_talk_msg.talk_para1 )
		return 0;

	static const char* tributeStrArray[] = { "$500.", "$1000.", "$2000.", "$3000.", "$4000." };
	static short tributeAmtArray[] = { 500, 1000, 2000, 3000, 4000 };

	for( int i=0 ; i<5 ; i++ )
	{
		if( cur_talk_msg.talk_id == TALK_DEMAND_TRIBUTE ||		// when demand tribute, the amount can be sent to any
			 cur_talk_msg.talk_id == TALK_DEMAND_AID ||
			 (~nation_array)->cash >= tributeAmtArray[i] )
		{
			add_talk_choice( tributeStrArray[i], tributeAmtArray[i] );
		}
	}

	return 1;
}
//----- End of function TalkRes::add_tribute_choices -------//


//---- Begin of function TalkRes::add_give_tech_choices ------//
//
int TalkRes::add_give_tech_choices()
{
	int i, techNationRecno;

	if( cur_talk_msg.talk_id == TALK_GIVE_TECH )
		techNationRecno = cur_talk_msg.from_nation_recno;
	else	// demand tech
		techNationRecno = cur_talk_msg.to_nation_recno;

	if( !cur_talk_msg.talk_para1 )
	{
		choice_question = _("Which technology?");

		for( i=1 ; i<=tech_res.tech_count ; i++ )
		{
			if( tech_res[i]->get_nation_tech_level(techNationRecno) > 0 )
			{
				add_talk_choice( tech_res[i]->tech_des(), i );
			}
		}

		return 1;
	}
	else if( !cur_talk_msg.talk_para2 && cur_talk_msg.talk_id == TALK_GIVE_TECH )
	{
		TechInfo* techInfo = tech_res[cur_talk_msg.talk_para1];

		if( techInfo->max_tech_level==1 )		// this tech only has one level
			return 0;

		choice_question = _("Which version?");

		int nationLevel = techInfo->get_nation_tech_level(techNationRecno);

		err_when( nationLevel<1 || nationLevel>3 );

		static const char* verStrArray[] = { "Mark I", "Mark II", "Mark III" };

		for( i=1 ; i<=MIN(3, nationLevel) ; i++ )
			add_talk_choice( verStrArray[i-1], i );

		return 1;
	}
	else
		return 0;
}
//----- End of function TalkRes::add_give_tech_choices -------//


//---- Begin of function TalkRes::add_request_surrender_choices ------//
//
int TalkRes::add_request_surrender_choices()
{
	if( cur_talk_msg.talk_para1 )
		return 0;

	choice_question = _("How much do you offer?");

	static const char* strArray[] = { "$5000.", "$7500.", "$10000.",
		"$15000.", "$20000.", "$30000.", "$40000.", "$50000." };

	static int amtArray[] = { 5000, 7500, 10000, 15000, 20000, 30000, 40000, 50000 };

	for( int i=0 ; i<8 ; i++ )
	{
		if( (~nation_array)->cash >= amtArray[i] )
		{
			add_talk_choice( strArray[i], amtArray[i]/10 );		// divided by 10 to cope with the limit of <short>
		}
	}

	return 1;
}
//----- End of function TalkRes::add_request_surrender_choices -------//


//---- Begin of function TalkRes::add_surrender_choices ------//
//
int TalkRes::add_surrender_choices()
{
	if( cur_talk_msg.talk_para1 )
		return 0;

	static String str;

	snprintf( str, MAX_STR_LEN+1, _("Do you really want to Surrender to %s's Kingdom?"), nation_array[cur_talk_msg.to_nation_recno]->king_name(1) );
	choice_question = str;

	add_talk_choice( _("Confirm."), 1 );

	return 1;
}
//----- End of function TalkRes::add_surrender_choices -------//


//------- Begin of function TalkRes::can_send_any_msg --------//
//
int TalkRes::can_send_any_msg(int toNationRecno, int fromNationRecno)
{
	return wait_msg_count(toNationRecno, fromNationRecno) < MAX_WAIT_MSG_PER_NATION;
}
//-------- End of function TalkRes::can_send_any_msg ---------//


//------- Begin of function TalkRes::can_send_msg --------//
//
// return whether one specific nation can send a specific message
// to another specific nation.
//
int TalkRes::can_send_msg(int toNationRecno, int fromNationRecno, int talkId)
{
	Nation* fromNation = nation_array[fromNationRecno];
	Nation* toNation 	 = nation_array[toNationRecno];

	NationRelation *nationRelation = fromNation->get_relation(toNationRecno);
	int relationStatus = nationRelation->status;

	switch( talkId )
	{
		case TALK_PROPOSE_TRADE_TREATY:
			return relationStatus !=NATION_ALLIANCE && 		// allied nations are oblied to trade with each other
					 relationStatus !=NATION_HOSTILE &&
					 !toNation->get_relation(fromNationRecno)->trade_treaty;

		case TALK_PROPOSE_FRIENDLY_TREATY:
			return relationStatus==NATION_TENSE ||
					 relationStatus==NATION_NEUTRAL;

		case TALK_PROPOSE_ALLIANCE_TREATY:
			return relationStatus==NATION_FRIENDLY ||
					 relationStatus==NATION_NEUTRAL;

		case TALK_END_TRADE_TREATY:
			return relationStatus !=NATION_ALLIANCE && 		// allied nations are oblied to trade with each other
					 toNation->get_relation(fromNationRecno)->trade_treaty;

		case TALK_END_FRIENDLY_TREATY:
			return relationStatus==NATION_FRIENDLY;

		case TALK_END_ALLIANCE_TREATY:
			return relationStatus==NATION_ALLIANCE;

		case TALK_REQUEST_MILITARY_AID:
			return fromNation->is_at_war() &&
					 (relationStatus==NATION_FRIENDLY ||
					  relationStatus==NATION_ALLIANCE);

		case TALK_REQUEST_TRADE_EMBARGO:
			return relationStatus==NATION_FRIENDLY ||
					 relationStatus==NATION_ALLIANCE;

		case TALK_REQUEST_CEASE_WAR:
			return relationStatus==NATION_HOSTILE;

		case TALK_REQUEST_DECLARE_WAR:		// can only request an allied nation to declare war with another nation
		{
			if( relationStatus != NATION_ALLIANCE )
				return 0;

			//--- see if this nation has an enemy right now ---//

			for( int i=nation_array.size() ; i>0 ; i-- )
			{
				if( nation_array.is_deleted(i) )
					continue;

				if( fromNation->get_relation(i)->status == NATION_HOSTILE )
					return 1;
			}

			return 0;
		}

		case TALK_REQUEST_BUY_FOOD:
			return relationStatus != NATION_HOSTILE;

		case TALK_DECLARE_WAR:
			return relationStatus != NATION_ALLIANCE &&
					 relationStatus != NATION_FRIENDLY &&
					 relationStatus != NATION_HOSTILE;

		case TALK_GIVE_TRIBUTE:
		case TALK_DEMAND_TRIBUTE:
			return relationStatus <= NATION_NEUTRAL;

		case TALK_GIVE_AID:
		case TALK_DEMAND_AID:
			return relationStatus >= NATION_FRIENDLY;

		case TALK_GIVE_TECH:
			return fromNation->total_tech_level() > 0;

		case TALK_DEMAND_TECH:
			return toNation->total_tech_level() > 0;

		case TALK_REQUEST_SURRENDER:
			return 1;

		case TALK_SURRENDER:
			return 1;
	}

	return 0;
}
//-------- End of function TalkRes::can_send_msg ---------//


//------- Begin of function TalkRes::add_talk_choice --------//
//
void TalkRes::add_talk_choice(const char* talkStr, int talkPara)
{
	err_when( talk_choice_count >= MAX_TALK_CHOICE );

	talk_choice_count++;

	talk_choice_array[talk_choice_count-1].str  = talkStr;
	talk_choice_array[talk_choice_count-1].para = talkPara;
}
//-------- End of function TalkRes::add_talk_choice ---------//


//------- Begin of function TalkRes::ai_send_talk_msg --------//
//
// <int> toNationRecno	 - the nation which this message is being sent to
// <int> fromNationRecno - the nation which this message is from
// <int> talkId			 - id. of the talk
// [int] talkPara1		 - talk para 1
// [int] talkPara2		 - talk para 2
// [int] forceSend		 - if 1, then should_diplomacy_retry() won't
//									be checked. (default: 0)
//
int TalkRes::ai_send_talk_msg(int toNationRecno, int fromNationRecno, int talkId, int talkPara1, int talkPara2, int forceSend)
{
	Nation* fromNation = nation_array[fromNationRecno];

	if( !fromNation->is_ai() )
		return 0;

	//--- first check again if the nation should send the message now ---//

	if( !forceSend )
	{
		if( !fromNation->should_diplomacy_retry(talkId, toNationRecno) )
			return 0;
	}

	//-------- avoid send opposite message too soon ----//

	int oppTalkId=0;

	switch( talkId )
	{
		case TALK_PROPOSE_ALLIANCE_TREATY:
			oppTalkId = TALK_END_FRIENDLY_TREATY;
			break;

		case TALK_GIVE_TRIBUTE:
			oppTalkId = TALK_DEMAND_TRIBUTE;
			break;

		case TALK_DEMAND_TRIBUTE:
			oppTalkId = TALK_GIVE_TRIBUTE;
			break;

		case TALK_GIVE_AID:
			oppTalkId = TALK_DEMAND_AID;
			break;

		case TALK_DEMAND_AID:
			oppTalkId = TALK_GIVE_AID;
			break;

		case TALK_GIVE_TECH:
			oppTalkId = TALK_DEMAND_TECH;
			break;

		case TALK_DEMAND_TECH:
			oppTalkId = TALK_GIVE_TECH;
			break;
	}

	if( oppTalkId )
	{
		fromNation->get_relation(toNationRecno)->
			last_talk_reject_date_array[oppTalkId-1] = info.game_date;
	}

	//------------------------------------------//

	TalkMsg talkMsg;

	memset(&talkMsg, 0, sizeof(TalkMsg));

	talkMsg.to_nation_recno   = toNationRecno;
	talkMsg.from_nation_recno = fromNationRecno;
	talkMsg.talk_id  			  = talkId;
	talkMsg.talk_para1		  = talkPara1;
	talkMsg.talk_para2		  = talkPara2;

	err_when( !nation_array[fromNationRecno]->is_ai() );

	send_talk_msg( &talkMsg, COMMAND_AI );

	return 1;
}
//-------- End of function TalkRes::ai_send_talk_msg ---------//


//------- Begin of function TalkRes::send_talk_msg --------//
//
// Now records in talk_msg_array cannot be deleted as
// news_array.diplomacy() use recno to refer to talk_msg_array.
//
//###### begin jesse 2022/10/2 #######//
// This function calls linkin (send_talk_msg_now). This must not be called from
// the TalkMsg class. The talkMsgPtr argument must not be in talk_msg_array.
// If the calling function is referencing any pointers into talk_msg_array,
// they must be treated as invalid after calling this function.
//###### end jesse 2022/10/2 #######//
//
void TalkRes::send_talk_msg(TalkMsg* talkMsgPtr, char remoteAction)
{
	//-------- send multiplayer -----------//

	if( !remoteAction && remote.is_enable() )
	{
		// packet strcture : <TalkMsg>
		char* dataPtr = remote.new_send_queue_msg(MSG_SEND_TALK_MSG, sizeof(TalkMsg) );

		memcpy( dataPtr, talkMsgPtr, sizeof(TalkMsg) );
		return;
	}

	//------ the TalkMsg::reply_type ------//

	if( talkMsgPtr->is_reply_needed() )
		talkMsgPtr->reply_type = REPLY_WAITING;
	else
		talkMsgPtr->reply_type = REPLY_NOT_NEEDED;

	//-- If this is an AI message check if this message has already been sent --//

	if( nation_array[talkMsgPtr->from_nation_recno]->nation_type == NATION_AI )
	{
		if( talkMsgPtr->reply_type == REPLY_WAITING )	// for messages that do not need a reply, duplication in the message log is allowed.
		{
			if( is_talk_msg_exist(talkMsgPtr, 0) )		// 0-don't check talk_para1 & talk_para2
				return;
		}
	}

	//--- in a multiplayer game, when the msg comes back from the network, can_send_msg might be different, so we have to check it again ---//

	if( !can_send_msg( talkMsgPtr->to_nation_recno, talkMsgPtr->from_nation_recno, talkMsgPtr->talk_id ) )
		return;

	//-------- send the message now ---------//

	send_talk_msg_now(talkMsgPtr);

	//---- if it's a notification message ----//

	if( talkMsgPtr->reply_type == REPLY_NOT_NEEDED )
		process_accepted_reply(talkMsgPtr);
}
//-------- End of function TalkRes::send_talk_msg ---------//


//------- Begin of function TalkRes::send_talk_msg_now --------//
//
void TalkRes::send_talk_msg_now(TalkMsg* talkMsgPtr)
{
	//--------- add the message ------------//

	Nation* toNation = nation_array[talkMsgPtr->to_nation_recno];

	talkMsgPtr->date = info.game_date;
	talkMsgPtr->relation_status = toNation->get_relation_status(talkMsgPtr->from_nation_recno);

	talk_msg_array.linkin( talkMsgPtr );

	err_when( nation_array.is_deleted(talkMsgPtr->from_nation_recno) );
	err_when( nation_array.is_deleted(talkMsgPtr->to_nation_recno) );

	//--------------------------------------//

	switch(toNation->nation_type)
	{
		case NATION_OWN:		// can be from both AI or a remote player
			news_array.diplomacy( talk_msg_array.recno() );
			// ###### begin Gilbert 9/10 ########//
			// sound effect
			if( toNation->get_relation(talkMsgPtr->from_nation_recno)->has_contact )
				se_ctrl.immediate_sound(talkMsgPtr->talk_id==TALK_DECLARE_WAR ? (char*)"DECL_WAR":(char*)"GONG");
			// ###### end Gilbert 9/10 ########//
			break;

		case NATION_AI:
			if( talkMsgPtr->reply_type == REPLY_WAITING )
			{
				//-- put the message in the receiver's action queue --//

				toNation->add_action( 0, 0, 0, 0, ACTION_AI_PROCESS_TALK_MSG, talk_msg_array.recno() );
			}
			else if( talkMsgPtr->reply_type == REPLY_NOT_NEEDED )
			{
				//--- notify the receiver immediately ---//

				toNation->notify_talk_msg( talkMsgPtr );
			}
			else
				err_here();
			break;

		case NATION_REMOTE:	// do nothing here as NATION_OWN handle both msg from AI and a remote player
			break;
	}
}
//-------- End of function TalkRes::send_talk_msg_now ---------//


//------- Begin of function TalkRes::reply_talk_msg --------//
//
// If the nation which receives this message decides to accept
// the offer of this message, this function is called.
//
// <int>  talkMsgRecno - the recno of the TalkMsg
// <char> replyType	  - reply type, either REPLY_ACCEPT or REPLY_REJECT
// <char> remoteAction - remote action type
//
void TalkRes::reply_talk_msg(int talkMsgRecno, char replyType, char remoteAction)
{
	//-------- send multiplayer -----------//

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <talkRecno:int> <reply type:char> <padding:char>
		char* charPtr = remote.new_send_queue_msg( MSG_REPLY_TALK_MSG, sizeof(int)+2*sizeof(char) );
		*(int *)charPtr = talkMsgRecno;
		charPtr[sizeof(int)] = replyType;
		charPtr[sizeof(int)+sizeof(char)] = 0;
		return;
	}

	//-------------------------------------//

	err_when( is_talk_msg_deleted(talkMsgRecno) );

	TalkMsg* talkMsgPtr = get_talk_msg(talkMsgRecno);
	Nation*  fromNation = nation_array[talkMsgPtr->from_nation_recno];

	err_when( talkMsgPtr->reply_type == REPLY_NOT_NEEDED );
	err_when( replyType != REPLY_ACCEPT && replyType != REPLY_REJECT );

	talkMsgPtr->reply_type = replyType;
	talkMsgPtr->reply_date = info.game_date;

	switch( fromNation->nation_type )
	{
		case NATION_OWN:
			news_array.diplomacy( talkMsgRecno );
			// ###### begin Gilbert 9/10 ########//
			// sound effect
			se_ctrl.immediate_sound("GONG");
			// ###### end Gilbert 9/10 ########//
			break;

		case NATION_AI:
			fromNation->ai_notify_reply( talkMsgRecno );		// notify the AI nation about this reply.
			if( is_talk_msg_deleted(talkMsgRecno) )
				return;
			talkMsgPtr = get_talk_msg(talkMsgRecno);
			break;

		case NATION_REMOTE:
			break;
	}

	//------- if the offer is accepted -------//

	if( talkMsgPtr->reply_type == REPLY_ACCEPT )
	{
		process_accepted_reply(talkMsgPtr);
		if( is_talk_msg_deleted(talkMsgRecno) )
			return;
		talkMsgPtr = get_talk_msg(talkMsgRecno);
	}

	//--- if the player has replyed the message, remove it from the news display ---//

	if( talkMsgPtr->to_nation_recno == nation_array.player_recno )
		news_array.remove(NEWS_DIPLOMACY, talkMsgRecno);

}
//-------- End of function TalkRes::reply_talk_msg ---------//


//------- Begin of function TalkRes::disp_talk --------//

void TalkRes::disp_talk()
{
	//----- if the player can send no more message to the nation ---//

	if( !info.player_reply_mode &&
		 !can_send_any_msg(cur_talk_msg.to_nation_recno, nation_array.player_recno) )
	{
		if( !( choice_question && strcmp(choice_question, MESSAGE_SENT_STR)==0 ) )		// if it's currently displaying the has sent notification, display that message
		{
			const char* msgStr = _("You've sent too many messages to this kingdom. You cannot send any new messages until the existing ones are processed.");

			font_san.put_paragraph( TALK_X1, TALK_Y1, TALK_X2, TALK_Y2, msgStr, 4 );
			return;
		}
	}

	//--- return whether the talk choices should be refreshed. ---//

	if( !reply_talk_msg_recno && !cur_talk_msg.talk_id )		// not replying and haven't selected a message type yet.
	{
		for( int i=0 ; i<MAX_TALK_TYPE ; i++ )
		{
			if( available_talk_id_array[i] != can_send_msg(cur_talk_msg.to_nation_recno, cur_talk_msg.from_nation_recno, i+1) )
			{
				set_talk_choices();
				break;
			}
		}
	}

	//------ repaint the area for displaying the sub-title ------//

	int 	 y=TALK_Y1;
	String str;

	//----- if there is a question on the choices ------//

	if( choice_question )
	{
		font_san.put( TALK_X1+3, y, choice_question );
		y+=TALK_LINE_HEIGHT;
	}

	if( choice_question_second_line )		// second line message
	{
		font_san.put( TALK_X1+3, y, choice_question_second_line );
		y+=TALK_LINE_HEIGHT;
	}

	//--------- display talk choices ---------//

	for( int i=0 ; i<talk_choice_count ; i++, y+=TALK_LINE_HEIGHT )
	{
		if( i+1 == cur_choice_id )		// this is the one the mouse cursor is currently on
			vga_back.adjust_brightness( TALK_X1, y, TALK_X2, y+TALK_LINE_HEIGHT-1, -3 );

		if( choice_question )
		{
			str  = "- ";		// display bullets in front of the text strings
			str += talk_choice_array[i].str;
		}
		else
			str = talk_choice_array[i].str;

		font_san.put_paragraph( TALK_X1+3, y+2, TALK_X2-3, y+font_san.height()+1, str );
	}
}
//-------- End of function TalkRes::disp_talk ---------//


//------- Begin of function TalkRes::detect_talk_choices --------//
//
int TalkRes::detect_talk_choices()
{
	int i, y=TALK_Y1;
	int newChoiceId=0;

	if( choice_question )
		y+=TALK_LINE_HEIGHT;

	if( choice_question_second_line )
		y+=TALK_LINE_HEIGHT;

	for( i=0 ; i<talk_choice_count ; i++, y+=TALK_LINE_HEIGHT )
	{
		if( mouse.in_area(TALK_X1, y, TALK_X2, y+TALK_LINE_HEIGHT-1) )
		{
			newChoiceId = i+1;
			break;
		}
	}

	//-------- refresh talk display -------//

	if( newChoiceId != cur_choice_id )
	{
		cur_choice_id = newChoiceId;
		disp_talk();
	}

	//------ if pressed on one of the talks ------//

	if( newChoiceId && mouse.single_click(TALK_X1, y, TALK_X2, y+TALK_LINE_HEIGHT-1) )
		return newChoiceId;

	return 0;
}
//-------- End of function TalkRes::detect_talk_choices ---------//


//------- Begin of function TalkRes::detect_talk --------//
//
int TalkRes::detect_talk()
{
	//----- if the message which player is reply is deleted (this can only happen when the player stays on the reply message screen for almost 1 year without doing anything and then the message is automatically deleted after having being kept for 1 year ---//

	if( reply_talk_msg_recno &&
		 is_talk_msg_deleted(reply_talk_msg_recno) )		// the message may become invalid during the replying period
	{
		sys.set_view_mode(save_view_mode);

		if( save_view_mode == MODE_NATION )
		{
			cur_talk_msg.talk_id = 0;
			init_conversion(cur_talk_msg.to_nation_recno);
		}

		return 0;
	}

	//------------------------------------------//

	int choiceId = detect_talk_choices();

	if( !choiceId )
		return 0;

	int choicePara = talk_choice_array[choiceId-1].para;

	//---- if the player is replying message from other nation ----//

	if( reply_talk_msg_recno )
	{
		if( !is_talk_msg_deleted(reply_talk_msg_recno) )		// the message may become invalid during the replying period
		{
			if( choicePara==1 )
				reply_talk_msg(reply_talk_msg_recno, REPLY_ACCEPT, COMMAND_PLAYER);
			else
				reply_talk_msg(reply_talk_msg_recno, REPLY_REJECT, COMMAND_PLAYER);
		}

		sys.set_view_mode(save_view_mode);

		if( save_view_mode == MODE_NATION )
		{
			cur_talk_msg.talk_id = 0;
			init_conversion(cur_talk_msg.to_nation_recno);
		}

		return 1;
	}

	//---------------------------------------//

	if( strcmp( talk_choice_array[choiceId-1].str, _("Cancel.") ) == 0 ||
		 (choice_question && strcmp( choice_question, MESSAGE_SENT_STR )==0) )
	{
		cur_talk_msg.talk_id = 0;
		init_conversion(cur_talk_msg.to_nation_recno);
		return 1;
	}

	//------ set the current choice to cur_talk_msg -------//

	if( cur_talk_msg.talk_id == 0 )
		cur_talk_msg.talk_id = choicePara;

	else if( cur_talk_msg.talk_para1 == 0 )
		cur_talk_msg.talk_para1 = choicePara;

	else if( cur_talk_msg.talk_para2 == 0 )
		cur_talk_msg.talk_para2 = choicePara;

	else
		err_here();

	//------ prepare the next available choices ------//

	if( !set_talk_choices() )		// the talk is complete
	{
		send_talk_msg( &cur_talk_msg, COMMAND_PLAYER );

		//--- the message has been sent, display notification message ---//

		choice_question = MESSAGE_SENT_STR;
		choice_question_second_line = NULL;

		talk_choice_count = 0;
		add_talk_choice( _("Continue."), 0 );
	}

	return 1;
}
//-------- End of function TalkRes::detect_talk ---------//


//------- Begin of function TalkRes::get_talk_msg --------//

TalkMsg* TalkRes::get_talk_msg(int recNo)
{
	err_when( recNo < 1 || recNo > talk_msg_array.size() );

	TalkMsg* talkMsg = (TalkMsg*) talk_msg_array.get(recNo);

	if( !talkMsg->talk_id )
		err.run( "get_talk_msg() error, the TalkMsg is deleted." );

	return talkMsg;
}
//-------- End of function TalkRes::get_talk_msg ---------//


//------- Begin of function TalkRes::is_talk_msg_deleted --------//

int TalkRes::is_talk_msg_deleted(int recNo)
{
	if( recNo < 1 || recNo > talk_msg_array.size() )
		return 1;

	TalkMsg* talkMsg = (TalkMsg*) talk_msg_array.get(recNo);

	return talkMsg->talk_id==0;
}
//-------- End of function TalkRes::is_talk_msg_deleted ---------//


//------- Begin of function TalkRes::next_day --------//

void TalkRes::next_day()
{
	if( info.game_date%7 == 0 )
		process_talk_msg();
}
//-------- End of function TalkRes::next_day ---------//


//------ Begin of function TalkRes::process_talk_msg ------//
//
void TalkRes::process_talk_msg()
{
	int 		i;
	TalkMsg* talkMsg;

	for( i=talk_msg_count() ; i>0 ; i-- )
	{
		if( is_talk_msg_deleted(i) )
			continue;

		talkMsg = get_talk_msg(i);

		//--------------------------------------------------------//
		// If this is an AI message and there is no response from
		// the player after one month the message has been sent,
		// it presumes that the message has been rejected.
		//--------------------------------------------------------//

		if( nation_array[talkMsg->from_nation_recno]->nation_type == NATION_AI &&
			 talkMsg->reply_type == REPLY_WAITING &&
			 info.game_date > talkMsg->date + DISP_NEWS_DAYS )
		{
			talkMsg->reply_type = REPLY_REJECT;

			nation_array[talkMsg->from_nation_recno]->ai_notify_reply(i);
			talkMsg = get_talk_msg(i); // in case talkMsg ptr was invalidated in resize
		}

		//--- delete the talk message after a year ---//

		if( info.game_date > talkMsg->date + TALK_MSG_KEEP_DAYS )
			del_talk_msg(i);
	}
}
//-------- End of function TalkRes::process_talk_msg ---------//


//------ Begin of function TalkRes::del_talk_msg ------//
//
void TalkRes::del_talk_msg(int talkMsgRecno)
{
	err_when( is_talk_msg_deleted(talkMsgRecno) );

	TalkMsg* talkMsg = get_talk_msg(talkMsgRecno);

	//--- if this message is sent to an AI nation ---//

	Nation* nationPtr = nation_array[talkMsg->to_nation_recno];

	if( nationPtr->nation_type == NATION_AI &&
		 (talkMsg->reply_type == REPLY_NOT_NEEDED ||		// even if a reply is not needed, the message will still be sent to the AI for notification. 
		  talkMsg->reply_type == REPLY_WAITING) )
	{
		//--- it may still have the message in its action queue ---//

		ActionNode* actionNode;

		for( int i=nationPtr->action_count() ; i>0 ; i-- )
		{
			actionNode = nationPtr->get_action(i);

			if( actionNode->action_mode == ACTION_AI_PROCESS_TALK_MSG &&
				 actionNode->action_para == talkMsgRecno )
			{
				nationPtr->del_action(i);
				break;
			}
		}
	}

	//----- delete the message from the news array -----//

	if( talkMsg->to_nation_recno == nation_array.player_recno ||
		 talkMsg->from_nation_recno == nation_array.player_recno )
	{
		news_array.remove( NEWS_DIPLOMACY, talkMsgRecno );
	}

	//----- link it out from talk_msg_array -----//

	talk_msg_array.linkout(talkMsgRecno);
}
//-------- End of function TalkRes::del_talk_msg ---------//


//------- Begin of function TalkRes::is_talk_msg_exist --------//
//
// <int> checkPara - whether check talk_para1 and talk_para2
//						   in comparing TalkMsg.
//
// return : <int> >0 - the recno of the talk msg that already exists
//					  ==0 - not found
//
int TalkRes::is_talk_msg_exist(TalkMsg* thisTalkMsg, int checkPara)
{
	int 		i;
	TalkMsg* talkMsg;

	for( i=talk_msg_count() ; i>0 ; i-- )
	{
		if( is_talk_msg_deleted(i) )
			continue;

		talkMsg = get_talk_msg(i);

		if( talkMsg->reply_type == REPLY_WAITING ||
			 talkMsg->reply_type == REPLY_NOT_NEEDED )
		{
			if( talkMsg->talk_id 			 == thisTalkMsg->talk_id &&
				 talkMsg->from_nation_recno == thisTalkMsg->from_nation_recno &&
				 talkMsg->to_nation_recno   == thisTalkMsg->to_nation_recno )
			{
				if( checkPara )
				{
					if( talkMsg->talk_para1 == thisTalkMsg->talk_para1 &&
						 talkMsg->talk_para2 == thisTalkMsg->talk_para2 )
					{
						return i;
					}
				}
				else
					return i;
			}
		}
	}

	return 0;
}
//-------- End of function TalkRes::is_talk_msg_exist ---------//


//------- Begin of function TalkRes::wait_msg_count --------//
//
// Return the number of messages sent to the nation waiting
// for its reply.
//
int TalkRes::wait_msg_count(int toNationRecno, int fromNationRecno)
{
	int 		i, waitMsgCount=0;
	TalkMsg* talkMsg;

	for( i=talk_msg_count() ; i>0 ; i-- )
	{
		if( is_talk_msg_deleted(i) )
			continue;

		talkMsg = get_talk_msg(i);

		if( talkMsg->reply_type == REPLY_WAITING &&
			 talkMsg->to_nation_recno == toNationRecno &&
			 talkMsg->from_nation_recno == fromNationRecno &&
			 info.game_date < talkMsg->date + 30 )					// only count message in a month
		{
			waitMsgCount++;
		}
	}

	return waitMsgCount;
}
//-------- End of function TalkRes::wait_msg_count ---------//


//------- Begin of function TalkRes::player_reply --------//

void TalkRes::player_reply(int talkMsgRecno)
{
	//------- set the reply choices --------//

	err_when( is_talk_msg_deleted(talkMsgRecno) );

	TalkMsg* talkMsg = get_talk_msg(talkMsgRecno);

	if( nation_array.is_deleted(talkMsg->from_nation_recno) )
		return;

	init_conversion(talkMsg->from_nation_recno);

	talk_choice_count    = 0;
	cur_choice_id 		   = 0;
	reply_talk_msg_recno = talkMsgRecno;

	//--------- add talk choices ---------//

	static String msgStr, msgStr2;

	msgStr = talkMsg->msg_str(nation_array.player_recno);		// make a static copy of it.
	choice_question = msgStr;

	//---- see if this message has a second line -----//

	msgStr2 = talkMsg->msg_str(nation_array.player_recno, 0, 1);		// 1-display the second line of the question

	if( msgStr!=msgStr2 )
		choice_question_second_line = msgStr2;
	else
		choice_question_second_line = NULL;

	//--------- add choices to the question ---------//

	if( talkMsg->can_accept() )			// whether the replier can accept the request or demand of the message
		add_talk_choice( _("Accept."), 1 );

	add_talk_choice( _("Reject."), 0 );

	//--- switch to the nation report mode and go to the diplomacy mode ---//

	info.init_player_reply( talkMsg->from_nation_recno );

	save_view_mode = sys.view_mode;

	sys.set_view_mode(MODE_NATION);
}
//-------- End of function TalkRes::player_reply ---------//


//------- Begin of function TalkRes::del_all_nation_msg --------//
//
// Delete all messages related to this nation.
//
void TalkRes::del_all_nation_msg(int nationRecno)
{
	int 		i;
	TalkMsg* talkMsg;

	for( i=talk_msg_count() ; i>0 ; i-- )
	{
		if( is_talk_msg_deleted(i) )
			continue;

		talkMsg = get_talk_msg(i);

		// If the nation is referenced anywhere in the talk-message, is_valid_to_disp() will return 0 and the talk message should than be deleted. Need to explicitly specify nation as it hasn't been deleted yet.
		if( !talkMsg->is_valid_to_disp(nationRecno) )
		{
			del_talk_msg(i);
		}
	}
}
//-------- End of function TalkRes::del_all_nation_msg ---------//


#define ASCII_ZERO 0x30
//------ Begin of static function select_nation_color ------//
//
static char* select_nation_color(char nation_color)
{
	static char colorCodeStr[] = "@COL0 ";

	colorCodeStr[4] = ASCII_ZERO + nation_color;

	return colorCodeStr;
}
//------ End of static function select_nation_color ------//
