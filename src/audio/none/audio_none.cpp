#include "audio_none.h"

int AudioNone::init ()
{
	return 1;
}

void AudioNone::deinit ()
{
}

void AudioNone::yield ()
{
}

int AudioNone::play_wav (char *, const DsVolume &)
{
	return 1;
}

int AudioNone::play_wav (short resIdx, const DsVolume &)
{
	return 1;
}

int AudioNone::play_resided_wav (char *, const DsVolume &)
{
	return 1;
}

int AudioNone::get_free_wav_ch ()
{
	return 1000;
}

int AudioNone::stop_wav (int)
{
	return 1;
}

int AudioNone::play_long_wav (const char *, const DsVolume &)
{
	return 1;
}

int AudioNone::stop_long_wav (int)
{
	return 1;
}

int AudioNone::is_long_wav_playing (int)
{
	return 0;
}

void AudioNone::volume_long_wav (int ch, const DsVolume &)
{
}

int AudioNone::play_loop_wav (const char *, int repeatOffset, const DsVolume &)
{
	return 1;
}

void AudioNone::stop_loop_wav (int ch)
{
}

void AudioNone::volume_loop_wav (int ch, const DsVolume &)
{
}

void AudioNone::fade_out_loop_wav (int ch, int fadeRate)
{
}

DsVolume AudioNone::get_loop_wav_volume (int ch)
{
	return DsVolume (0, 0);
}

int AudioNone::is_loop_wav_fading (int ch)
{
	return 0;
}

int AudioNone::play_cd (int, int retVolume)
{
	return 0;
}

void AudioNone::stop_wav ()
{
}

void AudioNone::stop_cd ()
{
}

int AudioNone::is_cd_playing ()
{
	return 0;
}

void AudioNone::toggle_wav (bool)
{
}

void AudioNone::toggle_cd (bool)
{
}

void AudioNone::set_wav_volume (int)
{
}

void AudioNone::set_cd_volume (int)
{
}

int AudioNone::get_wav_volume () const
{
	return 0;
}
