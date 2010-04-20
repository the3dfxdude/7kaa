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
#ifndef MEM_INPUT_STREAM_H
#define MEM_INPUT_STREAM_H

#include <input_stream.h>

class MemInputStream: public InputStream
{
private:
	uint8_t *data;
	size_t length;
	size_t pos;
	bool own_data;

public:
	MemInputStream();
	~MemInputStream();
	void open(uint8_t *data, size_t length, bool own_data = true);
	long read(void *buffer, long length);
	bool seek(long offset, int whence);
	long tell();
	void close();
};

#endif
