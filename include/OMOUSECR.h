//Filename    : OMOUSECR.H
//Description : Header file of Object Cursor resource

#ifndef __OMOUSECR_H
#define __OMOUSECR_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OVGA_H
#include <OVGA.h>
#endif

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//------------ Define cursor id. ----------//

enum { CURSOR_NORMAL =1,      // nothing selected, 
       CURSOR_NORMAL_C,			// nothing selected, pointing a friend object
		 CURSOR_NORMAL_O,			// nothing selected, pointing an enemy object

		 CURSOR_UNIT,				// selected a friend unit  (walk)
		 CURSOR_UNIT_C,			// selected a friend unit, pointing a friend object
		 CURSOR_UNIT_O,			// selected a friend unit, pointing an enemy object (attack!)

		 CURSOR_C_TOWN,			// selected a friend town
		 CURSOR_C_TOWN_C,			// selected a friend town, pointing a friend object
		 CURSOR_C_TOWN_O,			// selected a friend town, pointing an enemy object
		 CURSOR_O_TOWN,			// selected an enemy town
		 CURSOR_O_TOWN_C,			// selected an enemy town, pointing a friend object
		 CURSOR_O_TOWN_O,			// selected an enemy town, pointing an enemy object

		 CURSOR_C_FIRM,			// selected a friend firm
		 CURSOR_C_FIRM_C,			// selected a friend firm, pointing a friend object
		 CURSOR_C_FIRM_O,			// selected a friend firm, pointing an enemy object
		 CURSOR_O_FIRM,			// selected an enemy firm
		 CURSOR_O_FIRM_C,			// selected an enemy firm, pointing a friend object
		 CURSOR_O_FIRM_O,			// selected an enemy firm, pointing an enemy object

		 CURSOR_WAITING,
		 CURSOR_BUILD,
		 CURSOR_DESTRUCT,
		 CURSOR_ASSIGN,
		 CURSOR_SET_STOP,
		 CURSOR_CANT_SET_STOP,
		 CURSOR_SHIP_STOP,
		 CURSOR_CANT_SHIP_STOP,
		 CURSOR_BURN,
		 CURSOR_SETTLE,
		 CURSOR_SETTLE_1,			// settle cursor for different color_scheme_id
		 CURSOR_SETTLE_2,
		 CURSOR_SETTLE_3,
		 CURSOR_SETTLE_4,
		 CURSOR_SETTLE_5,
		 CURSOR_SETTLE_6,
		 CURSOR_SETTLE_7,
		 CURSOR_ON_LINK,			// when pointing town/firm link
		 CURSOR_TRIGGER_EXPLODE,	// special of CURSOR_UNIT_C, when pointing at an explosive cart
		 CURSOR_CAPTURE_FIRM,
		 CURSOR_ENCYC,
     };

enum { STRETCH_NW, STRETCH_NE, STRETCH_SW, STRECH_SE };	// mouse_stretch_dir


//------------ Define struct CursorRec ---------------//

struct CursorRec
{
	enum { HOT_SPOT_LEN=3, FILE_NAME_LEN=8, BITMAP_PTR_LEN=4 };

	char file_name[FILE_NAME_LEN];

	char hot_spot_x[HOT_SPOT_LEN];
	char hot_spot_y[HOT_SPOT_LEN];

	char bitmap_ptr[BITMAP_PTR_LEN];
};

//------------- Define struct CursorInfo --------------//

struct CursorInfo
{
	short hot_spot_x;
	short hot_spot_y;

	char* bitmap_ptr;
};

//--------- Define class MouseCursor --------//

class MouseCursor
{
public:
	char  init_flag;
	char  hide_all_flag;
	char  hide_area_flag;

	// ###### begin Gilbert 18/8 #####//
	short	cur_icon;
	// ###### end Gilbert 18/8 #####//

	int   hide_x1, hide_y1, hide_x2, hide_y2;
	int   cur_x1, cur_y1, cur_x2, cur_y2;

	int	hot_spot_x, hot_spot_y;

	int   icon_width;
	int   icon_height;

	char  cursor_shown;     // whether the cursor has been shown on the screen
	char  processing_flag;  // whether process() is being run or not, prevent nested call by interrupt
	char  wait_mode_count;  // allow nested wait cursor

	char 	*icon_ptr, *icon_data_ptr;
	char 	*save_scr, *save_back_scr, *merge_buf;

	//------- nation selection frame --------//

	char  frame_flag;			// whether nation selection frame is on
	char  frame_shown;

	int	frame_x1, frame_y1, frame_x2, frame_y2;
	int	frame_origin_x, frame_origin_y;
	int	frame_border_x1, frame_border_y1, frame_border_x2, frame_border_y2;

	char  frame_top_save_scr[VGA_WIDTH+4];		// for saving the nation selection frame
	char  frame_bottom_save_scr[VGA_WIDTH+4];	// +4 is for the width & height info
	char  frame_left_save_scr[VGA_HEIGHT+4];
	char  frame_right_save_scr[VGA_HEIGHT+4];

	//------- cursor bitmap resource --------//

private:
	short    	 cursor_count;
	CursorInfo*  cursor_info_array;

	ResourceDb	 res_bitmap;

public:
	MouseCursor();
	~MouseCursor();

	void 	init();
	void 	deinit();

	void 	set_icon(int);
	void  set_frame(char,char=0);
	void  set_frame_border(int,int,int,int);

	void 	process(int,int);
	void	disp_back_buf(int,int,int,int);

	// ####### begin Gilbert 18/8 ########//
	int	get_icon()				{ return cur_icon; }
	void	restore_icon(int);
	// ####### end Gilbert 18/8 ########//

private:
	void	process_frame(int,int);
	void 	disp_frame(VgaBuf*);
	int	is_touch(int,int,int,int);

	void  load_cursor_info();
};

extern MouseCursor mouse_cursor;

//----------------------------------------------//

#endif

