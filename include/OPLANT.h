//Filename    : OPLANT.H
//Description : Header file of object PlantRes
//Owner       : Gilbert

#ifndef __OPLANT_H
#define __OPLANT_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//------------- Define climate zone ------------//

enum { ZONE_TROPICAL =1,
		 ZONE_TEMPERATE=2
	  };

//------------ Define struct PlantRec ---------------//

struct PlantRec
{
	enum { CODE_LEN=8, ZONE_LEN=1, TERA_TYPE_LEN=2, FIRST_BITMAP_LEN=3, BITMAP_COUNT_LEN=3 };

	char code[CODE_LEN];
	char climate_zone[ZONE_LEN];

	char tera_type1[TERA_TYPE_LEN];
	char tera_type2[TERA_TYPE_LEN];
	char tera_type3[TERA_TYPE_LEN];

	char first_bitmap[FIRST_BITMAP_LEN];
	char bitmap_count[BITMAP_COUNT_LEN];
};

//------------ Define struct PlantBitmapRec ---------------//

struct PlantBitmapRec
{
	enum { PLANT_LEN=8, SIZE_LEN=2, OFFSET_LEN=3, FILE_NAME_LEN=8, BITMAP_PTR_LEN=4 };

	char plant[PLANT_LEN];
	char size[SIZE_LEN];

	char offset_x[OFFSET_LEN];
	char offset_y[OFFSET_LEN];

	char town_age;

	char file_name[FILE_NAME_LEN];
	char bitmap_ptr[BITMAP_PTR_LEN];
};

//------------- Define struct PlantInfo --------------//

struct PlantInfo
{
	char	climate_zone;
	char	tera_type[3];

	short first_bitmap;
	short bitmap_count;
};

//------------- Define struct PlantBitmap -------------//

struct PlantBitmap
{
public:
	char  size;
	short offset_x, offset_y;
	char	town_age;

	short bitmap_width, bitmap_height;
	char* bitmap_ptr;

public:
	void	draw(int xLoc, int yLoc);
	void 	draw_at(int xLoc, int yLoc);
};

//----------- Define class PlantRes ---------------//

class PlantRes
{
public:
	short    	 plant_count;
	short			 plant_bitmap_count;

	PlantInfo* 	 plant_info_array;
	PlantBitmap* plant_bitmap_array;
	short*		 scan_id_array;				// a buffer for scaning

	char			 plant_map_color;

	char	   	 init_flag;
	ResourceDb	 res_bitmap;

public:
	PlantRes();

	void 		 	 init();
	void 		 	 deinit();

	int 			 scan(int climateZone, int teraType, int townAge);

	short			 plant_recno(short bitmapId);

	#ifdef DEBUG
		PlantBitmap* get_bitmap(int bitmapId);
		PlantInfo*   operator[](int plantId);
	#else
		PlantBitmap* get_bitmap(int bitmapId) 	{ return plant_bitmap_array+bitmapId-1; }
		PlantInfo*   operator[](int plantId)	{ return plant_info_array+plantId-1; }
	#endif

private:
	void 		    load_plant_info();
	void 		    load_plant_bitmap();
};

extern PlantRes plant_res;

//----------------------------------------------------//

#endif
