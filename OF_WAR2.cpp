//Filename    : OF_WAR2.CPP
//Description : Firm War Factory - AI functions

#include <ONATION.H>
#include <OINFO.H>
#include <OTOWN.H>
#include <OUNIT.H>
#include <OTECHRES.H>
#include <OF_WAR.H>

//--------- Begin of function FirmWar::process_ai ---------//

void FirmWar::process_ai()
{
	//---- think about which technology to research ----//

	if( !build_unit_id )
		think_new_production();

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
//----------- End of function FirmWar::process_ai -----------//


//------- Begin of function FirmWar::think_del -----------//
//
// Think about deleting this firm.
//
int FirmWar::think_del()
{
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
//--------- End of function FirmWar::think_del -----------//


//----- Begin of function FirmWar::think_new_production ------//
//
// Think about which weapon to produce.
//
void FirmWar::think_new_production()
{
	//----- first see if we have enough money to build & support the weapon ----//

	if( !should_build_new_weapon() )
		return;

	//---- calculate the average instance count of all available weapons ---//

	int 		 weaponTypeCount=0, totalWeaponCount=0;
	UnitInfo* unitInfo;

	for( int unitId=1; unitId<=MAX_UNIT_TYPE ; unitId++ )
	{
		unitInfo = unit_res[unitId];

		if( unitInfo->unit_class != UNIT_CLASS_WEAPON ||
			 unitInfo->get_nation_tech_level(nation_recno) == 0 )
		{
			continue;
		}

		if( unitId == UNIT_EXPLOSIVE_CART )		// AI doesn't use Porcupine
			continue;

		weaponTypeCount++;
		totalWeaponCount += unitInfo->nation_unit_count_array[nation_recno-1];
	}

	if( weaponTypeCount==0 )		// none of weapon technologies is available
		return;

	int averageWeaponCount = totalWeaponCount/weaponTypeCount;

	//----- think about which is best to build now ------//

	int curRating, bestRating=0, bestUnitId=0;

	for( unitId=1; unitId<=MAX_UNIT_TYPE ; unitId++ )
	{
		unitInfo = unit_res[unitId];

		if( unitInfo->unit_class != UNIT_CLASS_WEAPON )
			continue;

		int techLevel = unitInfo->get_nation_tech_level(nation_recno);

		if( techLevel==0 )
			continue;

		if( unitId == UNIT_EXPLOSIVE_CART )		//**BUGHERE, don't produce it yet, it needs a different usage than the others.
			continue;

		int unitCount = unitInfo->nation_unit_count_array[nation_recno-1];

		curRating = averageWeaponCount-unitCount + techLevel*3;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestUnitId = unitId;
		}
	}

	//------------------------------------//

	if( bestUnitId )
		add_queue( bestUnitId );
}
//------ End of function FirmWar::think_new_production -------//


//----- Begin of function FirmWar::should_build_new_weapon ------//
//
int FirmWar::should_build_new_weapon()
{
	//----- first see if we have enough money to build & support the weapon ----//

	Nation* nationPtr = nation_array[nation_recno];

	if( nationPtr->true_profit_365days() < 0 )		// don't build new weapons if we are currently losing money
		return 0;

	if( nationPtr->expense_365days(EXPENSE_WEAPON) >
		 nationPtr->income_365days() * 30 + nationPtr->pref_use_weapon/2 )		// if weapon expenses are larger than 30% to 80% of the total income, don't build new weapons
	{
		return 0;
	}

	//----- see if there is any space on existing camps -----//

	Firm* firmPtr;

	for( int i=0 ; i<nationPtr->ai_camp_count ; i++ )
	{
		firmPtr = firm_array[ nationPtr->ai_camp_array[i] ];

		if( firmPtr->region_id != region_id )
			continue;

		if( firmPtr->worker_count < MAX_WORKER )		// there is space in this firm
			return 1;
	}

	return 0;
}
//------ End of function FirmWar::should_build_new_weapon -------//
