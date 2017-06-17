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

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t  BYTE;
typedef uint8_t  UCHAR;
typedef uint16_t WORD;
typedef int32_t  BOOL;
typedef uint32_t UINT;
typedef uint32_t DWORD;

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

#else // WINE || WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>

#endif


#endif // _WIN32_COMPAT_H
