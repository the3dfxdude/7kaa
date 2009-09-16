// Filename    : OGENHILL.CPP
// Description : Generate hill
// Ownership   : Gilbert

#include <ALL.H>
#include <OWORLD.H>
#include <OMATRIX.H>
#include <OHILLRES.H>
#include <OTERRAIN.H>


// ---------- begin of function World::gen_hills --------//
void World::gen_hills(int terrainType)
{
	// ------- scan each tile for an above-hill terrain tile -----//
	int x, y=0;
	char priTerrain, secTerrain, lowTerrain, highTerrain;
	char patternId;
	Location *aboveLoc, *locPtr;
	TerrainInfo *terrainInfo;

	for(y = 0; y < max_y_loc; ++y)
	{
		x = 0;
		if( y > 0)
			aboveLoc = get_loc(x, y-1);
		else
			aboveLoc = NULL;
		locPtr = get_loc(x,y);
		for( ; x < max_x_loc; ++x, ++locPtr, ++aboveLoc)
		{
			terrainInfo = terrain_res[locPtr->terrain_id];
			priTerrain = terrainInfo->average_type;
			secTerrain = terrainInfo->secondary_type;
			highTerrain = (priTerrain >= secTerrain ? priTerrain : secTerrain);
			lowTerrain = (priTerrain >= secTerrain ? secTerrain : priTerrain);
			if( highTerrain >= terrainType)
			{
				// BUGHERE : ignore special or extra flag
				patternId = terrainInfo->pattern_id;
				if( lowTerrain >= terrainType)
				{
					// move this terrain one square north
					if( y > 0)
					{
						*aboveLoc = *locPtr;

						// if y is max_y_loc-1, aboveLoc and locPtr looks the same
						// BUGHERE : repeat the same pattern below is a bug if patternId is not 0,9,10,13,14
						if( y == max_y_loc -1)
							locPtr->terrain_id = terrain_res.scan(priTerrain, secTerrain, patternId);
					}			
				}
				else
				{
					short hillId = hill_res.scan(patternId, LOW_HILL_PRIORITY,0,0);
					err_when( !hillId );
					locPtr->set_hill(hillId);
					locPtr->set_fire_src(-100);
					//### begin alex 24/6 ###//
					locPtr->set_power_off();
					set_surr_power_off(x, y);
					//#### end alex 24/6 ####//
					if( y > 0)
					{
						aboveLoc->set_hill(hill_res.locate(patternId, 
							hill_res[hillId]->sub_pattern_id, HIGH_HILL_PRIORITY,0));
						aboveLoc->set_fire_src(-100);
						//### begin alex 24/6 ###//
						aboveLoc->set_power_off();
						set_surr_power_off(x, y-1);
						//#### end alex 24/6 ####//
					}
					// set terrain type to pure teraType-1
					locPtr->terrain_id = terrain_res.scan(lowTerrain, lowTerrain, 0);
				}
			}
		}
	}


	// ------ checking exit -------//
	// if an exit is set, no exit is scanned in next 7 squares
	const int MIN_EXIT_SEPARATION = 7;
	int lastExit;

	// ------ scan for south exit, width 1 --------//

#define SOUTH_PATTERN1 11
#define SOUTH_PATTERN2 15
#define SOUTH_PATTERN3 19
#define SOUTH_PATTERN4 23
#define IS_SOUTH_EXIT_PATTERN(h) (h==SOUTH_PATTERN1 || h==SOUTH_PATTERN2 || h==SOUTH_PATTERN3 || h==SOUTH_PATTERN4)
#define SOUTH_LEFT_SPECIAL 'B'
#define SOUTH_RIGHT_SPECIAL 'C'
#define SOUTH_CENTRE_SPECIAL 'A'

	for( y = 1; y < max_y_loc-1; ++y)
	{
		lastExit = 0;
		x=0;
		locPtr=get_loc(x,y);
		for( ; x < max_x_loc-2; ++x, ++locPtr, lastExit=lastExit>0?lastExit-1:0 )
		{
			HillBlockInfo *h1, *h2, *h3;
			char h1p, h2p, h3p;
			// three hill blocks on a row are pattern 11,15,19 or 23,
			// block above the second block is a walkable
			if( !lastExit && locPtr->has_hill()
				&& (h1=hill_res[locPtr->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& !h1->special_flag
				&& (h1p = h1->pattern_id)
				&& IS_SOUTH_EXIT_PATTERN(h1p)
				&& (locPtr+1)->has_hill()
				&& (h2=hill_res[(locPtr+1)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h2p = h2->pattern_id)
				&& IS_SOUTH_EXIT_PATTERN(h2p)
				&& (locPtr+2)->has_hill()
				&& (h3=hill_res[(locPtr+2)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h3p = h3->pattern_id)
				&& IS_SOUTH_EXIT_PATTERN(h3p)
				&& get_loc(x+1, y-1)->walkable() )
			{
				short hillId, terrainId;
				Location *loc2;

				// change this square
				if( h1p == SOUTH_PATTERN3)
					h1p = SOUTH_PATTERN1;
				else if( h1p == SOUTH_PATTERN4)
					h1p = SOUTH_PATTERN2;
				hillId = hill_res.scan(h1p, HIGH_HILL_PRIORITY, SOUTH_LEFT_SPECIAL, 0);
				locPtr->remove_hill();
				locPtr->set_hill(hillId);
				//### begin alex 24/6 ###//
				locPtr->set_power_off();
				set_surr_power_off(x, y);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0 )) != 0 )
					locPtr->terrain_id = terrainId;
				err_when( locPtr->has_hill() && locPtr->walkable());

				// next row
				loc2 = get_loc(x, y+1);
				hillId = hill_res.locate(h1p, hill_res[hillId]->sub_pattern_id,
					LOW_HILL_PRIORITY, SOUTH_LEFT_SPECIAL);
				if( !loc2->hill_id2() )
				{
					// if the location has only one block, remove it
					// if the location has two block, the bottom one is replaced
					loc2->remove_hill();
				}
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+1);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// second square
				loc2 = get_loc(x+1, y);
				loc2->remove_hill();
				loc2->walkable_reset();
				// ##### begin Gilbert 14/10 #####//
				//if((terrainId = terrain_res.scan(terrainType, terrainType,
				//	0, 0, 1, 0)) != 0 )
				if((terrainId = terrain_res.scan( terrainType, BOTTOM_MASK, terrainType,
					BOTTOM_MASK, terrainType, BOTTOM_MASK, terrainType, BOTTOM_MASK)) != 0)
				// ##### end Gilbert 14/10 #####//
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next row
				loc2 = get_loc(x+1, y+1);
				loc2->remove_hill();
				loc2->walkable_reset();
				if((terrainId = terrain_res.scan(terrainType, terrainType-1,
					SOUTH_PATTERN2, 0, 1, SOUTH_CENTRE_SPECIAL )) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// prev row
				// loc2 = get_loc(x+1, y-1);
				// if((terrainId = terrain_res.scan(terrainType, terrainType-1,
				// 	SOUTH_PATTERN2, 0, 1, SOUTH_CENTRE_SPECIAL )) != 0 )
				//	loc2->terrain_id = terrainId;
				// err_when( loc2->has_hill() && loc2->walkable());

				// third square
				loc2 = get_loc(x+2, y);
				if( h3p == SOUTH_PATTERN4)
					h3p = SOUTH_PATTERN1;
				if( h3p == SOUTH_PATTERN3)
					h3p = SOUTH_PATTERN2;
				hillId = hill_res.scan(h3p, HIGH_HILL_PRIORITY, SOUTH_RIGHT_SPECIAL, 0);
				loc2->remove_hill();
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x+2, y);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next row
				loc2 = get_loc(x+2, y+1);
				hillId = hill_res.locate(h3p, hill_res[hillId]->sub_pattern_id,
					LOW_HILL_PRIORITY, SOUTH_RIGHT_SPECIAL);
				if( !loc2->hill_id2() )
				{
					// if the location has only one block, remove it
					// if the location has two block, the bottom one is replaced
					loc2->remove_hill();
				}
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x+2, y+1);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				lastExit = MIN_EXIT_SEPARATION;
			}
		}
	}


	// ------ scan for north exit, width 1 --------//

#define NORTH_PATTERN1 12
#define NORTH_PATTERN2 16
#define NORTH_PATTERN3 20
#define NORTH_PATTERN4 24
#define IS_NORTH_EXIT_PATTERN(h) (h==NORTH_PATTERN1 || h==NORTH_PATTERN2 || h==NORTH_PATTERN3 || h==NORTH_PATTERN4)
#define NORTH_LEFT_SPECIAL 'D'
#define NORTH_RIGHT_SPECIAL 'E'
#define NORTH_CENTRE_SPECIAL 'F'

	for( y = 1; y < max_y_loc-2; ++y)
	{
		lastExit = 0;
		x = max_x_loc-3; // x=0;
		locPtr=get_loc(x,y);
		for( ; x >= 0; --x, --locPtr, lastExit=lastExit>0?lastExit-1:0)
		{
			HillBlockInfo *h1, *h2, *h3;
			char h1p, h2p, h3p;
			// three hill blocks on a row are pattern 12,16,20 or 24,
			// block below the second block is a walkable
			if( !lastExit && locPtr->has_hill()
				&& (h1=hill_res[locPtr->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& !h1->special_flag
				&& (h1p = h1->pattern_id)
				&& IS_NORTH_EXIT_PATTERN(h1p)
				&& (locPtr+1)->has_hill()
				&& (h2=hill_res[(locPtr+1)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h2p = h2->pattern_id)
				&& IS_NORTH_EXIT_PATTERN(h2p)
				&& (locPtr+2)->has_hill()
				&& (h3=hill_res[(locPtr+2)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h3p = h3->pattern_id)
				&& IS_NORTH_EXIT_PATTERN(h3p)
				&& get_loc(x+1, y+1)->walkable() )
			{
				short hillId, terrainId;
				Location *loc2;

				// change this square
				if( h1p == NORTH_PATTERN4)
					h1p = NORTH_PATTERN1;
				else if( h1p == NORTH_PATTERN3)
					h1p = NORTH_PATTERN2;
				hillId = hill_res.scan(h1p, HIGH_HILL_PRIORITY, NORTH_LEFT_SPECIAL, 0);
				locPtr->remove_hill();
				locPtr->set_hill(hillId);
				//### begin alex 24/6 ###//
				locPtr->set_power_off();
				set_surr_power_off(x, y);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					locPtr->terrain_id = terrainId;
				err_when( locPtr->has_hill() && locPtr->walkable());

				// second square
				loc2 = get_loc(x+1, y);
				loc2->remove_hill();
				loc2->walkable_reset();
				//if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
				//	0, 0, 1, NORTH_CENTRE_SPECIAL)) != 0 )
				//	loc2->terrain_id = terrainId;
				if((terrainId = terrain_res.scan(terrainType, terrainType-1,
					NORTH_PATTERN2, 0, 1, NORTH_CENTRE_SPECIAL )) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next row
				//loc2 = get_loc(x+1, y+1);
				//if((terrainId = terrain_res.scan(terrainType, terrainType-1,
				//	NORTH_PATTERN2, 0, 1, NORTH_CENTRE_SPECIAL )) != 0 )
				//	loc2->terrain_id = terrainId;
				//err_when( loc2->has_hill() && loc2->walkable());

				// third square
				loc2 = get_loc(x+2, y);
				if( h3p == NORTH_PATTERN3)
					h3p = NORTH_PATTERN1;
				if( h3p == NORTH_PATTERN4)
					h3p = NORTH_PATTERN2;
				hillId = hill_res.scan(h3p, HIGH_HILL_PRIORITY, NORTH_RIGHT_SPECIAL, 0);
				loc2->remove_hill();
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x+2, y);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				lastExit = MIN_EXIT_SEPARATION;
			}
		}
	}


	// ------ scan for west exit, width 1 --------//

#define WEST_PATTERN1 9
#define WEST_PATTERN2 13
#define WEST_PATTERN3 17
#define WEST_PATTERN4 21
#define IS_WEST_EXIT_PATTERN(h) (h==WEST_PATTERN1 || h==WEST_PATTERN2 || h==WEST_PATTERN3 || h==WEST_PATTERN4)
#define WEST_TOP_SPECIAL 'G'
#define WEST_BOTTOM_SPECIAL 'I'
#define WEST_CENTRE_SPECIAL 'H'

	for( x = 1; x < max_x_loc-1; ++x)
	{
		lastExit = 0;
		for( y = 0; y < max_y_loc-4; ++y, lastExit=lastExit>0?lastExit-1:0)
		{
			locPtr=get_loc(x,y);
			HillBlockInfo *h1, *h2, *h3;
			char h1p, h2p, h3p;
			// three hill blocks on a row are pattern 9, 13, 17, 21
			// block above the second block is a walkable
			if( !lastExit && locPtr->has_hill() 
				&& (h1=hill_res[locPtr->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& !h1->special_flag
				&& (h1p = h1->pattern_id)
				&& IS_WEST_EXIT_PATTERN(h1p)
				&& get_loc(x,y+1)->has_hill()
				&& get_loc(x,y+2)->has_hill()
				&& (h2=hill_res[get_loc(x,y+2)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h2p = h2->pattern_id)
				&& IS_WEST_EXIT_PATTERN(h2p)
				&& get_loc(x,y+3)->has_hill()
				&& (h3=hill_res[get_loc(x,y+3)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h3p = h3->pattern_id)
				&& (h3p == WEST_PATTERN1 || h3p == WEST_PATTERN4)
				&& get_loc(x+1, y+2)->walkable() )
			{
				short hillId, terrainId, hill2;
				Location *loc2;

				// change this square
				if( h1p == WEST_PATTERN3)
					h1p = WEST_PATTERN1;
				else if( h1p == WEST_PATTERN4)
					h1p = WEST_PATTERN2;
				hillId = hill_res.scan(h1p, HIGH_HILL_PRIORITY, WEST_TOP_SPECIAL, 0);
				hill2 = locPtr->hill_id2();
				locPtr->remove_hill();
				locPtr->set_hill(hillId);
				//### begin alex 24/6 ###//
				locPtr->set_power_off();
				set_surr_power_off(x, y);
				//#### end alex 24/6 ####//
				if( hill2 )
					locPtr->set_hill(hill2);
				err_when( locPtr->has_hill() && locPtr->walkable());

				// next row
				loc2 = get_loc(x, y+1);
				hillId = hill_res.locate(h1p, hill_res[hillId]->sub_pattern_id,
					LOW_HILL_PRIORITY, WEST_TOP_SPECIAL);
				loc2->remove_hill();
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+1);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// third row
				loc2 = get_loc(x, y+2);
				loc2->remove_hill();
				loc2->walkable_reset();
				//if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
				//	0, 0, 1, WEST_CENTRE_SPECIAL)) != 0 )
				//	loc2->terrain_id = terrainId;
				if((terrainId = terrain_res.scan(terrainType, terrainType-1,
					WEST_PATTERN2, 0, 1, WEST_CENTRE_SPECIAL )) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next column
				//loc2 = get_loc(x+1, y+2);
				//if((terrainId = terrain_res.scan(terrainType, terrainType-1,
				//	WEST_PATTERN2, 0, 1, WEST_CENTRE_SPECIAL )) != 0 )
				//	loc2->terrain_id = terrainId;
				//err_when( loc2->has_hill() && loc2->walkable());

				// fourth row
				loc2 = get_loc(x, y+3);
				if( h3p == WEST_PATTERN4)
					h3p = WEST_PATTERN1;
				if( h3p == WEST_PATTERN3)
					h3p = WEST_PATTERN2;
				hillId = hill_res.scan(h3p, HIGH_HILL_PRIORITY, WEST_BOTTOM_SPECIAL, 0);
				loc2->remove_hill();
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+3);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0 )) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next row
				loc2 = get_loc(x, y+4);
				hillId = hill_res.locate(h3p, hill_res[hillId]->sub_pattern_id,
					LOW_HILL_PRIORITY, WEST_BOTTOM_SPECIAL);
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+4);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());
				lastExit = MIN_EXIT_SEPARATION;
			}
		}
	}

	// ------ scan for east exit, width 1 --------//

#define EAST_PATTERN1 10
#define EAST_PATTERN2 14
#define EAST_PATTERN3 18
#define EAST_PATTERN4 22
#define IS_EAST_EXIT_PATTERN(h) (h==EAST_PATTERN1 || h==EAST_PATTERN2 || h==EAST_PATTERN3 || h==EAST_PATTERN4)
#define EAST_TOP_SPECIAL 'J'
#define EAST_BOTTOM_SPECIAL 'L'
#define EAST_CENTRE_SPECIAL 'K'

	for( x=1; x < max_x_loc-1; ++x)
	{
		lastExit = 0;
		for( y = max_y_loc-5; y >= 0; --y, lastExit=lastExit>0?lastExit-1:0)
		{
			locPtr=get_loc(x,y);
			HillBlockInfo *h1, *h2, *h3;
			char h1p, h2p, h3p;
			// three hill blocks on a row are pattern 9, 13, 17, 21
			// block above the second block is a walkable
			if( !lastExit && locPtr->has_hill()
				&& (h1=hill_res[locPtr->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& !h1->special_flag
				&& (h1p = h1->pattern_id)
				&& IS_EAST_EXIT_PATTERN(h1p)
				&& get_loc(x,y+1)->has_hill()
				&& get_loc(x,y+2)->has_hill()
				&& (h2=hill_res[get_loc(x,y+2)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h2p = h2->pattern_id)
				&& IS_EAST_EXIT_PATTERN(h2p)
				&& get_loc(x,y+3)->has_hill()
				&& (h3=hill_res[get_loc(x,y+3)->hill_id1()])->priority == HIGH_HILL_PRIORITY
				&& (h3p = h3->pattern_id)
				&& (h3p == EAST_PATTERN1 || h3p == EAST_PATTERN4)
				&& get_loc(x-1, y+2)->walkable() )
			{
				short hillId, terrainId, hill2;
				Location *loc2;

				// change this square
				if( h1p == EAST_PATTERN3)
					h1p = EAST_PATTERN1;
				else if( h1p == EAST_PATTERN4)
					h1p = EAST_PATTERN2;
				hillId = hill_res.scan(h1p, HIGH_HILL_PRIORITY, EAST_TOP_SPECIAL, 0);
				hill2 = locPtr->hill_id2();
				locPtr->remove_hill();
				locPtr->set_hill(hillId);
				if( hill2 )
					locPtr->set_hill(hill2);
				err_when( locPtr->has_hill() && locPtr->walkable());
				//### begin alex 24/6 ###//
				locPtr->set_power_off();
				set_surr_power_off(x, y);
				//#### end alex 24/6 ####//

				// next row
				loc2 = get_loc(x, y+1);
				hillId = hill_res.locate(h1p, hill_res[hillId]->sub_pattern_id,
					LOW_HILL_PRIORITY, EAST_TOP_SPECIAL);
				loc2->remove_hill();
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+1);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// third row
				loc2 = get_loc(x, y+2);
				loc2->remove_hill();
				loc2->walkable_reset();
				//if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
				//	0, 0, 1, EAST_CENTRE_SPECIAL)) != 0 )
				//	loc2->terrain_id = terrainId;
				if((terrainId = terrain_res.scan(terrainType, terrainType-1,
					EAST_PATTERN2, 0, 1, EAST_CENTRE_SPECIAL )) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next column
				//loc2 = get_loc(x-1, y+2);
				//if((terrainId = terrain_res.scan(terrainType, terrainType-1,
				//	EAST_PATTERN2, 0, 1, EAST_CENTRE_SPECIAL )) != 0 )
				//	loc2->terrain_id = terrainId;
				//err_when( loc2->has_hill() && loc2->walkable());

				// fourth row
				loc2 = get_loc(x, y+3);
				if( h3p == EAST_PATTERN4)
					h3p = EAST_PATTERN1;
				if( h3p == EAST_PATTERN3)
					h3p = EAST_PATTERN2;
				hillId = hill_res.scan(h3p, HIGH_HILL_PRIORITY, EAST_BOTTOM_SPECIAL, 0);
				loc2->remove_hill();
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+3);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());

				// next row
				loc2 = get_loc(x, y+4);
				hillId = hill_res.locate(h3p, hill_res[hillId]->sub_pattern_id,
					LOW_HILL_PRIORITY, EAST_BOTTOM_SPECIAL);
				loc2->set_hill(hillId);
				//### begin alex 24/6 ###//
				loc2->set_power_off();
				set_surr_power_off(x, y+4);
				//#### end alex 24/6 ####//
				if((terrainId = terrain_res.scan(terrainType-1, terrainType-1,
					0, 0, 1, 0)) != 0 )
					loc2->terrain_id = terrainId;
				err_when( loc2->has_hill() && loc2->walkable());
				lastExit = MIN_EXIT_SEPARATION;
			}
		}
	}
}
// ---------- end of function World::gen_hills -------//


// ---------- begin of function World::put_hill_set ------//
void World::put_hill_set(short *px, short *py, short hSetId)
{
}
// ---------- end of function World::put_hill_pattern ------//


// ---------- begin of function World::put_hill_pattern ------//
void World::put_hill_pattern(short *px, short *py, unsigned char patternId)
{
}
// ---------- end of function World::put_hill_pattern ------//


// ---------- begin of function World::fill_hill -------//
void World::fill_hill(short x, short y)
{
}
// ---------- end of function World::fill_hill -------//
