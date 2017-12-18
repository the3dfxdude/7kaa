#pragma once

#include <audio_base.h>

class AudioNone : public AudioBase {
public:
	virtual int init () override;
	virtual void deinit () override;
	virtual void yield () override;
	virtual int play_wav (char *, const DsVolume &) override;
	virtual int play_wav (short resIdx, const DsVolume &) override;
	virtual int play_resided_wav (char *, const DsVolume &) override;
	virtual int get_free_wav_ch () override;
	virtual int stop_wav (int) override;
	virtual int play_long_wav (const char *, const DsVolume &) override;
	virtual int stop_long_wav (int) override;
	virtual int is_long_wav_playing (int) override;
	virtual void volume_long_wav (int ch, const DsVolume &) override;
	virtual int play_loop_wav (const char *, int repeatOffset, const DsVolume &) override;
	virtual void stop_loop_wav (int ch) override;
	virtual void volume_loop_wav (int ch, const DsVolume &) override;
	virtual void fade_out_loop_wav (int ch, int fadeRate) override;
	virtual DsVolume get_loop_wav_volume (int ch) override;
	virtual int is_loop_wav_fading (int ch) override;
	virtual int play_cd (int, int retVolume) override;
	virtual void stop_wav () override;
	virtual void stop_cd () override;
	virtual int is_cd_playing () override;
	virtual void toggle_wav (bool) override;
	virtual void toggle_cd (bool) override;
	virtual void set_wav_volume (int) override;
	virtual void set_cd_volume (int) override;
	virtual int get_wav_volume () const override;
};

typedef AudioNone Audio;
