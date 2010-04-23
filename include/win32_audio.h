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
//Description : Object Midi Win32Audio and Digitized Sound
//Ownership   : Gilbert


#ifndef WIN32_AUDIO_H
#define WIN32_AUDIO_H

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include <audio_base.h>

//------ Define constant -----------//

#define DEFAULT_MID_BUF_SIZE   0x8000
#define DEFAULT_WAV_BUF_SIZE   0x8000

#define MAX_AUDIO_VOLUME   		 100
#define MAX_WAV_CHANNEL        8
#define MAX_LONG_WAV_CH        4

#define MAX_LOOP_WAV_CH        4

//--------------- Define class Win32Audio ---------------//

class Win32Audio: public AudioBase
{
public:
   Win32Audio();
   ~Win32Audio();

   int  	init();
   void 	deinit();

   void  yield();		// called by sys every some time

   int  	play_mid(char*);

   // functions on short wave
   int  	play_wav(char*, const DsVolume &);
   int  	play_wav(short resIdx, const DsVolume &);
   int	play_resided_wav(char *, const DsVolume &);
   int	get_free_wav_ch();
   int	stop_wav(int);
   int	is_wav_playing(int);

   // functions on long wave
   int   play_long_wav(const char*, const DsVolume &);
   int	stop_long_wav(int);
   int	is_long_wav_playing(int);
   void	volume_long_wav(int ch, const DsVolume &);

   // functions on loop wave
   int	play_loop_wav(const char *, int repeatOffset, const DsVolume &);	// return channel no.
   void	stop_loop_wav(int ch);
   void	volume_loop_wav(int ch, const DsVolume &);
   void	fade_out_loop_wav(int ch, int fadeRate);
   DsVolume get_loop_wav_volume(int ch);
   int	is_loop_wav_fading(int ch);

   int   play_cd(int, int retVolume);

   void 	stop_mid();
   void 	stop_wav();		// and stop also long wav
   void	stop_cd();
   void	stop_long_wav();

   int  	is_mid_playing();
   int  	is_wav_playing();
   int	is_cd_playing();

   void 	toggle_mid(int);
   void 	toggle_wav(int);
   void 	toggle_cd(int);

   void 	set_mid_volume(int);
   void 	set_wav_volume(int);    // 0 to 100
   void 	set_cd_volume(int);

   int	get_wav_volume() const; // 0 to 100

private:
   int  	init_mid();
   int  	init_wav();
   int	init_cd();

   void	deinit_mid();
   void	deinit_wav();
   void	deinit_cd();

   LPDIRECTSOUND 		  lp_direct_sound;		// DirectSound object
   LPDIRECTSOUNDBUFFER lp_wav_ch_dsb[MAX_WAV_CHANNEL];
   // DirectSoundBuffer of each channel
   int	wav_serial_no[MAX_WAV_CHANNEL];
   int	max_wav_serial_no;
   LPDIRECTSOUNDBUFFER lp_lwav_ch_dsb[MAX_LONG_WAV_CH];
   int	lwav_serial_no[MAX_LONG_WAV_CH];
   int	max_lwav_serial_no;
   // DirectSoundBuffer of each long wave
   File* lwav_fileptr[MAX_LONG_WAV_CH];
   // file point of each long wave
   short	lwav_bank[MAX_LONG_WAV_CH];	// which bank to be filled next
   short	lwav_bufsiz[MAX_LONG_WAV_CH];
   // buffer size of each channel = lwav_bufsiz[c]*LWAV_BANKS
   long	wav_volume;						// -10000 to 0
   char	run_yield;						// 0 = skip Win32Audio::yield()

   LPDIRECTSOUNDBUFFER lp_loop_ch_dsb[MAX_LOOP_WAV_CH];
   File* loopwav_fileptr[MAX_LOOP_WAV_CH];
   int	repeat_offset[MAX_LOOP_WAV_CH];
   short	loopwav_bank[MAX_LOOP_WAV_CH];
   DWORD	loopwav_fade_time[MAX_LOOP_WAV_CH];
   int	loopwav_fade_rate[MAX_LOOP_WAV_CH];

   char* mid_buf;
   char* wav_buf;

   int	mid_buf_size;
   int	wav_buf_size;

   ResourceIdx mid_res;

   int	assign_serial( int &);
};

//--------------------------------------------------//

#endif
