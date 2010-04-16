/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010 Enlight Software Ltd. and others
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

//Filename    : OAUDIO.CPP
//Description : Object Midi Audio and Digitized Sound

#define WARN_UNIMPLEMENTED(func) \
	do { \
		fprintf(stderr, __FILE__":%i: %s unimplemented.\n", \
			__LINE__, func); \
	} while (0)

#include <cassert>
#include <climits>
#include <stdio.h>

#include <OBOX.h>
#include <OSYS.h>
#include <OVGALOCK.h>
#include <audio-openal.h>

#define LWAV_STREAM_BUFSIZ    0x1000
#define LWAV_BANKS            4
#define LOOPWAV_STREAM_BUFSIZ 0x1000
#define LOOPWAV_BANKS         4

Audio::Audio()
{
	this->al_context = NULL;
	this->al_device  = NULL;
}

Audio::~Audio()
{
	deinit();
}

// Initialize the mid driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init()
{
	this->init_wav();

	return 1;
}

void Audio::deinit()
{
	WARN_UNIMPLEMENTED("deinit");
}

// Initialize digitized wav driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init_wav()
{
	assert(!this->wav_init_flag);

	this->al_device = alcOpenDevice(NULL);
	if (this->al_device == NULL)
	{
		fprintf(stderr, __FILE__":%i: alcOpenDevice failed\n",
			__LINE__);
		goto err;
	}

	this->al_context = alcCreateContext(this->al_device, NULL);
	if (this->al_context == NULL)
	{
		fprintf(stderr, __FILE__":%i: alcCreateContext failed: 0x%x\n",
			__LINE__, alcGetError(this->al_device));
		goto err;
	}

	this->wav_init_flag = 1;
	return 1;

err:
	this->deinit_wav();
	return 0;
}

void Audio::deinit_wav()
{
	this->wav_init_flag = 0;

	if (this->al_context != NULL)
	{
		alcDestroyContext(this->al_context);
		this->al_context = NULL;
	}

	if (this->al_device != NULL)
	{
		alcCloseDevice(this->al_device);
		this->al_device = NULL;
	}
}

// Initialize MIDI mid driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init_mid()
{
	WARN_UNIMPLEMENTED("init_mid");
	this->mid_init_flag = 0;
	return this->mid_init_flag;
}

void Audio::deinit_mid()
{
}

// Initialize the audio CD player
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int Audio::init_cd()
{
	WARN_UNIMPLEMENTED("init_cd");
	this->cd_init_flag = 0;
	return this->cd_init_flag;
}

void Audio::deinit_cd()
{
}

// Play a midi mid from the mid resource file
//
// <char*> midName = name of the mid in the resource file
//
// return : <int> 1 - mid loaded and is playing
//                0 - mid not played
//
int Audio::play_mid(char* midName)
{
	WARN_UNIMPLEMENTED("play_mid");
}

void Audio::stop_mid()
{
	WARN_UNIMPLEMENTED("stop_mid");
}

// Play digitized wav from the wav resource file
//
// <char*>        wavName = name of the wav in the resource file
// long vol = volume (0 to 100)
// long pan = pan (-10000 to 10000)
//
// return : <int> non-zero - wav loaded and is playing, return a serial no. to be referred in stop_wav and is_wav_playing
//                0 - wav not played
//
int Audio::play_wav(char* wavName, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("play_wav");
	return 0;
}

// Play digitized wav from the wav resource file
//
// short resIdx = index of wave file in A_WAVE2.RES
// long vol = volume (0 to 100)
// long pan = pan (-10000 to 10000)
//
// return : <int> 1 - wav loaded and is playing
//                0 - wav not played
//
int Audio::play_wav(short resIdx, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("play_wav");
	return 0;
}

// Play digitized wav from the wav file in memory
//
// <char*>        wavBuf = point to the wav in memory
// long vol = volume (0 to 100)
// long pan = pan (-10000 to 10000)
//
// return : <int> 1 - wav loaded and is playing
//                0 - wav not played
//
int Audio::play_resided_wav(char* wavBuf, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("play_resided_wav");
	return 0;
}

int Audio::get_free_wav_ch()
{
	WARN_UNIMPLEMENTED("get_free_wav_ch");
	return 0;
}

// stop a short sound effect started by play_wav or play_resided_wav
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int Audio::stop_wav(int serial)
{
	WARN_UNIMPLEMENTED("stop_wav");
	return 1;
}

// return wheather a short sound effect is stopped
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
int Audio::is_wav_playing(int serial)
{
	WARN_UNIMPLEMENTED("is_wav_playing");
	return 0;
}

// Play digitized wav from the wav file
// suitable for very large wave file
//
// <char*>        wavName = name of the wave file
//
// return : <int> 1 - wav loaded and is playing
//                0 - wav not played
// Audio::yield() keeps on feeding data to it

int Audio::play_long_wav(const char *wavName, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("play_long_wav");
	return 0;
}

// stop a short sound effect started by play_long_wav
//
// <int>        the serial no returned by play_long_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int Audio::stop_long_wav(int serial)
{
	WARN_UNIMPLEMENTED("stop_long_wav");
	return 1;
}

// return wheather a short sound effect is stopped
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
int Audio::is_long_wav_playing(int serial)
{
	WARN_UNIMPLEMENTED("is_long_wav_playing");
	return 0;
}

// Play digitized wav from the wav resource file
//
// <char*>        wavName = name of the wav in the resource file
// int				repeatOffset = offset of wave data to play on repeat
//											i.e. 0 to start of wave data
//
// return : <int> channel number (1 - MAX_LOOP_WAV_CH)
//          0     not played
//
int Audio::play_loop_wav(const char *wavName, int repeatOffset, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("play_loop_wav");
	return 0;
}

void Audio::volume_loop_wav(int ch, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("volume_loop_wav");
}

void Audio::fade_out_loop_wav(int ch, int fadeRate)
{
	WARN_UNIMPLEMENTED("fade_out_loop_wav");
}

DsVolume Audio::get_loop_wav_volume(int ch)
{
	WARN_UNIMPLEMENTED("get_loop_wav_volume");
	return DsVolume(0, 0);
}

int Audio::is_loop_wav_fading(int ch)
{
	WARN_UNIMPLEMENTED("is_loop_wav_fading");
}

void Audio::yield()
{
	VgaFrontLock vgaLock;
	//WARN_UNIMPLEMENTED("yield");
}

void Audio::stop_wav()
{
	WARN_UNIMPLEMENTED("stop_wav");
}

void Audio::stop_long_wav()
{
	WARN_UNIMPLEMENTED("stop_long_wav");
}

void Audio::stop_loop_wav(int ch)
{
	WARN_UNIMPLEMENTED("stop_loop_wav");
}

// <int> trackId - the id. of the CD track to play.
//
int Audio::play_cd(int trackId, int volume)
{
	WARN_UNIMPLEMENTED("play_cd");
}

void Audio::stop_cd()
{
	WARN_UNIMPLEMENTED("stop_cd");
}

int Audio::is_mid_playing()
{
	WARN_UNIMPLEMENTED("is_mid_playing");
	return 0;
}

int Audio::is_wav_playing()
{
	WARN_UNIMPLEMENTED("is_wav_playing");
}

int Audio::is_cd_playing()
{
	WARN_UNIMPLEMENTED("is_cd_playing");
	return 0;
}

void Audio::toggle_mid(int midFlag)
{
	WARN_UNIMPLEMENTED("toggle_mid");
}

void Audio::toggle_wav(int wavFlag)
{
	WARN_UNIMPLEMENTED("toggle_wav");
}

void Audio::toggle_cd(int cdFlag)
{
	WARN_UNIMPLEMENTED("toggle_cd");
}

// Set mid volume
//
// <int> midVolume = mid volume, 0-100
//
void Audio::set_mid_volume(int midVolume)
{
	WARN_UNIMPLEMENTED("set_mid_volume");
}


// Set wav volume
//
// <int> wavVolume = wav volume, 0-100
//
void Audio::set_wav_volume(int wavVolume)
{
	WARN_UNIMPLEMENTED("set_wav_volume");
}

int Audio::get_wav_volume() const
{
  return 0;
}

// Set cd volume
//
// <int> cdVolume = cd volume, 0-100
//
void Audio::set_cd_volume(int cdVolume)
{
	WARN_UNIMPLEMENTED("set_cd_volume");

}

void Audio::volume_long_wav(int serial, DsVolume dsVolume)
{
	WARN_UNIMPLEMENTED("volume_long_wav");
}

// vi: sw=8
