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

//Filename    : OWORLD.H
//Description : Header file of Object World

#ifndef __OWORLD_H
#define __OWORLD_H

#ifndef __OWORLDMT_H
#include <OWORLDMT.h>
#endif

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OUNITRES_H
#include <OUNITRES.h>
#endif

//----------- Define constant ------------//

#define EXPLORE_RANGE   10
#define SCAN_FIRE_DIST 4

//--------------- Define constant -----------------//

#define MIN_LAND_COST   500000       // Minimum land cost even there is no population at all

//------- define terrain map --------//

#define MIN_GRASS_HEIGHT    100
#define MIN_HILL_HEIGHT  	 230
#define MIN_MOUNTAIN_HEIGHT 242
#define MIN_ICE_HEIGHT 		 252

//---------------- Define class World -------------//

class Weather;
class Plasma;

class World
{
public:
	MapMatrix    *map_matrix;
	ZoomMatrix   *zoom_matrix;
	Location     *loc_matrix;

	unsigned long	 		 next_scroll_time;		 // next scroll time

	char			 scan_fire_x;				// cycle from 0 to SCAN_FIRE_DIST-1
	char			 scan_fire_y;
	char			 lightning_signal;
	int			 plant_count;
	int			 plant_limit;

	//--------- static member vars --------------//

	static short view_top_x, view_top_y;		// the view window in the scene, they are relative coordinations on the entire virtual surface.
	static int   max_x_loc , max_y_loc;		   // these must be static vars as MAX_WORLD_X_LOC & MAX_WORLD_Y_LOC are defined to use them.

public:
	World();
	~World();

	void 		init();
	void 		deinit();

	void 		generate_map();
	void 		assign_map();

	void 		paint();
	void 		refresh();
	void		disp_zoom()		{ zoom_matrix->disp(); }

	void		load_map(char*);

	int  		detect();
	// ###### begin Gilbert 16/9 ########//
	int		detect_firm_town();
	// ###### end Gilbert 16/9 ########//
	void 		go_loc(int xLoc, int yLoc, int selectedFlag=0);
	void		disp_next(int seekDir, int sameNation);

	int  		write_file(File*);
	int  		read_file(File*);

public:
	#ifdef DEBUG3
		Location* get_loc(int xLoc,int yLoc);
		uint8_t		 get_region_id(int xLoc,int yLoc);
	#else
		Location* get_loc(int xLoc,int yLoc)
						{ return loc_matrix + MAX_WORLD_X_LOC * yLoc + xLoc; }

		uint8_t		 get_region_id(int xLoc,int yLoc)
						{ return loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].region_id; }
	#endif

	short		get_unit_recno(int xLoc,int yLoc, int mobileType);
	void 		set_unit_recno(int xLoc, int yLoc, int mobileType, int newCargoRecno);

	int 		distance_rating(int xLoc1, int yLoc1, int xLoc2, int yLoc2);

	void		unveil(int xLoc1, int yLoc1, int xLoc2, int yLoc2);
	void		explore(int xLoc1, int yLoc1, int xLoc2, int yLoc2);
	int  		is_explored(int xLoc1, int yLoc1, int xLoc2, int yLoc2);
	// always call unveil before visit //
	void		visit(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int range, int extend =0);
	void		visit_shell(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int visitLevel);

	int		can_build_firm(int xLoc1, int yLoc1, int firmId, short unitRecno= -1);
	int		can_build_town(int xLoc1, int yLoc1, short unitRecno= -1);
	int		can_build_wall(int xLoc1, int yLoc1, short nationRecno);
	int		can_destruct_wall(int xLoc1, int yLoc1, short nationRecno);
	void		build_wall_tile(int xLoc1, int yLoc1, short nationRecno, char remoteAction);
	void		destruct_wall_tile(int xLoc1, int yLoc1, short nationRecno, char remoteAction);

	int 		locate_space(int* /*in/out*/ xLoc1, int* /*in/out*/ yLoc1, int xLoc2, int yLoc2,
										int spaceLocWidth, int spaceLocHeight, int mobileType=UNIT_LAND, int regionId=0, int buildFlag=0);
	int		check_unit_space(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int mobileType=UNIT_LAND, int buildFlag=0);

	int 		locate_space_random(int& xLoc1, int& yLoc1, int xLoc2, int yLoc2,
										  int spaceLocWidth, int spaceLocHeight, int maxTries,
										  int regionId=0, int buildSite=0, char teraMask=1);
	int		is_adjacent_region( int x, int y, int regionId );
	int		is_harbor_region(int xLoc, int yLoc, int landRegionId, int seaRegionId);

	void 		draw_link_line(int srcFirmId, int srcTownRecno, int srcXLoc1, int srcYLoc1, int srcXLoc2, int srcYLoc2, int giveEffectiveDis=0);

	void 		set_all_power();
	void 		set_power(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int nationRecno);
	void 		restore_power(int xLoc1, int yLoc1, int xLoc2, int yLoc2, int townRecno, int firmRecno);
	void		set_surr_power_off(int xLoc, int yLoc);

	void		process();
	void		process_visibility();
	void		next_day();

	//------- functions related to plant's growth, see ow_plant.cpp -----//

	void		plant_ops();
	void		plant_grow(int pGrow =4, int scanDensity =8);
	void		plant_reprod(int pRepord =1, int scanDensity =8);
	void		plant_death(int scanDensity =8);
	void		plant_spread(int pSpread =5);
	void		plant_init();
	void		plant_spray(short *plantIdArray, char strength, short x, short y);

	//------- functions related to fire's spreading, see ow_fire.cpp ----//

	void		init_fire();
	void		spread_fire(Weather &);
	void		setup_fire(short x, short y, char fireStrength = 30);

	//------- function related to city wall ----------//
	void		build_wall_section(short x1, short y1, short x2, short y2,
				short townRecno, short initHp = 99);
	void		build_wall(int townRecno, short initHp = 99);

	void		open_west_gate(short x2, short y1, short townRecno);
	void		open_east_gate(short x1, short y1, short townRecno);
	void		open_north_gate(short x1, short y2, short townRecno);
	void		open_south_gate(short x1, short y1, short townRecno);

	int		form_wall(short x, short y, short maxRecur=0);
	void		form_world_wall();
	int		correct_wall(short x, short y, short maxRecur=2);

	//-------- function related to rock ----------//
	void		add_rock(short rockRecno, short x1, short y1);
	void		add_dirt(short dirtRecno, short x1, short y1);
	// ###### begin Gilbert 22/9 ######//
	int		can_add_rock(short x1, short y1, short x2, short y2);
	int		can_add_dirt(short x1, short y1, short x2, short y2);
	// ###### end Gilbert 22/9 ######//

	// ------ function related to weather ---------//

   void     earth_quake();
   void     lightning_strike(short x, short y, short radius=0);

private:
	int	  	detect_scroll();
	// int		detect_firm_town();

	//--------- ambient sound functions --------//

	void		process_ambient_sound();

	//--- called by generate_map() only ---//

	void    add_base_level();
	void    gen_plasma_map();
	void    set_tera_id(Plasma &);
	void    remove_odd(Plasma &, short x, short y, short recur);
	void    set_climate();
	void	  set_loc_flags();
	void	  substitute_pattern();
	void    set_region_id();
	void    fill_region(short x, short y);
	// ####### begin Gilbert 22/9 ########//
	void    gen_rocks(int nGrouped, int nLarge, int nSmall);
	void    gen_dirt(int nGrouped, int nLarge, int nSmall);
	// ####### end Gilbert 22/9 ########//
	void    set_harbor_bit();

	// private function called by build_wall_section
	int	can_build_area(short x1, short y1, short x2, short y2);
	void	build_west_gate(short x1, short y1, short townRec, short initHp);
	void	build_east_gate(short x1, short y1, short townRec, short initHp);
	void	build_north_gate(short x1, short y1, short townRec, short initHp);
	void	build_south_gate(short x1, short y1, short townRec, short initHp);

	void	build_west_wall(short x1, short y1, short y2, short townRec, short initHp);
	void	build_east_wall(short x1, short y1, short y2, short townRec, short initHp);
	void	build_north_wall(short x1, short x2, short y1, short townRec, short initHp);
	void	build_south_wall(short x1, short x2, short y1, short townRec, short initHp);

	// see OGENHILL.CPP
	void	gen_hills(int terrainType);
	void	put_hill_set(short *px, short *py, short hSetId);
	void	put_hill_pattern(short *px, short *py, unsigned char patternId);
	void	fill_hill(short x, short y);
};

//-------- Begin of function World::get_unit_recno -------//

inline short World::get_unit_recno(int xLoc, int yLoc, int mobileType)
{
	if( mobileType==UNIT_AIR )
		return loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].air_cargo_recno;
	else
		return loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].cargo_recno;
}
//--------- End of function World::get_unit_recno -------//


//-------- Begin of function World::set_unit_recno -------//

inline void World::set_unit_recno(int xLoc,int yLoc, int mobileType, int newCargoRecno)
{
	if( mobileType==UNIT_AIR )
		loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].air_cargo_recno = newCargoRecno;
	else
		loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].cargo_recno = newCargoRecno;

	err_when(mobileType!=UNIT_AIR && loc_matrix[MAX_WORLD_X_LOC*yLoc+xLoc].is_firm());
}
//--------- End of function World::set_unit_recno -------//


//--------- Begin of function World::distance_rating --------//
//
inline int World::distance_rating(int xLoc1, int yLoc1, int xLoc2, int yLoc2)
{
	int curDistance = misc.points_distance(xLoc1, yLoc1, xLoc2, yLoc2);

	return 100 - 100 * curDistance / MAX_WORLD_X_LOC;
}
//----------- End of function World::distance_rating --------//


extern World world;

#endif
