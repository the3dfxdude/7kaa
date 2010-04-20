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

//Filename    : OBULLET.CPP
//Description : Object Bullet
//Owner		  : Alex

#include <OVGA.h>
#include <OUNIT.h>
#include <OBULLET.h>
#include <OWORLD.h>
#include <OSERES.h>
#include <OU_CART.h>
#include <OTOWN.h>
#include <ONATIONA.h>

// -------- Define constant ---------//

const int SCAN_RADIUS = 2;
const int SCAN_RANGE = SCAN_RADIUS * 2 + 1;

// from the closet to the far
static char spiral_x[SCAN_RANGE*SCAN_RANGE] = 
	{ 0, 0,-1, 0, 1,-1,-1, 1, 1, 0,-2, 0, 2, -1,-2,-2,-1, 1, 2, 2, 1,-2,-2, 2, 2};
static char spiral_y[SCAN_RANGE*SCAN_RANGE] = 
	{ 0,-1, 0, 1, 0,-1, 1, 1,-1,-2, 0, 2, 0, -2,-1, 1, 2, 2, 1,-1,-2,-2, 2, 2,-2};

//--------- Begin of function Bullet::Bullet -------//

Bullet::Bullet()
{
	sprite_id = 0;
}
//--------- End of function Bullet::Bullet -------//


//--------- Begin of function Bullet::init ---------//
//
// <char> parentType		- the type of object emits the bullet
// <short> parentRecno	- the recno of the object
// <short> targetXLoc	- the x loc of the target
// <short> targetYLoc	- the y loc of the target
//	<char> targetMobileType - target mobile type
//
void Bullet::init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType)
{
	parent_type = parentType;
	parent_recno = parentRecno;
	target_mobile_type = targetMobileType;

	//**** BUGHERE, using parentType and parentRecno to allow bullet by firm, town, etc.
	//**** BUGHERE, only allow bullet by unit for this version
	err_when(parent_type!=BULLET_BY_UNIT);
	Unit *parentUnit = unit_array[parentRecno];

	//---------- copy attack info from the parent unit --------//

	AttackInfo* attackInfo = parentUnit->attack_info_array+parentUnit->cur_attack;

	attack_damage  = parentUnit->actual_damage();
	damage_radius  = attackInfo->bullet_radius;
	nation_recno   = parentUnit->nation_recno;
	// ###### begin Gilbert 26/6 ########## //
	fire_radius    = attackInfo->fire_radius;
	// ###### end Gilbert 26/6 ########## //

	//----- clone vars from sprite_res for fast access -----//

	sprite_id 	= attackInfo->bullet_sprite_id;
	sprite_info = sprite_res[sprite_id];

	sprite_info->load_bitmap_res();		// the sprite bitmap will be freed by ~Sprite(), so we don't have to add ~Bullet() to free it. 

	//--------- set the starting position of the bullet -------//

	cur_action = SPRITE_MOVE;
	cur_frame  = 1;
	set_dir(parentUnit->attack_dir);

	SpriteFrame* spriteFrame = cur_sprite_frame();

	origin_x = cur_x = parentUnit->cur_x;
	origin_y = cur_y = parentUnit->cur_y;

	//------ set the target position and bullet mobile_type -------//

	target_x_loc = targetXLoc;
	target_y_loc = targetYLoc;

	go_x = target_x_loc * ZOOM_LOC_WIDTH  + ZOOM_LOC_WIDTH/2  - spriteFrame->offset_x - spriteFrame->width/2;			// -spriteFrame->offset_x to make abs_x1 & abs_y1 = original x1 & y1. So the bullet will be centered on the target
	go_y = target_y_loc * ZOOM_LOC_HEIGHT + ZOOM_LOC_HEIGHT/2 - spriteFrame->offset_y - spriteFrame->height/2;

	mobile_type = parentUnit->mobile_type;

	//---------- set bullet movement steps -----------//

	int xStep 	= (go_x - cur_x)/attackInfo->bullet_speed;
	int yStep 	= (go_y - cur_y)/attackInfo->bullet_speed;

	total_step  = MAX(1, MAX(abs(xStep), abs(yStep)));
	cur_step    = 0;

	err_when( total_step < 0 );		// number overflow
}
//----------- End of function Bullet::init -----------//


//--------- Begin of function Bullet::process_move --------//

void Bullet::process_move()
{
	//-------------- update position -----------------//
	//
	// If it gets very close to the destination, fit it
	// to the destination ingoring the normal vector.
	//
	//------------------------------------------------//

	cur_x = origin_x + (int)(go_x-origin_x) * cur_step / total_step;
	cur_y = origin_y + (int)(go_y-origin_y) * cur_step / total_step;

	//cur_step++;

	//------- update frame id. --------//

	if( ++cur_frame > cur_sprite_move()->frame_count )
		cur_frame = 1;

	//----- if the sprite has reach the destintion ----//

	//if( cur_step > total_step )
	if( ++cur_step > total_step )
	{
		check_hit();

		cur_action = SPRITE_DIE;		// Explosion

		// ###### begin Gilbert 17/5 #########//
		// if it has die frame, adjust cur_x, cur_y to be align with the target_x_loc, target_y_loc
		if( sprite_info->die.first_frame_recno )
		{
			next_x = cur_x = target_x_loc * ZOOM_LOC_WIDTH;
			next_y =cur_y = target_y_loc * ZOOM_LOC_HEIGHT;
		}
		// ###### end Gilbert 17/5 #########//

		cur_frame = 1;
	}
	else if( total_step - cur_step == 1 )
	{
		warn_target();
	}
}
//---------- End of function Bullet::process_move ----------//


//--------- Begin of function Bullet::process_die --------//
//
// return : <int> 1 - dying animation completes.
//					   0 - still dying 
//
int Bullet::process_die()
{

	// ------- sound effect --------//
	se_res.sound(cur_x_loc(), cur_y_loc(), cur_frame, 'S',sprite_id,"DIE");

	//--------- next frame ---------//
	if( ++cur_frame > sprite_info->die.frame_count )
	// ####### begin Gilbert 28/6 ########//
	if( ++cur_frame > sprite_info->die.frame_count )
	{
		// ------- set fire on the target area --------//
		if( fire_radius > 0)
		{
			Location *locPtr;
			if( fire_radius == 1)
			{
				locPtr = world.get_loc(target_x_loc, target_y_loc);
				if( locPtr->can_set_fire() && locPtr->fire_str() < 30 )
					locPtr->set_fire_str(30);
				if( locPtr->fire_src() > 0 )
					locPtr->set_fire_src(1);		// such that the fire will be put out quickly
			}
			else
			{
				short x, y, x1, y1, x2, y2;
				// ##### begin Gilbert 2/10 ######//
				x1 = target_x_loc - fire_radius + 1;
				if( x1 < 0 )
					x1 = 0;
				y1 = target_y_loc - fire_radius + 1;
				if( y1 < 0 )
					y1 = 0;
				x2 = target_x_loc + fire_radius - 1;
				if( x2 >= world.max_x_loc )
					x2 = world.max_x_loc-1;
				y2 = target_y_loc + fire_radius - 1;
				if( y2 >= world.max_y_loc )
					y2 = world.max_y_loc-1;
				// ##### end Gilbert 2/10 ######//
				for( y = y1; y <= y2; ++y)
				{
					locPtr = world.get_loc(x1, y);
					for( x = x1; x <= x2; ++x, ++locPtr)
					{
						// ##### begin Gilbert 30/10 ######//
						int dist = abs(x-target_x_loc) + abs(y-target_y_loc);
						if( dist > fire_radius)
							continue;
						int fl = 30 - dist * 7;
						if( fl < 10 )
							fl = 10;
						if( locPtr->can_set_fire() && locPtr->fire_str() < fl )
							locPtr->set_fire_str(fl);
						if( locPtr->fire_src() > 0 )
							locPtr->set_fire_src(1);		// such that the fire will be put out quickly
						// ##### begin Gilbert 30/10 ######//
					}
				}
			}
		}
		return 1;
	}
	// ####### end Gilbert 28/6 ########//
	return 0;
}
//--------- End of function Bullet::process_die --------//


//--------- Begin of function Bullet::hit_target --------//

// ####### begin Gilbert 14/5 #########//
void Bullet::hit_target(short x, short y)
{
	//---- check if there is any unit in the target location ----//

	Location* locPtr = world.get_loc(x, y);
// ####### end Gilbert 14/5 #########//

	short	targetUnitRecno = locPtr->unit_recno(target_mobile_type);
	if(unit_array.is_deleted(targetUnitRecno))
		return; // the target unit is deleted

	Unit* targetUnit = unit_array[targetUnitRecno];

	Unit* parentUnit;
	if(unit_array.is_deleted(parent_recno))
	//### begin alex 26/9 ###//
	//	parentUnit = NULL;	// parent is dead
	{
		parentUnit = NULL;	// parent is dead
		if(nation_array.is_deleted(nation_recno))
			return;
	}
	//#### end alex 26/9 ####//
	else
	{
		parentUnit = unit_array[parent_recno];
		nation_recno = parentUnit->nation_recno;
	}

	float attackDamage = attenuated_damage(targetUnit->cur_x, targetUnit->cur_y);

	// -------- if the unit is guarding reduce damage ----------//
	err_when(unit_array.is_deleted(locPtr->unit_recno(target_mobile_type)));
	// ##### begin Gilbert 14/5 #########//
	if( attackDamage == 0 )
		return;

	if( targetUnit->is_nation(nation_recno) )
	{
		if( targetUnit->unit_id == UNIT_EXPLOSIVE_CART )
			((UnitExpCart *)targetUnit)->trigger_explode();
		return;
	}
	// ##### end Gilbert 14/5 #########//

	// ##### begin Gilbert 3/9 #########//
	if( !nation_array.should_attack(nation_recno, targetUnit->nation_recno) )
		return;
	// ##### end Gilbert 3/9 #########//
	if(targetUnit->is_guarding())
	{
		switch(targetUnit->cur_action)
		{
		case SPRITE_IDLE:
		case SPRITE_READY_TO_MOVE:
		case SPRITE_TURN:
		case SPRITE_MOVE:
// #ifdef AMPLUS
		case SPRITE_ATTACK:
// #endif
			// ####### begin Gilbert 9/9 #######//
			// check if on the opposite direction
			if( (targetUnit->cur_dir & 7)== ((cur_dir + 4 ) & 7)
				|| (targetUnit->cur_dir & 7)== ((cur_dir + 3 ) & 7)
				|| (targetUnit->cur_dir & 7)== ((cur_dir + 5 ) & 7) )
			// ####### end Gilbert 9/9 #######//
			{
				attackDamage = attackDamage > (float)10/ATTACK_SLOW_DOWN ? attackDamage - (float)10/ATTACK_SLOW_DOWN : 0;
				se_res.sound( targetUnit->cur_x_loc(), targetUnit->cur_y_loc(), 1,
				'S', targetUnit->sprite_id, "DEF", 'S', sprite_id );
			}
			break;
		}
	}

	targetUnit->hit_target(parentUnit, targetUnit, attackDamage);
}
//---------- End of function Bullet::hit_target ----------//


//------- Begin of function Bullet::hit_building -----//
//	building means firm or town
//
// ###### begin Gilbert 14/5 #########//
void Bullet::hit_building(short x, short y)
{
	Location* locPtr = world.get_loc(x, y);

	if(locPtr->is_firm())
	{
		Firm *firmPtr = firm_array[locPtr->firm_recno()];
		// ##### begin Gilbert 3/9 #########//
		if( !firmPtr || !nation_array.should_attack(nation_recno, firmPtr->nation_recno) )
		// ##### end Gilbert 3/9 #########//
			return;
	}
	else if(locPtr->is_town())
	{
		Town *townPtr = town_array[locPtr->town_recno()];
		// ##### begin Gilbert 3/9 #########//
		if( !townPtr || !nation_array.should_attack(nation_recno, townPtr->nation_recno) )
		// ##### end Gilbert 3/9 #########//
			return;
	}
	else
		return;

	float attackDamage = attenuated_damage(x * ZOOM_LOC_WIDTH, y * ZOOM_LOC_HEIGHT );
	// BUGHERE : hit building of same nation?
	if( attackDamage == 0)
		return;

	Unit *virtualUnit, *parentUnit;
	if(unit_array.is_deleted(parent_recno))
	{
		parentUnit = NULL;
		//### begin alex 26/9 ###//
		if(nation_array.is_deleted(nation_recno))
			return;
		//#### end alex 26/9 ####//

		for(int i=unit_array.size(); i>0; i--)
		{
			if(unit_array.is_deleted(i))
				continue;

			virtualUnit = unit_array[i];
			break;
		}

		if(!virtualUnit)
			return; //**** BUGHERE
	}
	else
		virtualUnit = parentUnit = unit_array[parent_recno];

	virtualUnit->hit_building(parentUnit, target_x_loc, target_y_loc, attackDamage);
	// ####### end Gilbert 14/5 ########//
}
//---------- End of function Bullet::hit_building ----------//


//------- Begin of function Bullet::hit_wall -----//
// ###### begin Gilbert 14/5 #########//
void Bullet::hit_wall(short x, short y)
{
	Location* locPtr = world.get_loc(x, y);

	if(!locPtr->is_wall())
		return;

	float attackDamage = attenuated_damage(x * ZOOM_LOC_WIDTH, y * ZOOM_LOC_HEIGHT );
	if( attackDamage == 0)
		return;
// ###### end Gilbert 14/5 #########//

	Unit *virtualUnit, *parentUnit;
	if(unit_array.is_deleted(parent_recno))
	{
		parentUnit = NULL;
		//### begin alex 26/9 ###//
		if(nation_array.is_deleted(nation_recno))
			return;
		//#### end alex 26/9 ####//

		for(int i=unit_array.size(); i>0; i--)
		{
			if(unit_array.is_deleted(i))
				continue;

			virtualUnit = unit_array[i];
			break;
		}

		if(!virtualUnit)
			return; //**** BUGHERE
	}
	else
		virtualUnit = parentUnit = unit_array[parent_recno];

	// ###### begin Gilbert 14/5 #########//
	virtualUnit->hit_wall(parentUnit, target_x_loc, target_y_loc, attackDamage);
	// ###### end Gilbert 14/5 ########//
}
//---------- End of function Bullet::hit_wall ----------//


//--------- Begin of function Bullet::check_hit -------//
// check if the bullet hit a target
// return true if hit
int Bullet::check_hit()
{
	err_when(SCAN_RANGE != 5);
	
	short x,y;
	short townHit[SCAN_RANGE*SCAN_RANGE];
	short firmHit[SCAN_RANGE*SCAN_RANGE];
	int hitCount = 0;
	int townHitCount = 0;
	int firmHitCount = 0;

	for( int c = 0; c < SCAN_RANGE*SCAN_RANGE; ++c )
	{
		x = target_x_loc + spiral_x[c];
		y = target_y_loc + spiral_y[c];
		if( x >= 0 && x < world.max_x_loc && y >= 0 && y < world.max_y_loc )
		{
			Location *locPtr = world.get_loc(x, y);
			if(target_mobile_type==UNIT_AIR)
			{
				if(locPtr->has_unit(UNIT_AIR))
				{
					hit_target(x,y);
					hitCount++;
				}
			}
			else
			{
				if(locPtr->is_firm())
				{
					short firmRecno = locPtr->firm_recno();
					// check this firm has not been attacked
					short *firmHitPtr;
					for( firmHitPtr = firmHit+firmHitCount-1; firmHitPtr >= firmHit; --firmHitPtr )
					{
						if( *firmHitPtr == firmRecno )
							break;
					}
					if( firmHitPtr < firmHit )				// not found
					{
						firmHit[firmHitCount++] = firmRecno;
						hit_building(x,y);
						hitCount++;
					}
				}
				else if( locPtr->is_town() )
				{
					short townRecno = locPtr->town_recno();
					// check this town has not been attacked
					short *townHitPtr;
					for( townHitPtr = townHit+townHitCount-1; townHitPtr >= townHit; --townHitPtr )
					{
						if( *townHitPtr == townRecno )
							break;
					}
					if( townHitPtr < townHit )				// not found
					{
						townHit[townHitCount++] = townRecno;
						hit_building(x,y);
						hitCount++;
					}
				}
				else if(locPtr->is_wall())
				{
					hit_wall(x,y);
					hitCount++;
				}
				else
				{
					hit_target(x,y);	// note: no error checking here because mobile_type should be taken into account
					hitCount++;
				}
			}
		}
	}

	return hitCount;
}
//--------- End of function Bullet::check_hit -------//


//--------- Begin of function Bullet::warn_target -------//
//
// warn a unit before hit
// return true if a unit is warned
int Bullet::warn_target()
{
	err_when(SCAN_RANGE != 5);
	
	short x,y;
	int warnCount = 0;

	for( int c = 0; c < SCAN_RANGE*SCAN_RANGE; ++c )
	{
		x = target_x_loc + spiral_x[c];
		y = target_y_loc + spiral_y[c];
		if( x >= 0 && x < world.max_x_loc && y >= 0 && y < world.max_y_loc )
		{
			Location *locPtr = world.get_loc(x, y);
			//char targetMobileType;
			//if( (targetMobileType = locPtr->has_any_unit()) != 0)
			//{
			//	short unitRecno = locPtr->unit_recno(UNIT_LAND);
				short unitRecno = locPtr->unit_recno(target_mobile_type);
				if( !unit_array.is_deleted(unitRecno) )
				{
					Unit *unitPtr = unit_array[unitRecno];
					// ####### begin Gilbert 9/9 ########//
					if( attenuated_damage( unitPtr->cur_x, unitPtr->cur_y) > 0 )
					// ####### end Gilbert 9/9 ########//
					{
						warnCount++;
						switch(unitPtr->cur_action)
						{
						case SPRITE_IDLE:
						case SPRITE_READY_TO_MOVE:
						//case SPRITE_TURN:
							if( unitPtr->can_stand_guard() && !unitPtr->is_guarding() )
							{
								unitPtr->set_dir( (cur_dir + 4 ) & 7);  // opposite direction of arrow
								unitPtr->set_guard_on();
							}
							break;
						case SPRITE_MOVE:
							if( unitPtr->can_move_guard() && !unitPtr->is_guarding() 
								// ###### begin Gilbert 9/9 #######//
								&& (	(unitPtr->cur_dir & 7)== ((cur_dir + 4 ) & 7)
									||	(unitPtr->cur_dir & 7)== ((cur_dir + 5 ) & 7)
									||	(unitPtr->cur_dir & 7)== ((cur_dir + 3 ) & 7)
									)
								)
								// ###### end Gilbert 9/9 #######//
							{
								unitPtr->set_guard_on();
							}
							break;
#ifdef AMPLUS
						case SPRITE_ATTACK:
							if( unitPtr->can_attack_guard() && !unitPtr->is_guarding()
								&& unitPtr->remain_attack_delay >= GUARD_COUNT_MAX
								&& (	(unitPtr->cur_dir & 7)== ((cur_dir + 4 ) & 7)
									||	(unitPtr->cur_dir & 7)== ((cur_dir + 5 ) & 7)
									||	(unitPtr->cur_dir & 7)== ((cur_dir + 3 ) & 7)
									)
								)
							{
								unitPtr->set_guard_on();
							}
							break;
#endif
						}
					}
				}
			//}
		}
	}

	return warnCount;
}
//--------- End of function Bullet::warn_target -------//


//--------- Begin of function Bullet::display_layer -------//
char Bullet::display_layer()
{
	if( mobile_type == UNIT_AIR || target_mobile_type == UNIT_AIR )
		return 8;
	else
		return 1;
}
//--------- End of function Bullet::display_layer -------//


//------- Begin of function Bullet::attenuated_damage -----//
float	Bullet::attenuated_damage(short curX, short curY)
{
	short d = m.points_distance(curX, curY, target_x_loc * ZOOM_LOC_WIDTH, target_y_loc * ZOOM_LOC_HEIGHT);
	// damage drops from attack_damage to attack_damage/2, as range drops from 0 to damage_radius
	err_when(damage_radius == 0);
	if( d > damage_radius)
		return (float) 0;
	else
		//return ((attack_damage * (2*damage_radius-d) + 2*damage_radius-1)/ (2*damage_radius) );		// ceiling
		return attack_damage - attack_damage*d/(2*damage_radius);
}
//------- End of function Bullet::attenuated_damage -----//

#ifdef DEBUG

//------- Begin of function BulletArray::operator[] -----//

Bullet* BulletArray::operator[](int recNo)
{
	Bullet* bulletPtr = (Bullet*) get_ptr(recNo);

	if( !bulletPtr )
		err.run( "BulletArray[] is deleted" );

	return bulletPtr;
}

//--------- End of function BulletArray::operator[] ----//

#endif


