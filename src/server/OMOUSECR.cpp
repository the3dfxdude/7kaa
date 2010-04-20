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

//Filename    : OMOUSECR.CPP
//Description : Object Cursor Resource

#include <ALL.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OWORLD.h>
#include <ODB.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>

//---------- define constant ------------//

#define CURSOR_DBF  	DIR_RES"CURSOR.RES"

//---------- Begin of function MouseCursor::MouseCursor --------//

MouseCursor::MouseCursor()
{
	init_flag = 0;
	hide_all_flag = 0;
	hide_area_flag = 0;
	cur_icon = 0;
	hide_x1 = hide_y1 = hide_x2 = hide_y2 = 0;
	cur_x1 = cur_y1 = cur_x2 = cur_y2 = 0;
	hot_spot_x = hot_spot_y = 0;
	icon_width = 0;
	icon_height = 0;
	cursor_shown = 0;
	processing_flag = 0;
	wait_mode_count = 0;
	icon_ptr = icon_data_ptr = NULL;
	save_scr = save_back_scr = merge_buf = NULL;
	frame_flag = 0;
	frame_shown = 0;
	frame_x1 = frame_y1 = frame_x2 = frame_y2 = 0;
	frame_origin_x = frame_origin_y = 0;
	frame_border_x1 = frame_border_y1 = frame_border_x2 = frame_border_y2 = 0;
	memset(frame_top_save_scr, 0, VGA_WIDTH + 4);
	memset(frame_bottom_save_scr, 0, VGA_WIDTH + 4);
	memset(frame_left_save_scr, 0, VGA_HEIGHT + 4);
	memset(frame_right_save_scr, 0, VGA_HEIGHT + 4);
	cursor_count = 0;
	cursor_info_array = NULL;
}
//----------- End of function MouseCursor::MouseCursor ------//


//---------- Begin of function MouseCursor::~MouseCursor --------//

MouseCursor::~MouseCursor()
{
	deinit();
}
//----------- End of function MouseCursor::~MouseCursor ------//


//---------- Begin of function MouseCursor::init --------//
//
void MouseCursor::init()
{
	//----- open plant material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_CURSOR.RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_cursor_info();

	init_flag=1;
}
//----------- End of function MouseCursor::init ------//


//---------- Begin of function MouseCursor::deinit --------//

void MouseCursor::deinit()
{
	if( init_flag )
	{
		mem_del(cursor_info_array);

		if( save_scr )
		{
			mem_del( save_scr );
			save_scr = NULL;
		}

		if( save_back_scr )
		{
			mem_del( save_back_scr );
			save_back_scr = NULL;
		}

		if( merge_buf )		// buffer for merging save screen from the front and back buffers
		{
			mem_del( merge_buf );
			merge_buf = NULL;
		}

		init_flag = 0;
		icon_ptr  = NULL;
		// ###### begin Gilbert 18/8 #####//
		cur_icon = 0;
		// ###### end Gilbert 18/8 #####//
	}
}
//----------- End of function MouseCursor::deinit ------//


//------- Begin of function MouseCursor::load_cursor_info -------//
//
void MouseCursor::load_cursor_info()
{
	CursorRec  		*cursorRec;
	CursorInfo     *cursorInfo;
	int      		i;
	long				bitmapOffset;
	Database 		dbCursor(CURSOR_DBF, 1);		// 1-read all into the buffer

	cursor_count = (short) dbCursor.rec_count();
	cursor_info_array = (CursorInfo*) mem_add( sizeof(CursorInfo)*cursor_count );

	//-------- read in PLANTBMP.DBF -------//

	memset( cursor_info_array, 0, sizeof(CursorInfo) * cursor_count );

	for( i=0 ; i<cursor_count ; i++ )
	{
		cursorRec  = (CursorRec*) dbCursor.read(i+1);
		cursorInfo = cursor_info_array+i;

		memcpy( &bitmapOffset, cursorRec->bitmap_ptr, sizeof(long) );

		cursorInfo->bitmap_ptr = res_bitmap.read_imported(bitmapOffset);

		cursorInfo->hot_spot_x = m.atoi( cursorRec->hot_spot_x, cursorRec->HOT_SPOT_LEN );
		cursorInfo->hot_spot_y = m.atoi( cursorRec->hot_spot_y, cursorRec->HOT_SPOT_LEN );
	}
}
//--------- End of function MouseCursor::load_cursor_info ---------//


//--------- Begin of function MouseCursor::set_icon ------------//
//
// Set the bitmap of the mouse cursor
//
// <int> cursorId = id. of the cursor
//
void MouseCursor::set_icon(int cursorId)
{
	if( !init_flag )
		return;

	//-------- hide the cursor first ----------//

	int hideAllFlag = hide_all_flag;

	if( !hideAllFlag )     // if the mouse has been hiden before, don't hide and show it
		mouse.hide();

	//------------ set cursor icon ------------//

	CursorInfo* cursorInfo = cursor_info_array+cursorId-1;

	icon_ptr   = cursorInfo->bitmap_ptr;
	hot_spot_x = cursorInfo->hot_spot_x;
	hot_spot_y = cursorInfo->hot_spot_y;

	err_when( !icon_ptr );

	icon_width  = *((short*)icon_ptr);
	icon_height = *((short*)icon_ptr+1);

	// ###### begin Gilbert 18/8 #####//
	cur_icon = cursorId;
	// ###### end Gilbert 18/8 #####//

	//------- allocate buffer for screen saving ------//

	save_scr  		= mem_resize( save_scr 		, icon_width*icon_height+sizeof(short)*2 );		// sizeof(short)*2 is the header for storing width and height info
	save_back_scr  = mem_resize( save_back_scr, icon_width*icon_height+sizeof(short)*2 );		// sizeof(short)*2 is the header for storing width and height info
	merge_buf 		= mem_resize( merge_buf	 	, icon_width*icon_height+sizeof(short)*2 );		// sizeof(short)*2 is the header for storing width and height info

	//------------ redisplay cursor -----------//

	if( !hideAllFlag )
		mouse.show();
}
//----------- End of function MouseCursor::set_icon -------------//


//----------- Begin of function MouseCursor::set_frame --------//
//
// <char> frameFlag  - frame flag
// [char] buildTrack - treat the frame as a line for building track,
//							  also align the frame's position to locations.
//							  (default:0)
//
void MouseCursor::set_frame(char frameFlag, char buildTrack)
{
	if( frame_flag == frameFlag )
		return;

	frame_flag  = frameFlag;
	frame_shown = 0;
}
//----------- End of function MouseCursor::set_frame -------//


//----------- Begin of function MouseCursor::set_frame_border --------//

void MouseCursor::set_frame_border(int borderX1, int borderY1, int borderX2, int borderY2)
{
	frame_border_x1 = borderX1;
	frame_border_y1 = borderY1;
	frame_border_x2 = borderX2;
	frame_border_y2 = borderY2;
}
//----------- End of function MouseCursor::set_frame_border -------//


//----------- Begin of function MouseCursor::process --------//

void MouseCursor::process(int curX, int curY)
{
	if( processing_flag || !icon_ptr)   // it is being nested call by interrupt
		return;                          // when another instance of process is
													// being run.
	processing_flag = 1;    				// Prevent nested call

	//---------- store screen area ------------//

	if( cursor_shown )
	{
		// restore screen previously saved
		int save_x1, save_x2, save_y1, save_y2;

		save_x1 = MAX(cur_x1, 0);
		save_y1 = MAX(cur_y1, 0);
		save_x2 = MIN(cur_x2, VGA_WIDTH-1);
		save_y2 = MIN(cur_y2, VGA_HEIGHT-1);

		if ( save_x1 < save_x2 && save_y1 < save_y2 )
		{
			vga_front.put_bitmap_area_trans( save_x1,
							 save_y1,
							 save_scr,
							 save_x1-cur_x1,
							 save_y1-cur_y1,
							 save_x2-cur_x1,
							 save_y2-cur_y1 );
		}
	}

	//---- only the zoom map can be framed, limit the frame inside that area ----//

	if( frame_flag )
	{
		curX = MAX(curX, ZOOM_X1);
		curY = MAX(curY, ZOOM_Y1);
		curX = MIN(curX, ZOOM_X2);
		curY = MIN(curY, ZOOM_Y2);

		process_frame(curX, curY);
	}

	//------- update cursor postions ----------//

	cur_x1 = curX - hot_spot_x;		// handle the offset of the hot site
	cur_y1 = curY - hot_spot_y;
	cur_x2 = cur_x1 + icon_width  - 1;
	cur_y2 = cur_y1 + icon_height - 1;

	//------- save screen and put cursor -------//

	if( hide_all_flag || ( hide_area_flag &&
		 is_touch( hide_x1, hide_y1, hide_x2, hide_y2 ) ) )
	{
		cursor_shown = 0;
	}
	else
	{
		/* Save the screen underneath the cursor
		 * where it will be drawn.
		 */
		int save_x1, save_x2, save_y1, save_y2;

		save_x1 = MAX(cur_x1, 0);
		save_y1 = MAX(cur_y1, 0);
		save_x2 = MIN(cur_x2, VGA_WIDTH-1);
		save_y2 = MIN(cur_y2, VGA_HEIGHT-1);

		if ( save_x1 < save_x2 && save_y1 < save_y2 ) {
			vga_front.read_bitmap( save_x1, save_y1,
					       save_x2, save_y2, save_scr );
			vga_front.put_bitmap_area_trans( save_x1,
							 save_y1,
							 icon_ptr,
							 save_x1-cur_x1,
							 save_y1-cur_y1,
							 save_x2-cur_x1,
							 save_y2-cur_y1 );

			cursor_shown = 1;
		}
	}

	//------------------------------------------//

	processing_flag = 0;     // cancel prevention of nested call
}
//----------- End of function MouseCursor::process -------//


//-------- Begin of function MouseCursor::process_frame --------//

void MouseCursor::process_frame(int curX, int curY)
{
	//---- restore the screen area overwritten by the last frame ---//

	if( frame_shown )
	{
		vga_front.fast_put_bitmap( frame_x1, frame_y1, frame_top_save_scr    );
		vga_front.fast_put_bitmap( frame_x1, frame_y2, frame_bottom_save_scr );
		vga_front.fast_put_bitmap( frame_x1, frame_y1, frame_left_save_scr   );
		vga_front.fast_put_bitmap( frame_x2, frame_y1, frame_right_save_scr  );
	}

	//---------- update frame position ----------//

	if( !frame_shown )			// a new frame
	{
		frame_origin_x = curX;
		frame_origin_y = curY;

		frame_x1 = curX;
		frame_y1 = curY;
		frame_x2 = curX;
		frame_y2 = curY;
	}
	else  	// update the postion of the existing frame
	{
		//---------- update frame position ----------//

		if( curX > frame_origin_x )
		{
			if( curY > frame_origin_y )		// stretching towards the lower right end
			{
				frame_x1 = frame_origin_x;
				frame_y1 = frame_origin_y;
				frame_x2 = curX;
				frame_y2 = curY;
			}
			else		// stretching towards the upper right end
			{
				frame_x1 = frame_origin_x;
				frame_y1 = curY;
				frame_x2 = curX;
				frame_y2 = frame_origin_y;
			}
		}
		else
		{
			if( curY > frame_origin_y )		// stretching towards the lower left end
			{
				frame_x1 = curX;
				frame_y1 = frame_origin_y;
				frame_x2 =	frame_origin_x;
				frame_y2 = curY;
			}
			else		// stretching towards the upper left end
			{
				frame_x1 = curX;
				frame_y1 = curY;
				frame_x2 = frame_origin_x;
				frame_y2 = frame_origin_y;
			}
		}
	}

	//------- save the screen area and display the frame ------//

	disp_frame(&vga_front);
}
//----------- End of function MouseCursor::process_frame -------//


//----------- Begin of function MouseCursor::disp_frame --------//

void MouseCursor::disp_frame(VgaBuf* vgaBufPtr)
{
	//------- save the screen area that will be overwriteen -------//

	vgaBufPtr->read_bitmap( frame_x1, frame_y1, frame_x2, frame_y1, frame_top_save_scr );
	vgaBufPtr->read_bitmap( frame_x1, frame_y2, frame_x2, frame_y2, frame_bottom_save_scr );
	vgaBufPtr->read_bitmap( frame_x1, frame_y1, frame_x1, frame_y2, frame_left_save_scr );
	vgaBufPtr->read_bitmap( frame_x2, frame_y1, frame_x2, frame_y2, frame_right_save_scr );

	//---------- draw the rectagular frame now -----------//

	vgaBufPtr->rect( frame_x1, frame_y1, frame_x2, frame_y2, 1, OWN_SELECT_FRAME_COLOR );

	//---------- set frame flag ----------//

	frame_shown = 1;
}
//----------- End of function MouseCursor::disp_frame -------//


//----------- Begin of function MouseCursor::disp_back_buf --------//
//
// Display the mouse cursor on the back buffer.
//
void MouseCursor::disp_back_buf(int bltX1, int bltY1, int bltX2, int bltY2)
{
	if( !icon_ptr )
		return;

	//-------- display frame on the back buffer ----//

	if( frame_flag )
		disp_frame(&vga_back);

	//----- display mouse cursor on the back buffer ----//

	if( is_touch(bltX1, bltY1, bltX2, bltY2) )
	{
		//--- save the front buffer area which will be overwritten ---//

		int x1 = MAX(cur_x1,bltX1);
		int y1 = MAX(cur_y1,bltY1);
		int x2 = MIN(cur_x2,bltX2);
		int y2 = MIN(cur_y2,bltY2);

		vga_back.read_bitmap( x1, y1, x2, y2, save_back_scr );

		//--- merge the save area of the back buf with the front buf's save area ---//

		// save_scr width  : MIN(cur_x2,VGA_WIDTH-1) -MAX(cur_x1,0)+1;
		// save_scr height : MIN(cur_y2,VGA_HEIGHT-1)-MAX(cur_y1,0)+1;

		IMGblt( save_scr+4, MIN(cur_x2,VGA_WIDTH-1) -MAX(cur_x1,0)+1, x1-MAX(cur_x1,0), y1-MAX(cur_y1,0), save_back_scr );		// +4 is the width & height info

		//--------- display the mouse cursor now -----------//

		if( cur_x1 < bltX1 || cur_x2 > bltX2 || cur_y1 < bltY1 || cur_y2 > bltY2 )
		{
			vga_back.put_bitmap_area_trans( cur_x1, cur_y1, icon_ptr,
				MAX(bltX1,cur_x1)-cur_x1, MAX(bltY1,cur_y1)-cur_y1,
				MIN(bltX2,cur_x2)-cur_x1, MIN(bltY2,cur_y2)-cur_y1 );
		}

		//---- the whole sprite is inside the view area ------//

		else
		{
			vga_back.put_bitmap_trans(cur_x1, cur_y1, icon_ptr);
		}

		cursor_shown = 1;
	}
}
//----------- End of function MouseCursor::disp_back_buf -------//


//--------- Begin of function MouseCursor::is_touch ------------//
//
// Check if the given area touch the area defined by cur_??.
//
// Return : 1 or 0
//
int MouseCursor::is_touch(int x1, int y1, int x2, int y2)
{
	return (( cur_y1 <=  y1 && cur_y2 >=  y1 ) ||
			  (  y1 <= cur_y1 &&  y2 >= cur_y1 )) &&
			 (( cur_x1 <=  x1 && cur_x2 >=  x1 ) ||
			  (  x1 <= cur_x1 &&  x2 >= cur_x1 ));
}
//--------- End of function MouseCursor::is_touch -----------//


// ####### begin Gilbert 18/8 ########//
//--------- Begin of function MouseCursor::restore_icon ------------//
void MouseCursor::restore_icon(int newCursorId)
{
	if( newCursorId == 0)
	{
		err_here();

		// should restore to invisible cursor ?
		if( !hide_all_flag )
			mouse.hide();
		cur_icon = 0;
		icon_ptr = NULL;
	}
	else if( cur_icon == 0 || newCursorId != cur_icon )
	{
		set_icon(newCursorId);
	}
}
//--------- End of function MouseCursor::restore_icon ------------//
// ####### end Gilbert 18/8 ########//
