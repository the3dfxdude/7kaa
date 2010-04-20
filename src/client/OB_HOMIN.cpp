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

// Filename    : OB_HOMIN.H
// Description : BulletHoming, Homing bullet

#include <OB_HOMIN.h>
#include <OWORLD.h>


// --------- define constant --------//

enum
{
	BULLET_TARGET_NONE =0,
	BULLET_TARGET_UNIT =1,
	BULLET_TARGET_TOWN,
	BULLET_TARGET_FIRM,
	BULLET_TARGET_WALL,
};


// --------- begin of function BulletHoming::BulletHoming --------//
BulletHoming::BulletHoming() : Bullet()
{
	target_type = BULLET_TARGET_NONE;
	target_recno = 0;
}
// --------- end of function BulletHoming::BulletHoming --------//


// --------- begin of function BulletHoming::~BulletHoming --------//
BulletHoming::~BulletHoming()
{
	deinit();
}
// --------- end of function BulletHoming::BulletHoming --------//


// --------- begin of function BulletHoming::init --------//
void BulletHoming::init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType)
{
	Bullet::init(parentType, parentRecno, targetXLoc, targetYLoc, targetMobileType);

	// ------- find the maximum range --------//

	//**** BUGHERE, using parentType and parentRecno to allow bullet by firm, town, etc.
	//**** BUGHERE, only allow bullet by unit for this version
	err_when(parent_type!=BULLET_BY_UNIT);
	Unit *parentUnit = unit_array[parentRecno];

	//---------- copy attack info from the parent unit --------//

	AttackInfo* attackInfo = parentUnit->attack_info_array+parentUnit->cur_attack;
	speed = attackInfo->bullet_speed;
	max_step = char((attackInfo->attack_range * ZOOM_LOC_WIDTH + speed-1)/ speed);

	//--------- keep backup of centre of the bullet ---------//
	SpriteFrame *spriteFrame = cur_sprite_frame();

	// origin_x/y and origin2_x/y are pointing at the centre of the bullet bitmap //
	origin_x += spriteFrame->offset_x + spriteFrame->width/2;
	origin_y += spriteFrame->offset_y + spriteFrame->height/2;
	origin2_x = origin_x; 
	origin2_y = origin_y;
	go_x += spriteFrame->offset_x + spriteFrame->width/2;
	go_y += spriteFrame->offset_y + spriteFrame->height/2;

	// ------- find the target_type and target_recno ------//
	Location *locPtr = world.get_loc(targetXLoc, targetYLoc);
	//### begin alex 16/5 ###//
	//if( locPtr->has_unit(mobile_type) )
	if(locPtr->has_unit(targetMobileType))
	{
		target_type = BULLET_TARGET_UNIT;
		//target_recno = locPtr->unit_recno(mobile_type);
		target_recno = locPtr->unit_recno(targetMobileType);
	}
	//#### end alex 16/5 ####//
	else if( locPtr->is_town() )
	{
		target_type = BULLET_TARGET_TOWN;
		target_recno = locPtr->town_recno();
	}
	else if( locPtr->is_firm() )
	{
		target_type = BULLET_TARGET_FIRM;
		target_recno = locPtr->firm_recno();
	}
	else if( locPtr->is_wall() )
	{
		target_type = BULLET_TARGET_WALL;
	}
}
// --------- end of function BulletHoming::init --------//


// --------- begin of function BulletHoming::deinit --------//
void BulletHoming::deinit()
{
}
// --------- end of function BulletHoming::deinit --------//


// --------- begin of function BulletHoming::process_move --------//
void BulletHoming::process_move()
{
	int actualStep = total_step;

	if(target_type == BULLET_TARGET_UNIT)
	{
		Unit *unitPtr;
		if( unit_array.is_deleted(target_recno) || 
			!(unitPtr = unit_array[target_recno]) || 
			!unitPtr->is_visible() )
		{
			// target lost/die, proceed to Bullet::process_move
			target_type = BULLET_TARGET_NONE;
		}
		else
		{
			// ---- calculate new target_x_loc, target_y_loc -----//	

			target_x_loc = unitPtr->next_x_loc();
			target_y_loc = unitPtr->next_y_loc();

			// ---- re-calculate go_x, go_y  ------//
			// go_x/y and origin2_x/y are pointing at the centre of the bullet bitmap
			// it is different from Bullet
			go_x = unitPtr->cur_x + ZOOM_LOC_WIDTH / 2;
			go_y = unitPtr->cur_y + ZOOM_LOC_HEIGHT /2;

			//---------- set bullet movement steps -----------//
			SpriteFrame *spriteFrame = cur_sprite_frame();
			int adjX = spriteFrame->offset_x+spriteFrame->width/2;
			int adjY = spriteFrame->offset_y+spriteFrame->height/2;

			int xStep 	= abs(go_x - (cur_x+adjX))/speed;
			int yStep 	= abs(go_y - (cur_y+adjY))/speed;
			total_step  = cur_step +  MAX(xStep, yStep);

			// a homing bullet has a limited range, if the target go outside the
			// the limit, the bullet can't attack the target
			// in this case, actualStep is the number step from the source
			// to the target; total_step is the max_step
			// otherwise, actualStep is as same as total_step
			
			actualStep = total_step;
			if( total_step > max_step )
			{
				total_step = max_step;
				// target_x_loc and target_y_loc is limited also
				target_x_loc = (cur_x + adjX) + (int)(go_x-(cur_x+adjX)) / (actualStep - total_step) / ZOOM_LOC_WIDTH;
				target_x_loc = (cur_y + adjY) + (int)(go_y-(cur_y+adjY)) / (actualStep - total_step) / ZOOM_LOC_HEIGHT;
			}
		}
	}

//	origin2_x = origin_x;
//	origin2_y = origin_y;

	// origin_x/y and origin2_x/y are pointing at the centre of the bullet bitmap //
	SpriteFrame *spriteFrame = cur_sprite_frame();
	short adjX = spriteFrame->offset_x + spriteFrame->width/2;
	short adjY = spriteFrame->offset_y + spriteFrame->height/2;
	origin_x = cur_x + adjX;
	origin_y = cur_y + adjY;

	cur_x = origin_x + (int)(go_x-origin_x) / (actualStep + 1 - cur_step);
	cur_y = origin_y + (int)(go_y-origin_y) / (actualStep + 1 - cur_step);
	// cur_x, cur_y is temporary pointing at the centre of bullet bitmap

	// detect changing direction
	if( cur_step > 3 )	// not allow changing direction so fast
		set_dir(origin2_x, origin2_y, cur_x, cur_y);

	// change cur_x, cur_y to bitmap reference point
	spriteFrame= cur_sprite_frame();
	adjX = spriteFrame->offset_x + spriteFrame->width/2;
	adjY = spriteFrame->offset_y + spriteFrame->height/2;
	cur_x -= adjX;
	cur_y -= adjY;

	cur_step++;

	//------- update frame id. --------//

	if( ++cur_frame > cur_sprite_move()->frame_count )
		cur_frame = 1;

	//----- if the sprite has reach the destintion ----//

	if( cur_step > total_step )
	{
		check_hit();

		cur_action = SPRITE_DIE;		// Explosion
		// ###### begin Gilbert 17/5 ########//
		// if it has die frame, adjust cur_x, cur_y to be align with the target_x_loc, target_y_loc
		if( sprite_info->die.first_frame_recno )
		{
			next_x = cur_x = target_x_loc * ZOOM_LOC_WIDTH;
			next_y = cur_y = target_y_loc * ZOOM_LOC_HEIGHT;
		}
		// ###### end Gilbert 17/5 ########//
		cur_frame = 1;
	}
	// change of total_step may not call warn_target, so call more warn_target
	else if( total_step - cur_step <= 1 )
	{
		warn_target();
	}
}
// --------- end of function BulletHoming::process_move --------//
