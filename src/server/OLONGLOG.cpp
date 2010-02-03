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

// Filename   : OLONGLOG.CPP

#include <OLONGLOG.H>
#include <stdio.h>


LongLog::LongLog(char suffix)
{
	char filename[] = "LLONGx.LOG";
	filename[5] = suffix;
	file_create(filename);
}

LongLog::~LongLog()
{
	file_close();
}

void LongLog::printf(char *format, ...)
{
	//---- translate the message and the arguments into one message ----//

	char strBuf[150];

	va_list argPtr;        // the argument list structure

	va_start( argPtr, format );
	vsprintf( strBuf, format, argPtr );
	file_write(strBuf, strlen(strBuf));
	va_end( argPtr );
}


