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

//Filename    : OTORNADO.CPP
//Description : Object Tornado
//Ownership   : Gilbert

#include <OSYS.h>
#include <OVGA.h>
#include <OUNIT.h>
#include <OTORNADO.h>
#include <OWORLD.h>
#include <OWEATHER.h>
#include <math.h>
#include <OFIRM.h>
#include <OFIRMA.h>
#include <OSERES.h>
#include <OTOWN.h>


#define PI 3.141592654
#define DAMAGE_POINT_RADIUS 32

#define TORNADO_SPRITE_ID  12          // Tornado sprite in SPRITE.DBF


//--------- Begin of function Tornado::init ---------//
//
// <short> startX, startY       starting location of the tornado
// <short> lifeTime             time (no. of frames) the tornado survive
//
void Tornado::init(short startX, short startY, short lifeTime)
{
	Sprite::init(TORNADO_SPRITE_ID, startX, startY);
	err_when( strcmp("TORNADO", sprite_info->sprite_code ) );

	attack_damage  = (float) 2.0 / ATTACK_SLOW_DOWN;
	life_time = lifeTime;

	set_dir(DIR_N);
	cur_action = SPRITE_MOVE;		// always moving
	dmg_offset_x = 0;
	dmg_offset_y = 0;
}
//----------- End of function Tornado::init -----------//

//----------- Begin of function Tornado::pre_process ----------//
void Tornado::pre_process()
{
	double angle = misc.random(32) / 16.0 * PI;
	dmg_offset_x = short(DAMAGE_POINT_RADIUS * sin(angle));
	dmg_offset_y = short(DAMAGE_POINT_RADIUS * cos(angle));
	if( --life_time <= 0)
		cur_action = SPRITE_DIE;
}
//----------- End of function Tornado::pre_process ----------//

//--------- Begin of function Tornado::process_move --------//

void Tornado::process_move()
{
	int speed = weather.wind_speed() / 6;
	int minSpeed = magic_weather.wind_day > 0 ? 1 : 5;
	if( speed < minSpeed)
		speed = minSpeed;
	if( speed > 10)
		speed = 10;

	double windDir = weather.wind_direct_rad() + (misc.random(31)-15)*PI/180.0;
	cur_x += short(speed * sin(windDir));
	cur_y -= short(speed * cos(windDir));
	if( ++cur_frame > cur_sprite_move()->frame_count )
		cur_frame = 1;
	// static UCHAR nextFrame[] = { 1,6,1,1,1,1,4 };		// 1->6->4->1 ...
	// cur_frame = nextFrame[cur_frame];

	hit_target();
}
//---------- End of function Tornado::process_move ----------//


//--------- Begin of function Tornado::hit_target --------//

void Tornado::hit_target()
{
	//---- check if there is any unit in the target location ----//

	short damageXLoc = damage_x_loc();
	short damageYLoc = damage_y_loc();
	if( damageXLoc < 0 || damageXLoc >= world.max_x_loc ||
		damageYLoc < 0 || damageYLoc >= world.max_y_loc)
		return;

	Location *locPtr = world.get_loc(damageXLoc, damageYLoc);

	Unit *targetUnit;
	if( locPtr->has_unit(UNIT_AIR))
	{
		targetUnit = unit_array[locPtr->unit_recno(UNIT_AIR)];
		targetUnit->hit_points -= 2*attack_damage;
		if( targetUnit->hit_points <= 0)
			targetUnit->hit_points = (float) 0;
	}

	if( locPtr->has_unit(UNIT_LAND))
	{
		targetUnit = unit_array[locPtr->unit_recno(UNIT_LAND)];
		targetUnit->hit_points -= attack_damage;
		if( targetUnit->hit_points <= 0)
			targetUnit->hit_points = (float) 0;
	}
	else if( locPtr->has_unit(UNIT_SEA))
	{
		targetUnit = unit_array[locPtr->unit_recno(UNIT_SEA)];
		targetUnit->hit_points -= attack_damage;
		if( targetUnit->hit_points <= 0)
			targetUnit->hit_points = (float) 0;
	}
	else
	{
		hit_building();	// pass to hit_building to check whether a building is in the location
	}

	hit_plant();
	hit_fire();
}
//---------- End of function Tornado::hit_target ----------//


//------- Begin of function Tornado::hit_building -----//
//	building means firm or town
//
void Tornado::hit_building()
{
	short damageXLoc = damage_x_loc();
	short damageYLoc = damage_y_loc();
	if( damageXLoc < 0 || damageXLoc >= world.max_x_loc ||
		damageYLoc < 0 || damageYLoc >= world.max_y_loc)
		return;

	Location* locPtr = world.get_loc(damageXLoc, damageYLoc);

	// ##### begin Gilbert 30/10 #####//
	if(locPtr->is_firm() )
//		&& firm_res[(firmPtr=firm_array[locPtr->firm_recno()])->firm_id]->buildable )
	{
		Firm *firmPtr = firm_array[locPtr->firm_recno()];
		firmPtr->hit_points -= attack_damage*2;
		if( firmPtr->hit_points <= 0)
		{
			firmPtr->hit_points = (float) 0;

			se_res.sound(firmPtr->center_x, firmPtr->center_y, 1,
				'F', firmPtr->firm_id, "DIE" );

			firm_array.del_firm(locPtr->firm_recno());
		}
	}

	if( locPtr->is_town() )
	{
		Town *townPtr = town_array[locPtr->town_recno()];
		if( (life_time % 30) == 0 ) 
			townPtr->kill_town_people(0);
	}
	// ##### end Gilbert 30/10 #####//
}
//---------- End of function Tornado::hit_building ----------//


//---------- Begin of function Tornado::hit_plant --------//
void Tornado::hit_plant()
{
	short damageXLoc = damage_x_loc();
	short damageYLoc = damage_y_loc();
	if( damageXLoc < 0 || damageXLoc >= world.max_x_loc ||
		damageYLoc < 0 || damageYLoc >= world.max_y_loc)
		return;

	Location *locPtr = world.get_loc(damageXLoc, damageYLoc);
	if(locPtr->is_plant())
	{
		locPtr->remove_plant();
		// ###### begin Gilbert 24/6 ########//
		world.plant_count--;
		// ###### end Gilbert 24/6 ########//
	}
}
//---------- End of function Tornado::hit_plant --------//


//---------- Begin of function Tornado::hit_fire --------//
void Tornado::hit_fire()
{
	short damageXLoc = damage_x_loc();
	short damageYLoc = damage_y_loc();
	if( damageXLoc < 0 || damageXLoc >= world.max_x_loc ||
		damageYLoc < 0 || damageYLoc >= world.max_y_loc)
		return;

	Location *locPtr = world.get_loc(damageXLoc, damageYLoc);
	if(locPtr->fire_str() > 0)
	{
		locPtr->set_fire_str(1);
	}
}
//---------- End of function Tornado::hit_plant --------//



#ifdef DEBUG

//------- Begin of function TornadoArray::operator[] -----//

Tornado* TornadoArray::operator[](int recNo)
{
	Tornado* tornadoPtr = (Tornado*) get_ptr(recNo);

	if( !tornadoPtr )
		err.run( "TornadoArray[] is deleted" );

	return tornadoPtr;
}

//--------- End of function TornadoArray::operator[] ----//

#endif

//--------- Begin of function TornadoArray::TornadoArray ------//
TornadoArray::TornadoArray(int initArraySize): SpriteArray(initArraySize)
{
}
//--------- End of function TornadoArray::TornadoArray ------//


//--------- Begin of function TornadoArray::add_tornado ------//
//
// <short> startX, startY       starting location of the tornado
// <short> lifeTime             time (no. of frames) the tornado survive
//
short TornadoArray::add_tornado(int xLoc, int yLoc, short lifeTime)
{
	Tornado *tornadoPtr = new Tornado;
	tornadoPtr->init(xLoc, yLoc, lifeTime);
	add(tornadoPtr);
	return 1;
}
//--------- End of function TornadoArray::add_tornado ------//


//--------- Begin of function TornadoArray::create_tornado ------//
//
// return: <int> recno of the tornado created. 
//
short TornadoArray::create_tornado()
{
	Tornado *tornadoPtr = new Tornado;
	add(tornadoPtr);

	return recno();
}
//--------- End of function TornadoArray::create_tornado ------//


//--------- Begin of function TornadoArray::process ---------//

void TornadoArray::process()
{
	int 	  i;
	Tornado* tornadoPtr;

	for (i=1; i <=size() ; i++)
	{
		tornadoPtr = (Tornado*)get_ptr(i);

		if( !tornadoPtr)
			continue;

		//-------- system yield ---------//

		if( i%20==1 )
			sys.yield();

		//------- process tornado --------//

		tornadoPtr->pre_process();

		switch(tornadoPtr->cur_action)
		{
			case SPRITE_IDLE:
			case SPRITE_READY_TO_MOVE:
				tornadoPtr->process_idle();
				break;

			case SPRITE_MOVE:
				tornadoPtr->process_move();
				break;

			//### begin alex 7/3 ###//
			case SPRITE_TURN:
				err_here();
				break;
			//#### end alex 7/3 ####//

			case SPRITE_WAIT:
				tornadoPtr->process_wait();
				break;

			case SPRITE_ATTACK:
				tornadoPtr->process_attack();
				break;

			case SPRITE_DIE:
				if( tornadoPtr->process_die() )
					del(i);
				break;
		}
	}
}
//----------- End of function TornadoArray::process -----------//


//--------- Begin of function TornadoArray::draw_dot ---------//
//
// Draw 2x2 tiny squares on map window representing the location of the unit ---//
//
void TornadoArray::draw_dot()
{
	char*	  vgaBufPtr = vga_back.buf_ptr();
	char*	  writePtr;
	Tornado* tornadoPtr;
	int	  i, mapX, mapY;
	int		vgaBufPitch = vga_back.buf_pitch();

	for(i=1; i <=size() ; i++)
	{
		tornadoPtr = (Tornado*)get_ptr(i);

		if( !tornadoPtr )
			continue;

		mapX = MAP_X1 + tornadoPtr->cur_x_loc();
		mapY = MAP_Y1 + tornadoPtr->cur_y_loc();

		// ####### begin Gilbert 13/11 #########//
		if( mapX < MAP_X1 || mapX > MAP_X2 || mapY < MAP_Y1 || mapY > MAP_Y2 )
			return;
		// ####### end Gilbert 13/11 #########//

		writePtr = vgaBufPtr + mapY*vgaBufPitch + mapX;

		if( mapY > MAP_Y1)
		{
			if( mapX > MAP_X1 && writePtr[-vgaBufPitch-1] != UNEXPLORED_COLOR)
				writePtr[-vgaBufPitch-1] = (char)TORNADO_COLOR2;
			if( writePtr[-vgaBufPitch] != UNEXPLORED_COLOR)
				writePtr[-vgaBufPitch] = (char)TORNADO_COLOR1;
			if( mapX < MAP_X2 && writePtr[-vgaBufPitch+1] != UNEXPLORED_COLOR)
				writePtr[-vgaBufPitch+1] = (char)TORNADO_COLOR2;
		}

		if( mapX > MAP_X1 && writePtr[-1] != UNEXPLORED_COLOR)
			writePtr[-1] = (char)TORNADO_COLOR1;
		if( mapX < MAP_X2 && writePtr[1] != UNEXPLORED_COLOR)
			writePtr[1] = (char)TORNADO_COLOR1;

		if( mapY < MAP_Y2)
		{
			if( mapX > MAP_X1 && writePtr[vgaBufPitch-1] != UNEXPLORED_COLOR)
				writePtr[vgaBufPitch-1] = (char)TORNADO_COLOR2;
			if( writePtr[vgaBufPitch] != UNEXPLORED_COLOR)
				writePtr[vgaBufPitch] = (char)TORNADO_COLOR1;
			if( mapX < MAP_X2 && writePtr[vgaBufPitch+1] != UNEXPLORED_COLOR)
				writePtr[vgaBufPitch+1] = (char)TORNADO_COLOR2;
		}

	}
}
//----------- End of function UnitArray::draw_dot -----------//

