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

// Filename    : OB_FLAME.CPP
// Description : non-moving bullet attack (flamethrower)

#include <OB_FLAME.h>


BulletFlame::BulletFlame() : Bullet()
{
}


//### begin alex 3/5 ###//
void BulletFlame::init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType)
{
	// note : BulletFlame should have at least one dummy moving frame for each direction
	Bullet::init(parentType, parentRecno, targetXLoc, targetYLoc, targetMobileType);
//#### end alex 3/5 ####//

	cur_action = SPRITE_IDLE;
	
	// sound effect here
}


void BulletFlame::process_idle()
{

	// Sprite::process_idle();
	if( ++cur_frame <= cur_sprite_stop()->frame_count )
	{
		// ----- warn/ attack target every frame -------//
		warn_target();
		check_hit();
	}
	else
	{
		cur_action = SPRITE_DIE;
		cur_frame = 1;
	}
}


//--------- Begin of function BulletFlame::display_layer -------//
char BulletFlame::display_layer()
{
	switch(mobile_type)
	{
	case UNIT_LAND:
	case UNIT_SEA:
		return 2;
	case UNIT_AIR:
		return 8;
	default:
		err_here();
		return 1;
	}
}
//--------- End of function BulletFlame::display_layer -------//
