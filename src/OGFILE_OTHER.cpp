/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
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

//Filename    : OGFILE_OTHER.CPP
//Description : Object Game file, save game and restore game, part 2, other (Config, Sys, etc.)


#include <OGAME.h>
#include <OCONFIG.h>
#include <OSYS.h>
#include <OPOWER.h>
#include <OINFO.h>
#include <OTUTOR.h>
#include <OWEATHER.h>
#include <OSPATH.h>
#include <OROCK.h>

#include <file_io_visitor.h>
#include <visitor_functions.h>
#include <OGFILE_DYNARRAYB.inl>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_color_remap_members(Visitor* v, ColorRemap* c)
{
	visit<int8_t>(v, &c->main_color);
	visit_array<int8_t>(v, c->color_table);
}

template <typename Visitor>
static void visit_game_members(Visitor* v, Game* c)
{
	visit<int8_t>(v, &c->init_flag);
	visit<int8_t>(v, &c->started_flag);
	visit<int8_t>(v, &c->game_mode);
	visit<int8_t>(v, &c->game_has_ended);
	visit_array(v, c->color_remap_array, visit_color_remap_members<Visitor>);
}

enum { GAME_RECORD_SIZE = 2060 };

//-------- Start of function Game::write_file -------------//
//
int Game::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_game_members<FileWriterVisitor>, GAME_RECORD_SIZE);
}
//--------- End of function Game::write_file ---------------//

//-------- Start of function Game::read_file -------------//
//
int Game::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_game_members<FileReaderVisitor>, GAME_RECORD_SIZE);
}
//--------- End of function Game::read_file ---------------//


//***//


template <typename Visitor>
static void visit_config(Visitor *v, Config *cfg, bool includeSystemSettings)
{
	visit<int16_t>(v, &cfg->difficulty_rating);
	visit<int8_t>(v, &cfg->ai_nation_count);
	visit<int16_t>(v, &cfg->start_up_cash);
	visit<int16_t>(v, &cfg->ai_start_up_cash);
	visit<int8_t>(v, &cfg->ai_aggressiveness);
	visit<int16_t>(v, &cfg->start_up_independent_town);
	visit<int16_t>(v, &cfg->start_up_raw_site);
	visit<int8_t>(v, &cfg->difficulty_level);
	visit<int8_t>(v, &cfg->explore_whole_map);
	visit<int8_t>(v, &cfg->fog_of_war);
	visit<int16_t>(v, &cfg->terrain_set);
	visit<int16_t>(v, &cfg->latitude);
	visit<int8_t>(v, &cfg->weather_effect);
	visit<int8_t>(v, &cfg->land_mass);
	visit<int8_t>(v, &cfg->new_independent_town_emerge);
	visit<int8_t>(v, &cfg->independent_town_resistance);
	visit<int8_t>(v, &cfg->random_event_frequency);
	visit<int8_t>(v, &cfg->new_nation_emerge);
	visit<int8_t>(v, &cfg->monster_type);
	visit<int8_t>(v, &cfg->start_up_has_mine_nearby);
	visit<int8_t>(v, &cfg->random_start_up);
	visit<int8_t>(v, &cfg->goal_destroy_monster);
	visit<int8_t>(v, &cfg->goal_population_flag);
	visit<int8_t>(v, &cfg->goal_economic_score_flag);
	visit<int8_t>(v, &cfg->goal_total_score_flag);
	visit<int8_t>(v, &cfg->goal_year_limit_flag);
	visit<int32_t>(v, &cfg->goal_population);
	visit<int32_t>(v, &cfg->goal_economic_score);
	visit<int32_t>(v, &cfg->goal_total_score);
	visit<int32_t>(v, &cfg->goal_year_limit);
	visit<int8_t>(v, &cfg->fire_spread_rate);
	visit<int8_t>(v, &cfg->wind_spread_fire_rate);
	visit<int8_t>(v, &cfg->fire_fade_rate);
	visit<int8_t>(v, &cfg->fire_restore_prob);
	visit<int8_t>(v, &cfg->rain_reduce_fire_rate);
	visit<int8_t>(v, &cfg->fire_damage);
	visit<int8_t>(v, &cfg->show_ai_info);
	visit<int8_t>(v, &cfg->fast_build);
	visit<int8_t>(v, &cfg->disable_ai_flag);
	visit<int8_t>(v, &cfg->king_undie_flag);
	visit<int8_t>(v, &cfg->race_id);
	visit_array<int8_t>(v, cfg->player_name);
	visit<int8_t>(v, &cfg->player_nation_color);
	visit<int8_t>(v, &cfg->expired_flag);
	visit<int8_t>(v, &cfg->opaque_report);
	visit<int8_t>(v, &cfg->disp_news_flag);
	visit<int16_t>(v, &cfg->scroll_speed);
	visit<int16_t>(v, &cfg->frame_speed);
	if (includeSystemSettings)
	{
		visit<int8_t>(v, &cfg->help_mode);
	}
	else {
		v->skip(1);
	}
	visit<int8_t>(v, &cfg->disp_town_name);
	visit<int8_t>(v, &cfg->disp_spy_sign);
	visit<int8_t>(v, &cfg->show_all_unit_icon);
	visit<int8_t>(v, &cfg->show_unit_path);
	if (includeSystemSettings)
	{
		visit<int8_t>(v, &cfg->music_flag);
		visit<int16_t>(v, &cfg->cd_music_volume);
		visit<int16_t>(v, &cfg->wav_music_volume);
		visit<int8_t>(v, &cfg->sound_effect_flag);
		visit<int16_t>(v, &cfg->sound_effect_volume);
	}
	else {
		v->skip(8);
	}
	visit<int8_t>(v, &cfg->pan_control);
	visit<int8_t>(v, &cfg->lightning_visual);
	visit<int8_t>(v, &cfg->earthquake_visual);
	visit<int8_t>(v, &cfg->rain_visual);
	visit<int8_t>(v, &cfg->snow_visual);
	visit<int8_t>(v, &cfg->snow_ground);
	visit<int8_t>(v, &cfg->lightning_audio);
	visit<int8_t>(v, &cfg->earthquake_audio);
	visit<int8_t>(v, &cfg->rain_audio);
	visit<int8_t>(v, &cfg->snow_audio);
	visit<int8_t>(v, &cfg->wind_audio);
	visit<int32_t>(v, &cfg->lightning_brightness);
	visit<int32_t>(v, &cfg->cloud_darkness);
	visit<int32_t>(v, &cfg->lightning_volume);
	visit<int32_t>(v, &cfg->earthquake_volume);
	visit<int32_t>(v, &cfg->rain_volume);
	visit<int32_t>(v, &cfg->snow_volume);
	visit<int32_t>(v, &cfg->wind_volume);
	visit<int8_t>(v, &cfg->blacken_map);
	visit<int8_t>(v, &cfg->explore_mask_method);
	visit<int8_t>(v, &cfg->fog_mask_method);
}

enum { CONFIG_RECORD_SIZE = 144 };

//-------- Start of function Config::write_file -------------//
//
int Config::write_file(File* filePtr, bool includeSysSettings)
{
	FileWriterVisitor v(filePtr);
	v.with_record_size(CONFIG_RECORD_SIZE);
	visit_config(&v, this, includeSysSettings);
	return v.good();
}
//--------- End of function Config::write_file ---------------//

//-------- Start of function Config::read_file -------------//
//
int Config::read_file(File* filePtr, bool includeSysSettings)
{
	FileReaderVisitor v(filePtr);
	v.with_record_size(CONFIG_RECORD_SIZE);
	visit_config(&v, this, includeSysSettings);
	return v.good();
}
//--------- End of function Config::read_file ---------------//


//***//


template <typename Visitor>
static void visit_chat_info_members(Visitor* v, ChatInfo* c)
{
	visit<int32_t>(v, &c->received_date);
	visit<int8_t>(v, &c->from_nation_recno);
	visit_array<int8_t>(v, c->chat_str);
}

template <typename Visitor>
static void visit_info_members(Visitor* v, Info* c)
{
	visit<int32_t>(v, &c->game_start_date);
	visit<int32_t>(v, &c->game_date);
	visit<int32_t>(v, &c->game_day);
	visit<int32_t>(v, &c->game_month);
	visit<int32_t>(v, &c->game_year);
	visit<int32_t>(v, &c->goal_deadline);
	visit<int16_t>(v, &c->goal_difficulty);
	visit<int16_t>(v, &c->goal_score_bonus);
	visit<int32_t>(v, &c->week_day);
	visit<int32_t>(v, &c->year_day);
	visit<int32_t>(v, &c->year_passed);
	visit<int32_t>(v, &c->random_seed);
	visit<uint32_t>(v, &c->start_play_time);
	visit<uint32_t>(v, &c->total_play_time);
	visit<int16_t>(v, &c->viewing_nation_recno);
	visit<int16_t>(v, &c->viewing_spy_recno);
	visit<int16_t>(v, &c->default_viewing_nation_recno);
	visit<int16_t>(v, &c->browse_nation_recno);
	visit<int16_t>(v, &c->browse_race_recno);
	visit<int16_t>(v, &c->browse_firm_recno);
	visit<int16_t>(v, &c->browse_income_recno);
	visit<int16_t>(v, &c->browse_expense_recno);
	visit<int16_t>(v, &c->browse_troop_recno);
	visit<int16_t>(v, &c->browse_unit_recno);
	visit<int16_t>(v, &c->browse_tech_recno);
	visit<int16_t>(v, &c->browse_god_recno);
	visit<int16_t>(v, &c->browse_town_recno);
	visit<int16_t>(v, &c->browse_spy_recno);
	visit<int16_t>(v, &c->browse_caravan_recno);
	visit<int16_t>(v, &c->browse_ship_recno);
	visit<int16_t>(v, &c->browse_talk_msg_recno);
	visit<int16_t>(v, &c->browse_news_recno);
	visit<int16_t>(v, &c->browse_ai_action_recno);
	visit<int16_t>(v, &c->browse_ai_attack_recno);
	visit<int8_t>(v, &c->nation_report_mode);
	visit<int16_t>(v, &c->last_talk_nation_recno);
	visit<int8_t>(v, &c->player_reply_mode);
	visit<int8_t>(v, &c->chat_receiver_type);
	visit_array<int8_t>(v, c->player_chat_str);
	visit_array(v, c->remote_chat_array, visit_chat_info_members<Visitor>);
}

enum {INFO_RECORD_SIZE = 1258 };

//-------- Start of function Info::write_file -------------//
//
int Info::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_info_members<FileWriterVisitor>, INFO_RECORD_SIZE);
}
//--------- End of function Info::write_file ---------------//


//-------- Start of function Info::read_file -------------//
//
int Info::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_info_members<FileReaderVisitor>, INFO_RECORD_SIZE);
}
//--------- End of function Info::read_file ---------------//


//***//


template <typename Visitor>
static void visit_power_members(Visitor* v, Power* c)
{
	visit<int32_t>(v, &c->command_id);
	visit<int32_t>(v, &c->command_unit_recno);
	visit<int32_t>(v, &c->command_para);
	visit<int8_t>(v, &c->win_opened);
	v->skip(1); // enable_flag
	visit_array<int32_t>(v, c->key_str_pos);
}

enum { POWER_RECORD_SIZE = 34 };

//-------- Start of function Power::write_file -------------//
//
int Power::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_power_members<FileWriterVisitor>, POWER_RECORD_SIZE);
}
//--------- End of function Power::write_file ---------------//


//-------- Start of function Power::read_file -------------//
//
int Power::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_power_members<FileReaderVisitor>, POWER_RECORD_SIZE);
}
//--------- End of function Power::read_file ---------------//


//***//


template <typename Visitor>
static void visit_sys_members(Visitor* v, Sys* c)
{
	visit<int32_t>(v, &c->loaded_random_seed);
	visit<int32_t>(v, &c->day_frame_count);
	visit<int32_t>(v, &c->frame_count);
	visit<int16_t>(v, &c->view_mode);
}


//-------- Start of function Sys::write_file -------------//
//
int Sys::write_file(File* filePtr)
{
	loaded_random_seed = misc.get_random_seed();

	FileWriterVisitor v(filePtr);
	visit_sys_members(&v, this);
	return v.good();
}
//--------- End of function Sys::write_file ---------------//


//-------- Start of function Sys::read_file -------------//
//
int Sys::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_sys_members(&v, this);
	return v.good();
}
//--------- End of function Sys::read_file ---------------//


//***//


enum { WEATHER_RECORD_SIZE = 35 };

template <typename Visitor>
static void visit_weather_members(Visitor* v, Weather* c)
{
	visit<uint32_t>(v, &c->seed);
	visit<int16_t>(v, &c->season_phase);
	visit<int16_t>(v, &c->day_to_quake);
	visit<int16_t>(v, &c->avg_temp);
	visit<int16_t>(v, &c->temp_amp);
	visit<int16_t>(v, &c->wind_spd);
	visit<int32_t>(v, &c->high_wind_day);
	visit<int16_t>(v, &c->wind_dir);
	visit<int16_t>(v, &c->windy_speed);
	visit<int16_t>(v, &c->tornado_count);
	visit<int8_t>(v, &c->cur_cloud_str);
	visit<int8_t>(v, &c->cur_cloud_len);
	visit<int8_t>(v, &c->cur_cloud_type);
	visit<int32_t>(v, &c->quake_frequency);
	visit<int16_t>(v, &c->quake_x);
	visit<int16_t>(v, &c->quake_y);
}

//-------- Start of function Weather::write_file -------------//
//
int Weather::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_weather_members<FileWriterVisitor>, WEATHER_RECORD_SIZE);
}
//--------- End of function Weather::write_file ---------------//


//-------- Start of function Weather::read_file -------------//
//
int Weather::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_weather_members<FileReaderVisitor>, WEATHER_RECORD_SIZE);
}
//--------- End of function Weather::read_file ---------------//


//***//


enum { MAGIC_WEATHER_RECORD_SIZE = 11 };

template <typename Visitor>
static void visit_magic_weather_members(Visitor* v, MagicWeather* c)
{
	visit<int8_t>(v, &c->rain_str);
	visit<int16_t>(v, &c->wind_spd);
	visit<int16_t>(v, &c->wind_dir);
	visit<int16_t>(v, &c->rain_day);
	visit<int16_t>(v, &c->wind_day);
	visit<int16_t>(v, &c->lightning_day);
}

//-------- Start of function MagicWeather::write_file -------------//
//
int MagicWeather::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_magic_weather_members<FileWriterVisitor>, MAGIC_WEATHER_RECORD_SIZE);
}
//--------- End of function MagicWeahter::write_file ---------------//


//-------- Start of function MagicWeahter::read_file -------------//
//
int MagicWeather::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_magic_weather_members<FileReaderVisitor>, MAGIC_WEATHER_RECORD_SIZE);
}
//--------- End of function MagicWeahter::read_file ---------------//


//***//


template <typename Visitor>
static void visit_location_members(Visitor* v, Location* c)
{
	visit<uint16_t>(v, &c->loc_flag);
	visit<int16_t>(v, &c->terrain_id);
	visit<int16_t>(v, &c->cargo_recno);
	visit<int16_t>(v, &c->air_cargo_recno);
	visit<uint8_t>(v, &c->extra_para);
	visit<int8_t>(v, &c->fire_level);
	visit<int8_t>(v, &c->flammability);
	visit<int8_t>(v, &c->power_nation_recno);
	visit<uint8_t>(v, &c->region_id);
	visit<uint8_t>(v, &c->visit_level);
}

template <typename Visitor>
static void visit_world_members(Visitor* v, World* c)
{
	//--------- save map -------------//

	enum { LOCATION_RECORD_SIZE = 14 };

	const int mapSize = World::max_x_loc * World::max_y_loc;
	if (is_reader_visitor(v))
	{
		c->loc_matrix = (Location*) mem_resize(c->loc_matrix, mapSize * sizeof(Location));
	}
	v->with_record_size(mapSize * LOCATION_RECORD_SIZE);
	for (int i = 0; i < mapSize; ++i)
	{
		visit_location_members(v, &c->loc_matrix[i]);
	}
	if (is_reader_visitor(v))
	{
		c->assign_map();
	}

	//--------- save vars -----------//

	visit<int16_t>(v, &c->scan_fire_x);
	visit<int16_t>(v, &c->scan_fire_y);
	visit<int16_t>(v, &c->lightning_signal);
	visit<int32_t>(v, &c->plant_count);
	visit<int32_t>(v, &c->plant_limit);
	visit<int16_t>(v, &c->map_matrix->map_mode);
	visit<int16_t>(v, &c->map_matrix->power_mode);
	visit<int32_t>(v, &c->map_matrix->cur_x_loc);
	visit<int32_t>(v, &c->map_matrix->cur_y_loc);
	visit<int32_t>(v, &c->zoom_matrix->init_lightning);
	visit<int32_t>(v, &c->zoom_matrix->vibration);
	visit<int16_t>(v, &c->zoom_matrix->lightning_x1);
	visit<int16_t>(v, &c->zoom_matrix->lightning_y1);
	visit<int16_t>(v, &c->zoom_matrix->lightning_x2);
	visit<int16_t>(v, &c->zoom_matrix->lightning_y2);
}

//-------- Start of function World::write_file -------------//
//
int World::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_world_members(&v, this);
	return v.good();
}
//--------- End of function World::write_file ---------------//


//-------- Start of function World::read_file -------------//
//
int World::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_world_members(&v, this);
	map_matrix->last_map_mode = -1;
	zoom_matrix->top_x_loc = map_matrix->cur_x_loc;
	zoom_matrix->top_y_loc = map_matrix->cur_y_loc;
	sys.zoom_need_redraw = 1;
	return v.good();
}
//--------- End of function World::read_file ---------------//


//***//


template <typename Visitor>
static void visit_tutor_members(Visitor* v, Tutor* c)
{
	visit_property<short, int16_t>(v, [c] () {return c->cur_tutor_id;},
		[c] (short tutorId) {
			if (tutorId > 0) {
				c->load(tutorId);		// load() will reset cur_text_block_id
			}
			else {
				c->cur_tutor_id = 0;
			}
		});
	visit<int16_t>(v, &c->cur_text_block_id);
}

//-------- Start of function Tutor::write_file -------------//
//
int Tutor::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_tutor_members(&v, this);
	return v.good();
}
//--------- End of function Tutor::write_file ---------------//


//-------- Start of function Tutor::read_file -------------//
//
int Tutor::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_tutor_members(&v, this);
	return v.good();
}
//--------- End of function Tutor::read_file ---------------//


//***//


template <typename Visitor>
static void visit_seek_path_members(Visitor* v, SeekPath* c)
{
	visit<int16_t>(v, &c->total_node_avail);
}

//### begin alex 23/9 ###//
//-------- Start of function SeekPath::write_file -------------//
//
int SeekPath::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_seek_path_members(&v, this);
	return v.good();
}
//--------- End of function SeekPath::write_file ---------------//


//-------- Start of function SeekPath::read_file -------------//
//
int SeekPath::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_seek_path_members(&v, this);
	return v.good();
}
//--------- End of function SeekPath::read_file ---------------//
//#### end alex 23/9 ####//


//***//


template <typename Visitor>
static void visit_rock_members(Visitor* v, Rock* c)
{
	visit<int16_t>(v, &c->rock_recno);
	visit<int8_t>(v, &c->cur_frame);
	visit<int8_t>(v, &c->delay_remain);
	visit<int16_t>(v, &c->loc_x);
	visit<int16_t>(v, &c->loc_y);
	visit<uint32_t>(v, &c->seed);
}

enum { ROCK_RECORD_SIZE = 12 };

int RockArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_rock_members<FileWriterVisitor>, ROCK_RECORD_SIZE);
	return v.good();
}

int RockArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_rock_members<FileReaderVisitor>, ROCK_RECORD_SIZE);
	return v.good();
}
