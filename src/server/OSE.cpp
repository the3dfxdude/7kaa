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

// Filename    : OSE.CPP
// Description : Sound Effect Controller
// Owner       : Gilbert

#include <OSE.h>
#include <OAUDIO.h>
#include <OGAMESET.h>
#include <OMISC.h>
#include <OCONFIG.h>

#define MIN_AUDIO_VOL 10

// ------ Begin Function SERequest::add_request -------//
void SERequest::add_request(RelVolume relVolume)
{
	if( req_used < MAX_SE_STORE)
	{
		play_vol[req_used] = relVolume;
		req_used++;
	}
	else
	{
		// not enough space, remove the MIN volume one.
		RelVolume minVolume = relVolume;
		for( int i = 0; i < MAX_SE_STORE; ++i)
		{
			if( play_vol[i].rel_vol < minVolume.rel_vol)
			{
				RelVolume temp;
				// swap volume[i] and minVolume
				temp = play_vol[i];
				play_vol[i] = minVolume;
				minVolume = temp;
			}
		}
	}
}
// ------ End Function SERequest::add_request -------//


// ------ Begin Function SERequest::max_entry -------//
int SERequest::max_entry()
{
	err_when( req_used <= 0);

	int maxEntry = 0;
	RelVolume *maxVolume = &play_vol[maxEntry];
	for( int i = 1; i < req_used; ++i)
	{
		if( maxVolume->rel_vol < play_vol[i].rel_vol)
		{
			maxVolume = &play_vol[i];
			maxEntry = i;
		}
	}
	
	return maxEntry;
}
// ------ End Function SERequest::max_entry -------//


// ------ Begin Function SERequest::remove_request -------//
void SERequest::remove_request(int slot)
{
	if( slot >= req_used || slot < 0)
		return;

	// ---- move element after slot -----/
	for( int i = slot+1; i < req_used; ++i)
	{
		play_vol[i-1] = play_vol[i];
	}
	req_used--;
}
// ------ End Function SERequest::remove_request -------//


// ------ Begin Function SERequest::clear_request -------//
void SERequest::clear_request()
{
	req_used = 0;
}
// ------ End Function SERequest::clear_request -------//


// ------ Begin Function SECtrl::SECtrl -------//
SECtrl::SECtrl(Audio *audioPtr) : audio_ptr(audioPtr), res_supp(audioPtr->wav_res)
{
	init_flag = 0;
	audio_flag = 0;
	req_pool = NULL;
	last_cycle = NULL;
	max_sound_effect = 0;
	max_supp_effect = 0;
	total_effect = 0;
	biased_se = 0;
}
// ------ End Function SECtrl::SECtrl -------//


// ------ Begin Function SECtrl::~SECtrl ------//
SECtrl::~SECtrl()
{
	deinit();
}
// ------ End Function SECtrl::~SECtrl ------//


// ------ Begin Function SECtrl::init -------//
void SECtrl::init()
{
	deinit();

	audio_flag = audio_ptr->wav_init_flag;
	if( !audio_flag )
	{
		init_flag = 1;
		return;
	}

	//----- open wave resource file -------//

	String str;

	str  = DIR_RES;
	str += "A_WAVE1.RES";

	res_wave.init(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_info();

	// ----- clear last_cycle array and wave_ptr --------//
	init_flag = 1;
	clear();
}
// ------ End Function SECtrl::init -------//


// ------ Begin Function SECtrl::deinit -------//
void SECtrl::deinit()
{
	if( init_flag )
	{
		init_flag = 0;
		if( audio_flag )
		{
			mem_del(req_pool);
			mem_del(last_cycle);
		}
	}
}
// ------ End Function SECtrl::deinit -------//


// ------ Begin Function SECtrl::load_info -------//
void SECtrl::load_info()
{
	int count = max_sound_effect = res_wave.rec_count;
	int suppCount = max_supp_effect = res_supp.rec_count;
	total_effect = max_sound_effect + max_supp_effect;
	
	req_pool = (SERequest *)mem_add(total_effect * sizeof(SERequest) );
	last_cycle = (char *)mem_add(total_effect * sizeof(char));

	short j;
	for(j=0; j < count; ++j)
	{
		req_pool[j].resx_id = j+1;
		req_pool[j].wave_ptr = res_wave.get_data(j+1);		// wave data pointer
		last_cycle[j] = 0;
	}

	for(short k=0; k < suppCount; ++k, ++j)
	{
		req_pool[j].resx_id = k+1;
		req_pool[j].wave_ptr = NULL;
		last_cycle[j] = 0;
	}
}
// ------ End Function SECtrl::load_info -------//


// ------ Begin Function SECtrl::clear -------//
void SECtrl::clear()
{
	for( int j = 0; j < total_effect; ++j)
	{
		req_pool[j].clear_request();
	}
}
// ------ End Function SECtrl::clear -------//


// ------ Begin Function SECtrl::request -------//
//
// Request to sound an effect
//
// <int> soundEffect        the id of the sound effect, return from SECtrl::search_effect_id
// <long> vol               volume (0 - 100 MAX loudness)
// <long> pan               pan (-10000 = full left; 10000 = full right)
// note the request is abolished if vol is 0 or soundEffect is 0
//
void SECtrl::request(int soundEffect, RelVolume relVolume)
{
	if( !audio_flag || !config.sound_effect_flag)
		return;					// skip if audio cannot init wave device
	err_when( soundEffect < 0 || soundEffect > total_effect);
	if( relVolume.rel_vol >= MIN_AUDIO_VOL && soundEffect)
		req_pool[soundEffect-1].add_request(relVolume);
}


void SECtrl::request(char *soundName, RelVolume relVolume)
{
	if( !audio_flag || !config.sound_effect_flag)
		return;					// skip if audio cannot init wave device
	int soundEffect = search_effect_id(soundName);
	err_when( soundEffect < 0 || soundEffect > total_effect);
	if( relVolume.rel_vol >= MIN_AUDIO_VOL && soundEffect)
		req_pool[soundEffect-1].add_request(relVolume);
}
// ------ End Function SECtrl::request -------//

// ------ Begin Function SECtrl::flush -------//
void SECtrl::flush()
{
	err_when(!init_flag);
	// ##### begin Gilbert 11/11 ######//
	// err_when(!audio_ptr->init_flag);
	// ##### end Gilbert 11/11 ######//
	if( !audio_flag || !config.sound_effect_flag)
	{
		clear();
		return;					// skip if audio cannot init wave device
	}
	int chCount = audio_ptr->get_free_wav_ch();
	int reqCount = 0, reqSum = 0;
	int i,j,k;
	SERequest *seRequest;
	
	k = 0;
	cached_size = 0;
	for( j = 0, seRequest=req_pool; j < total_effect ; ++j, ++seRequest)
	{
		if( seRequest->req_used > 0)
		{
			reqCount++;
			reqSum += seRequest->req_used;
		}

		// cached sound effect
		if( (seRequest->req_used > 0 || last_cycle[j]) && cached_size < MAX_SE_CACHED )
		{
			cached_index[cached_size++] = j;
		}
	}

	if( reqSum <= chCount )
	{
		// --------- enough for all requests --------//
		// for( j = 0, seRequest=req_pool; j < total_effect; ++j, ++seRequest)
		for( k = 0; k < cached_size; ++k)
		{
			j = cached_index[k]; seRequest = req_pool + j;

			last_cycle[j] = 0;
			for( i = seRequest->req_used-1; i >= 0; --i)
			{
				if( seRequest->wave_ptr)
				{
					audio_ptr->play_resided_wav( seRequest->wave_ptr,
						seRequest->play_vol[i]);
				}
				else
				{
					audio_ptr->play_wav( seRequest->resx_id,
						seRequest->play_vol[i]);
				}
				last_cycle[j]++;
				chCount--;
			}
		}
		reqSum = 0;
		reqCount = 0;
	}
	else if( reqCount <= chCount )
	{
		// --------- one channel for one sound effect --------//
		for( k = 0; k < cached_size; ++k)
		{
			j = cached_index[k]; seRequest = req_pool + j;
			last_cycle[j] = 0;
			if( seRequest->req_used > 0)
			{
				i = seRequest->max_entry();
				if( seRequest->wave_ptr)
				{
					audio_ptr->play_resided_wav( seRequest->wave_ptr,
						seRequest->play_vol[i]);
				}
				else
				{
					audio_ptr->play_wav( seRequest->resx_id,
						seRequest->play_vol[i]);
				}
				last_cycle[j]++;
			}
		}
	}
	else
	{
		// -------- not enough for each sound effect ------//

		// ------- one channel for one sound effect --------//
		for( k = 0; k < cached_size && biased_se > cached_index[k]; ++k);
		if( k >= cached_size)
			k = 0;

		for( int c = 0; chCount > 0 && c < cached_size; ++c)
		{
			if( ++k >= cached_size)
				k = 0;
			j = cached_index[k]; seRequest = req_pool + j;
			if( seRequest->req_used > 0 && !last_cycle[j])
			{
				i = seRequest->max_entry();
				if( seRequest->wave_ptr )
				{
					audio_ptr->play_resided_wav( seRequest->wave_ptr,
						seRequest->play_vol[i]);
				}
				else
				{
					audio_ptr->play_wav( seRequest->resx_id,
						seRequest->play_vol[i]);
				}
				last_cycle[j]++;
				chCount--;
				biased_se = j;
			}
			else
			{
				last_cycle[j] = 0;
			}
		}
	}

	clear();
}
// ------ End Function SECtrl::flush -------//


// ------ Begin Function SECtrl::get_effect_name -------//
//
// return the name of the sound effect
// int j           the id of the sound effect
//
char *SECtrl::get_effect_name(int j)
{
	err_when(!init_flag);
	err_when( j < 1 || j > total_effect );
	if( j > max_sound_effect )
		return res_supp.data_name(j - max_sound_effect);
	else
		return res_wave.data_name(j);
}
// ------ End Function SECtrl::get_effect_name -------//


// ------ Begin Function SECtrl::search_effect_id -------//
//
// find the sound effect id of an sound effect
//
// <char *> effectName      the name of the effect name
//
int SECtrl::search_effect_id(const char *effectName)
{
	err_when(!init_flag);
	if( !audio_flag )
		return 0;					// skip if audio cannot init wave device

	int idx = res_wave.get_index(effectName);
	if( idx )
		return idx;

	idx = res_supp.get_index(effectName);
	if( idx )
		return idx + max_sound_effect;

	return 0;
}
// ------ End Function SECtrl::search_effect_id -------//


// ------ Begin Function SECtrl::search_effect_id -------//
//
// find the sound effect id of an sound effect
//
// <char *> effectName      the name of the effect name
// <int> len                the size of the effectName
//
int SECtrl::search_effect_id(char *effectName, int len)
{
	err_when(!init_flag);
	if( !audio_flag)
		return 0;					// skip if audio cannot init wave device

	char tmpStr[16];
	err_when(len >= 16);
	memcpy(tmpStr, effectName, len);
	tmpStr[len] = '\0';
	m.rtrim(tmpStr);

	int idx = res_wave.get_index(tmpStr);
	if( idx )
		return idx;

	idx = res_supp.get_index(tmpStr);
	if( idx )
		return idx + max_sound_effect;

	return 0;

}
// ------ End Function SECtrl::search_effect_id -------//

/*
// ------ Begin Function SECtrl::sound_volume --------//
//
// calculate the volume from a location
//
// <short> locX, locY  - location x, y relative to the center of screen
// [short] limit       - volume is zero if dist > limit
// [short] drop        - volume is dropped (linearly) to zero when dist = drop
//                     drop > limit
//

const default_vol_limit = 20;
const default_vol_drop = 100;
long SECtrl::sound_volume(short locX, short locY)
{
	short dist = MAX( locX >= 0? locX : -locX, locY >= 0? locY:-locY);
	err_when( default_vol_drop <= default_vol_limit);

	if( dist > default_vol_limit)
		return 0;
	else
		return 90 - dist * 90 / default_vol_drop;
}


long SECtrl::sound_volume(short locX, short locY, short limit, short drop)
{
	short dist = MAX( locX >= 0? locX : -locX, locY >= 0? locY:-locY);
	err_when( drop <= limit);

	if( dist > limit)
		return 0;
	else
		return 90 - dist * 90 / drop;
}
// ------ End Function SECtrl::sound_volume --------//


// ------ Begin Function SECtrl::sound_pan --------//
//
// calculate the pan setting of a location
//
// short locX, locY  - location x, y relative to the center of screen
// short drop        - panning is set to extreme value when abs(locX) >= drop
// 

const default_pan_drop = 100;
long SECtrl::sound_pan(short locX, short locY)
{
	if( locX >= default_pan_drop )
		return 10000;
	if( locX <= -default_pan_drop )
		return -10000;
	return 10000 / default_pan_drop * locX;
}


long SECtrl::sound_pan(short locX, short locY, short drop)
{
	if( locX >= drop )
		return 10000;
	if( locX <= -drop )
		return -10000;
	return 10000 * locX / drop;
}
// ------ End Function SECtrl::sound_pan --------//
*/

// ------- Begin Function SECtrl::immediate_sound ------------//
int SECtrl::immediate_sound(const char *soundName, RelVolume relVolume)
{
	if( !config.sound_effect_flag )
		return 0;

	int effectId = search_effect_id(soundName);
	if( effectId )
	{
		SERequest *seRequest = req_pool + effectId-1;
		if( seRequest->wave_ptr )
			return audio_ptr->play_resided_wav( seRequest->wave_ptr, relVolume);
		else
			return audio_ptr->play_wav( seRequest->resx_id, relVolume);
	}
	return 0;
}
// ------- End Function SECtrl::immediate_sound ------------//
