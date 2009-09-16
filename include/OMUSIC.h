// Filename    : OMUSIC.H
// Description : header of music


#ifndef __OMUSIC_H
#define __OMUSIC_H

#define MUSIC_PLAY_LOOPED 1
#define MUSIC_PLAY_CD 2
#define MUSIC_CD_THEN_WAV 4

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