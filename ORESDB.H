//Filename    : ORESDB.H
//Description : Header file of Object Resource Database

#ifndef __ORESDB_H
#define __ORESDB_H

#ifndef __ALL_H
#include <ALL.H>
#endif

//--------- Define class ResourceDb ----------//

class Database;

class ResourceDb : public File
{
public:
   char			init_flag;

private:
   Database		*db_obj;
   int			index_field_offset;
   int			use_common_buf;
	char			*data_buf;
	int			data_buf_size;

   char			read_all;

public:
   ResourceDb()   { init_flag=0; }
   ~ResourceDb()  { deinit(); }

   ResourceDb(char* resName,Database* dbObj,int indexOffset,int useCommonBuf=0)
	{ init_flag=0; init(resName,dbObj,indexOffset,useCommonBuf); }

   void init(char*,Database*,int,int=0);
   void deinit();

   char* read(int= -1);
   File* get_file();

   void  init_imported(char*,int,int=0);
   char* read_imported(long);
};

#endif
