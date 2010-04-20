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

//Filename    : OWORLD.CPP
//Description : Object World

#include <OSYS.h>
#include <OGAME.h>
#include <OVGA.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OFIRMRES.h>
#include <OPLANT.h>
#include <OPOWER.h>
#include <OSITE.h>
#include <OINFO.h>
#include <OTOWN.h>
#include <ONATION.h>
#include <OWEATHER.h>
#include <OTERRAIN.h>
#include <OWORLD.h>
#include <OANLINE.h>
#include <OTORNADO.h>
#include <OU_VEHI.h>
#include <OSERES.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <ONEWS.h>


//------------ Define static class variables ------------//

short World::view_top_x, World::view_top_y;
int   World::max_x_loc=200, World::max_y_loc=200;

//----------- Begin of function World::World ----------//

World::World()
{
	loc_matrix = NULL;
	next_scroll_time = 0;
	scan_fire_x = 0;
	scan_fire_y = 0;
	lightning_signal = 0;
	plant_count = 0;
	plant_limit = 0;

   //------- initialize matrix objects -------//

   map_matrix  = new MapMatrix;
   zoom_matrix = new ZoomMatrix;
}
//------------- End of function World::World -----------//


//----------- Begin of function World::~World ----------//

World::~World()
{
   if( map_matrix )
	{
      delete map_matrix;
      map_matrix = NULL;
   }

   if( zoom_matrix )
   {
      delete zoom_matrix;
      zoom_matrix = NULL;
   }

   deinit();
}
//------------- End of function World::~World -----------//


//----------- Begin of function World::init ----------//

void World::init()
{
	//----------- initialize vars -------------//

	scan_fire_x = 0;
	scan_fire_y = 0;
	lightning_signal = 0;

	map_matrix->init_para();
	zoom_matrix->init_para();
}
//------------- End of function World::init -----------//


//----------- Begin of function World::deinit ----------//

void World::deinit()
{
   if( loc_matrix )
   {
      mem_del( loc_matrix );
      loc_matrix  = NULL;
   }
}
//------------- End of function World::deinit -----------//


//--------- Begin of function World::assign_map ----------//
//
// After a map is loaded, assign_map() need to be called to
// initial map_matrix and zoom_matrix
//
void World::assign_map()
{
	//------------- assign map -------------//

   map_matrix-> assign_map(loc_matrix, max_x_loc, max_y_loc );
	zoom_matrix->assign_map(loc_matrix, max_x_loc, max_y_loc );

   //-------- set the zoom area box on map matrix ------//

   map_matrix->cur_x_loc = 0;
   map_matrix->cur_y_loc = 0;
   map_matrix->cur_cargo_width  = zoom_matrix->disp_x_loc;
   map_matrix->cur_cargo_height = zoom_matrix->disp_y_loc;
}
//----------- End of function World::assign_map ----------//


//----------- Begin of function World::paint ------------//
//
// Paint world window and scroll bars
//
void World::paint()
{
   map_matrix->paint();
   zoom_matrix->paint();
}
//----------- End of function World::paint ------------//


//----------- Begin of function World::refresh ------------//
//
void World::refresh()
{
   map_matrix->refresh();
   zoom_matrix->refresh();
}
//----------- End of function World::refresh ------------//


//----------- Begin of function World::process ------------//
//
// Called every frame
//
void World::process()
{
	//-------- process wall ----------//

	form_world_wall();

	//-------- process fire -----------//

	// BUGHERE : set Location::flammability for every change in cargo

	world.spread_fire(weather);

	// ------- process visibility --------//
	process_visibility();

	//-------- process lightning ------//
	// ###### begin Gilbert 11/8 ########//
	if(lightning_signal== 0 && weather.is_lightning())
	{
		// ------- create new lightning ----------//
		lightning_signal = 110;
	}
	if( lightning_signal == 106 && config.weather_effect)
	{
		lightning_strike(m.random(MAX_MAP_WIDTH), m.random(MAX_MAP_HEIGHT), 1);
	}
	if(lightning_signal == 100)
		lightning_signal = 5 + m.random(10);
	else if( lightning_signal)
		lightning_signal--;
	// ###### end Gilbert 11/8 ########//

	//---------- process ambient sound ---------//

	if( sys.frame_count%10 == 0 )    // process once per ten frames
		process_ambient_sound();

	// --------- update scan fire x y ----------//
	if(++scan_fire_x >= SCAN_FIRE_DIST)
	{
		scan_fire_x = 0;
		if( ++scan_fire_y >= SCAN_FIRE_DIST)
			scan_fire_y =0;
	}

}
//----------- End of function World::process ------------//


//----------- Begin of function World::next_day ------------//
//
// Called every frame
//
void World::next_day()
{
   plant_ops();

   weather = weather_forecast[0];

	for(int foreDay=0; foreDay < MAX_WEATHER_FORECAST-1; ++foreDay)
   {
      weather_forecast[foreDay] = weather_forecast[foreDay+1];
   }

   weather_forecast[MAX_WEATHER_FORECAST-1].next_day();

	// ####### begin Gilbert 11/7 #########//
	magic_weather.next_day();
	// ####### end Gilbert 11/7 #########//

	if(weather.has_tornado() && config.weather_effect)
	{
		tornado_array.add_tornado(weather.tornado_x_loc(max_x_loc, max_y_loc),
			weather.tornado_y_loc(max_x_loc, max_y_loc), 600);
	}

	// ######## begin Gilbert 31/7 #######//
	if( weather.is_quake() && config.random_event_frequency)
	// ######## end Gilbert 31/7 #######//
	{
		earth_quake();
	}

	//-------- Debug code: BUGHERE ----------//

	#ifdef DEBUG

	Location* locPtr = loc_matrix;

	for( int y=0 ; y<MAX_WORLD_Y_LOC ; y++ )
	{
		for( int x=0 ; x<MAX_WORLD_X_LOC ; x++ )
		{
			if( locPtr->has_unit(UNIT_LAND) )
			{
				err_when( unit_array.is_truly_deleted( locPtr->unit_recno(UNIT_LAND) ) );
			}

			locPtr++;
		}
	}

	#endif
}
//----------- End of function World::next_day ------------//


//----------- Begin of function World::detect ------------//
//
// Detect mouse action from user
//
// Return : 1 - mouse pressed on World area
//          0 - mouse not pressed on World area
//
int World::detect()
{
   if( map_matrix->detect() )
      return 1;

   if( zoom_matrix->detect() )
      return 1;

	if( detect_scroll() )
		return 1;

	// ##### begin Gilbert 16/9 #######//
	// return detect_firm_town();
	return 0;
	// ##### end Gilbert 16/9 #######//
}
//----------- End of function World::detect ------------//


//--------- Begin of function World::detect_scroll ---------//
//
// Detect if the mouse cursor is pushed towards the border
// of the screen to scroll the zoom window.
//
int World::detect_scroll()
{
   if( mouse_cursor.frame_flag )    // if it's now in frame selection mode
      return 0;

   if( next_scroll_time && m.get_time() < next_scroll_time )      // just scrolled not too long ago, wait for a little while before next scroll.
      return 0;

   int rc=0;

   //----- scroll left -----//

   if( mouse.cur_x == mouse.bound_x1 )
   {
      zoom_matrix->scroll(-1,0);
      rc = 1;
   }

   //---- scroll right -----//

   if( mouse.cur_x == mouse.bound_x2 )
   {
      zoom_matrix->scroll(1,0);
      rc = 1;
   }

   //---- scroll top -------//

   if( mouse.cur_y == mouse.bound_y1 )
   {
      zoom_matrix->scroll(0,-1);
      rc = 1;
   }

   //---- scroll bottom ----//

   if( mouse.cur_y == mouse.bound_y2 )
   {
      zoom_matrix->scroll(0,1);
      rc = 1;
   }

   //----- set next scroll time based on scroll_speed -----//
   //
   // slowest scroll speed: 500/1  = 500 milliseconds or 1/2 second
   // fastest scroll speed: 500/10 = 50  milliseconds or 1/20 second
   //
   //------------------------------------------------------//

   if( rc )
   {
      sys.zoom_need_redraw = 1;        // ask the zoom window to refresh next time
      next_scroll_time     = m.get_time() + 500/(config.scroll_speed+1);
   }

   return rc;
}
//----------- End of function World::detect_scroll -----------//


//--------- Begin of function World::go_loc --------//
//
// Go to a specified location.
//
// <int> xLoc, yLoc - location to go to.
// [int] selectFlag - whether should the object on the location if
//							 there is one. (default: 0)
//
void World::go_loc(int xLoc, int yLoc, int selectFlag)
{
	//------- set location ---------//

	zoom_matrix->cur_x_loc = xLoc;
	zoom_matrix->cur_y_loc = yLoc;

	map_matrix->cur_x_loc = xLoc - zoom_matrix->disp_x_loc/2;
	map_matrix->cur_y_loc = yLoc - zoom_matrix->disp_y_loc/2;

	//--------- refresh ------------//

	map_matrix->valid_cur_box();

	zoom_matrix->top_x_loc = map_matrix->cur_x_loc;
	zoom_matrix->top_y_loc = map_matrix->cur_y_loc;

	sys.zoom_need_redraw = 1;

	//---- if should select the object on the location ----//

	if( selectFlag )
	{
		Location* locPtr = world.get_loc(xLoc, yLoc);

		if( locPtr->has_any_unit() )
		{
			int mobileType;
			int unitRecno = locPtr->get_any_unit( mobileType );

			power.reset_selection();

			unit_array[unitRecno]->selected_flag = 1;
			unit_array.selected_recno = unitRecno;
			unit_array.selected_count++;
		}
		else if( locPtr->is_firm() )
		{
			power.reset_selection();
			firm_array.selected_recno = locPtr->firm_recno();
		}
		else if( locPtr->is_town() )
		{
			power.reset_selection();
			town_array.selected_recno = locPtr->town_recno();
		}
		else if( locPtr->has_site() )
		{
			power.reset_selection();
			site_array.selected_recno = locPtr->site_recno();
		}
	}

	//------- refresh the display -------//

	info.disp();
}
//----------- End of function World::go_loc --------//


//-------- Begin of function World::unveil ---------//
//
// Unveil all surrounding areas of the given object.
//
// <int> xLoc1, yLoc1 = the position of the object.
// <int> xLoc2, yLoc2 = the position of the object.
//
void World::unveil(int xLoc1, int yLoc1, int xLoc2, int yLoc2)
{
	if( config.explore_whole_map )
		return;

	xLoc1 = MAX( 0, xLoc1 - EXPLORE_RANGE);
	yLoc1 = MAX( 0, yLoc1 - EXPLORE_RANGE);
	xLoc2 = MIN( MAX_WORLD_X_LOC-1, xLoc2 + EXPLORE_RANGE);
	yLoc2 = MIN( MAX_WORLD_Y_LOC-1, yLoc2 + EXPLORE_RANGE);

	explore( xLoc1, yLoc1, xLoc2, yLoc2 );
}
//--------- End of function World::unveil ---------//


//-------- Begin of function World::explore ---------//
//
// Explore a specific area. No further exploration around the area.
//
// <int> xLoc1, yLoc1 = the position of the area.
// <int> xLoc2, yLoc2 = the position of the area.
//
void World::explore(int xLoc1, int yLoc1, int xLoc2, int yLoc2)
{
	if( config.explore_whole_map )
		return;

	int 		 xLoc, yLoc;
	Location* locPtr;
	char* 	 imageBuf = map_matrix->save_image_buf + sizeof(short)*2;
	char*     nationColorArray = nation_array.nation_power_color_array;
	char* 	 writePtr;

	int		shadowMapDist = max_x_loc + 1;
	int		tileYOffset;
	Location	*northWestPtr;
	char		tilePixel;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		locPtr = get_loc(xLoc1, yLoc);

		for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
		{
			if( !locPtr->explored() )
			{
				locPtr->explored_on();

				//-------- draw pixel ----------//

				writePtr = imageBuf+MAP_WIDTH*yLoc+xLoc;

				switch( world.map_matrix->map_mode )
				{
					case MAP_MODE_TERRAIN:
						if( locPtr->fire_str() > 0)
							*writePtr = (char) FIRE_COLOR;

						else if( locPtr->is_plant() )
							*writePtr = plant_res.plant_map_color;

						else
						{
							tileYOffset = (yLoc & TERRAIN_TILE_Y_MASK) * TERRAIN_TILE_WIDTH;

							tilePixel = terrain_res.get_map_tile(locPtr->terrain_id)[tileYOffset + (xLoc & TERRAIN_TILE_X_MASK)];

							if( xLoc == 0 || yLoc == 0)
							{
								*writePtr = tilePixel;
							}
							else
							{
								northWestPtr = locPtr - shadowMapDist;
								if( (terrain_res[locPtr->terrain_id]->average_type >=
									terrain_res[northWestPtr->terrain_id]->average_type) )
								{
									*writePtr = tilePixel;
								}
								else
								{
									*writePtr = (char) VGA_GRAY;
								}
							}
							break;
						}
						break;

					case MAP_MODE_SPOT:
						if( locPtr->sailable() )
							*writePtr = (char) 0x32;

						else if( locPtr->has_hill() )
							*writePtr = (char) V_BROWN;

						else if( locPtr->is_plant() )
							*writePtr = (char) V_DARK_GREEN;

						else
							*writePtr = (char) VGA_GRAY+10;
						break;

					case MAP_MODE_POWER:
						if( locPtr->sailable() )
							*writePtr = (char) 0x32;

						else if( locPtr->has_hill() )
							*writePtr = (char) V_BROWN;

						else if( locPtr->is_plant() )
							*writePtr = (char) V_DARK_GREEN;

						else
							*writePtr = nationColorArray[locPtr->power_nation_recno];
						break;
				}

				//---- if the command base of the opponent revealed, establish contact ----//

				if( locPtr->is_firm() )
				{
					Firm* firmPtr = firm_array[locPtr->firm_recno()];

					if( firmPtr->nation_recno > 0 && nation_array.player_recno )
					{
						NationRelation *relation = (~nation_array)->get_relation(firmPtr->nation_recno);

						if( !relation->has_contact )
						{
#ifdef USE_DPLAY
							if( !remote.is_enable() )
							{
#endif
								(~nation_array)->establish_contact(firmPtr->nation_recno);
#ifdef USE_DPLAY
							}
							else
							{
								if( !relation->contact_msg_flag )
								{
									// packet structure : <player nation> <explored nation>
									short *shortPtr = (short *)remote.new_send_queue_msg(MSG_NATION_CONTACT, 2*sizeof(short));
									*shortPtr = nation_array.player_recno;
									shortPtr[1] = firmPtr->nation_recno;
									relation->contact_msg_flag = 1;
								}
							}
#endif
						}
					}
				}

				if( locPtr->is_town() )
				{
					Town* townPtr = town_array[locPtr->town_recno()];

					if( townPtr->nation_recno > 0 && nation_array.player_recno )
					{
						NationRelation *relation = (~nation_array)->get_relation(townPtr->nation_recno);

						if( !relation->has_contact )
						{
#ifdef USE_DPLAY
							if( !remote.is_enable() )
							{
#endif
								(~nation_array)->establish_contact(townPtr->nation_recno);
#ifdef USE_DPLAY
							}
							else
							{
								if( !relation->contact_msg_flag )
								{
									// packet structure : <player nation> <explored nation>
									short *shortPtr = (short *)remote.new_send_queue_msg(MSG_NATION_CONTACT, 2*sizeof(short));
									*shortPtr = nation_array.player_recno;
									shortPtr[1] = townPtr->nation_recno;
									relation->contact_msg_flag = 1;
								}
							}
#endif
						}
					}
				}
			}
		}
	}
}
//--------- End of function World::explore ---------//


//-------- Begin of function World::is_explored ---------//
//
// Check if the whole area has been explored or not.
//
// <int> xLoc1, yLoc1 = the coordination of the area to explore
// <int> xLoc2, yLoc2 = the coordination of the area to explore
//
int World::is_explored(int xLoc1, int yLoc1, int xLoc2, int yLoc2)
{
	if( config.explore_whole_map )
      return 1;

   int xLoc, yLoc;
   Location* locPtr;

   for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
   {
      locPtr = get_loc(xLoc1, yLoc);

      for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
      {
         if( !locPtr->explored() )
            return 0;
      }
   }

   return 1;
}
//--------- End of function World::is_explored ---------//


//----------- Begin of function World::load_map ------------//
//
// Load a custom map file.
//
void World::load_map(char* fileName)
{
   generate_map();

   return;

   //---------- initialize the map matrix --------//

   max_x_loc = 200;
   max_y_loc = 200;

   loc_matrix = (Location*) mem_resize( loc_matrix  , max_x_loc * max_y_loc * sizeof(Location) );

   memset( loc_matrix, 0, sizeof(Location) * max_x_loc * max_y_loc );

   int baseType = TERRAIN_DARK_DIRT;

   int terrainId    = terrain_res.scan(baseType, MIDDLE_MASK, baseType, MIDDLE_MASK, 
					  		 baseType, MIDDLE_MASK, baseType, MIDDLE_MASK, 1);     // 1-get the first instance

   for( int i=0 ; i<max_x_loc*max_y_loc ; i++ )
   {
      loc_matrix[i].terrain_id = terrainId+m.random(3);
   }

   assign_map();

   return;

   //---------- initialize the map matrix --------//

   max_x_loc = 120;
   max_y_loc = 120;

   loc_matrix = (Location*) mem_resize( loc_matrix  , max_x_loc * max_y_loc * sizeof(Location) );

   //-------------- read in the map -------------//

   File mapFile;

   mapFile.file_open(fileName);

   mapFile.file_read(loc_matrix, sizeof(Location)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC );

   mapFile.file_close();

   //----- assign the map to MapMatrix and ZoomMatrix -----//

   assign_map();
}
//----------- End of function World::load_map ------------//


//-------- Begin of function World::check_unit_space ------//
//
// To check whether or not the area bounded by the upper left
// corner (xLoc1, yLoc1) and lower right corner (xLoc2, yLoc2)
// can be built.
//
// <int>  xLoc1          = the upper left x
// <int>  yLoc1          = the upper left y
// <int>  xLoc2          = the lower right x
// <int>  yLoc2          = the lower right y
// [int]  mobileType		 = mobile type (default: UNIT_LAND)
// [int]  buildFlag 		 = whether the located area is for building a firm/town
//									if so, the location must no have any raw site.
//									(default: 0)
//
// return 1 for true. 0 for false
//
inline int World::check_unit_space(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int mobileType, int buildFlag)
{
	if(xLoc1<0 || xLoc1>=MAX_WORLD_X_LOC)
		return 0;
	if(yLoc1<0 || yLoc1>=MAX_WORLD_Y_LOC)
		return 0;
	if(xLoc2<0 || xLoc2>=MAX_WORLD_X_LOC)
		return 0;
	if(yLoc2<0 || yLoc2>=MAX_WORLD_Y_LOC)
		return 0;

	Location* locPtr = world.get_loc(xLoc1, yLoc1);
	int x, y;
	int canBuildFlag = 1;

	for(y=yLoc1; y<=yLoc2; y++)
	{
		locPtr = world.get_loc(xLoc1, y);

		for(x=xLoc1; x<=xLoc2; x++, locPtr++)
		{
			if( !locPtr->can_move(mobileType) ||
				 ( buildFlag && (locPtr->is_power_off() || locPtr->has_site()) ) ) 		// if build a firm/town, there must not be any sites in the area
			{
				canBuildFlag=0;
				break;
			}
		}

		if(canBuildFlag==0)
			break;
	}

	if( canBuildFlag )
		return 1;
	else
		return 0;
}
//-------- End of function World::check_unit_space ------//


//-------- Begin of function World::locate_space ------//
//
// Locate an area in the world map around the firm to place
// the unit
//
// <int&> xLoc1          = the upper left x location of the building, also for returning the result location
// <int&> yLoc1          = the upper left y location of the building
// <int>  xLoc2          = the lower right x location of the building
// <int>  yLoc2          = the lower right y location of the building
// <int>  spaceLocWidth  = the location width of the required space
// <int>  spaceLocHeight = the location height of the required space
// [int]  mobileType		 = mobile type (default: UNIT_LAND)
// [int]  regionId		 = specify the region no. of the location to locate
//									(default: region no. of xLoc1, yLoc1)
// [int]  buildFlag 		 = whether the located area is for building a firm/town
//									if so, the location must no have any raw site.
//									(default: 0)
//
// return : <int> 1 - free space found
//                0 - free space not found
//
int World::locate_space(int& xLoc1, int& yLoc1, int xLoc2, int yLoc2,
								int spaceLocWidth, int spaceLocHeight, int mobileType, int regionId, int buildFlag)
{
	if( !regionId )
		regionId = get_loc(xLoc1, yLoc1)->region_id;

	int isPlateau = get_loc(xLoc1, yLoc1)->is_plateau();

	//-----------------------------------------------------------//
	// xLoc, yLoc is the adjusted upper left corner location of
	// the firm. with the adjustment, it is easier to do the following
	// checking.
	//-----------------------------------------------------------//

	Location* locPtr;
	int xLoc = xLoc1 - spaceLocWidth + 1;
	int yLoc = yLoc1 - spaceLocHeight + 1;

	if(xLoc < 0)
		xLoc = 0;
	if(yLoc < 0)
		yLoc = 0;

	int width   = xLoc2 - xLoc + 1;
	int height  = yLoc2 - yLoc + 1;
	int loopCount=0;

	while(1)
	{
		err_when( ++loopCount > MAX_WORLD_X_LOC * MAX_WORLD_Y_LOC * 4 );

		//-----------------------------------------------------------//
		// step 1
		//-----------------------------------------------------------//
		int xOffset = width/2;
		int yOffset = height;
		int x, y;

		x = xLoc + xOffset;
		y = yLoc + yOffset;

		if(x>=0 && y>=0 && x+spaceLocWidth-1<MAX_WORLD_X_LOC && y+spaceLocHeight-1<MAX_WORLD_Y_LOC)
		{
			if(mobileType==UNIT_LAND || (x%2==0 && y%2==0))
			{
				locPtr = get_loc(x,y);

				if( locPtr->region_id == regionId &&
					 locPtr->is_plateau() == isPlateau && 
					 check_unit_space(x, y, x+spaceLocWidth-1, y+spaceLocHeight-1, mobileType, buildFlag))
				{
					xLoc1 = x;
					yLoc1 = y;
					return 1;
				}
			}
		}

		int sign = -1;
		int i, j, k, limit;

		//-----------------------------------------------------------//
		// step 2
		//-----------------------------------------------------------//
		//y = yLoc + yOffset;
		limit = width + 2;
		for(i=1; i<limit; i++)
		{
			xOffset += sign * i;
			x = xLoc + xOffset;

			if(x>=0 && y>=0 && x+spaceLocWidth-1<MAX_WORLD_X_LOC && y+spaceLocHeight-1<MAX_WORLD_Y_LOC)
			{
				if(mobileType==UNIT_LAND || (x%2==0 && y%2==0))
				{
					locPtr = get_loc(x,y);

					if( locPtr->region_id == regionId &&
						 locPtr->is_plateau() == isPlateau &&
						 check_unit_space(x, y, x+spaceLocWidth-1, y+spaceLocHeight-1, mobileType, buildFlag))
					{
						xLoc1 = x;
						yLoc1 = y;
						return 1;
					}
				}
			}

			sign *= -1;
		}

		//-----------------------------------------------------------//
		// step 3
		//-----------------------------------------------------------//
		i = limit-1;

		limit = (height+1)*2;
		int r = sign*i;
		int lastX = xOffset;
		//int lastY = yOffset;

		for(j=0; j<limit; j++)
		{
			if(j%2)
			{
				//x = xLoc + lastX;
				xOffset = lastX;
				x = xLoc + xOffset;
				//y = yLoc + yOffset;

				if(x>=0 && y>=0 && x+spaceLocWidth-1<MAX_WORLD_X_LOC && y+spaceLocHeight-1<MAX_WORLD_Y_LOC)
				{
					if(mobileType==UNIT_LAND || (x%2==0 && y%2==0))
					{
						locPtr = get_loc(x,y);

						if( locPtr->region_id == regionId &&
							 locPtr->is_plateau() == isPlateau &&
							 check_unit_space(x, y, x+spaceLocWidth-1, y+spaceLocHeight-1, mobileType, buildFlag))
						{
							xLoc1 = x;
							yLoc1 = y;
							return 1;
						}
					}
				}
			}
			else
			{
				xOffset = lastX + r;
				yOffset--;

				x = xLoc + xOffset;
				y = yLoc + yOffset;

				if(x>=0 && y>=0 && x+spaceLocWidth-1<MAX_WORLD_X_LOC && y+spaceLocHeight-1<MAX_WORLD_Y_LOC)
				{
					if(mobileType==UNIT_LAND || (x%2==0 && y%2==0))
					{
						locPtr = get_loc(x,y);

						if( locPtr->region_id == regionId &&
							 locPtr->is_plateau() == isPlateau &&
							 check_unit_space(x, y, x+spaceLocWidth-1, y+spaceLocHeight-1, mobileType, buildFlag))
						{
							xLoc1 = x;
							yLoc1 = y;
							return 1;
						}
					}
				}
			}
		}

		//-----------------------------------------------------------//
		// step 4
		//-----------------------------------------------------------//
		y = yLoc + yOffset;
		for(k=0; k<=width; k++)
		{
			sign *= -1;
			i--;
			r = sign*i;
			xOffset -= r;

			x = xLoc + xOffset;

			if(x>=0 && y>=0 && x+spaceLocWidth-1<MAX_WORLD_X_LOC && y+spaceLocHeight-1<MAX_WORLD_Y_LOC)
			{
				if(mobileType==UNIT_LAND || (x%2==0 && y%2==0))
				{
					locPtr = get_loc(x,y);

					if( locPtr->region_id == regionId &&
						 locPtr->is_plateau() == isPlateau &&
						 check_unit_space(x, y, x+spaceLocWidth-1, y+spaceLocHeight-1, mobileType, buildFlag))
					{
						xLoc1 = x;
						yLoc1 = y;
						return 1;
					}
				}
			}
		}

		//-----------------------------------------------------------//
		// re-init the parameters
		//-----------------------------------------------------------//
		if(xLoc<=0 && yLoc<=0 && width>=MAX_WORLD_X_LOC && height>=MAX_WORLD_Y_LOC)
			break;   // the whole map has been checked

		width += 2;
		height += 2;

		xLoc -= 1;
		yLoc -= 1;
		if(xLoc<0)
		{
			xLoc = 0;
			width--;
		}
		if(yLoc<0)
		{
			yLoc=0;
			height--;
		}

		if(xLoc+width>MAX_WORLD_X_LOC)
			width--;
		if(yLoc+height>MAX_WORLD_Y_LOC)
			height--;

		//if(width==xLoc2-xLoc1+spaceLocWidth && height==yLoc2-yLoc1+spaceLocHeight) // terminate the checking
		// return 0;
	}

	return 0;
}
//-------- End of function World::locate_space ------//


//-------- Begin of function World::locate_space_random ------//
//
// Locate an area of space in the world map randomly. Pick any
// space available in that area without a specific scanning order.
//
// <int&> xLoc1          = the scaning range, also for returning the result location
// <int&> yLoc1          = the scaning range
// <int>  xLoc2          = the scaning range
// <int>  yLoc2          = the scaning range
// <int>  spaceLocWidth  = the location width of the required space
// <int>  spaceLocHeight = the location height of the required space
// <int>  maxTries       = maximum no. of tries
// [int]  regionId		 = if this is specified, the result location will
//									be in this region.
// [int]  buildSite      = whether locating space for building a site
//                         (default: 0)
// [char] teraMask		 = terrain mask (default: 1)
//
// return : <int> 1 - free space found
//                0 - free space found
//
int World::locate_space_random(int& xLoc1, int& yLoc1, int xLoc2, int yLoc2,
								int spaceLocWidth, int spaceLocHeight, int maxTries,
								int regionId, int buildSite, char teraMask)
{
   int       i, x, y, xTemp, xLoc, yLoc, canBuildFlag;
   int       scanWidth  = xLoc2-xLoc1-spaceLocWidth+2; //xLoc2-xLoc1+1-spaceLocWidth+1;
   int       scanHeight = yLoc2-yLoc1-spaceLocHeight+2; //yLoc2-yLoc1+1-spaceLocHeight+1;
	Location* locPtr;

   for( i=0 ; i<maxTries ; i++ )
   {
      xLoc = xLoc1 + m.random(scanWidth);
      yLoc = yLoc1 + m.random(scanHeight);
		canBuildFlag=1;

		//---------- check if the area is all free ----------//

		xTemp = xLoc+spaceLocWidth-1;

		for( y=yLoc+spaceLocHeight-1; y>=yLoc; y-- )
		{
			locPtr = world.get_loc(xTemp, y);

			for(x=xTemp; x>=xLoc; x--, locPtr-- )
			{
				if( ( buildSite ? !locPtr->can_build_site(teraMask) : !locPtr->can_build_firm(teraMask) ) ||
					 locPtr->is_power_off() )
				{
					canBuildFlag=0;
					break;
				}
			}

			if(!canBuildFlag)
				break;
		}

		if( !canBuildFlag )
			continue;

		//------ check region id. ------------//

		locPtr = world.get_loc(xLoc, yLoc);

		if( regionId && locPtr->region_id != regionId )
			continue;

		//------------------------------------//

		xLoc1 = xLoc;
		yLoc1 = yLoc;

		err_when(buildSite && !locPtr->can_build_site(teraMask));//-*** hard codes for mine size 3x3
		return 1;
	}

	return 0;
}
//-------- End of function World::locate_space_random ------//


//-------- Begin of function World::can_build_firm ---------//
//
// Check if it is free to construct a building on the specific area.
//
// <int>   xLoc1, yLoc1 = the coordination of the area to can_build
// <int>   firmId       = id. of the firm
// [short] unitRecno    = the unit recno of the unit to build the firm
//								  if the builder unit stands on the building area, still consider the area as buildable
//								  (default: -1, do not take the builder into account)
//
int World::can_build_firm(int xLoc1, int yLoc1, int firmId, short unitRecno)
{
	if( xLoc1 < 0 || yLoc1 < 0 || xLoc1 > MAX_WORLD_X_LOC || yLoc1 > MAX_WORLD_Y_LOC )	
		return 0;		

	//------------------------------------------//

	FirmInfo* firmInfo = firm_res[firmId];

	int xLoc, yLoc;
	int xLoc2 = xLoc1 + firmInfo->loc_width - 1;
	int yLoc2 = yLoc1 + firmInfo->loc_height - 1;
	if(xLoc2>=max_x_loc || yLoc2>max_y_loc)
		return 0;

	Location* locPtr;
	char teraMask, pierFlag;

	switch(firmInfo->tera_type)
	{
	case 1:		// default : land firm
	case 2:		// sea firm
	case 3:		// land or sea firm
		teraMask = firmInfo->tera_type;
		for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
		{
			locPtr = get_loc(xLoc1, yLoc);

			for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
			{
				// ##### patch begin Gilbert 14/3 ######//
				if(!locPtr->can_build_firm(teraMask) && 
					(!locPtr->has_unit(UNIT_LAND) || locPtr->unit_recno(UNIT_LAND)!=unitRecno))
					return 0;
				// ##### patch end Gilbert 14/3 ######//

				if( firmId != FIRM_MINE && locPtr->has_site() )		// don't allow building any buildings other than mines on a location with a site
					return 0;
			}
		}
		return 1;

	case 4:				// special firm, such as harbor
		// must be 3x3,
		// centre square of one side is land (teraMask=1),
		// two squares on that side can be land or sea (teraMask=3)
		// and other (6 squares) are sea (teraMask=2)
		if( firmInfo->loc_width != 3 ||
			firmInfo->loc_height != 3)
			return 0;

		pierFlag = 1|2|4|8;		// bit0=north, bit1=south, bit2=west, bit3=east
		for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
		{
			locPtr = get_loc(xLoc1, yLoc);

			for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
			{
				if( locPtr->has_site() )		// don't allow building any buildings other than mines on a location with a site
					return 0;

static char northPierTera[3][3] = { {2,2,2},{2,2,2},{3,1,3} };
static char southPierTera[3][3] = { {3,1,3},{2,2,2},{2,2,2} };
static char westPierTera[3][3] = { {2,2,3},{2,2,1},{2,2,3} };
static char eastPierTera[3][3] = { {3,2,2},{1,2,2},{3,2,2} };
				int x = xLoc - xLoc1;
				int y = yLoc - yLoc1;
				if(!locPtr->can_build_harbor(northPierTera[y][x]) )
					pierFlag &= ~1;
				if(!locPtr->can_build_harbor(southPierTera[y][x]) )
					pierFlag &= ~2;
				if(!locPtr->can_build_harbor(westPierTera[y][x]) )
					pierFlag &= ~4;
				if(!locPtr->can_build_harbor(eastPierTera[y][x]) )
					pierFlag &= ~8;
			}
		}
		err_when( pierFlag != 0 && pierFlag != 1 && pierFlag != 2 &&
			pierFlag != 4 && pierFlag != 8 );
		return pierFlag;
		break;

	// other tera_type here

	default:
		err_here();
		return 0;
	}
}
//--------- End of function World::can_build_firm ---------//


//-------- Begin of function World::can_build_town ---------//
//
// <int>   xLoc1, yLoc1 = the coordination of the area to can_build
// [short] unitRecno    = the unit recno of the unit to build the town
//								  if the builder unit stands on the building area, still consider the area as buildable
//								  (default: -1, do not take the builder into account)
//
int World::can_build_town(int xLoc1, int yLoc1, short unitRecno)
{
	int xLoc, yLoc;
	int xLoc2 = xLoc1 + STD_TOWN_LOC_WIDTH  - 1;
	int yLoc2 = yLoc1 + STD_TOWN_LOC_HEIGHT - 1;

	if(xLoc2>=max_x_loc || yLoc2>=max_y_loc)
		return 0;

	Location* locPtr;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		locPtr = get_loc(xLoc1, yLoc);

		for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
		{
			// ##### patch begin Gilbert 14/3 ######//
			// allow the building unit to stand in the area
			if( !locPtr->can_build_town() && 
				(!locPtr->has_unit(UNIT_LAND) || locPtr->unit_recno(UNIT_LAND)!=unitRecno) )
				return 0;
			// ##### patch end Gilbert 14/3 ######//
		}
	}

	return 1;
}
//--------- End of function World::can_build_town ---------//


//-------- Begin of function World::can_build_wall ---------//
//
// <int> xLoc, yLoc  = the coordination of the area to can_build
// <int> nationRecno = recno of the builder nation.
//
int World::can_build_wall(int xLoc, int yLoc, short nationRecno)
{
	Location* locPtr = get_loc(xLoc, yLoc);

	return locPtr->can_build_wall() && locPtr->power_nation_recno == nationRecno;
}
//--------- End of function World::can_build_wall ---------//


//-------- Begin of function World::can_destruct_wall ---------//
//
// <int> xLoc, yLoc  = the coordination of the area to can_build
// <int> nationRecno = recno of the builder nation.
//
int World::can_destruct_wall(int xLoc, int yLoc, short nationRecno)
{
	Location* locPtr = get_loc(xLoc, yLoc);

	return locPtr->is_wall() && locPtr->power_nation_recno == nationRecno;
}
//--------- End of function World::can_destruct_wall ---------//


//---------- Begin of function World::draw_link_line -----------//
//
// <int> srcFirmId          - id. of the source firm.
//                            0 if the source is a town
// <int> srcTownRecno       - town recno of the source town
//                            0 if the source is a firm
// <int> srcXLoc1, srcYLoc1 - the location of the source area
//       srcXLoc2, srcYLoc2
//
// [int] giveEffectiveDis   - use this value as the effective distance if this is given
//										(default: 0)
//
void World::draw_link_line(int srcFirmId, int srcTownRecno, int srcXLoc1,
									int srcYLoc1, int srcXLoc2, int srcYLoc2, int givenEffectiveDis)
{
	if( srcFirmId == FIRM_INN ) 	// FirmInn's link is only for scan for neighbor inns quickly, the link line is not displayed
		return;

	//--------------------------------------//

	int srcXLoc = (srcXLoc1 + srcXLoc2)/2;
	int srcYLoc = (srcYLoc1 + srcYLoc2)/2;

	int srcX = ( ZOOM_X1 + (srcXLoc1-zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (srcXLoc2-zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

	int srcY = ( ZOOM_Y1 + (srcYLoc1-zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (srcYLoc2-zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

	//------- draw lines connected to town ---------//

	int   i, townX, townY, effectiveDis;
	Town* townPtr;

	if( givenEffectiveDis )
		effectiveDis = givenEffectiveDis;
	else
	{
		if( srcFirmId )
			effectiveDis = EFFECTIVE_FIRM_TOWN_DISTANCE;
		else
			effectiveDis = EFFECTIVE_TOWN_TOWN_DISTANCE;
	}

	if( !srcFirmId || firm_res[srcFirmId]->is_linkable_to_town )    // don't draw link line to town if it's an inn
	{
		for( i=town_array.size() ; i>0 ; i-- )
		{
			if( town_array.is_deleted(i) )
				continue;

			townPtr = town_array[i];

			if( srcTownRecno && townPtr->town_recno != srcTownRecno )
				continue;

			//-------- check the distance --------//

			if( m.points_distance( townPtr->center_x, townPtr->center_y,
				 srcXLoc, srcYLoc ) > effectiveDis )
			{
				continue;
			}

			//------ check if both are on the same terrain type ------//

			if( (world.get_loc(townPtr->center_x, townPtr->center_y)->is_plateau()==1)
				 != (world.get_loc(srcXLoc, srcYLoc)->is_plateau()==1) )
			{
				continue;
			}

			//---------- draw line now -----------//

			townX = ( ZOOM_X1 + (townPtr->loc_x1-zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
					  + ZOOM_X1 + (townPtr->loc_x2-zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

			townY = ( ZOOM_Y1 + (townPtr->loc_y1-zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
					  + ZOOM_Y1 + (townPtr->loc_y2-zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

			anim_line.draw_line(&vga_back, srcX, srcY, townX, townY);
		}
	}

	//------- draw lines connected to firms ---------//

	if( givenEffectiveDis )
		effectiveDis = givenEffectiveDis;
	else
	{
		if( srcFirmId )
			effectiveDis = EFFECTIVE_FIRM_FIRM_DISTANCE;
		else
			effectiveDis = EFFECTIVE_FIRM_TOWN_DISTANCE;
	}

	int   firmX, firmY, linkFlag;
	Firm* firmPtr;

	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
         continue;

		firmPtr = firm_array[i];

		//------ only link if the firms have relationship -----//

		if( srcFirmId )
			linkFlag = firm_res[firmPtr->firm_id]->is_linkable_to_firm(srcFirmId);
		else
			linkFlag = firm_res[firmPtr->firm_id]->is_linkable_to_town;

		if( !linkFlag )
			continue;

		//-------- check the distance --------//

		if( m.points_distance( firmPtr->center_x, firmPtr->center_y,
			 srcXLoc, srcYLoc ) > effectiveDis )
		{
			continue;
		}

		//------ check if both are on the same terrain type ------//

		if( (world.get_loc(firmPtr->center_x, firmPtr->center_y)->is_plateau()==1)
			 != (world.get_loc(srcXLoc, srcYLoc)->is_plateau()==1) )
		{
			continue;
		}

		//---------- draw line now -----------//

		firmX = ( ZOOM_X1 + (firmPtr->loc_x1-zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (firmPtr->loc_x2-zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

		firmY = ( ZOOM_Y1 + (firmPtr->loc_y1-zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (firmPtr->loc_y2-zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

		anim_line.draw_line(&vga_back, srcX, srcY, firmX, firmY);
	}
}
//----------- End of function World::draw_link_line ------------//


//-------- Begin of function World::set_surr_power_off ---------//
void World::set_surr_power_off(int xLoc, int yLoc)
{
	if(xLoc>0) // west
		get_loc(xLoc-1, yLoc)->set_power_off();

	if(xLoc<max_x_loc-1)
		get_loc(xLoc+1, yLoc)->set_power_off();
	
	if(yLoc>0) // north
		get_loc(xLoc, yLoc-1)->set_power_off();

	if(yLoc<max_y_loc-1) // south
		get_loc(xLoc, yLoc+1)->set_power_off();
}
//----------- End of function World::set_surr_power_off ------------//


//-------- Begin of function World::set_all_power ---------//
//
void World::set_all_power()
{
	//--------- set town's influence -----------//

	Town* townPtr;
	int i;

	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( !townPtr->nation_recno )
			continue;

		//------- set the influence range of this town -----//

		set_power(townPtr->loc_x1, townPtr->loc_y1, townPtr->loc_x2, townPtr->loc_y2, (char)townPtr->nation_recno);
	}

	//--------- set firm's influence -----------//

	Firm* firmPtr;

	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( !firmPtr->nation_recno )
			continue;

		if( !firmPtr->should_set_power )
			continue;

		//------- set the influence range of this firm -----//

		set_power(firmPtr->loc_x1, firmPtr->loc_y1, firmPtr->loc_x2, firmPtr->loc_y2, (char)firmPtr->nation_recno);
	}
}
//--------- End of function World::set_all_power ---------//


//-------- Begin of function World::set_power ---------//
//
// <int> xLoc1, yLoc1, - area on the map which the power should be set
//		   xLoc2, yLoc2
//
// <int> nationRcno - nation recno
//
void World::set_power(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int nationRecno)
{
	//------- reset power_nation_recno first ------//

	int	plateauResult = (get_loc((xLoc1+xLoc2)/2, (yLoc1+yLoc2)/2)->is_plateau()==1);

	int   	 xLoc, yLoc, centerY, t;
	Location* locPtr = loc_matrix;

	xLoc1 = MAX( 0, xLoc1 - EFFECTIVE_POWER_DISTANCE+1);
	yLoc1 = MAX( 0, yLoc1 - EFFECTIVE_POWER_DISTANCE+1);
	xLoc2 = MIN( MAX_WORLD_X_LOC-1, xLoc2 + EFFECTIVE_POWER_DISTANCE-1);
	yLoc2 = MIN( MAX_WORLD_Y_LOC-1, yLoc2 + EFFECTIVE_POWER_DISTANCE-1);

	centerY = (yLoc1+yLoc2) / 2;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		t=abs(yLoc-centerY)/2;

		for( xLoc=xLoc1+t ; xLoc<=xLoc2-t ; xLoc++, locPtr++ )
		{
			locPtr = get_loc(xLoc, yLoc);

			if(locPtr->sailable())//if(!locPtr->walkable())
				continue;

			if(locPtr->is_power_off())
				continue;

			if((locPtr->is_plateau()==1) != plateauResult)
				continue;

			if(locPtr->power_nation_recno==0)
			{
				locPtr->power_nation_recno = nationRecno;
				sys.map_need_redraw = 1;						// request redrawing the map next time
			}
		}
	}
}
//--------- End of function World::set_power ---------//


//-------- Begin of function World::restore_power ---------//
//
// <int> xLoc1, yLoc1, - area on the map which the power should be restored
//		   xLoc2, yLoc2
//
// <int> townRecno, firmRecno - either one
//
void World::restore_power(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int townRecno, int firmRecno)
{
	int nationRecno;

	if( townRecno )
	{
		nationRecno = town_array[townRecno]->nation_recno;
		town_array[townRecno]->nation_recno = 0;
	}

	if( firmRecno )
	{
		nationRecno = firm_array[firmRecno]->nation_recno;
		firm_array[firmRecno]->nation_recno = 0;
	}

	//------- reset power_nation_recno first ------//

	int   	 xLoc, yLoc, centerY, t;
	Location* locPtr = loc_matrix;

	xLoc1 = MAX( 0, xLoc1 - EFFECTIVE_POWER_DISTANCE+1);
	yLoc1 = MAX( 0, yLoc1 - EFFECTIVE_POWER_DISTANCE+1);
	xLoc2 = MIN( MAX_WORLD_X_LOC-1, xLoc2 + EFFECTIVE_POWER_DISTANCE-1);
	yLoc2 = MIN( MAX_WORLD_Y_LOC-1, yLoc2 + EFFECTIVE_POWER_DISTANCE-1);

	centerY = (yLoc1+yLoc2) / 2;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		t=abs(yLoc-centerY)/2;

		for( xLoc=xLoc1+t ; xLoc<=xLoc2-t ; xLoc++, locPtr++ )
		{
			locPtr = get_loc(xLoc, yLoc);

			if( locPtr->power_nation_recno==nationRecno )
			{
				locPtr->power_nation_recno = 0;
				sys.map_need_redraw = 1;						// request redrawing the map next time
			}
		}
	}

	//--- if some power areas are freed up, see if neighbor towns/firms should take up these power areas ----//

	if( sys.map_need_redraw )	// when calls set_all_power(), the nation_recno of the calling firm must be reset
		set_all_power();

	//------- restore the nation recno of the calling town/firm -------//

	if( townRecno )
		town_array[townRecno]->nation_recno = nationRecno;

	if( firmRecno )
		firm_array[firmRecno]->nation_recno = nationRecno;
}
//--------- End of function World::restore_power ---------//


//-------- Begin of function World::detect_firm_town ---------//
//
int World::detect_firm_town()
{
	// ##### begin Gilbert 19/9 ########//
	// int rc = mouse.single_click(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2, 2);
	if( !mouse.any_click(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2, 2) )
		return 0;
	// ##### end Gilbert 19/9 ########//

	//------ detect pressing on link enable/disable sign -----//

	Firm* firmPtr;

	if( firm_array.selected_recno )
	{
		firmPtr = firm_array[firm_array.selected_recno];

		if( firmPtr->should_show_info() &&				// only if should_show_info() is 1, we can detect links from this firm (it is not limited to player firms, as firms with player's workers should be allowed for resigning the player worker from the firm
			 firmPtr->draw_detect_link_line(1) )		// 1-detect action
		{
			return 1;
		}
	}

	Town* townPtr;

	if( town_array.selected_recno )
	{
		townPtr = town_array[town_array.selected_recno];

		if( townPtr->nation_recno==nation_array.player_recno &&
			 townPtr->draw_detect_link_line(1) )		// 1-detect action
		{
			return 1;
		}
	}

	// ####### begin Gilbert 12/9 #########// see Power::detect_select
/*
	//--------------- detect firm ------------------//

	if( rc==1 )			// left click 
	{
		int mouseAbsX = mouse.cur_x - ZOOM_X1 + World::view_top_x;
		int mouseAbsY = mouse.cur_y - ZOOM_Y1 + World::view_top_y;

		int  i;

		for( i=firm_array.size() ; i>0 ; i-- )
		{
			if( firm_array.is_deleted(i) )
				continue;

			firmPtr = firm_array[i];

			if( m.is_touch( mouseAbsX, mouseAbsY, mouseAbsX, mouseAbsY,
				firmPtr->abs_x1, firmPtr->abs_y1, firmPtr->abs_x2, firmPtr->abs_y2 ) )
			{
				power.reset_selection();
				firm_array.selected_recno = i;
				info.disp();

				// -------- sound effect -----------//

				if( firmPtr->nation_recno == nation_array.player_recno && se_res.mark_select_object_time() )
				{
					se_res.sound(firmPtr->center_x, firmPtr->center_y, 1,
						'F', firmPtr->firm_id, firmPtr->under_construction ? "SELU" : "SEL" );
				}
				return 1;
			}
		}

		//----------- detect town section --------------//

		for( i=town_array.size() ; i>0 ; i-- )
		{
			if( town_array.is_deleted(i) )
				continue;

			townPtr = town_array[i];

			if( m.is_touch( mouseAbsX, mouseAbsY, mouseAbsX, mouseAbsY,
				 townPtr->abs_x1, townPtr->abs_y1, townPtr->abs_x2, townPtr->abs_y2 ) )
			{
				power.reset_selection();
				town_array.selected_recno = i;
				info.disp();

				// -------- sound effect -----------//

				if( townPtr->nation_recno == nation_array.player_recno 
					&& se_res.mark_select_object_time() )
				{
					se_res.sound(townPtr->center_x, townPtr->center_y, 1,
						'T', 0, "SEL" );
				}
				return 1;
			}
		}
	}
	*/

	return 0;
}
//-------- End of function World::detect_firm_town ---------//


//-------- Begin of function World::earth_equake ------//
void World::earth_quake()
{
	Location *locPtr;
	int x,y;
	for(y = 0; y < max_y_loc; ++y)
	{
		locPtr = get_loc(0,y);
		for( x = 0; x < max_x_loc; ++x, ++locPtr)
		{
			if(locPtr->is_wall() )
			{
				locPtr->attack_wall( weather.quake_rate(x,y) /2 );
			}
		}
	}

	int firmDamage = 0;
	int firmDie = 0;
	int i;
	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) || !firm_res[firm_array[i]->firm_id]->buildable)
			continue;

		Firm *firmPtr = firm_array[i];
		x = firmPtr->center_x;
		y = firmPtr->center_y;
		firmPtr->hit_points -= weather.quake_rate(x,y);
		if( firmPtr->own_firm() )
			firmDamage++;
		if( firmPtr->hit_points <= 0)
		{
			firmPtr->hit_points = (float) 0;
			if( firmPtr->own_firm() )
				firmDie++;
			se_res.sound(firmPtr->center_x, firmPtr->center_y, 1,
				'F', firmPtr->firm_id, "DIE" );
			firm_array.del_firm(i);
		}
	}

	int townDamage = 0;
	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		Town *townPtr = town_array[i];
		int ownTown = townPtr->nation_recno == nation_array.player_recno;
		short beforePopulation = townPtr->population;
		short causalty = weather.quake_rate(townPtr->center_x, townPtr->center_y) / 10;
		for( ; causalty > 0 && !town_array.is_deleted(i); --causalty )
		{
			townPtr->kill_town_people(0);
		}
		if( town_array.is_deleted(i) )
			causalty = beforePopulation;
		else
			causalty = beforePopulation - townPtr->population;

		if(ownTown)
			townDamage += causalty;
	}

	int unitDamage = 0;
	int unitDie = 0;
	for( i=unit_array.size(); i > 0; i-- )
	{
		if( unit_array.is_deleted(i))
			continue;

		Unit *unitPtr = unit_array[i];

		// ###### begin Gilbert 30/8 ######//
		// no damage to air unit , sea unit or overseer
		if( !unitPtr->is_visible() || unitPtr->mobile_type == UNIT_AIR
			|| unitPtr->mobile_type == UNIT_SEA)
			continue;
		// ###### end Gilbert 30/8 ######//

		float damage = (float) weather.quake_rate(unitPtr->cur_x_loc(),unitPtr->cur_y_loc() ) *
			unitPtr->max_hit_points / 200;
		if( damage >= unitPtr->hit_points)
			damage = unitPtr->hit_points -1;
		if( damage < (float) 5)
			damage = (float) 5;

		unitPtr->hit_points -= damage;
		if( unitPtr->is_own() )
			unitDamage++;

		if( unitPtr->hit_points <= 0)
		{
			unitPtr->hit_points = (float) 0;
			if( unitPtr->is_own() )
				unitDie++;
		}
		else
		{
			if( unit_res[unitPtr->unit_id]->solider_id &&
				weather.quake_rate(unitPtr->cur_x_loc(),unitPtr->cur_y_loc()) >= 60)
			{
				((UnitVehicle *)unitPtr)->dismount();
			}
		}
	}

	news_array.earthquake_damage(unitDamage-unitDie, unitDie, townDamage, firmDamage-firmDie, firmDie);
}
//-------- End of function World::earth_equake ------//


//-------- Begin of function World::lightning_strike ------//
void World::lightning_strike(short cx, short cy, short radius)
{
	short x, y;
	for( y = cy-radius; y <= cy+radius; ++y)
	{
		if( y < 0 || y >= max_y_loc)
			continue;

		for( x = cx-radius; x <= cx+radius; ++x)
		{
			if( x < 0 || x >= max_x_loc)
				continue;

			Location *locPtr = get_loc(x,y);
			if( locPtr->is_plant() )
			{
				// ---- add a fire on it ------//
				locPtr->set_fire_str(80);
				// ##### begin Gilbert 11/8 #####//
				if( locPtr->can_set_fire() && locPtr->fire_str() < 5 )
					locPtr->set_fire_str(5);
				// ##### end Gilbert 11/8 #####//
			}
		}
	}

	// ------ check hitting units -------//
	int i;
	for( i=unit_array.size(); i > 0; i-- )
	{
		if( unit_array.is_deleted(i))
			continue;

		Unit *unitPtr = unit_array[i];

		// no damage to overseer
		if( !unitPtr->is_visible())
			continue;

		if( unitPtr->cur_x_loc() <= cx+ radius &&
			unitPtr->cur_x_loc() + unitPtr->sprite_info->loc_width > cx-radius &&
			unitPtr->cur_y_loc() <= cy+radius &&
			unitPtr->cur_y_loc() + unitPtr->sprite_info->loc_height > cy-radius )
		{
			unitPtr->hit_points -= (float) unitPtr->sprite_info->lightning_damage / ATTACK_SLOW_DOWN;

			// ---- add news -------//
			if( unitPtr->is_own() )
				news_array.lightning_damage(unitPtr->cur_x_loc(), unitPtr->cur_y_loc(),
					NEWS_LOC_UNIT, i, unitPtr->hit_points <= (float) 0);

			if( unitPtr->hit_points <= 0)
				unitPtr->hit_points = (float) 0;
		}
	}

	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) || !firm_res[firm_array[i]->firm_id]->buildable)
			continue;

		Firm *firmPtr = firm_array[i];
		if( firmPtr->loc_x1 <= cx+radius &&
			firmPtr->loc_x2 >= cx-radius &&
			firmPtr->loc_y1 <= cy+radius &&
			firmPtr->loc_y2 >= cy-radius)
		{
			firmPtr->hit_points -= (float) 50 / ATTACK_SLOW_DOWN;

			// ---- add news -------//
			if( firmPtr->own_firm() )
				news_array.lightning_damage(firmPtr->center_x, firmPtr->center_y,
					NEWS_LOC_FIRM, i, firmPtr->hit_points <= (float) 0);

			// ---- add a fire on it ------//
			Location *locPtr = get_loc(firmPtr->center_x, firmPtr->center_y);
			if( locPtr->can_set_fire() && locPtr->fire_str() < 5 )
				locPtr->set_fire_str(5);

			if( firmPtr->hit_points <= 0)
			{
				firmPtr->hit_points = (float) 0;
				se_res.sound(firmPtr->center_x, firmPtr->center_y, 1,
					'F', firmPtr->firm_id, "DIE" );
				firm_array.del_firm(i);
			}
		}
	}

	for( i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i))
			continue;

		Town *townPtr = town_array[i];

		if( townPtr->loc_x1 <= cx+radius &&
			townPtr->loc_x2 >= cx-radius &&
			townPtr->loc_y1 <= cy+radius &&
			townPtr->loc_y2 >= cy-radius)
		{
			// ---- add news -------//
			if( townPtr->nation_recno == nation_array.player_recno )
				news_array.lightning_damage(townPtr->center_x, townPtr->center_y,
					NEWS_LOC_TOWN, i, 0);

			// ---- add a fire on it ------//
			// ####### begin Gilbert 11/8 #########//
			Location *locPtr = get_loc(townPtr->center_x, townPtr->center_y);
			if( locPtr->can_set_fire() && locPtr->fire_str() < 5)
				locPtr->set_fire_str(5);
			// ####### end Gilbert 11/8 #########//

			townPtr->kill_town_people(0);
		}
	}
}
//-------- End of function World::lightning_strike -------//


// ------- Begin of function World::visit -----------//
// set the visit_level surrounding unit, town and firm
void World::visit(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int range, int extend)
{
	if(config.fog_of_war)
	{
		int left   = MAX( 0, xLoc1 - range);
		int top    = MAX( 0, yLoc1 - range);
		int right  = MIN( MAX_WORLD_X_LOC-1, xLoc2 + range);
		int bottom = MIN( MAX_WORLD_Y_LOC-1, yLoc2 + range);

		// ----- mark the visit_level of the square around the unit ------//
		for( int yLoc=top ; yLoc<=bottom ; yLoc++ )
		{
			Location *locPtr = get_loc(left, yLoc);
			for( int xLoc=left ; xLoc<=right ; xLoc++, locPtr++ )
			{
				locPtr->set_visited();
			}
		}

		// ----- visit_level decreasing outside the visible range ------//
		if( extend > 0)
		{
			int visitLevel = FULL_VISIBILITY;
			int levelDrop = (FULL_VISIBILITY - EXPLORED_VISIBILITY) / (extend+1);
			xLoc1 -= range;
			xLoc2 += range;
			yLoc1 -= range;
			yLoc2 += range;
			for( ++range; extend > 0; --extend, ++range)
			{
				xLoc1--;
				xLoc2++;
				yLoc1--;
				yLoc2++;
				visitLevel -= levelDrop;
				visit_shell(xLoc1, yLoc1, xLoc2, yLoc2, visitLevel);
			}
		}
	}
}
// ------- End of function World::visit -----------//


// ------- Begin of function World::visit_shell -----------//
// set specific visit_level on the surrounding unit, town and firm
void World::visit_shell(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int visitLevel)
{
	int left   = MAX( 0, xLoc1 );
	int top    = MAX( 0, yLoc1 );
	int right  = MIN( MAX_WORLD_X_LOC-1, xLoc2);
	int bottom = MIN( MAX_WORLD_Y_LOC-1, yLoc2);

	// ------- top side ---------//
	if( yLoc1 >= 0)
	{
		Location *locPtr = get_loc( left, yLoc1);
		for( int x = left; x <= right; ++x, ++locPtr)
			locPtr->set_visited(visitLevel);
	}

	// ------- bottom side ---------//
	if( yLoc2 < max_y_loc)
	{
		Location *locPtr = get_loc( left, yLoc2);
		for( int x = left; x <= right; ++x, ++locPtr)
			locPtr->set_visited(visitLevel);
	}

	// ------- left side -----------//
	if( xLoc1 >= 0)
	{
		for( int y = top; y <= bottom; ++y)
		{
			get_loc(xLoc1,y)->set_visited(visitLevel);
		}
	}

	// ------- right side -----------//
	if( xLoc2 < max_x_loc)
	{
		for( int y = top; y <= bottom; ++y)
		{
			get_loc(xLoc2,y)->set_visited(visitLevel);
		}
	}

}
// ------- End of function World::visit_shell -----------//


//------- Begin of function World::process_visibility -----------//

void World::process_visibility()
{
	if( config.fog_of_war )
	{
		// ###### begin Gilbert 13/10 ########//
		for( int y = 0; y < max_y_loc; ++y)
		{
			Location *locPtr = get_loc(0,y);
			for( int x = 0; x < max_x_loc; ++x, ++locPtr)
			{
				locPtr->dec_visibility();
			}
		}
		
		int count = max_x_loc * max_y_loc;
		const int sizeOfLoc = sizeof(Location);
		unsigned char *locVisitLevel = &get_loc(0,0)->visit_level;
		unsigned char decVisitLevel = EXPLORED_VISIBILITY*2+1;
#if 0
		/* Original Visual C++ assembly code for reference
		_asm
		{
			mov	ecx, count
			mov	ebx, locVisitLevel
			mov	edx, sizeOfLoc
			mov	ah, decVisitLevel
process_visit_level_1:
			mov	al,[ebx]
			cmp	al,ah			// if(al > EXPLORED_VISIBILITY*2) al--;
			cmc
			sbb	al,0
			mov	[ebx],al
			add	ebx, edx
			loop	process_visit_level_1
		}
		*/

		__asm__ __volatile__ (
			"movb %0, %%ah\n"
		"process_visit_level_1:\n\t"
			"movb (%%ebx), %%al\n\t"
			"cmpb %%ah, %%al\n\t"
			"cmc\n\t"
			"sbbb $0, %%al\n\t"
			"movb %%al, (%%ebx)\n\t"
			"addl %%edx, %%ebx\n\t"
			"loop process_visit_level_1\n\t"
			: 
			: "m"(decVisitLevel), "b"(locVisitLevel), "c"(count), "d"(sizeOfLoc)
			: "%eax"
		);
#endif
		// ###### end Gilbert 13/10 ########//
	}
}
//------- End of function World::process_visibility -----------//


//--------- Begin of function World::disp_next --------//
//
// Display the next object of the same type.
//
// <int> seekDir : -1 - display the previous one in the list.
// 					  1 - display the next one in the list.
//
// <int> sameNation - whether display the next object of the same
//							 nation only or of any nation.
//
void World::disp_next(int seekDir, int sameNation)
{
	//--- if the selected one is a unit ----//

	if( unit_array.selected_recno )
	{
		int unitRecno = unit_array.selected_recno;
		Unit* unitPtr = unit_array[unit_array.selected_recno];
		int unitClass = unit_res[unitPtr->unit_id]->unit_class;
		int nationRecno = unitPtr->nation_recno;

		while(1)
		{
			if( seekDir < 0 )
			{
				unitRecno--;

				if( unitRecno < 1 )
					unitRecno = unit_array.size();
			}
			else
			{
				unitRecno++;

				if( unitRecno > unit_array.size() )
					unitRecno = 1;
			}

			if( unit_array.is_deleted(unitRecno) )
				continue;

			unitPtr = unit_array[unitRecno];

			if( !unitPtr->is_visible() )
				continue;

			//--- check if the location of the unit has been explored ---//

			if( !world.get_loc(unitPtr->next_x_loc(), unitPtr->next_y_loc())->explored() )
				continue;

         //-------- if are of the same nation --------//

			if( sameNation && unitPtr->nation_recno != nationRecno )
				continue;

			//---------------------------------//

			if( unit_res[unitPtr->unit_id]->unit_class == unitClass )
			{
				power.reset_selection();
				unitPtr->selected_flag = 1;
				unit_array.selected_recno = unitRecno;
				unit_array.selected_count++;

				world.go_loc( unitPtr->cur_x_loc(), unitPtr->cur_y_loc() );
				return;
			}

			//--- if the recno loops back to the starting one ---//

			if( unitRecno == unit_array.selected_recno )
				break;
		}
	}

	//--- if the selected one is a firm ----//

	if( firm_array.selected_recno )
	{
		int firmRecno = firm_array.selected_recno;
		Firm* firmPtr = firm_array[firm_array.selected_recno];
		int firmId = firmPtr->firm_id;
		int nationRecno = firmPtr->nation_recno;

		while(1)
		{
			if( seekDir < 0 )
			{
				firmRecno--;

				if( firmRecno < 1 )
					firmRecno = firm_array.size();
			}
			else
			{
				firmRecno++;

				if( firmRecno > firm_array.size() )
					firmRecno = 1;
			}

			if( firm_array.is_deleted(firmRecno) )
				continue;

			firmPtr = firm_array[firmRecno];

			//-------- if are of the same nation --------//

			if( sameNation && firmPtr->nation_recno != nationRecno )
				continue;

			//--- check if the location of this firm has been explored ---//

			if( !world.get_loc(firmPtr->center_x, firmPtr->center_y)->explored() )
				continue;

			//---------------------------------//

			if( firmPtr->firm_id == firmId )
			{
				power.reset_selection();
				firm_array.selected_recno = firmRecno;

				world.go_loc( firmPtr->center_x, firmPtr->center_y );
				return;
			}

			//--- if the recno loops back to the starting one ---//

			if( firmRecno == firm_array.selected_recno )
				break;
		}
	}

	//--- if the selected one is a town ----//

	if( town_array.selected_recno )
	{
		int 	townRecno = town_array.selected_recno;
		int   nationRecno = town_array[townRecno]->nation_recno;
		Town* townPtr;

		while(1)
		{
			if( seekDir < 0 )
			{
				townRecno--;

				if( townRecno < 1 )
					townRecno = town_array.size();
			}
			else
			{
				townRecno++;

				if( townRecno > town_array.size() )
					townRecno = 1;
			}

			if( town_array.is_deleted(townRecno) )
				continue;

			townPtr = town_array[townRecno];

			//-------- if are of the same nation --------//

			if( sameNation && townPtr->nation_recno != nationRecno )
				continue;

			//--- check if the location of this town has been explored ---//

			if( !world.get_loc(townPtr->center_x, townPtr->center_y)->explored() )
				continue;

			//---------------------------------//

			power.reset_selection();
			town_array.selected_recno = townRecno;

			world.go_loc( townPtr->center_x, townPtr->center_y );
			return;

			//--- if the recno loops back to the starting one ---//

			if( townRecno == town_array.selected_recno )
				break;
		}
	}

	//--- if the selected one is a natural resource site ----//

	if( site_array.selected_recno )
	{
		int   siteRecno = site_array.selected_recno;
		Site* sitePtr   = site_array[site_array.selected_recno];
		int   siteType  = sitePtr->site_type;

		while(1)
		{
			if( seekDir < 0 )
			{
				siteRecno--;

				if( siteRecno < 1 )
					siteRecno = site_array.size();
			}
			else
			{
				siteRecno++;

				if( siteRecno > site_array.size() )
					siteRecno = 1;
			}

			if( site_array.is_deleted(siteRecno) )
				continue;

			sitePtr = site_array[siteRecno];

			//--- check if the location of this site has been explored ---//

			if( !world.get_loc(sitePtr->map_x_loc, sitePtr->map_y_loc)->explored() )
				continue;

			//---------------------------------//

			if( sitePtr->site_type == siteType )
			{
				power.reset_selection();
				site_array.selected_recno = siteRecno;

				world.go_loc( sitePtr->map_x_loc, sitePtr->map_y_loc );
				return;
			}

			//--- if the recno loops back to the starting one ---//

			if( siteRecno == site_array.selected_recno )
				break;
		}
	}
}
//----------- End of function World::disp_next --------//

#ifdef DEBUG3

//--------- Begin of function World::get_loc --------//
//
Location* World::get_loc(int xLoc, int yLoc)
{
	err_when( xLoc<0 || xLoc>=MAX_WORLD_X_LOC );
	err_when( yLoc<0 || yLoc>=MAX_WORLD_Y_LOC );

	return loc_matrix + MAX_WORLD_X_LOC * yLoc + xLoc;
}
//----------- End of function World::get_loc --------//


//--------- Begin of function World::get_region_id --------//
//
BYTE World::get_region_id(int xLoc, int yLoc)
{
	err_when( xLoc<0 || xLoc>=MAX_WORLD_X_LOC );
	err_when( yLoc<0 || yLoc>=MAX_WORLD_Y_LOC );

	return loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].region_id;
}
//----------- End of function World::get_region_id --------//

#endif

// ####### begin Gilbert 25/7 #########//
// return true if any location adjacent to (x,y) is on a particular region
int World::is_adjacent_region(int x, int y, int regionId)
{
	if( y > 0 )
	{
		if( x > 0 )
		{
			if( get_region_id(x-1,y-1) == regionId )
				return 1;
		}
		if( get_region_id(x,y-1) == regionId )
			return 1;
		if( x < max_x_loc-1 )
		{
			if( get_region_id(x+1,y-1) == regionId )
				return 1;
		}
	}
	if( x > 0 )
	{
		if( get_region_id(x-1,y) == regionId )
			return 1;
	}
	if( x < max_x_loc-1 )
	{
		if( get_region_id(x+1,y) == regionId )
			return 1;
	}
	if( y < max_y_loc-1)
	{
		if( x > 0 )
		{
			if( get_region_id(x-1,y+1) == regionId )
				return 1;
		}
		if( get_region_id(x,y+1) == regionId )
			return 1;
		if( x < max_x_loc-1 )
		{
			if( get_region_id(x+1,y+1) == regionId )
				return 1;
		}
	}
	return 0;
}
// ####### end Gilbert 25/7 #########//
