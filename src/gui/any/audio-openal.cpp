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

#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include <ALL.h>
#include <OBOX.h>
#include <OSYS.h>
#include <OVGALOCK.h>
#include <audio-openal.h>
#include <dbglog.h>
#include <wav_stream.h>

#define LWAV_STREAM_BUFSIZ    0x1000
#define LWAV_BANKS            4
#define LOOPWAV_STREAM_BUFSIZ 0x1000
#define LOOPWAV_BANKS         4

DBGLOG_DEFAULT_CHANNEL(Audio);

static bool check_al(int line)
{
	ALenum err = alGetError();
	if (err == AL_NO_ERROR)
		return true;

	ERR(__FILE__":%i: OpenAL error: 0x%x\n", line, err);
	return false;
}
#define check_al() check_al(__LINE__)

/* panning is in [-10,000; 10,000] */
static void set_source_panning(ALuint source, int panning)
{
	const float Z = -1.f;
	const float MAX_X = 20.f;

	panning = MAX(panning, -10000);
	panning = MIN(panning, 10000);

	alSource3f(source, AL_POSITION, MAX_X * panning / 10000.f, 0, Z);
}

/* volume is in [-10,000; 0] */
static void set_source_volume(ALuint source, int volume)
{
	volume = MAX(volume, -10000);
	volume = MIN(volume, 0);

	alSourcef(source, AL_GAIN, (volume + 10000.f) / 10000.f);
}

ALenum openal_format(AudioStream *as)
{
	switch (as->sample_size())
	{
		case 1:
			switch (as->channels())
			{
				case 1: return AL_FORMAT_MONO8;
				case 2: return AL_FORMAT_STEREO8;
			}
			break;

		case 2:
			switch (as->channels())
			{
				case 1: return AL_FORMAT_MONO16;
				case 2: return AL_FORMAT_STEREO16;
			}
			break;
	}

	abort();
}

template <typename M>
static typename M::key_type max_key(
	const M *map,
	const typename M::key_type &dflt = typename M::key_type())
{
	typename M::const_reverse_iterator i;

	i = map->rbegin();
	if (i != map->rend())
		return i->first;
	else
		return dflt;
}


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

	this->init_flag = this->wav_init_flag || this->mid_init_flag
	                  || this->cd_init_flag;

	return this->init_flag;
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
	ALCint attributes[] = {0};
	assert(!this->wav_init_flag);

	this->al_device = alcOpenDevice(NULL);
	if (this->al_device == NULL)
	{
		fprintf(stderr, __FILE__":%i: alcOpenDevice failed\n",
			__LINE__);
		goto err;
	}

	this->al_context = alcCreateContext(this->al_device, attributes);
	if (this->al_context == NULL)
	{
		fprintf(stderr, __FILE__":%i: alcCreateContext failed: 0x%x\n",
			__LINE__, alcGetError(this->al_device));
		goto err;
	}

	if (!alcMakeContextCurrent(this->al_context))
	{
		ERR("alcMakeContextCurrent failed\n");
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
	return 0;
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

int Audio::play_long_wav(const char *file_name, DsVolume volume)
{
	const int BUFFER_COUNT = 4;

	MSG("play_long_wav(\"%s\")\n", file_name);

	StreamContext *sc = NULL;
	WavStream *ws = NULL;
	int id;

	ws = new WavStream;
	if (!ws->open(file_name))
		goto err;

	sc = new StreamContext;

	if (!sc->init(ws))
		goto err;

	set_source_panning(sc->source, volume.ds_pan);
	set_source_volume(sc->source, volume.ds_vol);

	if (!check_al())
		goto err;

	if (!sc->stream_data(BUFFER_COUNT))
		goto err;

	id = max_key(&this->streams) + 1;
	this->streams[id] = sc;

	return id;

err:
	delete sc;
	delete ws;
	return 0;
}

// stop a short sound effect started by play_long_wav
//
// <int>        the serial no returned by play_long_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int Audio::stop_long_wav(int id)
{
	StreamMap::iterator itr;
	StreamContext *sc;

	MSG("stop_long_wav(%i)\n", id);

	itr = this->streams.find(id);
	if (itr == this->streams.end())
		return 1;

	sc = itr->second;
	sc->stop();
	this->streams.erase(itr);
	delete sc;

	return 1;
}

// return wheather a short sound effect is stopped
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
int Audio::is_long_wav_playing(int id)
{
	return (this->streams.find(id) != this->streams.end());
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

void Audio::fade_out_loop_wav(int ch, int fade_rate_msec)
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
	return 0;
}

void Audio::yield()
{
	VgaFrontLock vgaLock;
	StreamMap::iterator si;

	for (si = this->streams.begin(); si != this->streams.end();)
	{
		StreamContext *sc = si->second;
		sc->stream_data();

		if (!sc->streaming)
		{
			/* TODO: should make sure it's drained first */
			delete sc;
			this->streams.erase(si++);
		}
		else
			++si;
	}
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
	return 0;
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
	return 0;
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


Audio::StreamContext::StreamContext()
{
	this->stream = NULL;
	this->source = 0;
	this->fade_frames_played = 0;
	this->fade_frames = 0;
	this->looping = false;
	this->loop_start_frame = 0;
	this->streaming = true;
}

Audio::StreamContext::~StreamContext()
{
	/* TODO: drain */

	if (this->source != 0)
		alDeleteSources(1, &this->source);

	delete this->stream;
}

bool Audio::StreamContext::init(AudioStream *as)
{
	if (this->source != 0)
		return false;

	alGenSources(1, &this->source);
	if (!check_al())
		goto err;

	this->stream = as;

	return true;

err:
	return false;
}

bool Audio::StreamContext::stream_data(int new_buffer_count)
{
	const size_t BUFFER_SIZE = 0x4000;

	/*
	 * This constant determines how many milliseconds of audio data go into
	 * one buffer.  The larger it is, the less probability of skipping, but
	 * the longer the delay from calling stop() to the point when it
	 * actually stops.
	 */
	const size_t MAX_BUFFER_TIME_MS = 50;

	uint8_t data[BUFFER_SIZE];
	size_t frames_read;
	size_t max_frames;
	ALenum format;
	ALuint buf;
	ALint state;

	if (!this->streaming)
		return false;

	format = openal_format(this->stream);
	max_frames = this->stream->frame_rate() * MAX_BUFFER_TIME_MS / 1000;

	for (;;)
	{
		buf = 0;

		if (new_buffer_count > 0)
		{
			alGenBuffers(1, &buf);
			if (!check_al())
				goto err;

			new_buffer_count--;
		}
		else
		{
			ALint processed;

			alGetSourcei(this->source, AL_BUFFERS_PROCESSED,
			             &processed);

			if (processed == 0)
				break;

			alSourceUnqueueBuffers(this->source, 1, &buf);
			if (!check_al())
				goto err;
		}

		size_t space_frames = sizeof(data) / this->stream->frame_size();
		space_frames = MIN(space_frames, max_frames);
		frames_read = this->stream->read(data, space_frames);

		/* TODO: handle looping and fading */
		if (frames_read == 0)
		{
			printf("END OF STREAM\n");
			this->streaming = false;
			return true;
		}

		alBufferData(buf, format, data,
		             frames_read * this->stream->frame_size(),
		             this->stream->frame_rate());
		if (!check_al())
			goto err;

		alSourceQueueBuffers(this->source, 1, &buf);
		if (!check_al())
			goto err;
	}

	alGetSourcei(this->source, AL_SOURCE_STATE, &state);

	if (state != AL_PLAYING)
	{
		alSourcePlay(this->source);
		check_al();
	}

	return true;

err:
	if (buf != 0)
		alDeleteBuffers(1, &buf);

	this->streaming = false;
	return false;
}

void Audio::StreamContext::stop()
{
	ALint count;
	ALuint buf;

	alSourceStop(this->source);

	for (;;)
	{
		alGetSourcei(this->source, AL_BUFFERS_PROCESSED, &count);
		if (count == 0)
			break;

		alSourceUnqueueBuffers(this->source, 1, &buf);
		alDeleteBuffers(1, &buf);
	}
}
