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

//Filename    : OSPRITEA.CPP
//Description : Object SpriteArray

#include <ALL.h>
#include <OVGA.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OSPRTRES.h>
#include <OSPRITE.h>
#include <OUNIT.h>

#ifdef DEBUG
#include <OBULLET.h>
#include <OTORNADO.h>

//### begin alex 3/10 ###//
static int num_of_unit;
//#### end alex 3/10 ####//
#endif

//--------- Begin of function SpriteArray::SpriteArray ---------//
//
// <int> initArraySize - the initial size of this array.
//
SpriteArray::SpriteArray(int initArraySize) : DynArrayB(sizeof(Sprite*),initArraySize, DEFAULT_REUSE_INTERVAL_DAYS)
{
}
//----------- End of function SpriteArray::SpriteArray ---------//


//--------- Begin of function SpriteArray::~SpriteArray ---------//
//
SpriteArray::~SpriteArray()
{
	deinit();
}
//----------- End of function SpriteArray::~SpriteArray ---------//


//--------- Begin of function SpriteArray::init ---------//
//
void SpriteArray::init()
{
	restart_recno = 1;
}
//----------- End of function SpriteArray::init ---------//


//--------- Begin of function SpriteArray::deinit ---------//
//
// All firms should be deleted when the system terminated
//
// Chain : delete nation --> delete firm --> delete job --> delete item
//
// Some data is corrupted when the some firms still exist when
// the system terminated
//
void SpriteArray::deinit()
{
	if( size()==0 )
		return;

	//----- delete sprite objects ------//

	Sprite* emptyPtr = NULL;
	Sprite* spritePtr;

	for( int i=1 ; i<=size() ; i++ )
	{
		spritePtr = (Sprite*) get_ptr(i);

		if( spritePtr )
			delete spritePtr;

		update( &emptyPtr, i );		// set the pointer in SpriteArray to NULL, so is_deleted() can return correct result, this is needed as Unit::deinit() will call is_deleted()
	}

	//-------- zap the array -----------//

	zap();
}
//----------- End of function SpriteArray::deinit ---------//


//--------- Begin of function SpriteArray::add ---------//

void SpriteArray::add(Sprite *newSprite)
{
	linkin(&newSprite);

	newSprite->sprite_recno = recno();
}
//----------- End of function SpriteArray::add -----------//


//--------- Begin of function SpriteArray::add_sorted ---------//
//
// Add the sprite into the array in a sorted order.
//
// <Sprite*> spritePtr - pointer to the sprite to be added
//
// Note: it does not call Sprite::init_recno() as it is supposed to be used by disp_sprite_array
//		   only and sprites to be added are existing sprites only.
//
// return : <int> - the recno of the newly added sprite in SpriteArray.
//
void SpriteArray::add_sorted(Sprite *newSprite)
{
	int l=0, r=size(), x=0;
	int addY  = newSprite->abs_y2;
	int testY = addY + 1;

	//---------------------------------------------------------------------//
	// Use a binary search to find the right location to add the new sprite.
	//---------------------------------------------------------------------//

	while (r > l)
	{
	  x = (l + r) / 2;

	  testY = operator[](x+1)->abs_y2;   // the one to be compared with the adding one.

	  if (addY < testY)
		  r = x;
	  else
		  l = x + 1;

	  if (addY == testY)
		  break;
	}

	if (addY >= testY)
		x++;

	insert_at(x+1, &newSprite);
}
//----------- End of function SpriteArray::add_sorted -----------//


//--------- Begin of function SpriteArray::del ---------//

void SpriteArray::del(int recNo)
{
	Sprite* spritePtr = (Sprite*) get_ptr(recNo);

	err_when( !spritePtr );

	delete spritePtr;

	linkout(recNo);
}
//----------- End of function SpriteArray::del -----------//


//--------- Begin of function SpriteArray::process ---------//
void SpriteArray::process()
{
	#define SYS_YIELD_INTERVAL	20
	
	Sprite* spritePtr;
	int arraySize = size();
	if(arraySize<1)
		return;	// no unit for process

	int i = restart_recno;
	//unit_search_node_used = 0;	// reset unit_search_node_used for each process
	//### begin alex 3/10 ###//
	//int oldRecno = restart_recno;
	//restart_recno = 0;
	int newRecno = 0;
	//#### end alex 3/10 ####//
	int sysYieldCount = arraySize - arraySize%SYS_YIELD_INTERVAL;

	//### begin alex 3/10 ###//
	#ifdef DEBUG
		num_of_unit = 0;
	#endif
	//#### end alex 3/10 ####//

	for(int j=arraySize; j; --j, ++i) //for(int j=1; j<=arraySize; j++, i++)
	{
		//-------- system yield ---------//
		if(j==sysYieldCount)
		{
			sysYieldCount -= SYS_YIELD_INTERVAL;
			sys.yield();
		}

		if(i>arraySize)
			i = 1;

		spritePtr = (Sprite*)get_ptr(i);
		if(!spritePtr)
			continue;

		if(spritePtr->remain_attack_delay)
			spritePtr->remain_attack_delay--;

		if(spritePtr->cur_x==-1) // cur_x == -1 if the unit has removed from the map and gone into a firm
			continue;

		//### begin alex 3/10 ###//
		#ifdef DEBUG
			num_of_unit++;
		#endif
		//#### end alex 3/10 ####//

		//------- process sprite --------//
		err_when(!spritePtr->sprite_info->need_turning && spritePtr->cur_dir!=spritePtr->final_dir);

		#ifdef DEBUG
			unsigned long profileStartTime = misc.get_time();
		#endif

		spritePtr->pre_process();		// it's actually calling Unit::pre_process() and other derived Unit classes

		#ifdef DEBUG
			unit_profile_time += misc.get_time() - profileStartTime;
		#endif

		//-----------------------------------------------------//
		// note: for unit cur_x == -1, the unit is invisible and
		//			no pre_process is done.
		//
		//			for unit cur_x == -2, eg caravan, the unit is
		//			invisible but pre_process is still processed.
		//			However, sprite cur_action should be skipped.
		//-----------------------------------------------------//
		//if( spritePtr->cur_x == -1 )

		if( get_ptr(i)==NULL )		// in case pre_process() kills the current Sprite
			continue;

		if(spritePtr->cur_x<0) //if( spritePtr->cur_x == -1 || spritePtr->cur_x==-2)
			continue;

		#ifdef DEBUG
			long startTime;
		#endif

		#ifdef DEBUG
			profileStartTime = misc.get_time();
		#endif

		switch(spritePtr->cur_action)
		{
			case SPRITE_IDLE:
				#ifdef DEBUG
					startTime = misc.get_time();
				#endif
				spritePtr->process_idle();
				#ifdef DEBUG
					sprite_idle_profile_time += misc.get_time() - startTime;
				#endif
				break;

			case SPRITE_READY_TO_MOVE:
				spritePtr->cur_action = SPRITE_IDLE; // to avoid problems of insensitive of mouse cursor
				spritePtr->process_idle();
				break;

			case SPRITE_MOVE:
				#ifdef DEBUG
					startTime = misc.get_time();
				#endif
				spritePtr->process_move();
				#ifdef DEBUG
					sprite_move_profile_time += misc.get_time() - startTime;
				#endif
				break;

			case SPRITE_WAIT:
				#ifdef DEBUG
					startTime = misc.get_time();
				#endif
				spritePtr->process_wait();
				#ifdef DEBUG
					sprite_wait_profile_time += misc.get_time() - startTime;
				#endif
				break;

			case SPRITE_ATTACK:
				#ifdef DEBUG
					startTime = misc.get_time();
				#endif
				spritePtr->process_attack();
				#ifdef DEBUG
					sprite_attack_profile_time += misc.get_time() - startTime;
				#endif
				break;

			case SPRITE_TURN:
				spritePtr->process_turn();
				break;

			case SPRITE_SHIP_EXTRA_MOVE: // for ship only
				spritePtr->process_extra_move();
				break;	// do nothing

			case SPRITE_DIE:
				if( spritePtr->process_die() )
				{
					die(i);
					spritePtr = NULL;
				}
				break;
		}

		#ifdef DEBUG
			sprite_array_profile_time += misc.get_time() - profileStartTime;
		#endif

		//----- can use other reasonable value to replace MIN_BACKGROUND_NODE_USED_UP ----//
		//### begin alex 3/10 ###//
		//if(!restart_recno && seek_path.total_node_avail<MIN_BACKGROUND_NODE_USED_UP)
		//	restart_recno = i+1;
		if(!newRecno && seek_path.total_node_avail<MIN_BACKGROUND_NODE_USED_UP)
			newRecno = i+1;
		//#### end alex 3/10 ####//

		if(!is_deleted(i) && spritePtr)
		{
			if(spritePtr->guard_count > 0)
			{
				if(++spritePtr->guard_count > GUARD_COUNT_MAX )
				spritePtr->guard_count = 0;
			}
		}
	}

	//### begin alex 3/10 ###//
	//if(!restart_recno)
	//	restart_recno = oldRecno;
	if(newRecno)
		restart_recno = newRecno;
	//#### end alex 3/10 ####//

	#ifdef DEBUG
	if(this==&unit_array || this==&bullet_array || this==&tornado_array)
		misc.set_random_seed(misc.get_random_seed() + restart_recno);
	#endif
}
//----------- End of function SpriteArray::process -----------//

#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS

//------- Begin of function SpriteArray::operator[] -----//

Sprite* SpriteArray::operator[](int recNo)
{
	Sprite* spritePtr = (Sprite*) get_ptr(recNo);

	if( !spritePtr )
		err.run( "SpriteArray[] is deleted" );

	return spritePtr;
}

//--------- End of function SpriteArray::operator[] ----//

#endif
