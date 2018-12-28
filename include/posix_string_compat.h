/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2010 Jesse Allen
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
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


#ifndef _POSIX_STRING_COMPAT_H
#define _POSIX_STRING_COMPAT_H

#include <string.h>

#ifndef USE_WINDOWS
#include <strings.h>
#define strcmpi(s1,s2) strcasecmp(s1,s2)
#define strnicmp(s1,s2,len) strncasecmp(s1,s2,len)
#endif


#endif // _POSIX_STRING_COMPAT_H
