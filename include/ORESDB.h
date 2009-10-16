/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : ORESDB.H
//Description : Header file of Object Resource Database

#ifndef __ORESDB_H
#define __ORESDB_H

#ifndef __ALL_H
#include <ALL.h>
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
