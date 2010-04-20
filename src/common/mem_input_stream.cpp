/*
 * Seven Kingdoms: Ancient Adversaries
 * MemInputStream 
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
#include <string.h>

#include <ALL.h>
#include <mem_input_stream.h>

MemInputStream::MemInputStream()
{
	this->data = NULL;
}

MemInputStream::~MemInputStream()
{
	this->close();
}

void MemInputStream::open(void *data, size_t length, bool own_data)
{
	this->close();
	this->data = static_cast<uint8_t *>(data);
	this->length = length;
	this->own_data = own_data;
}

long MemInputStream::read(void *buffer, long length)
{
	long read_count;

	if (this->data == NULL)
		return 0;

	read_count = MIN(length, this->length - this->pos);
	memcpy(buffer, this->data + this->pos, read_count);
	this->pos += read_count;

	return read_count;
}

bool MemInputStream::seek(long offset, int whence)
{
	long target;

	switch (whence)
	{
		case SEEK_SET: target = offset; break;
		case SEEK_CUR: target = this->pos + offset; break;
		case SEEK_END: target = this->length + offset; break;
		default: return false;
	}

	if (target < 0 || target >= this->length)
		return false;

	this->pos = target;
	return true;
}

long MemInputStream::tell()
{
	return this->pos;
}

void MemInputStream::close()
{
	if (this->data == NULL)
		return;

	if (this->own_data)
		delete[] this->data;

	this->data = NULL;
}
