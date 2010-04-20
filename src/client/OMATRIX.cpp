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

//Filename    : OMATRIX.CPP
//Description : Object road direction turn, derived by World, Chain class

#include <ALL.h>
#include <OVGA.h>
#include <OMOUSE.h>
#include <OSYS.h>
#include <OSITE.h>
#include <OFIRM.h>
#include <OTERRAIN.h>
#include <OPLANT.h>
#include <OPOWER.h>
#include <OUNIT.h>
#include <OWORLD.h>
#include <OHILLRES.h>

// --------- define constant ----------//
#define DEFAULT_WALL_TIMEOUT 10
#define WALL_DEFENCE 5
#define MIN_WALL_DAMAGE 3

//----------- Begin of function Matrix::init ----------//
//
// <int> winX1,winY1 = the coordination of the win,
//       winX2,winY2   including the scroll bar and up,down panel
//
// <int> areaWidth   = width and height of the bitmap area
//       areaHeight
//
// <int> locWidth    = width and height of each location
// <int> locHeight
//
// <int> maxCargoWidth  = MAX. widht of height of cargo
//       maxCargoHeight  (unit is Loc, not pixel
//
// <int> saveAreaToBuf = whether save the area to a buffer,
//                       set it true - when the area will be ruined by
//                                     animation
//
void Matrix::init(int winX1, int winY1, int winX2, int winY2, int areaWidth,
						int areaHeight, int locWidth, int locHeight, int saveAreaToBuf )
{
	err_when(FULL_VISIBILITY != MAX_BRIGHTNESS_ADJUST_DEGREE * 8 + 7);
	win_x1 = winX1;      // window area,
	win_y1 = winY1;      // including scroll bar
	win_x2 = winX2;
	win_y2 = winY2;

	image_width  = areaWidth;
	image_height = areaHeight;

	image_x1 = win_x1;  // bitmap area only
	image_y1 = win_y1;
	image_x2 = image_x1+image_width-1;
	image_y2 = image_y1+image_height-1;

	loc_width  = locWidth;
	loc_height = locHeight;

	//----------------------------------------------//

	if( saveAreaToBuf )
		save_image_buf = mem_add( sizeof(short)*2 + image_width * image_height );	// 2 <short> for width & height info
	else
		save_image_buf = NULL;

   just_drawn_flag = 0;

	//---------------------------------------------//

	loc_matrix = NULL;

	top_x_loc = 0;
	top_y_loc = 0;

	cur_x_loc = -1;
	cur_y_loc = -1;
}
//------------- End of function Matrix::init -----------//


//----------- Begin of function Matrix::~Matrix ----------//

Matrix::~Matrix()
{
	if( save_image_buf )
		mem_del( save_image_buf );

	if( own_matrix && loc_matrix )       // the matrix is allocated by us
		mem_del( loc_matrix );
}
//------------- End of function Matrix::~Matrix -----------//


//----------- Begin of function Matrix::assign_map -----------//
//
// Instead of loading the map file, we can assign a pre-loaded
// map to it.
//
void Matrix::assign_map(Matrix* matrixPtr)
{
	own_matrix = 0;

	loc_matrix = matrixPtr->loc_matrix;

	max_x_loc  = matrixPtr->max_x_loc;
	max_y_loc  = matrixPtr->max_y_loc;

	init_var();

	top_x_loc  = 0;
	top_y_loc  = 0;

	cur_x_loc = 0;
   cur_y_loc = 0;
}
//------------- End of function Matrix::assign_map -----------//


//----------- Begin of function Matrix::assign_map -----------//
//
// Instead of loading the map file, we can assign a pre-loaded
// map to it.
//
void Matrix::assign_map(Location* locMatrix, int maxXLoc, int maxYLoc )
{
	own_matrix = 0;

   loc_matrix = locMatrix;

   max_x_loc  = maxXLoc;
   max_y_loc  = maxYLoc;

   init_var();

   cur_x_loc = 0;
	cur_y_loc = 0;
}
//------------- End of function Matrix::assign_map -----------//


//-------- Begin of function Matrix::init_var ---------//
//
// Called by assign_map() & MatrixMap::set_view_map()
//
void Matrix::init_var()
{
   disp_x_loc = image_width/loc_width;
   if( disp_x_loc > max_x_loc )
      disp_x_loc = max_x_loc;

	disp_y_loc = image_height/loc_height;
   if( disp_y_loc > max_y_loc )
      disp_y_loc = max_y_loc;
}
//------------ End of function Matrix::init_var -----------//


//----------- Begin of function Matrix::paint ------------//
//
// Paint world window and scroll bars
//
void Matrix::paint()
{
}
//----------- End of function Matrix::paint ------------//


//---------- Begin of function Matrix::draw ------------//
//
// Draw world zoom
//
void Matrix::draw()
{
	int       i=0, x, y, xLoc, yLoc;
	Location* locPtr;

	int maxXLoc = top_x_loc + disp_x_loc;        // divide by 2 for world_info
	int maxYLoc = top_y_loc + disp_y_loc;

	//----------------------------------------------------//

	for( y=image_y1,yLoc=top_y_loc ; yLoc<maxYLoc ; yLoc++, y+=loc_height )
	{
		if( (i++)%10==0 )
			sys.yield();

		locPtr = get_loc(top_x_loc,yLoc);

		for( x=image_x1,xLoc=top_x_loc ; xLoc<maxXLoc ; xLoc++, x+=loc_width, locPtr++ )
			draw_loc( x, y, xLoc, yLoc, locPtr );
	}

	//---- derived function for ZoomMatrix & MapMatrix ----//

	post_draw();	// if they have anything to draw after the basic terrain (e.g.firms)

	//---------------------------------------------------//

	if( save_image_buf )
	{
		vga_back.read_bitmap( image_x1, image_y1, image_x2, image_y2, save_image_buf );
		just_drawn_flag = 1;
	}
}
//------------ End of function Matrix::draw ------------//


//----------- Begin of function Matrix::disp ------------//
//
// Display the drawn world zoom on screen
//
void Matrix::disp()
{
	if( !just_drawn_flag )		// if the map has just been drawn in draw()
	{
		if( save_image_buf )
		{
			err_when( image_width%4 != 0 );
#ifdef DEBUG_TIMING
			unsigned long start_time, elapsed_time;
			int i = 0;
			start_time = m.get_time();
			for( i = 0; i < 10; ++i)
#endif
			vga_back.put_bitmap_dw( image_x1, image_y1, save_image_buf );
#ifdef DEBUG_TIMING
			elapsed_time = m.get_time() - start_time;
			// dummy
			start_time = 0;
#endif
		}
		else
			draw();
	}

	just_drawn_flag=0;
}
//----------- End of function Matrix::disp ------------//


//--------- Begin of function Matrix::refresh ------------//
//
void Matrix::refresh()
{
	if( save_image_buf )	// redraw the background, so disp() won't use the old background
		draw();

	disp();
}
//----------- End of function Matrix::refresh ------------//


//----------- Begin of function Matrix::valid_cur_box ------------//
//
// Validate the position of the zoom area box, scroll window if necessary
// called by detect_area()
//
// Call valid_cur_box() if current highlight box is specified,
//                      adjust window area to fit it
//
// Call valid_disp_area() if window display area is specified,
//                        adjust highlight box to fit it
//
// [int] = callRefresh = call refresh() if scrolled
//                       (default : 1)
//
// return : <int> whether the window is scrolled
//
int Matrix::valid_cur_box(int callRefresh)
{
	int scrolledFlag=0;

	//------- valid current highlight box first --------//

	if( cur_x_loc < 0 )
		cur_x_loc = 0;

	if( cur_y_loc < 0 )
		cur_y_loc = 0;

	if( cur_x_loc+cur_cargo_width > max_x_loc )
		cur_x_loc = max_x_loc-cur_cargo_width;

	if( cur_y_loc+cur_cargo_height > max_y_loc )
		cur_y_loc = max_y_loc-cur_cargo_height;


	//--- scroll the display area to fit the current highlight ---//

	if( cur_x_loc < top_x_loc ||
		 cur_x_loc >= top_x_loc + disp_x_loc )
	{
		top_x_loc = cur_x_loc - disp_x_loc/2;

		if( top_x_loc < 0 )
			top_x_loc = 0;

		if( top_x_loc + disp_x_loc > max_x_loc )
			top_x_loc = max_x_loc - disp_x_loc;

		scrolledFlag=1;
	}

	if( cur_y_loc < top_y_loc ||
		 cur_y_loc >= top_y_loc + disp_y_loc )
	{
		top_y_loc = cur_y_loc - disp_y_loc/2;

		if( top_y_loc < 0 )
			top_y_loc = 0;

		if( top_y_loc + disp_y_loc > max_y_loc )
			top_y_loc = max_y_loc - disp_y_loc;

		scrolledFlag=1;
	}

	return scrolledFlag;
}
//----------- End of function Matrix::valid_cur_box ------------//


//--------- Begin of function Matrix::valid_disp_area ------//
//
// Validate the current location, adjust it if it is out of border
//
// Call valid_cur_box() if current highlight box is specified,
//                  adjust window area to fit it
//
// Call valid_disp_area() if window display area is specified,
//                      adjust highlight box to fit it
//
// [int] fitInBox = whether must fit in current box into the
//                  display area (default : 0)
//
void Matrix::valid_disp_area(int fitInBox)
{
	//------- valid display area first ---------//

	if( top_x_loc < 0 )
		top_x_loc = 0;

	if( top_y_loc < 0 )
		top_y_loc = 0;

	if( top_x_loc + disp_x_loc > max_x_loc )
		top_x_loc = max_x_loc - disp_x_loc;

	if( top_y_loc + disp_y_loc > max_y_loc )
		top_y_loc = max_y_loc - disp_y_loc;

	//--- if the current highlighted location is outside the display area, then reposition it ----//

	if( fitInBox )
	{
		if( cur_x_loc < top_x_loc )
			cur_x_loc = top_x_loc;

		if( cur_x_loc >= top_x_loc + disp_x_loc )
			cur_x_loc = top_x_loc + disp_x_loc - 1;

		if( cur_y_loc < top_y_loc )
			cur_y_loc = top_y_loc;

		if( cur_y_loc >= top_y_loc + disp_y_loc )
			cur_y_loc = top_y_loc + disp_y_loc - 1;
	}
}
//--------- End of function Matrix::valid_disp_area ------//


//---------- Begin of function Matrix::scroll -----------//
//
// <int> xScroll - horizontal scroll step (negative:left, positive:right)
// <int> yScroll - vertical scroll step   (negative:left, positive:right)
//
void Matrix::scroll(int xScroll, int yScroll)
{
	top_x_loc += xScroll;
	top_y_loc += yScroll;

	valid_disp_area();
}
//----------- End of function Matrix::scroll ------------//


// ------- Begin of function Location::walkable_reset -----/
void Location::walkable_reset()
{
	if( loc_flag & LOCATE_BLOCK_MASK )
	{
		walkable_off();
	}
	else
	{
		if( terrain_res[terrain_id]->average_type == TERRAIN_OCEAN)
		{
			loc_flag |= LOCATE_WALK_SEA;
		}
		else
		{
			loc_flag |= LOCATE_WALK_LAND;
		}
	}
}
// ----------- End of function Location::walkable_reset -------//


// ----------- Begin of function Location::is_plateau ---------//
int Location::is_plateau()
{
	return terrain_res[terrain_id]->average_type == TERRAIN_DARK_DIRT;		//**BUGHERE, to be changed to TERRAIN_HILL when the no. of terrain type has been reduced to 4 from 7
}
// ----------- End of function Location::is_plateau ---------//


//---------- Begin of function Location::set_site ------------//
//
void Location::set_site(int siteRecno)
{
	err_when( !can_build_site() );

	loc_flag = loc_flag & ~LOCATE_SITE_MASK | LOCATE_HAS_SITE;
	// loc_flag |= LOCATION_HAS_SITE;

	extra_para = siteRecno;
}
//------------ End of function Location::set_site ------------//


//---------- Begin of function Location::remove_site ------------//
//
void Location::remove_site()
{
	err_when( !has_site() );

	loc_flag &= ~LOCATE_SITE_MASK;

	extra_para  = 0;
}
//------------ End of function Location::remove_site ------------//


//------------ Begin of function Location::set_wall_timeout ------------//
void Location::set_wall_timeout(int initTimeout)
{
	err_when((!can_build_site() && !had_wall()) || initTimeout <= 0);
	loc_flag = (loc_flag & ~LOCATE_SITE_MASK) | LOCATE_HAD_WALL;
	extra_para = (unsigned char) initTimeout;
}
//------------ End of function Location::set_wall_timeout ------------//


//------------ Begin of function Location::dec_wall_timeout ------------//
// return : true if the timeout drop to zero, and is removed
int Location::dec_wall_timeout(int t)
{
	err_when( !had_wall() );
	if( (extra_para -= t) <= 0)
	{
		remove_wall_timeout();
		return 1;
	}
	return 0;
}
//------------ End of function Location::dec_wall_timeout ------------//


//------------ Begin of function Location::remove_wall_timeout ------------//
void Location::remove_wall_timeout()
{
	err_when( !had_wall() );
	loc_flag &= ~LOCATE_SITE_MASK;
	extra_para = 0;
}
//------------ End of function Location::remove_wall_timeout ------------//


//---------- Begin of function Location::set_dirt ------------//
//
void Location::set_dirt(int dirtRecno)
{
	err_when( !can_add_dirt() );

	loc_flag = loc_flag & ~LOCATE_SITE_MASK | LOCATE_HAS_DIRT;

	extra_para = dirtRecno;
}
//------------ End of function Location::set_dirt ------------//


//---------- Begin of function Location::remove_dirt ------------//
//
void Location::remove_dirt()
{
	err_when( !has_dirt() );

	loc_flag &= ~LOCATE_SITE_MASK;

	extra_para  = 0;
}
//------------ End of function Location::remove_dirt ------------//


//---------- Begin of function Location::set_firm ------------//
//
void Location::set_firm(int firmRecno)
{
	// can't check the terrain type here
	err_when( !can_build_firm() && !firmRecno );

	walkable_off();
	loc_flag = (loc_flag & ~LOCATE_BLOCK_MASK) | LOCATE_IS_FIRM;

	cargo_recno = firmRecno;
}
//------------ End of function Location::set_firm ------------//


//---------- Begin of function Location::remove_firm ------------//
//
void Location::remove_firm()
{
	err_when( !is_firm() );

	loc_flag &= ~LOCATE_BLOCK_MASK;
	cargo_recno = 0;
	walkable_reset();

	err_when(is_firm());
}
//------------ End of function Location::remove_firm ------------//


//---------- Begin of function Location::set_town ------------//
//
void Location::set_town(int townRecno)
{
	err_when( !can_build_town() || !townRecno );

	walkable_off();
	loc_flag = loc_flag & ~LOCATE_BLOCK_MASK | LOCATE_IS_TOWN;

	cargo_recno = townRecno;
}
//------------ End of function Location::set_town ------------//


//---------- Begin of function Location::remove_town ------------//
//
void Location::remove_town()
{
	err_when( !is_town() );

	loc_flag &= ~LOCATE_BLOCK_MASK;
	cargo_recno = 0;
	walkable_reset();

	err_when(is_firm());
}
//------------ End of function Location::remove_town ------------//


//---------- Begin of function Location::set_hill ------------//
// set hillId to hill_id1 (cargo_recno) or hill_id2 (extra_para)
// depend on the priority of the hill block
void Location::set_hill(int hillId)
{
	err_when( !can_add_hill() || !hillId );
	err_when( !hill_res[hillId] );

	// clear LOCATE_WALK_LAND and LOCATE_WALK_SEA bits
	walkable_off();

	if( has_hill() )
	{
		// already has a hill block
		// compare which is on the top, swap if necessary
		if(hill_res[cargo_recno]->priority <= hill_res[hillId]->priority)
		{
			err_when(cargo_recno >= 256);
			extra_para = (unsigned char) cargo_recno;
			cargo_recno = hillId;
		}
		else
		{
			// if two hill blocks there, the lower one get replaced
			err_when( hillId >= 256);
			extra_para = (unsigned char) hillId;
		}
	}
	else
	{
		// no existing hill block
		loc_flag = loc_flag & ~(LOCATE_BLOCK_MASK | LOCATE_SITE_MASK )
			| (LOCATE_IS_HILL | LOCATE_SITE_RESERVED);
		cargo_recno = hillId;
		extra_para = 0;
	}
}
//------------ End of function Location::set_hill ------------//


//---------- Begin of function Location::remove_hill ------------//
void Location::remove_hill()
{
	err_when( !has_hill() );

	loc_flag &= ~(LOCATE_BLOCK_MASK | LOCATE_SITE_MASK);

	extra_para  = 0;
	cargo_recno = 0;
	// err_when(is_firm());
	// BUGHERE : need to call walkable_reset();
}
//------------ End of function Location::remove_hill ------------//


//---------- Begin of function Location::set_wall ------------//
//
// <int> wallId	 = the id. of the wall
// <int> townRecno = recno of the town which the wall belongs to
// <int> hitPoints = hit points remained for the wall
//
void Location::set_wall(int wallId, int townRecno, int hitPoints)
{
	err_when( !can_build_wall() || !wallId );

	walkable_off();
	loc_flag = loc_flag & ~(LOCATE_BLOCK_MASK | LOCATE_SITE_MASK )
		| (LOCATE_IS_WALL | LOCATE_SITE_RESERVED);

	extra_para  = wallId;
	cargo_recno = (hitPoints<<8) + townRecno;
}
//------------ End of function Location::set_wall ------------//


//------------ Begin of function Location::set_wall_creating ------------//
void Location::set_wall_creating()
{
	err_when( !is_wall() );
	int hp = wall_hit_point();
	if( hp < 0)
		hp = -hp;
	cargo_recno = (hp << 8) | wall_town_recno();
}
//------------ End of function Location::set_wall_creating ------------//


//------------ Begin of function Location::set_wall_destructing------------//
void Location::set_wall_destructing()
{
	err_when( !is_wall() );
	int hp = wall_hit_point();
	if( hp > 0)
		hp = -hp;
	cargo_recno = (hp << 8) | wall_town_recno();
}
//------------ End of function Location::set_wall_desctructing ------------//


//------------ Begin of function Location::inc_wall_hit_point ------------//
int Location::inc_wall_hit_point(int grow)
{
	err_when( !is_wall() );
	int hp = wall_hit_point();
	if( hp < 0 && hp > -grow)
	{
		hp = 0;
	}
	else if( hp > 100-grow)
	{
		hp = 100;
	}
	else
		hp += grow;
	cargo_recno = (hp << 8) | wall_town_recno();
	return hp;
}
//------------ End of function Location::inc_wall_hit_point ------------//


//------------ Begin of function Location::attack_wall ------------//
//
// attack wall
// int damage       damage to a wall
// note : if the return value is 0, call world.correct_wall to
//        correct the shape of the adjacent squares
//
int Location::attack_wall(int damage)
{
	err_when( !is_wall() );

	if(damage >= WALL_DEFENCE + MIN_WALL_DAMAGE)    // damage >= 8, damage -= 5
		damage -= WALL_DEFENCE;
	else if( damage >= MIN_WALL_DAMAGE )            // 3 <= damage < 8, damage = 3
		damage = MIN_WALL_DAMAGE;
	else if( damage <= 0)                           // 0 < damage < 3, no change to
		return wall_hit_point();                     // no change to hit point to damage

	int hp = wall_hit_point();
	if( hp > 0)
	{
		hp-= damage;
		if( hp <= 0)
		{
			hp = 0;
			remove_wall();
			return 0;
		}
	}
	else if( hp < 0)
	{
		hp+= damage;
		if( hp >= 0)
		{
			hp = 0;
			remove_wall();
			return 0;
		}
	}
	cargo_recno = (hp << 8) | wall_town_recno();
	return hp;
}
//------------ End of function Location::attack_wall ------------//


//---------- Begin of function Location::remove_wall ------------//
//
// <int> setTimeOut   call set_wall_timeout to refuse building wall at the same place
//                    0 to disable timeout, -1 to take default: 10
//
void Location::remove_wall(int setTimeOut)
{
	err_when( !is_wall() );

	loc_flag &= ~(LOCATE_BLOCK_MASK | LOCATE_SITE_MASK);
	extra_para  = 0;
	cargo_recno = 0;
	walkable_reset();

	if( setTimeOut < 0)
		set_wall_timeout( DEFAULT_WALL_TIMEOUT );
	else if( setTimeOut > 0)
	{
		err_when( setTimeOut > 255 );
		set_wall_timeout( setTimeOut );
	}


	err_when(is_firm());
}
//------------ End of function Location::remove_wall ------------//


//---------- Begin of function Location::set_plant ------------//
//
void Location::set_plant(int plantId, int offsetX, int offsetY)
{
	err_when( !can_add_plant() || !plantId );

	walkable_off();
	loc_flag = loc_flag & ~(LOCATE_BLOCK_MASK | LOCATE_SITE_MASK )
		| (LOCATE_IS_PLANT | LOCATE_SITE_RESERVED);

	extra_para  = plantId;
	cargo_recno = (offsetY<<8) + offsetX;
	err_when(cargo_recno==0 || is_firm());
}
//------------ End of function Location::set_plant ------------//


//---------- Begin of function Location::remove_plant ------------//
//
void Location::remove_plant()
{
	err_when( !is_plant() );

	loc_flag &= ~(LOCATE_BLOCK_MASK | LOCATE_SITE_MASK);
	extra_para  = 0;
	cargo_recno = 0;
	walkable_reset();

	err_when(is_firm());
}
//------------ End of function Location::remove_plant ------------//


//---------- Begin of function Location::set_rock ------------//
//
void Location::set_rock(short rockArrayRecno)
{
	err_when( !can_add_rock(3) || !rockArrayRecno );
	walkable_off();
	loc_flag = loc_flag & ~LOCATE_BLOCK_MASK | LOCATE_IS_ROCK;

	cargo_recno = rockArrayRecno;
}
//------------ End of function Location::set_rock ------------//


//---------- Begin of function Location::remove_rock ------------//
//
void Location::remove_rock()
{
	err_when( !is_rock() );

	loc_flag &= ~LOCATE_BLOCK_MASK;
	cargo_recno = 0;
	walkable_reset();
}
//------------ End of function Location::remove_rock ------------//


//-------- Begin of function Location::has_unit --------//
// return 0 or unit recno
int Location::has_unit(int mobileType)
{
	switch(mobileType)
	{
		// #### patch begin Gilbert 5/8 #######//
		case UNIT_LAND:
			if( walkable() )
				return cargo_recno;
			break;

		case UNIT_SEA:
			if( sailable() )
				return cargo_recno;
			break;

		case UNIT_AIR:
			return air_cargo_recno;
			break;
		// #### patch end Gilbert 5/8 #######//
	}

	return 0;
}
//-------- End of function Location::has_unit --------//


//-------- Begin of function Location::has_any_unit --------//
// <int> mobileType - (default: UNIT_LAND)
//
// return the mobile_type if there is any unit here
// return 0 otherwise
//
int Location::has_any_unit(int mobileType)
{
	if(mobileType==UNIT_LAND)
	{
		if(air_cargo_recno)
			return UNIT_AIR;
		else if(walkable() && cargo_recno)
			return UNIT_LAND;
		else if(sailable() && cargo_recno)
			return UNIT_SEA;	
	}
	else
	{
		if(walkable() && cargo_recno)
			return UNIT_LAND;
		else if(sailable() && cargo_recno)
			return UNIT_SEA;	
		else if(air_cargo_recno)
			return UNIT_AIR;
	}

	return 0;
}
//-------- End of function Location::has_any_unit --------//


//-------- Begin of function Location::get_any_unit --------//
//
// <int&> mobileType - var for returning the mobile type of
//								unit in the location.
//
// return: <int> unit recno of the unit.
//
int Location::get_any_unit(int& mobileType)
{
	if(air_cargo_recno)
	{
		mobileType = UNIT_AIR;
		return air_cargo_recno;
	}
	else if(walkable() && cargo_recno)
	{
		mobileType = UNIT_LAND;
		return cargo_recno;
	}
	else if(sailable() && cargo_recno)
	{
		mobileType = UNIT_SEA;
		return cargo_recno;
	}

	return 0;
}
//-------- End of function Location::get_any_unit --------//


//-------- Begin of function Location::is_unit_group_accessible --------//
//
// Return whether the location can be accessed by the unit of the specific
// unit group id.
//
// return : <int> whether the location can be accessed by the unit of
//						the specific unit group id.
//
int Location::is_unit_group_accessible(int mobileType, DWORD curGroupId)
{
	if(is_accessible(mobileType))
	{
		int unitRecno = unit_recno(mobileType);

		return unitRecno==0 || unit_array[unitRecno]->unit_group_id == curGroupId;
	}

	return 0;
}
//-------- End of function Location::is_unit_group_accessible --------//

//### begin alex 24/6 ###//
//-------- Begin of function Location::set_power_on --------//
void Location::set_power_on()
{
	loc_flag &= ~LOCATE_POWER_OFF;
}
//-------- End of function Location::set_power_on --------//


//-------- Begin of function Location::set_power_off --------//
void Location::set_power_off()
{
	loc_flag |= LOCATE_POWER_OFF;
}
//-------- End of function Location::set_power_off --------//


//-------- Begin of function Location::is_power_off --------//
int Location::is_power_off()
{
	return (loc_flag & LOCATE_POWER_OFF);
}
//-------- End of function Location::is_power_off --------//
//#### end alex 24/6 ####//
