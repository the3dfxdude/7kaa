#ifndef VISIT_SPRITE_H_
/*
* Seven Kingdoms: Ancient Adversaries
*
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

#define VISIT_SPRITE_H_
#define VISIT_SPRITE_H_

#include <file_io_visitor.h>


template <typename Visitor>
static void visit_sprite_members(Visitor *v, Sprite *s)
{
	using namespace FileIOVisitor;

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

#endif // !VISIT_SPRITE_H_
