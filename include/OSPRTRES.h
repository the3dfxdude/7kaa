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

//Filename    : OSPRTRES.H
//Description : Header file of Object Sprite resource

#ifndef __OSPRTRES_H
#define __OSPRTRES_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#include <win32_compat.h>

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

#ifndef __OSFRMRES_H
#include <OSFRMRES.h>
#endif

//-------- Define struct SpriteRec ----------//

struct SpriteRec
{
	enum { CODE_LEN=8, RECNO_LEN=5, COUNT_LEN=5, SPRITE_PARA_LEN=2, DAMAGE_LEN=3, TURN_RES_LEN=2 };

	char sprite_code[CODE_LEN];

	char sprite_type;
	char sprite_sub_type;

	char need_turning;
	char turn_resolution[TURN_RES_LEN];

	char loc_width[SPRITE_PARA_LEN];
	char loc_height[SPRITE_PARA_LEN];

	char speed[SPRITE_PARA_LEN];
	char frames_per_step[SPRITE_PARA_LEN];
	char max_rain_slowdown[SPRITE_PARA_LEN];
	char max_snow_slowdown[SPRITE_PARA_LEN];
	char lightning_damage[DAMAGE_LEN];
	// ###### begin Gilbert 21/8 #######//
	char remap_bitmap_flag;
	// ###### end Gilbert 21/8 #######//

	char first_move_recno[RECNO_LEN];
	char move_count[COUNT_LEN];
};

//----- Define struct SpriteActionRec ------//

struct SpriteActionRec
{
	enum { NAME_LEN=8, ACTION_LEN=2, DIR_ID_LEN=2, RECNO_LEN=5, COUNT_LEN=2 };

	char sprite_name[NAME_LEN];
	char action[ACTION_LEN];
	char dir_id[DIR_ID_LEN];
	char first_frame_recno[RECNO_LEN];
	char frame_count[COUNT_LEN];
};

//----- Define struct SpriteMove ------//

struct SpriteMove
{
	short first_frame_recno;		// first frame recno to frame_array.
	char  frame_count;				// no. of frames in the movement
};

//----- Define struct SpriteAttack ------//

struct SpriteAttack
{
	short first_frame_recno;		// first frame recno to frame_array.
	char  frame_count;				// no. of frames in the movement
	char  attack_delay;		      // no. of frames should be delayed between attack motions. (i.e. when one motion is complete, it will delay <delay_frames> before move on to the next action motion in the cycle
};

//----- Define struct SpriteStop ------//

struct SpriteStop
{
	short frame_recno;		// frame recno to frame_array.
	char	frame_count;
};

//----- Define struct SpriteDie ------//

struct SpriteDie
{
	short first_frame_recno;		// first frame recno to frame_array.
	char  frame_count;				// no. of frames in the movement
};

//----- Define struct SpriteGuard ------//

struct SpriteGuardStop
{
	short first_frame_recno;		// first frame recno to frame_array.
	char	frame_count;
};

//----- Define struct SpriteGuard ------//

struct SpriteGuardMove
{
	short first_frame_recno;		// first frame recno to frame_array.
	char  frame_count;				// no. of frames in the movement
};

//---------- Define struct SpriteInfo -----------//
struct SubSpriteInfo;

struct SpriteInfo
{
public:
	enum { CODE_LEN=8 };

	char 			 sprite_code[CODE_LEN+1];

	char			 sprite_type;
	char			 sprite_sub_type;

	char 			 need_turning;
	char			 turn_resolution;

	short			 loc_width;			// no. of locations it takes horizontally and vertically
	short 		 loc_height;

	BYTE 			 speed;					// based on UnitRes, can be upgraded during the game.
	BYTE			 frames_per_step;
	BYTE			 max_rain_slowdown;
	BYTE			 max_snow_slowdown;
	BYTE			 lightning_damage;
	// ###### begin Gilbert 21/8 #######//
	char			 remap_bitmap_flag;
	// ###### end Gilbert 21/8 #######//
	BYTE			 max_speed;				// original speed
	char			 can_guard_flag;            // bit0= standing guard, bit1=moving guard

	int  			 loaded_count;			// if it >= 1, it has been loaded into the memory
	ResourceDb	 res_bitmap;			// frame bitmap resource

	// move_array[24] to cater upward and downward directions for projectile
	// and also 16-direction movement for weapons
	SpriteMove 	 move_array[3*MAX_SPRITE_DIR_TYPE];
	SpriteAttack attack_array[MAX_UNIT_ATTACK_TYPE][MAX_SPRITE_DIR_TYPE];
	SpriteStop	 stop_array[3*MAX_SPRITE_DIR_TYPE];
	SpriteDie	 die;
	SpriteGuardStop guard_stop_array[MAX_SPRITE_DIR_TYPE];
	SpriteGuardMove guard_move_array[MAX_SPRITE_DIR_TYPE];

	int			sub_sprite_count;
	SubSpriteInfo	*sub_sprite_info;

public:
	~SpriteInfo();

	void			 load_bitmap_res();
	void			 free_bitmap_res();

	int			 is_loaded()		{ return loaded_count>0; }
	SpriteInfo *get_sub_sprite(int i);
	SubSpriteInfo *get_sub_sprite_info(int i);
	int			 can_stand_guard()  { return can_guard_flag & 1;}
	int			 can_move_guard()  { return can_guard_flag & 2;};

	int 			 travel_days(int travelDistance);
};

//------ Define struct SubSpriteRec ---------//

struct SubSpriteRec
{
	enum { CODE_LEN=8, SUB_NO_LEN=3, OFFSET_LEN=3, RECNO_LEN=3 };
	char sprite_code[CODE_LEN];
	char sub_no[SUB_NO_LEN];
	char sub_sprite_code[CODE_LEN];
	char offset_x[OFFSET_LEN];
	char offset_y[OFFSET_LEN];
	char sprite_id[RECNO_LEN];
	char sub_sprite_id[RECNO_LEN];
};

// ------ Define struct SubSpriteInfo ---------//

struct SubSpriteInfo
{
	SpriteInfo *sprite_info;
	short sprite_id;
	short offset_x;
	short offset_y;
};


//---------- Define class SpriteRes ------------//

class SpriteRes
{
public:
	char init_flag;
	int  sprite_info_count;

private:
	SpriteInfo* sprite_info_array;
	SubSpriteInfo *sub_sprite_info_array;

public:
	SpriteRes() 	{ init_flag=0; }

	void	init();
	void  deinit();

	void	update_speed();

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		SpriteInfo* operator[](int recNo);
	#else
		SpriteInfo* operator[](int recNo)   { return sprite_info_array+recNo-1; }
	#endif

private:
	void	load_sprite_info();
	void	load_sub_sprite_info();
};

extern SpriteRes sprite_res;

//----------------------------------------------//

#endif

