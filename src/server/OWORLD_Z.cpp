/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

//Filename    : OWORLD_Z.CPP
//Description : Object ZoomMatrix

#include <math.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OPOWER.h>
#include <OSTR.h>
#include <OSITE.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OGAME.h>
#include <OUNIT.h>
#include <ONATION.h>
#include <OSPRITE.h>
#include <OBULLET.h>
#include <OPLANT.h>
#include <OTERRAIN.h>
#include <OWALLRES.h>
#include <OLIGHTN.h>
#include <ORAIN.h>
#include <OSNOW.h>
#include <OWORLD.h>
#include <OWEATHER.h>
#include <OFLAME.h>
#include <OGODRES.h>
#include <OU_GOD.h>
#include <OAUDIO.h>
#include <OHILLRES.h>
#include <OTORNADO.h>
#include <OSNOWG.h>
#include <OSNOWRES.h>
#include <OEXPMASK.h>
#include <OCOLTBL.h>
#include <OROCKRES.h>
#include <OROCK.h>
#include <OEFFECT.h>
#include <COLCODE.h>
#include <OANLINE.h>
#include <OFIRMDIE.h>
#include <OIMGRES.h>

//--------- Define static vars -----------//

//static int 			init_rain = 0;					// reset on new game and load game
static Rain 		rain;
//static int			rain_channel_id = 0;			// reset on new game and load game
//static int			wind_channel_id = 0;			// reset on new game and load game
//static int			fire_channel_id = 0;			// reset on new game and load game
//static int			last_fire_vol = 0;			// reset on new game and load game
//static int			init_lightning = 0;			// reset on new game, save on save game
static YLightning lightning;
//static int			init_snow = 0;					// reset on new game and load game
static Snow			snow;
//static short		last_brightness = 0;			// reset on new game and load game
//static int			vibration = -1;				// reset on new game, save on save game
//static short		lightning_x1, lightning_y1, lightning_x2, lightning_y2; // save on save game
static int init_fire = -10;						// reset on new game and load game

//-------- Declare static functions ---------//

static int sort_display_function( const void *a, const void *b );


//------- Define constant for object_type --------//

enum { OBJECT_UNIT,
		 OBJECT_POINTED_UNIT,
		 OBJECT_BULLET,
		 OBJECT_FIRM,
		 OBJECT_TOWN,
		 OBJECT_PLANT,
		 OBJECT_FIRE,
		 OBJECT_WALL,
		 OBJECT_TORNADO,
		 OBJECT_HILL,
		 OBJECT_ROCK,
		 OBJECT_EFFECT,
		 // ###### begin Gilbert 2/10 #######//
		 OBJECT_FIRM_DIE,
		 // ###### end Gilbert 2/10 #######//
	  };

enum { LAND_DISP_LAYER_MASK=1,
       LAND_TOP_DISP_LAYER_MASK=2,
		 LAND_BOTTOM_DISP_LAYER_MASK=4,
		 AIR_DISP_LAYER_MASK=8,
     };

//---------- Define struct DisplaySort ----------//

struct DisplaySort
{
	char	object_type;
	short object_recno;
	short object_y2;
	short x_loc, y_loc;
};


//------------ begin of static function draw_unit_path_on_zoom_map -----------//
// ##### begin Gilbert 9/10 #######//
static void draw_unit_path_on_zoom_map(int displayLayer)
// ##### end Gilbert 9/10 #######//
{
	// ###### begin Gilbert 29/8 ######//
	if( !(config.show_unit_path & 1) )
		return;
	// ###### end Gilbert 29/8 ######//

	short nationRecno = nation_array.player_recno;

	Unit *unitPtr;
	int i, j, resultNodeCount, resultNodeRecno;
	short lineFromX, lineFromY, lineToX, lineToY;
	ResultNode *resultNode1, *resultNode2;

	for(i=unit_array.size(); i>0; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];
		if(!unitPtr->is_visible())
			continue;

		if(!unitPtr->selected_flag)
			continue;

		// ####### begin Gilbert 11/9 #######//
		if( !config.show_ai_info && nationRecno && !unitPtr->is_nation(nationRecno) )
			continue;
		// ####### end Gilbert 11/9 #######//

		// ##### begin Gilbert 9/10 #######//
		if( unitPtr->mobile_type == UNIT_LAND || unitPtr->mobile_type == UNIT_SEA )
		{
			if( !(displayLayer & LAND_DISP_LAYER_MASK) )
				continue;
		}
		else if( unitPtr->mobile_type == UNIT_AIR )
		{
			if( !(displayLayer & AIR_DISP_LAYER_MASK) )
				continue;
		}
		else
		{
			err_here();
			continue;
		}
		// ##### end Gilbert 9/10 #######//

		//--------------- draw unit's path ----------------//

		resultNodeRecno = unitPtr->result_node_recno;
		resultNodeCount = unitPtr->result_node_count;
		if(!resultNodeCount || resultNodeRecno>resultNodeCount)
			continue;

		//-----------------------------------------------------------//
		if(unitPtr->cur_x!=unitPtr->go_x || unitPtr->cur_y!=unitPtr->go_y)
		{
			lineFromX = unitPtr->go_x - world.zoom_matrix->top_x_loc*ZOOM_LOC_WIDTH + ZOOM_X1 + ZOOM_LOC_WIDTH/2;
			lineFromY = unitPtr->go_y - world.zoom_matrix->top_y_loc*ZOOM_LOC_HEIGHT + ZOOM_Y1 + ZOOM_LOC_HEIGHT/2;
			lineToX = unitPtr->cur_x - world.zoom_matrix->top_x_loc*ZOOM_LOC_WIDTH + ZOOM_X1 + ZOOM_LOC_WIDTH/2;
			lineToY = unitPtr->cur_y - world.zoom_matrix->top_y_loc*ZOOM_LOC_HEIGHT + ZOOM_Y1 + ZOOM_LOC_HEIGHT/2;
			anim_line.draw_line(&vga_back, lineFromX, lineFromY, lineToX, lineToY);
		}

		//-----------------------------------------------------------//
		err_when(resultNodeRecno<1);
		resultNode1 = unitPtr->result_node_array + resultNodeRecno - 1;
		resultNode2 = resultNode1 + 1;
		lineToX = (resultNode1->node_x - world.zoom_matrix->top_x_loc)*ZOOM_LOC_WIDTH + ZOOM_X1 + ZOOM_LOC_WIDTH/2;
		lineToY = (resultNode1->node_y - world.zoom_matrix->top_y_loc)*ZOOM_LOC_HEIGHT	+ ZOOM_Y1 + ZOOM_LOC_HEIGHT/2;
		for(j=resultNodeRecno+1; j<=resultNodeCount; j++, resultNode1++, resultNode2++)
		{
			lineFromX = (resultNode2->node_x - world.zoom_matrix->top_x_loc)*ZOOM_LOC_WIDTH + ZOOM_X1 + ZOOM_LOC_WIDTH/2;
			lineFromY = (resultNode2->node_y - world.zoom_matrix->top_y_loc)*ZOOM_LOC_HEIGHT + ZOOM_Y1 + ZOOM_LOC_HEIGHT/2;
			anim_line.draw_line(&vga_back, lineFromX, lineFromY, lineToX, lineToY);
			lineToX = lineFromX;
			lineToY = lineFromY;
		}
	}
}
//------------ end of static function draw_unit_path_on_zoom_map -----------//


//------------ begin of static function draw_unit_way_point_on_zoom_map -----------//
static void draw_unit_way_point_on_zoom_map()
{
	short nationRecno = nation_array.player_recno;

	Unit *unitPtr;
	int i, j, resultNodeCount;
	short lineFromX, lineFromY, lineToX, lineToY;
	ResultNode *resultNode1, *resultNode2;
	// ##### begin Gilbert 12/11 #######//
	char *chPtr = image_icon.get_ptr("WAYPOINT");
	short chOffsetX = - (*(short *)chPtr / 2);
	short chOffsetY = - (*(1+(short *)chPtr) / 2);
	// ##### end Gilbert 12/11 #######//

	for(i=unit_array.size(); i>0; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];
		if(!unitPtr->is_visible())
			continue;

		if(!unitPtr->selected_flag)
			continue;

		if( !config.show_ai_info && nationRecno && !unitPtr->is_nation(nationRecno) )
			continue;

		if(unitPtr->way_point_count)
		{
			resultNodeCount = unitPtr->way_point_count;
			resultNode1 = unitPtr->way_point_array;
			// ##### begin Gilbert 12/11 #######//
			// char *chPtr = image_icon.get_ptr("WAYPOINT");
			lineToX = (resultNode1->node_x - world.zoom_matrix->top_x_loc)*ZOOM_LOC_WIDTH + ZOOM_X1 + ZOOM_LOC_WIDTH/2;
			lineToY = (resultNode1->node_y - world.zoom_matrix->top_y_loc)*ZOOM_LOC_HEIGHT	+ ZOOM_Y1 + ZOOM_LOC_HEIGHT/2;
			world.zoom_matrix->put_bitmap_clip(lineToX+chOffsetX, lineToY+chOffsetY, chPtr);
			// ##### begin Gilbert 12/11 #######//

			if(resultNodeCount>1)
			{
				resultNode2 = resultNode1+1;
				for(j=1; j<resultNodeCount; j++, resultNode1++, resultNode2++)
				{
					lineFromX = (resultNode2->node_x - world.zoom_matrix->top_x_loc)*ZOOM_LOC_WIDTH + ZOOM_X1 + ZOOM_LOC_WIDTH/2;
					lineFromY = (resultNode2->node_y - world.zoom_matrix->top_y_loc)*ZOOM_LOC_HEIGHT + ZOOM_Y1 + ZOOM_LOC_HEIGHT/2;
					anim_line.draw_line(&vga_back, lineFromX, lineFromY, lineToX, lineToY, 0, 1);
					lineToX = lineFromX;
					lineToY = lineFromY;
					// ##### begin Gilbert 12/11 #######//
					world.zoom_matrix->put_bitmap_clip(lineToX+chOffsetX, lineToY+chOffsetY, chPtr);
					// ##### begin Gilbert 12/11 #######//
				}
			}
		}
	}
}
//------------ end of static function draw_unit_way_point_on_zoom_map -----------//


//-------- Begin of function ZoomMatrix::ZoomMatrix ----------//

ZoomMatrix::ZoomMatrix() : land_disp_sort_array(sizeof(DisplaySort),100),
									air_disp_sort_array(sizeof(DisplaySort),50),
									land_top_disp_sort_array(sizeof(DisplaySort), 40),
									land_bottom_disp_sort_array(sizeof(DisplaySort), 20)
{
	init( ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2,
			ZOOM_WIDTH, ZOOM_HEIGHT,
			ZOOM_LOC_WIDTH, ZOOM_LOC_HEIGHT, 0 );		// 0-don't create a background buffer
}
//---------- End of function ZoomMatrix::ZoomMatrix ----------//


//---------- Begin of function ZoomMatrix::init_para ------------//
void ZoomMatrix::init_para()
{
	init_rain = 0;
	// #### begin Gilbert 7/10 ######//
	rain.clear();
	rain.stop_rain();
	// #### end Gilbert 7/10 ######//
	rain_channel_id = 0;
	wind_channel_id = 0;
	fire_channel_id = 0;
	last_fire_vol = 0;
	init_lightning = 0;
	init_snow = 0;
	last_brightness = 0;
	vibration = -1;
}
//---------- End of function ZoomMatrix::init_para ----------//


//---------- Begin of function ZoomMatrix::draw ------------//
//
// Draw world map
//
void ZoomMatrix::draw()
{
	int       i=0, x, y, xLoc, yLoc, dispPower;
	Location* locPtr;
	char*     nationColorArray = nation_array.nation_power_color_array;

	int maxXLoc = top_x_loc + disp_x_loc;        // divide by 2 for world_info
	int maxYLoc = top_y_loc + disp_y_loc;

	dispPower = (world.map_matrix->map_mode == MAP_MODE_POWER &&
					 world.map_matrix->power_mode ) ||
					power.command_id == COMMAND_BUILD_FIRM ||
					power.command_id == COMMAND_SETTLE;

	sys.yield();

	//----------------------------------------------------//

	int nationRecno, borderColor;

	for( y=image_y1,yLoc=top_y_loc ; yLoc<maxYLoc ; yLoc++, y+=loc_height )
	{
		locPtr = get_loc(top_x_loc,yLoc);

		long snowSeed = (snow_ground_array.snow_pattern << 16) + (yLoc << 8);

		for( x=image_x1,xLoc=top_x_loc ; xLoc<maxXLoc ; xLoc++, x+=loc_width, locPtr++ )
		{
			if( locPtr->explored() )		// only draw if the location has been explored
			{
				//---------- draw terrain bitmap -----------//

				vga_back.put_bitmap_32x32( x, y, terrain_res[locPtr->terrain_id]->bitmap_ptr );
				char *overlayBitmap = terrain_res[locPtr->terrain_id]->get_bitmap(sys.frame_count /4);
				if( overlayBitmap)
					vga_back.put_bitmap_trans_decompress( x, y, overlayBitmap);

				#ifdef DEBUG
				if(debug2_enable_flag)
				{
					if(locPtr->is_coast())
					{
						VgaBuf *activeBufBackup = Vga::active_buf;
						Vga::active_buf = &vga_back;
						font_std.put( x+24, y+20, terrain_res[locPtr->terrain_id]->average_type);
						Vga::active_buf = activeBufBackup;
					}
				}
				#endif

				// --------- draw dirt block --------//
				if( locPtr->has_dirt() )
				{
					dirt_array[locPtr->dirt_recno()]->draw_block(xLoc,yLoc);
				}

				if(terrain_res[locPtr->terrain_id]->can_snow() )
				{
					if( config.snow_ground==1 && snow_ground_array.snow_thick > 0)
					{
						vga_back.snow_32x32(x,y, snowSeed+xLoc, 0xffff - snow_ground_array.snow_thick);
					}

					if( config.snow_ground==2)
					{
						int snowMapId = snow_ground_array.has_snow(xLoc,yLoc);
						if( snowMapId )
						{
							snow_res[snowMapId]->draw_at(xLoc*ZOOM_LOC_WIDTH+ZOOM_LOC_WIDTH/2, yLoc*ZOOM_LOC_HEIGHT+ZOOM_LOC_HEIGHT/2);
						}
					}
				}

				// --------- draw hill square --------//
				if( locPtr->has_hill() )
				{
					if( locPtr->hill_id2())
						hill_res[locPtr->hill_id2()]->draw(xLoc,yLoc,1);
					hill_res[locPtr->hill_id1()]->draw(xLoc, yLoc,1);
				}

				//---------- if in power map mode -----------//

				if( dispPower && (nationRecno=locPtr->power_nation_recno) > 0 )
				{
					vga_back.pixelize_32x32( x, y, nationColorArray[nationRecno] );

					borderColor = nationColorArray[nationRecno] + 1;

					if( yLoc==0 || get_loc(xLoc, yLoc-1)->power_nation_recno!=nationRecno )
						vga_back.bar( x, y, x+31, y, borderColor );

					if( yLoc==MAX_WORLD_Y_LOC-1 || get_loc(xLoc, yLoc+1)->power_nation_recno!=nationRecno )
						vga_back.bar( x, y+31, x+31, y+31, borderColor );

					if( xLoc==0 || get_loc(xLoc-1, yLoc)->power_nation_recno!=nationRecno )
						vga_back.bar( x, y, x, y+31, borderColor );

					if( xLoc==MAX_WORLD_X_LOC-1 || get_loc(xLoc+1, yLoc)->power_nation_recno!=nationRecno )
						vga_back.bar( x+31, y, x+31, y+31, borderColor );
				}

				//--------- draw raw material icon ---------//

				if( locPtr->has_site() && locPtr->walkable(3) )		// don't display if a building/object has already been built on the location
					site_array[locPtr->site_recno()]->draw(x, y);

				//----- draw grids, for debugging only -----//

				#ifdef DEBUG
					if(debug2_enable_flag)
					{
						vga_back.bar( x, y, x+31, y, V_WHITE );
						vga_back.bar( x, y, x, y+31, V_WHITE );

						// display x, y location
						if(!(xLoc%5) && !(yLoc%5))
						{
							VgaBuf *activeBufBackup = Vga::active_buf;
							Vga::active_buf = &vga_back;
							font_std.put( x+4, y+3, xLoc );
							font_std.put( x+4, y+15, yLoc );
							Vga::active_buf = activeBufBackup;
						}
					}
				#endif
			}
		}
	}

   sys.yield();

	//---------------------------------------------------//

	if( save_image_buf )
	{
		vga_back.read_bitmap( image_x1, image_y1, image_x2, image_y2, save_image_buf );
		just_drawn_flag = 1;
	}
}
//------------ End of function ZoomMatrix::draw ------------//


//---------- Begin of function ZoomMatrix::draw_white_site ------------//
//
void ZoomMatrix::draw_white_site()
{
	int       i=0, x, y, xLoc, yLoc;
	Location* locPtr;

	int maxXLoc = top_x_loc + disp_x_loc;        // divide by 2 for world_info
	int maxYLoc = top_y_loc + disp_y_loc;

	//------- draw occupied locations in whie ---------//

	for( y=image_y1,yLoc=top_y_loc ; yLoc<maxYLoc ; yLoc++, y+=loc_height )
	{
		locPtr = get_loc(top_x_loc,yLoc);

		for( x=image_x1,xLoc=top_x_loc ; xLoc<maxXLoc ; xLoc++, x+=loc_width, locPtr++ )
		{
			if(locPtr->has_unit(UNIT_LAND) || locPtr->has_unit(UNIT_SEA) || locPtr->has_unit(UNIT_AIR))
				vga_back.bar( x, y, x+31, y+31, V_WHITE );
		}
	}
}
//------------ End of function ZoomMatrix::draw_white_site ------------//


//---------- Begin of function ZoomMatrix::draw_frame -----------//
//
void ZoomMatrix::draw_frame()
{
	draw_objects();

	draw_weather_effects();

	draw_build_marker();

	if(config.blacken_map && config.fog_of_war)
		blacken_fog_of_war();

	else if( !config.explore_whole_map )
		blacken_unexplored();

	disp_text();
}
//----------- End of function ZoomMatrix::draw_frame ------------//


//---------- Begin of function ZoomMatrix::draw_weather_effects -----------//
//
void ZoomMatrix::draw_weather_effects()
{
	//---------- Earthquake -----------//

	if( weather.is_quake() )
	{
		if(vibration == -1)
		{
			// start of an earthquake
			vibration = weather.quake_rate(top_x_loc+disp_x_loc/2, top_y_loc+disp_y_loc/2)*16/100;
			if( config.sound_effect_flag && config.earthquake_audio)
			{
				RelVolume r(config.earthquake_volume,0);
				audio.play_long_wav( DIR_SOUND"QUAKE.WAV", DsVolume(r) );
			}
		}
		int vPitch = vga_back.buf_pitch();
		char *destBitmap = vga_back.buf_ptr() + ZOOM_Y1 * vPitch + ZOOM_X1;
		char *srcBitmap = destBitmap + vibration * vPitch;
		int lineCount = ZOOM_HEIGHT - vibration;

		// shift back buffer up

		if( vibration )
		{
			if( config.earthquake_visual)
			{
				for(int lineRun = 0; lineRun < lineCount; ++lineRun)
				{
					memcpy(destBitmap, srcBitmap, ZOOM_WIDTH);
					destBitmap += vPitch;
					srcBitmap  += vPitch;
				}
			}
			vibration = 0;
		}
		else
		{
			vibration = weather.quake_rate(top_x_loc+disp_x_loc/2, top_y_loc+disp_y_loc/2)*16/100;
		}
	}
	else
	{
		vibration = -1;
	}

	
	//---------- raining --------//

	short newRainScale = weather.rain_scale();

	if( newRainScale != init_rain )
	{
		// BUGHERE : did not handle, wind change (direction/speed) during a rain
		if( newRainScale)
		{
			rain.start_rain(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2, newRainScale,
				weather.wind_speed()*sin(weather.wind_direct_rad())/100.0);

			// turn on rain noise

			int relVolume = config.rain_volume + newRainScale;
			if( relVolume > 100)
				relVolume = 100;

			if( rain_channel_id == 0)	// from no rain to rain
			{
				if( config.sound_effect_flag && config.rain_audio)
				{
					RelVolume r(relVolume,0);
					rain_channel_id = audio.play_loop_wav(DIR_SOUND"RAIN.WAV",11008*2, DsVolume(r));
				}
			}
			else
			{
				// changing rain
				if( config.sound_effect_flag && config.rain_audio)
				{
					RelVolume r(relVolume,0);
					audio.volume_loop_wav(rain_channel_id, DsVolume(r));
				}
				else
				{
					// can't stop rain audio immediately
					// but at least stop it when rain change
					audio.stop_loop_wav(rain_channel_id);
					rain_channel_id = 0;
				}

			}

		}
		else
		{
			// rain stop, rain sound fade out
			rain.stop_rain();
			if( rain_channel_id )
			{
				audio.fade_out_loop_wav(rain_channel_id, 10);
			}
		}
		init_rain = newRainScale;
	}
	else
	{
		// rain stopped, check rain sound fade out
		if( newRainScale == 0 && rain_channel_id )
		{
			DsVolume dsVolume(audio.get_loop_wav_volume(rain_channel_id));
			AbsVolume absVolume(dsVolume);
			if( absVolume.abs_vol < 10 )
			{
				audio.stop_loop_wav(rain_channel_id);
				rain_channel_id = 0;
			}
		}
	}

	// ##### begin Gilbert 6/9 #######//
	if( config.frame_speed > 0)
	{
		rain.new_drops();
		if( config.rain_visual)
		{
			rain.draw_step(&vga_back);
		}
	}
	// ##### end Gilbert 6/9 #######//


	//---------- Lightning -----------//
	// world.lightning_signal == 0 (no lightning)
	// world.lightning_signal == 110, 109 (ready lightning)
	//                108, 107, 106, 105 (flashing), Sound effect start on 6
	//                104, 103, 102, 101 (rest, decaying lighting effect )
	//                100					 (decrease randomly)
	//                99 - 1             (rest states)
	// see world.process

	unsigned long mRandom = m.get_random_seed();
	if( world.lightning_signal >= 105 && world.lightning_signal <= 108)
	{
		if( !init_lightning )
		{
			// play sound
			if( world.lightning_signal == 108 && config.sound_effect_flag && config.lightning_audio)
			{
				RelVolume r(config.lightning_volume,0);
				audio.play_long_wav(DIR_SOUND"THUNDER.WAV", DsVolume(r));
			}

			// find the starting and ending point of the lightning
			lightning_x1 = Lightning::bound_x1 + 20 + short(mRandom % (Lightning::bound_x2-Lightning::bound_x1 - 40));
			lightning_y1 = Lightning::bound_y1 - 50;
			lightning_x2 = Lightning::bound_x1 + 10 + short(mRandom % (Lightning::bound_x2-Lightning::bound_x1 - 20));
			lightning_y2 = (Lightning::bound_y1+Lightning::bound_y2) / 2 +
				short(mRandom % ( (Lightning::bound_y2-Lightning::bound_y1) / 2));
			init_lightning = 1;
		}

		lightning.init( lightning_x1, lightning_y1, lightning_x2, lightning_y2, 8);
		if( config.lightning_visual)
			lightning.draw_section(&vga_back, (109-world.lightning_signal)/4.0);
	}
	else
	{
		init_lightning = 0;
	}

	//------------ snowing ------------//
	short snowScale = weather.snow_scale();
	if( snowScale > 0 && init_snow == 0)
	{
		long backupSeed = m.get_random_seed();

		// start of snow
		snow.set_bound(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2);
		snow.init(weather.wind_speed()*sin(weather.wind_direct_rad())/200.0,
			snowScale+2);
		if( config.sound_effect_flag && config.snow_audio )
		{
			// audio.play_wav("SNOW", config.snow_volume);
		}
	}
	// ###### begin Gilbert 6/9 #######//
	if( snowScale > 0 && config.snow_visual && config.frame_speed > 0)
		snow.draw_step(&vga_back);
	// ###### end Gilbert 6/9 #######//
	init_snow = snowScale;

	//------------ brightness, effect of lightning and cloud -------//
	short newBrightness;
	short maxBrightness = config.lightning_brightness;
	if( config.lightning_visual && init_lightning > 107 )
	{
		newBrightness = -maxBrightness;
	}
	else if( config.lightning_visual && init_lightning >= 104 && init_lightning <= 107)
	{
		newBrightness = maxBrightness;
	}
	else if( config.lightning_visual && init_lightning >= 101 && init_lightning <= 103)
	{
		newBrightness = (init_lightning-100) * maxBrightness / 4;
	}
	else
	{
		newBrightness = -weather.cloud() * config.cloud_darkness ;
	}
	if( newBrightness != last_brightness )
	{
		vga.adjust_brightness(newBrightness);
		last_brightness = newBrightness;
	}

	// ------------- wind sound ----------//
	int windSpeed = weather.wind_speed();
	if( windSpeed >= 20)
	{
		int relVolume = config.wind_volume + 5 + windSpeed/4;
		if( relVolume > 100)
			relVolume = 100;
		if( wind_channel_id == 0)
		{
			if( config.sound_effect_flag && config.wind_audio )
			{
				// ###### begin Gilbert 6/8 #######//
				// wind_channel_id = audio.play_loop_wav(DIR_SOUND"WIND.WAV",0, relVolume);
				RelVolume r(relVolume,0);
				wind_channel_id = audio.play_loop_wav(DIR_SOUND"WIND.WAV",25088*2, DsVolume(r));  // 25088 samples, 8-bit stereo, so *2
				// ###### end Gilbert 6/8 #######//
			}
		}
		else
		{
			if( config.wind_audio)
			{
				RelVolume r(relVolume,0);
				audio.volume_loop_wav(wind_channel_id, DsVolume(r));
			}
			else
			{
				audio.stop_loop_wav(wind_channel_id);
				wind_channel_id = 0;
			}
		}
	}
	else
	{
		if( wind_channel_id )
		{
			if( !audio.is_loop_wav_fading(wind_channel_id) )
			{
				audio.fade_out_loop_wav(wind_channel_id, 5);
			}
			else
			{
				DsVolume dsVolume = audio.get_loop_wav_volume(wind_channel_id);
				AbsVolume absVolume(dsVolume);
				if( absVolume.abs_vol < 5 )
				{
					audio.stop_loop_wav(wind_channel_id);
					wind_channel_id =0 ;
				}
			}
		}
	}

}
//----------- End of function ZoomMatrix::draw_weather_effects ------------//


//---------- Begin of function ZoomMatrix::draw_build_marker -----------//
//
void ZoomMatrix::draw_build_marker()
{
	if( !(mouse.cur_x >= ZOOM_X1 && mouse.cur_x <= ZOOM_X2 &&		// if the mouse is inside the zoom area
			mouse.cur_y >= ZOOM_Y1 && mouse.cur_y <= ZOOM_Y2) )
	{
		return;
	}

	// ##### begin Gilbert 24/10 #######//
	if( power.win_opened )
		return;
	// ##### end Gilbert 24/10 #######//

	//------- COMMAND_GOD_CAST_POWER --------//

	else if( power.command_id == COMMAND_GOD_CAST_POWER )
	{
		draw_god_cast_range();
		return;
	}

	//----------------------------------------------//

	int xLoc = (mouse.cur_x-ZOOM_X1)/ZOOM_LOC_WIDTH;
	int yLoc = (mouse.cur_y-ZOOM_Y1)/ZOOM_LOC_HEIGHT;
	int locWidth, locHeight, validAction;
	Location* locPtr = world.get_loc(top_x_loc+xLoc, top_y_loc+yLoc);

	//------- if it's in firm building mode now ----//

	if( power.command_id == COMMAND_BUILD_FIRM )
	{
		FirmInfo* firmInfo = firm_res[power.command_para];

		locWidth  = firmInfo->loc_width;
		locHeight = firmInfo->loc_height;

		validAction  = world.can_build_firm( top_x_loc+xLoc, top_y_loc+yLoc, power.command_para, unit_array.selected_recno );
	}

	//------- if it's in settling mode now ----//

	else if( power.command_id == COMMAND_SETTLE && unit_array.selected_recno )
	{
		// assign to an existing town

		Unit* selectedUnit = unit_array[ unit_array.selected_recno ];

		if( locPtr->is_town() && town_array[locPtr->town_recno()]->nation_recno == selectedUnit->nation_recno )
			return;		// don't draw the settling mask.

		locWidth  = STD_TOWN_LOC_WIDTH;
		locHeight = STD_TOWN_LOC_HEIGHT;

		validAction  = world.can_build_town( top_x_loc+xLoc, top_y_loc+yLoc, unit_array.selected_recno );
	}

	//------- COMMAND_BUILD_WALL --------//

	else if( power.command_id == COMMAND_BUILD_WALL )
	{
		// see also World::build_wall_tile
		locWidth  = 1;
		locHeight = 1;
		Location *locPtr;

		validAction = world.can_build_wall(top_x_loc+xLoc, top_y_loc+yLoc, nation_array.player_recno)
			|| ( (locPtr=get_loc(top_x_loc+xLoc, top_y_loc+yLoc))->is_wall() && locPtr->is_wall_destructing() &&
			world.can_destruct_wall(top_x_loc+xLoc, top_y_loc+yLoc, nation_array.player_recno) );
	}

	//------- COMMAND_DESTRUCT_WALL --------//

	else if( power.command_id == COMMAND_DESTRUCT_WALL )
	{
		locWidth  = 1;
		locHeight = 1;
		// see also World::destruct_wall_tile
		validAction = world.can_destruct_wall(top_x_loc+xLoc, top_y_loc+yLoc, nation_array.player_recno)
			&& get_loc(top_x_loc+xLoc, top_y_loc+yLoc)->is_wall_creating() ;
	}

	else
		return;

	//---------- draw an highlight area -----------//

	int x1 = ZOOM_X1 + xLoc * ZOOM_LOC_WIDTH;
	int y1 = ZOOM_Y1 + yLoc * ZOOM_LOC_HEIGHT;
	int x2 = ZOOM_X1 + (xLoc+locWidth)  * ZOOM_LOC_WIDTH -1;
	int y2 = ZOOM_Y1 + (yLoc+locHeight) * ZOOM_LOC_HEIGHT-1;

	int pixelColor;

	if( validAction )
#ifdef AMPLUS
		pixelColor = anim_line.get_series_color_array(-1)[2];
#else
		pixelColor = V_WHITE;
#endif
	else
		pixelColor = V_BLACK;

	vga_back.pixelize( x1, y1, MIN(x2,ZOOM_X2), MIN(y2,ZOOM_Y2), pixelColor );

	//------- draw lines connected to towns and firms ---------//

	if( validAction )
	{
		if( power.command_id==COMMAND_BUILD_FIRM )
		{
			world.draw_link_line( power.command_para, 0, top_x_loc+xLoc, top_y_loc+yLoc,
										 top_x_loc+xLoc+locWidth-1, top_y_loc+yLoc+locHeight-1 );
		}
		else if( power.command_id==COMMAND_SETTLE )
		{
			world.draw_link_line( 0, 0, top_x_loc+xLoc, top_y_loc+yLoc,
										 top_x_loc+xLoc+locWidth-1, top_y_loc+yLoc+locHeight-1 );
		}
	}
}
//----------- End of function ZoomMatrix::draw_build_marker ------------//


//---------- Begin of function ZoomMatrix::draw_god_cast_range -----------//
//
void ZoomMatrix::draw_god_cast_range()
{
	#define GOD_CAST_RANGE_COLOR 	V_WHITE

	int   	 xLoc, yLoc, centerY, t;
	int		 x1, y1, x2, y2;
	Location* locPtr;

	Unit* 	unitPtr = unit_array[power.command_unit_recno];
	GodInfo*	godInfo = god_res[ ((UnitGod*)unitPtr)->god_id ];

	xLoc = (mouse.cur_x-ZOOM_X1)/ZOOM_LOC_WIDTH;
	yLoc = (mouse.cur_y-ZOOM_Y1)/ZOOM_LOC_HEIGHT;

	int xLoc1 = xLoc - godInfo->cast_power_range + 1;
	int yLoc1 = yLoc - godInfo->cast_power_range + 1;
	int xLoc2 = xLoc + godInfo->cast_power_range - 1;
	int yLoc2 = yLoc + godInfo->cast_power_range - 1;

	centerY = (yLoc1+yLoc2) / 2;

	//----- pixelize the area within which the power can casted ----//

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		t=abs(yLoc-centerY)/2;

		for( xLoc=xLoc1+t ; xLoc<=xLoc2-t ; xLoc++, locPtr++ )
		{
			if( xLoc>=0 && xLoc<MAX_WORLD_X_LOC &&
				 yLoc>=0 && yLoc<MAX_WORLD_Y_LOC )
			{
				x1 = ZOOM_X1 + xLoc * ZOOM_LOC_WIDTH;
				y1 = ZOOM_Y1 + yLoc * ZOOM_LOC_HEIGHT;
				x2 = ZOOM_X1 + (xLoc+1) * ZOOM_LOC_WIDTH -1;
				y2 = ZOOM_Y1 + (yLoc+1) * ZOOM_LOC_HEIGHT-1;

				vga_back.pixelize( x1, y1, MIN(x2,ZOOM_X2), MIN(y2,ZOOM_Y2), GOD_CAST_RANGE_COLOR );
			}
		}
	}
}
//----------- End of function ZoomMatrix::draw_god_cast_range ------------//


//---------- Begin of function ZoomMatrix::disp_text -----------//
//
// Function for displaying town names and spy indicator.
//
void ZoomMatrix::disp_text()
{
	//------- towns -------//

	int y, dispSpy;

	if( config.disp_town_name || config.disp_spy_sign )
	{
		Town* townPtr;

		for( int i=town_array.size() ; i>0 ; i-- )
		{
			if( town_array.is_deleted(i) )
				continue;

			townPtr = town_array[i];

			if( !world.get_loc(townPtr->center_x, townPtr->center_y)->explored() )
				continue;

			y = (townPtr->abs_y1+townPtr->abs_y2)/2;

			dispSpy = config.disp_spy_sign && townPtr->has_player_spy();

			if( config.disp_town_name && dispSpy )
				y-=7;

			if( config.disp_town_name )
			{
				String str(townPtr->town_name());

				put_center_text( (townPtr->abs_x1+townPtr->abs_x2)/2,	y, str );

				y+=14;
			}

			if( dispSpy )
				put_center_text( (townPtr->abs_x1+townPtr->abs_x2)/2, y, "(Spy)" );
		}
	}

	//------ firms -------//

	if( config.disp_spy_sign )
	{
		Firm* firmPtr;

		for( int i=firm_array.size() ; i>0 ; i-- )
		{
			if( firm_array.is_deleted(i) )
				continue;

			firmPtr = firm_array[i];

			if( firmPtr->player_spy_count )
			{
				put_center_text( (firmPtr->abs_x1+firmPtr->abs_x2)/2,
									  (firmPtr->abs_y1+firmPtr->abs_y2)/2, "(Spy)" );
			}
		}
	}
}
//----------- End of function ZoomMatrix::disp_text ------------//


//---------- Begin of function ZoomMatrix::put_center_text -----------//
//
// <int>   x, y - center of the absolute position where the text should be put.
// <char*> str  - the display string.
//
void ZoomMatrix::put_center_text(int x, int y, const char* str)
{
	str = translate.process(str);

	const unsigned int TEMP_BUFFER_SIZE = 0x2000;
	char tempBuffer[TEMP_BUFFER_SIZE];
	short w = font_news.text_width(str);
	// ###### begin Gilbert 15/10 #######//
	// short h = font_news.text_height();
	short h = font_news.max_font_height;
	// ###### end Gilbert 15/10 #######//

	if( w * h + 2*sizeof(short) <= TEMP_BUFFER_SIZE )
	{
		char *bufferPtr = tempBuffer;
		*(short *)bufferPtr = w;
		bufferPtr += sizeof(short);
		*(short *)bufferPtr = h;
		bufferPtr += sizeof(short);
		memset( bufferPtr, TRANSPARENT_CODE, w * h );
		font_news.put_to_buffer(bufferPtr, w, 0, 0, str);

		// test clipping against ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2
		int x1 = x - World::view_top_x - w / 2 ;
		int x2 = x1 + w - 1;
		int y1 = y - World::view_top_y - h / 2;
		int y2 = y1 + h - 1;
		if( x1 < ZOOM_X2 && x2 >= 0 && y1 < ZOOM_HEIGHT && y2 >= 0)
		{
			if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
			{
				vga_back.put_bitmap_area_trans( x1+ZOOM_X1, y1+ZOOM_Y1, tempBuffer, 
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1);
			}
			else
			{
				vga_back.put_bitmap_trans( x1+ZOOM_X1, y1+ZOOM_Y1, tempBuffer );
			}
		}
	}
}
//----------- End of function ZoomMatrix::put_center_text ------------//


//---------- Begin of function ZoomMatrix::blacken_unexplored -----------//
//
void ZoomMatrix::blacken_unexplored()
{
	//----------- black out unexplored area -------------//

	int leftLoc = top_x_loc;
	int topLoc = top_y_loc;
	int rightLoc = leftLoc + disp_x_loc - 1;
	int bottomLoc = topLoc + disp_y_loc - 1;
	int scrnY, scrnX;		// screen coordinate
	int x, y;				// x,y Location
	Location *thisRowLoc, *northRowLoc, *southRowLoc;

	scrnY = ZOOM_Y1;
	for( y = topLoc; y <= bottomLoc; ++y, scrnY += ZOOM_LOC_HEIGHT)
	{
		thisRowLoc = get_loc(leftLoc, y);
		northRowLoc = y > 0 ? get_loc(leftLoc, y-1) : thisRowLoc;
		southRowLoc = y+1 < max_y_loc ? get_loc(leftLoc, y+1): thisRowLoc;

		// load north bit into bit0, north west bit into bit 1
		int northRow = northRowLoc->explored() ? 1 : 0;
		int thisRow = thisRowLoc->explored() ? 1 : 0;
		int southRow = southRowLoc->explored() ? 1 : 0;

		if( leftLoc > 0)
		{
			northRow |= (northRowLoc-1)->explored() ? 2 : 0;
			thisRow  |= (thisRowLoc -1)->explored() ? 2 : 0;
			southRow |= (southRowLoc-1)->explored() ? 2 : 0;
		}
		else
		{
			// replicate bit 0 to bit 1;
			northRow *= 3;
			thisRow  *= 3;
			southRow *= 3;
		}

		scrnX = ZOOM_X1;
		for( x = leftLoc; x <= rightLoc; ++x, scrnX += ZOOM_LOC_WIDTH )
		{
			if( x+1 < max_x_loc)
			{
				northRow = (northRow << 1) | ((++northRowLoc)->explored() ? 1 : 0);
				thisRow  = (thisRow  << 1) | ((++thisRowLoc )->explored() ? 1 : 0);
				southRow = (southRow << 1) | ((++southRowLoc)->explored() ? 1 : 0);
			}
			else
			{
				// replicate bit 1
				northRow = (northRow << 1) | (northRow & 1);
				thisRow  = (thisRow  << 1) | (thisRow  & 1);
				southRow = (southRow << 1) | (southRow & 1);
			}

			// optional 
			// northRow &= 7;
			// thisRow &= 7;
			// southRow &= 7;

			// ---------- Draw mask to vgabuf --------//

			if( thisRow & 2)		// center square
			{
				explored_mask.draw(scrnX, scrnY, northRow, thisRow, southRow);
			}
			else
			{
				vga_back.black_32x32(scrnX, scrnY);
			}
		}
	}
}
//----------- End of function ZoomMatrix::blacken_unexplored ------------//


//---------- Begin of function ZoomMatrix::blacken_fog_of_war -----------//
//
void ZoomMatrix::blacken_fog_of_war()
{
	int leftLoc = top_x_loc;
	int topLoc = top_y_loc;
	int rightLoc = leftLoc + disp_x_loc - 1;
	int bottomLoc = topLoc + disp_y_loc - 1;
	int scrnY, scrnX;		// screen coordinate
	int x, y;				// x,y Location
	Location *thisRowLoc, *northRowLoc, *southRowLoc;

	if( config.fog_mask_method == 1)
	{
		// use fast method
		scrnY = ZOOM_Y1;
		for( y = topLoc; y <= bottomLoc; ++y, scrnY += ZOOM_LOC_HEIGHT)
		{
			thisRowLoc = get_loc(leftLoc,y);
			scrnX = ZOOM_X1;
			for( x = leftLoc; x <= rightLoc; ++x, scrnX += ZOOM_LOC_WIDTH, ++thisRowLoc )
			{
				if( !thisRowLoc->explored() )
				{
					vga_back.bar(scrnX, scrnY, scrnX+ZOOM_LOC_WIDTH-1, scrnY+ZOOM_LOC_HEIGHT-1, 0);
				}
				else
				{
					unsigned char v = thisRowLoc->visibility();
					if( v < MAX_VISIT_LEVEL-7)
					{
						// more visible draw 1/4 tone
						vga_back.pixelize_32x32(scrnX+1, scrnY, 0);
						vga_back.pixelize_32x32(scrnX, scrnY+1, 0);
					}
					// for visibility >= MAX_VISIT_LEVEL, draw nothing
				}
			}
		}
	}
	else
	{
		// use slow method
		scrnY = ZOOM_Y1;
		for( y = topLoc; y <= bottomLoc; ++y, scrnY += ZOOM_LOC_HEIGHT)
		{
			thisRowLoc = get_loc(leftLoc, y);
			northRowLoc = y > 0 ? get_loc(leftLoc, y-1) : thisRowLoc;
			southRowLoc = y+1 < max_y_loc ? get_loc(leftLoc, y+1): thisRowLoc;

			// load north bit into bit0, north west bit into bit 1
			// [2] = west, [1] = this, [0] = east
			unsigned char northRow[3];
			unsigned char thisRow[3];
			unsigned char southRow[3];
			northRow[0] = northRowLoc->visibility();
			thisRow[0] = thisRowLoc->visibility();
			southRow[0] = southRowLoc->visibility();

			if( leftLoc > 0)
			{
				northRow[1] = (northRowLoc-1)->visibility();
				thisRow[1] = (thisRowLoc-1)->visibility();
				southRow[1] = (southRowLoc-1)->visibility();
			}
			else
			{
				// copy [0] to [1]
				northRow[1] = northRow[0];
				thisRow[1] = thisRow[0];
				southRow[1] = southRow[0];
			}

			scrnX = ZOOM_X1;
			for( x = leftLoc; x <= rightLoc; ++x, scrnX += ZOOM_LOC_WIDTH )
			{
				// shift to west
				northRow[2] = northRow[1]; northRow[1] = northRow[0];
				thisRow[2] = thisRow[1]; thisRow[1] = thisRow[0];
				southRow[2] = southRow[1]; southRow[1] = southRow[0];

				// shift in east squares of each row
				if( x+1 < max_x_loc)
				{
					northRow[0] = (++northRowLoc)->visibility();
					thisRow[0] = (++thisRowLoc)->visibility();
					southRow[0] = (++southRowLoc)->visibility();
				}
				// if on the east of the map, simply replicate the eastest square

				// ---------- Draw mask to vgabuf --------//
				unsigned char midNorthRow[3];
				unsigned char midThisRow[3];
				unsigned char midSouthRow[3];
				midThisRow[2] = MIN( thisRow[2], thisRow[1]);
				midThisRow[0] = MIN( thisRow[0], thisRow[1]);
				midNorthRow[2] = MIN( MIN(northRow[2], northRow[1]), midThisRow[2] );
				midNorthRow[1] = MIN( northRow[1], thisRow[1]);
				midNorthRow[0] = MIN( MIN(northRow[0], northRow[1]), midThisRow[0] );
				midSouthRow[2] = MIN( MIN(southRow[2], southRow[1]), midThisRow[2] );
				midSouthRow[1] = MIN( southRow[1], thisRow[1]);
				midSouthRow[0] = MIN( MIN(southRow[0], southRow[1]), midThisRow[0] );
				unsigned char midMean = ((int) thisRow[0] + thisRow[2] +
					northRow[0] + northRow[1] + northRow[2] +
					southRow[0] + southRow[1] + southRow[2] ) /8;
				midThisRow[1] = MIN(thisRow[1], midMean );

				vga_back.fog_remap(scrnX, scrnY, (char **)explored_mask.brightness_table->get_table_array(),
					midNorthRow, midThisRow, midSouthRow);
			}
		}
	}
}
//---------- End of function ZoomMatrix::blacken_fog_of_war -----------//


//--------- Begin of function ZoomMatrix::draw_objects ---------//
//
// Draw the following types of objects on the zoom map in a sorted order.
//
// 1. Firms
// 2. Town sections
// 3. Sprites
//
void ZoomMatrix::draw_objects()
{
	//----- get the location of the zoom area ------//

	const int DRAW_OUTSIDE = 3;
  
	int zoomXLoc1 = world.zoom_matrix->top_x_loc - DRAW_OUTSIDE;
	int zoomYLoc1 = world.zoom_matrix->top_y_loc - DRAW_OUTSIDE;
	int zoomXLoc2 = world.zoom_matrix->top_x_loc + world.zoom_matrix->disp_x_loc - 1 + DRAW_OUTSIDE;
	int zoomYLoc2 = world.zoom_matrix->top_y_loc + world.zoom_matrix->disp_y_loc - 1 + DRAW_OUTSIDE;

	if( zoomXLoc1 < 0)
		zoomXLoc1 = 0;
	if( zoomYLoc1 < 0)
		zoomYLoc1 = 0;
	if( zoomXLoc2 >= max_x_loc)
		zoomXLoc2 = max_x_loc-1;
	if( zoomYLoc2 >= max_y_loc)
		zoomYLoc2 = max_y_loc-1;

	//---- add the objects on the zoom area to land_disp_sort_array in a sorted display order ---//

	int 		 	 xLoc, yLoc;
	Location* 	 locPtr;
	DisplaySort  displaySort;
	Unit*			 unitPtr;
	Firm*			 firmPtr;
	Town* 	 	 townPtr;
	PlantBitmap* plantBitmap;
	int			 innerY, drawY;
	int			 dispFire = 0;
	// ####### begin Gilbert 31/7 ########//
	char			 pMobileType;           // pointing mobileType
	Location*	 pLoc = power.test_detect(mouse.cur_x, mouse.cur_y, &pMobileType);
   // ####### end Gilbert 31/7 ########//

	for( yLoc=zoomYLoc1 ; yLoc<=zoomYLoc2 ; yLoc++ )
	{
		locPtr = world.get_loc(zoomXLoc1,yLoc);

		for( xLoc=zoomXLoc1 ; xLoc<=zoomXLoc2 ; xLoc++, locPtr++ )
		{
			//------- if there is an unit in the air --------//

			if( locPtr->has_unit(UNIT_AIR) )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				unitPtr = unit_array[locPtr->air_cargo_recno];

				unitPtr->update_abs_pos();		// update its absolute position

				displaySort.object_type  = OBJECT_UNIT;
				displaySort.object_recno = locPtr->air_cargo_recno;
				displaySort.object_y2 	 = unitPtr->abs_y2;

				if(pLoc == locPtr && pMobileType == UNIT_AIR)
				{
					displaySort.object_type  = OBJECT_POINTED_UNIT;
					// BUGHERE : this part may fails if sprite size > 1x1
				}

				if( !unitPtr->is_shealth() )
				{
					if( unitPtr->sprite_info->loc_width > 1 )
					{
						if( xLoc==MAX( unitPtr->next_x_loc(), zoomXLoc1 ) &&
							 yLoc==MAX( unitPtr->next_y_loc(), zoomYLoc1 ) )
						{
							air_disp_sort_array.linkin(&displaySort);
						}
					}
					else
					{
						air_disp_sort_array.linkin(&displaySort);
					}
				}
			}

			//------- if there is an unit on the land or sea -------//

			if( locPtr->has_unit(UNIT_LAND) || locPtr->has_unit(UNIT_SEA) )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				unitPtr = unit_array[locPtr->cargo_recno];

				unitPtr->update_abs_pos();		// update its absolute position

				displaySort.object_type  = OBJECT_UNIT;
				displaySort.object_recno = locPtr->cargo_recno;
				displaySort.object_y2 	 = unitPtr->abs_y2;

				if( pLoc == locPtr && (pMobileType == UNIT_LAND || pMobileType == UNIT_SEA))
				{
					displaySort.object_type = OBJECT_POINTED_UNIT;
					// BUGHERE : this part may fails if sprite size > 1x1
				}

				if( !unitPtr->is_shealth() )
				{
					if( unitPtr->sprite_info->loc_width > 1 )
					{
						if( xLoc==MAX( unitPtr->next_x_loc(), zoomXLoc1 ) &&
							 yLoc==MAX( unitPtr->next_y_loc(), zoomYLoc1 ) )
						{
							land_disp_sort_array.linkin(&displaySort);
						}
					}
					else
					{
						land_disp_sort_array.linkin(&displaySort);
					}
				}
			}

			//--------- if there is a firm on the location --------//

			else if( locPtr->is_firm() )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				displaySort.object_type  = OBJECT_FIRM;
				displaySort.object_recno = locPtr->firm_recno();

				firmPtr = firm_array[locPtr->firm_recno()];

				displaySort.object_y2 = firmPtr->abs_y2;

				if( xLoc==MAX( firmPtr->loc_x1, zoomXLoc1 ) &&
					 yLoc==MAX( firmPtr->loc_y1, zoomYLoc1 ) )
				{
					land_bottom_disp_sort_array.linkin(&displaySort);
					land_disp_sort_array.linkin(&displaySort);
				}
			}

			//------ if there is a town section on the location -----//

			else if( locPtr->is_town() )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				displaySort.object_type  = OBJECT_TOWN;
				displaySort.object_recno = locPtr->town_recno();

				townPtr = town_array[locPtr->town_recno()];

				displaySort.object_y2 = townPtr->abs_y2;

				if( xLoc==MAX( townPtr->loc_x1, zoomXLoc1 ) &&
					 yLoc==MAX( townPtr->loc_y1, zoomYLoc1 ) )
				{
					land_bottom_disp_sort_array.linkin(&displaySort);
					land_disp_sort_array.linkin(&displaySort);
				}
			}

			//------ if there is a plant on the location -----//

			else if( locPtr->is_plant() )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				displaySort.object_type  = OBJECT_PLANT;
				displaySort.object_recno = locPtr->plant_id();

				plantBitmap = plant_res.get_bitmap(locPtr->plant_id());

				innerY  = locPtr->plant_inner_y();
				drawY   = yLoc*ZOOM_LOC_HEIGHT + innerY-ZOOM_LOC_HEIGHT/2 + plantBitmap->offset_y + plantBitmap->bitmap_height - 1;

				displaySort.object_y2 =	drawY;
				displaySort.x_loc	    = xLoc;
				displaySort.y_loc	    = yLoc;

				land_disp_sort_array.linkin(&displaySort);
			}

			//------ if there is a wall on the location -------//

			else if( locPtr->is_wall() )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				WallInfo *wallInfo = wall_res[locPtr->wall_id()];
				displaySort.object_type	= OBJECT_WALL;

				// high byte of object_recno stores nation_recno
				// low byte of object_recno stores wall_id
				displaySort.object_recno = wallInfo->draw_wall_id;
				if( locPtr->power_nation_recno > 0)
				{
					displaySort.object_recno += locPtr->power_nation_recno << 8;
				}
				if( ! wallInfo->is_gate())
				{
					// -------- non-gate square
					displaySort.object_y2 = yLoc* ZOOM_LOC_HEIGHT +ZOOM_LOC_HEIGHT-1;
					displaySort.x_loc = xLoc * ZOOM_LOC_WIDTH;
					displaySort.y_loc = yLoc * ZOOM_LOC_HEIGHT;
					land_disp_sort_array.linkin(&displaySort);
				}
				else
				{
					// -------- gate square ---------//
					displaySort.object_y2 = yLoc * ZOOM_LOC_HEIGHT + wallInfo->offset_y +
						wallInfo->bitmap_height() -1;
					displaySort.x_loc = xLoc + wallInfo->loc_off_x;
					displaySort.y_loc = yLoc + wallInfo->loc_off_y;
					if( xLoc == MAX( displaySort.x_loc, zoomXLoc1) &&
						 yLoc == MAX( displaySort.y_loc, zoomYLoc1) )
					{
						displaySort.x_loc = xLoc * ZOOM_LOC_WIDTH + wallInfo->offset_x;
						displaySort.y_loc = yLoc * ZOOM_LOC_HEIGHT + wallInfo->offset_y;
						land_disp_sort_array.linkin(&displaySort);
					}
				}
			}
			else if(locPtr->has_hill() && hill_res[locPtr->hill_id1()]->layer & 2 )
			{
				memset(&displaySort, 0, sizeof(displaySort));
				displaySort.object_type = OBJECT_HILL;
				displaySort.object_recno = locPtr->hill_id1();
				displaySort.object_y2 = (yLoc+1)*ZOOM_LOC_HEIGHT-1;
				displaySort.x_loc = xLoc;
				displaySort.y_loc = yLoc;
				land_disp_sort_array.linkin(&displaySort);
			}
			else if(locPtr->is_rock())
			{
				memset(&displaySort, 0, sizeof(displaySort));
				displaySort.object_type  = OBJECT_ROCK;
				Rock *rockPtr = rock_array[displaySort.object_recno = locPtr->rock_array_recno()];
				displaySort.object_y2 =	ZOOM_LOC_HEIGHT  * (rockPtr->loc_y 
					+ rock_res.get_rock_info(rockPtr->rock_recno)->loc_height) -1;

				if( xLoc==MAX( rockPtr->loc_x, zoomXLoc1 ) &&
					 yLoc==MAX( rockPtr->loc_y, zoomYLoc1 ) )
				{
					land_disp_sort_array.linkin(&displaySort);
				}
			}

			#ifdef DEBUG2	
			if(debug_sim_game_type!=2 && locPtr->fire_str()>0)
			#else
			if( locPtr->fire_str() > 0 )
			#endif
			{
				memset(&displaySort, 0, sizeof(displaySort));
				displaySort.object_type = OBJECT_FIRE;
				displaySort.object_recno = locPtr->fire_str();
				displaySort.object_y2 = (yLoc+1)*ZOOM_LOC_HEIGHT -
					((((xLoc+13) * (yLoc+17)) % 16) & ~1);

				displaySort.x_loc	    = xLoc;
				displaySort.y_loc	    = yLoc;

				land_disp_sort_array.linkin(&displaySort);
				land_top_disp_sort_array.linkin(&displaySort);
				dispFire++;
			}

		}
	}

	//------ add bullet sprites to the display array -------//

	Bullet* bulletPtr;

	int i;
	for( i=bullet_array.size() ; i>0 ; i-- )
	{
		if( bullet_array.is_deleted(i) )
			continue;

		bulletPtr = bullet_array[i];

		// ######### begin Gilbert 20/6 #########//
		if( bulletPtr->is_shealth() || 
			bulletPtr->cur_x_loc() < zoomXLoc1 || bulletPtr->cur_x_loc() > zoomXLoc2 ||
			bulletPtr->cur_y_loc() < zoomYLoc1 || bulletPtr->cur_y_loc() > zoomYLoc2 )
			continue;
		// ######### end Gilbert 20/6 #########//

		bulletPtr->update_abs_pos();		// update its absolute position

		displaySort.object_type  = OBJECT_BULLET;
		displaySort.object_recno = i;
		displaySort.object_y2 	 = bulletPtr->abs_y2;

		switch( bulletPtr->display_layer() )
		{
		case AIR_DISP_LAYER_MASK:
			air_disp_sort_array.linkin(&displaySort);
			break;
		case LAND_BOTTOM_DISP_LAYER_MASK:
			land_bottom_disp_sort_array.linkin(&displaySort);
			break;
		case LAND_TOP_DISP_LAYER_MASK:
			land_top_disp_sort_array.linkin(&displaySort);
			break;
		case 0:
		case LAND_DISP_LAYER_MASK:
			land_disp_sort_array.linkin(&displaySort);
			break;
		default:
			err_here();
		}
	}

	// --------- draw tornado --------//
	Tornado *tornadoPtr;
	for( i=tornado_array.size(); i > 0; i--)
	{
		if( tornado_array.is_deleted(i) )
			continue;
		tornadoPtr = tornado_array[i];
		// ######### begin Gilbert 28/5 #########//
		// if( tornadoPtr->is_shealth() )
		if( // tornadoPtr->is_shealth() || 
			tornadoPtr->cur_x_loc() < zoomXLoc1 || tornadoPtr->cur_x_loc() > zoomXLoc2 ||
			tornadoPtr->cur_y_loc() < zoomYLoc1 || tornadoPtr->cur_y_loc() > zoomYLoc2 )
			continue;
		// ######### end Gilbert 28/5 #########//

		tornadoPtr->update_abs_pos();

		displaySort.object_type = OBJECT_TORNADO;
		displaySort.object_recno  = i;
		displaySort.object_y2 = tornadoPtr->abs_y2;
		air_disp_sort_array.linkin(&displaySort);
	}

	for( i=effect_array.size(); i > 0; i--)
	{
		if( effect_array.is_deleted(i) )
			continue;
		Effect *effectPtr = (Effect *)effect_array[i];
		// ######### begin Gilbert 28/5 #########//
		if( effectPtr->is_shealth() ||
			effectPtr->cur_x_loc() < zoomXLoc1 || effectPtr->cur_x_loc() > zoomXLoc2 ||
			effectPtr->cur_y_loc() < zoomYLoc1 || effectPtr->cur_y_loc() > zoomYLoc2 )
			continue;
		// ######### end Gilbert 28/5 #########//
		effectPtr->update_abs_pos();

		displaySort.object_type = OBJECT_EFFECT;
		displaySort.object_recno  = i;
		displaySort.object_y2 = effectPtr->abs_y2;

		switch( effectPtr->layer )
		{
		case AIR_DISP_LAYER_MASK:
			air_disp_sort_array.linkin(&displaySort);
			break;
		case LAND_BOTTOM_DISP_LAYER_MASK:
			land_bottom_disp_sort_array.linkin(&displaySort);
			break;
		case LAND_TOP_DISP_LAYER_MASK:
			land_top_disp_sort_array.linkin(&displaySort);
			break;
		case 0:
		case LAND_DISP_LAYER_MASK:
			land_disp_sort_array.linkin(&displaySort);
			break;
		default:
			err_here();
		}
	}

	// ###### begin Gilbert 2/10 #######//
	for( i=firm_die_array.size(); i > 0; i--)
	{
		if( firm_die_array.is_deleted(i) )
			continue;

		FirmDie *firmDiePtr = (FirmDie *)firm_die_array[i];

		if( firmDiePtr->loc_x2 < zoomXLoc1 || firmDiePtr->loc_x1 > zoomXLoc2 ||
			firmDiePtr->loc_y2 < zoomYLoc1 || firmDiePtr->loc_y1 > zoomYLoc2 )
			continue;

		//--------- if there is a dying firm on the location --------//

		memset(&displaySort, 0, sizeof(displaySort));
		displaySort.object_type  = OBJECT_FIRM_DIE;
		displaySort.object_recno = i;

		displaySort.object_y2 = firmDiePtr->loc_y2 * ZOOM_LOC_HEIGHT;

		land_bottom_disp_sort_array.linkin(&displaySort);
		land_disp_sort_array.linkin(&displaySort);
	}
	// ###### end Gilbert 2/10 #######//


	//---------- quicksort the array -----------//

	land_disp_sort_array.quick_sort( sort_display_function );
	air_disp_sort_array.quick_sort( sort_display_function );
	land_top_disp_sort_array.quick_sort( sort_display_function );
	land_bottom_disp_sort_array.quick_sort( sort_display_function );

	// ##### begin Gilbert 9/10 ######//
	//------------ draw unit path and objects ---------------//

	draw_objects_now(&land_bottom_disp_sort_array, LAND_BOTTOM_DISP_LAYER_MASK);
	draw_unit_path_on_zoom_map(LAND_DISP_LAYER_MASK);
	draw_objects_now(&land_disp_sort_array,LAND_DISP_LAYER_MASK);
	draw_objects_now(&land_top_disp_sort_array,LAND_TOP_DISP_LAYER_MASK);

	draw_unit_path_on_zoom_map(AIR_DISP_LAYER_MASK);
	draw_unit_way_point_on_zoom_map();
	draw_objects_now(&air_disp_sort_array);
	// ##### end Gilbert 9/10 ######//


	//----------- clean up the array ----------//

	land_disp_sort_array.zap(0);		// 0-don't resize the array, keep its current size
	air_disp_sort_array.zap(0);		// 0-don't resize the array, keep its current size
	land_top_disp_sort_array.zap(0);
	land_bottom_disp_sort_array.zap(0);

	//----------- fire sound ----------//
	if(dispFire > 0)
	{
		int relVolume = 80 + dispFire/2;
		if( relVolume > 100)
			relVolume = 100;
		if( fire_channel_id == 0)
		{
			last_fire_vol = relVolume;
			RelVolume r(relVolume,0);
			fire_channel_id = audio.play_loop_wav( DIR_SOUND"FIRE.WAV",8447 *2, DsVolume(r));
		}
		else if( last_fire_vol - relVolume > 2 || last_fire_vol - relVolume < 2)
		{
			last_fire_vol = relVolume;
			RelVolume r(relVolume,0);
			audio.volume_loop_wav(fire_channel_id, DsVolume(r));
		}
	}
	else
	{
		if( fire_channel_id != 0)
		{
			audio.stop_loop_wav(fire_channel_id);
			fire_channel_id = 0;
			last_fire_vol = 0;
		}
	}
}
//----------- End of function ZoomMatrix::draw_objects -----------//


//---------- Begin of function ZoomMatrix::draw_objects_now -----------//
//
void ZoomMatrix::draw_objects_now(DynArray* unitArray, int displayLayer)
{
	//------------ display objects ------------//

	DisplaySort *displaySortPtr;
	Firm			*firmPtr;
	int 			i, dispCount = unitArray->size();
	char			firstFire[FLAME_GROW_STEP];
	memset( firstFire, 0, sizeof(firstFire));
	int			riseFirePara = 0;
	int			needFlushFire = weather.rain_scale() + weather.snow_scale();
	double hWindSpeed = weather.wind_speed()*sin(weather.wind_direct_rad());
	if( hWindSpeed >= 20.0)
		riseFirePara = 1;
	else if( hWindSpeed > -20.0)
		riseFirePara = 0;
	else
		riseFirePara = -1;

	if( init_fire <= flame[FLAME_GROW_STEP-1].map_height)
	{
		for( int f = FLAME_GROW_STEP-1 ; f >= 0; --f)
		{
			if( init_fire <= flame[f].map_height)
			{
				flame[f].rise(riseFirePara);
			}
			else
			{
				break;
			}
		}
		init_fire++;
	}

	int dispPower = (world.map_matrix->map_mode == MAP_MODE_POWER &&
						  world.map_matrix->power_mode == 1) ||
						 power.command_id == COMMAND_BUILD_FIRM ||
						 power.command_id == COMMAND_SETTLE ||
						 power.command_id == COMMAND_BUILD_WALL;

	for( i=1 ; i<=dispCount ; i++ )
	{
		if( i%10==1 )
			sys.yield();

		displaySortPtr = (DisplaySort*) unitArray->get(i);

		switch(displaySortPtr->object_type)
		{
			case OBJECT_UNIT:
				unit_array[displaySortPtr->object_recno]->draw();
				break;

			case OBJECT_POINTED_UNIT:
				unit_array[displaySortPtr->object_recno]->draw_outlined();
				break;

			case OBJECT_BULLET:
				bullet_array[displaySortPtr->object_recno]->draw();
				break;

			case OBJECT_FIRM:
				firmPtr = firm_array[displaySortPtr->object_recno];
				firmPtr->draw(displayLayer);
				break;

			case OBJECT_TOWN:
				town_array[displaySortPtr->object_recno]->draw(displayLayer);
				break;

			case OBJECT_PLANT:
				plant_res.get_bitmap(displaySortPtr->object_recno)
					->draw(displaySortPtr->x_loc, displaySortPtr->y_loc);
				break;

			case OBJECT_ROCK:
				// object_recno is rockArrayRecno
				rock_array[displaySortPtr->object_recno]->draw();
				break;

			case OBJECT_FIRE:
				{
					int f;
					// when displayLayer = 0, no fire is assumed to be drawn
					// pass fireDisplayerLayer as 1 to this function
					err_when(!displayLayer);

					// ------- decide bitmap to draw ----
					// display flame[f], where f = (fire_str()-1) /25
					f = Flame::grade( displaySortPtr->object_recno );
					err_when(f >= FLAME_GROW_STEP);
					if( !firstFire[f])
					{
						firstFire[f] = 1;
						if( displayLayer == 1)
						{
							if( needFlushFire )
								flame[f].flush_point();
							flame[f].rise(riseFirePara);
							flame[f].gen_bitmap(0xe3);		// 0xb4
							flame[f].mask_bottom();
						}
						else
						{
							flame[f].mask_transparent();
						}
					}

					int x1 = displaySortPtr->x_loc * ZOOM_LOC_WIDTH + Flame::offset_x(f) - World::view_top_x;
					int y1 = displaySortPtr->y_loc * ZOOM_LOC_HEIGHT + Flame::offset_y(f) - World::view_top_y;

					// ------- shift 'randomly' but even number---------
					x1 += (((displaySortPtr->x_loc+11) * (displaySortPtr->y_loc+13)) % 16) & ~1;
					x1 -= 6;
					y1 -= (((displaySortPtr->x_loc+13) * (displaySortPtr->y_loc+17)) % 16) & ~1;

					int x2 = x1 + Flame::default_width(f) -1;
					int y2 = y1 + Flame::default_height(f) -1;

					if( x2 >= 0 && x1 < ZOOM_WIDTH && y2 >= 0 && y1 < ZOOM_HEIGHT )
					{
						if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
						{
							vga_back.put_bitmap_area_trans( x1+ZOOM_X1, y1+ZOOM_Y1, (char *)flame[f].bitmap,
								MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
						}
						else
						{
							vga_back.put_bitmap_trans( x1+ZOOM_X1, y1+ZOOM_Y1, (char *)flame[f].bitmap );
						}
					}
				}
				break;

			case OBJECT_WALL:
				{
					int nationRecno = displaySortPtr->object_recno >> 8;
					char *remapTable = game.get_color_remap_table(nationRecno, 0);
					wall_res[displaySortPtr->object_recno & 0xff]->draw_at(
						displaySortPtr->x_loc, displaySortPtr->y_loc, remapTable);
				}
				break;
			case OBJECT_TORNADO:
				tornado_array[displaySortPtr->object_recno]->draw();
				break;

			case OBJECT_HILL:
				{
					short xLoc = displaySortPtr->x_loc;
					short yLoc = displaySortPtr->y_loc;
					hill_res[displaySortPtr->object_recno]->draw(xLoc, yLoc, 2);
					
					// ------ draw power, because hill covers the power colour drawn ------//
					int nationRecno = get_loc(xLoc, yLoc)->power_nation_recno;
					if( dispPower && nationRecno > 0)
					{
						int x1 = xLoc*ZOOM_LOC_WIDTH - World::view_top_x;
						int y1 = yLoc*ZOOM_LOC_HEIGHT - World::view_top_y;
						if( x1 >= 0 && y1 >= 0 && x1 < ZOOM_WIDTH - (ZOOM_LOC_WIDTH-1) && y1 < ZOOM_HEIGHT - (ZOOM_LOC_HEIGHT-1))
						{
							vga_back.pixelize_32x32( x1 + ZOOM_X1, y1 + ZOOM_Y1,
								nation_array.nation_power_color_array[nationRecno] );
						}
					}
				}
				break;

			// #### begin Gilbert 4/10 #######//
			case OBJECT_EFFECT:
				effect_array[displaySortPtr->object_recno]->draw();
				break;

			case OBJECT_FIRM_DIE:
				firm_die_array[displaySortPtr->object_recno]->draw(displayLayer);
				break;
			// #### end Gilbert 4/10 #######//
		}
	}
}
//----------- End of function ZoomMatrix::draw_objects_now ------------//


//---------- Begin of function ZoomMatrix::scroll -----------//
//
// <int> xScroll - horizontal scroll step (negative:left, positive:right)
// <int> yScroll - vertical scroll step   (negative:left, positive:right)
//
void ZoomMatrix::scroll(int xScroll, int yScroll)
{
	Matrix::scroll(xScroll,yScroll);

	world.map_matrix->cur_x_loc = top_x_loc;
	world.map_matrix->cur_y_loc = top_y_loc;
}
//----------- End of function ZoomMatrix::scroll ------------//


//------ Begin of function sort_display_function ------//
//
static int sort_display_function( const void *a, const void *b )
{
	return ((DisplaySort*)a)->object_y2 - ((DisplaySort*)b)->object_y2;
}
//------- End of function sort_display_function ------//


//------ Begin of function ZoomMatrix::put_bitmap_clip ---------//
//
// Put a bitmap on the surface buffer
//
// <int>   x, y 			  - the location of the bitmap, in the current screen coordination
// <char*> bitmapPtr 	  - bitmap ptr
// [int]   compressedFlag - whether the bitmap is compressed or not
//									 (default: 0)
//
void ZoomMatrix::put_bitmap_clip(int x, int y, char* bitmapPtr, int compressedFlag)
{
	int x2 = x + *((short*)bitmapPtr) 	  - 1;
	int y2 = y + *(((short*)bitmapPtr)+1) - 1;

	if( x2 < ZOOM_X1 || y2 < ZOOM_Y1 || x > ZOOM_X2 || y > ZOOM_Y2 )
		return;

	//---- only portion of the sprite is inside the view area ------//

	if( x < ZOOM_X1 || x2 > ZOOM_X2 || y < ZOOM_Y1 || y2 > ZOOM_Y2 )
	{
		if( compressedFlag )
		{
			vga_back.put_bitmap_area_trans_decompress( x, y, bitmapPtr,
				MAX(ZOOM_X1,x)-x, MAX(ZOOM_Y1,y)-y, MIN(ZOOM_X2,x2)-x, MIN(ZOOM_Y2,y2)-y );
		}
		else
		{
			vga_back.put_bitmap_area_trans( x, y, bitmapPtr,
				MAX(ZOOM_X1,x)-x, MAX(ZOOM_Y1,y)-y, MIN(ZOOM_X2,x2)-x, MIN(ZOOM_Y2,y2)-y );
		}
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		if( compressedFlag )
			vga_back.put_bitmap_trans_decompress( x, y, bitmapPtr );
		else
			vga_back.put_bitmap_trans( x, y, bitmapPtr );
	}
}
//--------- End of function ZoomMatrix::put_bitmap_clip ---------//


//------ Begin of function ZoomMatrix::detect_bitmap_clip ---------//
//
// Detect clicking on the bitmap.
//
// return: <int> 0 - not detected
//					  1 - left clicked
//					  2 - right clicked
//
int ZoomMatrix::detect_bitmap_clip(int x, int y, char* bitmapPtr)
{
	int x2 = x + *((short*)bitmapPtr) 	  - 1;
	int y2 = y + *(((short*)bitmapPtr)+1) - 1;

	if( x2 < ZOOM_X1 || y2 < ZOOM_Y1 || x > ZOOM_X2 || y > ZOOM_Y2 )
		return 0;

	//---- only portion of the sprite is inside the view area ------//

	// return mouse.single_click( MAX(ZOOM_X1,x), MAX(ZOOM_Y1,y), MIN(ZOOM_X2,x2), MIN(ZOOM_Y2,y2), 2 );
	return mouse.any_click( MAX(ZOOM_X1,x), MAX(ZOOM_Y1,y), MIN(ZOOM_X2,x2), MIN(ZOOM_Y2,y2), 0 ) ? 1 :
	mouse.any_click( MAX(ZOOM_X1,x), MAX(ZOOM_Y1,y), MIN(ZOOM_X2,x2), MIN(ZOOM_Y2,y2), 1 ) ? 2 : 0;
}
//--------- End of function ZoomMatrix::detect_bitmap_clip ---------//


//------ Begin of function ZoomMatrix::put_bitmap_remap_clip ---------//
//
// Put a bitmap on the surface buffer
//
// <int>   x, y 				- the location of the bitmap
// <char*> bitmapPtr 		- bitmap ptr
// [char*] colorRemapTable - color remap table
// [int]   compressedFlag  - whether the bitmap is compressed or not
//									  (default: 0)
//
void ZoomMatrix::put_bitmap_remap_clip(int x, int y, char* bitmapPtr, char* colorRemapTable, int compressedFlag)
{
	int x2 = x + *((short*)bitmapPtr) 	  - 1;
	int y2 = y + *(((short*)bitmapPtr)+1) - 1;

	if( x2 < ZOOM_X1 || y2 < ZOOM_Y1 || x > ZOOM_X2 || y > ZOOM_Y2 )
		return;

	//---- only portion of the sprite is inside the view arec ------//

	if( x < ZOOM_X1 || x2 > ZOOM_X2 || y < ZOOM_Y1 || y2 > ZOOM_Y2 )
	{
		if( compressedFlag )
		{
			if( colorRemapTable )
			{
				vga_back.put_bitmap_area_trans_remap_decompress( x, y, bitmapPtr,
					MAX(ZOOM_X1,x)-x, MAX(ZOOM_Y1,y)-y, MIN(ZOOM_X2,x2)-x, MIN(ZOOM_Y2,y2)-y, colorRemapTable );
			}
			else
			{
				vga_back.put_bitmap_area_trans_decompress( x, y, bitmapPtr,
					MAX(ZOOM_X1,x)-x, MAX(ZOOM_Y1,y)-y, MIN(ZOOM_X2,x2)-x, MIN(ZOOM_Y2,y2)-y );
			}
		}
		else
		{
			if( colorRemapTable )
			{
				vga_back.put_bitmap_area_trans_remap( x, y, bitmapPtr,
					MAX(ZOOM_X1,x)-x, MAX(ZOOM_Y1,y)-y, MIN(ZOOM_X2,x2)-x, MIN(ZOOM_Y2,y2)-y, colorRemapTable );
			}
			else
			{
				vga_back.put_bitmap_area_trans( x, y, bitmapPtr,
					MAX(ZOOM_X1,x)-x, MAX(ZOOM_Y1,y)-y, MIN(ZOOM_X2,x2)-x, MIN(ZOOM_Y2,y2)-y );
			}
		}
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		if( compressedFlag )
		{
			if( colorRemapTable )
				vga_back.put_bitmap_trans_remap_decompress( x, y, bitmapPtr, colorRemapTable );
			else
				vga_back.put_bitmap_trans_decompress( x, y, bitmapPtr );
		}
		else
		{
			if( colorRemapTable )
				vga_back.put_bitmap_trans_remap( x, y, bitmapPtr, colorRemapTable );
			else
				vga_back.put_bitmap_trans( x, y, bitmapPtr );
		}
	}
}
//--------- End of function ZoomMatrix::put_bitmap_remap_clip ---------//
