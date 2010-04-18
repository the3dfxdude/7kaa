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

#include <assert.h>
#include <limits.h>
#include <string.h>

#include <ALL.h>
#include <dbglog.h>
#include <file_util.h>
#include <wav_stream.h>

DBGLOG_DEFAULT_CHANNEL(Audio);

struct FormatHeader
{
	enum { SIZE = 16 };

	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t frame_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};

static bool read_format_header(File *file, FormatHeader *hdrp)
{
	bool ok = true;

	ok = ok && read_le<uint16_t>(file, &hdrp->audio_format);
	ok = ok && read_le<uint16_t>(file, &hdrp->num_channels);
	ok = ok && read_le<uint32_t>(file, &hdrp->frame_rate);
	ok = ok && read_le<uint32_t>(file, &hdrp->byte_rate);
	ok = ok && read_le<uint16_t>(file, &hdrp->block_align);
	ok = ok && read_le<uint16_t>(file, &hdrp->bits_per_sample);

	return ok;
}


WavStream::WavStream()
{
	this->clear();
}

WavStream::~WavStream()
{
	this->close();
}

void WavStream::clear()
{
	this->file = NULL;
	this->data_offset = 0;
	this->data_length = 0;
	this->data_left = 0;
	this->fram_rate = 0;
	this->chans = 0;
	this->bytes = 0;
	this->good = true;
}

#include <stdio.h>

bool WavStream::advance_to_chunk(const char *name, uint32_t *sizep)
{
	char buffer[4];
	uint32_t size;
	bool ok = true;

	for (;;)
	{
		ok = ok && file->file_read(buffer, 4);
		ok = ok && read_le<uint32_t>(this->file, &size);

		if (!ok)
			return false;

		if (memcmp(buffer, name, 4) == 0)
		{
			*sizep = size;
			return true;
		}

		ok = ok && ::seek(this->file, size, SEEK_CUR);
	}

	return false;
}

bool WavStream::open(File *file)
{
	char name[4];
	uint32_t size;
	bool ok = true;
	FormatHeader fmth;

	this->close();
	this->file = file;

	ok = ok && ::seek(file, 0, SEEK_SET);
	ok = ok && file->file_read(name, 4);
	ok = ok && (memcmp(name, "RIFF", 4) == 0);
	ok = ok && ::seek(file, 4, SEEK_CUR);
	ok = ok && file->file_read(name, 4);
	ok = ok && (memcmp(name, "WAVE", 4) == 0);

	if (!ok)
	{
		ERR("[WavStream::open] Not a wave file\n");
		goto err;
	}

	ok = ok && this->advance_to_chunk("fmt ", &size);
	ok = ok && (size >= FormatHeader::SIZE);
	ok = ok && read_format_header(this->file, &fmth);
	ok = ok && ::seek(this->file, size - FormatHeader::SIZE, SEEK_CUR);
	if (fmth.audio_format != 1
	    || (fmth.bits_per_sample != 8 && fmth.bits_per_sample != 16)
	    || (fmth.num_channels != 1 && fmth.num_channels != 2))
	{
		ERR("[WavStream::open] Unsupported format\n");
		goto err;
	}

	this->bytes = fmth.bits_per_sample / 8;
	this->chans = fmth.num_channels;
	this->fram_rate = fmth.frame_rate;

	ok = ok && this->advance_to_chunk("data", &size);
	if (!ok)
	{
		ERR("[WavStream::open] Missing data chunk\n");
		goto err;
	}

	this->data_offset = this->file->file_pos();
	this->data_length = size / this->frame_size();
	this->data_left = this->data_length;

	return true;

err:
	this->close();
	return false;
}

bool WavStream::seek(size_t frame_no)
{
	if (!this->good)
		return false;

	if (frame_no >= this->data_length)
		return false;

	if (!::seek(this->file,
	            this->data_offset
	            + sizeof(uint16_t) * frame_no * this->chans,
	            SEEK_SET))
	{
		ERR("[WavStream::seek] Seek failed\n");
		this->good = false;
		return false;
	}

	this->data_left = this->data_length - frame_no;

	return true;
}

void WavStream::close()
{
	delete this->file;
	this->clear();
}

long WavStream::read(void *buffer, size_t frame_count)
{
	size_t read_size;

	if (!this->good)
		return -1;

	read_size = min(frame_count, this->data_left);
	if (read_size == 0)
		return 0;

	if (this->bytes == 2)
	{
		int16_t *buf = static_cast<int16_t *>(buffer);
		int16_t *p;

		for (size_t n = 0; n < read_size; n++)
		{
			for (int c = 0; c < this->chans; c++)
			{
				p = &buf[n * this->chans + c];

				if (!read_le<int16_t>(this->file, p))
				{
					this->good = false;
					return n;
				}
			}
		}
	}
	else if (this->bytes == 1)
	{
		if (!this->file->file_read(buffer, this->chans * read_size))
		{
			this->good = false;
			return 0;
		}
	}
	else
		abort();

	this->data_left -= read_size;

	return read_size;
}

int32_t WavStream::frame_rate() const
{
	return this->fram_rate;
}

int WavStream::channels() const
{
	return this->chans;
}

int WavStream::sample_size() const
{
	return this->bytes;
}
