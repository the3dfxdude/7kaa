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

//Filename    : openal_audio.cpp
//Description : Object Midi OpenALAudio and Digitized Sound

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <vector>

#include <ALL.h>
#include <OBOX.h>
#include <OSYS.h>
#include <OVGALOCK.h>
#include <dbglog.h>
#include <file_input_stream.h>
#include <mem_input_stream.h>
#include <openal_audio.h>
#include <wav_stream.h>

#define LWAV_STREAM_BUFSIZ    0x1000
#define LWAV_BANKS            4
#define LOOPWAV_STREAM_BUFSIZ 0x1000
#define LOOPWAV_BANKS         4

#define PANNING_Z     (-1.f)
#define PANNING_MAX_X (20.f)

#define WARN_UNIMPLEMENTED(func) \
	do { \
		ERR(__FILE__":%i: %s unimplemented.\n", __LINE__, func); \
	} while (0)

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

/* By this conversion, 0dB == 1 and -6dB == 0.5 */
static inline float millibels_to_ratio(long cb)
{
	if (cb <= -10000)
		return 0.f;

	return pow(10.f, cb / 2000.f);
}

static inline long ratio_to_millibels(float ratio)
{
	const float log10 = log(10.f);

	if (ratio == 0.f)
		return -10000;

	return (2000.f * log(ratio) / log10 + .5f);
}

/* panning is in [-10,000; 10,000] */
static void set_source_panning(ALuint source, int panning)
{
	panning = MAX(panning, -10000);
	panning = MIN(panning, 10000);

	alSource3f(source, AL_POSITION,
	           PANNING_MAX_X * panning / 10000.f,
	           0,
	           PANNING_Z);
}

/* volume is in [-10,000; 0] */
static void set_source_volume(ALuint source, int volume_millibels)
{
	volume_millibels = MAX(volume_millibels, -10000);
	volume_millibels = MIN(volume_millibels, 0);

	MSG("set_source_volume(%x, %f)\n",
	    source, millibels_to_ratio(volume_millibels));

	alSourcef(source, AL_GAIN, millibels_to_ratio(volume_millibels));
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

/* ueturns an unused key that's greater than 0 */
template <typename M>
static typename M::key_type unused_key(const M *map)
{
	typename M::key_type id;

	id = max_key(map) + 1;
	id = MAX(id, 1);

	if (map->find(id) == map->end())
		return id;

	do
	{
		id++;
		id = MAX(id, 1);
	}
	while (map->find(id) != map->end());

	return id;
}


OpenALAudio::OpenALAudio()
{
	this->al_context = NULL;
	this->al_device  = NULL;
}

OpenALAudio::~OpenALAudio()
{
	this->deinit();
}

// Initialize the mid driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int OpenALAudio::init()
{
	this->wav_flag = true;
	this->mid_flag = true;
	this->cd_flag = true;

	this->init_wav();

	this->init_flag = this->wav_init_flag || this->mid_init_flag
	                  || this->cd_init_flag;

	return this->init_flag;
}

void OpenALAudio::deinit()
{
	this->init_flag = 0;
	this->deinit_wav();
	this->deinit_mid();
	this->deinit_cd();
}

// Initialize digitized wav driver
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int OpenALAudio::init_wav()
{
	ALCint size;

	std::vector<ALCint> attributes;

	assert(!this->wav_init_flag);

	this->wav_res.init(DIR_RES"A_WAVE2.RES", 0, 0);

	this->al_device = alcOpenDevice(NULL);
	if (this->al_device == NULL)
	{
		ERR("alcOpenDevice failed\n");
		goto err;
	}

	attributes.push_back(0);

	this->al_context = alcCreateContext(this->al_device, &attributes[0]);
	if (this->al_context == NULL)
	{
		ERR("alcCreateContext failed: 0x%x\n",
		    alcGetError(this->al_device));
		goto err;
	}

	if (!alcMakeContextCurrent(this->al_context))
	{
		ERR("alcMakeContextCurrent failed: 0x%x\n",
		    alcGetError(this->al_device));
		goto err;
	}

	attributes.clear();
	alcGetIntegerv(this->al_device, ALC_ATTRIBUTES_SIZE, 1, &size);
	attributes.resize(size);
	alcGetIntegerv(this->al_device, ALC_ALL_ATTRIBUTES,
	               attributes.size(), &attributes[0]);

	this->max_sources = 16; /* default, in case OpenAL doesn't tell us */

	for (int n = 0;; n += 2)
	{
		if (attributes[n] == 0)
			break;

		switch (attributes[n])
		{
			case ALC_MONO_SOURCES:
				MSG("ALC_MONO_SOURCES: %i\n",
				    attributes[n + 1]);
				this->max_sources = attributes[n + 1];
				break;
			case ALC_STEREO_SOURCES:
				MSG("ALC_STEREO_SOURCES: %i\n",
				    attributes[n + 1]);
				 attributes[n + 1];
				break;
		}
	}

	this->wav_init_flag = true;
	return 1;

err:
	this->deinit_wav();
	return 0;
}

void OpenALAudio::deinit_wav()
{
	this->wav_init_flag = false;

	this->stop_wav();

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
int OpenALAudio::init_mid()
{
	WARN_UNIMPLEMENTED("init_mid");
	this->mid_init_flag = 0;
	return this->mid_init_flag;
}

void OpenALAudio::deinit_mid()
{
	WARN_UNIMPLEMENTED("deinit_mid");
}

// Initialize the audio CD player
//
// return : <int> 1 - initialized successfully
//                0 - init fail
//
int OpenALAudio::init_cd()
{
	WARN_UNIMPLEMENTED("init_cd");
	this->cd_init_flag = 0;
	return this->cd_init_flag;
}

void OpenALAudio::deinit_cd()
{
	WARN_UNIMPLEMENTED("deinit_cd");
}

// Play a midi mid from the mid resource file
//
// <char*> midName = name of the mid in the resource file
//
// return : <int> 1 - mid loaded and is playing
//                0 - mid not played
//
int OpenALAudio::play_mid(char *midName)
{
	WARN_UNIMPLEMENTED("play_mid");
	return 0;
}

void OpenALAudio::stop_mid()
{
	WARN_UNIMPLEMENTED("stop_mid");
}

// Play digitized wav from the wav resource file
//
// <char*>        file_name = name of the wav in the resource file
//
// return : <int> non-zero - wav loaded and is playing, return a serial no. to be referred in stop_wav and is_wav_playing
//                0 - wav not played
//
int OpenALAudio::play_wav(char *file_name, const DsVolume &vol)
{
	int idx;

	if (!this->wav_init_flag || !this->wav_flag)
		return 0;

	MSG("play_wav(\"%s\")\n", file_name);

	if (m.is_file_exist(file_name))
		return this->play_long_wav(file_name, vol);

	idx = this->wav_res.get_index(file_name);
	if (idx == 0)
		return 0;

	return this->play_wav(idx, vol);
}

// Play digitized wav from the wav resource file
//
// index - index of wave file in A_WAVE2.RES
//
// return: 1 - wav loaded and is playing
//         0 - wav not played
//
int OpenALAudio::play_wav(short index, const DsVolume &vol)
{
	int size;
	char *data;
	MemInputStream *in;

	if (!this->wav_init_flag || !this->wav_flag)
		return 0;

	/* get size by ref */
	if (this->wav_res.get_file(index, size) == NULL)
		return 0;

	data = new char[size];

	this->wav_res.set_user_buf(data, size);
	if (this->wav_res.get_data(index) == NULL)
	{
		this->wav_res.reset_user_buf();
		delete[] data;
		return 0;
	}

	this->wav_res.reset_user_buf();

	in = new MemInputStream;
	in->open(data, size);

	return this->play_long_wav(in, vol);
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
int OpenALAudio::play_resided_wav(char *buf, const DsVolume &vol)
{
	uint32_t size;
	MemInputStream *in;

	if (!this->wav_init_flag || !this->wav_flag)
		return 0;

	MSG("play_resided_wav(%p))\n", buf);

	/* read the wav size from the RIFF header */
	size = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);
	size += 8;

	in = new MemInputStream;
	in->open(buf, size, false);

	return this->play_long_wav(in, vol);
}

int OpenALAudio::get_free_wav_ch()
{
	int free_count;

	if (!this->wav_init_flag)
		return 0;

	free_count = this->max_sources
	             - static_cast<int>(this->streams.size());

	return MAX(free_count, 0);
}

// stop a short sound effect started by play_wav or play_resided_wav
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int OpenALAudio::stop_wav(int id)
{
	return this->stop_long_wav(id);
}

// return wheather a short sound effect is stopped
//
// <int>        the serial no returned by play_wav or play_resided_wav
//
int OpenALAudio::is_wav_playing(int id)
{
	return this->is_long_wav_playing(id);
}

// Play digitized wav from the wav file
// suitable for very large wave file
//
// file_name - name of the wave file
//
// return: 1 - wav loaded and is playing
//         0 - wav not played
// OpenALAudio::yield() keeps on feeding data to it
//
int OpenALAudio::play_long_wav(const char *file_name, const DsVolume &vol)
{
	FileInputStream *in = new FileInputStream;

	if (!this->wav_init_flag)
		return 0;

	MSG("play_long_wav(\"%s\", (%li, %li))\n", file_name,
	    vol.ds_vol, vol.ds_pan);

	if (!in->open(file_name))
	{
		delete in;
		return 0;
	}

	return this->play_long_wav(in, vol);
}

/*
 * in  - wav file stream to play.  Claims ownership and will delete it.
 * vol - volume/panning
 *
 * return: 1 on success, 0 on failure
 */
int OpenALAudio::play_long_wav(InputStream *in, const DsVolume &vol)
{
	const int BUFFER_COUNT = 4;

	StreamContext *sc = NULL;
	WavStream *ws = NULL;
	int id;

	assert(this->wav_init_flag);

	ws = new WavStream;
	if (!ws->open(in))
	{
		delete in;
		goto err;
	}

	sc = new StreamContext;

	if (!sc->init(ws))
		goto err;

	set_source_panning(sc->source, vol.ds_pan);
	set_source_volume(sc->source, vol.ds_vol + this->wav_volume);

	if (!check_al())
		goto err;

	sc->stream_data(BUFFER_COUNT);

	id = unused_key(&this->streams);
	this->streams[id] = sc;

	return id;

err:
	delete sc;
	delete ws;
	return 0;
}

// stop a short sound effect started by play_long_wav
//
// id - the serial no returned by play_long_wav
//
// return 1 - channel is found and stopped / channel not found
// return 0 - cannot stop the channel
//
int OpenALAudio::stop_long_wav(int id)
{
	StreamMap::iterator itr;
	StreamContext *sc;

	if (!this->wav_init_flag)
		return 1;

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
int OpenALAudio::is_long_wav_playing(int id)
{
	return (this->streams.find(id) != this->streams.end());
}

// Play digitized wav from the wav resource file
//
// file_name     - name of the wav in the resource file
// repeat_offset - offset of wave data to play on repeat
//                 i.e. 0 to start of wave data
//
// return: channel number
//         0 - not played
//
int OpenALAudio::play_loop_wav(const char *file_name, int repeat_offset,
                               const DsVolume &vol)
{
	int id;
	StreamContext *sc;

	if (!this->wav_init_flag || !this->wav_flag)
		return 0;

	MSG("play_loop_wav(\"%s\", %i, (%li, %li)\n", file_name, repeat_offset,
	    vol.ds_vol, vol.ds_pan);

	id = this->play_long_wav(file_name, vol);
	if (id == 0)
		return 0;

	sc = this->streams[id];
	sc->loop_start_frame = repeat_offset / sc->stream->frame_size();
	sc->looping = true;

	return id;
}

void OpenALAudio::volume_loop_wav(int id, const DsVolume &vol)
{
	this->volume_long_wav(id, vol);
}

void OpenALAudio::fade_out_loop_wav(int id, int fade_duration_msec)
{
	StreamMap::const_iterator itr;
	StreamContext *sc;

	if (!this->wav_init_flag)
		return;

	MSG("fade_out_loop_wav(%i, %i)\n", id, fade_duration_msec);

	itr = this->streams.find(id);
	if (itr == this->streams.end())
		return;

	sc = itr->second;
	sc->fade_frames_played = 0;
	sc->fade_frames = sc->stream->frame_rate() * fade_duration_msec
	                  / 1000;
}

DsVolume OpenALAudio::get_loop_wav_volume(int id)
{
	StreamContext *sc;
	StreamMap::const_iterator itr;
	float position[3];
	float gain;

	if (!this->wav_init_flag)
		return DsVolume(0, 0);

	itr = this->streams.find(id);
	if (itr == this->streams.end())
		return DsVolume(0, 0);

	sc = itr->second;

	alGetSourcef(sc->source, AL_GAIN, &gain);
	alGetSourcefv(sc->source, AL_POSITION, position);

	if (sc->fade_frames != 0)
		gain *= 1.f - float(sc->fade_frames_played)
		              / sc->fade_frames;

	return DsVolume(ratio_to_millibels(gain),
	                (position[0] / PANNING_MAX_X) * 10000.f + .5f);
}

int OpenALAudio::is_loop_wav_fading(int id)
{
	StreamMap::const_iterator itr;

	if (!this->wav_init_flag)
		return false;

	itr = this->streams.find(id);
	if (itr == this->streams.end())
		return false;

	return (itr->second->fade_frames != 0);
}

void OpenALAudio::yield()
{
	/* FIXME: This object causes a frame flip.  Looks like a hack that could
	 * use some fixing.
	 */
	VgaFrontLock vgaLock;

	StreamMap::iterator si;

	for (si = this->streams.begin(); si != this->streams.end();)
	{
		ALint state;
		StreamContext *sc;

		sc = si->second;

		if (sc->stream_data())
		{
			++si;
			continue;
		}

		alGetSourcei(sc->source, AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED)
		{
			delete sc;
			this->streams.erase(si++);
			continue;
		}

		++si;
	}
}

void OpenALAudio::stop_wav()
{
	StreamMap::const_iterator itr;

	for (itr = this->streams.begin(); itr != this->streams.end(); ++itr)
		delete itr->second;

	this->streams.clear();
}

void OpenALAudio::stop_loop_wav(int id)
{
	if (!this->wav_init_flag)
		return;

	this->stop_long_wav(id);
}

// <int> trackId - the id. of the CD track to play.
//
int OpenALAudio::play_cd(int trackId, int volume)
{
	WARN_UNIMPLEMENTED("play_cd");
	return 0;
}

void OpenALAudio::stop_cd()
{
	WARN_UNIMPLEMENTED("stop_cd");
}

int OpenALAudio::is_mid_playing()
{
	WARN_UNIMPLEMENTED("is_mid_playing");
	return 0;
}

int OpenALAudio::is_wav_playing()
{
	if (!this->wav_init_flag)
		return false;

	return (!this->streams.empty());
}

int OpenALAudio::is_cd_playing()
{
	WARN_UNIMPLEMENTED("is_cd_playing");
	return 0;
}

void OpenALAudio::toggle_mid(int midFlag)
{
	WARN_UNIMPLEMENTED("toggle_mid");
}

void OpenALAudio::toggle_wav(int wav_flag)
{
	if (!wav_flag)
		this->stop_wav();

	this->wav_flag = wav_flag;
}

void OpenALAudio::toggle_cd(int cdFlag)
{
	WARN_UNIMPLEMENTED("toggle_cd");
}

// Set mid volume
//
// <int> midVolume = mid volume, 0-100
//
void OpenALAudio::set_mid_volume(int midVolume)
{
	WARN_UNIMPLEMENTED("set_mid_volume");
}

// Set wav volume
//
// <int> vol = wav volume, 0-100
//
void OpenALAudio::set_wav_volume(int vol)
{
	StreamMap::const_iterator itr;
	float gain;
	float gain_mult;
	int diff;

	if (!this->wav_init_flag)
		return;

	MSG("set_wav_volume(%i)\n", vol);

	vol = MAX(vol, 0);
	vol = MIN(vol, 100);
	vol = vol * 100 - 10000;
	diff = vol - this->wav_volume;
	gain_mult = millibels_to_ratio(diff);

	for (itr = this->streams.begin(); itr != this->streams.end(); ++itr)
	{
		alGetSourcef(itr->second->source, AL_GAIN, &gain);
		alSourcef(itr->second->source, AL_GAIN, gain * gain_mult);
		check_al();
	}

	this->wav_volume = vol;
}

int OpenALAudio::get_wav_volume() const
{
	if (!this->wav_init_flag)
		return 0;

	return ((this->wav_volume + 10000) / 100);
}

// Set cd volume
//
// <int> cdVolume = cd volume, 0-100
//
void OpenALAudio::set_cd_volume(int cdVolume)
{
	WARN_UNIMPLEMENTED("set_cd_volume");
}

/* Changes the volume/panning of a wave.  Does not attenuate by wav_volume. */
void OpenALAudio::volume_long_wav(int id, const DsVolume &vol)
{
	StreamContext *sc;
	StreamMap::const_iterator itr;

	if (!this->wav_init_flag)
		return;

	MSG("volume_long_wav(%i, (%li, %li))\n", id, vol.ds_vol, vol.ds_pan);

	itr = this->streams.find(id);
	if (itr == this->streams.end())
		return;

	sc = itr->second;
	set_source_volume(sc->source, vol.ds_vol);
	set_source_panning(sc->source, vol.ds_pan);
	check_al();
}


OpenALAudio::StreamContext::StreamContext()
{
	this->stream = NULL;
	this->source = 0;
	this->fade_frames_played = 0;
	this->fade_frames = 0;
	this->looping = false;
	this->loop_start_frame = 0;
	this->streaming = true;
}

OpenALAudio::StreamContext::~StreamContext()
{
	if (this->source != 0)
	{
		this->stop();
		alDeleteSources(1, &this->source);
	}
}

bool OpenALAudio::StreamContext::init(AudioStream *as)
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

void OpenALAudio::StreamContext::apply_fading(void *buffer, size_t frames)
{
	size_t n;
	int ch;
	uint8_t *u8buf;
	int16_t *s16buf;
	float incr;
	float f;

	assert(this->fade_frames > 0);

	if (this->fade_frames_played >= this->fade_frames)
		return;

	ch = this->stream->channels();
	incr = -1.f / this->fade_frames;
	f = 1.f - float(this->fade_frames_played) / this->fade_frames;

	switch (this->stream->sample_size())
	{
		case 1:
			u8buf = static_cast<uint8_t *>(buffer);

			for (n = 0; n < frames; n++)
			{
				for (int c = 0; c < ch; c++)
				{
					uint8_t v = u8buf[n * ch + c];

					u8buf[n * ch + c] =
						f * (v - 128) + 128 + .5f;
				}

				f += incr;
				f = MAX(f, 0.f);
			}
			break;

		case 2:
			s16buf = static_cast<int16_t *>(buffer);

			for (n = 0; n < frames; n++)
			{
				for (int c = 0; c < ch; c++)
				{
					s16buf[n * ch + c] =
						f * s16buf[n * ch + c] + .5f;
				}

				f += incr;
				f = MAX(f, 0.f);
			}
			break;

		default:
			abort();
			break;
	}

	this->fade_frames_played += frames;
	this->fade_frames_played = MIN(this->fade_frames_played,
	                               this->fade_frames);
}

bool OpenALAudio::StreamContext::stream_data(int new_buffer_count)
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

		if (frames_read == 0)
		{
			if (this->looping)
			{
				this->stream->seek(this->loop_start_frame);
				frames_read = this->stream->read(data,
				                                 space_frames);
				if (frames_read == 0)
				{
					this->streaming = false;
					break;
				}
			}
			else
			{
				this->streaming = false;
				break;
			}
		}

		if (this->fade_frames != 0)
			this->apply_fading(data, frames_read);

		alBufferData(buf, format, data,
		             frames_read * this->stream->frame_size(),
		             this->stream->frame_rate());
		if (!check_al())
			goto err;

		alSourceQueueBuffers(this->source, 1, &buf);
		if (!check_al())
			goto err;
	}

	if (!this->streaming)
	{
		alDeleteBuffers(1, &buf);
		check_al();
	}

	alGetSourcei(this->source, AL_SOURCE_STATE, &state);

	if (state != AL_PLAYING)
	{
		alSourcePlay(this->source);
		check_al();
	}

	return this->streaming;

err:
	if (buf != 0)
		alDeleteBuffers(1, &buf);

	this->streaming = false;
	return false;
}

void OpenALAudio::StreamContext::stop()
{
	ALint count;
	ALuint buf;

	assert(this->source != 0);

	alSourceStop(this->source);
	alGetSourcei(this->source, AL_BUFFERS_PROCESSED, &count);

	while (count-- > 0)
	{
		alSourceUnqueueBuffers(this->source, 1, &buf);
		alDeleteBuffers(1, &buf);
	}
}
