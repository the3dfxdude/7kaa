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

// Filename    : OEFFECT.CPP
// Description : effect array

// note : Effect and effect_array are not saved

#include <OEFFECT.h>

// ------- Begin of function Effect::Effect -------//
Effect::Effect() : Sprite()
{
	layer = 1;	// default in land display layer
	life = 0;	// disappear when life < 0
}
// ------- End of function Effect::Effect -------//


// ------- Begin of function Effect::~Effect -------//
Effect::~Effect()
{
}
// ------- End of function Effect::~Effect -------//


// ------- Begin of function Effect::init -------//
//
// for directed effect, put initAction as SPRITE_IDLE and set appropriate initDir
// undirected effect, put initAction as SPRITE_DIE and set initDir to 0
// to find the life of sprite automatically, set effectLife to 0 or negative
//
void Effect::init(short spriteId, short startX, short startY, char initAction, char initDir, char dispLayer, int effectLife)
{
	sprite_id = spriteId;

	cur_x = startX;
	cur_y = startY;

	go_x = next_x = cur_x;
	go_y = next_y = cur_y;

	cur_attack = 0;

	err_when( initAction != SPRITE_IDLE && initAction != SPRITE_DIE);
	cur_action = initAction;
	cur_dir 	  = initDir;
	cur_frame  = 1;

	//----- clone vars from sprite_res for fast access -----//

	sprite_info = sprite_res[sprite_id];

	sprite_info->load_bitmap_res();

	// -------- adjust cur_dir -----------//
	if( sprite_info->turn_resolution <= 1)
		cur_dir = 0;
	final_dir  = cur_dir;

	//------------- init other vars --------------//

	remain_attack_delay = 0;
	remain_frames_per_step = sprite_info->frames_per_step;

	layer = dispLayer;
	if( effectLife > 0)
		life = effectLife;
	else
	{
		switch( cur_action )
		{
		case SPRITE_IDLE:
			life = sprite_info->stop_array[cur_dir].frame_count - cur_frame;
			break;
		case SPRITE_DIE:
			life = sprite_info->die.frame_count - cur_frame;
			break;
		default:
			err_here();
		}
	}
}
// ------- End of function Effect::init -------//


// ------- Begin of function Effect::pre_process -------//
void Effect::pre_process()
{
	if( --life < 0)
	{
		effect_array.del(sprite_recno);
		return;
	}
}
// ------- End of function Effect::pre_process -------//


// ------- Begin of function Effect::process_idle -------//
void Effect::process_idle()
{
	if( ++cur_frame > cur_sprite_stop()->frame_count )
		cur_frame = 1;
}
// ------- End of function Effect::process_idle -------//


// ------- Begin of function Effect::process_die -------//
int Effect::process_die()
{
	if( ++cur_frame > cur_sprite_die()->frame_count )
		cur_frame = 1;
	return 0;
}
// ------- End of function Effect::process_die -------//


// ------- Begin of function Effect::create -------//
void Effect::create(short spriteId, short startX, short startY, char initAction, char initDir, char dispLayer, int effectLife)
{
	Effect *effectPtr = new Effect;
	effectPtr->init(spriteId, startX, startY, initAction, initDir, dispLayer, effectLife);
	effect_array.add(effectPtr);
}
// ------- End of function Effect::create -------//
