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

// Filename    : OFIRERES.H
// Description : Fire bitmap

#ifndef __FIRERES_H
#define __FIRERES_H

#include <OFLAME.h>
#include <ORESDB.h>

#define FIRE_FRAME_MAX 20

struct FireResRec
{
	enum { FIRE_ID_LEN = 3, FIRE_GRADE_LEN = 2, FRAME_NO_LEN = 2, OFFSET_LEN=4, FILE_NAME_LEN = 8, BITMAP_PTR_LEN = 4 };
	char fire_id[FIRE_ID_LEN];
	char fire_grade[FIRE_ID_LEN];
	char frame[FRAME_NO_LEN];
	char offset_x[OFFSET_LEN];
	char offset_y[OFFSET_LEN];
	char file_name[FILE_NAME_LEN];
	char bitmap_ptr[BITMAP_PTR_LEN];
};


struct FireResInfo
{
	int	max_frame[FLAME_GROW_STEP];
	char *bitmap_offset[FLAME_GROW_STEP][FIRE_FRAME_MAX];
	short offset_x[FLAME_GROW_STEP][FIRE_FRAME_MAX];
	short offset_y[FLAME_GROW_STEP][FIRE_FRAME_MAX];
};


class FireRes
{
public:
	int	init_flag;
	int	fire_info_count;
	struct FireResInfo *fire_info_array;
	ResourceDb       res_bitmap;

public:
	FireRes();
	~FireRes();
	void	init();
	void	deinit();

	FireResInfo *get_fire_info(int fireId);
	int	get_max_frame(int fireId, int fireGrade);
	char *get_fire_bitmap(int fireId, int fireGrade, int frameNo,
		short *offsetX, short *offsetY);

private:
	void	load_info();
};

#endif