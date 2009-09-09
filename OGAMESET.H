//Filename    : OGAMESET.H
//Description : Header file of Object GameSet

#ifndef __OGAMESET_H
#define __OGAMESET_H

#ifndef __ORESX_H
#include <ORESX.H>
#endif

#ifndef __ODB_H
#include <ODB.H>
#endif

//-------- Define struct SetRec ---------//

struct SetRec
{
	enum { CODE_LEN=8, DES_LEN=60 };

	char code[CODE_LEN];
	char des[DES_LEN];
};


//-------- Define struct SetInfo ---------//

struct SetInfo
{
	enum { CODE_LEN=8, DES_LEN=60 };

	char  code[CODE_LEN+1];
	char  des[DES_LEN+1];
};


//-------- Define class GameSet ---------//

class GameSet
{
public:
	char			init_flag;
   short       cur_set_id;

	short       set_count;
	SetInfo*    set_info_array;

	ResourceIdx set_res;
	Database    set_db;

	char	      set_opened_flag;

public:
	GameSet()	{ init_flag=0; }
	~GameSet()	{ deinit(); }

	void        init();
	void        deinit();

	char*       cur_set_code() 	    { return set_info_array[cur_set_id-1].code; }

	void	      open_set(int);
	void	      close_set();

	Database*   open_db(char*);
	Database*   get_db();

	int         find_set(char*);

	SetInfo*    operator()()          { return set_info_array+cur_set_id-1; }
	SetInfo*    operator[](int recNo) { return set_info_array+recNo-1; }

private:
	void        load_set_header();
};

//---------------------------------------//

extern GameSet game_set;

#endif
