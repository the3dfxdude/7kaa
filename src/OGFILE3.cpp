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
#include <OSaveGameArray.h>
#include <dbglog.h>
#include <file_io_visitor.h>
#include <file_reader.h>

#include <OGF_V1.h>

using namespace FileIOVisitor;

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
	// unused short*4
	filePtr->file_put_short(0);
	filePtr->file_put_short(0);
	filePtr->file_put_short(0);
	filePtr->file_put_short(0);

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
	// unused short*4
	filePtr->file_get_short();
	filePtr->file_get_short();
	filePtr->file_get_short();
	filePtr->file_get_short();

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

			unitPtr->fix_attack_info();
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

template <typename Visitor>
static void visit_unit(Visitor *v, Unit *u)
{
	/* Sprite */
	visit_sprite(v, u);

	/* Unit */
	visit<int8_t>(v, &u->unit_id);
	visit<int8_t>(v, &u->rank_id);
	visit<int8_t>(v, &u->race_id);
	visit<int8_t>(v, &u->nation_recno);
	visit<int8_t>(v, &u->ai_unit);
	visit<uint16_t>(v, &u->name_id);
	visit<uint32_t>(v, &u->unit_group_id);
	visit<uint32_t>(v, &u->team_id);
	visit<int8_t>(v, &u->selected_flag);
	visit<int8_t>(v, &u->group_select_id);
	visit<int8_t>(v, &u->waiting_term);
	visit<int8_t>(v, &u->blocked_by_member);
	visit<int8_t>(v, &u->swapping);
	visit<int16_t>(v, &u->leader_unit_recno);
	visit<int8_t>(v, &u->action_misc);
	visit<int16_t>(v, &u->action_misc_para);
	visit<int8_t>(v, &u->action_mode);
	visit<int16_t>(v, &u->action_para);
	visit<int16_t>(v, &u->action_x_loc);
	visit<int16_t>(v, &u->action_y_loc);
	visit<int8_t>(v, &u->action_mode2);
	visit<int16_t>(v, &u->action_para2);
	visit<int16_t>(v, &u->action_x_loc2);
	visit<int16_t>(v, &u->action_y_loc2);
	visit_array<int8_t>(v, u->blocked_edge, 4);
	visit<uint8_t>(v, &u->attack_dir);
	visit<int16_t>(v, &u->range_attack_x_loc);
	visit<int16_t>(v, &u->range_attack_y_loc);
	visit<int16_t>(v, &u->move_to_x_loc);
	visit<int16_t>(v, &u->move_to_y_loc);
	visit<int8_t>(v, &u->loyalty);
	visit<int8_t>(v, &u->target_loyalty);
	visit<float>(v, &u->hit_points);
	visit<int16_t>(v, &u->max_hit_points);

	visit<int8_t>(v, &u->skill.combat_level);
	visit<int8_t>(v, &u->skill.skill_id);
	visit<int8_t>(v, &u->skill.skill_level);
	visit<uint8_t>(v, &u->skill.combat_level_minor);
	visit<uint8_t>(v, &u->skill.skill_level_minor);
	visit<uint8_t>(v, &u->skill.skill_potential);

	visit<int8_t>(v, &u->unit_mode);
	visit<int16_t>(v, &u->unit_mode_para);
	visit<int16_t>(v, &u->spy_recno);
	visit<int16_t>(v, &u->nation_contribution);
	visit<int16_t>(v, &u->total_reward);
	visit_pointer(v, &u->attack_info_array);
	visit<int8_t>(v, &u->attack_count);
	visit<int8_t>(v, &u->attack_range);
	visit<int16_t>(v, &u->cur_power);
	visit<int16_t>(v, &u->max_power);
	visit_pointer(v, &u->result_node_array);
	visit<int32_t>(v, &u->result_node_count);
	visit<int16_t>(v, &u->result_node_recno);
	visit<int16_t>(v, &u->result_path_dist);
	visit_pointer(v, &u->way_point_array);
	visit<int16_t>(v, &u->way_point_array_size);
	visit<int16_t>(v, &u->way_point_count);
	visit<uint16_t>(v, &u->ai_action_id);
	visit<int8_t>(v, &u->original_action_mode);
	visit<int16_t>(v, &u->original_action_para);
	visit<int16_t>(v, &u->original_action_x_loc);
	visit<int16_t>(v, &u->original_action_y_loc);
	visit<int16_t>(v, &u->original_target_x_loc);
	visit<int16_t>(v, &u->original_target_y_loc);
	visit<int16_t>(v, &u->ai_original_target_x_loc);
	visit<int16_t>(v, &u->ai_original_target_y_loc);
	visit<int8_t>(v, &u->ai_no_suitable_action);
	visit<int8_t>(v, &u->can_guard_flag);
	visit<int8_t>(v, &u->can_attack_flag);
	visit<int8_t>(v, &u->force_move_flag);
	visit<int16_t>(v, &u->home_camp_firm_recno);
	visit<int8_t>(v, &u->aggressive_mode);
	visit<int8_t>(v, &u->seek_path_fail_count);
	visit<int8_t>(v, &u->ignore_power_nation);
	visit_pointer(v, &u->team_info);
}

//--------- Begin of function Unit::write_file ---------//
//
// Write data in derived class.
//
// If the derived Unit don't have any special data,
// just use Unit::write_file(), otherwise make its own derived copy of write_file()
//
int Unit::write_file(File* filePtr)
{
	if (!write_with_record_size(filePtr, this, &visit_unit<FileWriterVisitor>, 169))
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

template <typename Visitor>
static void visit_sprite(Visitor *v, Sprite *s)
{
	v->skip(4); /* virtual table pointer */

	visit<int16_t>(v, &s->sprite_id);
	visit<int16_t>(v, &s->sprite_recno);
	visit<int8_t>(v, &s->mobile_type);
	visit<uint8_t>(v, &s->cur_action);
	visit<uint8_t>(v, &s->cur_dir);
	visit<uint8_t>(v, &s->cur_frame);
	visit<uint8_t>(v, &s->cur_attack);
	visit<uint8_t>(v, &s->final_dir);
	visit<int8_t>(v, &s->turn_delay);
	visit<int8_t>(v, &s->guard_count);
	visit<uint8_t>(v, &s->remain_attack_delay);
	visit<uint8_t>(v, &s->remain_frames_per_step);
	visit<int16_t>(v, &s->cur_x);
	visit<int16_t>(v, &s->cur_y);
	visit<int16_t>(v, &s->go_x);
	visit<int16_t>(v, &s->go_y);
	visit<int16_t>(v, &s->next_x);
	visit<int16_t>(v, &s->next_y);
	visit_pointer(v, &s->sprite_info);
}

//--------- Begin of function Unit::read_file ---------//
//
int Unit::read_file(File* filePtr)
{
	FileReader r;
	FileReaderVisitor v;

	if (!r.init(filePtr))
		return 0;

	r.check_record_size(169);
	v.init(&r);
	visit_unit(&v, this);

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

   return 1;
}
//----------- End of function Unit::read_derived_file ---------//

template <typename Visitor>
static void visit_trade_stop(Visitor *v, TradeStop *ts)
{
	visit<int16_t>(v, &ts->firm_recno);
	visit<int16_t>(v, &ts->firm_loc_x1);
	visit<int16_t>(v, &ts->firm_loc_y1);
	visit<int8_t>(v, &ts->pick_up_type);
	visit_array<int8_t>(v, ts->pick_up_array, MAX_PICK_UP_GOODS);
}

template <typename Visitor>
static void visit_attack_info(Visitor *v, AttackInfo *ai)
{
	visit<uint8_t>(v, &ai->combat_level);
	visit<uint8_t>(v, &ai->attack_delay);
	visit<uint8_t>(v, &ai->attack_range);
	visit<uint8_t>(v, &ai->attack_damage);
   visit<uint8_t>(v, &ai->pierce_damage);
	visit<int16_t>(v, &ai->bullet_out_frame);
	visit<int8_t>(v, &ai->bullet_speed);
	visit<int8_t>(v, &ai->bullet_radius);
	visit<int8_t>(v, &ai->bullet_sprite_id);
	visit<int8_t>(v, &ai->dll_bullet_sprite_id);
	visit<int8_t>(v, &ai->eqv_attack_next);
	visit<int16_t>(v, &ai->min_power);
	visit<int16_t>(v, &ai->consume_power);
	visit<int8_t>(v, &ai->fire_radius);
	visit<int16_t>(v, &ai->effect_id);
}

template <typename Visitor>
static void visit_unit_marine_derived(Visitor *v, UnitMarine *u)
{
	visit_sprite(v, &u->splash);
	visit<int8_t>(v, &u->menu_mode);
	visit<int8_t>(v, &u->extra_move_in_beach);
	visit<int8_t>(v, &u->in_beach);
	visit<int8_t>(v, &u->selected_unit_id);
	visit_array<int16_t>(v, u->unit_recno_array, MAX_UNIT_IN_SHIP);
	visit<int8_t>(v, &u->unit_count);
	visit<int8_t>(v, &u->journey_status);
	visit<int8_t>(v, &u->dest_stop_id);
	visit<int8_t>(v, &u->stop_defined_num);
	visit<int8_t>(v, &u->wait_count);
	visit<int16_t>(v, &u->stop_x_loc);
	visit<int16_t>(v, &u->stop_y_loc);
	visit<int8_t>(v, &u->auto_mode);
	visit<int16_t>(v, &u->cur_firm_recno);
	visit<int16_t>(v, &u->carry_goods_capacity);

	for (int n = 0; n < MAX_STOP_FOR_SHIP; n++)
		visit_trade_stop(v, &u->stop_array[n]);

	visit_array<int16_t>(v, u->raw_qty_array, MAX_RAW);
	visit_array<int16_t>(v, u->product_raw_qty_array, MAX_PRODUCT);
	visit_attack_info(v, &u->ship_attack_info);
	visit<uint8_t>(v, &u->attack_mode_selected);
	visit<int32_t>(v, &u->last_load_goods_date);
}

enum { UNIT_MARINE_DERIVED_RECORD_SIZE = 145 };

//--------- Begin of function UnitMarine::read_derived_file ---------//
int UnitMarine::read_derived_file(File* filePtr)
{
	if (!read_with_record_size(filePtr, this, &visit_unit_marine_derived<FileReaderVisitor>,
										UNIT_MARINE_DERIVED_RECORD_SIZE))
		return 0;

	// ------- post-process the data read --------//
	splash.sprite_info = sprite_res[splash.sprite_id];
	splash.sprite_info->load_bitmap_res();

	return 1;
}
//--------- End of function UnitMarine::read_derived_file ---------//

int UnitMarine::write_derived_file(File *filePtr)
{
	return write_with_record_size(filePtr, this, &visit_unit_marine_derived<FileWriterVisitor>,
											UNIT_MARINE_DERIVED_RECORD_SIZE);
}


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

template <typename Visitor>
static void visit_bullet(Visitor *v, Bullet *b)
{
	visit_sprite(v, b);
	visit<int8_t>(v, &b->parent_type);
	visit<int16_t>(v, &b->parent_recno);
	visit<int8_t>(v, &b->target_mobile_type);
	visit<float>(v, &b->attack_damage);
	visit<int16_t>(v, &b->damage_radius);
	visit<int16_t>(v, &b->nation_recno);
	visit<int8_t>(v, &b->fire_radius);
	visit<int16_t>(v, &b->origin_x);
	visit<int16_t>(v, &b->origin_y);
	visit<int16_t>(v, &b->target_x_loc);
	visit<int16_t>(v, &b->target_y_loc);
	visit<int8_t>(v, &b->cur_step);
	visit<int8_t>(v, &b->total_step);
}

enum { BULLET_RECORD_SIZE = 57 };

//--------- Begin of function Bullet::write_file ---------//
//
int Bullet::write_file(File* filePtr)
{
	return write_with_record_size(filePtr, this, &visit_bullet<FileWriterVisitor>,
											BULLET_RECORD_SIZE);
}
//----------- End of function Bullet::write_file ---------//

//--------- Begin of function Bullet::read_file ---------//
//
int Bullet::read_file(File* filePtr)
{
	if (!read_with_record_size(filePtr, this, &visit_bullet<FileReaderVisitor>,
										BULLET_RECORD_SIZE))
		return 0;

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

template <typename Visitor>
static void visit_projectile(Visitor *v, Projectile *p)
{
	visit<float>(v, &p->z_coff);
	visit_sprite(v, &p->act_bullet);
	visit_sprite(v, &p->bullet_shadow);
}

enum { PROJECTILE_RECORD_SIZE = 72 };

//----------- Begin of function Projectile::read_derived_file ---------//

int Projectile::write_derived_file(File *filePtr)
{
	return write_with_record_size(filePtr, this, &visit_projectile<FileWriterVisitor>,
											PROJECTILE_RECORD_SIZE);
}

int Projectile::read_derived_file(File *filePtr)
{
	if (!read_with_record_size(filePtr, this, &visit_projectile<FileReaderVisitor>,
										PROJECTILE_RECORD_SIZE))
		return 0;

   //----------- post-process the data read ----------//
	act_bullet.sprite_info = sprite_res[act_bullet.sprite_id];
	act_bullet.sprite_info->load_bitmap_res();
	bullet_shadow.sprite_info = sprite_res[bullet_shadow.sprite_id];
	bullet_shadow.sprite_info->load_bitmap_res();

	return 1;
}
//----------- End of function Projectile::read_derived_file ---------//

//*****//

template <typename Visitor>
static void visit_firm(Visitor *v, Firm *f)
{
	v->skip(4); /* virtual table pointer */

	visit<int8_t>(v, &f->firm_id);
	visit<int16_t>(v, &f->firm_build_id);
	visit<int16_t>(v, &f->firm_recno);
	visit<int8_t>(v, &f->firm_ai);
	visit<int8_t>(v, &f->ai_processed);
	visit<int8_t>(v, &f->ai_status);
	visit<int8_t>(v, &f->ai_link_checked);
	visit<int8_t>(v, &f->ai_sell_flag);
	visit<int8_t>(v, &f->race_id);
	visit<int16_t>(v, &f->nation_recno);
	visit<int16_t>(v, &f->closest_town_name_id);
	visit<int16_t>(v, &f->firm_name_instance_id);
	visit<int16_t>(v, &f->loc_x1);
	visit<int16_t>(v, &f->loc_y1);
	visit<int16_t>(v, &f->loc_x2);
	visit<int16_t>(v, &f->loc_y2);
	visit<int16_t>(v, &f->abs_x1);
	visit<int16_t>(v, &f->abs_y1);
	visit<int16_t>(v, &f->abs_x2);
	visit<int16_t>(v, &f->abs_y2);
	visit<int16_t>(v, &f->center_x);
	visit<int16_t>(v, &f->center_y);
	visit<uint8_t>(v, &f->region_id);
	visit<int8_t>(v, &f->cur_frame);
	visit<int8_t>(v, &f->remain_frame_delay);
	visit<float>(v, &f->hit_points);
	visit<float>(v, &f->max_hit_points);
	visit<int8_t>(v, &f->under_construction);
	visit<int8_t>(v, &f->firm_skill_id);
	visit<int16_t>(v, &f->overseer_recno);
	visit<int16_t>(v, &f->overseer_town_recno);
	visit<int16_t>(v, &f->builder_recno);
	visit<uint8_t>(v, &f->builder_region_id);
	visit<float>(v, &f->productivity);
	visit_pointer(v, &f->worker_array);
	visit<int8_t>(v, &f->worker_count);
	visit<int8_t>(v, &f->selected_worker_id);
	visit<int8_t>(v, &f->player_spy_count);
	visit<uint8_t>(v, &f->sabotage_level);
	visit<int8_t>(v, &f->linked_firm_count);
	visit<int8_t>(v, &f->linked_town_count);
	visit_array<int16_t>(v, f->linked_firm_array, MAX_LINKED_FIRM_FIRM);
	visit_array<int16_t>(v, f->linked_town_array, MAX_LINKED_FIRM_TOWN);

	visit_array<int8_t>(v, f->linked_firm_enable_array,
							  MAX_LINKED_FIRM_FIRM);

	visit_array<int8_t>(v, f->linked_town_enable_array,
							  MAX_LINKED_FIRM_TOWN);

	visit<float>(v, &f->last_year_income);
	visit<float>(v, &f->cur_year_income);
	visit<int32_t>(v, &f->setup_date);
	visit<int8_t>(v, &f->should_set_power);
	visit<int32_t>(v, &f->last_attacked_date);
	visit<int8_t>(v, &f->should_close_flag);
	visit<int8_t>(v, &f->no_neighbor_space);
	visit<int8_t>(v, &f->ai_should_build_factory_count);
}

enum { FIRM_RECORD_SIZE = 254 };

static bool read_firm(File *file, Firm *firm)
{
	return read_with_record_size(file, firm, &visit_firm<FileReaderVisitor>, FIRM_RECORD_SIZE);
}

static bool write_firm(File *file, Firm *firm)
{
	return write_with_record_size(file, firm, &visit_firm<FileWriterVisitor>, FIRM_RECORD_SIZE);
}

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

			if (!write_firm(filePtr, firmPtr))
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
         //----- create firm object -----------//

         firmRecno = create_firm( firmId );
         firmPtr   = firm_array[firmRecno];

			if (!read_firm(filePtr, firmPtr))
			  return 0;

         //---- read data in base class -----//

			if(!GameFile::read_file_same_version && firmPtr->firm_id > FIRM_BASE)
				firmPtr->firm_build_id += MAX_RACE - VERSION_1_MAX_RACE;

         //--------- read worker_array ---------//

         if( firm_res[firmId]->need_worker )
         {
            firmPtr->worker_array = (Worker*) mem_add( MAX_WORKER*sizeof(Worker) );

            if( !filePtr->file_read( firmPtr->worker_array, MAX_WORKER*sizeof(Worker) ) )
               return 0;

            firmPtr->sort_worker(); // if this one selected, refresh interface
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

         if( !filePtr->file_write( townPtr, sizeof(Town) - Town::SIZEOF_NONSAVED_ELEMENTS ) )
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

	if(!GameFile::read_file_same_version)
	{
		memset(race_wander_pop_array, 0, sizeof(race_wander_pop_array));
		filePtr->file_read( race_wander_pop_array, sizeof(race_wander_pop_array[0])*VERSION_1_MAX_RACE );
	}
	else
		filePtr->file_read( race_wander_pop_array, sizeof(race_wander_pop_array) );

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

			if(!GameFile::read_file_same_version)
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
				if( !filePtr->file_read( townPtr, sizeof(Town) - Town::SIZEOF_NONSAVED_ELEMENTS ) )
					return 0;
			}

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


template <typename Visitor>
static void visit_nation_array(Visitor *v, NationArray *na)
{
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
		visit_array<int8_t>(v, na->human_name_array[n],
								  HUMAN_NAME_LEN+1);
}

enum { NATION_ARRAY_RECORD_SIZE = 288 };

static bool read_nation_array(File *file, NationArray *na)
{
	return read_with_record_size(file, na, &visit_nation_array<FileReaderVisitor>,
										  NATION_ARRAY_RECORD_SIZE);
}

//-------- Start of function NationArray::write_file -------------//
//
int NationArray::write_file(File* filePtr)
{
	//------ write info in NationArray ------//
	
	if (!write_with_record_size(filePtr, this, &visit_nation_array<FileWriterVisitor>,
										 NATION_ARRAY_RECORD_SIZE))
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

template <typename Visitor>
static void visit_version_1_nation_array(Visitor *v, Version_1_NationArray *na)
{
	visit<int16_t>(v, &na->nation_count);
	visit<int16_t>(v, &na->ai_nation_count);
	visit<int32_t>(v, &na->last_del_nation_date);
	visit<int32_t>(v, &na->last_new_nation_date);
	visit<int32_t>(v, &na->max_nation_population);
	visit<int32_t>(v, &na->all_nation_population);
	visit<int16_t>(v, &na->independent_town_count);
	visit_array<int16_t>(v, na->independent_town_count_race_array,
								VERSION_1_MAX_RACE);
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
		visit_array<int8_t>(v, na->human_name_array[n],
								  HUMAN_NAME_LEN+1);
}

enum { VERSION_1_NATION_ARRAY_RECORD_SIZE = 282 };

//-------- Start of function NationArray::read_file -------------//
//
int NationArray::read_file(File* filePtr)
{
   //------ read info in NationArray ------//
	if(!GameFile::read_file_same_version)
	{
		Version_1_NationArray *oldNationArrayPtr = (Version_1_NationArray*) mem_add(sizeof(Version_1_NationArray));
		if (!read_with_record_size(filePtr, oldNationArrayPtr,
											&visit_version_1_nation_array<FileReaderVisitor>,
											VERSION_1_NATION_ARRAY_RECORD_SIZE))
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
	visit_array<uint16_t>(v, v1n->last_unit_name_id_array,
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

enum { VERSION_1_NATION_RECORD_SIZE = 2182 };

static bool read_version_1_nation(File *file, Version_1_Nation *v1n)
{
	if (!read_with_record_size(file, v1n, &visit_version_1_nation<FileReaderVisitor>,
										VERSION_1_NATION_RECORD_SIZE))
		return false;

	memset(&v1n->action_array, 0, sizeof(v1n->action_array));
	return true;
}

template <typename Visitor>
static void visit_nation(Visitor *v, Nation *nat)
{
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
	visit_array<uint16_t>(v, nat->last_unit_name_id_array, MAX_UNIT_TYPE);
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

enum { NATION_RECORD_SIZE = 2202 };

//--------- Begin of function Nation::write_file ---------//
//
int Nation::write_file(File* filePtr)
{
	if (!write_with_record_size(filePtr, this, &visit_nation<FileWriterVisitor>,
										 NATION_RECORD_SIZE))
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

static bool read_nation(File *file, Nation *nat)
{
	if (!read_with_record_size(file, nat, &visit_nation<FileReaderVisitor>, NATION_RECORD_SIZE))
		return false;

	memset(&nat->action_array, 0, sizeof(nat->action_array));
	return true;
}

//--------- Begin of function Nation::read_file ---------//
//
int Nation::read_file(File* filePtr)
{
	if(!GameFile::read_file_same_version)
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

template <typename Visitor>
static void visit_tornado(Visitor *v, Tornado *t)
{
	visit_sprite(v, t);
   visit<float>(v, &t->attack_damage);
   visit<int16_t>(v, &t->life_time);
   visit<int16_t>(v, &t->dmg_offset_x);
   visit<int16_t>(v, &t->dmg_offset_y);
}

enum { TORNADO_RECORD_SIZE = 44 };

//--------- Begin of function Tornado::write_file ---------//
//
int Tornado::write_file(File* filePtr)
{
	return write_with_record_size(filePtr, this, &visit_tornado<FileWriterVisitor>,
											TORNADO_RECORD_SIZE);
}
//----------- End of function Tornado::write_file ---------//

//--------- Begin of function Tornado::read_file ---------//
//
int Tornado::read_file(File* filePtr)
{
	if (!read_with_record_size(filePtr, this, &visit_tornado<FileReaderVisitor>,
										TORNADO_RECORD_SIZE))
		return 0;

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
	filePtr->file_put_unsigned_short(29); // sizeof(DynArray)
	// write DynArray -- 29 bytes
	filePtr->file_put_long(ele_num);
	filePtr->file_put_long(block_num);
	filePtr->file_put_long(cur_pos);
	filePtr->file_put_long(last_ele);
	filePtr->file_put_long(15); // sizeof(Spy)
	filePtr->file_put_long(sort_offset);
	filePtr->file_put_char(sort_type);
	filePtr->file_put_long(0);

        //---------- write body_buf ---------//

	if( last_ele > 0 )
	{
		Spy spyBlank;
		memset(&spyBlank, 0, sizeof(spyBlank));

		filePtr->file_put_unsigned_short(15*last_ele); // sizeof(Spy)*last_ele

		for( int i=1; i<=last_ele; i++ )
		{
			Spy* spyPtr = (Spy*) get_ptr(i);
			if( !spyPtr )
				spyPtr = &spyBlank;

			// write Spy -- 15 bytes
			filePtr->file_put_short(spyPtr->spy_recno);
			filePtr->file_put_char(spyPtr->spy_place);
			filePtr->file_put_short(spyPtr->spy_place_para);
			filePtr->file_put_char(spyPtr->spy_skill);
			filePtr->file_put_char(spyPtr->spy_loyalty);
			filePtr->file_put_char(spyPtr->true_nation_recno);
			filePtr->file_put_char(spyPtr->cloaked_nation_recno);
			filePtr->file_put_char(spyPtr->notify_cloaked_nation_flag);
			filePtr->file_put_char(spyPtr->exposed_flag);
			filePtr->file_put_char(spyPtr->race_id);
			filePtr->file_put_unsigned_short(spyPtr->name_id);
			filePtr->file_put_char(spyPtr->action_mode);
		}
	}

	//---------- write empty_room_array ---------//

	write_empty_room(filePtr);

	return 1;
}
//--------- End of function SpyArray::write_file ---------------//


//-------- Start of function SpyArray::read_file -------------//
//
int SpyArray::read_file(File* filePtr)
{
	unsigned short recSize = filePtr->file_get_unsigned_short();
	if( recSize != 29 )
		return 0;

	int32_t fileEleNum = filePtr->file_get_long(); // skip overwriting ele_num
	resize(fileEleNum);
	filePtr->file_get_long(); // skip overwriting block_num
	filePtr->file_get_long(); // skip overwriting cur_pos
	int32_t readNum = filePtr->file_get_long(); // skip overwriting last_ele
	filePtr->file_get_long(); // skip overwriting ele_size
	filePtr->file_get_long(); // skip overwriting sort_offset
	filePtr->file_get_char(); // skip overwriting sort_type
	filePtr->file_get_long();

	//---------- read body_buf ---------//

	if( readNum > 0 )
	{
		filePtr->file_get_unsigned_short(); // skip body_buf record len

		for( int i=0; i<readNum; i++ )
		{
			int spy_recno = filePtr->file_get_short();

			if( spy_recno )
			{
				spy_recno = spy_array.add_spy();

				Spy* spyPtr = spy_array[spy_recno];
				//spyPtr->spy_recno = spy_recno;
				spyPtr->spy_place = filePtr->file_get_char();
				spyPtr->spy_place_para = filePtr->file_get_short();
				spyPtr->spy_skill = filePtr->file_get_char();
				spyPtr->spy_loyalty = filePtr->file_get_char();
				spyPtr->true_nation_recno = filePtr->file_get_char();
				spyPtr->cloaked_nation_recno = filePtr->file_get_char();
				spyPtr->notify_cloaked_nation_flag = filePtr->file_get_char();
				spyPtr->exposed_flag = filePtr->file_get_char();
				spyPtr->race_id = filePtr->file_get_char();
				spyPtr->name_id = filePtr->file_get_unsigned_short();
				spyPtr->action_mode = filePtr->file_get_char();
			}
			else
			{
				add_blank(1);     // it's a DynArrayB function

				// read 13 zeroed bytes
				filePtr->file_get_char(); //skip spy_place
				filePtr->file_get_short(); //skip spy_place_para
				filePtr->file_get_char(); //skip spy_skill
				filePtr->file_get_char(); // skip spy_loyalty
				filePtr->file_get_char(); // skip true_nation_recno
				filePtr->file_get_char(); // skip cloaked_nation_recno
				filePtr->file_get_char(); // skip notify_cloaked_nation_flag
				filePtr->file_get_char(); // skip exposed_flag
				filePtr->file_get_char(); // skip race_id
				filePtr->file_get_unsigned_short(); // skip name_id
				filePtr->file_get_char(); // skip action_mode
			}
		}
	}

	//---------- read empty_room_array ---------//

	read_empty_room(filePtr);

	//------------------------------------------//

	start();    // go top

	return 1;
}
//--------- End of function SpyArray::read_file ---------------//


//*****//


//-------- Start of function SnowGroundArray::write_file -------------//
//
int SnowGroundArray::write_file(File* filePtr)
{
	MSG(__FILE__":%d: file_write(this, ...);\n", __LINE__);

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

template <typename Visitor>
static void visit_region_array(Visitor *v, RegionArray *ra)
{
	visit<int32_t>(v, &ra->init_flag);
	visit_pointer(v, &ra->region_info_array);
	visit<int32_t>(v, &ra->region_info_count);
	visit_pointer(v, &ra->region_stat_array);
	visit<int32_t>(v, &ra->region_stat_count);
	visit_pointer(v, &ra->connect_bits);
	visit_array<uint8_t>(v, ra->region_sorted_array, MAX_REGION);
}

enum { REGION_ARRAY_RECORD_SIZE = 279 };

//-------- Start of function RegionArray::write_file -------------//
//
int RegionArray::write_file(File* filePtr)
{
	if (!write_with_record_size(filePtr, this, &visit_region_array<FileWriterVisitor>,
										 REGION_ARRAY_RECORD_SIZE))
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
	if (!read_with_record_size(filePtr, this, &visit_region_array<FileReaderVisitor>,
										REGION_ARRAY_RECORD_SIZE))
		return 0;

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
/* vim:set ts=3 sw=3: */
