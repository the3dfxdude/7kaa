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

//Filename    : OSPRITE.CPP
//Description : Object Sprite

#include <ALL.h>
#include <OSTR.h>
#include <OVGA.h>
#include <OINFO.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OSPRITE.h>
#include <OCOLTBL.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Graphics);

//----------- Define static class member variables -----------//

short Sprite::abs_x1, Sprite::abs_y1;		// the absolute postion, taking in account of sprite offset
short Sprite::abs_x2, Sprite::abs_y2;


//-------- Begin of function Sprite::Sprite --------//

Sprite::Sprite()
{
	// preserve virtual pointer
	memset( (char *)this + sizeof(void *), 0, sizeof(Sprite) - sizeof(void *) );
}
//--------- End of function Sprite::Sprite --------//


//-------- Begin of function Sprite::~Sprite --------//

Sprite::~Sprite()
{
 	deinit();
}
//--------- End of function Sprite::~Sprite --------//


//--------- Begin of function Sprite::init ---------//
//
void Sprite::init(short spriteId, short startXLoc, short startYLoc)
{
	sprite_id = spriteId;

	cur_x = startXLoc * ZOOM_LOC_WIDTH;
	cur_y = startYLoc * ZOOM_LOC_HEIGHT;

	go_x = next_x = cur_x;
	go_y = next_y = cur_y;

	cur_attack = 0;

	cur_action = SPRITE_IDLE;
	cur_dir 	  = m.random(MAX_SPRITE_DIR_TYPE);	// facing any of the eight directions
	cur_frame  = 1;
	final_dir  = cur_dir;

	//----- clone vars from sprite_res for fast access -----//

	sprite_info = sprite_res[sprite_id];

	sprite_info->load_bitmap_res();

	//------------- init other vars --------------//

	remain_attack_delay = 0;
	remain_frames_per_step = sprite_info->frames_per_step;
}
//----------- End of function Sprite::init -----------//


//--------- Begin of function Sprite::deinit ---------//
//
void Sprite::deinit()
{
	if( sprite_id && cur_x >= 0 )
	{
		sprite_info->free_bitmap_res();
		sprite_id = 0;
	}
}
//----------- End of function Sprite::deinit -----------//


//--------- Begin of function Sprite::cur_sprite_frame ---------//
//
// Return the current frame of the sprite
//
SpriteFrame* Sprite::cur_sprite_frame(int *needMirror)
{
	UCHAR curDir = display_dir();
	if( needMirror)
		*needMirror = need_mirror(curDir);

	// do not update cur_dir as curDir
	err_when(curDir<0 || curDir>=3*MAX_SPRITE_DIR_TYPE);

	switch( cur_action )
	{
		case SPRITE_MOVE:
		//### begin alex 14/4 ###//
		case SPRITE_SHIP_EXTRA_MOVE:
		//#### end alex 14/4 ####//
			if( guard_count)
			{
				if( curDir >= MAX_SPRITE_DIR_TYPE)
				{
					err_here();
					curDir %= MAX_SPRITE_DIR_TYPE;
				}
				return sprite_frame_res[sprite_info->guard_move_array[curDir].first_frame_recno+cur_frame-1];
			}
			else
				return sprite_frame_res[sprite_info->move_array[curDir].first_frame_recno+cur_frame-1];

		case SPRITE_ATTACK:
			err_when(curDir<0 || curDir>=MAX_SPRITE_DIR_TYPE);
			if( guard_count )
			{
				SpriteGuardStop *guardStopAction = sprite_info->guard_stop_array + curDir;
				return sprite_frame_res[guardStopAction->first_frame_recno+
					MIN(guard_count,guardStopAction->frame_count)-1];
			}
			else
				return sprite_frame_res[sprite_info->attack_array[cur_attack][curDir].first_frame_recno+cur_frame-1];

		case SPRITE_TURN:
		case SPRITE_IDLE:
		case SPRITE_WAIT:
			// air unit needs it own stop frames to float on air
			{
				if( guard_count )
				{
					if( curDir >= MAX_SPRITE_DIR_TYPE)
					{
						// if the sprite is turning, adjust direction to next
						if( turn_delay > 0)
							curDir ++;
						curDir %= MAX_SPRITE_DIR_TYPE;
					}

					SpriteGuardStop *guardStopAction = sprite_info->guard_stop_array + curDir;
					return sprite_frame_res[guardStopAction->first_frame_recno+
						MIN(guard_count,guardStopAction->frame_count)-1];
				}
				else
				{
					SpriteStop *stopAction= sprite_info->stop_array +curDir;
					if(cur_frame > stopAction->frame_count)
						return sprite_frame_res[stopAction->frame_recno];       // first frame
					else            // only few sprite has stopAction->frame_count > 1
						return sprite_frame_res[stopAction->frame_recno+cur_frame-1];
				}
			}

		case SPRITE_DIE:
			if(sprite_info->die.first_frame_recno)			// only if this sprite has dying frame
			{
				if( needMirror)
					*needMirror = 0;			// no need to mirror at any direction
				return sprite_frame_res[sprite_info->die.first_frame_recno+cur_frame-1];
			}

		default:
			return sprite_frame_res[sprite_info->move_array[curDir].first_frame_recno+cur_frame-1];
	}
}
//----------- End of function Sprite::cur_sprite_frame -----------//


//--------- Begin of function Sprite::update_abs_pos ---------//
//
// Update the cur_width & cur_height vars of the sprite for later faster access.
//
// [SpriteFrame *] spriteFrame        pointer to the current (default : NULL)
void Sprite::update_abs_pos(SpriteFrame *spriteFrame)
{
	if( !spriteFrame )
		spriteFrame = cur_sprite_frame();

	abs_x1 = cur_x + spriteFrame->offset_x;		// absolute position 
	abs_y1 = cur_y + spriteFrame->offset_y;

	abs_x2 = abs_x1 + spriteFrame->width  - 1;
	abs_y2 = abs_y1 + spriteFrame->height - 1;
}
//----------- End of function Sprite::update_abs_pos -----------//


//--------- Begin of function Sprite::draw ---------//
//
void Sprite::draw()
{
	//--------- draw sprite on the zoom window ---------//

	int needMirror;
	SpriteFrame* spriteFrame = cur_sprite_frame(&needMirror);
	update_abs_pos(spriteFrame);

    if (!sprite_info->res_bitmap.initialized())
        ERR("[Sprite::draw] trying to read from uninitialized resource\n");

	char* bitmapPtr = sprite_info->res_bitmap.read_imported(spriteFrame->bitmap_offset);

	//-------- check if the sprite is inside the view area --------//

	int x1 =	abs_x1-World::view_top_x;	// the sprite's position in the view window

	if( x1 <= -spriteFrame->width || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 =	abs_y1-World::view_top_y;

	if( y1 <= -spriteFrame->height || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//

	int x2 = abs_x2-World::view_top_x;
	int y2 = abs_y2-World::view_top_y;

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		if( needMirror )	// if this direction needed to be mirrored
		{
			if( !sprite_info->remap_bitmap_flag )
			{
				vga_back.put_bitmap_area_trans_decompress_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
			}
			else
			{
				vga_back.remap_bitmap_area_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
					vga.vga_color_table->get_table_array(),
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
			}
		}
		else
		{
			if( !sprite_info->remap_bitmap_flag )
			{
				vga_back.put_bitmap_area_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
			}
			else
			{
				vga_back.remap_bitmap_area( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
					vga.vga_color_table->get_table_array(),
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
			}
		}
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		//------ mirror-bilting for certain directions ------//

		if( needMirror )  // if this direction needed to be mirrored
		{
			if( !sprite_info->remap_bitmap_flag )
			{
				vga_back.put_bitmap_trans_decompress_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1,
					bitmapPtr );
			}
			else
			{
				vga_back.remap_bitmap_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1,
					bitmapPtr, vga.vga_color_table->get_table_array() );
			}
		}
		else
		{
			if( !sprite_info->remap_bitmap_flag )
			{
				vga_back.put_bitmap_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1,
					bitmapPtr );
			}
			else
			{
				vga_back.remap_bitmap( x1+ZOOM_X1, y1+ZOOM_Y1,
					bitmapPtr, vga.vga_color_table->get_table_array() );
			}
		}
	}
}
//----------- End of function Sprite::draw -----------//


// ---------- Begin of function Sprite::display_dir ---------//
UCHAR Sprite::display_dir()
{
	UCHAR curDir = cur_dir;
	switch( sprite_info->turn_resolution)
	{
	case 0:		// fall through
	case 1:
		curDir &= ~7;		// direction less, remain upward or downard, but set to north
		break;
	case 8:
		// cur_dir can be 0 to 3*MAX_SPRITE_DIR_TYPE-1, such as projectile;
		// curDir = cur_dir;
		break;
	case 16:
		err_when(curDir<0 || curDir>=MAX_SPRITE_DIR_TYPE);
		// curDir should be (from due north, clockwisely) { 0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15 }
		if( turn_delay <= -30)
		{
			curDir = ((curDir+7) & 7) + 8;
		}
		else if( turn_delay >= 30)
		{
			curDir += 8;
		}
		break;
	case 24:
		err_when(curDir<0 || curDir>=MAX_SPRITE_DIR_TYPE);
		// curDir should be (from due north, clockwisely) 
		// { 0,8,16,1,9,17,2,10,18,3,11,19,4,12,20,5,13,21,6,14,22,7,15,23 }
		if( turn_delay <= -20)
		{
			if( turn_delay <= -40)
				curDir = ((curDir+7) & 7) + 8;
			else
				curDir = ((curDir+7) & 7) + 16;
		}
		else if( turn_delay >= 20)
		{
			if( turn_delay >= 40 )
				curDir += 16;
			else
				curDir += 8;
		}
		break;
	default:
		err_here();
	}
	return curDir;
}
// ---------- End of function Sprite::display_dir ---------//


// ---------- Begin of function Sprite::need_mirror --------//
int Sprite::need_mirror(UCHAR dispDir)
{
	return (dispDir < 8 || sprite_info->turn_resolution <= 8) ? (dispDir & 7) >= 5 : (dispDir & 7) >= 4;
}
// ---------- End of function Sprite::need_mirror --------//


// ---------- Begin of function Sprite::is_shealth --------//
int Sprite::is_shealth()
{
	// if the visibility of location is just explored, consider shealth
	return config.fog_of_war && world.get_loc(cur_x_loc(), cur_y_loc())->visibility() <= EXPLORED_VISIBILITY;
}
// ---------- End of function Sprite::is_shealth --------//
