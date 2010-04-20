/*
 * Seven Kingdoms: Ancient Adversaries
 * InputStream 
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
#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include <stdio.h>

class InputStream
{
public:
	virtual ~InputStream() {}
	virtual long read(void *buffer, long length) = 0;
	virtual bool seek(long offset, int whence) = 0;
	virtual void close() = 0;
};

#endif
