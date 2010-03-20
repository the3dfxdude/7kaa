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

//Filename    : OROCKRES.H
//Description : Header file of object RockRes
//Owner       : Gilbert


#ifndef __OROCKRES_H
#define __OROCKRES_H


#include <GAMEDEF.h>
#include <ORESDB.h>

// ----------- Define constant ----------//
#define ROCK_BLOCKING_TYPE 'R'
#define DIRT_NON_BLOCKING_TYPE 'D'
#define DIRT_BLOCKING_TYPE 'E'

#define MAX_ROCK_WIDTH 4
#define MAX_ROCK_HEIGHT 4

// ----------- Define struct RockRec -----------//

struct RockRec
{
	enum { ROCKID_LEN=5, LOC_LEN=2, RECNO_LEN=4, MAX_FRAME_LEN=2 };
	char rock_id[ROCKID_LEN];
	char rock_type;
	char loc_width[LOC_LEN];
	char loc_height[LOC_LEN];
	char terrain_1;
	char terrain_2;
	char first_anim_recno[RECNO_LEN];
	char max_frame[MAX_FRAME_LEN];
};

// ----------- Define struct RockBlockRec ---------//

struct RockBlockRec
{
	enum { ROCKID_LEN=5, LOC_LEN=2, RECNO_LEN=4 };
	char rock_id[ROCKID_LEN];
	char loc_x[LOC_LEN];
	char loc_y[LOC_LEN];
	char rock_recno[RECNO_LEN];
	char first_bitmap[RECNO_LEN];
};

// ------------ Define struct RockBitmapRec ---------//

struct RockBitmapRec
{
	enum { ROCKID_LEN=5, LOC_LEN=2, FRAME_NO_LEN=2, FILE_NAME_LEN=8,
		BITMAP_PTR_LEN=4};
	char rock_id[ROCKID_LEN];
	char loc_x[LOC_LEN];
	char loc_y[LOC_LEN];
	char frame[FRAME_NO_LEN];
	char file_name[FILE_NAME_LEN];
	char bitmap_ptr[BITMAP_PTR_LEN];
};

// ------------ Define struct RockAnimRec --------//

struct RockAnimRec
{
	enum { ROCKID_LEN=5, FRAME_NO_LEN=2, DELAY_LEN=3 };
	char rock_id[ROCKID_LEN];
	char frame[FRAME_NO_LEN];
	char delay[DELAY_LEN];
	char next_frame[FRAME_NO_LEN];
	char alt_next[FRAME_NO_LEN];
};

// ----------- Define struct RockInfo -----------//

struct RockInfo
{
	enum	{ ROCKID_LEN=5 };
	char	rock_name[ROCKID_LEN+1];
	char	rock_type;
	char	loc_width;
	char	loc_height;
	char	terrain_1;			// TerrainTypeCode
	char	terrain_2;			// TerrainTypeCode
	short	first_anim_recno;
	char	max_frame;
	short	first_block_recno;
	short block_offset[MAX_ROCK_HEIGHT][MAX_ROCK_WIDTH];	// make RockRes::locate_block faster

	int	valid_terrain(char terrainType) { return terrainType >= terrain_1 && terrainType <= terrain_2; }
};

// ----------- Define struct RockBlockInfo ---------//
struct RockBlockInfo
{
	char	loc_x;
	char	loc_y;
	short	rock_recno;			// recno in RockRec/RockInfo
	short	first_bitmap;     // recno in RockBitmapRec/RockBitmapInfo
};

// ------------ Define struct RockBitmapInfo ---------//

struct RockBitmapInfo
{
	char	loc_x;		// checking only
	char	loc_y;		// checking only
	char	frame;		// checking only
	char*	bitmap_ptr;

	short width()     { return *(short *)bitmap_ptr; }
	short height()    { return *(((short *)bitmap_ptr)+1); }
	void	draw(short xLoc, short yLoc);
};

// ------------ Define struct RockAnimInfo --------//

struct RockAnimInfo
{
	char	frame;			// checking only
	char	delay;
	char	next_frame;
	char	alt_next;

	char choose_next(long path) { return path ? next_frame : alt_next; }
};


// ------------ Define class RockRes -----------//

class RockRes
{
public:
	int              init_flag;

	int              rock_info_count;
	RockInfo*        rock_info_array;

	int              rock_block_count;
	RockBlockInfo*   rock_block_array;

	int              rock_bitmap_count;
	RockBitmapInfo*  rock_bitmap_array;

	int              rock_anim_count;
	RockAnimInfo*    rock_anim_array;

	ResourceDb       res_bitmap;


public:
	RockRes();
	~RockRes();
	void             init();
	void             deinit();

	RockInfo*        get_rock_info(short rockRecno);
	RockBlockInfo*   get_block_info(short rockBlockRecno);
	RockBitmapInfo*  get_bitmap_info(short rockBitmapRecno);
	RockAnimInfo*    get_anim_info(short rockAnimRecno);

	char             choose_next(short rockRecno, char curFrame, long path);
	void             draw(short rockRecno, short xLoc, short yLoc, char curFrame);
	void             draw_block(short rockRecno, short xLoc, short yLoc,
		short offsetX, short offsetY, char curFrame);

	RockBlockInfo*   operator[](short rockBlockRecno)
		{ return rock_block_array + rockBlockRecno -1; }

	// ###### begin Gilbert 24/9 ########//
	// return rockRecno
	short            search(const char *rockTypes, short minWidth, short maxWidth, 
		short minHeight, short maxHeight, int animatedFlag=-1, int findFirst=0,
		char terrainType=0);
	// ###### end Gilbert 24/9 ########//
	short            locate(char *rockName);

	// return rockBlockRecno
	short            locate_block(short rockRecno, short xLoc, short yLoc);

	// return rockBitmapRecno
	short            get_bitmap_recno(short rockBlockRecno, char curFrame);

	// return rockAnimRecno
	short            get_anim_recno(short rockRecno, char curFrame);

private:
	void             load_info();
	void             load_block_info();
	void             load_bitmap_info();
	void             load_anim_info();
};

extern RockRes rock_res;

#endif
