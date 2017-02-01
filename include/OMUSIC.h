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

// Filename    : OMUSIC.H
// Description : header of music


#ifndef __OMUSIC_H
#define __OMUSIC_H

#define MUSIC_PLAY_LOOPED 1
#define MUSIC_PLAY_CD 2
#define MUSIC_CD_THEN_WAV 4
#define MAX_RACE_TABLE 7

class Music
{
public:
	int	init_flag;
	int	song_id;
	int	music_channel;
	int	play_type;	// 0 = non-looped, 1 = looped, bit 1 = play from CD (looped not supported)

public:
	Music();
	~Music();
	void	init();
	void	deinit();

	int	stop();
	int	play(int songId, int playType=1);
	int	is_playing(int songId=0);
	void	change_volume(int volume);		// 0-100
	void  yield();

	static int max_song();
	static int random_bgm_track(int excludeId = 0);
};

extern Music music;

#endif