//Filename    : OFIRMRES.H
//Description : Header file of object FirmRes

#ifndef __OFIRMRES_H
#define __OFIRMRES_H

#ifndef __GAMEDEF_H
#include <GAMEDEF.h>
#endif

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//------- Define constant ----------//

#define MAX_FIRM_FRAME			11		// Maximum frames for firm animation

#define MAX_FIRM_LOC_WIDTH  	4	 	// Maximum no. of locations a firm can occupy
#define MAX_FIRM_LOC_HEIGHT 	4

//------------- Firm Mode ------------//

enum { FIRM_UNDER_CONSTRUCTION='U',		// definitions of FirmBuild::mode
		 FIRM_IDLE='I',
		 FIRM_ACTIVE='A',
	  };

//------------ Define struct FirmRec ---------------//

struct FirmRec
{
	enum { CODE_LEN=8, NAME_LEN=20, SHORT_NAME_LEN=12, TITLE_LEN=10, FIRST_BUILD_LEN=3, BUILD_COUNT_LEN=3,
			 HIT_POINTS_LEN=5, COST_LEN=5 };

	char code[CODE_LEN];
	char name[NAME_LEN];
	char short_name[SHORT_NAME_LEN];

	char overseer_title[TITLE_LEN];
	char worker_title[TITLE_LEN];

	char tera_type;
	char all_know;				// whether all nations know how to build this firm in the beginning of the game
	char live_in_town;		// whether the workers of the firm lives in towns or not.

	char hit_points[HIT_POINTS_LEN];

	char is_linkable_to_town;

	char setup_cost[COST_LEN];
	char year_cost[COST_LEN];

	char first_build[FIRST_BUILD_LEN];
	char build_count[BUILD_COUNT_LEN];
};

//------------ Define struct FirmBuildRec ---------------//

struct FirmBuildRec
{
	enum { FIRM_CODE_LEN=8, RACE_CODE_LEN=8, BITMAP_RECNO_LEN=5, FIRST_FRAME_LEN=5, FRAME_COUNT_LEN=2, RACE_ID_LEN=3 };

	char firm_code[FIRM_CODE_LEN];
	char race_code[RACE_CODE_LEN];

	char animate_full_size;

	char under_construction_bitmap_recno[BITMAP_RECNO_LEN];
	// ##### begin Gilbert 18/10 ########//
	char under_construction_bitmap_count[FRAME_COUNT_LEN];
	// ##### end Gilbert 18/10 ########//
	char idle_bitmap_recno[BITMAP_RECNO_LEN];
	char ground_bitmap_recno[BITMAP_RECNO_LEN];

	char first_frame[FIRST_FRAME_LEN];
	char frame_count[FRAME_COUNT_LEN];

	char race_id[RACE_ID_LEN];
};

//------------ Define struct FirmFrameRec ---------------//

struct FirmFrameRec
{
	enum { FIRM_CODE_LEN=8, RACE_CODE_LEN=8, FRAME_ID_LEN=2, DELAY_LEN=2, FIRST_BITMAP_LEN=5, BITMAP_COUNT_LEN=2 };

	char firm_code[FIRM_CODE_LEN];
	char race_code[RACE_CODE_LEN];

	char frame_id[FRAME_ID_LEN];

	char delay[DELAY_LEN];		// unit: 1/10 second

	char first_bitmap[FIRST_BITMAP_LEN];
	char bitmap_count[BITMAP_COUNT_LEN];
};

//------------ Define struct FirmBitmapRec ---------------//

struct FirmBitmapRec
{
	enum { FIRM_CODE_LEN=8, RACE_CODE_LEN=8, FRAME_ID_LEN=2, LOC_LEN=3, OFFSET_LEN=3, DELAY_LEN=2, FILE_NAME_LEN=8, BITMAP_PTR_LEN=4 };

	char firm_code[FIRM_CODE_LEN];
	char race_code[RACE_CODE_LEN];
	char mode;

	char frame_id[FRAME_ID_LEN];

	char loc_width[LOC_LEN];
	char loc_height[LOC_LEN];
	char layer;

	char offset_x[OFFSET_LEN];
	char offset_y[OFFSET_LEN];

	char delay[DELAY_LEN];	  // unit: 1/10 second

	char file_name[FILE_NAME_LEN];
	char bitmap_ptr[BITMAP_PTR_LEN];
};

//------------- Define struct FirmInfo --------------//

struct FirmInfo
{
	enum { NAME_LEN=20, SHORT_NAME_LEN=12, TITLE_LEN=10 };

	char	firm_id;
	char 	name[NAME_LEN+1];
	char  short_name[SHORT_NAME_LEN+1];

	char  overseer_title[TITLE_LEN+1];
	char  worker_title[TITLE_LEN+1];

	char  tera_type;
	char  buildable;					// whether this building can be built by the player or it exists in the game since the beginning of the game. If setup_cost==0, this firm is not buildable
	char  live_in_town;		// whether the workers of the firm lives in towns or not.
	short	max_hit_points;

	char	need_overseer;
	char	need_worker;
	char	need_unit()		{ return need_overseer || need_worker; }

	short setup_cost;
	short year_cost;

	short	first_build_id;
	short build_count;

	short loc_width;
	short loc_height;

	char	firm_skill_id;			// the id. of the skill that fits this firm
	char	firm_race_id;			// only can be built and operated by this race

	int	can_build(int unitRecno);

	char  is_linkable_to_town;
	int 	is_linkable_to_firm(int linkFirmId);

	int 	default_link_status(int linkFirmId);

	//---------- game vars -----------//

	short total_firm_count;								// total no. of this firm type on the map
	short nation_firm_count_array[MAX_NATION];

	char  nation_tech_level_array[MAX_NATION];
	int   get_nation_tech_level(int nationRecno) 						{ return nation_tech_level_array[nationRecno-1]; }
	void  set_nation_tech_level(int nationRecno, char techLevel) 	{ nation_tech_level_array[nationRecno-1] = techLevel; }

public:
	int	get_build_id(char* buildCode);

	void	inc_nation_firm_count(int nationRecno);
	void	dec_nation_firm_count(int nationRecno);
};

//------------- Define struct FirmBuild --------------//

struct FirmBuild
{
	enum { BUILD_CODE_LEN=8 };

	char  build_code[BUILD_CODE_LEN+1];		// building code, either a race code or a custom code for each firm's own use, it is actually read from FirmBuildRec::race_code[] 
	char  race_id;
	char  animate_full_size;

	//----- info of the first frame -----//

	char	loc_width;			// no. of locations it takes horizontally and vertically
	char	loc_height;

	char  min_offset_x, min_offset_y;
	short max_bitmap_width, max_bitmap_height;

	//----------- frame info ------------//

	short frame_count;

	short first_bitmap_array[MAX_FIRM_FRAME];
	short bitmap_count_array[MAX_FIRM_FRAME];
	short frame_delay_array[MAX_FIRM_FRAME];      // unit: 1/10 second

	short	under_construction_bitmap_recno;		// bitmap recno of the firm that is under construction
	// ##### begin Gilbert 18/10 ########//
	short	under_construction_bitmap_count;
	// ##### end Gilbert 18/10 ########//
	short	idle_bitmap_recno;						// bitmap recno of the firm that is idle
	short ground_bitmap_recno;

	short first_bitmap(int frameId) 	{ return first_bitmap_array[frameId-1]; }
	short bitmap_count(int frameId) 	{ return bitmap_count_array[frameId-1]; }
	short frame_delay(int frameId) 	{ return frame_delay_array[frameId-1];  }
};

//------------- Define struct FirmBitmap --------------//

struct FirmBitmap
{
	char* bitmap_ptr;
	short width, height;

	char	loc_width;			// no. of locations it takes horizontally and vertically
	char	loc_height;
	char  offset_x, offset_y;
	char	display_layer;

	void	draw_at(int absX, int absY, char *colorTable, int displayLayer);
};

//----------- Define class FirmRes ---------------//

class FirmRes
{
public:
	short    	firm_count;
	short			firm_build_count;
	short			firm_bitmap_count;

	FirmInfo* 	firm_info_array;
	FirmBuild*  firm_build_array;
	FirmBitmap* firm_bitmap_array;

	char	   	init_flag;
	ResourceDb	res_bitmap;

public:
	FirmRes();

	void 		 	init();
	void 		 	deinit();

	int 			write_file(File* filePtr);
	int			read_file(File* filePtr);

	FirmBitmap* get_bitmap(int bitmapId) 	{ return firm_bitmap_array+bitmapId-1; }
	FirmBuild*	get_build(int buildId)		{ return firm_build_array+buildId-1; }
	FirmInfo*   operator[](int firmId);

private:
	void 		   load_firm_info();
	void			load_firm_build();
	void 		   load_firm_bitmap();
	void			process_firm_info();
};

extern FirmRes firm_res;

//----------------------------------------------------//

#endif
