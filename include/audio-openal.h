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

//Filename    : OAUDIO.H
//Description : Object Midi Audio and Digitized Sound

#ifndef AUDIO_OPENAL_H
#define AUDIO_OPENAL_H

#include <map>

#include <AL/al.h>
#include <AL/alc.h>

#include <ORESX.h>
#include <OVOLUME.h>
#include <audio_stream.h>

class Audio
{
private:
	class StreamContext
	{
	public:
		AudioStream *stream;
		ALuint source;

		/* frames played since fade started */
		size_t fade_frames_played;

		/* 
		 * Number of frames over which volume fades to zero.  A value of
		 * 0 means not fading.
		 */
		size_t fade_frames;

		bool looping;

		/* where to restart playing after reaching the end */
		size_t loop_start_frame;

		bool streaming;

	public:
		StreamContext();
		~StreamContext();
		bool init(AudioStream *as);
		bool stream_data(int new_buffer_count = 0);
		void stop();

	private:
		/* forbid copying */
		StreamContext(const StreamContext &) {}
	};

	typedef std::map<int, StreamContext *> StreamMap;

public:
	char  init_flag;

	char  mid_init_flag;   // whether the midi driver has been installed
	char  wav_init_flag;   // whether the wave driver has been installed
	char  cd_init_flag;

	char  mid_flag;        // flag determing whether MIDI music should be playing
	char  wav_flag;		  // flag determing whether WAV sound effects should be playing
	char  cd_flag;			  // flag determing whether Audio CD track should be playing

	char* mid_buf;
	char* wav_buf;

	int   mid_buf_size;
	int   wav_buf_size;

	ResourceIdx mid_res;
	ResourceIdx wav_res;

public:
	Audio();
	~Audio();

	int	init();
	void	deinit();

	void	yield();		// called by sys every some time

	int	play_mid(char*);

	// functions on short wave
	int	play_wav(char*, DsVolume);
	int	play_wav(short resIdx, DsVolume);
	int	play_resided_wav(char *, DsVolume);
	int	get_free_wav_ch();
	int	stop_wav(int);
	int	is_wav_playing(int);

	// functions on long wave
	int	play_long_wav(const char*, DsVolume);
	int	stop_long_wav(int);
	int	is_long_wav_playing(int);
	void	volume_long_wav(int ch, DsVolume);

	// functions on loop wave
	int	play_loop_wav(const char *, int repeatOffset, DsVolume);	// return channel no.
	void	stop_loop_wav(int ch);
	void	volume_loop_wav(int ch, DsVolume);
	void	fade_out_loop_wav(int ch, int fadeRate);
	DsVolume get_loop_wav_volume(int ch);
	int	is_loop_wav_fading(int ch);

	int	play_cd(int, int retVolume);

	void	stop_mid();
	void	stop_wav();             // and stop also long wav
	void	stop_cd();
	void	stop_long_wav();

	int	is_mid_playing();
	int	is_wav_playing();
	int	is_cd_playing();

	void	toggle_mid(int);
	void	toggle_wav(int);
	void	toggle_cd(int);

	void	set_mid_volume(int);
	void	set_wav_volume(int);    // 0 to 100
	void	set_cd_volume(int);

	int	get_wav_volume() const; // 0 to 100

private:
	ALCdevice  *al_device;
	ALCcontext *al_context;

	StreamMap streams;

private:
	int	init_mid();
	int	init_wav();
	int	init_cd();

	void	deinit_mid();
	void	deinit_wav();
	void	deinit_cd();
};

extern Audio audio;

#endif
