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

// Filename    : OHILLRES.H
// Description : Header file for class HillRes
// Ownership   : Gilbert


#ifndef __OHILLRES_H
#define __OHILLRES_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//---------- Define constant -------//
enum
{
	HIGH_HILL_PRIORITY = 7,
	LOW_HILL_PRIORITY = 3,
};

//---------- Define struct HillBlockRec ------//

// ####### begin Gilbert 28/1 #######//
struct HillBlockRec
{
	enum	{ PATTERN_ID_LEN = 3, SUB_PATTERN_ID_LEN =3, PRIORITY_LEN = 1, 
		OFFSET_LEN=3, FILE_NAME_LEN = 8, BITMAP_PTR_LEN = 4 };
	char	pattern_id[PATTERN_ID_LEN];
	char	sub_pattern_id[SUB_PATTERN_ID_LEN];
	char	priority[PRIORITY_LEN];
	char	special_flag;
	char	layer;
	char	bitmap_type;
	char	offset_x[OFFSET_LEN];
	char	offset_y[OFFSET_LEN];
	char	file_name[FILE_NAME_LEN];
	char	bitmap_ptr[BITMAP_PTR_LEN];
};

//---------- Define struct HillBlockInfo ------//
struct HillBlockInfo
{
	short	block_id;
	char	pattern_id;
	char	sub_pattern_id;
	char	special_flag;
	char	priority;		// high value on top
	char	layer;			// 1= draw together with backgroud, 2= sort together with units
	char	bitmap_type;	// W=whole square; T=transparent; O=oversize
	short	offset_x;
	short	offset_y;
	char* bitmap_ptr;

public:
	short	bitmap_width()		{ return *(short *)bitmap_ptr; }
	short bitmap_height()	{ return *(((short *)bitmap_ptr)+1); }

	void	draw(int xLoc, int yLoc, int layerMask=-1);
	void 	draw_at(int absBaseX, int absBaseY, int layerMask=-1);
};

//----------- Define class HillRes ---------------//

class HillRes
{
public:
	HillBlockInfo*	hill_block_info_array;
	short			hill_block_count;

	short	*		first_block_index;		// array index of first hill block of each pattern
	short			max_pattern_id;

	char			init_flag;
	ResourceDb	res_bitmap;

public:
	HillRes();

	void			init();
	void			deinit();

	// find exact
	short			locate(char patternId, char subPattern, char searchPriority, char specialFlag);

	// return hill block id of any one of the subPattern
	short			scan(char patternId, char searchPriority, char specialFlag, char findFirst);

	//-------- function related to HillBlock ---------//
	HillBlockInfo*	operator[](int hillBlockId);
	short first_block(int hillPatternId);

private:
	void			load_hill_block_info();
};
// ####### end Gilbert 28/1 #######//

extern HillRes hill_res;

//----------------------------------------------------//

#endif
