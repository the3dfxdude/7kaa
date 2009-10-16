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

// Filename    : OBLOB.H
// Description : header file for fixed size binary block
// Owner       : Gilbert

#ifndef __OBLOB_H
#define __OBLOB_H

class Blob
{
public:
	char *ptr;				// more exactly char const *ptr
	int	size;				// const int

public:
	Blob();
	Blob(int);
	Blob( Blob &);
	~Blob();

	void	resize(int);
	Blob& operator=(Blob &);

	char *p()			{ return ptr; }
};


#endif
