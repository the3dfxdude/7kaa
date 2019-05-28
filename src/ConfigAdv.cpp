/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2019 Jesse Allen
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

//Filename    : ConfigAdv.cpp
//Description : Advanced Config

#include <ConfigAdv.h>
#include <FilePath.h>
#include <OFILETXT.h>
#include <OMISC.h>
#include <OSYS.h>
#include <posix_string_compat.h>
#include <version.h>
#include <errno.h>
#include "gettext.h"


//--------- Begin of function ConfigAdv::ConfigAdv -----------//

ConfigAdv::ConfigAdv()
{
	checksum = 0;
	flags = 0;
	#ifdef DEBUG
		flags |= FLAG_DEBUG_VER;
	#endif
	#ifdef DEV_VERSION
		flags |= FLAG_DEVEL_VER;
	#endif
}
//--------- End of function ConfigAdv::ConfigAdv --------//


//--------- Begin of function ConfigAdv::~ConfigAdv -----------//

ConfigAdv::~ConfigAdv()
{
}
//--------- End of function ConfigAdv::ConfigAdv --------//


//--------- Begin of function ConfigAdv::init -------------//
//
int ConfigAdv::init()
{
	char filename[] = "config.txt";
	reset();
	if( !load(filename) )
	{
		reset();
		return 0;
	}
	return 1;
}

//--------- Begin of function ConfigAdv::load -------------//
//
int ConfigAdv::load(char *filename)
{
	FilePath full_path(sys.dir_config);
	full_path += filename;
	if( full_path.error_flag )
		return 0;
	if( !misc.is_file_exist(full_path) )
	{
		full_path = DIR_RES;
		full_path += filename;
		if( full_path.error_flag || !misc.is_file_exist(full_path) )
			return 0;
	}

	FileTxt fileTxt(full_path);
	int line = 0;

	while( !fileTxt.is_eof() )
	{
		char *name;
		char *value;
		char save;

		line++;

		fileTxt.match_chars(" \t");
		name = fileTxt.data_ptr;
		if( !fileTxt.match_chars_ex("= \t\r\n\x1a") )
		{
			fileTxt.next_line();
			continue;
		}
		fileTxt.match_chars(" \t");

		if( *fileTxt.data_ptr != '=' )
			goto err_out;
		*fileTxt.data_ptr = 0;
		fileTxt.data_ptr++;

		fileTxt.match_chars(" \t");
		value = fileTxt.data_ptr;
		if( !fileTxt.match_chars_ex("\r\n\x1a") )
			goto err_out;

		// Preserve any newline/return so next_line() can know how to correctly position.
		save = *fileTxt.data_ptr;
		*fileTxt.data_ptr = 0;

		misc.rtrim(name);
		misc.rtrim(value);
		if( !set(name, value) )
			goto err_out;

		*fileTxt.data_ptr = save;
		fileTxt.next_line();
	}

	return 1;

err_out:
	String error_msg;
	error_msg.catf(_("Error in %s at line %d"), filename, line);
	sys.show_error_dialog(error_msg);
	return 0;
}
//--------- End of function ConfigAdv::load -------------//


//--------- Begin of function ConfigAdv::reset ---------//
//
void ConfigAdv::reset()
{
	vga_allow_highdpi = 0;
	vga_full_screen = 1;
	vga_keep_aspect_ratio = 1;

	vga_window_width = 0;
	vga_window_height = 0;

	// after applying defaults, checksum is not required
	checksum = 0;
	flags &= ~FLAG_CKSUM_REQ;
}
//--------- End of function ConfigAdv::reset ---------//


//--------- Begin of function ConfigAdv::set -------------//
//
// After any user-forced setting that modifies gameplay, update_check_sum().
// Non-gameplay settings will not require a checksum.
int ConfigAdv::set(char *name, char *value)
{
	if( !strcmp(name, "vga_allow_highdpi") )
	{
		if( !strcmpi(value, "true") )
			vga_allow_highdpi = 1;
		else if( !strcmpi(value, "false") )
			vga_allow_highdpi = 0;
		else
			return 0;
	}
	else if( !strcmp(name, "vga_full_screen") )
	{
		if( !strcmpi(value, "true") )
			vga_full_screen = 1;
		else if( !strcmpi(value, "false") )
			vga_full_screen = 0;
		else
			return 0;
	}
	else if( !strcmp(name, "vga_keep_aspect_ratio") )
	{
		if( !strcmpi(value, "true") )
			vga_keep_aspect_ratio = 1;
		else if( !strcmpi(value, "false") )
			vga_keep_aspect_ratio = 0;
		else
			return 0;
		// TODO: Update active renderer
	}
	else if( !strcmp(name, "vga_window_height") )
	{
		char *endptr;
		int tmp = strtol(value, &endptr, 10);
		if( endptr == value || *endptr )
			return 0;
		vga_window_height = tmp;
	}
	else if( !strcmp(name, "vga_window_width") )
	{
		char *endptr;
		int tmp = strtol(value, &endptr, 10);
		if( endptr == value || *endptr )
			return 0;
		vga_window_width = tmp;
	}
	else
	{
		return 0;
	}
	return 1;
}
//--------- End of function ConfigAdv::set -------------//


//--------- Begin of function ConfigAdv::update_check_sum -------------//
//
void ConfigAdv::update_check_sum(char *name, char *value)
{
	checksum += misc.check_sum(name);
	checksum += misc.check_sum(value);
	flags |= FLAG_CKSUM_REQ;
}
//--------- End of function ConfigAdv::update_check_sum -------------//
