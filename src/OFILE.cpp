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

//Filename    : OFILE.CPP
//Description : Object File

#include <ALL.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <dbglog.h>
#include <OFILE.h>
#include <errno.h>

DBGLOG_DEFAULT_CHANNEL(File);

//-------- Begin of function File::file_open ----------//
//
// Open an existing file for reading
//
// <char*> fileName      = name of the file
// [int]   handleError   = Treat RW operation failures as fatal or not
//                         (default true, 1). Original code maked the game 
//                         quit immediately on fatal errors; now this only
//                         affects error channel being used to output error
//                         message (ERR or MSG)
// [int]   fileType = FLAT (0, default) or STRUCTURED (1). Structured file stores 
//                    its data as a records, where every record has the
//                    following format: [entry size: uint16; entry data: bytes]
//                    However this behaviour is used with read/write methods only;
//                    put/get methods (e.g. put_short) still write raw data.
//
// return : 1-success, 0-fail
//
int File::file_open(const char* fileName, int handleError, int fileType)
{
	char name[MAX_PATH+1];
	int size = strlen(fileName);

	if(strlen(fileName) > MAX_PATH)
	{
		if (handleError)
			err.run("File : file name is too long.");
		return 0;
	}

	if (file_handle != NULL)
		file_close();

	strcpy(name, fileName);
	for (int i = 0; i < size; i++)
	{
		if (name[i] == '\\') name[i] = '/';
	}

	handle_error = handleError;
	file_type = (FileType)fileType;

	file_handle = fopen(name, "rb");
	if (!file_handle)
	{
		for (int i = 0; i < size; i++)
		{
			name[i] = tolower(name[i]);
		}
		file_handle = fopen(name, "rb");
	}
	if (!file_handle)
	{
		if (handleError)
			err.run("[File::file_open] error opening file %s: %s\n", name, strerror(errno));
		return 0;
	}

	strcpy(file_name, name);

	MSG("[File::file_open] opened %s\n", file_name);
	return 1;
}
//---------- End of function File::file_open ----------//


//-------- Begin of function File::file_create ----------//
//
// Create a new file for writing (reading is also permitted)
//
// <char*> fileName      = name of the file
// [int]   handleError   = Treat RW operation failures as fatal or not
//                         (default true, 1). Original code maked the game 
//                         quit immediately on fatal errors; now this only
//                         affects error channel being used to output error
//                         message (ERR or MSG)
// [int]   fileType = FLAT (0, default) or STRUCTURED (1). Structured file stores 
//                    its data as a records, where every record has the
//                    following format: [entry size: uint16; entry data: bytes]
//                    However this behaviour is used with read/write methods only;
//                    put/get methods (e.g. put_short) still write raw data.
//
// return : 1-success, 0-fail
//
int File::file_create(const char* fileName, int handleError, int fileType)
{
	if(strlen(fileName) > MAX_PATH)
	{
		if (handleError)
			err.run("File : file name is too long.");
		return 0;
	}

	strcpy(file_name, fileName);
	// FIXME: this fileName handling is broken
	for (int i = 0; i < strlen(fileName); i++)
	{
		file_name[i] = tolower(file_name[i]);
		if (file_name[i] == '\\') file_name[i] = '/';
	}

	handle_error = handleError;
	file_type = (FileType)fileType;

	file_handle = fopen(fileName, "wb+");
	if (!file_handle)
	{
		if (handleError)
			err.run("[File::file_create] couldn't create %s: %s\n", fileName, strerror(errno));
		return 0;
	}

	MSG("[File::file_create] created %s\n", file_name);
	return 1;
}
//---------- End of function File::file_create ----------//

//-------- Begin of function File::file_close ----------//
//
void File::file_close()
{
	if (file_handle != NULL)
	{
		MSG("[File::file_close] closing %s\n", file_name);
		file_name[0] = '\0';
		if (fclose(file_handle))
		{
			MSG("Error closing file descriptor: %s\n", strerror(errno));	
		}
		file_handle = NULL;
	}
}
//---------- End of function File::file_close ----------//


//-------- Begin of function File::~File ----------//
//
File::~File()
{
	file_close();
}
//---------- End of function File::~File ----------//


//-------- Begin of function File::file_write ----------//
//
// Write a block of data to the file
//
// <void*>    dataBuf = pointer to data buffer to be written to the file
// <unsigned> dataSize = length of the data (must < 64K)
//
// return : 1-success, 0-fail
//
int File::file_write(void* dataBuf, unsigned dataSize)
{
	err_when(!file_handle);

	if (file_type == File::STRUCTURED)
	{
		// writing record size
		if (dataSize > 0xFFFF)
		{
			// If 'dataSize' exceed the unsigned short limit, write 0
			// instead of actual record size. Such a record may be
			// correctly read though (if proper data size will be
			// specified when calling successive file_read).
			file_put_unsigned_short(0);
			MSG("[File::file_write] warning: record size exceeds uint16 MAX value\n");
		}
		else
		{
			file_put_unsigned_short(dataSize);
		}
	}

	fwrite(dataBuf, 1, dataSize, file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_write] error occured while writing file: %s\n", file_name);
		else
			ERR("[File::file_write] error occured while writing file: %s\n", file_name);
		return 0;
	}

	return 1;
}
//---------- End of function File::file_write ----------//


//-------- Begin of function File::file_read ----------//
//
// Read a block of data from the file
//
// <void*>    dataBuf = pointer to data buffer to be written to the file
// <unsigned> dataSize = length of the data (must < 64K)
//
// return : 1-success, 0-fail
//
int File::file_read(void* dataBuf, unsigned dataSize)
{
	err_when(!file_handle);

	unsigned bytesToRead = dataSize, recordSize = dataSize;

	if (file_type == File::STRUCTURED)
	{
		recordSize = file_get_unsigned_short();
		if (recordSize && recordSize < dataSize) // recordSize==0, if the size > 0xFFFF
			bytesToRead = recordSize; // the read size is the minimum of the record size and the supposed read size
	}

	fread(dataBuf, 1, bytesToRead, file_handle);

	// In the case of file_type == File::STRUCTURED
	// if the record was read partially,
	// skip remaining bytes in record and seek to next one
	if (bytesToRead < recordSize)
		file_seek(recordSize - bytesToRead, SEEK_CUR);

	// In the case of file_type == File::STRUCTURED
	// if the actual record size was smaller than requested data size,
	// fill the remaining bytes in buffer with 0
	if (bytesToRead < dataSize)
		memset((char*)dataBuf + bytesToRead, 0, dataSize - bytesToRead);

	if (ferror(file_handle))
	{
		// This used to prompt for a retry -- was this necessary?
		if (handle_error)
			err.run("[File::file_read] error occured while reading file: %s\n", file_name);
		else
			ERR("[File::file_read] error occured while reading file: %s\n", file_name);
		return 0;
	}

	return 1;
}
//---------- End of function File::file_read ----------//


int File::file_put_short(int16_t value)
{
	err_when(!file_handle);

	fwrite(&value, 1, sizeof(int16_t), file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_put_short] error occured while writing file: %s\n", file_name);
		else
			ERR("[File::file_put_short] error occured while writing file: %s\n", file_name);
		return 0;
	}

	return 1;
}

int16_t File::file_get_short()
{
    	err_when(!file_handle);

    int16_t value;
    fread(&value, 1, sizeof(int16_t), file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_get_short] error occured while reading file: %s\n", file_name);
		else
			ERR("[File::file_get_short] error occured while reading file: %s\n", file_name);
		return 0;
	}

	return value;
}

int File::file_put_unsigned_short(uint16_t value)
{
    	err_when(!file_handle);

    fwrite(&value, 1, sizeof(uint16_t), file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_put_unsigned_short] error occured while writing file: %s\n", file_name);
		else
			ERR("[File::file_put_unsigned_short] error occured while writing file: %s\n", file_name);
		return 0;
	}

	return 1;
}

uint16_t File::file_get_unsigned_short()
{
    	err_when(!file_handle);

    uint16_t value;
    fread(&value, 1, sizeof(uint16_t), file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_get_unsigned_short] error occured while reading file: %s\n", file_name);
		else
			ERR("[File::file_get_unsigned_short] error occured while reading file: %s\n", file_name);
		return 0;
	}

	return value;
}

int File::file_put_long(int32_t value)
{
    	err_when(!file_handle);

    fwrite(&value, 1, sizeof(int32_t), file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_put_long] error occured while writing file: %s\n", file_name);
		else
			ERR("[File::file_put_long] error occured while writing file: %s\n", file_name);
		return 0;
	}

	return 1;
}

int32_t File::file_get_long()
{
    	err_when(!file_handle);

    int32_t value;
    fread(&value, 1, sizeof(int32_t), file_handle);

	if (ferror(file_handle))
	{
		if (handle_error)
			err.run("[File::file_get_long] error occured while reading file: %s\n", file_name);
		else
			ERR("[File::file_get_long] error occured while reading file: %s\n", file_name);
		return 0;
	}

	return value;
}

//---------- Start of function File::file_seek ---------//
// whence = SEEK_SET, SEEK_CUR, SEEK_END (default: SEEK_SET)
// return : new offset from the file beginning.
long File::file_seek(long offset, int whence)
{
	fseek(file_handle, offset, whence);
	return ftell(file_handle);
}

long File::file_pos()
{
	return ftell(file_handle);
}

long File::file_size()
{
	long actual = ftell(file_handle);
	fseek(file_handle, 0, SEEK_END);

	long size = ftell(file_handle);

	fseek(file_handle, actual, SEEK_SET);
	return size;
}
