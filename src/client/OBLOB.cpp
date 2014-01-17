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

// Filename    : OBLOB.CPP
// Description : fixed size binary block


#include <OBLOB.h>
#include <ALL.h>


Blob::Blob() : ptr(NULL), size(0)
{
}


Blob::Blob(int s) : ptr(mem_add(s)), size(s)
{
}


Blob::Blob(Blob &b) : ptr(mem_add(b.size)), size(b.size)
{
	memcpy(ptr, b.ptr, b.size);
}

Blob::~Blob()
{
	if (ptr != NULL)
		mem_del(ptr);
}

void Blob::resize(int s)
{
	ptr = (char *)mem_resize(ptr, s);
	size = s;
}


Blob& Blob::operator= (Blob& b)
{
	ptr = mem_resize(ptr, b.size);
	size = b.size;
	memcpy( ptr, b.ptr, b.size);
	return *this;
}
