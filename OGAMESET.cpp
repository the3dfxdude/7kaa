//Filename    : OGAMESET.CPP
//Description : Object Game Set

#include <ALL.h>
#include <OSTR.h>
#include <ODIR.h>
#include <OSYS.h>
#include <OGAMESET.h>


//-------- Define constant -------------//

#define SET_HEADER_DB       "HEADER"

//--------- Begin of GameSet::init -------------//

void GameSet::init()
{
	set_opened_flag=0;   // no set has been opened

	load_set_header();     // load the info. of all sets

	init_flag=1;
}
//----------- End of GameSet::init -------------//


//--------- Begin of GameSet::deinit -------------//

void GameSet::deinit()
{
	if( init_flag )
	{
		mem_del(set_info_array);

		init_flag = 0;
	}
}
//----------- End of GameSet::deinit -------------//


//--------- Begin of GameSet::load_set_header -------------//

void GameSet::load_set_header()
{
	int        i;
	Directory  setDir;
	SetRec    *setRec;
   SetInfo   *setInfo;
   char      *dataPtr;
   String     str;

   setDir.read( DIR_RES"*.SET", 1 );	// read in the file list of all game sets in the directory, 1-Sort file names

   set_count = setDir.size();	// no. of game set available to choose

   err_when( !set_count ); 	// error if there is no GameSet

   set_info_array = (SetInfo*) mem_add( sizeof(SetInfo) * set_count );

   //-------- read in the headers of all game sets -------//

   for( i=1 ; i<=setDir.size() ; i++ )
   {
      str  = DIR_RES;
      str += setDir[i]->name;

      set_res.init( str, 0 );       // open the game set first

      dataPtr = set_res.read(SET_HEADER_DB);	// get the pointer to the header database

      err_when( !dataPtr );

      set_db.open_from_buf(dataPtr);		// read in the header database

      setRec  = (SetRec*) set_db.read(1);	// the header database only contains one record
      setInfo = set_info_array+i-1;

      m.rtrim_fld( setInfo->code, setRec->code, setRec->CODE_LEN );
      m.rtrim_fld( setInfo->des , setRec->des , setRec->DES_LEN  );
	}
}
//----------- End of GameSet::load_set_header -------------//


//--------- Begin of GameSet::open_set -------------//
//
// Open the specific set.
// Only one set can be activiated at a time in GameSet.
//
// <int> setId = the id. of the set
//
void GameSet::open_set(int setId)
{
   err_if( setId<0 || setId>set_count )
      err_here();

   cur_set_id = setId;

   String str;

   str  = DIR_RES;
   str += set_info_array[setId-1].code;
   str += ".SET";

   set_res.init( str, 0 );      // 0-don't read all into the buffer

	//-----------------------------------------------//

	set_opened_flag=1;
}
//----------- End of GameSet::open_set -------------//


//--------- Begin of GameSet::close_set -------------//
//
// Close current set
//
void GameSet::close_set()
{
	set_res.deinit();

	set_opened_flag=0;
}
//----------- End of GameSet::close_set -------------//


//--------- Begin of GameSet::open_db -------------//
//
// Open a database in the current set
//
// <char*> dbName = the name of the database to be opened
//
Database* GameSet::open_db(char* dbName)
{
	err_when(!set_opened_flag);

	char* dataPtr = set_res.read(dbName);

	err_when( !dataPtr );

	set_db.open_from_buf(dataPtr);

	return &set_db;
}
//----------- End of GameSet::open_db -------------//


//--------- Begin of GameSet::get_db -------------//

Database* GameSet::get_db()
{
	err_when(!set_opened_flag);

	return &set_db;
}
//----------- End of GameSet::get_db -------------//


//--------- Begin of GameSet::find_set -------------//
//
// Look for a specified game set
//
// <char*> setCode = the code of the game set.
//
// return : <int> >=1 - the id. of the game set
//		    0 - the game set code is not found
//
int GameSet::find_set(char* setCode)
{
   int i;

   for( i=0 ; i<set_count ; i++ )
   {
      if( strcmp( set_info_array[i].code, setCode )==0 )
			return i+1;
   }

   return 0;
}
//----------- End of GameSet::find_set -------------//
