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

// Filename : ConfigAdv.h
// Description : Advanced Config

#ifndef __CONFIGADV_H
#define __CONFIGADV_H

#include <stdint.h>

class ConfigAdv
{
public:
	enum {
		FLAG_DEBUG_VER = 1,
		FLAG_DEVEL_VER = 2,
		FLAG_CKSUM_REQ = 4,
	};

	uint32_t		checksum;
	uint32_t		flags;

	// nation settings
	char			nation_ai_unite_min_relation_level;

	// town settings
	int			town_ai_emerge_nation_pop_limit;
	int			town_ai_emerge_town_pop_limit;

	// vga settings
	char			vga_allow_highdpi;
	char			vga_full_screen;
	char			vga_keep_aspect_ratio;

	int			vga_window_width;
	int			vga_window_height;

public:
	ConfigAdv();
	~ConfigAdv();

	int			init();
	int			load(char *filename);
	void			reset();
	int			set(char *name, char *value);

private:
	void			update_check_sum(char *name, char *value);
};

//------------------------------------------//

extern ConfigAdv config_adv;

#endif
