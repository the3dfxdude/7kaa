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

//Filename    : OSITE.CPP
//Description : Object Site

#include <OINFO.h>
#include <OWORLD.h>
#include <OTOWN.h>
#include <OFIRM.h>
#include <OUNIT.h>
#include <ONEWS.h>
#include <OGODRES.h>
#include <ONATION.h>
#include <OSITE.h>

//-------------- Define constant -----------//

#define EXIST_RAW_RESERVE_QTY 		(MAX_RAW_RESERVE_QTY / 20)   // only sites with reserve qty >= 5% of MAX_RAW_RESERVE_QTY are counted as raw sites

//--------- Begin of function SiteArray::SiteArray -------//

SiteArray::SiteArray() : DynArrayB(sizeof(Site), 50, DEFAULT_REUSE_INTERVAL_DAYS)
{
}
//----------- End of function SiteArray::SiteArray -------//


//--------- Begin of function SiteArray::~SiteArray -------//

SiteArray::~SiteArray()
{
	deinit();
}
//----------- End of function SiteArray::~SiteArray -------//


//--------- Begin of function SiteArray::init -------//

void SiteArray::init()
{
	untapped_raw_count = 0;
	std_raw_site_count = 0;
}
//----------- End of function SiteArray::init -------//


//--------- Begin of function SiteArray::deinit -------//

void SiteArray::deinit()
{
	if( size()==0 )
		return;

	zap();       // zap the DynArrayB

	untapped_raw_count = 0;
}
//----------- End of function SiteArray::deinit -------//


//--------- Begin of function SiteArray::add_site ---------//
//
// Add a raw item to the site array
//
// Return : 1 - the raw is added
//          0 - duplicated, not added
//
int SiteArray::add_site(int xLoc, int yLoc, int siteType, int objectId, int reserveQty)
{
	//----- linkin the raw and update raw attribute ----//

	Site site;

	linkin(&site);

	Site* sitePtr = (Site*) get(recno());

	sitePtr->init(recno(), siteType, xLoc, yLoc);

	sitePtr->object_id 	= objectId;
	sitePtr->reserve_qty = reserveQty;

	switch( siteType )
	{
		case SITE_RAW:
			untapped_raw_count++;
			break;

		case SITE_SCROLL:
			scroll_count++;
			break;

		case SITE_GOLD_COIN:
			gold_coin_count++;
			break;
	}

	return 1;
}
//----------- End of function SiteArray::add_site ----------//


//--------- Begin of function SiteArray::del_site ----------//
//
// Delete a specified site.
//
// <int> siteRecno = the record no. of the site to be deleted
//
void SiteArray::del_site(int siteRecno)
{
	err_if( siteRecno == 0 )
		err_now( "SiteArray::del_site" );

	Site* sitePtr = site_array[siteRecno];

	switch( sitePtr->site_type )
	{
		case SITE_RAW:
			untapped_raw_count--;
			break;

		case SITE_SCROLL:
			scroll_count--;
			break;

		case SITE_GOLD_COIN:
			gold_coin_count--;
			break;
	}

	//-------------------------------//

	sitePtr->deinit();

	linkout(siteRecno);

	if( siteRecno == site_array.selected_recno )
		site_array.selected_recno = 0;
}
//--------- End of function SiteArray::del_site ----------//


//--------- Begin of function SiteArray::generate_raw_site ----------//
//
// Generate raw sites. This function is both called at the beginning
//	of a game and when existing raw sites are being used up.
//
// [int] rawGenCount - no. of raw sites to be generated.
//							  (if this is not given, it will use the existing std_raw_site_count)
//
void SiteArray::generate_raw_site(int rawGenCount)
{
	if( rawGenCount )
		std_raw_site_count = rawGenCount;		// use this number for determing whether new sites should emerge in the future

	#define MAX_RAW_REGION 			  3		// maximum no. of regions that has raw sites
	#define SMALLEST_RAW_REGION	 50 		// only put raw on the region if its size is larger than this
	#define REGION_SIZE_PER_RAW  	 100

	//----- count the no. of existing raw sites -------//

	Site* sitePtr;
	int   existRawSiteCount=0;

	int i;
	for( i=size() ; i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( sitePtr->site_type == SITE_RAW &&
			 sitePtr->reserve_qty >= EXIST_RAW_RESERVE_QTY )
		{
			existRawSiteCount++;
		}
	}

	if( existRawSiteCount >= std_raw_site_count )
		return;

	//----- check which regions are valid for raw sites -----//

	int regionCount = MIN( MAX_RAW_REGION, region_array.region_info_count );
	int validRegionCount, totalValidSize=0;
	RegionInfo* regionInfo;

	for( validRegionCount=0 ; validRegionCount<regionCount ; validRegionCount++ )
	{
		regionInfo = region_array.get_sorted_region(validRegionCount+1);

		if( regionInfo->region_type != REGION_LAND )
			continue;

		if( regionInfo->region_size < SMALLEST_RAW_REGION )
			break;

		totalValidSize += regionInfo->region_size;
	}

	if( validRegionCount==0 )	// valid regions are those that are big enough to put raw sites
		return;

	//----- count the no. of existing raw sites in each ragion ------//

	int   regionId;
	char	regionRawCountArray[MAX_REGION];

	memset( regionRawCountArray, 0, sizeof(regionRawCountArray) );

	for( i=size() ; i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( sitePtr->site_type == SITE_RAW &&
			 sitePtr->reserve_qty >= EXIST_RAW_RESERVE_QTY )
		{
			regionId = world.get_region_id(sitePtr->map_x_loc, sitePtr->map_y_loc);

			regionRawCountArray[regionId-1]++;

		}
	}

	//--------- generate raw sites now ----------//

	int avgValidSize = MIN( 10000, totalValidSize / std_raw_site_count );
	int j, createCount;

	err_when( validRegionCount > region_array.region_info_count || validRegionCount > MAX_RAW_REGION );

	for( int k=0 ; k<10 ; k++ )		//	one loop may not be enough to generate all raw sites, have more loops to make sure all are generated
	{
		for( i=0 ; i<regionCount ; i++ )
		{
			regionInfo = region_array.get_sorted_region(i+1);

			if( regionInfo->region_type != REGION_LAND )
				continue;

			if( regionInfo->region_size < SMALLEST_RAW_REGION )
				break;

			createCount = regionInfo->region_size / avgValidSize;
			createCount = MAX(1, createCount);

			//--------- create now --------//

			for( j=regionRawCountArray[regionInfo->region_id-1] ; j<createCount ; j++ )	// if currently there are already some, don't create new ones
			{
				if( create_raw_site(regionInfo->region_id) )
				{
					if( ++existRawSiteCount == std_raw_site_count )
						return;
				}
			}
		}
	}
}
//--------- End of function SiteArray::generate_raw_site ----------//


//--------- Begin of function SiteArray::create_raw_site ----------//
//
// <int> regionId  - if this parameter is given, the raw site
//							will be built on this region.
// [int] townRecno - if this parameter is given, the raw site
//							will be built near this town.
//
int SiteArray::create_raw_site(int regionId, int townRecno)
{
	//-------- count the no. of each raw material -------//

	Site* sitePtr;
	short rawCountArray[MAX_RAW];

	memset( rawCountArray, 0, sizeof(rawCountArray) );

	int i;
	for( i=size(); i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( sitePtr->site_type == SITE_RAW )
		{
			err_when( sitePtr->object_id < 1 || sitePtr->object_id > MAX_RAW );

			rawCountArray[ sitePtr->object_id-1 ]++;
		}
	}

	//---- find the minimum raw count ----//

	int minCount=0xFFFF;

	for( i=0 ; i<MAX_RAW ; i++ )
	{
		if( rawCountArray[i] < minCount )
			minCount = rawCountArray[i];
	}

	//----- pick a raw material type -----//

	int rawId = m.random(MAX_RAW)+1;

	for( i=0 ; i<MAX_RAW ; i++ )
	{
		if( ++rawId > MAX_RAW )
			rawId = 1;

		if( rawCountArray[rawId-1] == minCount )	// don't use this raw type unless it is one of the less available ones.
			break;
	}

	//--------- create the raw site now ------//

	int locX1, locY1, locX2, locY2;
	int maxTries;

	if( townRecno )
	{
		#define MAX_TOWN_SITE_DISTANCE   10

		Town* townPtr = town_array[townRecno];

		locX1 = townPtr->center_x - MAX_TOWN_SITE_DISTANCE;
		locX2 = townPtr->center_x + MAX_TOWN_SITE_DISTANCE;
		locY1 = townPtr->center_y - MAX_TOWN_SITE_DISTANCE;
		locY2 = townPtr->center_y + MAX_TOWN_SITE_DISTANCE;

		if(locX1<0)
			locX1 = 0;
		else if(locX2>=MAX_WORLD_X_LOC)
			locX2 = MAX_WORLD_X_LOC-1;

		if(locY1<0)
			locY1 = 0;
		else if(locY2>=MAX_WORLD_Y_LOC)
			locY2 = MAX_WORLD_Y_LOC-1;

		maxTries = (locX2-locX1+1)*(locY2-locY1+1);
		regionId = townPtr->region_id;
	}
	else
	{
		locX1 = 0;
		locY1 = 0;
		locX2 = MAX_WORLD_X_LOC-1;
		locY2 = MAX_WORLD_Y_LOC-1;

		maxTries = 10000;
	}

	//----- randomly locate a space to add the site -----//

	if( world.locate_space_random(locX1, locY1, locX2, locY2,
		 5, 5, maxTries, regionId, 1) )     	// 5,5 are the size of the raw site, it must be large enough for a mine to build and 1 location for the edges. The last para 1 = site building mode
	{
		int reserveQty = MAX_RAW_RESERVE_QTY * (50 + m.random(50)) / 100;

		add_site( locX1+2, locY1+2, SITE_RAW, rawId, reserveQty );		// xLoc+1 & yLoc+1 as the located size is 3x3, the raw site is at the center of it

		return 1;
	}
	else
	{
		return 0;
	}
}
//--------- End of function SiteArray::create_raw_site ----------//


//--------- Begin of function SiteArray::scan_site ----------//
//
// Scan for the a site that is closest the given location.
//
// <int> xLoc, yLoc - the location given.
// [int] siteType	  - if given, only scan for this site (default: 0)
//
// return: <int> nearestSiteRecno - the recno of the raw materials
//											   that is nearest to the given location.
//
int SiteArray::scan_site(int xLoc, int yLoc, int siteType)
{
	Site* sitePtr;
	int   siteDis, minDis=0x7FFFFFFF, nearestSiteRecno=0;

	for( int i=site_array.size() ; i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( siteType==0 || sitePtr->site_type==siteType )
		{
			siteDis = m.points_distance( xLoc, yLoc, sitePtr->map_x_loc, sitePtr->map_y_loc );

			if( siteDis < minDis )
			{
				minDis 		 	  = siteDis;
				nearestSiteRecno = i;
			}
		}
	}

	return nearestSiteRecno;
}
//---------- End of function SiteArray::scan_site -----------//


//--------- Begin of function SiteArray::next_day ----------//
//
void SiteArray::next_day()
{
	if( info.game_date%30 == 0 )
	{
		generate_raw_site();		// check if we need to generate existing raw sites are being used up and if we need to generate new ones
	}

	//-- if there is any scroll or gold coins available, ask AI to get them --//

	if(scroll_count || gold_coin_count)
	{
		int aiGetSiteObject = (info.game_date%5 == 0);

		Site* sitePtr;
		Location *locPtr;

		for(int i=size(); i; i--)
		{
			if(is_deleted(i))
				continue;

			sitePtr = site_array[i];

			switch(sitePtr->site_type)
			{
				case SITE_SCROLL:
				case SITE_GOLD_COIN:
						locPtr = world.get_loc(sitePtr->map_x_loc, sitePtr->map_y_loc);

						//---- if the unit is standing on a scroll site -----//

						if(locPtr->has_unit(UNIT_LAND))
						{
							sitePtr->get_site_object( locPtr->unit_recno(UNIT_LAND) );
						}
						else if(aiGetSiteObject)
						{
							sitePtr->ai_get_site_object();
                  }
						break;
			}
		}
	}

	//-------- debug testing --------//

#ifdef DEBUG

	if( info.game_date%10 == 0 )
	{
		Site* sitePtr;
		Location* locPtr;

		for( int i=1 ; i<=size() ; i++ )
		{
			if( site_array.is_deleted(i) )
				continue;

			sitePtr = site_array[i];

			locPtr = world.get_loc( sitePtr->map_x_loc, sitePtr->map_y_loc );

			err_when( !locPtr->has_site() );
			err_when( locPtr->site_recno() != i );

			if( sitePtr->has_mine )
			{
				err_when( !locPtr->is_firm() );
				err_when( firm_array[locPtr->firm_recno()]->firm_id != FIRM_MINE );
			}
			else
			{
				err_when( locPtr->is_firm() || locPtr->is_town() );
			}
		}
	}
#endif
}
//--------- End of function SiteArray::next_day ----------//


//--------- Begin of function SiteArray::ai_get_site_object -------//
//
// Notify AI units to acquire scrolls or gold coins available on the
// map.
//
void SiteArray::ai_get_site_object()
{
	Site* sitePtr;

	for( int i=1 ; i<=size() ; i++ )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( sitePtr->site_type == SITE_SCROLL ||
			 sitePtr->site_type == SITE_GOLD_COIN )
		{
			sitePtr->ai_get_site_object();
		}
	}
}
//----------- End of function SiteArray::ai_get_site_object -------//


//------- Begin of function Site::ai_get_site_object -------//
//
// Ask AI units around to get the object on this site.
//
int Site::ai_get_site_object()
{
	#define NOTIFY_GET_RANGE 	30		// only notify units within this range
	#define MAX_UNIT_TO_ORDER  5

	int		 xOffset, yOffset;
	int		 xLoc, yLoc;
	Location* locPtr;
	Unit*	    unitPtr;
	int		 unitOrderedCount=0;
	int 		 siteRaceId = 0;

	if( site_type == SITE_SCROLL )
		siteRaceId = god_res[object_id]->race_id;

	for( int i=2 ; i<NOTIFY_GET_RANGE*NOTIFY_GET_RANGE ; i++ )
	{
		m.cal_move_around_a_point(i, NOTIFY_GET_RANGE, NOTIFY_GET_RANGE, xOffset, yOffset);

		xLoc = map_x_loc + xOffset;
		yLoc = map_y_loc + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if( !locPtr->has_unit(UNIT_LAND) )
			continue;

		//------------------------------//

		int unitRecno = locPtr->unit_recno(UNIT_LAND);

		if( unit_array.is_deleted(unitRecno) )
			continue;

		unitPtr = unit_array[unitRecno];

		if( !unitPtr->race_id || !unitPtr->ai_unit || unitPtr->ai_action_id )
			continue;

		if( siteRaceId && siteRaceId != unitPtr->race_id )
			continue;

		unitPtr->move_to(map_x_loc, map_y_loc);

		//--- if the unit is just standing next to the site ---//

		if( abs(map_x_loc-xLoc)<=1 && abs(map_y_loc-yLoc)<=1 )
		{
			return 1;
		}
		else		// order more than one unit to get the site at the same time
		{
			if( ++unitOrderedCount >= MAX_UNIT_TO_ORDER )
				return 1;
		}
	}

	return 0;
}
//-------- End of function Site::ai_get_site_object -------//


//--------- Begin of function SiteArray::go_to_a_raw_site -------//
//
// Go to an untapped raw site.
//
void SiteArray::go_to_a_raw_site()
{
	//----- try to locate an untapped raw site -----//

	Site* sitePtr;
	//### begin alex 22/10 ###//
	int arraySize = size();
	int i = selected_recno ? selected_recno : 0;
	//#### end alex 22/10 ####//

	//### begin alex 22/10 ###//
	//for( int i=1 ; i<=size() ; i++ )
	//{
	int j;
	for( j=1 ; j<=arraySize ; j++ )
	{
		if(++i > arraySize)
			i = 1;
	//#### end alex 22/10 ####//
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( !sitePtr->has_mine )
		{
			if( world.get_loc(sitePtr->map_x_loc, sitePtr->map_y_loc)->explored() )
			{
				world.go_loc( sitePtr->map_x_loc, sitePtr->map_y_loc, 1 );		// 1-select the object on the location
				return;
			}
		}
	}

	//---- if no untapped raw sites left, jump to built mines ----//
	//### begin alex 22/10 ###//
	i = 1;
	if(firm_array.selected_recno)
	{
		//------- get the site_recno if a mine is selected ---------//
		Firm *firmPtr = firm_array[firm_array.selected_recno];
		if(firmPtr->firm_id==FIRM_MINE)
		{
			int x1 = firmPtr->loc_x1;
			int y1 = firmPtr->loc_y1;
			int x2 = firmPtr->loc_x2;
			int y2 = firmPtr->loc_y2;

			for(int count=1; count<=arraySize; ++count)
			{
				if(site_array.is_deleted(count))
					continue;

				sitePtr = site_array[count];
				if(sitePtr->map_x_loc>=x1 && sitePtr->map_x_loc<=x2 && sitePtr->map_y_loc>=y1 && sitePtr->map_y_loc<=y2)
				{
					i = count;
					break;
				}
			}
		}
	}
	//#### end alex 22/10 ####//

	//### begin alex 22/10 ###//
	//for( i=1 ; i<=size() ; i++ )
	//{
	for( j=1 ; j<=arraySize ; j++ )
	{
		if(++i > arraySize)
			i = 1;
	//#### end alex 22/10 ####//

		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( world.get_loc(sitePtr->map_x_loc, sitePtr->map_y_loc)->explored() )
		{
			world.go_loc( sitePtr->map_x_loc, sitePtr->map_y_loc, 1 );		// 1-select the object on the location
			return;
		}
	}
}
//----------- End of function SiteArray::go_to_a_raw_site -------//


//--------- Begin of function Site::init ----------//
//
void Site::init(int siteRecno, int siteType, int xLoc, int yLoc)
{
	site_recno  = siteRecno;
	site_type   = siteType;
	map_x_loc   = xLoc;
	map_y_loc   = yLoc;
	has_mine    = 0;

	//------- set world's location --------//

	Location* locPtr = world.get_loc(xLoc, yLoc);

	locPtr->set_site(siteRecno);

	region_id = locPtr->region_id;
}
//---------- End of function Site::init -----------//


//--------- Begin of function Site::deinit ----------//
//
void Site::deinit()
{
	//------ reset world's location ---------//

	world.get_loc(map_x_loc, map_y_loc)->remove_site();
}
//---------- End of function Site::deinit -----------//


//--------- Begin of function Site::get_site_object ----------//
//
// An unit takes the object from the site
//
// <int> unitRecno - recno of the unit that takes the object on the site.
//
int Site::get_site_object(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];
	int   objectTaken=0;

	if( !unitPtr->nation_recno )
		return 0;

	//----- if this is a scroll site ------//

	if( site_type == SITE_SCROLL )
	{
		if( god_res[object_id]->race_id == unitPtr->race_id )
		{
			god_res[object_id]->enable_know(unitPtr->nation_recno);

			objectTaken = 1;

			news_array.scroll_acquired(unitPtr->nation_recno, god_res[object_id]->race_id );
		}
	}

	//------ if there are gold coins on this site -----//

	if( site_type == SITE_GOLD_COIN )
	{
		nation_array[unitPtr->nation_recno]->add_income(INCOME_TREASURE, object_id);
		objectTaken = 1;

		if( unitPtr->nation_recno == nation_array.player_recno )
			news_array.monster_gold_acquired(object_id);
	}

	//---- if the object has been taken by the unit ----//

	if( objectTaken )
	{
		site_array.del_site(site_recno);
		return 1;
	}

	return 0;
}
//---------- End of function Site::get_site_object -----------//


//------- Begin of function SiteArray::is_deleted -----//

int SiteArray::is_deleted(int recNo)
{
	Site* sitePtr = (Site*) get(recNo);

	return !sitePtr || sitePtr->site_type==0;
}
//--------- End of function SiteArray::is_deleted ----//

#ifdef DEBUG

//------- Begin of function SiteArray::operator[] -----//

Site* SiteArray::operator[](int recNo)
{
	Site* sitePtr = (Site*) get(recNo);

	if( !sitePtr || sitePtr->site_type==0 )
		err.run( "SiteArray[] is deleted" );

	return sitePtr;
}
//--------- End of function SiteArray::operator[] ----//


//------- Begin of function SiteArray::operator() -----//

Site* SiteArray::operator()()
{
	Site* sitePtr = (Site*) get();

	if( !sitePtr || sitePtr->site_type==0 )
		err.run( "SiteArray[recno()] is deleted" );

	return sitePtr;
}
//--------- End of function SiteArray::operator() ----//

#endif

