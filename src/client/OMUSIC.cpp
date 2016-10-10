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

// Filename    : OMUSIC.CPP
// Description : music

#include <GAMEDEF.h>
#include <OAUDIO.h>
#include <OMUSIC.h>
#include <OSYS.h>
#include <OCONFIG.h>

// -------- define constant --------//
// random select 2 - 8 for background music
#define LOW_RANDOM_SONG 2
#define HIGH_RANDOM_SONG 8

// -------- define song name --------//

#ifdef DEMO

static const char *music_file[] =
{
	"DEMO.WAV",		// opening
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
	"DEMO.WAV",
};

#else

static const char *music_file[] =
{
	"WAR.WAV",		// opening
	"NORMAN.WAV",
	"MAYA.WAV",
	"GREEK.WAV",
	"VIKING.WAV",
	"PERSIAN.WAV",
	"CHINESE.WAV",
	"JAPANESE.WAV",
	"WIN.WAV",
	"LOSE.WAV",
};

#endif

// -------- begin of function Music::Music ---------//
Music::Music()
{
	init_flag = 0;
	song_id = 0;
	music_channel = 0;
}
// -------- end of function Music::Music ---------//


// -------- begin of function Music::~Music ---------//
Music::~Music()
{
	if (init_flag != 0)
		deinit();
}
// -------- end of function Music::~Music ---------//


// -------- begin of function Music::init ---------//
// note : call Music::init after audio.init
void Music::init()
{
	deinit();
	init_flag = audio.init_flag;
}
// -------- end of function Music::init ---------//


// -------- begin of function Music::deinit ---------//
// call deinit before audio.deinit
void Music::deinit()
{
	if( init_flag )
	{
		stop();
	}
}
// -------- end of function Music::deinit ---------//


// -------- begin of function Music::stop ---------//
int Music::stop()
{
	if( init_flag )
	{
		if(music_channel)
		{
			if( play_type & MUSIC_PLAY_CD )
			{
				audio.stop_cd();
			}
			else
			{
				if( play_type & MUSIC_PLAY_LOOPED )
					audio.stop_loop_wav(music_channel);
				else
					audio.stop_long_wav(music_channel);
			}
			music_channel = 0;
			song_id = 0;
		}
		return 1;
	}
	else
	{
		return -1;
	}
}
// -------- end of function Music::stop ---------//


// -------- begin of function Music::play ---------//
// <int> songId
// <int> playType   0 = non-looped, 1 = looped
int Music::play(int songId, int playType)
{
	if( !init_flag )
		return 0;

	stop();

#ifdef BUNDLE
	// disable CD music
	playType &= ~MUSIC_CD_THEN_WAV & ~MUSIC_PLAY_CD;
#endif

	if( playType & MUSIC_CD_THEN_WAV )
	{
		return play(songId, playType & ~MUSIC_CD_THEN_WAV | MUSIC_PLAY_CD) 
			|| play(songId, playType & ~MUSIC_CD_THEN_WAV & ~MUSIC_PLAY_CD);
	}
	else if( playType & MUSIC_PLAY_CD )
	{
		if( audio.cd_init_flag && audio.play_cd(songId +1, config.cd_music_volume) ) // skip the first data track
		{
			play_type = playType;
			song_id = songId;
			music_channel = 1;
			return 1;
		}
		return 0;
	}
	else 
	{
		if( audio.wav_init_flag )
		{
			String waveFileStr(DIR_MUSIC);
			waveFileStr += music_file[songId-1];
			if( !DIR_MUSIC[0] || !misc.is_file_exist(waveFileStr) || !audio.wav_init_flag )
				return 0;
			if( playType & MUSIC_PLAY_LOOPED )
			{
				AbsVolume absv(config.wav_music_volume,0);
				music_channel = audio.play_loop_wav(waveFileStr, 0, absv );
			}
			else
			{
				AbsVolume absv(config.wav_music_volume,0);
				music_channel = audio.play_long_wav(waveFileStr, absv );
			}
			play_type = playType;
			song_id = songId;
			return music_channel != 0;
		}
		return 0;
	}
}
// -------- end of function Music::play ---------//


// -------- begin of function Music::is_playing ---------//
// [int] songId        id of the song (default=0, don't care)
int Music::is_playing(int songId)
{
	if( !init_flag )
		return 0;

	if( !music_channel )
		return 0;

	if( play_type & MUSIC_PLAY_CD )
	{
		return audio.is_cd_playing() && (!songId || songId == song_id);
	}
	else
	{
		if( play_type & MUSIC_PLAY_LOOPED )
		{
			return (!songId || songId == song_id);		// loop wav always playing
		}
		else
		{
			return audio.is_long_wav_playing(music_channel) && (!songId || songId == song_id);
		}
	}

	return 0;
}
// -------- end of function Music::is_playing ---------//


// -------- begin of function Music::max_song --------//
// return no. of songs
int Music::max_song()
{
	return sizeof(music_file) / sizeof(char *);
}
// -------- end of function Music::max_song --------//


// -------- begin of function Music::random_bgm_track --------//
int Music::random_bgm_track(int excludeSong)
{
	int s = LOW_RANDOM_SONG + misc.get_time() % (HIGH_RANDOM_SONG - LOW_RANDOM_SONG + 1);
	if( s == excludeSong )
	{
		// avoid selecting excludeSong if possible
		if( ++s > HIGH_RANDOM_SONG )
			s = LOW_RANDOM_SONG;
	}
	err_when(s < 1 || s > max_song() );
	return s;
}
// -------- end of function Music::random_bgm_track --------//


// -------- begin of function Music::change_volume --------//
void Music::change_volume(int vol)
{
	if( !init_flag )
		return;

	if( is_playing() )
	{
		if( play_type & MUSIC_PLAY_CD )
		{
			audio.set_cd_volume(vol);
		}
		else
		{
			AbsVolume absv(vol,0);
			audio.volume_long_wav(music_channel, DsVolume(absv));
		}
	}
}
//-------- end of function Music::change_volume --------//


//-------- begin of function Music::yield --------//

void Music::yield()
{
	if( !init_flag )
		return;

	int oldSongId = song_id;

	if( config.music_flag )
	{
		if( !is_playing() )
			play(random_bgm_track(oldSongId), sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
	}
	else
	{
		stop();			// stop any music playing
	}
}
//-------- end of function Music::yield --------//
