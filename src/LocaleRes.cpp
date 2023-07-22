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

//Filename    : LocaleRes.cpp
//Description : Locale Resources

#include <stdlib.h>
#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#endif

#include <ALL.h>
#include <ODB.h>
#include <LocaleRes.h>


#ifndef HAVE_SETENV
static String lc_all_str;
int setenv(const char *name, const char *value, int overwrite)
{
	if( !value )
		return putenv(name);

	lc_all_str = name;
	lc_all_str += "=";
	lc_all_str += value;
	return putenv(lc_all_str);
}
#endif


//------------- End of function Constructor -------//
//
LocaleRes::LocaleRes()
{
	lang[0] = 0;
	fontset[0] = 0;
	codeset[0] = 0;
	init_flag = 0;
#ifdef ENABLE_NLS
	cd = (iconv_t)-1;
	cd_latin = (iconv_t)-1;
	cd_from_sdl = (iconv_t)-1;
#endif
	in_buf = NULL;
	out_buf = NULL;
}
//------------- End of function Constructor -------//


//----------- Begin of function Destructor -------//
//
LocaleRes::~LocaleRes()
{
	deinit();
}
//------------- End of function Destructor -------//


//-------- Begin of function LocaleRes::init -----------//
//
#define INIT_BUF_SIZE 2048
void LocaleRes::init()
{
#ifdef ENABLE_NLS
	const char *locale_dir;
	locale_dir = get_locale_dir();
	if( locale_dir )
	{
		bindtextdomain(PACKAGE, locale_dir);
	}
	textdomain(PACKAGE);
	setlocale(LC_ALL, "");
	load(getenv("SKMESSAGES"));

	in_buf = mem_add(INIT_BUF_SIZE+1);
	in_buf_size = INIT_BUF_SIZE;
	out_buf = mem_add(INIT_BUF_SIZE+1);
	out_buf_size = INIT_BUF_SIZE;
#endif

	init_flag = 1;
}
//---------- End of function LocaleRes::init ----------//


//----------- Start of function LocaleRes::deinit ---------//
//
void LocaleRes::deinit()
{
	if( !init_flag )
		return;
#ifdef ENABLE_NLS
	if( cd != (iconv_t)-1 )
		iconv_close(cd);
	if( cd_latin != (iconv_t)-1 )
		iconv_close(cd_latin);
	cd = (iconv_t)-1;
	cd_latin = (iconv_t)-1;
#endif
	if( in_buf )
		mem_del(in_buf);
	if( out_buf )
		mem_del(out_buf);
	init_flag = 0;
}
//------------- End of function LocaleRes::deinit ---------//


//----------- Start of function LocaleRes::load ---------//
//
// Performs setlocale and initializes codeset conversion.
//
void LocaleRes::load(const char *locale)
{
#ifdef ENABLE_NLS
	if( locale && locale[0] )
	{
		setlocale(LC_MESSAGES, locale);
		setlocale(LC_CTYPE, locale);
#ifndef HAVE_LC_MESSAGES
		// Gettext fakes the setlocale for LC_MESSAGES above via a
		// wrapper, set the env var the same way since the var is
		// actually used by gettext instead of the real setlocale.
		setenv("LC_MESSAGES", locale, 1);
#endif
	}
	locale = get_messages_locale();

	if( !locale || !locale[0] )
	{
		// The platform doesn't have full POSIX localization, and the
		// user did not specify a locale in the game config. Default to
		// English. The reason why to do this is if gettext does end up
		// mapping the locale internally, we don't know what font to
		// use. English is a safe choice.
		locale = "en_US";
		setlocale(LC_MESSAGES, locale);
		setlocale(LC_CTYPE, locale);
		setenv("LC_MESSAGES", locale, 1);
	}

	LocaleRec *localeRec;
	String localeDbName;
	localeDbName = DIR_RES;
	localeDbName += "LOCALE.RES";
	Database localeDbObj(localeDbName, 1);
	Database *dbLocale = &localeDbObj;

	short locale_count = (short) dbLocale->rec_count();

	//------ read in locale information -------//

	int i;
	for( i=0 ; i<locale_count ; i++ )
	{
		localeRec = (LocaleRec*) dbLocale->read(i+1);

		misc.rtrim_fld( lang, localeRec->lang, localeRec->LANG_LEN );
		if( !misc.str_icmpx(locale, lang) )
			continue;

		misc.rtrim_fld( fontset, localeRec->fontset, localeRec->FONTSET_LEN );
		misc.rtrim_fld( codeset, localeRec->codeset, localeRec->CODESET_LEN );
		break;
	}

	if( i >= locale_count )
	{
		strcpy(lang, "??");
		strcpy(codeset, "ISO-8859-1");
	}

	String tocode(codeset);
	tocode += "//TRANSLIT";

	if( cd != (iconv_t)-1 )
		iconv_close(cd);
	if( cd_latin != (iconv_t)-1 )
		iconv_close(cd_latin);
	cd = iconv_open(tocode, "");
	cd_latin = iconv_open("ISO-8859-1", "");
	cd_from_sdl = iconv_open("ISO-8859-1//TRANSLIT//IGNORE", "UTF-8");
#endif
}
//------------- End of function LocaleRes::load ---------//


#ifdef ENABLE_NLS
#define BUF_INCR 1000
const char *LocaleRes::conv_str(iconv_t cd, const char *s)
{
	if( cd == (iconv_t)-1 )
		return s;

	size_t in_left;
	size_t out_left;
	char *p1 = in_buf;
	char *p2 = out_buf;

	in_left = strlen(s);
	if( in_left > in_buf_size )
	{
		in_buf_size = in_left;
		in_buf = mem_resize(in_buf, in_buf_size+1);
	}
	strncpy(in_buf, s, in_left);
	in_buf[in_left] = 0;
	out_left = out_buf_size;

	size_t c;
	while( in_left>0 )
	{
		c = iconv(cd, &p1, &in_left, &p2, &out_left);
		if( c == (size_t)-1 )
			return s;
		if( in_left )
		{
			out_left += BUF_INCR;
			out_buf_size += BUF_INCR;
			out_buf = mem_resize(out_buf, out_buf_size+1);
		}
	}
	out_buf[out_buf_size-out_left] = 0;

	return out_buf;
}
#endif


//-------- Begin of function LocaleRes::get_locale_dir -----------//
//
const char *LocaleRes::get_locale_dir()
{
	if( misc.is_file_exist("locale") )
		return "locale";
	if( misc.is_file_exist(getenv("SKLOCALE")) )
		return getenv("SKLOCALE");
#ifdef LOCALE_DIR
	if( misc.is_file_exist(LOCALE_DIR) )
		return LOCALE_DIR;
#endif
	return NULL;
}
//---------- End of function LocaleRes::get_locale_dir ----------//


//-------- Begin of function LocaleRes::get_messages_locale -----------//
//
const char *LocaleRes::get_messages_locale()
{
	const char *locale;

#ifdef HAVE_LC_MESSAGES
	/* setlocale(LC_ALL, "") has been done previously */
	locale = setlocale(LC_MESSAGES, NULL);
	if( locale && locale[0] )
		return locale;
#else /* Missing LC_MESSAGES */
	// gettext uses the env vars on this platform
	locale = getenv("LC_ALL");
	if( locale && locale[0] )
		return locale;

	locale = getenv("LC_MESSAGES");
	if( locale && locale[0] )
		return locale;

	locale = getenv("LANG");
	if( locale && locale[0] )
		return locale;
#endif
	// We don't spend the time to map what Windows uses for locales. And
	// some platforms don't have a POSIX setlocale, so if the user does not
	// manually set an option in this case, we don't know the locale.

	return NULL;
}
//---------- End of function LocaleRes::get_messages_locale ----------//
