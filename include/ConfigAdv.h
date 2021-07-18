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

#include <GAMEDEF.h>
#include <stdint.h>

class ConfigAdv
{
public:
	enum {
		FLAG_DEBUG_VER = 1,
		FLAG_DEVEL_VER = 2,
		FLAG_CKSUM_REQ = 4,
		FLAG_UNKNOWN_BUILD = 8,
	};

	enum {
		LOCALE_LEN = 40,
	};

	uint32_t		checksum;
	uint32_t		flags;

	// locale settings
	char			locale[LOCALE_LEN+1];

	// monster settings
	char			monster_alternate_attack_curve;
	int			monster_attack_divisor;

	// nation settings
	char			nation_ai_unite_min_relation_level;
	int			nation_start_god_level;
	int			nation_start_tech_inc_all_level;

	// race settings
	char			race_random_list[MAX_RACE];
	int			race_random_list_max;

	// remote settings
	char		        remote_compare_object_crc;
	char			remote_compare_random_seed;

	// scenario settings
	char			scenario_config;

	// town settings
	int			town_ai_emerge_nation_pop_limit;
	int			town_ai_emerge_town_pop_limit;
	char			town_loyalty_qol;

	// unit settings
	char			unit_loyalty_require_local_leader;
	char			unit_spy_fixed_target_loyalty;
	char			unit_target_move_range_cycle;

	// vga settings
	char			vga_allow_highdpi;
	char			vga_full_screen;
	char			vga_full_screen_desktop;
	char			vga_keep_aspect_ratio;
	char			vga_pause_on_focus_loss;

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
