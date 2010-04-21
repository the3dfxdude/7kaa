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

// Filename    : OVOLUME.H
// Description : audio volume unit

#ifndef __OVOLUME_H
#define __OVOLUME_H

class DsVolume;
class AbsVolume;
class RelVolume;
class PosVolume;

class DsVolume
{
public:
	long	ds_vol;			// -10,000 to 0 [cB]
	long	ds_pan;			// -10,000 to 10,000

public:
	DsVolume(long dsVol, long dsPan);
	DsVolume(AbsVolume &);
	DsVolume(RelVolume &);
};

class AbsVolume
{
public:
	long	abs_vol;
	long	ds_pan;

public:
	AbsVolume(long absVol, long dsPan);
	AbsVolume(DsVolume &);
};

class RelVolume
{
public:
	long	rel_vol;			// 0 to 100
	long	ds_pan;			// -10,000 to 10,000

public:
	RelVolume()	{}
	RelVolume(long relVol, long dsPan);
	RelVolume(PosVolume &);
	RelVolume(PosVolume &, int drop, int limit);
};

class PosVolume
{
public:
	long	x;
	long	y;

public:
	PosVolume(long relLocX, long relLocY);
};

extern RelVolume DEF_REL_VOLUME;

#endif
