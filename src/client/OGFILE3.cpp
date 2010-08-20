/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
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

//Filename    : OGFILE3.CPP
//Description : Object Game file, save game and restore game, part 3

#include <OUNIT.h>

#include <OBULLET.h>
#include <OB_PROJ.h>
#include <OFIRM.h>
#include <OGFILE.h>
#include <ONATION.h>
#include <ONEWS.h>
#include <OREBEL.h>
#include <OREGION.h>
#include <OREGIONS.h>
#include <OSITE.h>
#include <OSNOWG.h>
#include <OSPY.h>
#include <OTORNADO.h>
#include <OTOWN.h>
#include <OU_MARI.h>
#include <dbglog.h>
#include <file_io_visitor.h>
#include <file_reader.h>

#include <OGF_V1.h>

DBGLOG_DEFAULT_CHANNEL(GameFile);

//------- declare static functions -------//

static char* create_monster_func();
static char* create_rebel_func();

static void write_ai_info(File* filePtr, short* aiInfoArray, short aiInfoCount, short aiInfoSize);
static void read_ai_info(File* filePtr, short** aiInfoArrayPtr, short& aiInfoCount, short& aiInfoSize);


//-------- Start of function UnitArray::write_file -------------//
//
int UnitArray::write_file(File* filePtr)
{
   int  i;
   Unit *unitPtr;

	filePtr->file_put_short(restart_recno);  // variable in SpriteArray

	filePtr->file_put_short( size()  );  // no. of units in unit_array

	filePtr->file_put_short( selected_recno );
	filePtr->file_put_short( selected_count );
	filePtr->file_put_long ( cur_group_id   );
	filePtr->file_put_long ( cur_team_id    );
	filePtr->file_put_short(idle_blocked_unit_reset_count);
	filePtr->file_put_long (unit_search_tries);
	filePtr->file_put_short(unit_search_tries_flag);

	filePtr->file_put_short(visible_unit_count);
	filePtr->file_put_short(mp_first_frame_to_select_caravan);
	filePtr->file_put_short(mp_first_frame_to_select_ship);
	filePtr->file_put_short(mp_pre_selected_caravan_recno);
	filePtr->file_put_short(mp_pre_selected_ship_recno);

	for( i=1; i<=size() ; i++ )
   {
      unitPtr = (Unit*) get_ptr(i);

      //----- write unitId or 0 if the unit is deleted -----//

      if( !unitPtr )    // the unit is deleted
      {
         filePtr->file_put_short(0);
      }
      else
      {
         //--------- write unit_id -------------//

         filePtr->file_put_short(unitPtr->unit_id);

         //------ write data in the base class ------//

         if( !unitPtr->write_file(filePtr) )
            return 0;

         //------ write data in the derived class ------//

         if( !unitPtr->write_derived_file(filePtr) )
				return 0;
      }
   }

   //------- write empty room array --------//

   write_empty_room(filePtr);

   return 1;
}
//--------- End of function UnitArray::write_file ---------------//


//-------- Start of function UnitArray::read_file -------------//
//
int UnitArray::read_file(File* filePtr)
{
	Unit*   unitPtr;
	int     i, unitId, emptyRoomCount=0;

	restart_recno    = filePtr->file_get_short();

	int unitCount    = filePtr->file_get_short();  // get no. of units from file

	selected_recno   = filePtr->file_get_short();
	selected_count   = filePtr->file_get_short();
	cur_group_id     = filePtr->file_get_long();
	cur_team_id      = filePtr->file_get_long();
	idle_blocked_unit_reset_count = filePtr->file_get_short();
	unit_search_tries	= filePtr->file_get_long ();
	unit_search_tries_flag = (char) filePtr->file_get_short();

   visible_unit_count					= filePtr->file_get_short();
	mp_first_frame_to_select_caravan = (char) filePtr->file_get_short();
	mp_first_frame_to_select_ship		= (char) filePtr->file_get_short();
	mp_pre_selected_caravan_recno		= filePtr->file_get_short();
	mp_pre_selected_ship_recno			= filePtr->file_get_short();

   for( i=1 ; i<=unitCount ; i++ )
   {
      unitId = filePtr->file_get_short();

      if( unitId==0 )  // the unit has been deleted
      {
         add_blank(1);     // it's a DynArrayB function
         emptyRoomCount++;
      }
      else
      {
         //----- create unit object -----------//

			unitPtr = create_unit( unitId );
         unitPtr->unit_id = unitId;

         //---- read data in base class -----//

         if( !unitPtr->read_file( filePtr ) )
            return 0;

         //----- read data in derived class -----//

         if( !unitPtr->read_derived_file( filePtr ) )
            return 0;
      }
   }

	//-------- linkout() those record added by add_blank() ----------//
   //-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

   for( i=size() ; i>0 ; i-- )
	{
		DynArrayB::go(i);             // since UnitArray has its own go() which will call GroupArray::go()

      if( get_ptr() == NULL )       // add_blank() record
         linkout();
   }

   //------- read empty room array --------//

   read_empty_room(filePtr);

   //------- verify the empty_room_array loading -----//

#ifdef DEBUG
   err_when( empty_room_count != emptyRoomCount );

   for( i=0 ; i<empty_room_count ; i++ )
   {
      if( !is_deleted( empty_room_array[i].recno ) )
         err_here();
   }
#endif

   return 1;
}
//--------- End of function UnitArray::read_file ---------------//


//--------- Begin of function Unit::write_file ---------//
//
// Write data in derived class.
//
// If the derived Unit don't have any special data,
// just use Unit::write_file(), otherwise make its own derived copy of write_file()
//
int Unit::write_file(File* filePtr)
{
   if( !filePtr->file_write( this, sizeof(Unit) ) )
		return 0;

   //--------------- write memory data ----------------//

	if( result_node_array )
	{
		if( !filePtr->file_write( result_node_array, sizeof(ResultNode) * result_node_count ) )
			return 0;
	}

	//### begin alex 15/10 ###//
	if(way_point_array)
	{
		err_when(way_point_array_size==0 || way_point_array_size<way_point_count);
		if(!filePtr->file_write(way_point_array, sizeof(ResultNode)*way_point_array_size))
			return 0;
	}
	//#### end alex 15/10 ####//

	if( team_info )
	{
		if( !filePtr->file_write( team_info, sizeof(TeamInfo) ) )
			return 0;
   }

   return 1;
}
//----------- End of function Unit::write_file ---------//

static void read_sprite(FileReader *r, Sprite *s)
{
	r->skip(4); /* virtual table pointer */

	r->read<int16_t>(&s->sprite_id);
	r->read<int16_t>(&s->sprite_recno);
	r->read<int8_t>(&s->mobile_type);
	r->read<uint8_t>(&s->cur_action);
	r->read<uint8_t>(&s->cur_dir);
	r->read<uint8_t>(&s->cur_frame);
	r->read<uint8_t>(&s->cur_attack);
	r->read<uint8_t>(&s->final_dir);
	r->read<int8_t>(&s->turn_delay);
	r->read<int8_t>(&s->guard_count);
	r->read<uint8_t>(&s->remain_attack_delay);
	r->read<uint8_t>(&s->remain_frames_per_step);
	r->read<int16_t>(&s->cur_x);
	r->read<int16_t>(&s->cur_y);
	r->read<int16_t>(&s->go_x);
	r->read<int16_t>(&s->go_y);
	r->read<int16_t>(&s->next_x);
	r->read<int16_t>(&s->next_y);
	r->read(&s->sprite_info);
}

template <typename FileT, typename MemT, typename Visitor>
static bool visit(Visitor *vis, MemT *val)
{
	return vis->template visit<FileT, MemT>(val);
}

template <typename FileT, typename MemT, typename Visitor>
static bool visit_array(Visitor *vis, MemT *array, size_t len)
{
	return vis->template visit_array<FileT, MemT>(array, len);
}

template <typename T, typename Visitor>
static bool visit_pointer(Visitor *vis, T **ptr)
{
	return vis->template visit(ptr);
}

//--------- Begin of function Unit::read_file ---------//
//
int Unit::read_file(File* filePtr)
{
	FileReader r;

	if (!r.init(filePtr))
		return 0;

	r.skip(2); /* record size */

	/* Sprite */
	read_sprite(&r, this);

	/* Unit */
	r.read<int8_t>(&this->unit_id);
	r.read<int8_t>(&this->rank_id);
	r.read<int8_t>(&this->race_id);
	r.read<int8_t>(&this->nation_recno);
	r.read<int8_t>(&this->ai_unit);
	r.read<uint16_t>(&this->name_id);
	r.read<uint32_t>(&this->unit_group_id);
	r.read<uint32_t>(&this->team_id);
	r.read<int8_t>(&this->selected_flag);
	r.read<int8_t>(&this->group_select_id);
	r.read<int8_t>(&this->waiting_term);
	r.read<int8_t>(&this->blocked_by_member);
	r.read<int8_t>(&this->swapping);
	r.read<int16_t>(&this->leader_unit_recno);
	r.read<int8_t>(&this->action_misc);
	r.read<int16_t>(&this->action_misc_para);
	r.read<int8_t>(&this->action_mode);
	r.read<int16_t>(&this->action_para);
	r.read<int16_t>(&this->action_x_loc);
	r.read<int16_t>(&this->action_y_loc);
	r.read<int8_t>(&this->action_mode2);
	r.read<int16_t>(&this->action_para2);
	r.read<int16_t>(&this->action_x_loc2);
	r.read<int16_t>(&this->action_y_loc2);
	r.read_array<int8_t>(this->blocked_edge, 4);
	r.read<uint8_t>(&this->attack_dir);
	r.read<int16_t>(&this->range_attack_x_loc);
	r.read<int16_t>(&this->range_attack_y_loc);
	r.read<int16_t>(&this->move_to_x_loc);
	r.read<int16_t>(&this->move_to_y_loc);
	r.read<int8_t>(&this->loyalty);
	r.read<int8_t>(&this->target_loyalty);
	r.read<float>(&this->hit_points);
	r.read<int16_t>(&this->max_hit_points);

	r.read<int8_t>(&this->skill.combat_level);
	r.read<int8_t>(&this->skill.skill_id);
	r.read<int8_t>(&this->skill.skill_level);
	r.read<uint8_t>(&this->skill.combat_level_minor);
	r.read<uint8_t>(&this->skill.skill_level_minor);
	r.read<uint8_t>(&this->skill.skill_potential);

	r.read<int8_t>(&this->unit_mode);
	r.read<int16_t>(&this->unit_mode_para);
	r.read<int16_t>(&this->spy_recno);
	r.read<int16_t>(&this->nation_contribution);
	r.read<int16_t>(&this->total_reward);
	r.read(&this->attack_info_array);
	r.read<int8_t>(&this->attack_count);
	r.read<int8_t>(&this->attack_range);
	r.read<int16_t>(&this->cur_power);
	r.read<int16_t>(&this->max_power);
	r.read(&this->result_node_array);
	r.read<int32_t>(&this->result_node_count);
	r.read<int16_t>(&this->result_node_recno);
	r.read<int16_t>(&this->result_path_dist);
	r.read(&this->way_point_array);
	r.read<int16_t>(&this->way_point_array_size);
	r.read<int16_t>(&this->way_point_count);
	r.read<uint16_t>(&this->ai_action_id);
	r.read<int8_t>(&this->original_action_mode);
	r.read<int16_t>(&this->original_action_para);
	r.read<int16_t>(&this->original_action_x_loc);
	r.read<int16_t>(&this->original_action_y_loc);
	r.read<int16_t>(&this->original_target_x_loc);
	r.read<int16_t>(&this->original_target_y_loc);
	r.read<int16_t>(&this->ai_original_target_x_loc);
	r.read<int16_t>(&this->ai_original_target_y_loc);
	r.read<int8_t>(&this->ai_no_suitable_action);
	r.read<int8_t>(&this->can_guard_flag);
	r.read<int8_t>(&this->can_attack_flag);
	r.read<int8_t>(&this->force_move_flag);
	r.read<int16_t>(&this->home_camp_firm_recno);
	r.read<int8_t>(&this->aggressive_mode);
	r.read<int8_t>(&this->seek_path_fail_count);
	r.read<int8_t>(&this->ignore_power_nation);
	r.read(&this->team_info);

	if (!r.good())
		return 0;

	r.deinit();

	//--------------- read in memory data ----------------//

	if( result_node_array )
	{
		result_node_array = (ResultNode*) mem_add( sizeof(ResultNode) * result_node_count );

		if( !filePtr->file_read( result_node_array, sizeof(ResultNode) * result_node_count ) )
			return 0;
	}

	//### begin alex 15/10 ###//
	if(way_point_array)
	{
		way_point_array = (ResultNode*) mem_add(sizeof(ResultNode) * way_point_array_size);

		if(!filePtr->file_read(way_point_array, sizeof(ResultNode)*way_point_array_size))
			return 0;
	}
	//#### end alex 15/10 ####//

	if( team_info )
	{
		team_info = (TeamInfo*) mem_add( sizeof(TeamInfo) );

		if( !filePtr->file_read( team_info, sizeof(TeamInfo) ) )
			return 0;
	}

	//----------- post-process the data read ----------//

	// attack_info_array = unit_res.attack_info_array+unit_res[unit_id]->first_attack-1;
	sprite_info       = sprite_res[sprite_id];

	sprite_info->load_bitmap_res();

	//--------- special process of UNIT_MARINE --------//

	// move to read_derived_file
	//if( unit_res[unit_id]->unit_class == UNIT_CLASS_SHIP )
	//{
	//	((UnitMarine*)this)->splash.sprite_info = sprite_res[sprite_id];
	//	((UnitMarine*)this)->splash.sprite_info->load_bitmap_res();
	//}

   return 1;
}
//----------- End of function Unit::read_file ---------//


//--------- Begin of function Unit::write_derived_file ---------//
//
int Unit::write_derived_file(File* filePtr)
{
   //--- write data in derived class -----//

	int writeSize = unit_array.unit_class_size(unit_id)-sizeof(Unit);

   if( writeSize > 0 )
   {
      if( !filePtr->file_write( (char*) this + sizeof(Unit), writeSize ) )
         return 0;
   }

   return 1;
}
//----------- End of function Unit::write_derived_file ---------//


//--------- Begin of function Unit::read_derived_file ---------//
//
int Unit::read_derived_file(File* filePtr)
{
	//--- read data in derived class -----//

   int readSize = unit_array.unit_class_size(unit_id) - sizeof(Unit);

   if( readSize > 0 )
   {
      if( !filePtr->file_read( (char*) this + sizeof(Unit), readSize ) )
         return 0;
	}

	// ###### begin Gilbert 13/8 #######//
	fix_attack_info();
	// ###### end Gilbert 13/8 #######//

   return 1;
}
//----------- End of function Unit::read_derived_file ---------//

static void read_trade_stop(FileReader *r, TradeStop *ts)
{
	r->read<int16_t>(&ts->firm_recno);
	r->read<int16_t>(&ts->firm_loc_x1);
	r->read<int16_t>(&ts->firm_loc_y1);
	r->read<int8_t>(&ts->pick_up_type);
	r->read_array<int8_t>(ts->pick_up_array, MAX_PICK_UP_GOODS);
}

static void read_attack_info(FileReader *r, AttackInfo *ai)
{
	r->read<uint8_t>(&ai->combat_level);
	r->read<uint8_t>(&ai->attack_delay);
	r->read<uint8_t>(&ai->attack_range);
	r->read<uint8_t>(&ai->attack_damage);
   r->read<uint8_t>(&ai->pierce_damage);
	r->read<int16_t>(&ai->bullet_out_frame);
	r->read<int8_t>(&ai->bullet_speed);
	r->read<int8_t>(&ai->bullet_radius);
	r->read<int8_t>(&ai->bullet_sprite_id);
	r->read<int8_t>(&ai->dll_bullet_sprite_id);
	r->read<int8_t>(&ai->eqv_attack_next);
	r->read<int16_t>(&ai->min_power);
	r->read<int16_t>(&ai->consume_power);
	r->read<int8_t>(&ai->fire_radius);
	r->read<int16_t>(&ai->effect_id);
}

//--------- Begin of function UnitMarine::read_derived_file ---------//
int UnitMarine::read_derived_file(File* filePtr)
{
	FileReader r;

	if (!r.init(filePtr))
		return 0;

	r.skip(2); /* record size */

	read_sprite(&r, &this->splash);
	r.read<int8_t>(&this->menu_mode);
	r.read<int8_t>(&this->extra_move_in_beach);
	r.read<int8_t>(&this->in_beach);
	r.read<int8_t>(&this->selected_unit_id);
	r.read_array<int16_t>(this->unit_recno_array, MAX_UNIT_IN_SHIP);
	r.read<int8_t>(&this->unit_count);
	r.read<int8_t>(&this->journey_status);
	r.read<int8_t>(&this->dest_stop_id);
	r.read<int8_t>(&this->stop_defined_num);
	r.read<int8_t>(&this->wait_count);
	r.read<int16_t>(&this->stop_x_loc);
	r.read<int16_t>(&this->stop_y_loc);
	r.read<int8_t>(&this->auto_mode);
	r.read<int16_t>(&this->cur_firm_recno);
	r.read<int16_t>(&this->carry_goods_capacity);

	for (int n = 0; n < MAX_STOP_FOR_SHIP; n++)
		read_trade_stop(&r, &this->stop_array[n]);

	r.read_array<int16_t>(this->raw_qty_array, MAX_RAW);
	r.read_array<int16_t>(this->product_raw_qty_array, MAX_PRODUCT);
	read_attack_info(&r, &this->ship_attack_info);
	r.read<uint8_t>(&this->attack_mode_selected);
	r.read<int32_t>(&this->last_load_goods_date);

	r.deinit();

	// ------- post-process the data read --------//
	splash.sprite_info = sprite_res[splash.sprite_id];
	splash.sprite_info->load_bitmap_res();

	return 1;
}
//--------- End of function UnitMarine::read_derived_file ---------//


//*****//


//-------- Start of function BulletArray::write_file -------------//
//
int BulletArray::write_file(File* filePtr)
{
	filePtr->file_put_short(restart_recno);  // variable in SpriteArray

	int    i, emptyRoomCount=0;;
	Bullet *bulletPtr;

	filePtr->file_put_short( size() );  // no. of bullets in bullet_array

	for( i=1; i<=size() ; i++ )
	{
		bulletPtr = (Bullet*) get_ptr(i);

		//----- write bulletId or 0 if the bullet is deleted -----//

		if( !bulletPtr )    // the bullet is deleted
		{
			filePtr->file_put_short(0);
			emptyRoomCount++;
		}
		else
		{
			filePtr->file_put_short(bulletPtr->sprite_id);      // there is a bullet in this record

			//------ write data in the base class ------//

			if( !bulletPtr->write_file(filePtr) )
				return 0;

			//------ write data in the derived class -------//

			if( !bulletPtr->write_derived_file(filePtr) )
				return 0;
		}
	}

	//------- write empty room array --------//

	write_empty_room(filePtr);

	//------- verify the empty_room_array loading -----//

#ifdef DEBUG
	err_when( empty_room_count != emptyRoomCount );

   for( i=0 ; i<empty_room_count ; i++ )
   {
		if( !is_deleted( empty_room_array[i].recno ) )
         err_here();
   }
#endif

	return 1;
}
//--------- End of function BulletArray::write_file -------------//


//-------- Start of function BulletArray::read_file -------------//
//
int BulletArray::read_file(File* filePtr)
{
	restart_recno    = filePtr->file_get_short();

	int     i, bulletRecno, bulletCount, emptyRoomCount=0, spriteId;
	Bullet* bulletPtr;

	bulletCount = filePtr->file_get_short();  // get no. of bullets from file

	for( i=1 ; i<=bulletCount ; i++ )
	{
		spriteId = filePtr->file_get_short();
		if( spriteId == 0 )
		{
			add_blank(1);     // it's a DynArrayB function

			emptyRoomCount++;
		}
		else
		{
			//----- create bullet object -----------//

			bulletRecno = create_bullet(spriteId);
			bulletPtr   = bullet_array[bulletRecno];

         //----- read data in base class --------//

         if( !bulletPtr->read_file( filePtr ) )
            return 0;

			//----- read data in derived class -----//

			if( !bulletPtr->read_derived_file( filePtr ) )
				return 0;
      }
	}

   //-------- linkout() those record added by add_blank() ----------//
	//-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

	for( i=1 ; i<=size() ; i++ )
	{
		DynArrayB::go(i);             // since BulletArray has its own go() which will call GroupArray::go()

		if( get_ptr() == NULL )       // add_blank() record
			linkout();
	}

	//------- read empty room array --------//

	read_empty_room(filePtr);

	//------- verify the empty_room_array loading -----//

#ifdef DEBUG
	err_when( empty_room_count != emptyRoomCount );

	for( i=0 ; i<empty_room_count ; i++ )
	{
		if( !is_deleted( empty_room_array[i].recno ) )
			err_here();
	}
#endif

	return 1;
}
//--------- End of function BulletArray::read_file ---------------//


//--------- Begin of function Bullet::write_file ---------//
//
int Bullet::write_file(File* filePtr)
{
	if( !filePtr->file_write( this, sizeof(Bullet) ) )
		return 0;

	return 1;
}
//----------- End of function Bullet::write_file ---------//


//--------- Begin of function Bullet::read_file ---------//
//
int Bullet::read_file(File* filePtr)
{
	FileReader r;

	MSG("Bullet::read_file()\n");

	if (!r.init(filePtr))
		return 0;

	r.skip(2); /* record size */

	read_sprite(&r, this);

	r.read<int8_t>(&this->parent_type);
	r.read<int16_t>(&this->parent_recno);
	r.read<int8_t>(&this->target_mobile_type);
	r.read<float>(&this->attack_damage);
	r.read<int16_t>(&this->damage_radius);
	r.read<int16_t>(&this->nation_recno);
	r.read<int8_t>(&this->fire_radius);
	r.read<int16_t>(&this->origin_x);
	r.read<int16_t>(&this->origin_y);
	r.read<int16_t>(&this->target_x_loc);
	r.read<int16_t>(&this->target_y_loc);
	r.read<int8_t>(&this->cur_step);
	r.read<int8_t>(&this->total_step);

	r.deinit();

   //------------ post-process the data read ----------//

	sprite_info = sprite_res[sprite_id];

	sprite_info->load_bitmap_res();

	return 1;
}
//----------- End of function Bullet::read_file ---------//


//----------- Begin of function Bullet::write_derived_file ---------//
int Bullet::write_derived_file(File *filePtr)
{
	//--- write data in derived class -----//

	int writeSize = bullet_array.bullet_class_size(sprite_id)-sizeof(Bullet);

	if( writeSize > 0 )
	{
		if( !filePtr->file_write( (char*) this + sizeof(Bullet), writeSize ) )
			return 0;
	}

	return 1;

}
//----------- End of function Bullet::write_derived_file ---------//


//----------- Begin of function Bullet::read_derived_file ---------//
int Bullet::read_derived_file(File *filePtr)
{
	//--- read data in derived class -----//

	int readSize = bullet_array.bullet_class_size(sprite_id) - sizeof(Bullet);

	if( readSize > 0 )
	{
		if( !filePtr->file_read( (char*) this + sizeof(Bullet), readSize ) )
			return 0;
	}

	return 1;
}
//----------- End of function Bullet::read_derived_file ---------//


//----------- Begin of function Projectile::read_derived_file ---------//

int Projectile::read_derived_file(File *filePtr)
{
	FileReader r;

	if (!r.init(filePtr))
		return 0;

	r.skip(2); /* record size */

	r.read<float>(&this->z_coff);
	read_sprite(&r, &this->act_bullet);
	read_sprite(&r, &this->bullet_shadow);

	if (!r.good())
		return 0;

	r.deinit();

   //----------- post-process the data read ----------//
	act_bullet.sprite_info = sprite_res[act_bullet.sprite_id];
	act_bullet.sprite_info->load_bitmap_res();
	bullet_shadow.sprite_info = sprite_res[bullet_shadow.sprite_id];
	bullet_shadow.sprite_info->load_bitmap_res();

	return 1;
}
//----------- End of function Projectile::read_derived_file ---------//

//*****//

//-------- Start of function FirmArray::write_file -------------//
//
int FirmArray::write_file(File* filePtr)
{
   int  i;
   Firm *firmPtr;

   filePtr->file_put_short( size()  );  // no. of firms in firm_array
   filePtr->file_put_short( process_recno );
	filePtr->file_put_short( selected_recno );

	filePtr->file_put_short( Firm::firm_menu_mode );
	filePtr->file_put_short( Firm::action_spy_recno );
	filePtr->file_put_short( Firm::bribe_result );
	filePtr->file_put_short( Firm::assassinate_result );

	for( i=1; i<=size() ; i++ )
   {
      firmPtr = (Firm*) get_ptr(i);

      //----- write firmId or 0 if the firm is deleted -----//

      if( !firmPtr )    // the firm is deleted
		{
         filePtr->file_put_short(0);
      }
      else
      {
         //--------- write firm_id -------------//

         filePtr->file_put_short(firmPtr->firm_id);

         //------ write data in base class --------//

			if( !filePtr->file_write( firmPtr, sizeof(Firm) ) )
            return 0;

         //--------- write worker_array ---------//

         if( firmPtr->worker_array )
         {
            if( !filePtr->file_write( firmPtr->worker_array, MAX_WORKER*sizeof(Worker) ) )
               return 0;
         }

         //------ write data in derived class ------//

         if( !firmPtr->write_derived_file(filePtr) )
            return 0;
      }
   }

   //------- write empty room array --------//

	write_empty_room(filePtr);

   return 1;
}
//--------- End of function FirmArray::write_file ---------------//


//-------- Start of function FirmArray::read_file -------------//
//
int FirmArray::read_file(File* filePtr)
{
	Firm*   firmPtr;
	int     i, firmId, firmRecno;

	int firmCount      = filePtr->file_get_short();  // get no. of firms from file
	process_recno      = filePtr->file_get_short();
	selected_recno     = filePtr->file_get_short();

	Firm::firm_menu_mode  	 = (char) filePtr->file_get_short();
	Firm::action_spy_recno   = filePtr->file_get_short();
	Firm::bribe_result    	 = (char) filePtr->file_get_short();
	Firm::assassinate_result = (char) filePtr->file_get_short();

   for( i=1 ; i<=firmCount ; i++ )
   {
      firmId = filePtr->file_get_short();

      if( firmId==0 )  // the firm has been deleted
      {
         add_blank(1);     // it's a DynArrayB function
      }
      else
      {
			FileReader r;

         //----- create firm object -----------//

         firmRecno = create_firm( firmId );
         firmPtr   = firm_array[firmRecno];

			if (!r.init(filePtr))
				return 0;

			r.skip(2); /* record size */
			r.skip(4); /* virtual table pointer */

			r.read<int8_t>(&firmPtr->firm_id);
			r.read<int16_t>(&firmPtr->firm_build_id);
			r.read<int16_t>(&firmPtr->firm_recno);
			r.read<int8_t>(&firmPtr->firm_ai);
			r.read<int8_t>(&firmPtr->ai_processed);
			r.read<int8_t>(&firmPtr->ai_status);
			r.read<int8_t>(&firmPtr->ai_link_checked);
			r.read<int8_t>(&firmPtr->ai_sell_flag);
			r.read<int8_t>(&firmPtr->race_id);
			r.read<int16_t>(&firmPtr->nation_recno);
			r.read<int16_t>(&firmPtr->closest_town_name_id);
			r.read<int16_t>(&firmPtr->firm_name_instance_id);
			r.read<int16_t>(&firmPtr->loc_x1);
			r.read<int16_t>(&firmPtr->loc_y1);
			r.read<int16_t>(&firmPtr->loc_x2);
			r.read<int16_t>(&firmPtr->loc_y2);
			r.read<int16_t>(&firmPtr->abs_x1);
			r.read<int16_t>(&firmPtr->abs_y1);
			r.read<int16_t>(&firmPtr->abs_x2);
			r.read<int16_t>(&firmPtr->abs_y2);
			r.read<int16_t>(&firmPtr->center_x);
			r.read<int16_t>(&firmPtr->center_y);
			r.read<uint8_t>(&firmPtr->region_id);
			r.read<int8_t>(&firmPtr->cur_frame);
			r.read<int8_t>(&firmPtr->remain_frame_delay);
			r.read<float>(&firmPtr->hit_points);
			r.read<float>(&firmPtr->max_hit_points);
			r.read<int8_t>(&firmPtr->under_construction);
			r.read<int8_t>(&firmPtr->firm_skill_id);
			r.read<int16_t>(&firmPtr->overseer_recno);
			r.read<int16_t>(&firmPtr->overseer_town_recno);
			r.read<int16_t>(&firmPtr->builder_recno);
			r.read<uint8_t>(&firmPtr->builder_region_id);
			r.read<float>(&firmPtr->productivity);
			r.read(&firmPtr->worker_array);
			r.read<int8_t>(&firmPtr->worker_count);
			r.read<int8_t>(&firmPtr->selected_worker_id);
			r.read<int8_t>(&firmPtr->player_spy_count);
			r.read<uint8_t>(&firmPtr->sabotage_level);
			r.read<int8_t>(&firmPtr->linked_firm_count);
			r.read<int8_t>(&firmPtr->linked_town_count);
			r.read_array<int16_t>(firmPtr->linked_firm_array, MAX_LINKED_FIRM_FIRM);
			r.read_array<int16_t>(firmPtr->linked_town_array, MAX_LINKED_FIRM_TOWN);
			r.read_array<int8_t>(firmPtr->linked_firm_enable_array, MAX_LINKED_FIRM_FIRM);
			r.read_array<int8_t>(firmPtr->linked_town_enable_array, MAX_LINKED_FIRM_TOWN);
			r.read<float>(&firmPtr->last_year_income);
			r.read<float>(&firmPtr->cur_year_income);
			r.read<int32_t>(&firmPtr->setup_date);
			r.read<int8_t>(&firmPtr->should_set_power);
			r.read<int32_t>(&firmPtr->last_attacked_date);
			r.read<int8_t>(&firmPtr->should_close_flag);
			r.read<int8_t>(&firmPtr->no_neighbor_space);
			r.read<int8_t>(&firmPtr->ai_should_build_factory_count);

			if (!r.good())
				return 0;

			r.deinit();

         //---- read data in base class -----//

			#ifdef AMPLUS
				if(!game_file_array.same_version && firmPtr->firm_id > FIRM_BASE)
					firmPtr->firm_build_id += MAX_RACE - VERSION_1_MAX_RACE;
			#endif

         //--------- read worker_array ---------//

         if( firm_res[firmId]->need_worker )
         {
            firmPtr->worker_array = (Worker*) mem_add( MAX_WORKER*sizeof(Worker) );

            if( !filePtr->file_read( firmPtr->worker_array, MAX_WORKER*sizeof(Worker) ) )
               return 0;
         }

         //----- read data in derived class -----//

         if( !firmPtr->read_derived_file( filePtr ) )
            return 0;
      }
   }

   //-------- linkout() those record added by add_blank() ----------//
   //-- So they will be marked deleted in DynArrayB and can be -----//
   //-- undeleted and used when a new record is going to be added --//

   for( i=size() ; i>0 ; i-- )
   {
      DynArrayB::go(i);             // since FirmArray has its own go() which will call GroupArray::go()

      if( get_ptr() == NULL )       // add_blank() record
         linkout();
   }

   //------- read empty room array --------//

   read_empty_room(filePtr);

   return 1;
}
//--------- End of function FirmArray::read_file ---------------//


//--------- Begin of function Firm::write_derived_file ---------//
//
// Write data in derived class.
//
// If the derived Firm don't have any special data,
// just use Firm::write_file(), otherwise make its own derived copy of write_file()
//
int Firm::write_derived_file(File* filePtr)
{
   //--- write data in derived class -----//

   int writeSize = firm_array.firm_class_size(firm_id)-sizeof(Firm);

   if( writeSize > 0 )
   {
      if( !filePtr->file_write( (char*) this + sizeof(Firm), writeSize ) )
         return 0;
   }

   return 1;
}
//----------- End of function Firm::write_derived_file ---------//


//--------- Begin of function Firm::read_derived_file ---------//
//
// Read data in derived class.
//
// If the derived Firm don't have any special data,
// just use Firm::read_file(), otherwise make its own derived copy of read_file()
//
int Firm::read_derived_file(File* filePtr)
{
   //--- read data in derived class -----//

   int readSize = firm_array.firm_class_size(firm_id)-sizeof(Firm);

   if( readSize > 0 )
   {
		MSG(__FILE__":%d: file_read(this, ...);\n", __LINE__);

      if( !filePtr->file_read( (char*) this + sizeof(Firm), readSize ) )
         return 0;
   }

   return 1;
}
//----------- End of function Firm::read_derived_file ---------//


//*****//


//-------- Start of function SiteArray::write_file -------------//
//
int SiteArray::write_file(File* filePtr)
{
	filePtr->file_put_short(selected_recno);
	filePtr->file_put_short(untapped_raw_count);
	filePtr->file_put_short(scroll_count);
	filePtr->file_put_short(gold_coin_count);
	filePtr->file_put_short(std_raw_site_count);

	return DynArrayB::write_file( filePtr );
}
//--------- End of function SiteArray::write_file ---------------//


//-------- Start of function SiteArray::read_file -------------//
//
int SiteArray::read_file(File* filePtr)
{
	selected_recno		 = filePtr->file_get_short();
	untapped_raw_count =	filePtr->file_get_short();
	scroll_count		 = filePtr->file_get_short();
	gold_coin_count	 =	filePtr->file_get_short();
	std_raw_site_count =	filePtr->file_get_short();

	return DynArrayB::read_file( filePtr );
}
//--------- End of function SiteArray::read_file ---------------//


//*****//


//-------- Start of function TownArray::write_file -------------//
//
int TownArray::write_file(File* filePtr)
{
   int  i;
   Town *townPtr;

	filePtr->file_put_short( size()  );  // no. of towns in town_array
	filePtr->file_put_short( selected_recno );
	filePtr->file_write( race_wander_pop_array, sizeof(race_wander_pop_array) );

	filePtr->file_put_short( Town::if_town_recno );

	//-----------------------------------------//

	for( i=1; i<=size() ; i++ )
	{
		townPtr = (Town*) get_ptr(i);

      //----- write townId or 0 if the town is deleted -----//

      if( !townPtr )    // the town is deleted
      {
         filePtr->file_put_short(0);
      }
      else
		{
			#ifdef DEBUG
				townPtr->verify_slot_object_id_array();		// for debugging only
			#endif

			filePtr->file_put_short(1);      // the town exists

         if( !filePtr->file_write( townPtr, sizeof(Town) ) )
            return 0;
      }
   }

   //------- write empty room array --------//

   write_empty_room(filePtr);

   return 1;
}
//--------- End of function TownArray::write_file ---------------//


//-------- Start of function TownArray::read_file -------------//
//
int TownArray::read_file(File* filePtr)
{
   Town*   townPtr;
   int     i;

	int townCount = filePtr->file_get_short();  // get no. of towns from file
	selected_recno = filePtr->file_get_short();

#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		memset(race_wander_pop_array, 0, sizeof(race_wander_pop_array));
		filePtr->file_read( race_wander_pop_array, sizeof(race_wander_pop_array[0])*VERSION_1_MAX_RACE );
	}
	else
		filePtr->file_read( race_wander_pop_array, sizeof(race_wander_pop_array) );
#else
	filePtr->file_read( race_wander_pop_array, sizeof(race_wander_pop_array) );
#endif

	Town::if_town_recno = filePtr->file_get_short();

	//------------------------------------------//

	for( i=1 ; i<=townCount ; i++ )
	{
		if( filePtr->file_get_short()==0 )  // the town has been deleted
		{
			add_blank(1);     // it's a DynArrayB function
		}
		else
		{
			townPtr = town_array.create_town();

			#ifdef AMPLUS
				if(!game_file_array.same_version)
				{
					Version_1_Town *oldTown = (Version_1_Town*) mem_add(sizeof(Version_1_Town));
					if(!filePtr->file_read(oldTown, sizeof(Version_1_Town)))
					{
						mem_del(oldTown);
						return 0;
					}

					oldTown->convert_to_version_2(townPtr);
					mem_del(oldTown);
				}
				else
				{
					if( !filePtr->file_read( townPtr, sizeof(Town) ) )
						return 0;
				}
			#else
				if( !filePtr->file_read( townPtr, sizeof(Town) ) )
					return 0;
			#endif

			#ifdef DEBUG
				townPtr->verify_slot_object_id_array();		// for debugging only
			#endif
		}
	}

	//-------- linkout() those record added by add_blank() ----------//
	//-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

   for( i=size() ; i>0 ; i-- )
   {
      DynArrayB::go(i);             // since TownArray has its own go() which will call GroupArray::go()

      if( get_ptr() == NULL )       // add_blank() record
         linkout();
   }

   //------- read empty room array --------//

   read_empty_room(filePtr);

   return 1;
}
//--------- End of function TownArray::read_file ---------------//


//*****//


//-------- Start of function NationArray::write_file -------------//
//
int NationArray::write_file(File* filePtr)
{
	//------ write info in NationArray ------//

   if( !filePtr->file_write( (char*) this + sizeof(DynArrayB), sizeof(NationArray)-sizeof(DynArrayB) ) )
      return 0;

   //---------- write Nations --------------//

   int    i;
   Nation *nationPtr;

   filePtr->file_put_short( size() );  // no. of nations in nation_array

   for( i=1; i<=size() ; i++ )
   {
      nationPtr = (Nation*) get_ptr(i);

      //----- write nationId or 0 if the nation is deleted -----//

      if( !nationPtr )    // the nation is deleted
      {
         filePtr->file_put_short(0);
      }
      else
      {
         filePtr->file_put_short(1);      // there is a nation in this record

         //------ write data in the base class ------//

         if( !nationPtr->write_file(filePtr) )
            return 0;
      }
   }

   //------- write empty room array --------//

   write_empty_room(filePtr);

   return 1;
}
//--------- End of function NationArray::write_file -------------//

static bool read_version_1_nation_array(File *file, Version_1_NationArray *na)
{
	FileReader r;

	if (!r.init(file))
		return false;

	r.skip(2); /* record size */

	r.read<int16_t>(&na->nation_count);
	r.read<int16_t>(&na->ai_nation_count);
	r.read<int32_t>(&na->last_del_nation_date);
	r.read<int32_t>(&na->last_new_nation_date);
	r.read<int32_t>(&na->max_nation_population);
	r.read<int32_t>(&na->all_nation_population);
	r.read<int16_t>(&na->independent_town_count);
	r.read_array<int16_t>(na->independent_town_count_race_array, VERSION_1_MAX_RACE);
	r.read<int32_t>(&na->max_nation_units);
	r.read<int32_t>(&na->max_nation_humans);
	r.read<int32_t>(&na->max_nation_generals);
	r.read<int32_t>(&na->max_nation_weapons);
	r.read<int32_t>(&na->max_nation_ships);
	r.read<int32_t>(&na->max_nation_spies);
	r.read<int32_t>(&na->max_nation_firms);
	r.read<int32_t>(&na->max_nation_tech_level);
	r.read<int32_t>(&na->max_population_rating);
	r.read<int32_t>(&na->max_military_rating);
	r.read<int32_t>(&na->max_economic_rating);
	r.read<int32_t>(&na->max_reputation);
	r.read<int32_t>(&na->max_kill_monster_score);
	r.read<int32_t>(&na->max_overall_rating);
	r.read<int16_t>(&na->max_population_nation_recno);
	r.read<int16_t>(&na->max_military_nation_recno);
	r.read<int16_t>(&na->max_economic_nation_recno);
	r.read<int16_t>(&na->max_reputation_nation_recno);
	r.read<int16_t>(&na->max_kill_monster_nation_recno);
	r.read<int16_t>(&na->max_overall_nation_recno);
	r.read<int32_t>(&na->last_alliance_id);
	r.read<int32_t>(&na->nation_peace_days);
	r.read<int16_t>(&na->player_recno);
	r.read(&na->player_ptr);
	r.read_array<int8_t>(na->nation_color_array, MAX_NATION+1);
	r.read_array<int8_t>(na->nation_power_color_array, MAX_NATION+2);

	for (int n = 0; n < MAX_NATION; n++)
		r.read_array<int8_t>(na->human_name_array[n], NationArray::HUMAN_NAME_LEN+1);

	return r.good();
}

template <typename Visitor>
static void visit_nation_array(Visitor *v, NationArray *na)
{
	v->skip(2); /* record size */

	/* DynArray and DynArrayB skipped */

	visit<int16_t>(v, &na->nation_count);
	visit<int16_t>(v, &na->ai_nation_count);
	visit<int32_t>(v, &na->last_del_nation_date);
	visit<int32_t>(v, &na->last_new_nation_date);
	visit<int32_t>(v, &na->max_nation_population);
	visit<int32_t>(v, &na->all_nation_population);
   visit<int16_t>(v, &na->independent_town_count);
	visit_array<int16_t>(v, na->independent_town_count_race_array, MAX_RACE);
	visit<int32_t>(v, &na->max_nation_units);
	visit<int32_t>(v, &na->max_nation_humans);
	visit<int32_t>(v, &na->max_nation_generals);
	visit<int32_t>(v, &na->max_nation_weapons);
	visit<int32_t>(v, &na->max_nation_ships);
	visit<int32_t>(v, &na->max_nation_spies);
	visit<int32_t>(v, &na->max_nation_firms);
	visit<int32_t>(v, &na->max_nation_tech_level);
	visit<int32_t>(v, &na->max_population_rating);
	visit<int32_t>(v, &na->max_military_rating);
	visit<int32_t>(v, &na->max_economic_rating);
	visit<int32_t>(v, &na->max_reputation);
	visit<int32_t>(v, &na->max_kill_monster_score);
	visit<int32_t>(v, &na->max_overall_rating);
	visit<int16_t>(v, &na->max_population_nation_recno);
	visit<int16_t>(v, &na->max_military_nation_recno);
	visit<int16_t>(v, &na->max_economic_nation_recno);
	visit<int16_t>(v, &na->max_reputation_nation_recno);
	visit<int16_t>(v, &na->max_kill_monster_nation_recno);
	visit<int16_t>(v, &na->max_overall_nation_recno);
	visit<int32_t>(v, &na->last_alliance_id);
	visit<int32_t>(v, &na->nation_peace_days);
	visit<int16_t>(v, &na->player_recno);
	visit_pointer(v, &na->player_ptr);
	visit_array<int8_t>(v, na->nation_color_array, MAX_NATION+1);
	visit_array<int8_t>(v, na->nation_power_color_array, MAX_NATION+2);

	for (int n = 0; n < MAX_NATION; n++)
		visit_array<int8_t>(v, na->human_name_array[n], NationArray::HUMAN_NAME_LEN+1);
}

static bool read_nation_array(File *file, NationArray *na)
{
	FileReader r;
	FileReaderVisitor v(&r);

	if (!r.init(file))
		return false;

	visit_nation_array(&v, na);

	return r.good();
}

//-------- Start of function NationArray::read_file -------------//
//
int NationArray::read_file(File* filePtr)
{
   //------ read info in NationArray ------//
#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		Version_1_NationArray *oldNationArrayPtr = (Version_1_NationArray*) mem_add(sizeof(Version_1_NationArray));
		if (!read_version_1_nation_array(filePtr, oldNationArrayPtr))
		{
			mem_del(oldNationArrayPtr);
			return 0;
		}
		oldNationArrayPtr->convert_to_version_2(this);
		mem_del(oldNationArrayPtr);
	}
	else
	{
		if (!read_nation_array(filePtr, this))
			return 0;
	}
#else
	if (!read_nation_array(filePtr, this))
      return 0;
#endif

   //---------- read Nations --------------//

   int     i, nationRecno, nationCount;
   Nation* nationPtr;

   nationCount = filePtr->file_get_short();  // get no. of nations from file

   for( i=1 ; i<=nationCount ; i++ )
   {
      if( filePtr->file_get_short() == 0 )
      {
         add_blank(1);     // it's a DynArrayB function
      }
      else
      {
         //----- create nation object -----------//

         nationRecno = create_nation();
         nationPtr   = nation_array[nationRecno];

         //----- read data in base class --------//

         if( !nationPtr->read_file( filePtr ) )
            return 0;
      }
   }

   //-------- linkout() those record added by add_blank() ----------//
   //-- So they will be marked deleted in DynArrayB and can be -----//
   //-- undeleted and used when a new record is going to be added --//

   for( i=size() ; i>0 ; i-- )
   {
      DynArrayB::go(i);             // since NationArray has its own go() which will call GroupArray::go()

      if( get_ptr() == NULL )       // add_blank() record
         linkout();
   }

	//-------- set NationArray::player_ptr -----------//

   player_ptr = nation_array[player_recno];

	//------- read empty room array --------//

	read_empty_room(filePtr);

	return 1;
}
//--------- End of function NationArray::read_file ---------------//


//--------- Begin of function Nation::write_file ---------//
//
int Nation::write_file(File* filePtr)
{
	if( !filePtr->file_write( this, sizeof(Nation) ) )
		return 0;

	//----------- write AI Action Array ------------//

	action_array.write_file(filePtr);

	//------ write AI info array ---------//

	write_ai_info(filePtr, ai_town_array, ai_town_count, ai_town_size);

	write_ai_info(filePtr, ai_base_array, ai_base_count, ai_base_size);
	write_ai_info(filePtr, ai_mine_array, ai_mine_count, ai_mine_size);
	write_ai_info(filePtr, ai_factory_array, ai_factory_count, ai_factory_size);
	write_ai_info(filePtr, ai_market_array, ai_market_count, ai_market_size);
	write_ai_info(filePtr, ai_inn_array, ai_inn_count, ai_inn_size);
	write_ai_info(filePtr, ai_camp_array, ai_camp_count, ai_camp_size);
	write_ai_info(filePtr, ai_research_array, ai_research_count, ai_research_size);
	write_ai_info(filePtr, ai_war_array, ai_war_count, ai_war_size);
	write_ai_info(filePtr, ai_harbor_array, ai_harbor_count, ai_harbor_size);

	write_ai_info(filePtr, ai_caravan_array, ai_caravan_count, ai_caravan_size);
	write_ai_info(filePtr, ai_ship_array, ai_ship_count, ai_ship_size);
	write_ai_info(filePtr, ai_general_array, ai_general_count, ai_general_size);

	return 1;
}
//----------- End of function Nation::write_file ---------//


//--------- Begin of static function write_ai_info ---------//
//
static void write_ai_info(File* filePtr, short* aiInfoArray, short aiInfoCount, short aiInfoSize)
{
	filePtr->file_put_short( aiInfoCount );
	filePtr->file_put_short( aiInfoSize  );
	filePtr->file_write( aiInfoArray, sizeof(short) * aiInfoCount );
}
//----------- End of static function write_ai_info ---------//

template <typename Visitor>
static void visit_nation_relation(Visitor *v, NationRelation *nr)
{
	visit<int8_t>(v, &nr->has_contact);
	visit<int8_t>(v, &nr->should_attack);

	visit<int8_t>(v, &nr->trade_treaty);

	visit<int8_t>(v, &nr->status);

	visit<int32_t>(v, &nr->last_change_status_date);

	visit<int8_t>(v, &nr->ai_relation_level);
	visit<int8_t>(v, &nr->ai_secret_attack);
	visit<int8_t>(v, &nr->ai_demand_trade_treaty);

	visit<float>(v, &nr->good_relation_duration_rating);
	visit<int16_t>(v, &nr->started_war_on_us_count);

	visit_array<float>(v, nr->cur_year_import, IMPORT_TYPE_COUNT);
	visit_array<float>(v, nr->last_year_import, IMPORT_TYPE_COUNT);
	visit_array<float>(v, nr->lifetime_import, IMPORT_TYPE_COUNT);

	visit_array<int32_t>(v, nr->last_talk_reject_date_array, MAX_TALK_TYPE);

	visit<int32_t>(v, &nr->last_military_aid_date);

	visit<int32_t>(v, &nr->last_give_gift_date);
	visit<int16_t>(v, &nr->total_given_gift_amount);

	visit<int8_t>(v, &nr->contact_msg_flag);
}

template <typename Visitor>
static void visit_attack_camp(Visitor *v, AttackCamp *ac)
{
	visit<int16_t>(v, &ac->firm_recno);
	visit<int16_t>(v, &ac->combat_level);
	visit<int16_t>(v, &ac->distance);
	visit<int32_t>(v, &ac->patrol_date);
}

template <typename Visitor>
static void visit_ai_region(Visitor *v, AIRegion *reg)
{
	visit<int8_t>(v, &reg->region_id);
	visit<int8_t>(v, &reg->town_count);
	visit<int8_t>(v, &reg->base_town_count);
}

template <typename Visitor>
static void visit_version_1_nation(Visitor *v, Version_1_Nation *v1n)
{
	v->skip(2); /* record size */
	v->skip(4); /* virtual table pointer */

	/* NationBase */
	visit<int16_t>(v, &v1n->nation_recno);
	visit<int8_t>(v, &v1n->nation_type);
	visit<int8_t>(v, &v1n->race_id);
	visit<int8_t>(v, &v1n->color_scheme_id);
	visit<int8_t>(v, &v1n->nation_color);
	visit<int16_t>(v, &v1n->king_unit_recno);
	visit<int8_t>(v, &v1n->king_leadership);
	visit<int32_t>(v, &v1n->nation_name_id);

	visit_array<int8_t>(v, v1n->nation_name_str,
							  Version_1_Nation::NATION_NAME_LEN+1);

	visit<uint32_t>(v, &v1n->player_id);
	visit<int8_t>(v, &v1n->next_frame_ready);
	visit<int16_t>(v, &v1n->last_caravan_id);
	visit<int16_t>(v, &v1n->nation_firm_count);
	visit<int32_t>(v, &v1n->last_build_firm_date);
	visit_array<int8_t>(v, v1n->know_base_array, VERSION_1_MAX_RACE);
	visit_array<int8_t>(v, v1n->base_count_array, VERSION_1_MAX_RACE);
	visit<int8_t>(v, &v1n->is_at_war_today);
	visit<int8_t>(v, &v1n->is_at_war_yesterday);
	visit<int32_t>(v, &v1n->last_war_date);
	visit<int16_t>(v, &v1n->last_attacker_unit_recno);
	visit<int32_t>(v, &v1n->last_independent_unit_join_date);
	visit<int8_t>(v, &v1n->cheat_enabled_flag);
	visit<float>(v, &v1n->cash);
	visit<float>(v, &v1n->food);
	visit<float>(v, &v1n->reputation);
	visit<float>(v, &v1n->kill_monster_score);
	visit<int16_t>(v, &v1n->auto_collect_tax_loyalty);
	visit<int16_t>(v, &v1n->auto_grant_loyalty);
	visit<float>(v, &v1n->cur_year_profit);
	visit<float>(v, &v1n->last_year_profit);
	visit<float>(v, &v1n->cur_year_fixed_income);
	visit<float>(v, &v1n->last_year_fixed_income);
	visit<float>(v, &v1n->cur_year_fixed_expense);
	visit<float>(v, &v1n->last_year_fixed_expense);
	visit_array<float>(v, v1n->cur_year_income_array, INCOME_TYPE_COUNT);
	visit_array<float>(v, v1n->last_year_income_array, INCOME_TYPE_COUNT);
	visit<float>(v, &v1n->cur_year_income);
	visit<float>(v, &v1n->last_year_income);
	visit_array<float>(v, v1n->cur_year_expense_array, EXPENSE_TYPE_COUNT);
	visit_array<float>(v, v1n->last_year_expense_array, EXPENSE_TYPE_COUNT);
	visit<float>(v, &v1n->cur_year_expense);
	visit<float>(v, &v1n->last_year_expense);
	visit<float>(v, &v1n->cur_year_cheat);
	visit<float>(v, &v1n->last_year_cheat);
	visit<float>(v, &v1n->cur_year_food_in);
	visit<float>(v, &v1n->last_year_food_in);
	visit<float>(v, &v1n->cur_year_food_out);
	visit<float>(v, &v1n->last_year_food_out);
	visit<float>(v, &v1n->cur_year_food_change);
	visit<float>(v, &v1n->last_year_food_change);
	visit<float>(v, &v1n->cur_year_reputation_change);
	visit<float>(v, &v1n->last_year_reputation_change);

	for (int n = 0; n < MAX_NATION; n++)
		visit_nation_relation(v, &v1n->relation_array[n]);

	visit_array<int8_t>(v, v1n->relation_status_array, MAX_NATION);
	visit_array<int8_t>(v, v1n->relation_passable_array, MAX_NATION);

	visit_array<int8_t>(v, v1n->relation_should_attack_array, MAX_NATION);
	visit<int8_t>(v, &v1n->is_allied_with_player);
	visit<int32_t>(v, &v1n->total_population);
	visit<int32_t>(v, &v1n->total_jobless_population);
	visit<int32_t>(v, &v1n->total_unit_count);
	visit<int32_t>(v, &v1n->total_human_count);
	visit<int32_t>(v, &v1n->total_general_count);
	visit<int32_t>(v, &v1n->total_weapon_count);
	visit<int32_t>(v, &v1n->total_ship_count);
	visit<int32_t>(v, &v1n->total_firm_count);
	visit<int32_t>(v, &v1n->total_spy_count);
	visit<int32_t>(v, &v1n->total_ship_combat_level);
	visit<int16_t>(v, &v1n->largest_town_recno);
	visit<int16_t>(v, &v1n->largest_town_pop);
	visit_array<int16_t>(v, v1n->raw_count_array, MAX_RAW);
	visit_array<int16_t>(v, v1n->last_unit_name_id_array,
								VERSION_1_MAX_UNIT_TYPE);
	visit<int32_t>(v, &v1n->population_rating);
	visit<int32_t>(v, &v1n->military_rating);
	visit<int32_t>(v, &v1n->economic_rating);
	visit<int32_t>(v, &v1n->overall_rating);
	visit<int32_t>(v, &v1n->enemy_soldier_killed);
	visit<int32_t>(v, &v1n->own_soldier_killed);
	visit<int32_t>(v, &v1n->enemy_civilian_killed);
	visit<int32_t>(v, &v1n->own_civilian_killed);
	visit<int32_t>(v, &v1n->enemy_weapon_destroyed);
	visit<int32_t>(v, &v1n->own_weapon_destroyed);
	visit<int32_t>(v, &v1n->enemy_ship_destroyed);
	visit<int32_t>(v, &v1n->own_ship_destroyed);
	visit<int32_t>(v, &v1n->enemy_firm_destroyed);
	visit<int32_t>(v, &v1n->own_firm_destroyed);

	/* Nation */
	v->skip(29); /* action_array */

	visit<uint16_t>(v, &v1n->last_action_id);
	visit_pointer(v, &v1n->ai_town_array);
	visit_pointer(v, &v1n->ai_base_array);
	visit_pointer(v, &v1n->ai_mine_array);
	visit_pointer(v, &v1n->ai_factory_array);
	visit_pointer(v, &v1n->ai_camp_array);
	visit_pointer(v, &v1n->ai_research_array);
	visit_pointer(v, &v1n->ai_war_array);
	visit_pointer(v, &v1n->ai_harbor_array);
	visit_pointer(v, &v1n->ai_market_array);
	visit_pointer(v, &v1n->ai_inn_array);
	visit_pointer(v, &v1n->ai_general_array);
	visit_pointer(v, &v1n->ai_caravan_array);
	visit_pointer(v, &v1n->ai_ship_array);
	visit<int16_t>(v, &v1n->ai_town_size);
	visit<int16_t>(v, &v1n->ai_base_size);
	visit<int16_t>(v, &v1n->ai_mine_size);
	visit<int16_t>(v, &v1n->ai_factory_size);
	visit<int16_t>(v, &v1n->ai_camp_size);
	visit<int16_t>(v, &v1n->ai_research_size);
	visit<int16_t>(v, &v1n->ai_war_size);
	visit<int16_t>(v, &v1n->ai_harbor_size);
	visit<int16_t>(v, &v1n->ai_market_size);
	visit<int16_t>(v, &v1n->ai_inn_size);
	visit<int16_t>(v, &v1n->ai_general_size);
	visit<int16_t>(v, &v1n->ai_caravan_size);
	visit<int16_t>(v, &v1n->ai_ship_size);
	visit<int16_t>(v, &v1n->ai_town_count);
	visit<int16_t>(v, &v1n->ai_base_count);
	visit<int16_t>(v, &v1n->ai_mine_count);
	visit<int16_t>(v, &v1n->ai_factory_count);
	visit<int16_t>(v, &v1n->ai_camp_count);
	visit<int16_t>(v, &v1n->ai_research_count);
	visit<int16_t>(v, &v1n->ai_war_count);
	visit<int16_t>(v, &v1n->ai_harbor_count);
	visit<int16_t>(v, &v1n->ai_market_count);
	visit<int16_t>(v, &v1n->ai_inn_count);
	visit<int16_t>(v, &v1n->ai_general_count);
	visit<int16_t>(v, &v1n->ai_caravan_count);
	visit<int16_t>(v, &v1n->ai_ship_count);
	visit<int16_t>(v, &v1n->ai_base_town_count);
	visit_array<int16_t>(v, v1n->firm_should_close_array, MAX_FIRM_TYPE);
	
	for (int n = 0; n < MAX_AI_REGION; n++)
		visit_ai_region(v, &v1n->ai_region_array[n]);

	visit<int8_t>(v, &v1n->ai_region_count);
	visit<int8_t>(v, &v1n->pref_force_projection);
	visit<int8_t>(v, &v1n->pref_military_development);
	visit<int8_t>(v, &v1n->pref_economic_development);
	visit<int8_t>(v, &v1n->pref_inc_pop_by_capture);
	visit<int8_t>(v, &v1n->pref_inc_pop_by_growth);
	visit<int8_t>(v, &v1n->pref_peacefulness);
	visit<int8_t>(v, &v1n->pref_military_courage);
	visit<int8_t>(v, &v1n->pref_territorial_cohesiveness);
	visit<int8_t>(v, &v1n->pref_trading_tendency);
	visit<int8_t>(v, &v1n->pref_allying_tendency);
	visit<int8_t>(v, &v1n->pref_honesty);
	visit<int8_t>(v, &v1n->pref_town_harmony);
	visit<int8_t>(v, &v1n->pref_loyalty_concern);
	visit<int8_t>(v, &v1n->pref_forgiveness);
	visit<int8_t>(v, &v1n->pref_collect_tax);
	visit<int8_t>(v, &v1n->pref_hire_unit);
	visit<int8_t>(v, &v1n->pref_use_weapon);
	visit<int8_t>(v, &v1n->pref_keep_general);
	visit<int8_t>(v, &v1n->pref_keep_skilled_unit);
	visit<int8_t>(v, &v1n->pref_diplomacy_retry);
	visit<int8_t>(v, &v1n->pref_attack_monster);
	visit<int8_t>(v, &v1n->pref_spy);
	visit<int8_t>(v, &v1n->pref_counter_spy);
	visit<int8_t>(v, &v1n->pref_food_reserve);
	visit<int8_t>(v, &v1n->pref_cash_reserve);
	visit<int8_t>(v, &v1n->pref_use_marine);
	visit<int8_t>(v, &v1n->pref_unit_chase_distance);
	visit<int8_t>(v, &v1n->pref_repair_concern);
	visit<int8_t>(v, &v1n->pref_scout);
	visit<int16_t>(v, &v1n->ai_capture_enemy_town_recno);
	visit<int32_t>(v, &v1n->ai_capture_enemy_town_plan_date);
	visit<int32_t>(v, &v1n->ai_capture_enemy_town_start_attack_date);
	visit<int8_t>(v, &v1n->ai_capture_enemy_town_use_all_camp);
	visit<int32_t>(v, &v1n->ai_last_defend_action_date);
	visit<int16_t>(v, &v1n->ai_attack_target_x_loc);
	visit<int16_t>(v, &v1n->ai_attack_target_y_loc);
	visit<int16_t>(v, &v1n->ai_attack_target_nation_recno);

	for (int n = 0; n < MAX_SUITABLE_ATTACK_CAMP; n++)
		visit_attack_camp(v, &v1n->attack_camp_array[n]);

	visit<int16_t>(v, &v1n->attack_camp_count);
	visit<int16_t>(v, &v1n->lead_attack_camp_recno);
}

static bool read_version_1_nation(File *file, Version_1_Nation *v1n)
{
	FileReader r;
	FileReaderVisitor v(&r);

	if (!r.init(file))
		return false;

	visit_version_1_nation(&v, v1n);
	memset(&v1n->action_array, 0, sizeof(v1n->action_array));

	return r.good();
}

template <typename Visitor>
static bool visit_nation(Visitor *v, Nation *nat)
{
	v->skip(2); /* record size */
	v->skip(4); /* virtual table pointer */

	/* NationBase */
	visit<int16_t>(v, &nat->nation_recno);
	visit<int8_t>(v, &nat->nation_type);
	visit<int8_t>(v, &nat->race_id);
	visit<int8_t>(v, &nat->color_scheme_id);
	visit<int8_t>(v, &nat->nation_color);
	visit<int16_t>(v, &nat->king_unit_recno);
	visit<int8_t>(v, &nat->king_leadership);
	visit<int32_t>(v, &nat->nation_name_id);
	visit_array<int8_t>(v, nat->nation_name_str, Nation::NATION_NAME_LEN+1);
	visit<uint32_t>(v, &nat->player_id);
	visit<int8_t>(v, &nat->next_frame_ready);
	visit<int16_t>(v, &nat->last_caravan_id);
	visit<int16_t>(v, &nat->nation_firm_count);
	visit<int32_t>(v, &nat->last_build_firm_date);
	visit_array<int8_t>(v, nat->know_base_array, MAX_RACE);
	visit_array<int8_t>(v, nat->base_count_array, MAX_RACE);
	visit<int8_t>(v, &nat->is_at_war_today);
	visit<int8_t>(v, &nat->is_at_war_yesterday);
	visit<int32_t>(v, &nat->last_war_date);
	visit<int16_t>(v, &nat->last_attacker_unit_recno);
	visit<int32_t>(v, &nat->last_independent_unit_join_date);
	visit<int8_t>(v, &nat->cheat_enabled_flag);
	visit<float>(v, &nat->cash);
	visit<float>(v, &nat->food);
	visit<float>(v, &nat->reputation);
	visit<float>(v, &nat->kill_monster_score);
	visit<int16_t>(v, &nat->auto_collect_tax_loyalty);
	visit<int16_t>(v, &nat->auto_grant_loyalty);
	visit<float>(v, &nat->cur_year_profit);
	visit<float>(v, &nat->last_year_profit);
	visit<float>(v, &nat->cur_year_fixed_income);
	visit<float>(v, &nat->last_year_fixed_income);
	visit<float>(v, &nat->cur_year_fixed_expense);
	visit<float>(v, &nat->last_year_fixed_expense);
	visit_array<float>(v, nat->cur_year_income_array, INCOME_TYPE_COUNT);
	visit_array<float>(v, nat->last_year_income_array, INCOME_TYPE_COUNT);
	visit<float>(v, &nat->cur_year_income);
	visit<float>(v, &nat->last_year_income);
	visit_array<float>(v, nat->cur_year_expense_array, EXPENSE_TYPE_COUNT);
	visit_array<float>(v, nat->last_year_expense_array, EXPENSE_TYPE_COUNT);
	visit<float>(v, &nat->cur_year_expense);
	visit<float>(v, &nat->last_year_expense);
	visit<float>(v, &nat->cur_year_cheat);
	visit<float>(v, &nat->last_year_cheat);
	visit<float>(v, &nat->cur_year_food_in);
	visit<float>(v, &nat->last_year_food_in);
	visit<float>(v, &nat->cur_year_food_out);
	visit<float>(v, &nat->last_year_food_out);
	visit<float>(v, &nat->cur_year_food_change);
	visit<float>(v, &nat->last_year_food_change);
	visit<float>(v, &nat->cur_year_reputation_change);
	visit<float>(v, &nat->last_year_reputation_change);

	for (int n = 0; n < MAX_NATION; n++)
		visit_nation_relation(v, &nat->relation_array[n]);

	visit_array<int8_t>(v, nat->relation_status_array, MAX_NATION);
	visit_array<int8_t>(v, nat->relation_passable_array, MAX_NATION);
	visit_array<int8_t>(v, nat->relation_should_attack_array, MAX_NATION);
	visit<int8_t>(v, &nat->is_allied_with_player);
	visit<int32_t>(v, &nat->total_population);
	visit<int32_t>(v, &nat->total_jobless_population);
	visit<int32_t>(v, &nat->total_unit_count);
	visit<int32_t>(v, &nat->total_human_count);
	visit<int32_t>(v, &nat->total_general_count);
	visit<int32_t>(v, &nat->total_weapon_count);
	visit<int32_t>(v, &nat->total_ship_count);
	visit<int32_t>(v, &nat->total_firm_count);
	visit<int32_t>(v, &nat->total_spy_count);
	visit<int32_t>(v, &nat->total_ship_combat_level);
	visit<int16_t>(v, &nat->largest_town_recno);
	visit<int16_t>(v, &nat->largest_town_pop);
	visit_array<int16_t>(v, nat->raw_count_array, MAX_RAW);
	visit_array<int16_t>(v, nat->last_unit_name_id_array, MAX_UNIT_TYPE);
	visit<int32_t>(v, &nat->population_rating);
	visit<int32_t>(v, &nat->military_rating);
	visit<int32_t>(v, &nat->economic_rating);
   visit<int32_t>(v, &nat->overall_rating);
	visit<int32_t>(v, &nat->enemy_soldier_killed);
	visit<int32_t>(v, &nat->own_soldier_killed);
	visit<int32_t>(v, &nat->enemy_civilian_killed);
	visit<int32_t>(v, &nat->own_civilian_killed);
	visit<int32_t>(v, &nat->enemy_weapon_destroyed);
	visit<int32_t>(v, &nat->own_weapon_destroyed);
	visit<int32_t>(v, &nat->enemy_ship_destroyed);
	visit<int32_t>(v, &nat->own_ship_destroyed);
	visit<int32_t>(v, &nat->enemy_firm_destroyed);
	visit<int32_t>(v, &nat->own_firm_destroyed);

	/* Nation */
	v->skip(29); /* action_array */

	visit<uint16_t>(v, &nat->last_action_id);
	visit_pointer(v, &nat->ai_town_array);
	visit_pointer(v, &nat->ai_base_array);
	visit_pointer(v, &nat->ai_mine_array);
	visit_pointer(v, &nat->ai_factory_array);
	visit_pointer(v, &nat->ai_camp_array);
	visit_pointer(v, &nat->ai_research_array);
	visit_pointer(v, &nat->ai_war_array);
	visit_pointer(v, &nat->ai_harbor_array);
	visit_pointer(v, &nat->ai_market_array);
	visit_pointer(v, &nat->ai_inn_array);
	visit_pointer(v, &nat->ai_general_array);
	visit_pointer(v, &nat->ai_caravan_array);
	visit_pointer(v, &nat->ai_ship_array);
	visit<int16_t>(v, &nat->ai_town_size);
	visit<int16_t>(v, &nat->ai_base_size);
	visit<int16_t>(v, &nat->ai_mine_size);
	visit<int16_t>(v, &nat->ai_factory_size);
	visit<int16_t>(v, &nat->ai_camp_size);
	visit<int16_t>(v, &nat->ai_research_size);
	visit<int16_t>(v, &nat->ai_war_size);
	visit<int16_t>(v, &nat->ai_harbor_size);
	visit<int16_t>(v, &nat->ai_market_size);
	visit<int16_t>(v, &nat->ai_inn_size);
	visit<int16_t>(v, &nat->ai_general_size);
	visit<int16_t>(v, &nat->ai_caravan_size);
	visit<int16_t>(v, &nat->ai_ship_size);
	visit<int16_t>(v, &nat->ai_town_count);
	visit<int16_t>(v, &nat->ai_base_count);
	visit<int16_t>(v, &nat->ai_mine_count);
	visit<int16_t>(v, &nat->ai_factory_count);
	visit<int16_t>(v, &nat->ai_camp_count);
	visit<int16_t>(v, &nat->ai_research_count);
	visit<int16_t>(v, &nat->ai_war_count);
	visit<int16_t>(v, &nat->ai_harbor_count);
	visit<int16_t>(v, &nat->ai_market_count);
	visit<int16_t>(v, &nat->ai_inn_count);
	visit<int16_t>(v, &nat->ai_general_count);
	visit<int16_t>(v, &nat->ai_caravan_count);
	visit<int16_t>(v, &nat->ai_ship_count);
	visit<int16_t>(v, &nat->ai_base_town_count);
	visit_array<int16_t>(v, nat->firm_should_close_array, MAX_FIRM_TYPE);
	
	for (int n = 0; n < MAX_AI_REGION; n++)
		visit_ai_region(v, &nat->ai_region_array[n]);

	visit<int8_t>(v, &nat->ai_region_count);
	visit<int8_t>(v, &nat->pref_force_projection);
	visit<int8_t>(v, &nat->pref_military_development);
	visit<int8_t>(v, &nat->pref_economic_development);
	visit<int8_t>(v, &nat->pref_inc_pop_by_capture);
	visit<int8_t>(v, &nat->pref_inc_pop_by_growth);
	visit<int8_t>(v, &nat->pref_peacefulness);
	visit<int8_t>(v, &nat->pref_military_courage);
	visit<int8_t>(v, &nat->pref_territorial_cohesiveness);
	visit<int8_t>(v, &nat->pref_trading_tendency);
	visit<int8_t>(v, &nat->pref_allying_tendency);
	visit<int8_t>(v, &nat->pref_honesty);
	visit<int8_t>(v, &nat->pref_town_harmony);
	visit<int8_t>(v, &nat->pref_loyalty_concern);
	visit<int8_t>(v, &nat->pref_forgiveness);
	visit<int8_t>(v, &nat->pref_collect_tax);
	visit<int8_t>(v, &nat->pref_hire_unit);
	visit<int8_t>(v, &nat->pref_use_weapon);
	visit<int8_t>(v, &nat->pref_keep_general);
	visit<int8_t>(v, &nat->pref_keep_skilled_unit);
	visit<int8_t>(v, &nat->pref_diplomacy_retry);
	visit<int8_t>(v, &nat->pref_attack_monster);
	visit<int8_t>(v, &nat->pref_spy);
	visit<int8_t>(v, &nat->pref_counter_spy);
	visit<int8_t>(v, &nat->pref_food_reserve);
	visit<int8_t>(v, &nat->pref_cash_reserve);
	visit<int8_t>(v, &nat->pref_use_marine);
	visit<int8_t>(v, &nat->pref_unit_chase_distance);
	visit<int8_t>(v, &nat->pref_repair_concern);
	visit<int8_t>(v, &nat->pref_scout);
	visit<int16_t>(v, &nat->ai_capture_enemy_town_recno);
	visit<int32_t>(v, &nat->ai_capture_enemy_town_plan_date);
	visit<int32_t>(v, &nat->ai_capture_enemy_town_start_attack_date);
	visit<int8_t>(v, &nat->ai_capture_enemy_town_use_all_camp);
	visit<int32_t>(v, &nat->ai_last_defend_action_date);
	visit<int16_t>(v, &nat->ai_attack_target_x_loc);
	visit<int16_t>(v, &nat->ai_attack_target_y_loc);
	visit<int16_t>(v, &nat->ai_attack_target_nation_recno);

	for (int n = 0; n < MAX_SUITABLE_ATTACK_CAMP; n++)
		visit_attack_camp(v, &nat->attack_camp_array[n]);

	visit<int16_t>(v, &nat->attack_camp_count);
	visit<int16_t>(v, &nat->lead_attack_camp_recno);
}

static bool read_nation(File *file, Nation *nat)
{
	FileReader r;
	FileReaderVisitor v(&r);

	if (!r.init(file))
		return false;

	visit_nation(&v, nat);
	memset(&nat->action_array, 0, sizeof(nat->action_array));

	return r.good();
}

//--------- Begin of function Nation::read_file ---------//
//
int Nation::read_file(File* filePtr)
{
#ifdef AMPLUS
	if(!game_file_array.same_version)
	{
		Version_1_Nation *oldNationPtr = (Version_1_Nation*) mem_add(sizeof(Version_1_Nation));

		if (!read_version_1_nation(filePtr, oldNationPtr))
		{
			mem_del(oldNationPtr);
			return 0;
		}

		oldNationPtr->convert_to_version_2(this);
		mem_del(oldNationPtr);
	}
	else
	{
		if (!read_nation(filePtr, this))
			return 0;
	}
#else
	if (!read_nation(filePtr, this))
		return 0;
#endif

	//-------------- read AI Action Array --------------//

	action_array.read_file(filePtr);

	//------ write AI info array ---------//

	read_ai_info(filePtr, &ai_town_array, ai_town_count, ai_town_size);

	read_ai_info(filePtr, &ai_base_array, ai_base_count, ai_base_size);
	read_ai_info(filePtr, &ai_mine_array, ai_mine_count, ai_mine_size);
	read_ai_info(filePtr, &ai_factory_array, ai_factory_count, ai_factory_size);
	read_ai_info(filePtr, &ai_market_array, ai_market_count, ai_market_size);
	read_ai_info(filePtr, &ai_inn_array, ai_inn_count, ai_inn_size);
	read_ai_info(filePtr, &ai_camp_array, ai_camp_count, ai_camp_size);
	read_ai_info(filePtr, &ai_research_array, ai_research_count, ai_research_size);
	read_ai_info(filePtr, &ai_war_array, ai_war_count, ai_war_size);
	read_ai_info(filePtr, &ai_harbor_array, ai_harbor_count, ai_harbor_size);

	read_ai_info(filePtr, &ai_caravan_array, ai_caravan_count, ai_caravan_size);
	read_ai_info(filePtr, &ai_ship_array, ai_ship_count, ai_ship_size);
	read_ai_info(filePtr, &ai_general_array, ai_general_count, ai_general_size);

	return 1;
}
//----------- End of function Nation::read_file ---------//


//--------- Begin of static function read_ai_info ---------//
//
static void read_ai_info(File* filePtr, short** aiInfoArrayPtr, short& aiInfoCount, short& aiInfoSize)
{
	aiInfoCount = filePtr->file_get_short();
	aiInfoSize  = filePtr->file_get_short();

	*aiInfoArrayPtr = (short*) mem_add( aiInfoSize * sizeof(short) );

	filePtr->file_read( *aiInfoArrayPtr, sizeof(short) * aiInfoCount );
}
//----------- End of static function read_ai_info ---------//

//*****//

//-------- Start of function TornadoArray::write_file -------------//
//
int TornadoArray::write_file(File* filePtr)
{
	filePtr->file_put_short(restart_recno);  // variable in SpriteArray

	int    i;
   Tornado *tornadoPtr;

   filePtr->file_put_short( size() );  // no. of tornados in tornado_array

   for( i=1; i<=size() ; i++ )
   {
      tornadoPtr = (Tornado*) get_ptr(i);

      //----- write tornadoId or 0 if the tornado is deleted -----//

      if( !tornadoPtr )    // the tornado is deleted
      {
         filePtr->file_put_short(0);
      }
      else
      {
         filePtr->file_put_short(1);      // there is a tornado in this record

         //------ write data in the base class ------//

         if( !tornadoPtr->write_file(filePtr) )
            return 0;
      }
   }

   //------- write empty room array --------//

   write_empty_room(filePtr);

   return 1;
}
//--------- End of function TornadoArray::write_file -------------//


//-------- Start of function TornadoArray::read_file -------------//
//
int TornadoArray::read_file(File* filePtr)
{
	restart_recno    = filePtr->file_get_short();

   int     i, tornadoRecno, tornadoCount;
   Tornado* tornadoPtr;

   tornadoCount = filePtr->file_get_short();  // get no. of tornados from file

   for( i=1 ; i<=tornadoCount ; i++ )
   {
      if( filePtr->file_get_short() == 0 )
      {
         add_blank(1);     // it's a DynArrayB function
      }
      else
      {
         //----- create tornado object -----------//

         tornadoRecno = tornado_array.create_tornado();
         tornadoPtr   = tornado_array[tornadoRecno];

         //----- read data in base class --------//

         if( !tornadoPtr->read_file( filePtr ) )
            return 0;
      }
   }

   //-------- linkout() those record added by add_blank() ----------//
   //-- So they will be marked deleted in DynArrayB and can be -----//
   //-- undeleted and used when a new record is going to be added --//

   for( i=size() ; i>0 ; i-- )
   {
      DynArrayB::go(i);             // since TornadoArray has its own go() which will call GroupArray::go()

      if( get_ptr() == NULL )       // add_blank() record
         linkout();
   }

   //------- read empty room array --------//

   read_empty_room(filePtr);

   return 1;
}
//--------- End of function TornadoArray::read_file ---------------//


//--------- Begin of function Tornado::write_file ---------//
//
int Tornado::write_file(File* filePtr)
{
   if( !filePtr->file_write( this, sizeof(Tornado) ) )
      return 0;

   return 1;
}
//----------- End of function Tornado::write_file ---------//


//--------- Begin of function Tornado::read_file ---------//
//
int Tornado::read_file(File* filePtr)
{
	FileReader r;

	MSG("Tornado::read_file()\n");

	if (!r.init(filePtr))
		return 0;

	r.skip(2); /* record size */

	read_sprite(&r, this);
   r.read<float>(&this->attack_damage);
   r.read<int16_t>(&this->life_time);
   r.read<int16_t>(&this->dmg_offset_x);
   r.read<int16_t>(&this->dmg_offset_y);

	if (!r.good())
		return 0;

	r.deinit();

   //------------ post-process the data read ----------//

   sprite_info = sprite_res[sprite_id];

	sprite_info->load_bitmap_res();

	return 1;
}
//----------- End of function Tornado::read_file ---------//


//*****//


//-------- Start of function RebelArray::write_file -------------//
//
int RebelArray::write_file(File* filePtr)
{
	return write_ptr_array(filePtr, sizeof(Rebel));
}
//--------- End of function RebelArray::write_file ---------------//


//-------- Start of function RebelArray::read_file -------------//
//
int RebelArray::read_file(File* filePtr)
{
	return read_ptr_array(filePtr, sizeof(Rebel), create_rebel_func);
}
//--------- End of function RebelArray::read_file ---------------//


//-------- Start of static function create_rebel_func ---------//
//
static char* create_rebel_func()
{
	Rebel *rebelPtr = new Rebel;

	rebel_array.linkin(&rebelPtr);

	return (char*) rebelPtr;
}
//--------- End of static function create_rebel_func ----------//


//*****//


//-------- Start of function SpyArray::write_file -------------//
//
int SpyArray::write_file(File* filePtr)
{
	return DynArrayB::write_file( filePtr );
}
//--------- End of function SpyArray::write_file ---------------//


//-------- Start of function SpyArray::read_file -------------//
//
int SpyArray::read_file(File* filePtr)
{
	return DynArrayB::read_file( filePtr );
}
//--------- End of function SpyArray::read_file ---------------//


//*****//


//-------- Start of function SnowGroundArray::write_file -------------//
//
int SnowGroundArray::write_file(File* filePtr)
{
   if( !filePtr->file_write( this, sizeof(SnowGroundArray)) )
      return 0;

   return 1;
}
//--------- End of function SnowGroundArray::write_file ---------------//


//-------- Start of function SnowGroundArray::read_file -------------//
//
int SnowGroundArray::read_file(File* filePtr)
{
	MSG(__FILE__":%d: file_read(this, ...);\n", __LINE__);

   if( !filePtr->file_read( this, sizeof(SnowGroundArray)) )
      return 0;

   return 1;
}
//--------- End of function SnowGroundArray::read_file ---------------//

//*****//

//-------- Start of function RegionArray::write_file -------------//
//
int RegionArray::write_file(File* filePtr)
{
   if( !filePtr->file_write( this, sizeof(RegionArray)) )
      return 0;

	if( !filePtr->file_write( region_info_array, sizeof(RegionInfo)*region_info_count ) )
		return 0;

	//-------- write RegionStat ----------//

	filePtr->file_put_short( region_stat_count );

	if( !filePtr->file_write( region_stat_array, sizeof(RegionStat)*region_stat_count ) )
		return 0;

	//--------- write connection bits ----------//

	int connectBit = (region_info_count -1) * (region_info_count) /2;
	int connectByte = (connectBit +7) /8;

	if( connectByte > 0)
	{
		if( !filePtr->file_write(connect_bits, connectByte) )
			return 0;
	}

	return 1;
}
//--------- End of function RegionArray::write_file ---------------//


//-------- Start of function RegionArray::read_file -------------//
//
int RegionArray::read_file(File* filePtr)
{
	FileReader r;

	if (!r.init(filePtr))
		return 0;

	r.skip(2); /* record size */

	r.read<int32_t>(&this->init_flag);
	r.read(&this->region_info_array);
	r.read<int32_t>(&this->region_info_count);
	r.read(&this->region_stat_array);
	r.read<int32_t>(&this->region_stat_count);
	r.read(&this->connect_bits);
	r.read_array<uint8_t>(this->region_sorted_array, MAX_REGION);

	if (!r.good())
		return 0;

	r.deinit();

   if( region_info_count > 0 )
      region_info_array = (RegionInfo *) mem_add(sizeof(RegionInfo)*region_info_count);
   else
      region_info_array = NULL;

   if( !filePtr->file_read( region_info_array, sizeof(RegionInfo)*region_info_count))
      return 0;

	//-------- read RegionStat ----------//

	region_stat_count = filePtr->file_get_short();

	region_stat_array = (RegionStat*) mem_add( region_stat_count * sizeof(RegionStat) );

	if( !filePtr->file_read( region_stat_array, sizeof(RegionStat)*region_stat_count ) )
		return 0;

	//--------- read connection bits ----------//

	int connectBit = (region_info_count -1) * (region_info_count) /2;
	int connectByte = (connectBit +7) /8;

	if( connectByte > 0)
	{
		connect_bits = (unsigned char *)mem_add(connectByte);
		if( !filePtr->file_read(connect_bits, connectByte) )
			return 0;
	}
	else
	{
		connect_bits = NULL;
	}

	return 1;
}
//--------- End of function RegionArray::read_file ---------------//

//*****//

//-------- Start of function NewsArray::write_file -------------//
//
int NewsArray::write_file(File* filePtr)
{
   //----- save news_array parameters -----//

   filePtr->file_write( news_type_option, sizeof(news_type_option) );

   filePtr->file_put_short(news_who_option);
   filePtr->file_put_long (last_clear_recno);

   //---------- save news data -----------//

   return DynArray::write_file(filePtr);
}
//--------- End of function NewsArray::write_file ---------------//


//-------- Start of function NewsArray::read_file -------------//
//
int NewsArray::read_file(File* filePtr)
{
   //----- read news_array parameters -----//

   filePtr->file_read( news_type_option, sizeof(news_type_option) );

   news_who_option   = (char) filePtr->file_get_short();
   last_clear_recno  = filePtr->file_get_long();

   //---------- read news data -----------//

   return DynArray::read_file(filePtr);
}
//--------- End of function NewsArray::read_file ---------------//


