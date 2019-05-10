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

//Filename    : OWORLDMT.H
//Description : Header file for World Matrix WorldMap & WorldZoom

#ifndef __OWORLDMT_H
#define __OWORLDMT_H

#ifndef __OMATRIX_H
#include <OMATRIX.h>
#endif

#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif

//-------- World matrix size ------------//

#define MAX_WORLD_X_LOC  (World::max_x_loc)
#define MAX_WORLD_Y_LOC  (World::max_y_loc)

//------------- Map window -------------//

#define MAP_WIDTH       MAX_WORLD_X_LOC
#define MAP_HEIGHT      MAX_WORLD_Y_LOC

#define MAX_MAP_WIDTH	200
#define MAX_MAP_HEIGHT	200

// #define MAP_X1          (588+(MAX_MAP_WIDTH-MAP_WIDTH)/2)
// #define MAP_Y1          (56 +(MAX_MAP_HEIGHT-MAP_HEIGHT)/2)
#define MAP_X1          (ZOOM_WIDTH+10+(MAX_MAP_WIDTH-MAP_WIDTH)/2)
#define MAP_Y1          (ZOOM_Y1+(MAX_MAP_HEIGHT-MAP_HEIGHT)/2)
#define MAP_X2          (MAP_X1+MAP_WIDTH-1)
#define MAP_Y2          (MAP_Y1+MAP_HEIGHT-1)

#define MAP_LOC_HEIGHT   1 		// when MAP_VIEW_ENTIRE
#define MAP_LOC_WIDTH    1

#define MAP2_LOC_HEIGHT  2			// when MAP_VIEW_SECTION
#define MAP2_LOC_WIDTH   2

#define MAPMODE_X1   (VGA_WIDTH-220)
#define MAPMODE_Y1   3
#define MAP_MODE_BUTTON_WIDTH 40
//----------- Zoom window -------------//

#define ZOOM_X1           0     // World Zoom Window
#define ZOOM_Y1          56


// #define ZOOM_WIDTH      576     // ZOOM_LOC_WIDTH(32)  * 18 = 576
// #define ZOOM_HEIGHT     544     // ZOOM_LOC_HEIGHT(32) * 17 = 544
// #define ZOOM_WIDTH      800     // ZOOM_LOC_WIDTH(32)  * 25 = 800
// #define ZOOM_HEIGHT     704     // ZOOM_LOC_HEIGHT(32) * 22 = 704
#define ZOOM_WIDTH      config.zoom_width
#define ZOOM_HEIGHT     config.zoom_height

// #define ZOOM_X2         575
// #define ZOOM_Y2         599
// #define ZOOM_X2         ZOOM_X1 + ZOOM_WIDTH
// #define ZOOM_Y2         ZOOM_Y1 + ZOOM_HEIGHT
#define ZOOM_X2         (ZOOM_X1 + ZOOM_WIDTH)
#define ZOOM_Y2         (ZOOM_Y1 + ZOOM_HEIGHT)


#define ZOOM_LOC_HEIGHT  32     // in world zoom window
#define ZOOM_LOC_WIDTH   32

#define ZOOM_X_SHIFT_COUNT  5    // x>>5 = xLoc
#define ZOOM_Y_SHIFT_COUNT  5    // y>>5 = yLoc

#define ZOOM_X_PIXELS  (MAX_WORLD_X_LOC * ZOOM_LOC_WIDTH)
#define ZOOM_Y_PIXELS  (MAX_WORLD_Y_LOC * ZOOM_LOC_HEIGHT)

//---------- define map modes -----------//

#define MAP_MODE_COUNT  3

enum { MAP_MODE_TERRAIN=0,
		 MAP_MODE_POWER,
		 MAP_MODE_SPOT,
	  };

//-------- Define class MapMatrix -------//

class MapMatrix : public Matrix
{
public:
	char  last_map_mode;
	char	map_mode;
	char	power_mode;		// 1-also display power regions on the zoom map, 0-only display power regions on the mini map

public:
	MapMatrix();
   ~MapMatrix();

	void init_para();
	void draw();
	void paint();
	void disp();
	void draw_square();
	int  detect();
	void toggle_map_mode(int modeId);
	void cycle_map_mode();

protected:
	void draw_map();
	int  detect_area();

	void disp_mode_button(int putFront=0);
};

//-------- Define class ZoomMatrix -------//

class ZoomMatrix : public Matrix
{
public:
	DynArray land_disp_sort_array;     // an array for displaying objects in a sorted order
	DynArray air_disp_sort_array;
	DynArray land_top_disp_sort_array;
	DynArray land_bottom_disp_sort_array;

	int	init_rain;
	int	rain_channel_id;
	int	wind_channel_id;
	int	fire_channel_id;
	int	last_fire_vol;
	int	init_lightning; // reset on new game, save on save game
	int	init_snow;
	short	last_brightness;
	int	vibration; // reset on new game, save on save game
	short	lightning_x1, lightning_y1, lightning_x2, lightning_y2; // save on save game

public:
   ZoomMatrix();

	void init_para();
	void draw();
	void draw_frame();
	void scroll(int,int);
	void draw_white_site();
	void put_bitmap_clip(int x, int y, char* bitmapPtr,int compressedFlag=0);
	void put_bitmap_remap_clip(int x, int y, char* bitmapPtr, char* colorRemapTable=NULL,int compressedFlag=0);
	int  detect_bitmap_clip(int x, int y, char* bitmapPtr);
	bool is_bitmap_clip(int x, int y, char* bitmapPtr);

protected:
	void draw_objects();
	void draw_objects_now(DynArray* unitArray, int = 0);

	void draw_weather_effects();

	void draw_build_marker();
	void draw_god_cast_range();

	void blacken_unexplored();
	void blacken_fog_of_war();

	void disp_text();
	void put_center_text(int x, int y, const char* str);
};

//------------------------------------------------//

#endif
