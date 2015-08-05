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
  #include <locale.h>
  #if (! defined LC_MESSAGES)
    #define LC_MESSAGES 1729
  #endif
#endif

#include <c99_printf.h>
#include "gettext-gnu.h"

#define _(String) gettext(String)
#define N_(String) gettext_noop(String)

#endif
