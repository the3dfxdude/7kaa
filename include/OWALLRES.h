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

//Filename    : OWALLRES.H
//Description : Header file of object WallRes
//Ownership   : Gilbert

#ifndef __OWALLRES_H
#define __OWALLRES_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//------------ Define constant -------------//

#define	WALL_SPACE_LOC 		 5		// no. of space between the wall and the town border

#define  BUILD_WALL_COST    	10
#define  DESTRUCT_WALL_COST    5

//------------ Define struct WallRec ---------------//

struct WallRec
{
	enum	{ WALL_ID_LEN=2, OFFSET_LEN=4, LOC_OFF_LEN=2, FILE_NAME_LEN=8, BITMAP_PTR_LEN=4 };

	char	wall_id[WALL_ID_LEN];
	char	gate_flag;
	char	trans_flag;
	char	offset_x[OFFSET_LEN];
	char	offset_y[OFFSET_LEN];
	char	loc_off_x[LOC_OFF_LEN];
	char	loc_off_y[LOC_OFF_LEN];
	char	draw_wall[WALL_ID_LEN];
	char	file_name[FILE_NAME_LEN];
	char	bitmap_ptr[BITMAP_PTR_LEN];
};

//------------- Define struct WallInfo --------------//

struct WallInfo
{
	char	wall_id;
	char	flags;		// b0 = gate_flag, b1 = trans_flag
	short	offset_x;	// pixels to the drawing pixel
	short offset_y;
	char	loc_off_x;	// location to the top left location
	char	loc_off_y;

	char	draw_wall_id;
	char	dummy;
	char* bitmap_ptr;


public:
	int	is_gate()			{ return flags & 1; }
	int	is_trans()			{ return flags & 2; }
	void	set_gate()			{ flags |= 1;}
	void	set_trans()			{ flags |= 2;}
	short	bitmap_width()		{ return *(short *)bitmap_ptr; }
	short bitmap_height()	{ return *(((short *)bitmap_ptr)+1); }

	void	draw(int xLoc, int yLoc, char *remapTbl = NULL);
	void 	draw_at(int absBaseX, int absBaseY, char *remapTbl = NULL);
};

//----------- Define class WallRes ---------------//

class WallRes
{
public:
	WallInfo*	wall_info_array;
	WallInfo**	wall_index;				// wallInfoPtr = wall_index[wall_id-1]; 
	short			wall_count;
	char			max_wall_id;
	char			init_flag;
	ResourceDb	res_bitmap;

   short			selected_x_loc;
	short			selected_y_loc;

public:
	WallRes();

	void			init();
	void			deinit();

	void			disp_info(int refreshFlag);
	void			draw_selected();

	WallInfo*   operator[](int WallId);

private:
	void			load_wall_info();
};

extern WallRes wall_res;

//----------------------------------------------------//

#endif
