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

//Filename    : OF_RESE2.CPP
//Description : Firm Research Center - AI functions

#include <ONATION.h>
#include <OINFO.h>
#include <OTOWN.h>
#include <OUNIT.h>
#include <OTECHRES.h>
#include <OF_RESE.h>

//--------- Begin of function FirmResearch::process_ai ---------//

void FirmResearch::process_ai()
{
	//---- think about which technology to research ----//

	if( !tech_id )
		think_new_research();

	//------- recruit workers ---------//

	if( info.game_date%15==firm_recno%15 )
	{
		if( worker_count < MAX_WORKER )
			ai_recruit_worker();
	}

	//----- think about closing down this firm -----//

	if( info.game_date%30==firm_recno%30 )
	{
		if( think_del() )
			return;
	}
}
//----------- End of function FirmResearch::process_ai -----------//


//------- Begin of function FirmResearch::think_del -----------//
//
// Think about deleting this firm.
//
int FirmResearch::think_del()
{
	//----- if all technologies have been researched -----//

	if( nation_array[nation_recno]->total_tech_level() == tech_res.total_tech_level )		// all technology have been researched
	{
		ai_del_firm();
		return 1;
	}

   //----------------------------------------------// 

	if( worker_count > 0 )
		return 0;

	//-- check whether the firm is linked to any towns or not --//

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		if( linked_town_enable_array[i] == LINK_EE )
			return 0;
	}

	//------------------------------------------------//

	ai_del_firm();

	return 1;
}
//--------- End of function FirmResearch::think_del -----------//


//----- Begin of function FirmResearch::think_new_research ------//
//
// Think about which technology to research.
//
void FirmResearch::think_new_research()
{
	TechInfo* techInfo;
	int		 bestTechId=0, curRating, bestRating=0;

	for( int techId=tech_res.tech_count ; techId>0 ; techId-- )
	{
		techInfo = tech_res[techId];

		if( techInfo->can_research(nation_recno) )
		{
			curRating = 100 + techInfo->is_nation_researching(nation_recno)*20;

			if( curRating > bestRating ||
				 ( curRating==bestRating && misc.random(2)==0 ) )
			{
				bestTechId = techId;
				bestRating = curRating;
			}
		}
	}

	//------------------------------------//

	if( bestTechId )
		start_research( bestTechId, COMMAND_AI );
}
//------ End of function FirmResearch::think_new_research -------//

