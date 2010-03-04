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

#include <OFILE.h>

//--------- Define class ResourceDb ----------//

class ResourceDb : public File
{
private:

    bool    init_flag;
    int     use_common_buf;
    char    *data_buf;
    int     data_buf_size;
    char    read_all;

public:

    ResourceDb()   { init_flag=0; }
    ~ResourceDb()  { deinit(); }

    bool  initialized() { return init_flag; }
    void  deinit();
    void  init_imported(const char * filename, int cacheWholeFile, int useCommonBuffer = 0);
    char* read_imported(long);
};

#endif
