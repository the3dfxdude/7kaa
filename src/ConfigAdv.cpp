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
#include <ONATIONB.h>
#include <OFILETXT.h>
#include <OMISC.h>
#include <OMOUSE.h>
#include <OSYS.h>
#include <posix_string_compat.h>
#include <version.h>
#include <errno.h>
#include "gettext.h"

#define CHECK_BOUND(n,x,y) n<x || n>y

union KeyEventMap
{
	int index;
	KeyEventType type;
};
static const char *keyevent_map[] = {
	"KEYEVENT_UNSET",

	"KEYEVENT_FIRM_BUILD",
	"KEYEVENT_FIRM_PATROL",

	"KEYEVENT_TOWN_RECRUIT",
	"KEYEVENT_TOWN_TRAIN",

	"KEYEVENT_UNIT_BUILD",
	"KEYEVENT_UNIT_RETURN",
	"KEYEVENT_UNIT_SETTLE",
	"KEYEVENT_UNIT_UNLOAD",

	"KEYEVENT_BUILD_BASE",
	"KEYEVENT_BUILD_CAMP",
	"KEYEVENT_BUILD_FACTORY",
	"KEYEVENT_BUILD_HARBOR",
	"KEYEVENT_BUILD_INN",
	"KEYEVENT_BUILD_MARKET",
	"KEYEVENT_BUILD_MINE",
	"KEYEVENT_BUILD_MONSTER",
	"KEYEVENT_BUILD_RESEARCH",
	"KEYEVENT_BUILD_WAR_FACTORY",

	"KEYEVENT_MAP_MODE_CYCLE",
	"KEYEVENT_MAP_MODE0",
	"KEYEVENT_MAP_MODE1",
	"KEYEVENT_MAP_MODE2",
	"KEYEVENT_REPORT_OPAQUE_TOGGLE",
	"KEYEVENT_CLEAR_NEWS",
	"KEYEVENT_OPEN_DIPLOMATIC_MSG",
	"KEYEVENT_OPEN_OPTION_MENU",

	"KEYEVENT_TUTOR_PREV",
	"KEYEVENT_TUTOR_NEXT",

	"KEYEVENT_SAVE_GAME",
	"KEYEVENT_LOAD_GAME",

	"KEYEVENT_OBJECT_PREV",
	"KEYEVENT_OBJECT_NEXT",
	"KEYEVENT_NATION_OBJECT_PREV",
	"KEYEVENT_NATION_OBJECT_NEXT",

	"KEYEVENT_GOTO_RAW",
	"KEYEVENT_GOTO_KING",
	"KEYEVENT_GOTO_GENERAL",
	"KEYEVENT_GOTO_SPY",
	"KEYEVENT_GOTO_SHIP",
	"KEYEVENT_GOTO_CAMP",

	"KEYEVENT_CHEAT_ENABLE1",
	"KEYEVENT_CHEAT_ENABLE2",
	"KEYEVENT_CHEAT_ENABLE3",

	"KEYEVENT_MANUF_QUEUE_UP",
	"KEYEVENT_MANUF_QUEUE_DOWN",
	"KEYEVENT_MANUF_QUEUE_ADD",
	"KEYEVENT_MANUF_QUEUE_ADD_BATCH",
	"KEYEVENT_MANUF_QUEUE_REMOVE",
	"KEYEVENT_MANUF_QUEUE_REMOVE_BATCH",

	"KEYEVENT_MAX"
};

static int read_int(char *in, int *out);
static int read_bool(char *in, char *out);
static int read_key(char *in, char **out, KeyEventMap *event);

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
	#ifndef HAVE_KNOWN_BUILD
		flags |= FLAG_UNKNOWN_BUILD;
	#endif

	// this is set on program load for LocaleRes
	locale[0] = 0;
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
		full_path = filename;
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
	firm_mobilize_civilian_aggressive = 0;
	firm_migrate_stricter_rules = 1;

	fix_recruit_dec_loyalty = 1;
	fix_town_unjob_worker = 1;

	locale[0] = 0;

	monster_alternate_attack_curve = 0;
	monster_attack_divisor = 4;

	nation_ai_unite_min_relation_level = NATION_NEUTRAL;
	nation_start_god_level = 0;
	nation_start_tech_inc_all_level = 0;

	race_random_list_max = MAX_RACE;
	for (int i = 0; i < race_random_list_max; i++)
		race_random_list[i] = i+1;

	remote_compare_object_crc = 1;
	remote_compare_random_seed = 1;

	scenario_config = 1;

	town_ai_emerge_nation_pop_limit = 60 * MAX_NATION;
	town_ai_emerge_town_pop_limit = 1000;
	town_migration = 1;
	town_loyalty_qol = 1;

	unit_ai_team_help = 1;
	unit_finish_attack_move = 1;
	unit_loyalty_require_local_leader = 1;
	unit_spy_fixed_target_loyalty = 0;
	unit_target_move_range_cycle = 0;

	vga_allow_highdpi = 0;
	vga_full_screen = 1;
	vga_full_screen_desktop = 1;
	vga_keep_aspect_ratio = 1;
	vga_pause_on_focus_loss = 0;

	vga_window_width = 0;
	vga_window_height = 0;

	wall_building_allowed = 0;

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
	if( !strcmp(name, "bindkey") )
	{
		KeyEventMap event;
		char *key;
		if( !read_key(value, &key, &event) || !mouse.bind_key(event.type, key) )
			return 0;
	}
	else if( !strcmp(name, "firm_mobilize_civilian_aggressive") )
	{
		if( !read_bool(value, &firm_mobilize_civilian_aggressive) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "firm_migrate_stricter_rules") )
	{
		if( !read_bool(value, &firm_migrate_stricter_rules) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "fix_recruit_dec_loyalty") )
	{
		if( !read_bool(value, &fix_recruit_dec_loyalty) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "fix_town_unjob_worker") )
	{
		if( !read_bool(value, &fix_town_unjob_worker) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "locale") )
	{
		strncpy(locale, value, LOCALE_LEN);
		locale[LOCALE_LEN] = 0;
	}
	else if( !strcmp(name, "monster_alternate_attack_curve") )
	{
		if( !read_bool(value, &monster_alternate_attack_curve) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "monster_attack_divisor") )
	{
		if( !read_int(value, &monster_attack_divisor) )
			return 0;
		if( CHECK_BOUND(monster_attack_divisor, 1, 6) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "nation_ai_unite_min_relation_level") )
	{
		if( !strcmpi(value, "hostile") )
			nation_ai_unite_min_relation_level = NATION_HOSTILE;
		else if( !strcmpi(value, "tense") )
			nation_ai_unite_min_relation_level = NATION_TENSE;
		else if( !strcmpi(value, "neutral") )
			nation_ai_unite_min_relation_level = NATION_NEUTRAL;
		else if( !strcmpi(value, "friendly") )
			nation_ai_unite_min_relation_level = NATION_FRIENDLY;
		else if( !strcmpi(value, "alliance") )
			nation_ai_unite_min_relation_level = NATION_ALLIANCE;
		else if( !strcmpi(value, "off") )
			nation_ai_unite_min_relation_level = NATION_ALLIANCE+1; // disables
		else
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "nation_start_god_level") )
	{
		if( !read_int(value, &nation_start_god_level) )
			return 0;
		if( CHECK_BOUND(nation_start_god_level, 0, 2) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "nation_start_tech_inc_all_level") )
	{
		if( !read_int(value, &nation_start_tech_inc_all_level) )
			return 0;
		if( CHECK_BOUND(nation_start_tech_inc_all_level, 0, 2) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "race_random_list") )
	{
		// the game defaults to all
		if( !strcmpi(value, "original") )
		{
			race_random_list_max = 7;
			for (int i = 0; i < race_random_list_max; i++)
				race_random_list[i] = i+1;
			update_check_sum(name, value);
		}
		else
		{
			return 0;
		}
	}
	else if( !strcmp(name, "remote_compare_object_crc") )
	{
		if( !read_bool(value, &remote_compare_object_crc) )
			return 0;
	}
	else if( !strcmp(name, "remote_compare_random_seed") )
	{
		if( !read_bool(value, &remote_compare_random_seed) )
			return 0;
	}
	else if( !strcmp(name, "scenario_config") )
	{
		if( !read_bool(value, &scenario_config) )
			return 0;
	}
	else if( !strcmp(name, "town_ai_emerge_nation_pop_limit") )
	{
		if( !read_int(value, &town_ai_emerge_nation_pop_limit) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "town_ai_emerge_town_pop_limit") )
	{
		if( !read_int(value, &town_ai_emerge_town_pop_limit) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "town_migration") )
	{
		if( !read_bool(value, &town_migration) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "town_loyalty_qol") )
	{
		if( !read_bool(value, &town_loyalty_qol) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "unit_ai_team_help") )
	{
		if( !read_bool(value, &unit_ai_team_help) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "unit_finish_attack_move") )
	{
		if( !read_bool(value, &unit_finish_attack_move) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "unit_loyalty_require_local_leader") )
	{
		if( !read_bool(value, &unit_loyalty_require_local_leader) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "unit_spy_fixed_target_loyalty") )
	{
		if( !read_bool(value, &unit_spy_fixed_target_loyalty) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "unit_target_move_range_cycle") )
	{
		if( !read_bool(value, &unit_target_move_range_cycle) )
			return 0;
		update_check_sum(name, value);
	}
	else if( !strcmp(name, "vga_allow_highdpi") )
	{
		if( !read_bool(value, &vga_allow_highdpi) )
			return 0;
	}
	else if( !strcmp(name, "vga_full_screen") )
	{
		if( !read_bool(value, &vga_full_screen) )
			return 0;
	}
	else if( !strcmp(name, "vga_full_screen_desktop") )
	{
		if( !read_bool(value, &vga_full_screen_desktop) )
			return 0;
	}
	else if( !strcmp(name, "vga_keep_aspect_ratio") )
	{
		if( !read_bool(value, &vga_keep_aspect_ratio) )
			return 0;
	}
	else if( !strcmp(name, "vga_pause_on_focus_loss") )
	{
		if( !read_bool(value, &vga_pause_on_focus_loss) )
			return 0;
	}
	else if( !strcmp(name, "vga_window_height") )
	{
		if( !read_int(value, &vga_window_height) )
			return 0;
	}
	else if( !strcmp(name, "vga_window_width") )
	{
		if( !read_int(value, &vga_window_width) )
			return 0;
	}
	else if( !strcmp(name, "wall_building_allowed") )
	{
		if( !read_bool(value, &wall_building_allowed) )
			return 0;
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


static int read_int(char *in, int *out)
{
	char *endptr;
	int tmp = strtol(in, &endptr, 10);
	if( endptr == in || *endptr )
		return 0;
	*out = tmp;
	return 1;
}


static int read_bool(char *in, char *out)
{
	if( !strcmpi(in, "true") )
		*out = 1;
	else if( !strcmpi(in, "false") )
		*out = 0;
	else
		return 0;
	return 1;
}


static int read_key(char *in, char **out, KeyEventMap *event)
{
	char *p = strchr(in, ',');
	if( !p )
		return 0;
	*p = 0;
	p++;
	if( !*p )
		return 0;
	*out = p;
	int i;
	for( i=0; i<(int)KEYEVENT_MAX; i++ )
		if( strcmp(keyevent_map[i], in)==0 )
			break;
	if( i>=(int)KEYEVENT_MAX )
		return 0;
	event->index = i;
	return 1;
}
