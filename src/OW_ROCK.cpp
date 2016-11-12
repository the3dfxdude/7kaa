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

// Filename    : OW_ROCK.CPP
// Description : generate random rock on map


#include <OWORLD.h>
#include <OROCKRES.h>
#include <OROCK.h>
#include <OTERRAIN.h>


//--------------- begin of function World::can_add_rock ----------//
int World::can_add_rock(short x1, short y1, short x2, short y2)
{
	int x,y;
	err_when(x1 > x2 || y1 > y2);
	for(y = y1; y <= y2; ++y)
		for( x = x1; x <= x2; ++x)
			if( !get_loc(x,y)->can_add_rock(3) )
				return FALSE;
	return TRUE;
}
//--------------- end of function World::can_add_rock ----------//


// --------- begin of function World::add_rock ----------//
// 
// note : make sure the location in the area is free
//
void World::add_rock(short rockRecno, short x1, short y1)
{
	// ------- get the delay remain count for the first frame -----//
	Rock newRock(rockRecno, x1, y1);
	int rockArrayRecno = rock_array.add(&newRock);
	RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);

	for( short dy = 0; dy < rockInfo->loc_height && y1+dy < max_y_loc; ++dy)
	{
		for( short dx = 0; dx < rockInfo->loc_width && x1+dx < max_x_loc; ++dx)
		{
			short rockBlockRecno = rock_res.locate_block(rockRecno, dx, dy);
			if( rockBlockRecno )
			{
				Location *locPtr = get_loc(x1+dx, y1+dy);
				err_when( !locPtr->can_add_rock(3) );
				locPtr->set_rock(rockArrayRecno);
				locPtr->set_power_off();
				set_surr_power_off(x1, y1);
			}
		}
	}
}
// --------- end of function World::add_rock ----------//


// --------- begin of function World::gen_rocks ----------//
void World::gen_rocks(int nGrouped, int nLarge, int nSmall)
{
	// one 'large' (size 1 to 4) at the center
	// and a number 'small' (size 1 to 2) at the surroundings

	const int GAP=4;
	const int HUGE_ROCK_SIZE=6;
	const int LARGE_ROCK_SIZE=4;
	const int SMALL_ROCK_SIZE=2;

	int trial = (nGrouped + nLarge + nSmall) * 2;

	while( (nGrouped > 0 || nLarge > 0 || nSmall > 0) && --trial > 0)
	{
		// generate grouped rocks
		if( nGrouped > 0 )
		{
			short x = (GAP+SMALL_ROCK_SIZE)+misc.random( max_x_loc - LARGE_ROCK_SIZE +1 - 2*(GAP+SMALL_ROCK_SIZE));
			short y = (GAP+SMALL_ROCK_SIZE)+misc.random( max_y_loc - LARGE_ROCK_SIZE +1 - 2*(GAP+SMALL_ROCK_SIZE));
			short x2 = x + LARGE_ROCK_SIZE -1;
			short y2 = y + LARGE_ROCK_SIZE -1;

			if( can_add_rock(x,y, x2,y2) )
			{
				short rockRecno = rock_res.search("R", 1,LARGE_ROCK_SIZE,1,LARGE_ROCK_SIZE,-1,0,
					terrain_res[get_loc(x,y)->terrain_id]->average_type );
				if( !rockRecno )
					continue;

				RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
				x2 = x + rockInfo->loc_width - 1;
				y2 = y + rockInfo->loc_height -1;
				if( rockInfo->valid_terrain(terrain_res[get_loc(x2, y)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x, y2)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x2,y2)->terrain_id]->average_type))
				{
					add_rock(rockRecno, x, y);

					// add other smaller rock
					for(int subTrial = misc.random(14); subTrial > 0 ; --subTrial)
					{
						// sx from x-SMALL_ROCK_SIZE to x+4-1+SMALL_ROCK_SIZE
						short sx = x - SMALL_ROCK_SIZE - GAP + misc.random( LARGE_ROCK_SIZE + SMALL_ROCK_SIZE + 2*GAP);
						short sy = y - SMALL_ROCK_SIZE - GAP + misc.random( LARGE_ROCK_SIZE + SMALL_ROCK_SIZE + 2*GAP);
						short sx2 = sx + SMALL_ROCK_SIZE-1;
						short sy2 = sy + SMALL_ROCK_SIZE-1;

						if( can_add_rock( sx, sy, sx2, sy2))
						{
							short rock2Recno = rock_res.search("R", 1,SMALL_ROCK_SIZE,1,SMALL_ROCK_SIZE,-1,0,
								terrain_res[get_loc(sx,sy)->terrain_id]->average_type );
							if(!rock2Recno)
								continue;

							RockInfo *rock2Info = rock_res.get_rock_info(rock2Recno);
							sx2 = sx + rock2Info->loc_width -1;
							sy2 = sy + rock2Info->loc_height -1;
							if( rock2Info->valid_terrain(terrain_res[get_loc(sx2,sy)->terrain_id]->average_type)
								&& rock2Info->valid_terrain(terrain_res[get_loc(sx,sy2)->terrain_id]->average_type)
								&& rock2Info->valid_terrain(terrain_res[get_loc(sx2,sy2)->terrain_id]->average_type) )
							{
								add_rock(rock2Recno, sx, sy);
							}
						}
					}
					nGrouped--;
				}
			}
		}

		// generate stand-alone large rock
		if( nLarge > 0 )
		{
			short x = misc.random( max_x_loc - HUGE_ROCK_SIZE);
			short y = misc.random( max_y_loc - HUGE_ROCK_SIZE);
			short x2 = x + HUGE_ROCK_SIZE -1;
			short y2 = y + HUGE_ROCK_SIZE -1;

			if( can_add_rock( x, y, x2, y2) )
			{
				short rockRecno = rock_res.search("R", SMALL_ROCK_SIZE+1,HUGE_ROCK_SIZE,LARGE_ROCK_SIZE+1,HUGE_ROCK_SIZE,-1,0,
					terrain_res[get_loc(x,y)->terrain_id]->average_type );
				if( !rockRecno )
					continue;

				RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
				x2 = x + rockInfo->loc_width - 1;
				y2 = y + rockInfo->loc_height -1;
				if( rockInfo->valid_terrain(terrain_res[get_loc(x2, y)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x, y2)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x2,y2)->terrain_id]->average_type))
				{
					add_rock(rockRecno, x, y);
					nLarge--;
				}
			}
		}

		// generate stand-alone small rock
		if( nSmall > 0)
		{
			short x = misc.random( max_x_loc - SMALL_ROCK_SIZE);
			short y = misc.random( max_y_loc - SMALL_ROCK_SIZE);
			short x2 = x + SMALL_ROCK_SIZE -1;
			short y2 = y + SMALL_ROCK_SIZE -1;

			if( can_add_rock( x, y, x2, y2) )
			{
				short rockRecno = rock_res.search("R", 1,SMALL_ROCK_SIZE,1,SMALL_ROCK_SIZE,-1,0,
					terrain_res[get_loc(x,y)->terrain_id]->average_type );
				if( !rockRecno )
					continue;

				RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
				x2 = x + rockInfo->loc_width - 1;
				y2 = y + rockInfo->loc_height -1;
				if( rockInfo->valid_terrain(terrain_res[get_loc(x2, y)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x, y2)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x2,y2)->terrain_id]->average_type))
				{
					add_rock(rockRecno, x, y);
					nSmall--;
				}
			}
		}
	}
}
// --------- end of function World::gen_rocks ----------//


//--------------- begin of function World::can_add_dirt ----------//
int World::can_add_dirt(short x1, short y1, short x2, short y2)
{
	int x,y;
	err_when(x1 > x2 || y1 > y2);
	for(y = y1; y <= y2; ++y)
		for( x = x1; x <= x2; ++x)
			if( !get_loc(x,y)->can_add_dirt() )
				return FALSE;
	return TRUE;
}
//--------------- end of function World::can_add_dirt ----------//


// --------- begin of function World::add_dirt ----------//
//
// note : make sure the location in the area is free
//
void World::add_dirt(short dirtRecno, short x1, short y1)
{
	if( dirt_array.size() >= 255 )
		return;

	// ------- get the delay remain count for the first frame -----//
	Rock newDirt(dirtRecno, x1, y1);
	int dirtArrayRecno = dirt_array.add(&newDirt);

	if( dirtArrayRecno >= 255)		// Location::extra_para is only BYTE
		return;

	RockInfo *dirtInfo = rock_res.get_rock_info(dirtRecno);

	for( short dy = 0; dy < dirtInfo->loc_height && y1+dy < max_y_loc; ++dy)
	{
		for( short dx = 0; dx < dirtInfo->loc_width && x1+dx < max_x_loc; ++dx)
		{
			short dirtBlockRecno = rock_res.locate_block(dirtRecno, dx, dy);
			if( dirtBlockRecno )
			{
				Location *locPtr = get_loc(x1+dx, y1+dy);
				err_when( !locPtr->can_add_dirt() );
				locPtr->set_dirt(dirtArrayRecno);

				if( dirtInfo->rock_type == DIRT_BLOCKING_TYPE )
					locPtr->walkable_off();
			}
		}
	}
}
// --------- end of function World::add_rock ----------//


// --------- begin of function World::gen_dirt ----------//
void World::gen_dirt(int nGrouped, int nLarge, int nSmall)
{
	// one 'large' (size 1 to 4) at the center
	// and a number 'small' (size 1 to 2) at the surroundings

	const int GAP=4;
	const int HUGE_ROCK_SIZE=6;
	const int LARGE_ROCK_SIZE=4;
	const int SMALL_ROCK_SIZE=2;

	int trial = (nGrouped + nLarge + nSmall) * 2;

	while( (nGrouped > 0 || nLarge > 0 || nSmall > 0) && --trial > 0)
	{
		// generate grouped dirt
		if( nGrouped > 0 )
		{
			short x = (GAP+SMALL_ROCK_SIZE)+misc.random( max_x_loc - LARGE_ROCK_SIZE +1 - 2*(GAP+SMALL_ROCK_SIZE));
			short y = (GAP+SMALL_ROCK_SIZE)+misc.random( max_y_loc - LARGE_ROCK_SIZE +1 - 2*(GAP+SMALL_ROCK_SIZE));
			short x2 = x + LARGE_ROCK_SIZE -1;
			short y2 = y + LARGE_ROCK_SIZE -1;

			if( can_add_dirt(x,y, x2,y2) )
			{
				short rockRecno = rock_res.search("DE", 1,LARGE_ROCK_SIZE,1,LARGE_ROCK_SIZE,-1,0,
					terrain_res[get_loc(x,y)->terrain_id]->average_type );
				if( !rockRecno )
					continue;

				RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
				x2 = x + rockInfo->loc_width - 1;
				y2 = y + rockInfo->loc_height -1;
				if( rockInfo->valid_terrain(terrain_res[get_loc(x2, y)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x, y2)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x2,y2)->terrain_id]->average_type))
				{
					add_dirt(rockRecno, x, y);

					// add other smaller rock
					for(int subTrial = misc.random(14); subTrial > 0 ; --subTrial)
					{
						// sx from x-SMALL_ROCK_SIZE to x+4-1+SMALL_ROCK_SIZE
						short sx = x - SMALL_ROCK_SIZE - GAP + misc.random( LARGE_ROCK_SIZE + SMALL_ROCK_SIZE + 2*GAP);
						short sy = y - SMALL_ROCK_SIZE - GAP + misc.random( LARGE_ROCK_SIZE + SMALL_ROCK_SIZE + 2*GAP);
						short sx2 = sx + SMALL_ROCK_SIZE-1;
						short sy2 = sy + SMALL_ROCK_SIZE-1;

						if( can_add_dirt( sx, sy, sx2, sy2))
						{
							short rock2Recno = rock_res.search("DE", 1,SMALL_ROCK_SIZE,1,SMALL_ROCK_SIZE,-1,0,
								terrain_res[get_loc(sx,sy)->terrain_id]->average_type );
							if(!rock2Recno)
								continue;

							RockInfo *rock2Info = rock_res.get_rock_info(rock2Recno);
							sx2 = sx + rock2Info->loc_width -1;
							sy2 = sy + rock2Info->loc_height -1;
							if( rock2Info->valid_terrain(terrain_res[get_loc(sx2,sy)->terrain_id]->average_type)
								&& rock2Info->valid_terrain(terrain_res[get_loc(sx,sy2)->terrain_id]->average_type)
								&& rock2Info->valid_terrain(terrain_res[get_loc(sx2,sy2)->terrain_id]->average_type) )
							{
								add_dirt(rock2Recno, sx, sy);
							}
						}
					}
					nGrouped--;
				}
			}
		}

		// generate stand-alone large dirt
		if( nLarge > 0 )
		{
			short x = misc.random( max_x_loc - HUGE_ROCK_SIZE);
			short y = misc.random( max_y_loc - HUGE_ROCK_SIZE);
			short x2 = x + HUGE_ROCK_SIZE -1;
			short y2 = y + HUGE_ROCK_SIZE -1;

			if( can_add_dirt( x, y, x2, y2) )
			{
				short rockRecno = rock_res.search("DE", SMALL_ROCK_SIZE+1,HUGE_ROCK_SIZE,SMALL_ROCK_SIZE+1,HUGE_ROCK_SIZE,-1,0,
					terrain_res[get_loc(x,y)->terrain_id]->average_type );
				if( !rockRecno )
					continue;

				RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
				x2 = x + rockInfo->loc_width - 1;
				y2 = y + rockInfo->loc_height -1;
				if( rockInfo->valid_terrain(terrain_res[get_loc(x2, y)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x, y2)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x2,y2)->terrain_id]->average_type))
				{
					add_dirt(rockRecno, x, y);
					nLarge--;
				}
			}
		}

		// generate stand-alone small dirt
		if( nSmall > 0 )
		{
			short x = misc.random( max_x_loc - SMALL_ROCK_SIZE);
			short y = misc.random( max_y_loc - SMALL_ROCK_SIZE);
			short x2 = x + SMALL_ROCK_SIZE -1;
			short y2 = y + SMALL_ROCK_SIZE -1;

			if( can_add_dirt( x, y, x2, y2) )
			{
				short rockRecno = rock_res.search("DE", 1,SMALL_ROCK_SIZE,1,SMALL_ROCK_SIZE,-1,0,
					terrain_res[get_loc(x,y)->terrain_id]->average_type );
				if( !rockRecno )
					continue;

				RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
				x2 = x + rockInfo->loc_width - 1;
				y2 = y + rockInfo->loc_height -1;
				if( rockInfo->valid_terrain(terrain_res[get_loc(x2, y)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x, y2)->terrain_id]->average_type)
					&& rockInfo->valid_terrain(terrain_res[get_loc(x2,y2)->terrain_id]->average_type))
				{
					add_dirt(rockRecno, x, y);
					nSmall--;
				}
			}
		}
	}
}
// --------- end of function World::gen_dirt ----------//
