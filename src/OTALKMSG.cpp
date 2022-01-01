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
// If invalid_player_recno > 0 then messages that involve this nation are considered invalid as well.
//
int TalkMsg::is_valid_to_disp(int invalid_player_recno)
{
	//--- check if the nations are still there -----//

	if( nation_array.is_deleted(from_nation_recno) || (invalid_player_recno && from_nation_recno == invalid_player_recno) )
		return 0;

	if( nation_array.is_deleted(to_nation_recno) || (invalid_player_recno && to_nation_recno == invalid_player_recno) )
		return 0;

	//--------------------------------------//

	Nation* toNation   = nation_array[to_nation_recno];
	Nation* fromNation = nation_array[from_nation_recno];

	switch( talk_id )
	{
		case TALK_REQUEST_TRADE_EMBARGO:
		case TALK_REQUEST_DECLARE_WAR:
			if( nation_array.is_deleted(talk_para1) || (invalid_player_recno && talk_para1 == invalid_player_recno) )
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
// Used in printf as part of "<King's> Kingdom<Color>"
char* TalkMsg::from_nation_name()
{
	static String str;

	str = nation_array[from_nation_recno]->king_name(1);

	return str;
}
//------- End of function TalkMsg::from_nation_name ------//


//----- Begin of function TalkMsg::to_nation_name ------//
//
// Used in printf as part of "<King's> Kingdom<Color>"
char* TalkMsg::to_nation_name()
{
	static String str;

	str = nation_array[to_nation_recno]->king_name(1);

	return str;
}
//------- End of function TalkMsg::to_nation_name ------//


//----- Begin of function TalkMsg::para1_nation_name ------//
//
// Used in printf as part of "<King's> Kingdom<Color>"
char* TalkMsg::para1_nation_name()
{
	static String str;

	str = nation_array[talk_para1]->king_name(1);

	return str;
}
//------- End of function TalkMsg::para1_nation_name ------//


#define ASCII_ZERO 0x30
//----- Begin of function TalkMsg::from_king_name ------//
//
// Returns king's first name with optional color
char* TalkMsg::from_king_name()
{
	static String str;

	str = nation_array[from_nation_recno]->king_name();

	//------ add nation color bar -------//

	if( talk_res.msg_add_nation_color )
	{
		char colorCodeStr[] = " @COL0";

		colorCodeStr[5] = ASCII_ZERO + nation_array[from_nation_recno]->color_scheme_id;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function TalkMsg::from_king_name ------//


//----- Begin of function TalkMsg::to_king_name ------//
//
// Returns king's first name with optional color
char* TalkMsg::to_king_name()
{
	static String str;

	str = nation_array[to_nation_recno]->king_name();

	//------ add nation color bar -------//

	if( talk_res.msg_add_nation_color )
	{
		char colorCodeStr[] = " @COL0";

		colorCodeStr[5] = ASCII_ZERO + nation_array[to_nation_recno]->color_scheme_id;

		str += colorCodeStr;
	}

	return str;
}
//------- End of function TalkMsg::to_king_name ------//


//----- Begin of function TalkMsg::nation_color_code_str ------//
//
char* TalkMsg::nation_color_code_str(int nationRecno)
{
	static char colorCodeStr[] = " @COL0";

	colorCodeStr[5] = ASCII_ZERO + nation_array[nationRecno]->color_scheme_id;
	return colorCodeStr;
}
//------- End of function TalkMsg::nation_color_code_str ------//


//----- Begin of function TalkMsg::nation_color_code_str2 ------//
//
char* TalkMsg::nation_color_code_str2(int nationRecno)
{
	static char colorCodeStr[] = " @COL0";

	if( talk_res.msg_add_nation_color )
	{
		colorCodeStr[0] = ' ';
		colorCodeStr[5] = ASCII_ZERO + nation_array[nationRecno]->color_scheme_id;
	}
	else
	{
		colorCodeStr[0] = 0;
	}

	return colorCodeStr;
}
//------- End of function TalkMsg::nation_color_code_str2 ------//
