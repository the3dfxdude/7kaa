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

// Filename    : OEFFECT.H
// Description : header file of effect array

#ifndef __OEFFECT_H
#define __OEFFECT_H

#include <OSPRITE.h>

class Effect : public Sprite
{
public:
	char	layer;
	int	life;

public:
	Effect();
	~Effect();

	void	init(short spriteId, short startX, short startY, char initAction, char initDir, char dispLayer, int effectLife);
	void	pre_process();
	void	process_idle();
	int	process_die();

	static void create(short spriteId, short startX, short startY, char initAction, char initDir, char dispLayer, int effectLife);
};

// to add an effect to effect_array,
// Effect *effectPtr = new Effect;
// effectPtr->init(...)
// effect_array.add(effectPtr);
// or call Effect::create();

extern SpriteArray effect_array;

#endif