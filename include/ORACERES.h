//Filename    : ORACERES.H
//Description : Header file of object RaceRes

#ifndef __ORACERES_H
#define __ORACERES_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

//------------ Define race id. -------------//

enum { RACE_NORMAN=1,
		 RACE_MAYA,
		 RACE_GREEK,
		 RACE_VIKING,
		 RACE_PERSIAN,
		 RACE_CHINESE,
		 RACE_JAPANESE,
#ifdef AMPLUS
		 RACE_EGYPTIAN,
		 RACE_INDIAN,
		 RACE_ZULU,
#endif
	  };

//--------- Define constant -----------//

enum { RACE_ICON_WIDTH=24,
		 RACE_ICON_HEIGHT=20,
	  };

//------------ Define struct RaceRec ---------------//

struct RaceRec
{
	enum { CODE_LEN=8, NAME_LEN=12, ADJECTIVE_LEN=12, FILE_NAME_LEN=8, BITMAP_PTR_LEN=4 };

	char code[CODE_LEN];
	char name[NAME_LEN];
	char adjective[ADJECTIVE_LEN];

	char icon_file_name[FILE_NAME_LEN];
	char icon_bitmap_ptr[BITMAP_PTR_LEN];
};

//------------- Define struct RaceInfo --------------//

struct RaceInfo
{
public:
	//------ constant vars --------//

	enum  { CODE_LEN=8, NAME_LEN=12, ADJECTIVE_LEN=12 };

	char  race_id;

	char	code[CODE_LEN+1];
	char  name[NAME_LEN+1];
	char  adjective[ADJECTIVE_LEN+1];

	char* icon_bitmap_ptr;

	//----------------------//

	short	first_first_name_id;		// first <first name> of this race in first_name_array[]
	short first_last_name_id;		// first <last name> of this race in last_name_array[]

	short first_name_count;
	short last_name_count;

	//----------------------//

	short first_town_name_recno;
	short town_name_count;

	//----------------------//

	short	basic_unit_id;

	//--------- game vars ----------//

	short town_name_used_count;

public:
	char* get_name(WORD nameId, int nameType=0);
	char* get_single_name(WORD nameId);

	WORD	get_new_name_id();
	void	free_name_id(WORD nameId);
	void	use_name_id(WORD nameId);
};

//-------- Define struct NameRec ----------//

struct RaceNameRec
{
	enum { NAME_LEN=20 };
	char name[NAME_LEN+1];
};

//-------- Define struct NameInfo ----------//

struct RaceName
{
	enum { NAME_LEN=20 };
	char	name[NAME_LEN+1];
};

//----------- Define class RaceRes ---------------//

class RaceRes
{
public:
	char        init_flag;

	short       race_count;
	RaceInfo*   race_info_array;
	short			name_count;
	RaceName*	name_array;
	char*			name_used_array;

	ResourceDb  res_bitmap;

public:
	RaceRes();

	void        init();
	void        deinit();

	int         write_file(File*);
	int         read_file(File*);

	int			is_same_race(int,int);

	RaceInfo*   operator[](int raceId);      // pass raceId  as recno

private:
	void        load_race_info();
	void  		load_name();
};

extern RaceRes race_res;

//----------------------------------------------------//

#endif
