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

//Filename    : OMATRIX.H
//Description : Object road direction turn

#ifndef __OMATRIX_H
#define __OMATRIX_H

#ifndef __OUNITRES_H
#include <OUNITRES.h>
#endif

#include <OREGION.h>

//----- Define bit meanings Location::flag ------//
#define 	LOCATE_WALK_LAND			0x01
#define	LOCATE_WALK_SEA			0x02
#define	LOCATE_COAST				0x08

// ----- govern the usage of extra_para ---------//
#define	LOCATE_SITE_MASK			0xf0
#define	LOCATE_HAS_SITE			0x10
#define	LOCATE_HAD_WALL			0x20
#define	LOCATE_HAS_DIRT			0x30
#define	LOCATE_SITE_RESERVED		0xf0
//	occupied by other block such as hill, plant

// ----- govern the usage of cargo_recno -------//
#define	LOCATE_BLOCK_MASK			0xf00
#define	LOCATE_IS_HILL				0x100
#define	LOCATE_IS_PLANT			0x200
#define	LOCATE_IS_TOWN				0x300
#define	LOCATE_IS_FIRM				0x400
#define  LOCATE_IS_WALL				0x500
#define	LOCATE_IS_ROCK				0xf00

#define	LOCATE_POWER_OFF			0x1000 // true if no power_nation_recno can be set in this location
#define	LOCATE_HARBOR_BIT       0x2000 // true if the terrain is suitable to build harbor (x,y) to (x+2,y+2)

// ------- constant on visibility ----------//
// const unsigned char FULL_VISIBILITY = MAX_BRIGHTNESS_ADJUST_DEGREE * 8 + 7;
const unsigned char FULL_VISIBILITY = 87;
// if a location has not been explored, visibility = 0
// if a location has been explored, visibility is bewtween 36-87
const unsigned char EXPLORED_VISIBILITY = 30;	// don't see this to multiple of 8
const unsigned char MAX_VISIT_LEVEL = FULL_VISIBILITY;


//------- Define structure Location -------//

#pragma pack(1)
struct Location
{
public:
	unsigned short loc_flag;
	short			  terrain_id;

	short         cargo_recno;
	short         air_cargo_recno;

	unsigned char extra_para;

	//------------------------------------------------//
	// when (loc_flag & LOCATE_SITE_MASK) == LOCATE_HAS_SITE
	// > extra_para = raw recno
	//
	// when (loc_flag & LOCATE_SITE_MASK) == LOCATE_HAD_WALL
	// > extra_para = time remained that can't build wall
	//
	// when (loc_flag & LOCATE_SITE_MASK) == LOCATE_HAS_DIRT
	// > extra_para = dirt recno
	//
	// when (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_HILL
	// > cargo_recno = top hill block id
	// > extra_para = bottom hill block id
	//
	// when (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_FIRM
	// > cargo_recno = firm recno
	//
	// when (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_TOWN
	// > cargo_recno = town zone recno
	//
	//	when (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_PLANT
	// > extra_para  = id. of the plant bitmap
	// > cargo_recno = low byte - inner x, high byte - inner y
	//
	//	when (loc_flag & LOCATE_BLOCK_MASK) == LOCATION_IS_WALL
	// > extra_para  = id. of the city wall bitmap
	// > high byte of cargo_recno = hit points remained for the wall
	//
	//	when (loc_flag & LOCATE_BLOCK_MASK) == LOCATION_IS_ROCK
	// > cargo_recno = rock recno in rock_array
	//
	// when (loc_flag & LOCATE_BLOCK_MASK) == 0 and cargo_recno != 0
	// > carge_recno = unit id
	//------------------------------------------------//

	char		fire_level;					// -100 to 100, current fire level
	char		flammability;				// -100 to 100, likelihood of fire

	char		power_nation_recno;		// 0-no nation has power over this location
	uint8_t		region_id;
	unsigned char visit_level;			// drop from FULL_VISIBILITY to 0

public:
	//------ functions that check the type of the location ------//

	int   walkable()	      { return loc_flag & LOCATE_WALK_LAND; }
	int   sailable()			{ return loc_flag & LOCATE_WALK_SEA; }
	int   walkable(int teraMask)
									{ return loc_flag & teraMask; }
	void	walkable_reset();
	// void	walkable_on()		{ loc_flag |= LOCATE_WALK_LAND; }
	void	walkable_off()		{ loc_flag &= ~(LOCATE_WALK_LAND | LOCATE_WALK_SEA); }

	void	walkable_on(int teraMask)		{ loc_flag |= teraMask; }
	void	walkable_off(int teraMask)		{ loc_flag &= ~teraMask; }

	int	is_coast()			{ return loc_flag & LOCATE_COAST; }

//	int	explored()        { return loc_flag & LOCATE_EXPLORED; }
//	void	explored_on()		{ loc_flag |= LOCATE_EXPLORED; }
//	void	explored_off()		{ loc_flag &= (~LOCATE_EXPLORED); }
	int	explored()        { return visit_level > 0; }
	void	explored_on()		{ if( visit_level < EXPLORED_VISIBILITY*2) visit_level = EXPLORED_VISIBILITY*2; }
	void	explored_off()		{ visit_level = 0; }

	// ---------- visibility --------//
	unsigned char visibility()				{ return visit_level/2; }
	void	dec_visibility()					{ if( visit_level > EXPLORED_VISIBILITY*2) --visit_level; }
	void	set_visited()						{ visit_level = MAX_VISIT_LEVEL*2; }
	void	set_visited(unsigned char v)	{ if( visit_level < v*2) visit_level = v*2; }

	int	is_plateau();


	// ----------- site -------------//
	int	can_build_site(int teraMask=LOCATE_WALK_LAND)
			{ return (loc_flag & teraMask) && !(loc_flag & LOCATE_SITE_MASK) && !has_site(); }
	void	set_site(int siteRecno);
	int	has_site()			{ return (loc_flag & LOCATE_SITE_MASK) == LOCATE_HAS_SITE; }
	int	site_recno()		{ if( has_site() )  return extra_para; else return 0; }
	void	remove_site();

	// ------------ wall timeout ----------//
	int	had_wall()			{ return (loc_flag & LOCATE_SITE_MASK) == LOCATE_HAD_WALL; }
	// after wall destructed, cannot build wall again for some time
	// the decrease time
	// (LOCATE_HAD_WALL)
	void	set_wall_timeout(int initTimeout);
	int	wall_timeout()					{ return extra_para; }
	int	dec_wall_timeout(int=1);
	void	remove_wall_timeout();

	// ----------- dirt -------------//
	int	can_add_dirt()	
			{ return !(loc_flag & LOCATE_SITE_MASK); }
	void	set_dirt(int dirtRecno);
	int	has_dirt()			{ return (loc_flag & LOCATE_SITE_MASK) == LOCATE_HAS_DIRT; }
	int	dirt_recno()		{ if( has_dirt() )  return extra_para; else return 0; }
	void	remove_dirt();

	// ---------- firm ----------//
	int   is_firm()         { return (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_FIRM; }
	int   can_build_firm(int teraMask = LOCATE_WALK_LAND)
			{ return !cargo_recno && (loc_flag & teraMask) && !(loc_flag & LOCATE_BLOCK_MASK) && !is_power_off(); }
	// ####### begin Gilbert 17/7 ###########//
	int	can_build_harbor(int teraMask = LOCATE_WALK_LAND)
			{ return !cargo_recno && (loc_flag & teraMask) && !(loc_flag & LOCATE_BLOCK_MASK); }
	// ####### end Gilbert 17/7 ###########//
	void	set_firm(int firmRecno);
	int 	firm_recno()		{ if( is_firm() )   	  return cargo_recno; else return 0; }
	void	remove_firm();

	// ---------- town ------------//
	int   is_town()    	 	{ return (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_TOWN; }
	void	set_town(int townRecno);
	int   can_build_town()
			{ return !cargo_recno && (loc_flag & LOCATE_WALK_LAND) && !(loc_flag & LOCATE_BLOCK_MASK) && !has_site() && !is_power_off(); }
	int	town_recno()		{ if( is_town() ) return cargo_recno; else return 0; }
	void	remove_town();

	// ---------- hill -------------//
	int	has_hill()			{ return (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_HILL; }
	int	can_add_hill()		// exception : it has already a hill
			{ return has_hill() || // (loc_flag & LOCATE_WALK_LAND) &&
			!cargo_recno && !(loc_flag & (LOCATE_BLOCK_MASK | LOCATE_SITE_MASK)); }
	void	set_hill(int hillId);
	int   hill_id1()			{ return cargo_recno; }
	int   hill_id2()			{ return extra_para; }
	void	remove_hill();

	// ---------- wall ------------//
	int	is_wall()			{ return (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_WALL; }
	int	can_build_wall()
			{ return !cargo_recno && (loc_flag & LOCATE_WALK_LAND) &&
			!(loc_flag & (LOCATE_BLOCK_MASK | LOCATE_SITE_MASK)) && !has_site(); }
	void	set_wall(int wallId, int townRecno, int hitPoints);
	void	set_wall_creating();
	void	set_wall_destructing();
	void	chg_wall_id( int wallId)		{ extra_para = wallId; }
	int	wall_id()			{ if( is_wall() ) return extra_para; else return 0; }
	int	wall_nation_recno() { return power_nation_recno; }
	int	wall_hit_point()	{ return cargo_recno >> 8; }
	int	wall_town_recno()	{ return cargo_recno || 0xFF; }
	//---------------------------------------------------//
	// initial 0, 1 to 100:creating, -1 to -100: destructing
	// except 0 or 100, hit point slowly increase by 1
	//---------------------------------------------------//
	int	wall_abs_hit_point()			{ return wall_hit_point() >= 0? wall_hit_point() : -wall_hit_point(); }
	int	inc_wall_hit_point(int grow=1);
	int	attack_wall(int damage=1);
	int	wall_grade()					{ return wall_hit_point() >= 0 ? (wall_hit_point()+24) / 25 : (wall_hit_point()-24)/25;}
	int	is_wall_creating()			{ return wall_hit_point() > 0; }
	int	is_wall_destructing()		{ return wall_hit_point() < 0; }
	void	remove_wall(int setTimeOut=-1);

	// ---------- plant -----------//
	int 	is_plant()						{ return (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_PLANT; }
	int 	can_add_plant(int teraMask = LOCATE_WALK_LAND)
			{ return !cargo_recno && (loc_flag & teraMask) && !(loc_flag & (LOCATE_BLOCK_MASK | LOCATE_SITE_MASK)) && !has_site(); }
	void	set_plant(int plantId, int offsetX, int offsetY);
	int	plant_id()						{ if( is_plant() ) return extra_para; else return 0; }
	int	plant_inner_x() 				{ return cargo_recno & 0xFF; }
	int	plant_inner_y() 				{ return cargo_recno >> 8;   }
	void	grow_plant()					{ extra_para++; }
	void	remove_plant();

	// ---------- rock ------------//
	int	is_rock()						{ return (loc_flag & LOCATE_BLOCK_MASK) == LOCATE_IS_ROCK; }
	int	can_add_rock(int teraMask = LOCATE_WALK_LAND)
			{ return !cargo_recno && (loc_flag & teraMask) && !(loc_flag & LOCATE_BLOCK_MASK); }
	void	set_rock(short rockArrayRecno);
	short	rock_array_recno()				{ if( is_rock() ) return cargo_recno; else return 0; }
	void	remove_rock();

	// call region_type only when generating region number
	RegionType region_type()			{ return walkable()?REGION_LAND:(sailable()?REGION_SEA:REGION_INPASSABLE); }

	// --------- functions on fire ---------//
	char	fire_str()						{ return fire_level; }
	char	fire_src()						{ return flammability; }
	void	set_fire_str(char str)		{ fire_level = str; }
	void	set_fire_src(char src)		{ flammability = src; }
	void	add_fire_str(char str)		{ fire_level += str; }
	void	add_fire_src(char src)		{ flammability += src; }
	int	can_set_fire()					{ return flammability >= -50; }

	//----- functions whose results affected by mobile_type -----//

	//int   is_blocked(int mobileType)    { return mobileType==UNIT_AIR ? air_cargo_recno : cargo_recno; }     // return 1 or 0 (although both are the same)
	int   unit_recno(int mobileType)    { return mobileType==UNIT_AIR ? air_cargo_recno : cargo_recno; }     // return the exact cargo recno

	int   has_unit(int mobileType);
	int	has_any_unit(int mobileType = UNIT_LAND);
	int 	get_any_unit(int& mobileType);
	int   is_accessible(int mobileType);      // whether the location is accessible to the unit of the specific mobile type

	int   is_unit_group_accessible(int mobileType, uint32_t curGroupId);

	//int   can_move(int mobileType)      { return is_accessible(mobileType) && cargo_recno==0; }
	int   can_move(int mobileType)      { return is_accessible(mobileType) && (mobileType==UNIT_AIR ? !air_cargo_recno : !cargo_recno); }

	//### begin alex 24/6 ###//
	//------------ power --------------//
	void	set_power_on();
	void	set_power_off();
	int	is_power_off();
	//#### end alex 24/6 ####//

	//----------- harbor bit -----------//
	void	set_harbor_bit()         { loc_flag |= LOCATE_HARBOR_BIT; }
	void	clear_harbor_bit()       { loc_flag &= ~LOCATE_HARBOR_BIT; }
	int	can_build_whole_harbor() { return loc_flag & LOCATE_HARBOR_BIT; }
};
#pragma pack()

//------------ Define class Matrix -----------//

class World;

class Matrix
{
friend class World;

public:
   int       max_x_loc, max_y_loc;      // read from map file
   Location  *loc_matrix;

	int       top_x_loc, top_y_loc;              // the top left location of current zoom window
	int       cur_x_loc, cur_y_loc;
	int       cur_cargo_width, cur_cargo_height; // cur_x2_loc = cur_x_loc + cur_cargo_width

	int       disp_x_loc, disp_y_loc;    // calculated in Matrix()
	int       loc_width, loc_height;     // passed as para in Matrix()

   int       win_x1, win_y1, win_x2, win_y2;
	int       image_x1, image_y1, image_x2, image_y2;
   int       image_width, image_height;

   char      own_matrix;

   char      *save_image_buf;
	char      just_drawn_flag;    // whether the matrix has just been drawn by draw()

public:
   virtual ~Matrix();

   void init(int,int,int,int,int,int,int,int,int);
   void assign_map(Matrix*);
   void assign_map(Location*,int,int);

   virtual void paint();
   virtual void draw();
   virtual void disp();
   virtual int  detect()      {return 0;}
   virtual void refresh();
   virtual void scroll(int,int);

   Location* get_loc(int xLoc,int yLoc)
        { return loc_matrix+yLoc * max_x_loc + xLoc; }

protected:
   virtual void post_draw()      {;}
   virtual void draw_loc(int x,int y,int xLoc,int yLoc,Location* locPtr) {;}

   void init_var();
	int  valid_cur_box(int=1);
   void valid_disp_area(int=0);
};

//--------- Define inline functions ----------//

//-------- Begin of function Location::is_accessible --------//

inline int Location::is_accessible(int mobileType)
{
	switch(mobileType)
	{
		case UNIT_LAND:
			return walkable();
			break;

		case UNIT_SEA:
			return sailable();
			break;

		case UNIT_AIR:
			return 1;
			break;
	}

	return 0;
}
//-------- End of function Location::is_accessible --------//

#endif
