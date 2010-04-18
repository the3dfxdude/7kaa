/*
 * Seven Kingdoms: Ancient Adversaries
 * AudioStream
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

#include <stddef.h>
#include <stdint.h>

#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

class AudioStream
{
public:
	virtual ~AudioStream() {};
	virtual long read(void *buffer, size_t frame_count) = 0;
	virtual bool seek(size_t frame_no) = 0;
	virtual int32_t frame_rate() const = 0; /* in PCM frames per sec */
	virtual int channels() const = 0;
	virtual int sample_size() const = 0;    /* in bytes */

	int frame_size() const
	{
		return (this->channels() * this->sample_size());
	}
};

#endif
