/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
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

#include <OBULLET.h>
#include <OB_FLAME.h>
#include <OB_HOMIN.h>
#include <OB_PROJ.h>
#include <file_io_visitor.h>
#include <visit_sprite.h>
#include <OGFILE_DYNARRAYB.inl>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_bullet_members(Visitor *v, Bullet *b)
{
	visit_sprite_members(v, b);
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

template <typename Visitor>
static void visit_projectile_members(Visitor *v, Projectile *p)
{
	visit<float>(v, &p->z_coff);
	visit_sprite_members(v, &p->act_bullet);
	visit_sprite_members(v, &p->bullet_shadow);
}

template <typename Visitor>
static void visit_bullet_homing_members(Visitor* v, BulletHoming* c)
{
	visit<int8_t>(v, &c->max_step);
	visit<int8_t>(v, &c->target_type);
	visit<int16_t>(v, &c->target_recno);
	visit<int16_t>(v, &c->speed);
	visit<int16_t>(v, &c->origin2_x);
	visit<int16_t>(v, &c->origin2_y);
}


// ===============================================================================

void Bullet::accept_file_visitor(FileReaderVisitor* v)
{
	visit_bullet_members(v, this);

	//------------ post-process the data read ----------//

	sprite_info = sprite_res[sprite_id];
	sprite_info->load_bitmap_res();
}

void Bullet::accept_file_visitor(FileWriterVisitor* v)
{
	visit_bullet_members(v, this);
}

enum { BULLET_PROJECTILE_DERIVED_RECORD_SIZE = 72 };

void Projectile::accept_file_visitor(FileReaderVisitor* v)
{
	Bullet::accept_file_visitor(v);
	v->with_record_size(BULLET_PROJECTILE_DERIVED_RECORD_SIZE);
	visit_projectile_members(v, this);

	//----------- post-process the data read ----------//

	act_bullet.sprite_info = sprite_res[act_bullet.sprite_id];
	act_bullet.sprite_info->load_bitmap_res();
	bullet_shadow.sprite_info = sprite_res[bullet_shadow.sprite_id];
	bullet_shadow.sprite_info->load_bitmap_res();
}

void Projectile::accept_file_visitor(FileWriterVisitor* v)
{
	Bullet::accept_file_visitor(v);
	v->with_record_size(BULLET_PROJECTILE_DERIVED_RECORD_SIZE);
	visit_projectile_members(v, this);
}

enum { BULLET_HOMING_DERIVED_RECORD_SIZE = 10 };

void BulletHoming::accept_file_visitor(FileReaderVisitor* v)
{
	Bullet::accept_file_visitor(v);
	v->with_record_size(BULLET_HOMING_DERIVED_RECORD_SIZE);
	visit_bullet_homing_members(v, this);
}

void BulletHoming::accept_file_visitor(FileWriterVisitor* v)
{
	Bullet::accept_file_visitor(v);
	v->with_record_size(BULLET_HOMING_DERIVED_RECORD_SIZE);
	visit_bullet_homing_members(v, this);
}


enum { BULLET_RECORD_SIZE = 57 };

template <typename Visitor>
static void visit_bullet_array(Visitor* v, BulletArray* b)
{
	visit<int16_t>(v, &b->restart_recno);
	b->accept_visitor_as_ptr_array<Bullet>(v, [] (Bullet* bullet) {return bullet->sprite_id;},
		BulletArray::create_bullet, polymorphic_visit<Visitor, Bullet>, BULLET_RECORD_SIZE);
}


//-------- Start of function BulletArray::write_file -------------//
//
int BulletArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_bullet_array(&v, this);
	return v.good();
}
//--------- End of function BulletArray::write_file -------------//


//-------- Start of function BulletArray::read_file -------------//
//
int BulletArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_bullet_array(&v, this);
	return v.good();
}
//--------- End of function BulletArray::read_file ---------------//
