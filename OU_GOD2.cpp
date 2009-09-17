//Filename   : OU_GOD2.CPP
//Description: God Unit - AI functions

#include <OGODRES.h>
#include <ORACERES.h>
#include <OF_CAMP.h>
#include <OU_GOD.h>

//------- Begin of function UnitGod::process_ai --------//

void UnitGod::process_ai()
{
	if( !is_ai_all_stop() )
		return;

	if( info.game_date%7 != sprite_recno%7 )
		return;

	switch( god_id )
	{
		case GOD_NORMAN:
			think_dragon();
			break;

		case GOD_MAYA:
			think_maya_god();
			break;

		case GOD_GREEK:
			think_phoenix();
			break;

		case GOD_VIKING:
			think_viking_god();
			break;

		case GOD_PERSIAN:
			think_persian_god();
			break;

		case GOD_CHINESE:
			think_chinese_dragon();
			break;

		case GOD_JAPANESE:
			think_japanese_god();
			break;

#if(MAX_RACE > 7)
		case GOD_EGYPTIAN:
			think_egyptian_god();
			break;

		case GOD_INDIAN:
			think_indian_god();
			break;

		case GOD_ZULU:
			think_zulu_god();
			break;
#endif
	}
}
//------- End of function UnitGod::process_ai --------//


//------- Begin of function UnitGod::think_dragon --------//

void UnitGod::think_dragon()
{
	int targetXLoc, targetYLoc;

	if( think_god_attack_target(targetXLoc, targetYLoc) )
		attack_firm( targetXLoc, targetYLoc );
}
//------- End of function UnitGod::think_dragon --------//


//------- Begin of function UnitGod::think_chinese_dragon --------//

void UnitGod::think_chinese_dragon()
{
	int targetXLoc, targetYLoc;

	if( think_god_attack_target(targetXLoc, targetYLoc) )
		attack_firm( targetXLoc, targetYLoc );
}
//------- End of function UnitGod::think_chinese_dragon --------//


//------- Begin of function UnitGod::think_phoenix --------//

void UnitGod::think_phoenix()
{
	int xLoc = m.random(MAX_WORLD_X_LOC);
	int yLoc = m.random(MAX_WORLD_Y_LOC);

	move_to( xLoc, yLoc );
}
//------- End of function UnitGod::think_phoenix --------//


//------- Begin of function UnitGod::think_viking_god --------//

void UnitGod::think_viking_god()
{
	int targetXLoc, targetYLoc;

	if( think_god_attack_target(targetXLoc, targetYLoc) )
	{
		go_cast_power(targetXLoc+1, targetYLoc+1, 2, COMMAND_AI);		// 2 - cast power type
	}
}
//------- End of function UnitGod::think_viking_god --------//


//------- Begin of function UnitGod::think_persian_god --------//

void UnitGod::think_persian_god()
{
	//------- there is no action, now think a new one ------//

	Nation* ownNation = nation_array[nation_recno];
	Firm*	  firmPtr;
	int  	  curRating, bestRating=0;
	int	  targetXLoc, targetYLoc;

	for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ ownNation->ai_camp_array[i] ];

		//----- calculate the injured rating of the camp ----//

		Worker* workerPtr = firmPtr->worker_array;
		int	  totalHitPoints = 0;
		int	  totalMaxHitPoints = 0;

		for( int j=0 ; j<firmPtr->worker_count ; j++, workerPtr++ )
		{
			totalHitPoints 	+= workerPtr->hit_points;
			totalMaxHitPoints += workerPtr->max_hit_points();
		}

		if( !totalMaxHitPoints )
			continue;

		curRating = 100 * (totalMaxHitPoints-totalHitPoints) / totalMaxHitPoints;

		//---- if the king is the commander of this camp -----//

		if( firmPtr->overseer_recno &&
			 unit_array[firmPtr->overseer_recno]->rank_id == RANK_KING )
		{
			curRating += 20;
		}

		if( curRating > bestRating )
		{
			bestRating = curRating;
			targetXLoc = firmPtr->center_x;
			targetYLoc = firmPtr->center_y;
		}
	}

	//-------------------------------------//

	if( bestRating )
	{
		go_cast_power(targetXLoc, targetYLoc, 1, COMMAND_AI);		// 1 - cast power type
	}
}
//------- End of function UnitGod::think_persian_god --------//


//------- Begin of function UnitGod::think_japanese_god --------//

void UnitGod::think_japanese_god()
{
	//------- there is no action, now think a new one ------//

	Nation* ownNation = nation_array[nation_recno];
	int  	  curRating, bestRating=0;
	int	  targetXLoc, targetYLoc;

	//------ think firm target --------//

	if( m.random(2)==0 )
	{
		for( int i=firm_array.size() ; i>0 ; i-- )
		{
			if( firm_array.is_deleted(i) )
				continue;

			Firm* firmPtr = firm_array[i];

			//------- only cast on camps ---------//

			if( firmPtr->firm_id != FIRM_CAMP )
				continue;

			//------ only cast on hostile and tense nations ------//

			if( ownNation->get_relation(firmPtr->nation_recno)->status > NATION_TENSE )
				continue;

			//------ calculate the rating of the firm -------//

			curRating = ((FirmCamp*)firmPtr)->total_combat_level()/10;

			if( curRating > bestRating )
			{
				bestRating  = curRating;
				targetXLoc  = firmPtr->center_x;
				targetYLoc  = firmPtr->center_y;
			}
		}
	}
	else
	{
		//------ think town target --------//

		for( int i=town_array.size() ; i>0 ; i-- )
		{
			if( town_array.is_deleted(i) )
				continue;

			Town* townPtr = town_array[i];

			//------ only cast on hostile and tense nations ------//

			if( townPtr->nation_recno && ownNation->get_relation(townPtr->nation_recno)->status > NATION_TENSE )
				continue;

			//------ calculate the rating of the firm -------//

			curRating = townPtr->population + (100-townPtr->average_loyalty());

			if( curRating > bestRating )
			{
				bestRating = curRating;
				targetXLoc = townPtr->center_x;
				targetYLoc = townPtr->center_y;
			}
		}
	}

	//-------------------------------------//

	if( bestRating )
	{
		go_cast_power(targetXLoc, targetYLoc, 1, COMMAND_AI);		// 1 - cast power type
	}
}
//------- End of function UnitGod::think_japanese_god --------//


//------- Begin of function UnitGod::think_god_attack_target --------//
//
// <int&> targetXLoc, targetYLoc - reference vars for returning the
//											  location of the target.
//
// return: <int> 1 - target selected
//					  0 - no target selected
//
int UnitGod::think_god_attack_target(int& targetXLoc, int& targetYLoc)
{
	Firm* 	firmPtr;
	Nation* 	ownNation = nation_array[nation_recno];
	int		curXLoc=next_x_loc(), curYLoc=next_y_loc();
	int   	totalFirm = firm_array.size();
	int   	firmRecno = m.random(totalFirm)+1;

	int i;
	for( i=totalFirm ; i>0 ; i-- )
	{
		if( ++firmRecno > totalFirm )
			firmRecno = 1;

		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->firm_id == FIRM_MONSTER )
			continue;			

		//-------- only attack enemies ----------//

		if( ownNation->get_relation(firmPtr->nation_recno)->status != NATION_HOSTILE )
			continue;

		//---- only attack enemy base and camp ----//

		if( firmPtr->firm_id != FIRM_BASE &&
			 firmPtr->firm_id != FIRM_CAMP )
		{
			continue;
		}

		//------- attack now --------//

		targetXLoc = firmPtr->loc_x1;
		targetYLoc = firmPtr->loc_y1;

		return 1;
	}

	//----- if there is no enemy to attack, attack Fryhtans ----//

	for( i=totalFirm ; i>0 ; i-- )
	{
		if( ++firmRecno > totalFirm )
			firmRecno = 1;

		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->firm_id == FIRM_MONSTER )
		{
			targetXLoc = firmPtr->loc_x1;
			targetYLoc = firmPtr->loc_y1;

			return 1;
		}
	}

	//---------------------------------------------------//

	return 0;
}
//------- End of function UnitGod::think_god_attack_target --------//


//------- Begin of function UnitGod::think_maya_god --------//

void UnitGod::think_maya_god()
{
	//------- there is no action, now think a new one ------//

	Nation* ownNation = nation_array[nation_recno];
	Firm*	  firmPtr;
	int  	  curRating, bestRating=0;
	int	  targetXLoc, targetYLoc;

	for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ ownNation->ai_camp_array[i] ];

		curRating = 0;

		if( firmPtr->overseer_recno )
		{
			Unit* unitPtr = unit_array[firmPtr->overseer_recno];

			if( unitPtr->race_id == RACE_MAYA && unitPtr->skill.combat_level < 100 )
				curRating += 10;
		}

		Worker* workerPtr = firmPtr->worker_array;

		for( int j=firmPtr->worker_count-1 ; j>=0 ; j--, workerPtr++ )
		{
			if( workerPtr->race_id == RACE_MAYA && workerPtr->combat_level < 100 )
				curRating += 5;
		}

		if( curRating > bestRating )
		{
			bestRating = curRating;
			targetXLoc = firmPtr->center_x;
			targetYLoc = firmPtr->center_y;
		}
	}

	//-------------------------------------//

	if( bestRating )
	{
		go_cast_power(targetXLoc, targetYLoc, 1, COMMAND_AI);		// 1 - cast power type
	}
}
//------- End of function UnitGod::think_maya_god --------//

#ifdef AMPLUS
//------- Begin of function UnitGod::think_egyptian_god --------//
void UnitGod::think_egyptian_god()
{
	Nation* ownNation = nation_array[nation_recno];
	int  	  curRating, bestRating=0;
	int	  targetXLoc, targetYLoc;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		Town* townPtr = town_array[i];

		//------ only cast on own nations ------//

		if( townPtr->nation_recno != nation_recno )
			continue;

		//------ calculate the rating of the firm -------//

		if( townPtr->population > MAX_TOWN_GROWTH_POPULATION - 5 )
			continue;

		// maximize the total loyalty gain.
		curRating = 5 * townPtr->average_loyalty();

		// calc rating on the number of people
		if( townPtr->population >= MAX_TOWN_GROWTH_POPULATION/2 )
			curRating -= (townPtr->population - MAX_TOWN_GROWTH_POPULATION/2) * 300 / MAX_TOWN_GROWTH_POPULATION;
		else
			curRating -= (MAX_TOWN_GROWTH_POPULATION/2 - townPtr->population) * 300 / MAX_TOWN_GROWTH_POPULATION;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			targetXLoc = townPtr->center_x;
			targetYLoc = townPtr->center_y;
		}
	}

	//-------------------------------------//

	if( bestRating )
	{
		go_cast_power(targetXLoc, targetYLoc, 1, COMMAND_AI);		// 1 - cast power type
	}
}
//------- End of function UnitGod::think_egyptian_god --------//


//------- Begin of function UnitGod::think_indian_god --------//
void UnitGod::think_indian_god()
{
	Nation* ownNation = nation_array[nation_recno];

	// see if any unit near by

	int castRadius = 2;
	int leftLocX = next_x_loc() - castRadius;
	if(leftLocX < 0)
		leftLocX = 0;

	int rightLocX = next_x_loc() + castRadius;
	if(rightLocX >= MAX_WORLD_X_LOC)
		rightLocX = MAX_WORLD_X_LOC-1;

	int topLocY = next_y_loc() - castRadius;
	if(topLocY < 0)
		topLocY = 0;

	int bottomLocY = next_y_loc() + castRadius;
	if(bottomLocY >= MAX_WORLD_Y_LOC)
		bottomLocY = MAX_WORLD_Y_LOC-1;

	int curRating = 0;
	int xLoc, yLoc;
	for( yLoc = topLocY; yLoc <= bottomLocY; ++yLoc)
	{
		for( xLoc = leftLocX; xLoc <= rightLocX; ++xLoc )
		{
			Location *locPtr = world.get_loc(xLoc, yLoc);
			int unitRecno;
			Unit *unitPtr;
			if( locPtr->has_unit(UNIT_LAND)
				&&	(unitRecno = locPtr->unit_recno(UNIT_LAND))
				&& !unit_array.is_deleted(unitRecno)
				&& (unitPtr = unit_array[unitRecno])

				&& unitPtr->nation_recno			// don't affect indepedent unit
				&& unitPtr->nation_recno != nation_recno
				&& (unitPtr->loyalty >= 20 && unitPtr->loyalty <= 60 ||
					unitPtr->loyalty <= 80 && unitPtr->target_loyalty < 30) )
			{
				switch( ownNation->get_relation(unitPtr->nation_recno)->status )
				{
				case NATION_HOSTILE:
					curRating += 3;
					break;

				case NATION_TENSE:
				case NATION_NEUTRAL:
					// curRating += 0;		// unchange
					break;

				case NATION_FRIENDLY:
					curRating -= 1;			// actually friendly humans are not affected
					break;

				case NATION_ALLIANCE:
					curRating -= 1;			// actually allied humans are not affected
					break;

				default:
					err_here();
				}
			}
		}
	}

	if( curRating > 1 )
	{
		// if enemy unit come near, cast
		go_cast_power(next_x_loc(), next_y_loc(), 1, COMMAND_AI);		// 1 - cast power type
	}
	else
	{
		// find any unit suitable, go to that area first
		int bestUnitCost = 20000;
		for( int unitRecno = unit_array.size(); unitRecno > 0; --unitRecno )
		{
			Unit *unitPtr;
			if( !unit_array.is_deleted(unitRecno)
				&& (unitPtr = unit_array[unitRecno])
				&& unitPtr->is_visible()
				&&	unitPtr->mobile_type == UNIT_LAND

				&& unitPtr->nation_recno			// don't affect indepedent unit
				&& unitPtr->nation_recno != nation_recno
				&& (unitPtr->loyalty >= 20 && unitPtr->loyalty <= 60 ||
					unitPtr->loyalty <= 80 && unitPtr->target_loyalty < 30)
				&& ownNation->get_relation(unitPtr->nation_recno)->status == NATION_HOSTILE )
			{
				int cost = m.points_distance(next_x_loc(), next_y_loc(), unitPtr->next_x_loc(), unitPtr->next_y_loc());
				if( cost < bestUnitCost )
				{
					bestUnitCost = cost;
					xLoc = unitPtr->next_x_loc();
					yLoc = unitPtr->next_y_loc();
				}
			}
		}

		if( bestUnitCost < 100 )
		{
			if( m.points_distance(next_x_loc(), next_y_loc(), xLoc, yLoc) <= god_res[god_id]->cast_power_range )
				go_cast_power(xLoc, yLoc, 1, COMMAND_AI);		// 1 - cast power type
			else
				move_to( xLoc, yLoc );
		}
		else if( m.random(4) == 0 )
		{
			// move to a near random location
			xLoc = next_x_loc() + m.random(100) - 50;
			if( xLoc < 0 )
				xLoc = 0;
			if( xLoc >= MAX_WORLD_X_LOC)
				xLoc = MAX_WORLD_X_LOC-1;
			yLoc = next_y_loc() + m.random(100) - 50;
			if( yLoc < 0 )
				yLoc = 0;
			if( yLoc >= MAX_WORLD_Y_LOC)
				yLoc = MAX_WORLD_Y_LOC-1;
			move_to( xLoc, yLoc );
		}
	}
}
//------- End of function UnitGod::think_indian_god --------//


//------- Begin of function UnitGod::think_zulu_god --------//
void UnitGod::think_zulu_god()
{
	//------- there is no action, now think a new one ------//

	Nation* ownNation = nation_array[nation_recno];
	Firm*	  firmPtr;
	int  	  curRating, bestRating=0;
	int	  targetXLoc, targetYLoc;

	for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ ownNation->ai_camp_array[i] ];

		curRating = 0;

		Unit* unitPtr;
		if( firmPtr->overseer_recno
			&& (unitPtr = unit_array[firmPtr->overseer_recno])
			&&	unitPtr->race_id == RACE_ZULU			// only consider ZULU leader
			&& unitPtr->skill.skill_level <= 70)
		{
			if( unitPtr->rank_id == RANK_KING )
				curRating += 5000;		// weak king need leadership very much

			if( unitPtr->skill.skill_level >= 40 )
				curRating += 5000 - (unitPtr->skill.skill_level - 40) * 60;	// strong leader need not be enhanced
			else
				curRating += 5000 - (40 - unitPtr->skill.skill_level) * 80;		// don't add weak leader

			// calculat the benefits to his soldiers
			Worker* workerPtr = firmPtr->worker_array;
			for( int j=firmPtr->worker_count-1 ; j>=0 ; j--, workerPtr++ )
			{
				if( workerPtr->race_id == RACE_ZULU )
					curRating += (unitPtr->skill.combat_level - workerPtr->combat_level)*2;
				else
					curRating += unitPtr->skill.combat_level - workerPtr->combat_level;
			}

			if( curRating > bestRating )
			{
				bestRating = curRating;
				targetXLoc = firmPtr->center_x;
				targetYLoc = firmPtr->center_y;
			}
		}
	}

	//-------------------------------------//

	if( bestRating )
	{
		go_cast_power(targetXLoc, targetYLoc, 1, COMMAND_AI);		// 1 - cast power type
	}
}
//------- End of function UnitGod::think_zulu_god --------//
#endif
