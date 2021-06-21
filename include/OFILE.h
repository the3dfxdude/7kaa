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

//Filename    : OFILE.H
//Description : Header file for file object

#ifndef __OFILE_H
#define __OFILE_H

#include <FilePath.h>

#include <stdio.h>
#include <stdint.h>

//--------------------------------------//

class File
{
public:

	enum  FileType { FLAT = 0, STRUCTURED = 1 };

	char     file_name[FilePath::MAX_FILE_PATH];
	FILE*    file_handle;
	int      handle_error;
	FileType file_type;

public:

	File(): file_handle(NULL) {}
	~File();

	int   file_open(const char*, int=1, int=0);
	int   file_create(const char*, int=1, int=0);
	void  file_close();

	long  file_size();
	long  file_seek(long, int = SEEK_SET);
	long  file_pos();

	int   file_read(void*, unsigned);
	int   file_write(void*, unsigned);

	int     file_put_char(int8_t);
	int8_t  file_get_char();

	int     file_put_short(int16_t);
	int16_t file_get_short();

	int      file_put_unsigned_short(uint16_t);
	uint16_t file_get_unsigned_short();

	int     file_put_long(int32_t);
	int32_t file_get_long();
};

#endif
