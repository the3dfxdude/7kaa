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

//Filename    : OTORNADO.H
//Description : Header file of Object Tornado
//Ownership   : Gilbert

#ifndef __OTORNADO_H
#define __OTORNADO_H

#ifndef __OSPRITE_H
#include <OSPRITE.h>
#endif

//#ifndef __OUNIT_H
//#include <OUNIT.h>
//#endif

//#ifndef __OFIRM_H
//#include <OFIRM.h>
//#endif

//--------- Define constant -----------//


//----------- Define class Tornado -----------//

#pragma pack(1)
class Tornado : public Sprite
{
public:
   float attack_damage;
   short life_time;
   short dmg_offset_x;
   short dmg_offset_y;

public:
   void  init(short startX, short startY, short lifeTime);
   //void  update_abs_pos(SpriteFrame* =0);
   //void  draw();
   void  pre_process();
   void  process_move();
   void  hit_target();
   void  hit_building();
   void  hit_plant();
   void  hit_fire();

   short damage_x_loc()    { return (cur_x + dmg_offset_x) >> ZOOM_X_SHIFT_COUNT; }
   short damage_y_loc()    { return (cur_y + dmg_offset_y) >> ZOOM_Y_SHIFT_COUNT; }

	//--------- file functions -----------//

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);
};
#pragma pack()

//------- Define class TornadoArray ---------//

class TornadoArray : public SpriteArray
{
public:
	TornadoArray(int initArraySize);

	short create_tornado();
	short add_tornado(int xLoc, int yLoc, short lifeTime);   // unit attacks firm, townzone
	// short tornado_possible(int parentXLoc, int parentYLoc, int targetXLoc, int targetYLoc, char tornadoSpeed);

	#ifdef DEBUG
		Tornado* operator[](int recNo);
	#else
		Tornado* operator[](int recNo)   { return (Tornado*) get_ptr(recNo); }
	#endif
	void process();
	void draw_dot();

	//--------- file functions -----------//

	int  write_file(File* filePtr);
	int  read_file(File* filePtr);
};

extern TornadoArray tornado_array;

//-----------------------------------------//

#endif
