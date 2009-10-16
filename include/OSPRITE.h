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

//Filename    : OSPRITE.H
//Description : Sprite object

#ifndef __OSPRITE_H
#define __OSPRITE_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.h>
#endif

#ifndef __OSPRTRES_H
#include <OSPRTRES.h>
#endif

#ifndef __OWORLD_H
#include <OWORLD.h>
#endif

//---------- Define action types ---------//

enum { SPRITE_IDLE=1,
		 SPRITE_READY_TO_MOVE,
		 SPRITE_MOVE,
		 SPRITE_WAIT,			// During a movement course, waiting for blocked sprites to pass
		 SPRITE_ATTACK,
		 SPRITE_TURN,
		 SPRITE_SHIP_EXTRA_MOVE,	// for ship only
		 SPRITE_DIE,
	  };

//----------- Define constant ----------//

#define GUARD_COUNT_MAX 5

//---------- Define class Sprite -----------//

#pragma pack(1)
class Sprite
{
public:

	//------------------------------------------//

	short sprite_id;			// sprite id. in SpriteRes
	short	sprite_recno;

	char 	mobile_type;

	UCHAR cur_action;       // current action
	UCHAR cur_dir;				// current direction
	UCHAR cur_frame;			// current frame
	UCHAR cur_attack;			// current attack mode
	UCHAR final_dir;			// for turning dir before attacking or moving
	char	turn_delay;			// value between -60 and 60
	char	guard_count;		// shield guarding, affecting move/stop frame
									// 0=not guarding, count up from 1 when guarding, reset to 0 after guard

	UCHAR remain_attack_delay;			// no. of frames has to be delayed before the next attack motion
	UCHAR remain_frames_per_step;    // no. of frames remained in this step

	short cur_x , cur_y;		// current location
	short	go_x  , go_y;		// the destination of the path
	short next_x, next_y;	// next tile in the moving path

	//----- clone vars from sprite_res for fast access -----//

	SpriteInfo*   sprite_info;

	//--------- static member vars --------------//

	static short  view_top_x, view_top_y;		// the view window in the scene, they are relative coordinations on the entire virtual surface.

	//---------- function member vars -------------//

	short	cur_x_loc()		{ return cur_x>>ZOOM_X_SHIFT_COUNT; }		// >>5 = divided by 32, which is ZOOM_LOC_WIDTH & ZOOM_LOC_HEIGHT
	short	cur_y_loc()		{ return cur_y>>ZOOM_Y_SHIFT_COUNT; }

	short	next_x_loc()		{ return next_x>>ZOOM_X_SHIFT_COUNT; }		// >>5 = divided by 32, which is ZOOM_LOC_WIDTH & ZOOM_LOC_HEIGHT
	short	next_y_loc()		{ return next_y>>ZOOM_Y_SHIFT_COUNT; }

	short go_x_loc()			{ return go_x>>ZOOM_X_SHIFT_COUNT; }
	short go_y_loc()			{ return go_y>>ZOOM_Y_SHIFT_COUNT; }

	SpriteMove*   cur_sprite_move()   { return sprite_info->move_array+cur_dir; }
	SpriteAttack* cur_sprite_attack() { return sprite_info->attack_array[cur_attack]+cur_dir; }
	SpriteStop*   cur_sprite_stop()   { return sprite_info->stop_array+cur_dir; }	
	SpriteDie*	  cur_sprite_die()	 { return &(sprite_info->die); }
	SpriteFrame*  cur_sprite_frame(int *needMirror=0);

	//----------- static variables -------------//

	static short abs_x1, abs_y1;	// the absolute postion, taking in account of sprite offset
	static short abs_x2, abs_y2;

public:
			  Sprite();
	virtual ~Sprite();

			  void 	init(short spriteId, short startX, short startY);
			  void	deinit();

	virtual void	draw();

			  void 	sprite_move(int desX, int desY);

			  void	set_cur(int curX, int curY) 		{ cur_x=curX; cur_y=curY; update_abs_pos(); }
	virtual void	set_next(int nextX, int nextY, int para=0, int blockedChecked=0);
			  int		move_step_magn();
	
	virtual void	pre_process()		{;}
	virtual void	process_idle();
	virtual void	process_move();
	virtual void	process_wait()		{;}
	virtual int 	process_attack();
	virtual int 	process_die();
			  void	process_turn();
	virtual void	process_extra_move() {;} // for ship only

			  void	set_dir(int curX, int curY, int destX, int destY);
			  void	set_dir(UCHAR newDir);	//	 overloading function
			  int 	get_dir(int curX, int curY, int destX, int destY);
			  int		is_dir_correct();
			  int		match_dir();

			  void	set_remain_attack_delay();
	virtual void	update_abs_pos(SpriteFrame* =0);

			  UCHAR	display_dir();
			  int		need_mirror(UCHAR dispDir);

			  void	set_guard_on();
			  void	set_guard_off();
			  int		is_guarding()     { return guard_count > 0; }
	virtual int		is_shealth();

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();
};
#pragma pack()

//------- Define class SpriteArray ---------//

class SpriteArray : public DynArrayB
{
public:
	short		restart_recno; // indicate the unit's sprite_recno to process first in next process_cycle
	
	SpriteArray(int initArraySize);
	~SpriteArray();

	void	init();
	void  deinit();

	void 	add(Sprite*);
	void 	add_sorted(Sprite*);
	void 	del(int);

	virtual void die(int spriteRecno)		{ del(spriteRecno); }

	virtual void process();

	#ifdef DEBUG
		Sprite* operator[](int recNo);
	#else
		Sprite* operator[](int recNo)   { return (Sprite*) get_ptr(recNo); }
	#endif

	int   is_deleted(int recNo)	 { return get_ptr(recNo) == NULL; }
};

//------------------------------------------//
extern unsigned long	last_unit_ai_profile_time;
extern unsigned long	unit_ai_profile_time;
extern unsigned long	last_unit_profile_time;
extern unsigned long	unit_profile_time;
extern unsigned long	last_sprite_array_profile_time;
extern unsigned long	sprite_array_profile_time;
extern unsigned long last_sprite_idle_profile_time;
extern unsigned long sprite_idle_profile_time;
extern unsigned long last_sprite_move_profile_time;
extern unsigned long sprite_move_profile_time;
extern unsigned long last_sprite_wait_profile_time;
extern unsigned long sprite_wait_profile_time;
extern unsigned long last_sprite_attack_profile_time;
extern unsigned long sprite_attack_profile_time;
extern unsigned long last_unit_attack_profile_time;
extern unsigned long unit_attack_profile_time;
extern unsigned long last_unit_assign_profile_time;
extern unsigned long unit_assign_profile_time;

#endif
