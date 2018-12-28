/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2018 Jesse Allen
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

//Filename    : FilePath.h
//Description : Light string class with error checking to support safe path creation

#ifndef __FILEPATH_H
#define __FILEPATH_H

#include <GAMEDEF.h>

#include <string.h>

struct FilePath
{
	enum { MAX_FILE_PATH = 260 }; // Windows: MAX_PATH=260

	char str_buf[MAX_FILE_PATH];
	char error_flag;

	FilePath()
	{
		str_buf[0] = 0;
		error_flag = 0;
	}

	FilePath(const char *s)
	{
		strcpy(str_buf, s);
		error_flag = 0;
	}

	operator char*() { return str_buf; }

	FilePath& operator=(const char *s)
	{
		strcpy(str_buf, s);
		return *this;
	}

	FilePath& operator+=(const char *s)
	{
		size_t s_len = strlen(s);
		if( !s_len )
			return *this;
		size_t buf_len = strlen(str_buf);
		if( buf_len+s_len >= MAX_FILE_PATH )
		{
			error_flag = 1;
			return *this;
		}
		memcpy(str_buf+buf_len, s, s_len+1);
		return *this;
	}

	FilePath& operator/=(const char *s)
	{
		size_t s_len = strlen(s);
		if( !s_len )
			return *this;
		size_t buf_len = strlen(str_buf);
		size_t path_delim_len = strlen(PATH_DELIM);
		if( buf_len+path_delim_len+s_len >= MAX_FILE_PATH )
		{
			error_flag = 1;
			return *this;
		}
		memcpy(str_buf+buf_len, PATH_DELIM, path_delim_len);
		memcpy(str_buf+buf_len+path_delim_len, s, s_len+1);
		return *this;
	}
};

#endif
