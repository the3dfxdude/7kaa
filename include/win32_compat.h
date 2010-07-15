/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2010 Jesse Allen
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


#ifndef _WIN32_COMPAT_H
#define _WIN32_COMPAT_H


#ifdef NO_WINDOWS // !WINE && !WIN32

#include <stdio.h>
#include <ctype.h>

typedef unsigned char BYTE;
typedef unsigned char UCHAR;
typedef unsigned short WORD;
typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned long DWORD;

#define TRUE 1
#define FALSE 0

#define MAX_PATH 260

typedef struct {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME;

typedef struct {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME;

inline char *itoa(int num, char *str, int radix)
{
	sprintf(str,"%d",num);
	return str;
}

inline char *ltoa(long num, char *str, int radix)
{
	sprintf(str,"%ld",num);
	return str;
}

#define strcmpi(s1,s2) strcasecmp(s1,s2)
#define strnicmp(s1,s2,len) strncasecmp(s1,s2,len)

#define _rotr(value,shift) (value<<shift)|(value>>(32-shift))

inline char *strupr(char *str)
{
	while (*str)
	{
		*str = toupper(*str);
		str++;
	}
	return str;
}

inline char *strlwr(char *str)
{
	while (*str)
	{
		*str = tolower(*str);
		str++;
	}
	return str;
}

#else // WINE || WIN32

#include <windows.h>

#endif


#endif // _WIN32_COMPAT_H
