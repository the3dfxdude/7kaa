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

//Filename    : OSPRITE2.CPP
//Description : Object Sprite

#include <ALL.h>
#include <OSYS.h>
#include <OVGA.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OUNIT.h>
#include <OSPRITE.h>
#include <OSERES.h>

#ifdef DEBUG
static int check_dir1, check_dir2;
/*static int check_dir(int curX, int curY, int destX, int destY)
{
	if( destX == curX )
	{
		if( destY > curY )
			return DIR_S;
		else
			return DIR_N;
	}
	else if( destX < curX )
	{
		if( destY == curY )
			return DIR_W;
		else if( destY > curY )
			return DIR_SW;
		else
			return DIR_NW;
	}
	else
	{
		if( destY == curY )
			return DIR_E;
		else if( destY > curY )
			return DIR_SE;
		else
			return DIR_NE;
	}
}*/
#endif

//--- Define no. of pixels per direction move (N, NE, E, SE, S, SW, W, NW) ---//

static short move_x_pixel_array[] = { 0, ZOOM_LOC_WIDTH, ZOOM_LOC_WIDTH, ZOOM_LOC_WIDTH, 0, -ZOOM_LOC_WIDTH, -ZOOM_LOC_WIDTH, -ZOOM_LOC_WIDTH };
static short move_y_pixel_array[] = { -ZOOM_LOC_HEIGHT, -ZOOM_LOC_HEIGHT, 0, ZOOM_LOC_HEIGHT, ZOOM_LOC_HEIGHT, ZOOM_LOC_HEIGHT, 0, -ZOOM_LOC_HEIGHT };

//--------- Begin of function Sprite::sprite_move --------//
//
// <int> desX, desY   - the destination coordination
//
void Sprite::sprite_move(int desX, int desY)
{
	if( cur_action != SPRITE_MOVE )
	{
		cur_action = SPRITE_MOVE;
		cur_frame  = 1;
	}

	err_when(desX<0 || desY<0 || desX>=MAX_WORLD_X_LOC*ZOOM_LOC_WIDTH || desY>=MAX_WORLD_Y_LOC*ZOOM_LOC_HEIGHT);

#ifdef DEBUG	
	short vectorX=desX-next_x;
	short vectorY=desY-next_y;
	
	if(vectorX && vectorY)	// both are non-zero
	{
		err_if(abs(vectorX) != abs(vectorY))
			err_here();
	}
#endif

	go_x = desX;
	go_y = desY;

	//----------- determine the movement direction ---------//
	set_dir(cur_x, cur_y, go_x, go_y);
	
	//------ set the next tile to move towards -------//
	int stepMagn = move_step_magn();
	set_next(cur_x+stepMagn*move_x_pixel_array[final_dir], cur_y+stepMagn*move_y_pixel_array[final_dir], -stepMagn);

	err_when(cur_action==SPRITE_MOVE && (cur_x!=next_x || cur_y!=next_y) &&
				final_dir!=(check_dir1=get_dir(cur_x, cur_y, next_x, next_y)));
}
//---------- End of function Sprite::sprite_move --------//


//------ Begin of function Sprite::match_dir -------//
// return 1 if matched
// return 0 otherwise
//
int Sprite::match_dir()
{
	err_when(final_dir<0 || final_dir>MAX_SPRITE_DIR_TYPE);

	if(!sprite_info->need_turning)
	{
		cur_dir = final_dir;
		err_when(turn_delay);
		return 1;
	}

	static char turn_amount[10] = {60, 30, 20, 15, 12, 10, 9, 8, 7, 6};
	#define HALF_SPRITE_DIR_TYPE		MAX_SPRITE_DIR_TYPE/2
	#define TURN_REQUIRE_AMOUNT		60

	if(cur_dir==final_dir) // same direction
	{
		turn_delay = 0;
		return 1;
	}

	char turnAmount = turn_amount[sprite_info->need_turning];
	
	if((cur_dir+HALF_SPRITE_DIR_TYPE)%MAX_SPRITE_DIR_TYPE == final_dir) // opposite direction
	{
		cur_dir += (final_dir%2 ? 1 : -1);
		cur_dir = cur_dir%MAX_SPRITE_DIR_TYPE;
	}
	else
	{
		err_when(cur_dir==final_dir || ((cur_dir+HALF_SPRITE_DIR_TYPE)%MAX_SPRITE_DIR_TYPE)==final_dir);

		UCHAR dirDiff = (final_dir - cur_dir + MAX_SPRITE_DIR_TYPE) % MAX_SPRITE_DIR_TYPE;
		if(dirDiff<HALF_SPRITE_DIR_TYPE)
		{
			turn_delay += turnAmount;
			if(turn_delay>=TURN_REQUIRE_AMOUNT)
			{
				cur_dir = (cur_dir+1)%MAX_SPRITE_DIR_TYPE;
				turn_delay = 0;
			}
		}
		else
		{
			turn_delay -= turnAmount;
			if(turn_delay<=-TURN_REQUIRE_AMOUNT)
			{
				cur_dir = (cur_dir-1+MAX_SPRITE_DIR_TYPE)%MAX_SPRITE_DIR_TYPE;
				turn_delay = 0;
			}
		}
	}

	err_when(final_dir<0 || final_dir>MAX_SPRITE_DIR_TYPE);
	err_when(cur_dir<0 || cur_dir>MAX_SPRITE_DIR_TYPE);
	err_when(final_dir==cur_dir && turn_delay);

	return (final_dir==cur_dir);
}
//---------- End of function Sprite::match_dir --------//


//------ Begin of function Sprite::set_dir -------//
void Sprite::set_dir(int curX, int curY, int destX, int destY)
{
	UCHAR newDir = get_dir(curX, curY, destX, destY);
	if(newDir != final_dir)
	{
		final_dir = newDir;
		turn_delay = 0;
	}

	if(!sprite_info->need_turning)
		cur_dir = final_dir;
	else
		match_dir(); // start turning
}
//---------- End of function Sprite::set_dir --------//


//------ Begin of function Sprite::set_dir -------//
void Sprite::set_dir(UCHAR newDir)
{
	if(newDir != final_dir)
	{
		final_dir = newDir;
		turn_delay = 0;
	}

	if(!sprite_info->need_turning)
		cur_dir = final_dir;
	else
		match_dir();
}
//---------- End of function Sprite::set_dir --------//


//------ Begin of function Sprite::is_dir_correct -------//
int Sprite::is_dir_correct()
{
	//### begin alex 1/8 ###//
	//return (cur_dir==final_dir);
	return (cur_dir==final_dir && turn_delay==0);
	//#### end alex 1/8 ####//
}
//---------- End of function Sprite::is_dir_correct --------//


//------ Begin of function Sprite::get_dir -------//
//
	// Compare the current and destination locations and
// return which direction the sprite should move towards or attack at.
//
int Sprite::get_dir(int curX, int curY, int destX, int destY)
{
	unsigned xDiff( abs(destX - curX) );
	unsigned yDiff( abs(destY - curY) );
	unsigned squSize = MAX(xDiff, yDiff); // the size of the square we consider

	if( destX == curX )
	{
		if( destY > curY )
			return DIR_S;
		else
			return DIR_N;
	}
	else if( destX < curX )
	{
		// west side
		if( destY > curY )
		{ // south west quadrant
			if(2*xDiff <= squSize)
				return DIR_S;
			else if(2*yDiff <= squSize)
				return DIR_W;
			else
				return DIR_SW;
		}
		else
		{ // north west quadrant
			if(2*xDiff <= squSize)
				return DIR_N;
			else if(2*yDiff <= squSize)
				return DIR_W;
			else
				return DIR_NW;
		}
	}
	else // destX > curX
	{
		// east side
		if( destY > curY )
		{ // south east quadrant
			if(2*xDiff <= squSize)
				return DIR_S;
			else if(2*yDiff <= squSize)
				return DIR_E;
			else
				return DIR_SE;
		}
		else
		{
			// north east quadrant
			if(2*xDiff <= squSize)
				return DIR_N;
			else if(2*yDiff <= squSize)
				return DIR_E;
			else
				return DIR_NE;
		}
	}

	return DIR_N;
}
//------- End of function Sprite::get_dir --------//


//--------- Begin of function Sprite::process_idle --------//

void Sprite::process_idle()
{
	//-------- If it's an air unit --------//
	// note : most land units do have have stop frame,
	// so cur_sprite_stop->frame_count is 0
	if( ++cur_frame > cur_sprite_stop()->frame_count )
		cur_frame = 1;
}
//---------- End of function Sprite::process_idle ----------//


//--------- Begin of function Sprite::process_move --------//

void Sprite::process_move()
{
	#ifdef DEBUG
	int debugStepMagn1 = move_step_magn();
	err_when(abs(cur_x-next_x)>ZOOM_LOC_WIDTH*debugStepMagn1 || abs(cur_y-next_y)>ZOOM_LOC_HEIGHT*debugStepMagn1);
	#endif

	//---- for some sprite (e.g. elephant), move one step per a few frames ----//

	if( --remain_frames_per_step > 0 )
		return;
	else
		remain_frames_per_step = sprite_info->frames_per_step;

	err_when(cur_x < 0 || cur_y < 0 || cur_x >= ZOOM_X_PIXELS || cur_y >= ZOOM_Y_PIXELS);
	err_when(cur_x-next_x!=0 && cur_y-next_y!=0 && abs(next_x-cur_x)!=abs(next_y-cur_y));

	//----- if the sprite has reach the destintion ----//

	if( cur_x==go_x && cur_y==go_y)
	{
		cur_action = SPRITE_IDLE;
		set_next(cur_x, cur_y);			//********* BUGHERE

#ifdef DEBUG
		char h, w, blocked=0;
		short x, y;

		for(h=0, y=next_y_loc(); h<sprite_info->loc_height&&!blocked; h++, y++)
		{
			for(w=0, x=next_x_loc(); w<sprite_info->loc_width&&!blocked; w++, x++)
				blocked = world.get_unit_recno(x,y,mobile_type) != sprite_recno;
		}
		err_if(blocked)
			err_here();
#endif
		cur_frame  = 1;
		return;
	}

	err_when(cur_x-next_x!=0 && cur_y-next_y!=0 && abs(next_x-cur_x)!=abs(next_y-cur_y));

	//---- set the next tile the sprite will be moving towards ---//

	static short vector_x_array[] = {  0,  1, 1, 1, 0, -1, -1, -1 };	// default vectors, temporary only
	static short vector_y_array[] = { -1, -1, 0, 1, 1,  1,  0, -1 };

	short stepX   = sprite_info->speed;//abs(vectorX);	//********* improve later
	short stepY   = sprite_info->speed;//abs(vectorY);

	if( next_x != go_x || next_y != go_y )		// if next_x==go_x & next_y==go_y, reach destination already, don't move further.
	{
		if( abs(cur_x-next_x) <= stepX && abs(cur_y-next_y) <= stepY )
		{
			int stepMagn = move_step_magn();
			set_next(next_x+stepMagn*move_x_pixel_array[final_dir], next_y+stepMagn*move_y_pixel_array[final_dir], -stepMagn);
		}
	}

	err_when(cur_x-next_x!=0 && cur_y-next_y!=0 && abs(next_x-cur_x)!=abs(next_y-cur_y));
	
	//---- if the is blocked, cur_action is changed to SPRITE_WAIT, return now ----//

	if( cur_action != SPRITE_MOVE )
		return;

	//-------------- update position -----------------//
	//
	// If it gets very close to the destination, fit it
	// to the destination ingoring the normal vector.
	//
	//------------------------------------------------//

	short vectorX = vector_x_array[final_dir] * sprite_info->speed;	// cur_dir may be changed in the above set_next() call
	short vectorY = vector_y_array[final_dir] * sprite_info->speed;

	if( abs(cur_x-go_x) <= stepX )
		cur_x = go_x;
	else
		cur_x += vectorX;

	if( abs(cur_y-go_y) <= stepY )
		cur_y = go_y;
	else
		cur_y += vectorY;

	err_when(cur_x-next_x!=0 && cur_y-next_y!=0 && abs(next_x-cur_x)!=abs(next_y-cur_y));	// is a better checking if speed in all direction is equal
	#ifdef DEBUG
	int debugStepMagn2 = move_step_magn();
	err_when(abs(cur_x-next_x)>ZOOM_LOC_WIDTH*debugStepMagn1 || abs(cur_y-next_y)>ZOOM_LOC_HEIGHT*debugStepMagn1);
	#endif

	//-------- check boundary ---------//

	err_when(cur_x < 0 || cur_y < 0 || cur_x >= ZOOM_X_PIXELS || cur_y >= ZOOM_Y_PIXELS);

	//------- update frame id. --------//

	if( ++cur_frame > cur_sprite_move()->frame_count )
		cur_frame = 1;
}
//---------- End of function Sprite::process_move ----------//


//--------- Begin of function Sprite::process_attack --------//
//
// Return: <int> 1 - if the sprite just finished its current attack
//					  0 - other statuses - either waiting for next attack
//					      or is attacking.
//
int Sprite::process_attack()
{
	if(remain_attack_delay && cur_frame==1)
		return 0;

	//------- next attack frame --------//
	SpriteAttack* spriteAttack = cur_sprite_attack();

	// ------ sound effect --------//
	char action[] = "A1";
	action[1] += cur_attack;
	se_res.sound(cur_x_loc(), cur_y_loc(), cur_frame, 'S', sprite_id, action);

	if( ++cur_frame > spriteAttack->frame_count )
	{
		((Unit *)this)->cycle_eqv_attack();		// assume only unit can attack
		cur_frame = 1;
		return 1;
	}

	return 0;
}
//---------- End of function Sprite::process_attack ----------//


//--------- Begin of function Sprite::process_turn --------//
void Sprite::process_turn()
{
	err_when(!sprite_info->need_turning);
	match_dir();
}
//---------- End of function Sprite::process_turn ----------//


//--------- Begin of function Sprite::process_die --------//
//
// return : <int> 1 - dying animation completes.
//					   0 - still dying 
//
int Sprite::process_die()
{
	//--------- next frame ---------//

	if( sys.frame_count%3 == 0 )
	{
		se_res.sound(cur_x_loc(), cur_y_loc(), cur_frame, 'S',sprite_id,"DIE");
		if( ++cur_frame > sprite_info->die.frame_count )
			return 1;
	}

	return 0;
}
//---------- End of function Sprite::process_die ----------//


//--------- Begin of function Sprite::set_remain_attack_delay --------//
void Sprite::set_remain_attack_delay()
{
	Unit* unitPtr = (Unit*) this;		//**BUGHERE, assuming all Sprite that call process_attack() are Unit
	remain_attack_delay = unitPtr->attack_info_array[unitPtr->cur_attack].attack_delay;
}
//---------- End of function Sprite::set_remain_attack_delay ----------//


//--------- Begin of function Sprite::move_step_magn --------//
// return the magnitude of each step in moving
// i.e 1 for unit_land, 2 for unit_sea and unit_air
//
int Sprite::move_step_magn()
{
	return (mobile_type==UNIT_LAND) ? 1 : 2;
}
//---------- End of function Sprite::move_step_magn ----------//


//--------- Begin of function Sprite::set_next --------//
//
void Sprite::set_next(int nextX, int nextY, int para, int blockedChecked)
{
	err_here();		//**BUGHERE, for now, we don't have any pure Sprites, all functions should call Unit::set_next() instead of Sprite::set_next()

	next_x = nextX;
	next_y = nextY;
}
//---------- End of function Sprite::set_next ----------//


//--------- Begin of function Sprite::set_guard_on --------//
//
void Sprite::set_guard_on()
{
	err_when( !sprite_info->can_stand_guard() && !sprite_info->can_move_guard());
	guard_count = 1;
}
//--------- End of function Sprite::set_guard_on --------//


//--------- Begin of function Sprite::set_guard_off --------//
//
void Sprite::set_guard_off()
{
	guard_count = 0;
}
//--------- End of function Sprite::set_guard_off --------//

