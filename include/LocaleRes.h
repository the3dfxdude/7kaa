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

//Filename    : LocaleRes.h
//Description : Header file for locale resources

#ifndef __LOCALERES_H
#define __LOCALERES_H

#ifdef ENABLE_NLS
#include <iconv.h>
#endif

struct LocaleRec
{
	enum { LANG_LEN=4, FONTSET_LEN=16, CODESET_LEN=16 };

	char lang[LANG_LEN];
	char fontset[FONTSET_LEN];
	char codeset[CODESET_LEN];
};

class LocaleRes
{
public:
	int init_flag;
	char lang[LocaleRec::LANG_LEN+1];
	char fontset[LocaleRec::FONTSET_LEN+1];
	char codeset[LocaleRec::CODESET_LEN+1];
#ifdef ENABLE_NLS
	iconv_t cd;
	iconv_t cd_latin;
	iconv_t cd_from_sdl;
#endif

private:
	char *in_buf;
	char *out_buf;
	size_t in_buf_size;
	size_t out_buf_size;

//---------------------------------------//

public:
	LocaleRes();
	~LocaleRes();

	void init();
	void deinit();
	void load();

#ifdef ENABLE_NLS
	const char *conv_str(iconv_t cd, const char *s);
#endif
};

extern LocaleRes locale_res;

#endif
