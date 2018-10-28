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

//Filename    : OU_VEHI.H
//Description : Header file of Unit Vehicle

#ifndef __OU_VEHI_H
#define __OU_VEHI_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

struct UnitVehicleCrc;

//----------- Define class UnitVehicle -----------//

#pragma pack(1)
class UnitVehicle : public Unit
{
public:
	short solider_hit_points;		// the original hit points of the solider before it gets on the vehicle
	short vehicle_hit_points;		// the original hit points of the vehicle before the soliders gets on it

public:
	void	set_combat_level(int);
	void	dismount();

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(UnitVehicleCrc *c);
};
#pragma pack()

//--------------------------------------------//

#endif
