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

#ifndef AUDIO_BASE_H
#define AUDIO_BASE_H

#include <ORESX.h>
#include <OVOLUME.h>

class AudioBase
{
public:
   bool init_flag;

   bool mid_init_flag;   // whether the midi driver has been installed
   bool wav_init_flag;   // whether the wave driver has been installed
   bool cd_init_flag;

   // flag determing whether MIDI music should be playing
   bool mid_flag;

   // flag determing whether WAV sound effects should be playing
   bool wav_flag;

   // flag determing whether Audio CD track should be playing
   bool cd_flag;

   ResourceIdx wav_res;

public:
   virtual ~AudioBase() {};

   virtual int	init() = 0;
   virtual void	deinit() = 0;

   virtual void	yield() = 0; // called by sys every some time

   virtual int	play_mid(char *) = 0;

   // functions on short wave
   virtual int	play_wav(char *, const DsVolume &) = 0;
   virtual int	play_wav(short resIdx, const DsVolume &) = 0;
   virtual int	play_resided_wav(char *, const DsVolume &) = 0;
   virtual int	get_free_wav_ch() = 0;
   virtual int	stop_wav(int) = 0;
   virtual int	is_wav_playing(int) = 0;

   // functions on long wave
   virtual int	play_long_wav(const char *, const DsVolume &) = 0;
   virtual int	stop_long_wav(int) = 0;
   virtual int	is_long_wav_playing(int) = 0;
   virtual void	volume_long_wav(int ch, const DsVolume &) = 0;

   // functions on loop wave
   virtual int	play_loop_wav(const char *, int repeatOffset,
				   const DsVolume &) = 0;
   virtual void	stop_loop_wav(int ch) = 0;
   virtual void	volume_loop_wav(int ch, const DsVolume &) = 0;
   virtual void	fade_out_loop_wav(int ch, int fadeRate) = 0;
   virtual DsVolume get_loop_wav_volume(int ch) = 0;
   virtual int	is_loop_wav_fading(int ch) = 0;

   virtual int	play_cd(int, int retVolume) = 0;

   virtual void	stop_mid() = 0;
   virtual void	stop_wav() = 0;             // and stop also long wav
   virtual void	stop_cd() = 0;

   virtual int	is_mid_playing() = 0;
   virtual int	is_wav_playing() = 0;
   virtual int	is_cd_playing() = 0;

   virtual void	toggle_mid(int) = 0;
   virtual void	toggle_wav(int) = 0;
   virtual void	toggle_cd(int) = 0;

   virtual void	set_mid_volume(int) = 0;
   virtual void	set_wav_volume(int) = 0;    // 0 to 100
   virtual void	set_cd_volume(int) = 0;

   virtual int	get_wav_volume() const = 0; // 0 to 100
};

#endif
