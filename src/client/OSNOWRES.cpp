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

// Filename    : OSNOWRES.CPP
// Description : Snow Resource
// Onwer       : Gilbert


#include <OSNOWRES.h>
#include <OVGABUF.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <ALL.h>


// --------- define constant --------//

#define SNOW_DB	"SNOWG"


// ------- Begin of function SnowInfo::draw_at ------//
void SnowInfo::draw_at(short absX, short absY)
{
	//----------- calculate absolute positions ------------//

	int absX1   = absX + offset_x;
	int absY1   = absY + offset_y;

	//----------- use fast method for non-gate square
	short bitmapWidth = bitmap_width();
	short bitmapHeight = bitmap_height();
	int absX2 = absX1 + bitmapWidth  - 1;
	int absY2 = absY1 + bitmapHeight - 1;

	//-------- check if the firm is within the view area --------//

	int x1 = absX1 - World::view_top_x;
	if( x1 <= -bitmapWidth || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = absY1 - World::view_top_y;
	if( y1 <= -bitmapHeight || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//

	int x2 = absX2 - World::view_top_x;
	int y2 = absY2 - World::view_top_y;

	//---- only portion of the sprite is inside the view area ------//
	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		// no put_bitmap_area_remap
		vga_back.put_bitmap_area_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
			MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1);
	}
	else
	//---- the whole sprite is inside the view area ------//
	{
		vga_back.put_bitmap_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr);
	}
}
// ------- End of function SnowInfo::draw_at ------//


// ------- Begin of function SnowRes::SnowRes ------//
SnowRes::SnowRes()
{
	init_flag = 0;
}
// ------- End of function SnowRes::SnowRes ------//


// ------- Begin of function SnowRes::~SnowRes ------//
SnowRes::~SnowRes()
{
	deinit();
}
// ------- End of function SnowRes::~SnowRes ------//


// ------- Begin of function SnowRes::init ------//
void SnowRes::init()
{
	deinit();

	//----- open snow ground bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_SNOW.RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_info();

	init_flag=1;
}
// ------- End of function SnowRes::init ------//


// ------- Begin of function SnowRes::deinit ------//
void SnowRes::deinit()
{
	if( init_flag )
	{
		mem_del( snow_info_array );
		mem_del( root_info_array );
		init_flag = 0;
	}
}
// ------- End of function SnowRes::deinit ------//


// ------- Begin of function SnowRes::load_info ------//
void SnowRes::load_info()
{
	SnowRec		*snowRec;
	SnowInfo		*snowInfo;
	int			i;
	long			bitmapOffset;
	
	//---- read in snow map count and initialize snow info array ----//

	Database *dbSnow = game_set.open_db(SNOW_DB);	// only one database can be opened at a time

	snow_info_count = (int) dbSnow->rec_count();
	snow_info_array = (SnowInfo *) mem_add(sizeof(SnowInfo) * snow_info_count );

	memset( snow_info_array, 0, sizeof(SnowInfo) * snow_info_count );
	root_count = 0;

	//---------- read in SNOWG.DBF ---------//

	for( i=0 ; i<snow_info_count; i++ )
	{
		snowRec = (SnowRec *) dbSnow->read(i+1);
		snowInfo = snow_info_array+i;

		snowInfo->snow_map_id = i+1;
		memcpy( &bitmapOffset, snowRec->bitmap_ptr, sizeof(long) );
		snowInfo->bitmap_ptr	= res_bitmap.read_imported(bitmapOffset);
		if( snowRec->offset_x[0] != '\0' && snowRec->offset_x[0] != ' ')
			snowInfo->offset_x = (short) m.atoi(snowRec->offset_x, snowRec->OFFSET_LEN);
		else
			snowInfo->offset_x = -(snowInfo->bitmap_width() / 2);
		if( snowRec->offset_y[0] != '\0' && snowRec->offset_y[0] != ' ')
			snowInfo->offset_y = (short) m.atoi(snowRec->offset_y, snowRec->OFFSET_LEN);
		else
			snowInfo->offset_y = -(snowInfo->bitmap_height() / 2);

		snowInfo->next_count = 0;
		snowInfo->prev_count = 0;

		if( snowRec->next_file1[0] != '\0' && snowRec->next_file1[0] != ' ')
		{
			snowInfo->next_file[snowInfo->next_count++] = snow_info_array + m.atoi( snowRec->next_ptr1, snowRec->RECNO_LEN) -1;
		}
		if( snowRec->next_file2[0] != '\0' && snowRec->next_file2[0] != ' ')
		{
			snowInfo->next_file[snowInfo->next_count++] = snow_info_array + m.atoi( snowRec->next_ptr2, snowRec->RECNO_LEN) -1;
		}
		if( snowRec->next_file3[0] != '\0' && snowRec->next_file3[0] != ' ')
		{
			snowInfo->next_file[snowInfo->next_count++] = snow_info_array + m.atoi( snowRec->next_ptr3, snowRec->RECNO_LEN) -1;
		}
		if( snowRec->next_file4[0] != '\0' && snowRec->next_file4[0] != ' ')
		{
			snowInfo->next_file[snowInfo->next_count++] = snow_info_array + m.atoi( snowRec->next_ptr4, snowRec->RECNO_LEN) -1;
		}
		if( snowRec->prev_file1[0] != '\0' && snowRec->prev_file1[0] != ' ')
		{
			snowInfo->prev_file[snowInfo->prev_count++] = snow_info_array + m.atoi( snowRec->prev_ptr1, snowRec->RECNO_LEN) -1;
		}
		if( snowRec->prev_file2[0] != '\0' && snowRec->prev_file2[0] != ' ')
		{
			snowInfo->prev_file[snowInfo->prev_count++] = snow_info_array + m.atoi( snowRec->prev_ptr2, snowRec->RECNO_LEN) -1;
		}

		if(snowInfo->is_root())
		{
			root_count++;
		}
	}

	root_info_array = (SnowInfo **)mem_add(sizeof(SnowInfo *) * root_count);
	unsigned j = 0;
	for( i=0 ; i<snow_info_count; i++ )
	{
		snowInfo = snow_info_array+i;
		if(snowInfo->is_root())
		{
			root_info_array[j++] = snowInfo;
			if( j >= root_count)
				break;
		}
	}
}
// ------- End of function SnowRes::load_info ------//


// ------- Begin of function SnowRes::rand_root ------//
int SnowRes::rand_root(unsigned rand)
{
	err_when(!init_flag);
	if( root_count > 0)
	{
		return root_info_array[rand % root_count]->snow_map_id;
	}
	return NULL;
}
// ------- End of function SnowRes::rand_root ------//

#ifdef DEBUG
// ------- Begin of function SnowRes::operator[] ------//
SnowInfo * SnowRes::operator[](int snowMapId)
{
	err_when( snowMapId < 1 || snowMapId > snow_info_count );
	err_when( snow_info_array[snowMapId-1].snow_map_id != snowMapId);
	return snow_info_array + snowMapId -1;
}
// ------- End of function SnowRes::operator[] ------//
#endif
