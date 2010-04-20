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

// Filename    : OW_WALL.CPP
// Description : class World for building wall
// Ownership   : Gilbert

#include <OWORLD.h>
#include <OMATRIX.h>
#include <WALLTILE.h>
#include <OWALLRES.h>
#include <OTOWN.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <ONATIONA.h>

#define WALL_GROWTH_RATE 6

//--------------- begin of function World::can_build_area ----------//
int World::can_build_area(short x1, short y1, short x2, short y2)
{
	int x,y;
	err_when(x1 > x2 || y1 > y2);
	for(y = y1; y <= y2; ++y)
		for( x = x1; x <= x2; ++x)
			if( ! get_loc(x,y)->can_build_wall())
				return FALSE;
	return TRUE;
}
//--------------- end of function World::can_build_area ----------//

//--------------- begin of function World::build_wall ----------//
void World::build_wall(int townRecno, short initHp)
{
	Town* townPtr = town_array[townRecno];

	int xLoc1 = MAX(0, townPtr->loc_x1-WALL_SPACE_LOC);
	int yLoc1 = MAX(0, townPtr->loc_y1-WALL_SPACE_LOC);
	int xLoc2 = MIN(MAX_WORLD_X_LOC-1, townPtr->loc_x2+WALL_SPACE_LOC);
	int yLoc2 = MIN(MAX_WORLD_Y_LOC-1, townPtr->loc_y2+WALL_SPACE_LOC);

	//--------------- build city wall -------------//
	build_wall_section(xLoc1, yLoc1, xLoc2, yLoc2, townRecno, initHp);
}
//--------------- end of function World::build_wall ----------//

//--------------- begin of function World::build_wall_section ----------//
void World::build_wall_section(short x1, short y1, short x2, short y2,
	short townRecno, short initHp)
{
	//--------- build west wall -----------//
	Location *locPtr;
	short westWallFirst;
	for( westWallFirst = y1; westWallFirst < y2; ++westWallFirst)
	{
		if( get_loc(x1,westWallFirst)->can_build_wall() )
		{
			//--------- find place for the west gate ---------//
			if( x1 >= GATE_WIDTH-1 && y2-y1 >= GATE_LENGTH)
			{
				//----- try from the middle, and then butterfly outward ---//
				short increment = 0;
				short trial, westGateY;
				for( trial=y2-y1-GATE_LENGTH, westGateY=y1+(trial+1)/2;
					trial > 0; --trial, westGateY += increment)
				{
					if(can_build_area(x1-GATE_WIDTH+1, westGateY,
						x1, westGateY+GATE_LENGTH-1) )
					{
						//-------- put west gate --------- //
						build_west_gate(x1-GATE_WIDTH+1, westGateY, townRecno, initHp);
						break;
					}
					increment = -increment;
					if( increment >= 0)
						++increment;
					else
						--increment;
				}
				//---------- put other wall ----------//
				build_west_wall(x1, y1, y2, townRecno, initHp);

				//--------- open west gate ----------//
				open_west_gate(x1, westGateY, townRecno);

			}
			else
			{
				//-------- x1 too small for gate -------//
				get_loc(x1,westWallFirst)->set_wall(NTOWER, townRecno, initHp);
				get_loc(x1,westWallFirst)->set_fire_src(-50);
			}
			break;
		}
	}


	//--------- build east wall -----------
	short eastWallFirst;
	for( eastWallFirst = y1; eastWallFirst < y2; ++eastWallFirst)
	{
		if( get_loc(x2, eastWallFirst)->can_build_wall())
		{
			//---------- find place for the gate --------- //
			if( x2 <= max_x_loc - GATE_WIDTH +1 && y2-y1 >= GATE_LENGTH)
			{
				//---- try from the middle, and then butterfly outward ----//
				short increment = 0;
				short trial, eastGateY;
				for( trial=y2-y1-GATE_LENGTH, eastGateY=y1+(trial+1)/2;
					trial > 0; --trial, eastGateY += increment)
				{
					if(can_build_area(x2, eastGateY,	x2+GATE_WIDTH-1, eastGateY+GATE_LENGTH-1) )
					{
						//----------- put east gate ----------//
						build_east_gate(x2, eastGateY, townRecno, initHp);
						break;
					}
					increment = -increment;
					if( increment >= 0)
						++increment;
					else
						--increment;
				}
				//-------- put other wall ---------//
				build_east_wall(x2, y1, y2, townRecno, initHp);

				//--------- open east gate ----------//
				open_east_gate(x2, eastGateY, townRecno);
			}
			else
			{
				//------- x2 too big for gate ---------//
				get_loc(x2,eastWallFirst)->set_wall(NETOWER, townRecno, initHp);
				get_loc(x2,eastWallFirst)->set_fire_src(-50);
			}
			break;
		}
	}

	//--------- build north wall -----------//
	short northWallFirst;
	for( northWallFirst = x1; northWallFirst < x2; ++northWallFirst)
	{
		if( (locPtr = get_loc(northWallFirst, y1))->can_build_wall() ||
			( locPtr->is_wall() && locPtr->wall_town_recno() == townRecno) )
		{
			//-------- find place for the north gate --------//
			if( y1 >= GATE_WIDTH-1 && x2-x1 >= GATE_LENGTH)
			{
				//------- try from the middle, and then butterfly outward -------//
				short increment = 0;
				short trial, northGateX;
				for( trial=x2-x1-GATE_LENGTH, northGateX=x1+(trial+1)/2;
					trial > 0; --trial, northGateX += increment)
				{
					if(can_build_area(northGateX, y1-GATE_WIDTH+1, 
						northGateX+GATE_LENGTH-1, y1) )
					{
						//--------------- put north gate ----------------//
						build_north_gate(northGateX, y1-GATE_WIDTH+1, townRecno, initHp);
						break;
					}
					increment = -increment;
					if( increment >= 0)
						++increment;
					else
						--increment;
				}
				//----------- put other wall --------------//
				build_north_wall(x1, x2, y1, townRecno, initHp);

				//--------- open north gate ----------//
				open_north_gate(northGateX, y1, townRecno);

			}
			else
			{
				//------------ y1 too small for gate -----------//
				if(locPtr->can_build_wall() )
				{
					locPtr->set_wall(NTOWER, townRecno, initHp);
					locPtr->set_fire_src(-50);
				}
			}
			break;
		}
	}


	//--------- build south wall -----------
	short southWallFirst;
	for( southWallFirst = x1; southWallFirst < x2; ++southWallFirst)
	{
		if( (locPtr = get_loc(southWallFirst, y2))->can_build_wall() ||
			( locPtr->is_wall() && locPtr->wall_town_recno() == townRecno) )
		{
			//---------- find place for the south gate ----------//
			if( y2 <= max_y_loc-GATE_WIDTH+1 && x2-x1 >= GATE_LENGTH)
			{
				//----- try from the middle, and then butterfly outward ----//
				short increment = 0;
				short trial, southGateX;
				for( trial=x2-x1-GATE_LENGTH, southGateX=x1+(trial+1)/2;
					trial > 0; --trial, southGateX += increment)
				{
					if(can_build_area(southGateX, y2, 
						southGateX+GATE_LENGTH-1, y2+GATE_WIDTH-1) )
					{
						//----------- put south gate -------------//
						build_south_gate(southGateX, y2, townRecno, initHp);
						break;
					}
					increment = -increment;
					if( increment >= 0)
						++increment;
					else
						--increment;
				}
				//----------- put other wall -------------//
				build_south_wall(x1, x2, y2, townRecno, initHp);

				//--------- open north gate ----------//
				open_south_gate(southGateX, y2, townRecno);

			}
			else
			{
				//---------- y2 too big for gate ----------//
				if(locPtr->can_build_wall() )
				{
					locPtr->set_wall(STOWER, townRecno, initHp);
					locPtr->set_fire_src(-50);
				}
			}
			break;
		}
	}

}
//--------------- end of function World::build_wall_section ----------//

//--------------- begin of function World::build_west_gate ----------//
void World::build_west_gate(short x1, short y1, short townRecno, short initHp)
{
	//--------- put two gate towers -----------//
	Location *locPtr = get_loc(x1,y1);
	locPtr->set_wall(WGATE_NTOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(WGATE_NTOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1, y1+1);
	locPtr->set_wall(WGATE_NTOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(WGATE_NTOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1,y1+GATE_LENGTH-2);
	locPtr->set_wall(WGATE_STOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(WGATE_STOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1, y1+GATE_LENGTH-1);
	locPtr->set_wall(WGATE_STOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(WGATE_STOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	//---------- put gate -----------//
	char gateId = WGATE_BASE;
	for(short y = 2; y < GATE_LENGTH-2; ++y)
	{
		locPtr = get_loc(x1, y1+y);
		for(short x = 0; x < GATE_WIDTH; ++x, ++locPtr)
		{
			locPtr->set_wall(gateId++, townRecno, initHp);
			locPtr->set_fire_src(-50);
		}
	}
}
//--------------- end of function World::build_west_gate ----------//

//--------------- begin of function World::build_west_wall ----------//
void World::build_west_wall(short x1, short y1, short y2, short townRecno,
	short initHp)
{
	//---------- find segments of buildable terrain ---------//
	short startY = y1, endY = y1;
	while(startY <= y2)
	{
		for( ; startY <= y2 && !get_loc(x1,startY)->can_build_wall(); ++startY);
		if( startY <= y2)		// a place is found
		{
			//-------- found segment end -----------//
			for( endY = startY+1; endY <= y2 && get_loc(x1,endY)->can_build_wall() ;++endY);
			--endY;
			
			if( startY == endY)
			{
				//------- equal square, draw a single tower --------//
				get_loc(x1, startY)->set_wall(SINGLE_TOWER, townRecno, initHp);
				get_loc(x1, startY)->set_fire_src(-50);
			}
			else
			{
				short startSquare = NTOWER;
				short endSquare = STOWER;
				// if startY-1 is a wall, hence a gate, start with a wall with shadow
				if( startY > y1 && get_loc(x1, startY-1)->is_wall() )
				{
					startSquare = NSWALL_SHADOW;
				}
				// if endY+1 is a wall, hence a gate, start with a wall
				if( endY < y2 && get_loc(x1, endY+1)->is_wall() )
				{
					endSquare = NSWALL;
				}
				for( short y = startY; y < endY; ++y)
				{
					get_loc(x1, y)->set_wall(startSquare, townRecno, initHp);
					get_loc(x1, y)->set_fire_src(-50);

					switch(startSquare)
					{
					case NTOWER:
						startSquare = NSWALL_SHADOW;
						break;
					case NSWALL_SHADOW:
						startSquare = NSWALL;
						break;
					// otherwise unchange
					}
				}
				get_loc(x1, endY)->set_wall(endSquare, townRecno, initHp);
				get_loc(x1, endY)->set_fire_src(-50);
			}
			startY = endY +1;
		}
	}
}
//--------------- end of function World::build_west_wall ----------//

//--------------- begin of function World::open_west_gate ----------//
void World::open_west_gate(short x2, short y1, short townRecno)
{
	//------ check if any west gate tile is built ---------//
	Location *locPtr = get_loc(x2,y1+2);
	if( locPtr->is_wall() && locPtr->wall_id() == WGATE_N &&
		locPtr->wall_town_recno() == townRecno)
	{
		for(short y = 3; y < GATE_LENGTH-3; ++y)
			for(short x = 0; x < GATE_WIDTH; ++x)
				get_loc(x2-x, y1+y)->remove_wall();

	}
}
//--------------- end of function World::open_west_gate ----------//

//--------------- begin of function World::build_east_gate ----------//
void World::build_east_gate(short x1, short y1, short townRecno, short initHp)
{
	//---------- put two gate towers ----------//
	Location *locPtr = get_loc(x1,y1);
	locPtr->set_wall(EGATE_NTOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(EGATE_NTOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1, y1+1);
	locPtr->set_wall(EGATE_NTOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(EGATE_NTOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1,y1+GATE_LENGTH-2);
	locPtr->set_wall(EGATE_STOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(EGATE_STOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1, y1+GATE_LENGTH-1);
	locPtr->set_wall(EGATE_STOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(EGATE_STOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	//------------- put gate ----------------//
	char gateId = EGATE_BASE;
	for(short y = 2; y <  GATE_LENGTH-2; ++y)
	{
		locPtr = get_loc(x1, y+y1);
		for(short x = 0; x < GATE_WIDTH; ++x, ++locPtr)
		{
			locPtr->set_wall(gateId++, townRecno, initHp);
			locPtr->set_fire_src(-50);
		}
	}
}
//--------------- end of function World::build_east_gate ----------//

//--------------- begin of function World::build_east_wall ----------//
void World::build_east_wall(short x1, short y1, short y2, short townRecno,
	short initHp)
{
	//--------- find segments of buildable terrain ---------//
	short startY = y1, endY = y1;
	while(startY <= y2)
	{
		for( ; startY <= y2 && !get_loc(x1,startY)->can_build_wall(); ++startY);
		if( startY <= y2)		// a place is found
		{
			//-------- found segment end -------------//
			for( endY = startY+1; endY <= y2 && get_loc(x1,endY)->can_build_wall() ;++endY);
			--endY;
			
			if( startY == endY)
			{
				//----------- equal square, draw a single tower --------//
				get_loc(x1, startY)->set_wall(SINGLE_TOWER, townRecno, initHp);
				get_loc(x1, startY)->set_fire_src(-50);
			}
			else
			{
				short startSquare = NTOWER;
				short endSquare = STOWER;
				// if startY-1 is a wall, hence a gate, start with a wall with shadow
				if( startY > y1 && get_loc(x1, startY-1)->is_wall() )
				{
					startSquare = NSWALL_SHADOW;
				}
				// if endY+1 is a wall, hence a gate, start with a wall
				if( endY < y2 && get_loc(x1, endY+1)->is_wall() )
				{
					endSquare = NSWALL;
				}
				for( short y = startY; y < endY; ++y)
				{
					get_loc(x1, y)->set_wall(startSquare, townRecno, initHp);
					get_loc(x1, y)->set_fire_src(-50);
					switch(startSquare)
					{
					case NTOWER:
						startSquare = NSWALL_SHADOW;
						break;
					case NSWALL_SHADOW:
						startSquare = NSWALL;
						break;
					// otherwise unchange
					}
				}
				get_loc(x1, endY)->set_wall(endSquare, townRecno, initHp);
				get_loc(x1, endY)->set_fire_src(-50);
			}
			startY = endY +1;
		}
	}
}
//--------------- end of function World::build_east_wall ----------//

//--------------- begin of function World::open_east_gate ----------//
void World::open_east_gate(short x1, short y1, short townRecno)
{
	//------ check if any east gate tile is built ---------//
	Location *locPtr = get_loc(x1,y1+2);
	if( locPtr->is_wall() && locPtr->wall_id() == EGATE_N &&
		locPtr->wall_town_recno() == townRecno)
	{
		for(short y = 3; y < GATE_LENGTH-3; ++y)
			for(short x = 0; x < GATE_WIDTH; ++x)
				get_loc(x1+x, y1+y)->remove_wall();

	}
}
//--------------- end of function World::open_east_gate ----------//

//--------------- begin of function World::build_north_gate ----------//
void World::build_north_gate(short x1, short y1, short townRecno,
	short initHp)
{
	//---------- put two gate towers -----------//
	Location *locPtr = get_loc(x1,y1);
	locPtr->set_wall(NGATE_WTOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(NGATE_WTOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1+GATE_LENGTH-2, y1);
	locPtr->set_wall(NGATE_ETOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(NGATE_ETOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1,y1+1);
	locPtr->set_wall(NGATE_WTOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(NGATE_WTOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1+GATE_LENGTH-2, y1+1);
	locPtr->set_wall(NGATE_ETOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(NGATE_ETOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	//------------ put gate -------------//
	char gateId = NGATE_BASE;
	for(short y = 0; y < GATE_WIDTH; ++y)
	{
		locPtr = get_loc(x1+2, y1+y);
		for(short x = 2; x < GATE_LENGTH-2; ++x, ++locPtr)
		{
			locPtr->set_wall(gateId++, townRecno, initHp);
			locPtr->set_fire_src(-50);
		}
	}
}
//--------------- end of function World::build_north_gate ----------//

//--------------- begin of function World::build_north_wall ----------//
void World::build_north_wall(short x1, short x2, short y1, short townRecno,
	short initHp)
{
	//---------- find segments of buildable terrain ---------//
	short startX = x1, endX = x1;
	while(startX <= x2)
	{
		for( ; startX <= x2 && !get_loc(startX, y1)->can_build_wall(); ++startX);
		if( startX <= x2)		// a place is found
		{
			//--------- found segment end ----------//
			for( endX = startX+1; endX <= x2 && get_loc(endX, y1)->can_build_wall() ;++endX);
			--endX;
			
			if( startX == endX)
			{
				//---------- equal square, draw a single tower ---------//
				get_loc(startX, y1)->set_wall(SINGLE_TOWER, townRecno, initHp);
				get_loc(startX, y1)->set_fire_src(-50);
			}
			else
			{
				short startSquare = WTOWER;
				short endSquare = ETOWER;
				// if startX-1 is a wall, hence a gate, start with a wall with shadow
				if( startX > x1 && get_loc(startX-1, y1)->is_wall() )
				{
					startSquare = EWWALL_SHADOW;

					// if startX-1 is a NTOWER (corner) , change it to NWTOWER
					Location *leftLoc;
					if( (leftLoc = get_loc(startX-1, y1))->wall_id() == NTOWER
						&& leftLoc->wall_town_recno() == townRecno )
					{
						leftLoc->remove_wall();
						leftLoc->set_wall(NWTOWER, townRecno, initHp);
					}
				}
				// if endX+1 is a wall, hence a gate, start with a wall
				if( endX < x2 && get_loc(endX+1, y1)->is_wall() )
				{
					endSquare = EWWALL;

					// if endX+1 is a NTOWER (corner) , change it to NETOWER
					Location *rightLoc;
					if( (rightLoc = get_loc(endX+1,y1))->wall_id() == NTOWER
						&& rightLoc->wall_town_recno() == townRecno)
					{
						rightLoc->remove_wall();
						rightLoc->set_wall(NETOWER, townRecno, initHp);
					}
				}
				for( short x = startX; x < endX; ++x)
				{
					get_loc(x, y1)->set_wall(startSquare, townRecno, initHp);
					get_loc(x, y1)->set_fire_src(-50);
					switch(startSquare)
					{
					case WTOWER:
						startSquare = EWWALL_SHADOW;
						break;
					case EWWALL_SHADOW:
						startSquare = EWWALL;
						break;
					// otherwise unchange
					}
				}
				get_loc(endX, y1)->set_wall(endSquare, townRecno, initHp);
				get_loc(endX, y1)->set_fire_src(-50);
			}
			startX = endX +1;
		}
	}
}
//--------------- end of function World::build_north_wall ----------//

//--------------- begin of function World::open_north_gate ----------//
void World::open_north_gate(short x1, short y2, short townRecno)
{
	//------ check if any north gate tile is built ---------//
	Location *locPtr = get_loc(x1+2,y2);
	if( locPtr->is_wall() && locPtr->wall_id() == NGATE_W &&
		locPtr->wall_town_recno() == townRecno)
	{
		for(short x = 3; x < GATE_LENGTH-3; ++x)
			for(short y = 0; y < GATE_WIDTH; ++y)
				get_loc(x1+x, y2-y)->remove_wall();

	}
}
//--------------- end of function World::open_north_gate ----------//

//--------------- begin of function World::build_south_gate ----------//
void World::build_south_gate(short x1, short y1, short townRecno, short initHp)
{
	//---------- put two gate towers ----------//
	Location *locPtr = get_loc(x1,y1);
	locPtr->set_wall(SGATE_WTOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(SGATE_WTOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1+GATE_LENGTH-2, y1);
	locPtr->set_wall(SGATE_ETOWER_NW, townRecno, initHp);
	(locPtr+1)->set_wall(SGATE_ETOWER_NE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1,y1+1);
	locPtr->set_wall(SGATE_WTOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(SGATE_WTOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	locPtr = get_loc(x1+GATE_LENGTH-2, y1+1);
	locPtr->set_wall(SGATE_ETOWER_SW, townRecno, initHp);
	(locPtr+1)->set_wall(SGATE_ETOWER_SE, townRecno, initHp);
	locPtr->set_fire_src(-50);
	(locPtr+1)->set_fire_src(-50);

	//------------ put gate --------------//
	char gateId = SGATE_BASE;
	for(short y = 0; y < GATE_WIDTH; ++y)
	{
		locPtr = get_loc(x1+2, y1+y);
		for(short x = 2; x < GATE_LENGTH-2; ++x, ++locPtr)
		{
			locPtr->set_wall(gateId++, townRecno, initHp);
			locPtr->set_fire_src(-50);
		}
	}
}
//--------------- end of function World::build_south_gate ----------//

//--------------- begin of function World::build_south_wall ----------//
void World::build_south_wall(short x1, short x2, short y1, short townRecno,
	short initHp)
{
	//----------- find segments of buildable terrain -------------//
	short startX = x1, endX = x1;
	while(startX <= x2)
	{
		for( ; startX <= x2 && !get_loc(startX, y1)->can_build_wall(); ++startX);
		if( startX <= x2)		// a place is found
		{
			//---------- found segment end ---------//
			for( endX = startX+1; endX <= x2 && get_loc(endX, y1)->can_build_wall() ;++endX);
			--endX;
			
			if( startX == endX)
			{
				//--------- equal square, draw a single tower --------//
				get_loc(startX, y1)->set_wall(SINGLE_TOWER, townRecno, initHp);
				get_loc(startX, y1)->set_fire_src(-50);
			}
			else
			{
				short startSquare = WTOWER;
				short endSquare = ETOWER;
				// if startX-1 is a wall, hence a gate, start with a wall with shadow
				if( startX > x1 && get_loc(startX-1, y1)->is_wall() )
				{
					startSquare = EWWALL_SHADOW;
					// if startX-1 is a STOWER (corner) , change it to SWTOWER
					Location *leftLoc;
					if( (leftLoc = get_loc(startX-1, y1))->wall_id() == STOWER
						&& leftLoc->wall_town_recno() == townRecno )
					{
						leftLoc->remove_wall();
						leftLoc->set_wall(SWTOWER, townRecno, initHp);
					}
				}
				// if endY+1 is a wall, hence a gate, start with a wall with shadow
				if( endX < x2 && get_loc(endX+1, y1)->is_wall() )
				{
					endSquare = EWWALL;

					// if endX+1 is a STOWER (corner) , change it to SETOWER
					Location *rightLoc;
					if( (rightLoc = get_loc(endX+1, y1))->wall_id() == STOWER
						&& rightLoc->wall_town_recno() == townRecno)
					{
						rightLoc->remove_wall();
						rightLoc->set_wall(SETOWER, townRecno, initHp);
					}

				}
				for( short x = startX; x < endX; ++x)
				{
					get_loc(x, y1)->set_wall(startSquare, townRecno, initHp);
					get_loc(x, y1)->set_fire_src(-50);
					switch(startSquare)
					{
					case WTOWER:
						startSquare = EWWALL_SHADOW;
						break;
					case EWWALL_SHADOW:
						startSquare = EWWALL;
						break;
					// otherwise unchange
					}
				}
				get_loc(endX, y1)->set_wall(endSquare, townRecno, initHp);
				get_loc(endX, y1)->set_fire_src(-50);
			}
			startX = endX +1;
		}
	}
}
//--------------- end of function World::build_south_wall ----------//

//--------------- begin of function World::open_south_gate ----------//
void World::open_south_gate(short x1, short y1, short townRecno)
{
	//------ check if any south gate tile is built ---------//
	Location *locPtr = get_loc(x1+2,y1);
	if( locPtr->is_wall() && locPtr->wall_id() == SGATE_W &&
		locPtr->wall_town_recno() == townRecno)
	{
		for(short x = 3; x < GATE_LENGTH-3; ++x)
			for(short y = 0; y < GATE_WIDTH; ++y)
				get_loc(x1+x, y1+y)->remove_wall();

	}
}
//--------------- end of function World::open_south_gate ----------//

//--------------- begin of function World::form_wall -----------//
//
// adjust wall tile, return no. of tile changed
//
int World::form_wall(short x, short y, short maxRecur)
{
	Location *locPtr = get_loc(x,y);
	if( !locPtr->is_wall() || maxRecur < 0)
		return 0;

	int wallTile = locPtr->wall_id();
	int blockChanged = 0;

	int flag = 0;
	if( y == 0 || get_loc(x,y-1)->is_wall())					// north square
		flag |= 1;
	if( x == max_x_loc-1 || get_loc(x+1,y)->is_wall() )	// east square
		flag |= 2;
	if( y == max_y_loc-1 || get_loc(x,y+1)->is_wall() )	// south square
		flag |= 4;
	if( x == 0 || get_loc(x-1,y)->is_wall() )					// west square
		flag |= 8;
	int newWallTile, newWallRubble;

	// ------- find new wall tile ---------//
	switch(flag)
	{
	case 0:			// no surrounding square is wall
		newWallTile = SINGLE_TOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 1:			// only north square is wall
		newWallTile = STOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 2:			// only east
		newWallTile = WTOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 3:			// north and east
		newWallTile = SWTOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 4:			// south only
		newWallTile = NTOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 5:			// north and south
		newWallTile = NSWALL;
		if( y > 0 && get_loc(x,y-1)->is_wall() )
		{
			int northWallTile = get_loc(x,y-1)->wall_id();
			if( northWallTile != NSWALL && northWallTile != NSWALL_SHADOW &&
				!is_wall_rubble(northWallTile))
				newWallTile = NSWALL_SHADOW;
		}
		newWallRubble = NSWALL_CON1;
		break;
	case 6:			// east and south
		newWallTile = NWTOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 7:			// east and north and south
		newWallTile = NWTOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 8:			// west only
		newWallTile = ETOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 9:			// west and north
		newWallTile = SETOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 10:			// west and east
		newWallTile = EWWALL;
		if( x > 0 && get_loc(x-1,y)->is_wall() )
		{
			int westWallTile = get_loc(x-1,y)->wall_id();
			if( westWallTile != EWWALL && westWallTile != EWWALL_SHADOW &&
				!is_wall_rubble(westWallTile) )
				newWallTile = EWWALL_SHADOW;
		}
		newWallRubble = EWWALL_CON1;
		break;
	case 11:			// west and east and north
		newWallTile = ETOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 12:			// west and south
		newWallTile = NETOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 13:			// west and south and north
		newWallTile = NETOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 14:			// west and south and east
		newWallTile = NETOWER;
		newWallRubble = TOWER_CON1;
		break;
	case 15:			// all four
		newWallTile = NETOWER;
		newWallRubble = TOWER_CON1;
		break;
	}

	// --- adjust newWallTile from wall_grade() ------//
	switch(get_loc(x,y)->wall_grade())
	{
	case 1:
		newWallTile = newWallRubble;
		break;
	case -1:
		newWallTile = newWallRubble + TOWER_DES1 - TOWER_CON1; // e.g. TOWER_CON1 -> TOWER_DES1
		break;
	case 2:
		newWallTile = newWallRubble + 1;
		break;
	case -2:
		newWallTile = newWallRubble + TOWER_DES2 - TOWER_CON1;
		break;
	case 3:
		newWallTile = newWallRubble + 2;
		break;
	case -3:
		newWallTile = newWallRubble + TOWER_DES3 - TOWER_CON1;
		break;
	case 4:
	case -4:
		// no change
		break;	
	default:
		err_here();
	}
	
	// ---  change wall tile, such as change to tower, add shadow ----//
	if( wallTile != newWallTile)
	{
		locPtr->chg_wall_id(newWallTile);
		blockChanged ++;
	}
		
	// change adjacent sqaure
	if( y > 0 && get_loc(x,y-1)->is_wall() )
		blockChanged += form_wall(x, y-1, maxRecur-1);
	if( y < max_y_loc-1 && get_loc(x,y+1)->is_wall() )
		blockChanged += form_wall(x, y+1, maxRecur-1);
	if( x < max_x_loc-1 && get_loc(x+1,y)->is_wall() )
		blockChanged += form_wall(x+1, y, maxRecur-1);
	if( x > 0 && get_loc(x-1,y)->is_wall() )
		blockChanged += form_wall(x-1, y, maxRecur-1);

	return blockChanged;
}
//--------------- end of function World::form_wall -----------//


//--------------- begin of function World::correct_wall -----------//
//
// adjust adjacent wall tiles, return no. of tile changed
//
int World::correct_wall(short x, short y, short maxRecur)
{
	if( maxRecur < 0)
		return 0;

	int blockChanged = 0;
	// change adjacent sqaure
	if( y > 0 && get_loc(x,y-1)->is_wall() )
		blockChanged += form_wall(x, y-1, maxRecur-1);
	if( y < max_y_loc-1 && get_loc(x,y+1)->is_wall() )
		blockChanged += form_wall(x, y+1, maxRecur-1);
	if( x < max_x_loc-1 && get_loc(x+1,y)->is_wall() )
		blockChanged += form_wall(x+1, y, maxRecur-1);
	if( x > 0 && get_loc(x-1,y)->is_wall() )
		blockChanged += form_wall(x-1, y, maxRecur-1);

	return blockChanged;
}


//---------- begin of function World::form_world_wall -----//
void World::form_world_wall()
{
	static int init_build_wall_seq = 0;
	#define SPACING 12
	static char build_wall_x_seq[SPACING*SPACING];
	static char build_wall_y_seq[SPACING*SPACING];
	static int	next_build_wall_seq;

	int x, y;

	if(! init_build_wall_seq )
	{
		init_build_wall_seq = 1;
		DWORD seed = m.get_random_seed();
		// ------ initialize with linear sequence----- //
		for( y = 0; y < SPACING; ++y)
		{
			for( x = 0; x < SPACING; ++x)
			{
				build_wall_x_seq[y*SPACING+x] = x;
				build_wall_y_seq[y*SPACING+x] = y;
			}
		}

		m.set_random_seed(176682233); // hard code
		
		//------- shuffle randomly ----------//
		for(int t = SPACING * SPACING -1; t >= 0; --t)
		{
			int u = m.random(SPACING * SPACING);

			// ----- swap build_wall_x/y_seq[t] with [u]
			char tmp;
			tmp = build_wall_x_seq[t];
			build_wall_x_seq[t] = build_wall_x_seq[u];
			build_wall_x_seq[u] = tmp;
			tmp = build_wall_y_seq[t];
			build_wall_y_seq[t] = build_wall_y_seq[u];
			build_wall_y_seq[u] = tmp;
		}
		next_build_wall_seq = 0;
		m.set_random_seed(seed);
	}

	for(int trial = 7; trial > 0 ; --trial)
	{
		for(y = build_wall_y_seq[next_build_wall_seq]; y < max_y_loc; y+= SPACING)
		{
			for(x = build_wall_x_seq[next_build_wall_seq]; x < max_x_loc; x+= SPACING)
			{
				Location *locPtr = get_loc(x,y);
				// ######## begin Gilbert 7/3 ##########//
				if( locPtr->had_wall() )
				{
					locPtr->dec_wall_timeout();
				}
				else if( locPtr->is_wall())
				// ######## end Gilbert 7/3 ##########//
				{
					int prevGrade = locPtr->wall_grade();
					int newGrade;
					if( locPtr->inc_wall_hit_point(WALL_GROWTH_RATE) == 0)
					{
						locPtr->remove_wall();
						if( y > 0 && get_loc(x,y-1)->is_wall() )
							form_wall(x,y-1, 1);
						if( y < max_y_loc-1 && get_loc(x,y+1)->is_wall() )
							form_wall(x,y+1, 1);
						if( x > 0 && get_loc(x-1,y)->is_wall() )
							form_wall(x-1,y, 1);
						if( x < max_x_loc-1 && get_loc(x+1,y)->is_wall() )
							form_wall(x+1,y, 1);
					}
					else if( prevGrade != (newGrade=locPtr->wall_grade()) )
					{
						form_wall(x,y,2);
					}
				}
			}
		}
		next_build_wall_seq = (next_build_wall_seq+1) % (SPACING*SPACING);
	}
}
//---------- end of function World::form_world_wall -----//


//------- Begin of function World::build_wall_tile -------//
//
// <int> xLoc, yLoc  - the location on which the wall should be built
// <int> nationRecno - recno of the builder nation
//
// see also ZoomMatrix->draw_build_marker
//
void World::build_wall_tile(int xLoc, int yLoc, short nationRecno, char remoteAction)
{
	Location *locPtr = get_loc(xLoc, yLoc);
	if( can_build_wall(xLoc, yLoc, nationRecno))
	{
#ifdef USE_DPLAY
		if( !remoteAction && remote.is_enable() )
		{
			// packet structure : <nation recno> <xLoc> <yLoc>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_WALL_BUILD, 3*sizeof(short));
			shortPtr[0] = nationRecno;
			shortPtr[1] = xLoc;
			shortPtr[2] = yLoc;
		}
		else
#endif
		{
			locPtr->set_wall(TOWER_CON1,nationRecno, 1);
			locPtr->set_fire_src(-50);
//			nation_array[nationRecno]->add_expense( (float)BUILD_WALL_COST );
		}
	}
	else if( locPtr->is_wall_destructing() && 
		can_destruct_wall(xLoc, yLoc, nationRecno))
	{
#ifdef USE_DPLAY
		if( !remoteAction && remote.is_enable() )
		{
			// packet structure : <nation recno> <xLoc> <yLoc>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_WALL_BUILD, 3*sizeof(short));
			shortPtr[0] = nationRecno;
			shortPtr[1] = xLoc;
			shortPtr[2] = yLoc;
		}
		else
#endif
		{
			locPtr->set_wall_creating();
//			nation_array[nationRecno]->add_expense( (float)
//				BUILD_WALL_COST * (100-locPtr->wall_abs_hit_point()) / 100 );
		}
	}

}
//--------- End of function World::build_wall_tile ---------//


//------- Begin of function World::destruct_wall_tile -------//
//
// <int> xLoc, yLoc  - the location on which the wall should be destructed
// <int> nationRecno - recno of the destructer nation
//
// see also ZoomMatrix->draw_build_marker
//
void World::destruct_wall_tile(int xLoc, int yLoc, short nationRecno, char remoteAction)
{
	Location *locPtr = get_loc(xLoc, yLoc);

	if( locPtr->is_wall_creating() && can_destruct_wall(xLoc, yLoc, nationRecno) )
	{
#ifdef USE_DPLAY
		if( !remoteAction && remote.is_enable() )
		{
			// packet structure : <nation recno> <xLoc> <yLoc>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_WALL_DESTRUCT, 3*sizeof(short));
			shortPtr[0] = nationRecno;
			shortPtr[1] = xLoc;
			shortPtr[2] = yLoc;
		}
		else
#endif
		{
			locPtr->set_wall_destructing();
//			nation_array[nationRecno]->add_expense( (float) DESTRUCT_WALL_COST );
		}
	}
}
//--------- End of function World::destruct_wall_tile ---------//
