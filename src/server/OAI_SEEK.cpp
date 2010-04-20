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

//Filename   : OAI_SEEK.CPP
//Description: AI - action progressing functions

#include <ALL.h>
#include <OUNIT.h>
#include <OF_MINE.h>
#include <OF_FACT.h>
#include <OF_HARB.h>
#include <OTOWN.h>
#include <OSITE.h>
#include <ONATION.h>

//--------- Begin of function Nation::seek_mine --------//
//
// <short&> xLoc, yLoc 		  - reference vars for returning the building
//										 location.
// <short&> refXLoc, refYLoc - reference vars for returning the exact
//										 location of the raw material.
//
// return: <int> >0  the raw id. of the site.
//					  ==0 no suitable site found.
//
int Nation::seek_mine(short& xLoc, short& yLoc, short& refXLoc, short& refYLoc)
{
	err_when( site_array.untapped_raw_count < 0 );

	if( site_array.untapped_raw_count==0 )
		return 0;

	int		raw_kind_mined[MAX_RAW], i;
	Firm		*firmPtr;
	FirmMine	*minePtr;

	//-----------------------------------------------------------------//
	// count each kind of raw material that is being mined
	//-----------------------------------------------------------------//

	memset(raw_kind_mined, 0, sizeof(int)*MAX_RAW);

	for(i=0; i<ai_mine_count; i++)
	{
		firmPtr = firm_array[ai_mine_array[i]];
		minePtr = firmPtr->cast_to_FirmMine();

		if( minePtr->raw_id>=1 && minePtr->raw_id<=MAX_RAW )
			raw_kind_mined[minePtr->raw_id-1]++;
	}

	//-------------------- define parameters ----------------------//

	FirmInfo	*firmInfo = firm_res[FIRM_MINE];
	Location *locPtr, *siteLocPtr;
	Site		*sitePtr;
	Town		*townPtr;
	int		nearSite[MAX_RAW], minDist[MAX_RAW], townWithMine[MAX_RAW];
	short		buildXLoc[MAX_RAW], buildYLoc[MAX_RAW];
	short		dist;
	int		canBuild, connected, allHave;
	int		j, k, siteId;

	memset(townWithMine, 0, sizeof(int)*MAX_RAW);
	memset(nearSite, 0, sizeof(int)*MAX_RAW);
	memset(buildXLoc, 0, sizeof(short)*MAX_RAW);
	memset(buildYLoc, 0, sizeof(short)*MAX_RAW);

	for(i=0; i<MAX_RAW; i++)
		minDist[i] = 0x7FFFFF;

	//--------------------------------------------//
	// scan for the site array
	//--------------------------------------------//
	for(i=site_array.size(); i>0; i--)
	{
		if(site_array.is_deleted(i))
			continue;

		sitePtr = site_array[i];

		if( sitePtr->site_type != SITE_RAW )
			continue;

		siteLocPtr = world.get_loc(sitePtr->map_x_loc, sitePtr->map_y_loc);

		if(!siteLocPtr->can_build_firm())
			continue;

		siteId = sitePtr->object_id - 1;
		err_when(siteId<0 || siteId>MAX_RAW);

		if(townWithMine[siteId])
			continue; // a site connected to town is found before

		//--------------------------------------------//
		// continue if action to this site already exist
		//--------------------------------------------//

		if(is_action_exist(-1, -1, sitePtr->map_x_loc, sitePtr->map_y_loc, ACTION_AI_BUILD_FIRM, FIRM_MINE))
			continue;

		for(j=0; j<ai_town_count; j++)
		{
			townPtr = town_array[ai_town_array[j]];
			locPtr = world.get_loc(townPtr->loc_x1, townPtr->loc_y1);

			//-********* codes to move to other territory ***********-//
			if(siteLocPtr->region_id!=locPtr->region_id)
				continue; // not on the same territory

			dist = m.points_distance(sitePtr->map_x_loc, sitePtr->map_y_loc, townPtr->center_x, townPtr->center_y);

			//-------------------------------------------------------------------------//
			// check whether a mine is already connected to this town, if so, use it
			//-------------------------------------------------------------------------//
			for(connected=0, k=townPtr->linked_firm_count-1; k>=0; k--)
			{
				err_when(!townPtr->linked_firm_array[k] || firm_array.is_deleted(townPtr->linked_firm_array[k]));
				firmPtr = firm_array[townPtr->linked_firm_array[k]];

				if(firmPtr->nation_recno==nation_recno && firmPtr->firm_id==FIRM_MINE)
				{
					connected++;
					break;
				}
			}

			//-------------------------------------------------------------------------//
			// calculate the minimum distance from own towns
			//-------------------------------------------------------------------------//
			if(dist<minDist[siteId] || (connected && dist<=EFFECTIVE_FIRM_TOWN_DISTANCE))
			{
				//------ can build or not ----------//
				canBuild = 0;

				for(int ix=sitePtr->map_x_loc-firmInfo->loc_width+1; ix<=sitePtr->map_x_loc && !canBuild; ix++)
				{
					if(ix<0 || ix>=MAX_WORLD_X_LOC)
						continue;

					for(int iy=sitePtr->map_y_loc-firmInfo->loc_height+1; iy<=sitePtr->map_y_loc && !canBuild; iy++)
					{
						if(iy<0 || iy>=MAX_WORLD_Y_LOC)
							continue;

						if(world.can_build_firm(ix, iy, FIRM_MINE))
						{
							canBuild++;
							buildXLoc[siteId] = ix;
							buildYLoc[siteId] = iy;
							break;
						}
					}
				}

				if(canBuild)
				{
					nearSite[siteId] = i;
					minDist[siteId] = dist;
					
					if(connected && dist<=EFFECTIVE_FIRM_TOWN_DISTANCE)
						townWithMine[siteId]++;
				}
			}
		}

		for(allHave=1, j=0; j<MAX_RAW; j++)
		{
			if(!townWithMine[j])//if(!nearSite[j])
			{
				allHave = 0;
				break;
			}
		}

		if(allHave)
			break; // sites of each raw material have been found
	}
	
	//---------------------------------------------------------------------------//
	// determine which raw material is the best choice to build
	// Note: a better sorting algorithm should be used if there are many kind of
	//			raw material
	//---------------------------------------------------------------------------//
	int weight, pos;			// weight is the such kind of material mined, pos is the position in the array
	int siteRecno = 0;		// siteRecno is the recno of site to build
	int withoutThisRaw = 0;	// withoutThisRaw shows that this raw material is strongly recommended
	int closestDist=0x7FFFFF;
	
	for(pos=-1, weight=0x7FFFFF, j=0 ;j<MAX_RAW; j++)
	{
		if(!nearSite[j])
			continue; // no such kind of raw material

		if(!raw_kind_mined[j]) // no such kind of material and there is a possible site
		{
			if(withoutThisRaw)
			{
				if(minDist[j]<closestDist) // more than one kind of material we don't have
				{
					siteRecno = nearSite[j];
					closestDist = minDist[j];
					pos = j;
				}
			}
			else
			{
				siteRecno = nearSite[j];
				closestDist = minDist[j];
				withoutThisRaw++;
				pos = j;
			}
		}
		else if(!withoutThisRaw && weight>raw_kind_mined[j]) // scan for the kind of material with least num of this site
		{
			weight = raw_kind_mined[j];
			pos = j;
		}
	}

	if(!siteRecno && pos>=0)
		siteRecno = nearSite[pos];
	
	if(siteRecno)
	{
		sitePtr = site_array[siteRecno];
		xLoc = buildXLoc[pos];
		yLoc = buildYLoc[pos];
		refXLoc = sitePtr->map_x_loc;
		refYLoc = sitePtr->map_y_loc;

		err_when((xLoc-refXLoc)>=firm_res[FIRM_MINE]->loc_width || (yLoc-refYLoc)>=firm_res[FIRM_MINE]->loc_height);
		//--------------------------------------------------------------//
		// do some adjustment such that the firm will be built far away
		// from other firms by at least one step.
		//--------------------------------------------------------------//
		seek_best_build_mine_location(xLoc, yLoc, sitePtr->map_x_loc, sitePtr->map_y_loc);

		err_when((xLoc-refXLoc)>=firm_res[FIRM_MINE]->loc_width || (yLoc-refYLoc)>=firm_res[FIRM_MINE]->loc_height);
		return sitePtr->object_id;		// the raw id.
	}

	return 0;
}
//---------- End of function Nation::seek_mine --------//


//----- Begin of function Nation::seek_best_build_mine_location -----//
//
// <short&> xLoc	 -	a possible x location to build the mine, the final location is also returned by this reference
// <short&> yLoc	 -	a possible y location to build the mine.
//
// <short> mapXLoc -	the x location of the raw material
// <short> mapYLoc -	the y location of the raw material
//
void Nation::seek_best_build_mine_location(short& xLoc, short& yLoc, short mapXLoc, short mapYLoc)
{
	//--------------- define parameters -----------------//

	FirmInfo	*firmInfo = firm_res[FIRM_MINE];
	int		weight = 0, maxWeight = 0;
	short		xLeftLimit = mapXLoc-firmInfo->loc_width+1;
	short		yLeftLimit = mapYLoc-firmInfo->loc_height+1;
	short		resultXLoc = xLoc;
	short		resultYLoc = yLoc;
	short		ix, iy;

	for(ix=xLeftLimit; ix<=mapXLoc; ix++)
	{
		if(ix<0 || ix>=MAX_WORLD_X_LOC)
			continue;

		for(iy=yLeftLimit; iy<=mapYLoc; iy++)
		{
			if(iy<0 || iy>=MAX_WORLD_Y_LOC)
				continue;

			//---------------------------------------------------------------//
			// remove previous checked and useless locaton
			// Since all the possible location is checked from the top left
			// to the bottom right, the previous checked location should all
			// be impossible to build the mine.
			//---------------------------------------------------------------//
			if(ix<xLoc && iy<yLoc)
				continue;

			if(world.can_build_firm(ix, iy, FIRM_MINE))
			{
				//----------------------------------------//
				// calculate weight
				//----------------------------------------//
				weight = 0;
				cal_location_score(ix, iy, firmInfo->loc_width, firmInfo->loc_height, weight);

				if(weight>maxWeight)
				{
					resultXLoc = ix;
					resultYLoc = iy;

					if(weight == MAX_SCORE)	// very good locaton, stop checking
						break;

					maxWeight = weight;
				}
			}
		}
	}

	xLoc = resultXLoc;
	yLoc = resultYLoc;
}
//---------- End of function Nation::seek_best_build_mine_location --------//


//--------- Begin of function Nation::cal_location_score --------//
// Called by seek_best_build_mine_location() to calculate a specifed
// location score to build the mine.
//
// <short>	x1	- the upper left corner x location of the firm
// <short> y1 - the upper left corner y location of the firm
// <short> width - the width of the firm
// <short> height - the height of the firm
// <short&> score - return the location score
//
void Nation::cal_location_score(short x1, short y1, short width, short height, int& score)
{
	//-----------------------------------------------------------------//
	//	the score is calculated as follows, for instance, the firm is
	//	2x2 in dimension
	//
	// LU U U RU	L--left, R--right, U--upper, O--lower
	//	 L	x x R
	//	 L	x x R
	// LO O O RO
	//
	//	if any L can build, score += 1, if all set of L can build the total
	// score of this edge is 100. For each corner, the score is 50 if
	// can build.  Thus, the MAX. score == 600
	//
	//-----------------------------------------------------------------//
	short x, y, i, count;
	score = 0;

	//---------- left edge ---------//
	if((x=x1-1)>=0)
	{
		for(i=0, count=0; i<height; i++)
		{
			if(world.get_loc(x, y1+i)->can_build_firm())
				count++;
		}

		score += (count==height) ? 100 : count;
	}
	
	//---------- upper edge ---------//
	if((y=y1-1)>=0)
	{
		for(i=0, count=0; i<width; i++)
		{
			if(world.get_loc(x1+i, y)->can_build_firm())
				count++;
		}

		score += (count==width) ? 100 : count;
	}

	//---------- right edge ---------//
	if((x=x1+width)<MAX_WORLD_X_LOC)
	{
		for(i=0, count=0; i<height; i++)
		{
			if(world.get_loc(x, y1+i)->can_build_firm())
				count++;
		}

		score += (count==height) ? 100 : count;
	}
	
	//----------- lower edge -----------//

	if((y=y1+height)<MAX_WORLD_Y_LOC)
	{
		for(i=0, count=0; i<width; i++)
		{
			if(world.get_loc(x1+i, y)->can_build_firm())
				count++;
		}

		score += (count==width) ? 100 : count;
	}

	//------------------------------------------//
	// extra score
	//------------------------------------------//

	//------- upper left corner -------//
	if(x1>0 && y1>0 && world.get_loc(x1-1, y1-1)->can_build_firm())
		score += 50;

	//------- upper right corner ---------//
	if(x1<MAX_WORLD_X_LOC-1 && y1>0 && world.get_loc(x1+1, y1-1)->can_build_firm())
		score += 50;

	//----------- lower left corner ----------//
	if(x1>0 && y1<MAX_WORLD_Y_LOC-1 && world.get_loc(x1-1, y1+1)->can_build_firm())
		score += 50;

	//------- lower right corner ---------//
	if(x1<MAX_WORLD_X_LOC-1 && y1<MAX_WORLD_Y_LOC-1 && world.get_loc(x1+1, y1+1)->can_build_firm())
		score += 50;
}
//---------- End of function Nation::cal_location_score --------//


//----------- Begin of function Nation::find_best_firm_loc ----------//
//
// Determine the location of a new firm. It's best to have the
// new firm within the refective range of: towns, factories and
// mines.
//
// <short> buildFirmId		 - id. of the firm to be built
// <short> refXLoc, refYLoc - either the location of a town or a firm,
//									   the market must be built next to it.
// <short&> resultXLoc, resultYLoc - result location of the firm.
//
// return: <int> 1 - succeed, 0 - fail
//
int Nation::find_best_firm_loc(short buildFirmId, short refXLoc, short refYLoc, short& resultXLoc, short& resultYLoc)
{
	Location *locPtr = world.get_loc(refXLoc, refYLoc);
	short centerX, centerY, refX1, refY1, refX2, refY2;

	//-------- get the refective area ---------//

	int   originFirmRecno=0, originTownRecno=0;
	Firm* firmPtr;
	Town* townPtr;

	BYTE buildRegionId  = locPtr->region_id;
	int  buildIsPlateau = locPtr->is_plateau();

	if( locPtr->is_firm() )
	{
		originFirmRecno = locPtr->firm_recno();

		firmPtr = firm_array[originFirmRecno];

		centerX = firmPtr->center_x;
		centerY = firmPtr->center_y;

		refX1 = centerX - EFFECTIVE_FIRM_FIRM_DISTANCE;
		refY1 = centerY - EFFECTIVE_FIRM_FIRM_DISTANCE;
		refX2 = centerX + EFFECTIVE_FIRM_FIRM_DISTANCE;
		refY2 = centerY + EFFECTIVE_FIRM_FIRM_DISTANCE;

		if( firmPtr->firm_id == FIRM_HARBOR )
		{
			buildRegionId  = ((FirmHarbor*)firmPtr)->land_region_id;
			buildIsPlateau = 0;
		}
	}
	else if( locPtr->is_town() )
	{
		originTownRecno = locPtr->town_recno();

		townPtr = town_array[originTownRecno];

		centerX = townPtr->center_x;
		centerY = townPtr->center_y;

		refX1 = centerX - EFFECTIVE_FIRM_TOWN_DISTANCE;
		refY1 = centerY - EFFECTIVE_FIRM_TOWN_DISTANCE;
		refX2 = centerX + EFFECTIVE_FIRM_TOWN_DISTANCE;
		refY2 = centerY + EFFECTIVE_FIRM_TOWN_DISTANCE;
	}
	else
		err_here();

	//------------------------------------------------------//

	FirmInfo* firmInfo = firm_res[buildFirmId];
   int firmLocWidth   = firmInfo->loc_width;
	int firmLocHeight  = firmInfo->loc_height;

	refX1 -= firmLocWidth/2;		// since we use loc_x1 as the building reference, we need to shift it so it will match the use of center_x in effective distance
	refY1 -= firmLocHeight/2;
	refX1 = MAX(0, refX1);
	refY1 = MAX(0, refY1);

	if( refX2 - firmLocWidth/2 >= MAX_WORLD_X_LOC )
		refX2 = MAX_WORLD_X_LOC-1;
	else
		refX2 -= firmLocWidth/2;

	if( refY2 - firmLocHeight/2 >= MAX_WORLD_Y_LOC )
		refY2 = MAX_WORLD_Y_LOC-1;
	else
		refY2 -= firmLocHeight/2;

   err_when( refX2 >= MAX_WORLD_X_LOC );
	err_when( refY2 >= MAX_WORLD_Y_LOC );

	//-------- build a matrix on the refective area ---------//

	int 	 refWidth=refX2-refX1+1, refHeight=refY2-refY1+1;
	short* refMatrix = (short*) mem_add( sizeof(short) * refWidth * refHeight );
	short* refMatrixPtr;

	//------ initialize the weights of the matrix ------//

	int xLoc, yLoc;   	// inner locations in the matrix receives more weights than outer locations do
	int t1, t2;

	for( yLoc=refY1 ; yLoc<=refY2 ; yLoc++ )
	{
		refMatrixPtr = refMatrix + (yLoc-refY1)*refWidth;
		locPtr		 = world.get_loc( refX1, yLoc );

		for( xLoc=refX1 ; xLoc<=refX2 ; xLoc++, refMatrixPtr++, locPtr++ )
		{
			t1 = abs(xLoc-centerX);
			t2 = abs(yLoc-centerY);

			if( locPtr->region_id != buildRegionId ||
				 locPtr->is_plateau() != buildIsPlateau ||
				 locPtr->is_power_off() )
			{
				*refMatrixPtr = -1000;
			}
			else
			{
				*refMatrixPtr = 10-MAX(t1, t2);		// it's negative value, and the value is lower for the outer ones
			}
		}
	}

	//----- calculate weights of the locations in the matrix ----//

	int   xLocB, yLocB, weightAdd, weightReduce;
	short refBX1, refBY1, refBX2, refBY2;
	short refCX1, refCY1, refCX2, refCY2;

	for( yLoc=refY1 ; yLoc<=refY2 ; yLoc++ )
	{
		locPtr = world.get_loc(refX1, yLoc);

		for( xLoc=refX1 ; xLoc<=refX2 ; xLoc++, locPtr++ )
		{
			if( locPtr->region_id != buildRegionId ||
				 locPtr->is_plateau() != buildIsPlateau ||
				 locPtr->is_power_off() )
			{
				continue;
			}

			//------- if there is a firm on the location ------//

			weightAdd = 0;
			weightReduce = 0;

			if( locPtr->is_firm() )
			{
				firmPtr = firm_array[locPtr->firm_recno()];

				if( buildFirmId==FIRM_MARKET || buildFirmId==FIRM_FACTORY )		// only factories & market places need building close to other firms
				{
					int rc = 1;

					if( firmPtr->nation_recno != nation_recno )
						rc = 0;

					//----- check if the firm is of the right type ----//

					if( buildFirmId==FIRM_MARKET )		// build a market place close to mines and factories
					{
						if( firmPtr->firm_id!=FIRM_MINE && firmPtr->firm_id!=FIRM_FACTORY )	// market places should be built close to factories and mines and they are the only two firms that influence the location of the market place
							rc = 0;
					}
					else if( buildFirmId==FIRM_FACTORY )	// build a factory close to mines and market places
					{
						if( firmPtr->firm_id!=FIRM_MINE && firmPtr->firm_id!=FIRM_MARKET )	// market places should be built close to factories and mines and they are the only two firms that influence the location of the market place
							rc = 0;
					}

					//------------------------------------------/

					if( rc )
					{
						refBX1 = firmPtr->center_x - EFFECTIVE_FIRM_FIRM_DISTANCE;
						refBY1 = firmPtr->center_y - EFFECTIVE_FIRM_FIRM_DISTANCE;
						refBX2 = firmPtr->center_x + EFFECTIVE_FIRM_FIRM_DISTANCE;
						refBY2 = firmPtr->center_y + EFFECTIVE_FIRM_FIRM_DISTANCE;

						weightAdd = 30;
					}
				}

				refCX1 = firmPtr->loc_x1-1;		// add negative weights on space around this firm
				refCY1 = firmPtr->loc_y1-1;		// so to prevent firms from building right next to the firm
				refCX2 = firmPtr->loc_x2+1;		// and leave some space for walking path.
				refCY2 = firmPtr->loc_y2+1;

				weightReduce = 20;
			}

			//------- if there is a town on the location ------//

			else if( locPtr->is_town() )
			{
				townPtr = town_array[locPtr->town_recno()];

				refBX1 = townPtr->center_x - EFFECTIVE_FIRM_TOWN_DISTANCE;
				refBY1 = townPtr->center_y - EFFECTIVE_FIRM_TOWN_DISTANCE;
				refBX2 = townPtr->center_x + EFFECTIVE_FIRM_TOWN_DISTANCE;
				refBY2 = townPtr->center_y + EFFECTIVE_FIRM_TOWN_DISTANCE;

				weightAdd = townPtr->population*2;

				//----- if the town is not our own -----//

				if(townPtr->nation_recno != nation_recno)
				{
					if( townPtr->nation_recno==0 )  		// it's an independent town
						weightAdd = weightAdd * (100-townPtr->average_resistance(nation_recno)) / 100;
					else											// more friendly nations get higher weights
					{
						int relationStatus = get_relation_status(townPtr->nation_recno);

						if( relationStatus >= NATION_NEUTRAL )
							weightAdd = weightAdd * (relationStatus-NATION_NEUTRAL+1) / 4;
					}
				}

				refCX1 = townPtr->loc_x1-1;		// add negative weights on space around this firm
				refCY1 = townPtr->loc_y1-1;		// so to prevent firms from building right next to the firm
				refCX2 = townPtr->loc_x2+1;		// and leave some space for walking path.
				refCY2 = townPtr->loc_y2+1;

				weightReduce = 100;
			}
			else
				continue;

			//------ add weights to the matrix ------//

			if( weightAdd )
			{
				for( yLocB=MAX(refY1,refBY1) ; yLocB<=MIN(refY2,refBY2) ; yLocB++ )
				{
					xLocB = MAX(refX1,refBX1);
					refMatrixPtr = refMatrix + (yLocB-refY1)*refWidth + (xLocB-refX1);

					for( ; xLocB<=MIN(refX2,refBX2) ; xLocB++ )
					{
						*refMatrixPtr++ += weightAdd;
					}
				}
			}

			//------ reduce weights from the matrix ------//

			if( weightReduce )
			{
				for( yLocB=MAX(refY1,refCY1) ; yLocB<=MIN(refY2,refCY2) ; yLocB++ )
				{
					xLocB = MAX(refX1,refCX1);
					refMatrixPtr = refMatrix + (yLocB-refY1)*refWidth + (xLocB-refX1);

					for( ; xLocB<=MIN(refX2,refCX2) ; xLocB++ )
					{
						*refMatrixPtr++ -= weightReduce;
					}
				}
			}
		}
	}

	//------ select the best building site in the matrix -------//

	resultXLoc = -1;
	resultYLoc = -1;

	short thisWeight, bestWeight=0;

	refX2 -= firmLocWidth-1;			// do not scan beyond the border
	refY2 -= firmLocHeight-1;

	for( yLoc=refY1 ; yLoc<=refY2 ; yLoc++ )
	{
		for( xLoc=refX1 ; xLoc<=refX2 ; xLoc++ )
		{
			if( world.get_region_id(xLoc, yLoc) != buildRegionId ||
				 !world.can_build_firm(xLoc, yLoc, buildFirmId) )
			{
				continue;
			}

			//---- calculate the average weight of a firm area ----//

			int totalWeight=0;

			refMatrixPtr = refMatrix + (yLoc-refY1)*refWidth + (xLoc-refX1);

			for( int yCount=0 ; yCount<firmLocHeight ; yCount++ )
			{
				for( int xCount=0 ; xCount<firmLocWidth ; xCount++ )
				{
					totalWeight += *refMatrixPtr++;
				}

				refMatrixPtr += refWidth-firmLocWidth;
			}

			//------- compare the weights --------//

			thisWeight = totalWeight / (firmLocWidth*firmLocHeight);

			if( thisWeight > bestWeight )
			{
				bestWeight = thisWeight;

				resultXLoc = xLoc;
				resultYLoc = yLoc;
			}
		}
	}

	//------ release the refective matrix -----//

	mem_del( refMatrix );

	return resultXLoc >= 0;
}
//-------- End of function Nation::find_best_firm_loc --------//


