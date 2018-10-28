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

// Filename    : OU_CART.H
// Description : Header file for Explosive Cart

#ifndef __OU_CART_H
#define __OU_CART_H

#include <OUNIT.h>

struct UnitExpCartCrc;

#pragma pack(1)
class UnitExpCart : public Unit
{
public:
	char	triggered;

public:
	UnitExpCart();
	~UnitExpCart();

	int	process_die();
	void	trigger_explode();

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(UnitExpCartCrc *c);
};
#pragma pack()

#endif
