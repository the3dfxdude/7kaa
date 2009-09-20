//Filename    : OGENMAP.CPP
//Description : World Map generation function part 1
//Ownership   : Gilbert

#include <time.h>
#include <stdlib.h>

#include <ALL.h>
#include <OGAME.h>
#include <OVGA.h>
#include <OTERRAIN.h>
#include <OWORLD.h>
#include <OPLASMA.h>
#include <OREGION.h>
#include <OFIRMID.h>

//-------- Begin of function World::generate_map ----------//
//
void World::generate_map()
{
	const int dispProgress = 1;
	const int maxGenMapSteps = 14;
	vga_front.unlock_buf();

	int curGenMapSteps = 0;
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	//--- loc_matrix, first store terrain height, then world map icon id ---//

   loc_matrix = (Location*) mem_resize( loc_matrix, MAX_WORLD_X_LOC * MAX_WORLD_Y_LOC * sizeof(Location) );

   max_x_loc = MAX_WORLD_X_LOC;
	max_y_loc = MAX_WORLD_Y_LOC;
/*
#ifdef DEBUG
	// testing the completeness of Ocean-DarkGrass joint //
	static TerrainTypeCode ta[6] = { TERRAIN_OCEAN,TERRAIN_OCEAN,TERRAIN_OCEAN,
		TERRAIN_DARK_GRASS,TERRAIN_DARK_GRASS,TERRAIN_DARK_GRASS };
	static SubTerrainMask sta[6] = { BOTTOM_MASK, MIDDLE_MASK, TOP_MASK,
		BOTTOM_MASK, MIDDLE_MASK, TOP_MASK };
	int failure = 0;
	int nw, ne, sw, se;
	for( nw = 0; nw < 6; ++nw)
		for( ne = 0; ne < 6; ++ne)
			for( sw = 0; sw < 6; ++sw)
				for( se = 0; se < 6; ++se)
	{
		if(!terrain_res.scan( ta[nw], sta[nw], ta[ne], sta[ne],
			ta[sw], sta[sw], ta[se], sta[se], 1,0,0))
		{
			TerrainTypeCode nwType=ta[nw], neType=ta[ne], swType=ta[sw], seType=ta[se];
			SubTerrainMask nwMask=sta[nw], neMask=sta[ne], swMask=sta[sw], seMask=sta[se];
			failure++;
		};
	}
#endif
*/

	//----------- start generating -----------//

	// ---------- generate plasma map ----------//

	Plasma heightMap;
	memset( loc_matrix , 0, sizeof(Location) * MAX_WORLD_X_LOC * MAX_WORLD_Y_LOC );
	heightMap.init(max_x_loc, max_y_loc);
	heightMap.generate( m.random(2), 5, m.rand() );

	curGenMapSteps++;			// 1
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	// ###### begin Gilbert 27/8 ########//
	// ---------- add base level --------//
	// heightMap.add_base_level(heightMap.calc_tera_base_level(TerrainRes::min_height(TERRAIN_DARK_GRASS)));

	// grouping plasma sample data, find sea or land first
	int	totalLoc = (max_x_loc+1) * (max_y_loc+1);
	short heightLimit[2];
	int	heightFreq[2];
	int minLandCount, maxLandCount;
	int	initHeightLimit = TerrainRes::min_height(TERRAIN_DARK_GRASS);
	switch(config.land_mass)
	{
	case OPTION_LOW:
		minLandCount = totalLoc *4/10;
		maxLandCount = totalLoc *6/10;
		break;
	case OPTION_MODERATE:
		minLandCount = totalLoc *6/10;
		maxLandCount = totalLoc *8/10;
		break;
	case OPTION_HIGH:
		minLandCount = totalLoc *8 /10;
		maxLandCount = totalLoc;
		break;
	default:
		err_here();
	}
	int avgLandCount = (minLandCount + maxLandCount) /2;

	heightLimit[0] = 0;
	heightLimit[1] = initHeightLimit;
	heightMap.stat(2, heightLimit, heightFreq);
	
	int& landCount = heightFreq[1];
	int& seaCount = heightFreq[0];

	int loopCount = 0;
	while( ++loopCount <= 4 && (landCount<minLandCount || landCount>maxLandCount) )
	{
		if( landCount < minLandCount )
		{
			// positive add_base_level to gain more land
			// find a level between 0 to TerrainRes::min_height(TERRAIN_DARK_GRASS)
			// assume heightlevel below heightLimit[1] is evenly distributed,
			// approximate a new heightLimit[1] such that landCount is avgLandCount

			// (heightLimit[1] - newheightLimit[1]) * seaCount / (heightLimit[1] - heightLimit[0]) + landCount = avgLandCount
			
			heightLimit[1] = heightLimit[1] - (avgLandCount - landCount) * (heightLimit[1] - heightLimit[0]) / seaCount;
		}
		else if( landCount > maxLandCount )
		{
			// negative add_base_level to reduce land
			// find a level above TerrainRes::min_height(TERRAIN_DARK_GRASS)
			// assume heightlevel above heightLimit[1] is evenly distributed,
			// approximate a new heightLimit[1] such that landCount is avgLandCount

			const int maxHeightLimit = 255;
			// landCount * (maxHeightLimit - newheightLimit[1])/ (maxHeightLimit - heightLimit[1]) = avgLandCount
			heightLimit[1] = maxHeightLimit - avgLandCount * (maxHeightLimit - heightLimit[1]) / landCount;
		}
		heightMap.stat(2, heightLimit, heightFreq);
	}

	if( abs( heightLimit[1] - initHeightLimit ) > 2 )
	{
		heightMap.add_base_level(initHeightLimit - heightLimit[1]);
	}

	curGenMapSteps++;			// 2
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}
	// ###### end Gilbert 27/8 ########//

	// --------- remove odd terrain --------//

	for(short y = 0; y <= heightMap.max_y; ++y)
		for(short x = 0; x <= heightMap.max_x; ++x)
	{
		remove_odd(heightMap, x, y, 5);
	}

	curGenMapSteps++;			// 3
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	// ------------ shuffle sub-terrain level ---------//

	heightMap.shuffle_level(TerrainRes::min_height(TERRAIN_OCEAN), 
		TerrainRes::max_height(TERRAIN_OCEAN), -3 );
	heightMap.shuffle_level(TerrainRes::min_height(TERRAIN_DARK_GRASS), 
		TerrainRes::max_height(TERRAIN_DARK_GRASS), 3 );
	heightMap.shuffle_level(TerrainRes::min_height(TERRAIN_LIGHT_GRASS), 
		TerrainRes::max_height(TERRAIN_LIGHT_GRASS), 3 );
	heightMap.shuffle_level(TerrainRes::min_height(TERRAIN_DARK_DIRT), 
		TerrainRes::max_height(TERRAIN_DARK_DIRT), 3 );

	curGenMapSteps++;			// 4
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	set_tera_id(heightMap);
	curGenMapSteps++;			// 5
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	substitute_pattern();
	curGenMapSteps++;			// 6
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	set_loc_flags();

	//--------- assign the map --------//

	assign_map();
	curGenMapSteps++;			// 7
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	gen_hills(TERRAIN_DARK_DIRT);
	curGenMapSteps++;			// 8
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	set_region_id();
	curGenMapSteps++;			// 9
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}


	gen_dirt(40,30,60);
	curGenMapSteps++;			// 10
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	gen_rocks(5,10,30);
	curGenMapSteps++;		// 11
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	set_harbor_bit();
	curGenMapSteps++;		// 12
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	plant_init();
	curGenMapSteps++;		// 13
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

	init_fire();
	curGenMapSteps++;		// 14
	if( dispProgress )
	{
		vga_front.lock_buf();
		game.disp_gen_map_status( curGenMapSteps, maxGenMapSteps, 0 );
		vga_front.unlock_buf();
	}

/*
	// randomly put a fire
	Location *locPtr;

	do
	{
		locPtr = zoom_matrix->get_loc(m.random(MAX_MAP_WIDTH), m.random(MAX_MAP_HEIGHT));
		if( locPtr->flammability > 0)
		{
			locPtr->fire_level = 80;
			break;
		}
	} while(1);
*/

	vga_front.lock_buf();

	//----- debug code: validate terrain_id -----//

	#ifdef DEBUG

	Location* locPtr = loc_matrix;

	for( int i=0 ; i<MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC ; i++, locPtr++ )
	{
		err_when( locPtr->terrain_id < 1 ||
					 locPtr->terrain_id > terrain_res.terrain_count );
	}

	#endif

}
//---------- End of function World::generate_map ------------//


//---------- Begin of function World::set_tera_id -----------//
//
// Set terrain icon id
//
void World::set_tera_id(Plasma &plasma)
{
	//------- create a world map based on the terrain map ------//

	memset(loc_matrix, 0, sizeof(Location)*max_x_loc*max_y_loc);

	for( int y = 0; y < max_y_loc; ++y)
	{
		for( int x = 0; x < max_x_loc; ++x)
		{
			int nwType, neType, swType, seType;
			int nwSubType, neSubType, swSubType, seSubType;
			nwType = TerrainRes::terrain_height(plasma.get_pix(x,y), &nwSubType);
			neType = TerrainRes::terrain_height(plasma.get_pix(x+1,y), &neSubType);
			swType = TerrainRes::terrain_height(plasma.get_pix(x,y+1), &swSubType);
			seType = TerrainRes::terrain_height(plasma.get_pix(x+1,y+1), &seSubType);

			if((get_loc(x,y)->terrain_id = terrain_res.scan( nwType, nwSubType,
				neType, neSubType, swType, swSubType, seType, seSubType ,0,1,0)) == 0)
			{
				err.run("Error World::set_tera_id, Cannot find terrain type %d:%d, %d:%d, %d:%d, %d:%d",
					nwType, nwSubType, neType, neSubType, swType, swSubType,
					seType, seSubType);
			}
		}
	}
}
//---------- End of function World::set_tera_id -----------//


//---------- Begin of function World::set_loc_flags -----------//
//
void World::set_loc_flags()
{
	int       i;
	int       totalLoc=MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC;
	Location* locPtr=loc_matrix;

	//----- set power_off of the map edges -----//

	for( int xLoc=0 ; xLoc<MAX_WORLD_X_LOC ; xLoc++ )	// set the top and bottom edges
	{
		get_loc(xLoc, 0)->set_power_off();
		get_loc(xLoc, MAX_WORLD_Y_LOC-1)->set_power_off();
	}

	for( int yLoc=0 ; yLoc<MAX_WORLD_Y_LOC ; yLoc++ )	// set the left and right edges
	{
		get_loc(0, yLoc)->set_power_off();
		get_loc(MAX_WORLD_X_LOC-1, yLoc)->set_power_off();
	}

	//-----------------------------------------//

	if( config.explore_whole_map )
	{
		for( i=0 ; i<totalLoc ; i++, locPtr++ )
		{
			//------- set explored flag ----------//
			locPtr->explored_on();
			if( terrain_res[locPtr->terrain_id]->is_coast() )
			{
				locPtr->loc_flag |= LOCATE_COAST;
				if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN)
					locPtr->set_power_off();
				else
					set_surr_power_off(i%MAX_WORLD_X_LOC, i/MAX_WORLD_X_LOC);
			}
			locPtr->walkable_reset();
		}
	}
	else
	{
		for( i=0 ; i<totalLoc ; i++, locPtr++ )
		{
			//------- clear explored flag ----------//
			locPtr->explored_off();
			if( terrain_res[locPtr->terrain_id]->is_coast() )
			{
				locPtr->loc_flag |= LOCATE_COAST;
				if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN)
					locPtr->set_power_off();
				else
					set_surr_power_off(i%MAX_WORLD_X_LOC, i/MAX_WORLD_X_LOC);
			}
			locPtr->walkable_reset();
		}
	}
}
//---------- End of function World::set_loc_flags -----------//


//---------- Begin of function World::remove_odd --------//
void World::remove_odd(Plasma &plasma, short x, short y, short recur)
{
	if( recur < 0)
		return;

	// -------- compare the TerrainTypeCode of four adjacent square ------//
	int center = TerrainRes::terrain_height(plasma.get_pix(x,y));
	int same = 0;
	int diff = 0;
	short diffTerrain = -1;
	short diffHeight;
	short sameX, sameY;
	
	// ------- compare north square -------//
	if( y > 0)
	{
		if( center == TerrainRes::terrain_height(plasma.get_pix(x,y-1)) )
		{
			same++;
			sameX = x; sameY = y-1;
		}
		else
		{
			diff++;
			if( diffTerrain < 0)
			{
				// new diffHeight
				diffHeight = plasma.get_pix(x,y-1);
				diffTerrain = TerrainRes::terrain_height(diffHeight);
				
			}
			else
			{
				// three terrain types are close, don't change anything
				if( diffTerrain != TerrainRes::terrain_height(plasma.get_pix(x,y-1)))
					return;
			}
		}
	}

	// ------- compare south square -------//
	if( y < plasma.max_y)
	{
		if( center == TerrainRes::terrain_height(plasma.get_pix(x,y+1)) )
		{
			same++;
			sameX = x; sameY = y+1;
		}
		else
		{
			diff++;
			if( diffTerrain < 0)
			{
				// new diffHeight
				diffHeight = plasma.get_pix(x,y+1);
				diffTerrain = TerrainRes::terrain_height(diffHeight);
			}
			else
			{
				// three terrain types are close, don't change anything
				if( diffTerrain != TerrainRes::terrain_height(plasma.get_pix(x,y+1)))
					return;
			}
		}
	}

		// ------- compare west square -------//
	if( x > 0)
	{
		if( center == TerrainRes::terrain_height(plasma.get_pix(x-1,y)) )
		{
			same++;
			sameX = x-1; sameY = y;
		}
		else
		{
			diff++;
			if( diffTerrain < 0)
			{
				// new diffHeight
				diffHeight = plasma.get_pix(x-1,y);
				diffTerrain = TerrainRes::terrain_height(diffHeight);
			}
			else
			{
				// three terrain types are close, don't change anything
				if( diffTerrain != TerrainRes::terrain_height(plasma.get_pix(x-1,y)))
					return;
			}
		}
	}

	// ------- compare east square -------//
	if( x < plasma.max_x)
	{
		if( center == TerrainRes::terrain_height(plasma.get_pix(x+1,y)) )
		{
			same++;
			sameX = x+1; sameY = y;
		}
		else
		{
			diff++;
			if( diffTerrain < 0)
			{
				// new diffHeight
				diffHeight = plasma.get_pix(x+1,y);
				diffTerrain = TerrainRes::terrain_height(diffHeight);
			}
			else
			{
				// three terrain types are close, don't change anything
				if( diffTerrain != TerrainRes::terrain_height(plasma.get_pix(x+1,y)))
					return;
			}
		}
	}

	if( same <= 1 && diff >= 2)
	{
		// flatten
		plasma.plot(x,y, diffHeight);

		// propagate to next square
		if( same == 1)
		{
			remove_odd(plasma, sameX, sameY, recur-1);
		}
	}
}
//---------- End of function World::remove_odd --------//


//---------- Begin of function World::substitute_pattern -----//
void World::substitute_pattern()
{
	short terrainId;
	int SubFound;
	const unsigned int resultArraySize = 20;
	TerrainSubInfo *candSub[resultArraySize];

	for( short y = 0; y < max_y_loc; ++y)
	{
		for( short x = 0; x < max_x_loc; ++x)
		{
			terrainId = get_loc(x,y)->terrain_id;
			SubFound = terrain_res.search_pattern(
				terrain_res[terrainId]->pattern_id, candSub, resultArraySize);
			for( int i = 0; i < SubFound; ++i)
			{
				short tx = x, ty = y;
				char flag = 1;
				TerrainSubInfo *terrainSubInfo = candSub[i];

				// ----- test if a substitution matches
				for(terrainSubInfo = candSub[i]; terrainSubInfo != NULL; 
					terrainSubInfo = terrainSubInfo->next_step)
				{
					if( tx < 0 || tx >= max_x_loc || ty < 0 || ty >= max_y_loc ||
						terrain_res[get_loc(tx,ty)->terrain_id]->pattern_id
						!= terrainSubInfo->old_pattern_id)
					{
						flag = 0;
						break;
					}

					// ----- update tx, ty according to post_move -----//
					switch(terrainSubInfo->post_move)
					{
					case 1: ty--; break;				// North
					case 2: ty--; tx++; break;		// NE
					case 3: tx++; break;				// East
					case 4: tx++; ty++; break;		// SE
					case 5: ty++; break;				// South
					case 6: ty++; tx--; break;		// SW
					case 7: tx--; break;				// West
					case 8: tx--; ty--; break;		// NW
					}
				}

				// ------ replace pattern -------//
				if(flag)
				{
					tx = x; ty = y;
					for(terrainSubInfo = candSub[i]; terrainSubInfo != NULL; 
						terrainSubInfo = terrainSubInfo->next_step)
					{
						TerrainInfo *oldTerrain = terrain_res[get_loc(tx,ty)->terrain_id];
						if( !(get_loc(tx,ty)->terrain_id = terrain_res.scan(oldTerrain->average_type,
							oldTerrain->secondary_type + terrainSubInfo->sec_adj, 
							terrainSubInfo->new_pattern_id, 0,1,0) ))
						{
							err_here();		// cannot find terrain_id
						}

						// ----- update tx, ty according to post_move -----//
						switch(terrainSubInfo->post_move)
						{
						case 1: ty--; break;				// North
						case 2: ty--; tx++; break;		// NE
						case 3: tx++; break;				// East
						case 4: tx++; ty++; break;		// SE
						case 5: ty++; break;				// South
						case 6: ty++; tx--; break;		// SW
						case 7: tx--; break;				// West
						case 8: tx--; ty--; break;		// NW
						}
					}
					break;
				}
			}			
		}
	}
}
//---------- End of function World::substitute_pattern -----//


//---------- Begin of function World::set_region_id -----//
// must be called before any mountain or buildings on the map
static RegionType walkable;					// to save stack space
static unsigned char regionId;
void World::set_region_id()
{
	int            i,x,y;
	int            totalLoc=max_x_loc * max_y_loc;
	Location*      locPtr=loc_matrix;

	// -------- reset region_id to zero
	for( i=0 ; i<totalLoc ; i++, locPtr++ )
	{
		locPtr->region_id = 0;
	}

	regionId = 0;
	for( y = 0; y < max_y_loc; ++y)
	{
		locPtr = get_loc(0,y);
		for( x = 0; x < max_x_loc; ++x, ++locPtr)
		{
			if( !locPtr->region_id && locPtr->region_type() != REGION_INPASSABLE)
			{
				walkable = locPtr->region_type();
				++regionId;
				fill_region(x,y);
				err_when( regionId == 255);
			}
		}
	}

	region_array.init(regionId);

	// ------ update adjacency information and region area ------//

	regionId = 0;
	for( y = 0; y < max_y_loc; ++y)
	{
		locPtr = get_loc(0,y);
		for( x = 0; x < max_x_loc; ++x, ++locPtr)
		{
			int thisRegionId = locPtr->region_id;
			// #### begin Gilbert 19/2 ######//
			if( thisRegionId > 0)
			{
				region_array.inc_size( thisRegionId );
			}
			// #### end Gilbert 19/2 ######//
			if( thisRegionId > regionId)
			{
				if(thisRegionId == regionId+1)
					regionId++;
				region_array.set_region( thisRegionId, locPtr->region_type());
			}

			int adjRegionId;
			if( y > 0)
			{
				if( x > 0 && (adjRegionId = get_loc(x-1,y-1)->region_id) < thisRegionId )
					region_array.set_adjacent( thisRegionId, adjRegionId);
				if( (adjRegionId = get_loc(x,y-1)->region_id) < thisRegionId )
					region_array.set_adjacent( thisRegionId, adjRegionId);
				if( x < max_x_loc-1 && (adjRegionId = get_loc(x+1,y-1)->region_id) < thisRegionId )
					region_array.set_adjacent( thisRegionId, adjRegionId);
			}

			if( x > 0 && (adjRegionId = get_loc(x-1,y)->region_id) < thisRegionId )
				region_array.set_adjacent( thisRegionId, adjRegionId);
			if( x < max_x_loc-1 && (adjRegionId = get_loc(x+1,y)->region_id) < thisRegionId )
				region_array.set_adjacent( thisRegionId, adjRegionId);

			if( y < max_y_loc-1)
			{
				if( x > 0 && (adjRegionId = get_loc(x-1,y+1)->region_id) < thisRegionId )
					region_array.set_adjacent( thisRegionId, adjRegionId);
				if( (adjRegionId = get_loc(x,y+1)->region_id) < thisRegionId )
					region_array.set_adjacent( thisRegionId, adjRegionId);
				if( x < max_x_loc-1 && (adjRegionId = get_loc(x+1,y+1)->region_id) < thisRegionId )
					region_array.set_adjacent( thisRegionId, adjRegionId);
			}
		}
	}

   //---- sort the region after setting its size ----//

	region_array.sort_region();

	//-------- initialize region_stat_array ----------//

	region_array.init_region_stat();
}
//---------- End of function World::set_region_id -----//


//---------- Begin of function World::fill_region -----//
void World::fill_region(short x, short y)
{
	err_when( x < 0 || x >= max_x_loc || y < 0 || y >= max_y_loc);

	short left, right;
	// Location *locPtr;

	// extent x to left and right
	for( left = x; left >= 0 && !get_loc(left,y)->region_id && get_loc(left,y)->region_type() == walkable; --left)
	{
		get_loc(left,y)->region_id = regionId;
	}
	++left;

	for( right=x+1; right < max_x_loc && !get_loc(right,y)->region_id && get_loc(right,y)->region_type() == walkable; ++right)
	{
		get_loc(right,y)->region_id = regionId;
	}
	--right;

	// ------- scan line below ---------//
	y++;
	if( y < max_y_loc )
	{
		for( x = left>0?left-1:0 ; x <= right+1 && x < max_x_loc; ++x )
		{
			if( !get_loc(x,y)->region_id && get_loc(x,y)->region_type() == walkable)
			{
				fill_region(x,y);
			}
		}
	}

	// ------- scan line above -------- //
	y -= 2;
	if( y >= 0)
	{
		for( x = left>0?left-1:0 ; x <= right+1 && x < max_x_loc; ++x )
		{
			if( !get_loc(x,y)->region_id && get_loc(x,y)->region_type() == walkable)
			{
				fill_region(x,y);
			}
		}
	}
}
//---------- End of function World::fill_region -----//


//---------- Begin of function World::set_harbor_bit -----//
void World::set_harbor_bit()
{
	// a pass during genmap to set LOCATE_HARBOR_BIT
	// notice this bit is only a necessary condition to build harbor
	int x,y;
	Location *locPtr;
	for( y = 0; y < max_y_loc-2; ++y)
	{
		locPtr = get_loc(0,y);
		for( x = 0; x < max_x_loc-2; ++x, ++locPtr)
		{
			if( can_build_firm(x, y, FIRM_HARBOR))
			{
				locPtr->set_harbor_bit();
			}
		}
	}
}
//---------- End of function World::set_harbor_bit -----//
