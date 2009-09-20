//Filename    : OTOWNREC.H
//Description : Town database definition

#ifndef __OTOWNREC_H
#define __OTOWNREC_H

//------------ Define struct TownLayoutRec ---------------//

struct TownLayoutRec
{
	enum { CODE_LEN=8, GROUND_NAME_LEN=8, FIRST_SLOT_LEN=5, SLOT_COUNT_LEN=2 };

	char code[CODE_LEN];
	char ground_name[GROUND_NAME_LEN];		// name of the ground bitmap in image_spict

	char first_slot[FIRST_SLOT_LEN];
	char slot_count[SLOT_COUNT_LEN];
};

//------------ Define struct TownSlotRec ---------------//

struct TownSlotRec
{
	enum { CODE_LEN=8, POS_LEN=3, TYPE_LEN=8, BUILD_CODE_LEN=2, TYPE_ID_LEN=3 };

	char layout_code[CODE_LEN];

	char base_x[POS_LEN];
	char base_y[POS_LEN];

	char type[TYPE_LEN];
	char build_code[BUILD_CODE_LEN];

	char type_id[TYPE_ID_LEN];
};

//---------- Define struct TownBuildTypeRec -----------//

struct TownBuildTypeRec
{
	enum { TYPE_CODE_LEN=8, FIRST_BUILD_LEN=5, BUILD_COUNT_LEN=5 };

	char type_code[TYPE_CODE_LEN];

	char first_build[FIRST_BUILD_LEN];
	char build_count[BUILD_COUNT_LEN];
};

//------------ Define struct TownBuildRec ---------------//

struct TownBuildRec
{
	enum { TYPE_LEN=8, BUILD_CODE_LEN=2, RACE_LEN=8,
			 TYPE_ID_LEN=3, RACE_ID_LEN=3,
			 FILE_NAME_LEN=8, BITMAP_PTR_LEN=4 };

	char type[TYPE_LEN];
	char build_code[BUILD_CODE_LEN];
	char race[RACE_LEN];

	char type_id[TYPE_ID_LEN];
	char race_id[RACE_ID_LEN];

	char file_name[FILE_NAME_LEN];
	char bitmap_ptr[BITMAP_PTR_LEN];
};

//------------ Define struct TownNameRec ---------------//

struct TownNameRec
{
	enum { NAME_LEN=15 };

   char name[NAME_LEN];
};

//-------------------------------------------------------//

#endif

