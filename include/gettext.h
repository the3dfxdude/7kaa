/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2015 Jesse Allen
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

//Filename    : gettext.h
//Description : Wrapper to GNU gettext convenience header

#ifndef __GETTEXT_H_WRAPPER
#define __GETTEXT_H_WRAPPER

// gettext-gnu.h won't define LC_MESSAGES when !defined ENABLE_NLS (happens
// when skipping libintl.h) but tries to use LC_MESSAGES anyway.
// gettext-gnu.h assumes we have a good locale.h, but that is not always the
// case.
#if (! defined ENABLE_NLS && ! defined LC_MESSAGES)
#define LC_MESSAGES 1729
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900

#include <stdio.h>

// C99 snprintf compatibility for MSVC earlier than 2015
// credit: http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#endif // _MSC_VER < 1900

#include "gettext-gnu.h"

#define _(String) gettext(String)
#define N_(String) gettext_noop(String)

#endif
