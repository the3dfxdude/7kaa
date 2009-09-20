//Filename    : OSFRMRES.H
//Description : Header file of Object SpriteFrame resource

#ifndef __OSFRMRES_H
#define __OSFRMRES_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OSPRTRES_H
#include <OSPRTRES.h>
#endif


//-------- Define struct SpriteFrameRec ----------//

struct SpriteFrameRec
{
	enum { NAME_LEN=8, ACTION_LEN=2, DIR_LEN=2, FRAME_ID_LEN=2, OFFSET_LEN=4,
			 WIDTH_LEN=3, HEIGHT_LEN=3, FILE_NAME_LEN=8, BITMAP_OFFSET_LEN=4 };

	char sprite_name[NAME_LEN];
	char action[ACTION_LEN];
	char dir[DIR_LEN];
	char frame_id[FRAME_ID_LEN];

	char offset_x[OFFSET_LEN];
	char offset_y[OFFSET_LEN];
	char width[WIDTH_LEN];
	char height[HEIGHT_LEN];

	char file_name[FILE_NAME_LEN];
	char bitmap_offset[BITMAP_OFFSET_LEN];
};

//------- Define struct SpriteFrame --------//

struct SpriteFrame
{
	char  offset_x, offset_y;
	short width, height;
	long  bitmap_offset;
};

//--------- Define class SpriteFrameRes --------//

class SpriteFrameRes
{
public:
	int  sprite_frame_count;
	char init_flag;

private:
	SpriteFrame* sprite_frame_array;

public:
	SpriteFrameRes() 	{ init_flag=0; }

	void init();
	void deinit();

	void load_info();

	#ifdef DEBUG
		SpriteFrame* operator[](int recNo);
	#else
		SpriteFrame* operator[](int recNo)   { return sprite_frame_array+recNo-1; }
	#endif
};

extern SpriteFrameRes sprite_frame_res;

//----------------------------------------------//

#endif

