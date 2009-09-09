//Filename    : OTOWNRES.H
//Description : Town town resource

#ifndef __OTOWNRES_H
#define __OTOWNRES_H

#ifndef __ALL_H
#include <ALL.H>
#endif

#ifndef __ORESDB_H
#include <ORESDB.H>
#endif

#ifndef __OTOWNREC_H
#include <OTOWNREC.H>
#endif

//--------- define game constants --------//

#define MAX_TOWN_LAYOUT_SLOT           25
#define POPULATION_PER_HOUSE            6     // no. of people per building

#define EFFECTIVE_TOWN_TOWN_DISTANCE   8     // The minimum distance between 2 towns
#define EFFECTIVE_FIRM_TOWN_DISTANCE   8     // maximum distance between the town and the base until it's no longer considered as command base of the town
#define EFFECTIVE_FIRM_FIRM_DISTANCE   8     // maximum distance between the town and the base until it's no longer considered as command base of the town
#define EFFECTIVE_POWER_DISTANCE       3

// population range of each building levels:
// level 1 building: 1-3, level 2 building: 4-6, level 3 building: 7-9

//------- values of TownSlot::build_type -------//

enum { TOWN_OBJECT_HOUSE=1,
		 TOWN_OBJECT_PLANT,
		 TOWN_OBJECT_FARM,
		 TOWN_OBJECT_FLAG,
	  };

//------ define struct TownLayout --------//

struct TownLayout
{
	char  build_count;      // no. of building in this layout

	short first_slot_recno;
	char  slot_count;

	char* ground_bitmap_ptr;
};

//------ define struct TownSlot --------//

struct TownSlot
{
	short base_x, base_y;
	char  build_type;       // id. of the building type
	char  build_code;       // building direction
};

//------ define struct TownBuildType --------//

struct TownBuildType
{
	short first_build_recno;
	int   build_count;
};

//------ define struct TownName --------//

struct TownName
{
	enum { NAME_LEN=15 };
	char name[NAME_LEN+1];
};

//------ define struct TownBuild --------//

struct TownBuild
{
public:
   char  build_type;          // building type. e.g. house, wind mill, church

	char  race_id;
	char  build_code;

	char* bitmap_ptr;
	short bitmap_width;
	short bitmap_height;

public:
	void draw(int townRecno, int absBaseX, int absBaseY);
};

//--------- Define class TownRes ----------//

class TownRes
{
public:
	char  init_flag;
	int   town_layout_count, town_slot_count;
	int   town_build_type_count, town_build_count, town_name_count;

	TownLayout*    town_layout_array;
	TownSlot*      town_slot_array;
	TownBuildType* town_build_type_array;
	TownBuild*     town_build_array;
	TownName*      town_name_array;
	char*				town_name_used_array;	// store the used_count separately from town_name_array to faciliate file saving

	ResourceDb     res_bitmap;

public:
   TownRes();

   void init();
   void deinit();

	int   scan_build(int slotId, int raceId);

	char*	get_name(int recNo);
	int	get_new_name_id(int raceId);
	void	free_name_id(int townNameId);

	int   write_file(File*);
	int   read_file(File*);

	#ifdef DEBUG
		TownLayout*    get_layout(int recNo);
		TownSlot*      get_slot(int recNo);
		TownBuildType* get_build_type(int recNo);
		TownBuild*     get_build(int recNo);
	#else
		TownLayout*    get_layout(int recNo)      { return town_layout_array    +recNo-1; };
		TownSlot*      get_slot(int recNo)        { return town_slot_array      +recNo-1; };
      TownBuildType* get_build_type(int recNo)  { return town_build_type_array+recNo-1; };
      TownBuild*     get_build(int recNo)       { return town_build_array     +recNo-1; };
   #endif

private:
   void load_town_layout();
   void load_town_slot();
   void load_town_build_type();
   void load_town_build();
   void load_town_name();
};

extern TownRes town_res;

//------------------------------------------//

#endif
