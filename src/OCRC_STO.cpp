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

// Filename    : OCRC_STO.H
// Description : store of crc of objects

#include <ONATIONA.h>
#include <OUNIT.h>
#include <OFIRMA.h>
#include <OTOWN.h>
#include <OBULLET.h>
#include <OREBEL.h>
#include <OSPY.h>
#include <OTALKRES.h>
#include <OREMOTE.h>
#include <OCRC_STO.h>
#include <CRC.h>

CrcStore::CrcStore() :
	nations(0), units(0), firms(0), towns(0), bullets(0), rebels(0), spies(0), talk_msgs(0)
{
}

void CrcStore::init()
{
	deinit();
}

void CrcStore::deinit()
{
	nations.clear();
	units.clear();
	firms.clear();
	towns.clear();
	bullets.clear();
	rebels.clear();
	spies.clear();
	talk_msgs.clear();
	all_crc.clear();
	frame_check_num = 0;
}

void CrcStore::record_nations()
{
	nations.clear();
	*(short *)nations.reserve(sizeof(short)) = nation_array.size();

	for( short nationRecno = 1; nationRecno <= nation_array.size(); ++nationRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !nation_array.is_deleted(nationRecno) )
			checkNum = nation_array[nationRecno]->crc8();
		*(CRC_TYPE *)nations.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}

void CrcStore::record_units()
{
	units.clear();
	*(short *)units.reserve(sizeof(short)) = unit_array.size();

	for( short unitRecno = 1; unitRecno <= unit_array.size(); ++unitRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !unit_array.is_deleted(unitRecno) )
			checkNum = unit_array[unitRecno]->crc8();
		*(CRC_TYPE *)units.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}

void CrcStore::record_firms()
{
	firms.clear();
	*(short *)firms.reserve(sizeof(short)) = firm_array.size();

	for( short firmRecno = 1; firmRecno <= firm_array.size(); ++firmRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !firm_array.is_deleted(firmRecno) )
			checkNum = firm_array[firmRecno]->crc8();
		*(CRC_TYPE *)firms.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}

void CrcStore::record_towns()
{
	towns.clear();
	*(short *)towns.reserve(sizeof(short)) = town_array.size();

	for( short townRecno = 1; townRecno <= town_array.size(); ++townRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !town_array.is_deleted(townRecno) )
			checkNum = town_array[townRecno]->crc8();
		*(CRC_TYPE *)towns.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}

void CrcStore::record_bullets()
{
	bullets.clear();
	*(short *)bullets.reserve(sizeof(short)) = bullet_array.size();

	for( short bulletRecno = 1; bulletRecno <= bullet_array.size(); ++bulletRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !bullet_array.is_deleted(bulletRecno) )
			checkNum = bullet_array[bulletRecno]->crc8();
		*(CRC_TYPE *)bullets.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}


void CrcStore::record_rebels()
{
	rebels.clear();
	*(short *)rebels.reserve(sizeof(short)) = rebel_array.size();

	for( short rebelRecno = 1; rebelRecno <= rebel_array.size(); ++rebelRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !rebel_array.is_deleted(rebelRecno) )
		{
			checkNum = rebel_array[rebelRecno]->crc8();
		}
		*(CRC_TYPE *)rebels.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}


void CrcStore::record_spies()
{
	spies.clear();
	*(short *)spies.reserve(sizeof(short)) = spy_array.size();

	for( short spyRecno = 1; spyRecno <= spy_array.size(); ++spyRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !spy_array.is_deleted(spyRecno) )
		{
			checkNum = spy_array[spyRecno]->crc8();
		}
		*(CRC_TYPE *)spies.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}


void CrcStore::record_talk_msgs()
{
	talk_msgs.clear();
	*(short *)talk_msgs.reserve(sizeof(short)) = talk_res.talk_msg_count();

	for( short talkRecno = 1; talkRecno <= talk_res.talk_msg_count(); ++talkRecno)
	{
		CRC_TYPE checkNum = 0;
		if( !talk_res.is_talk_msg_deleted(talkRecno) )
		{
			 checkNum = talk_res.get_talk_msg(talkRecno)->crc8();
		}
		*(CRC_TYPE *)talk_msgs.reserve(sizeof(CRC_TYPE)) = checkNum;
		*(CRC_TYPE *)all_crc.reserve(sizeof(CRC_TYPE)) = checkNum;
	}
}

void CrcStore::record_all()
{
	all_crc.clear();
	record_nations();
	record_units();
	record_firms();
	record_towns();
	record_bullets();
	record_rebels();
	record_spies();
	record_talk_msgs();
	frame_check_num = all_crc.crc8();
}


void CrcStore::send_all()
{
	char *charPtr;
	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_NATION, nations.length() );
	memcpy(charPtr, nations.queue_buf, nations.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_UNIT, units.length() );
	memcpy(charPtr, units.queue_buf, units.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_FIRM, firms.length() );
	memcpy(charPtr, firms.queue_buf, firms.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_TOWN, towns.length() );
	memcpy(charPtr, towns.queue_buf, towns.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_BULLET, bullets.length() );
	memcpy(charPtr, bullets.queue_buf, bullets.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_REBEL, rebels.length() );
	memcpy(charPtr, rebels.queue_buf, rebels.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_SPY, spies.length() );
	memcpy(charPtr, spies.queue_buf, spies.length() );

	charPtr = (char *)remote.new_send_queue_msg(MSG_COMPARE_TALK, talk_msgs.length() );
	memcpy(charPtr, talk_msgs.queue_buf, talk_msgs.length() );
}


void CrcStore::send_frame()
{
	CRC_TYPE *dataPtr;
	dataPtr = (CRC_TYPE *)remote.new_send_queue_msg(MSG_COMPARE_CRC, sizeof(CRC_TYPE) );
	*dataPtr = frame_check_num;
}


// return 0 if equal
// otherwise not equal
int CrcStore::compare_remote(uint32_t remoteMsgId, char *dataPtr)
{
	VLenQueue *vq = NULL;
	const char *arrayName;

	switch(remoteMsgId)
	{
	case MSG_COMPARE_NATION:
		vq = &nations;
		arrayName = "nation_array";
		break;
	case MSG_COMPARE_UNIT:
		vq = &units;
		arrayName = "unit_array";
		break;
	case MSG_COMPARE_FIRM:
		vq = &firms;
		arrayName = "firm_array";
		break;
	case MSG_COMPARE_TOWN:
		vq = &towns;
		arrayName = "town_array";
		break;
	case MSG_COMPARE_BULLET:
		vq = &bullets;
		arrayName = "bullet_array";
		break;
	case MSG_COMPARE_REBEL:
		vq = &rebels;
		arrayName = "rebel_array";
		break;
	case MSG_COMPARE_SPY:
		vq = &spies;
		arrayName = "spy_array";
		break;
	case MSG_COMPARE_TALK:
		vq = &talk_msgs;
		arrayName = "talk_res";
		break;
	default:
		err_here();
		return 0;
	}

	err_when(vq->length() < sizeof(short));
	int rc = memcmp(vq->queue_buf, dataPtr, vq->length() );
	if( rc )
	{
		char *p1;
		char *p2;
		int diffOffset;

		// found out where is the first difference
		for( diffOffset = 0, p1 = vq->queue_buf, p2 = dataPtr; 
			diffOffset < vq->length() && *p1 == *p2; 
			++diffOffset, ++p1, ++p2);

		// ###### patch begin Gilbert 23/1 #######//
		// err.run("%s discrepency, offset : %d", arrayName, diffOffset);
		crc_error_string = arrayName;
		crc_error_string += "discrepency, offset : ";
		crc_error_string += diffOffset;
		// ###### patch end Gilbert 23/1 #######//
		diffOffset = 0;		// dummy code
	}
	return rc;
}


// return 0 if equal
// otherwise not equal
int CrcStore::compare_frame(char *dataPtr)
{
	if( *(CRC_TYPE*)dataPtr != frame_check_num )
	{
		send_all();
		return 1;
	}
	return 0;
}
