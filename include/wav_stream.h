/*
 * Seven Kingdoms: Ancient Adversaries
 * WavStream
 *
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
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

#ifndef WAV_STREAM_H
#define WAV_STREAM_H

#include <audio_stream.h>
#include <OFILE.h>

class WavStream: public AudioStream
{
private:
	File *file;
	int32_t fram_rate;    /* in PCM frames per second */
	uint32_t data_length; /* in PCM frames */
	uint32_t data_left;   /* in PCM frames */
	long data_offset;     /* in bytes from beginning of file */
	uint8_t bytes;        /* bytes per sample */
	int chans;            /* number of channels */
	bool good;

private:
	void clear();
	bool advance_to_chunk(const char *name, uint32_t *sizep);

public:
	WavStream();
	~WavStream();
	bool open(File *file);
	void close();
	ssize_t read(void *buffer, size_t frame_count);
	bool seek(size_t frame_no);
	int32_t frame_rate() const;
	int channels() const;
	int sample_size() const;
};

#endif
