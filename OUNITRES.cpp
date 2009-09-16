//Filename    : OUNITRES.CPP
//Description : Object Sprite Frame Resource

#include <ALL.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OGAMESET.h>
#include <ONATION.h>
#include <OSPRTRES.h>
#include <OUNITRES.h>
#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
#include <OTRANSL.h>
#endif

#ifdef NO_DEBUG_UNIT
#undef err_when
#undef err_here
#undef err_if
#undef err_else
#undef err_now
#define err_when(cond)
#define err_here()
#define err_if(cond)
#define err_else
#define err_now(msg)
#undef DEBUG
#endif

//-------- define file name -----------//

#define UNIT_DB    		  "UNIT"
#define UNIT_ATTACK_DB    "UNITATTK"

//-------- Begin of function UnitRes::init ---------//

void UnitRes::init()
{
	deinit();

	//----- open unit bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_UNITLI.RES";

	res_large_icon.init_imported(str, 1);  // 1-don't read all into buffer

	str  = DIR_RES;
	str += "I_UNITGI.RES";

	res_general_icon.init_imported(str, 1);  // 1-don't read all into buffer

	str  = DIR_RES;
	str += "I_UNITKI.RES";

	res_king_icon.init_imported(str, 1);  // 1-don't read all into buffer

	str  = DIR_RES;
	str += "I_UNITSI.RES";

	res_small_icon.init_imported(str, 1);  // 1-don't read all into buffer

	// ######## begin Gilbert 17/10 ##########//
	str  = DIR_RES;
	str += "I_UNITTI.RES";

	res_general_small_icon.init_imported(str, 1);  // 1-don't read all into buffer

	str  = DIR_RES;
	str += "I_UNITUI.RES";

	res_king_small_icon.init_imported(str, 1);  // 1-don't read all into buffer
	// ######## end Gilbert 17/10 ##########//

	//------- load database information --------//

	load_info();
	load_attack_info();

	//---------- set vars -----------//

	mobile_monster_count = 0;

	init_flag=1;
}
//--------- End of function UnitRes::init ----------//


//-------- Begin of function UnitRes::deinit ---------//

void UnitRes::deinit()
{
	if( init_flag )
	{
		mem_del(unit_info_array);
		mem_del(attack_info_array);

		res_large_icon.deinit();
		res_general_icon.deinit();
		res_king_icon.deinit();
		res_small_icon.deinit();
		// ######## begin Gilbert 17/10 ##########//
		res_general_small_icon.deinit();
		res_king_small_icon.deinit();
		// ######## end Gilbert 17/10 ##########//

		init_flag=0;
	}
}
//--------- End of function UnitRes::deinit ----------//


//------- Begin of function UnitRes::load_info ---------//

void UnitRes::load_info()
{
	Database  *dbUnit = game_set.open_db(UNIT_DB);
	UnitRec 	 *unitRec;
	UnitInfo  *unitInfo;
	long		 bitmapOffset;
	int		 i;

	unit_info_count = dbUnit->rec_count();
	unit_info_array = (UnitInfo*) mem_add(sizeof(UnitInfo) * unit_info_count);

	memset( unit_info_array, 0, sizeof(UnitInfo)*unit_info_count );

	//--------- read in frame information ---------//

	for( i=0 ; i<dbUnit->rec_count() ; i++ )
	{
		unitRec  = (UnitRec*) dbUnit->read(i+1);
		unitInfo = unit_info_array+i;

		m.rtrim_fld( unitInfo->name, unitRec->name, unitRec->NAME_LEN );
#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
		translate.multi_to_win(unitInfo->name, unitInfo->NAME_LEN);
#endif

		unitInfo->unit_id 	   = i+1;
		unitInfo->sprite_id	   = m.atoi(unitRec->sprite_id, unitRec->SPRITE_ID_LEN);
		unitInfo->dll_sprite_id	= m.atoi(unitRec->dll_sprite_id, unitRec->SPRITE_ID_LEN);
		unitInfo->race_id   	   = m.atoi(unitRec->race_id  , unitRec->RACE_ID_LEN);

		unitInfo->unit_class 	= unitRec->unit_class[0];
		unitInfo->mobile_type   = unitRec->mobile_type;

		unitInfo->visual_range  = m.atoi(unitRec->visual_range, unitRec->UNIT_PARA_LEN);
		unitInfo->visual_extend = m.atoi(unitRec->visual_extend , unitRec->UNIT_PARA_LEN);
		unitInfo->shealth       = m.atoi(unitRec->shealth       , unitRec->UNIT_PARA_LEN);
		unitInfo->hit_points    = m.atoi(unitRec->hit_points  , unitRec->UNIT_PARA_LEN);
		unitInfo->armor         = m.atoi(unitRec->armor       , unitRec->UNIT_PARA_LEN);

		unitInfo->build_days    = m.atoi(unitRec->build_days, unitRec->BUILD_DAYS_LEN);
		unitInfo->year_cost     = m.atoi(unitRec->year_cost, unitRec->YEAR_COST_LEN);
		unitInfo->build_cost    = unitInfo->year_cost;

		if( unitInfo->unit_class == UNIT_CLASS_WEAPON )
			unitInfo->weapon_power = unitRec->weapon_power-'0';

		unitInfo->carry_unit_capacity  = m.atoi(unitRec->carry_unit_capacity, unitRec->CARRY_CAPACITY_LEN);
		unitInfo->carry_goods_capacity = m.atoi(unitRec->carry_goods_capacity, unitRec->CARRY_CAPACITY_LEN);
		unitInfo->free_weapon_count 	 = m.atoi(unitRec->free_weapon_count, unitRec->FREE_WEAPON_COUNT_LEN);

		unitInfo->vehicle_id 	  = m.atoi(unitRec->vehicle_id	  , unitRec->SPRITE_ID_LEN);
		unitInfo->vehicle_unit_id = m.atoi(unitRec->vehicle_unit_id, unitRec->SPRITE_ID_LEN);

		unitInfo->transform_unit_id 	   = m.atoi(unitRec->transform_unit_id	    , unitRec->SPRITE_ID_LEN);
		unitInfo->transform_combat_level = m.atoi(unitRec->transform_combat_level, unitRec->UNIT_PARA_LEN);
		unitInfo->guard_combat_level     = m.atoi(unitRec->guard_combat_level, unitRec->UNIT_PARA_LEN);

		memcpy( &bitmapOffset, unitRec->large_icon_ptr, sizeof(long) );
		unitInfo->soldier_icon_ptr = res_large_icon.read_imported(bitmapOffset);

		if( unitRec->general_icon_file_name[0] != '\0' && unitRec->general_icon_file_name[0] != ' ')
		{
			memcpy( &bitmapOffset, unitRec->general_icon_ptr, sizeof(long) );
			unitInfo->general_icon_ptr = res_general_icon.read_imported(bitmapOffset);
		}
		else
		{
			unitInfo->general_icon_ptr = unitInfo->soldier_icon_ptr;
		}

		if( unitRec->king_icon_file_name[0] != '\0' && unitRec->king_icon_file_name[0] != ' ')
		{
			memcpy( &bitmapOffset, unitRec->king_icon_ptr, sizeof(long) );
			unitInfo->king_icon_ptr = res_king_icon.read_imported(bitmapOffset);
		}
		else
		{
			unitInfo->king_icon_ptr = unitInfo->soldier_icon_ptr;
		}

		// ###### begin Gilbert 17/10 ######//
		memcpy( &bitmapOffset, unitRec->small_icon_ptr, sizeof(long) );
		unitInfo->soldier_small_icon_ptr = res_small_icon.read_imported(bitmapOffset);

		if( unitRec->general_small_icon_file_name[0] != '\0' && unitRec->general_small_icon_file_name[0] != ' ')
		{
			memcpy( &bitmapOffset, unitRec->general_small_icon_ptr, sizeof(long) );
			unitInfo->general_small_icon_ptr = res_general_small_icon.read_imported(bitmapOffset);
		}
		else
		{
			unitInfo->general_small_icon_ptr = unitInfo->soldier_small_icon_ptr;
		}

		if( unitRec->king_small_icon_file_name[0] != '\0' && unitRec->king_small_icon_file_name[0] != ' ')
		{
			memcpy( &bitmapOffset, unitRec->king_small_icon_ptr, sizeof(long) );
			unitInfo->king_small_icon_ptr = res_king_small_icon.read_imported(bitmapOffset);
		}
		else
		{
			unitInfo->king_small_icon_ptr = unitInfo->soldier_small_icon_ptr;
		}
		// ###### end Gilbert 17/10 ######//

		unitInfo->first_attack = m.atoi(unitRec->first_attack, unitRec->UNIT_PARA_LEN);
		unitInfo->attack_count = m.atoi(unitRec->attack_count, unitRec->UNIT_PARA_LEN);
		unitInfo->die_effect_id = m.atoi(unitRec->die_effect_id, unitRec->UNIT_PARA_LEN);

		if( unitRec->all_know=='1' )
			memset( unitInfo->nation_tech_level_array, 1, sizeof(unitInfo->nation_tech_level_array) );
	}

	//--------- set vehicle info  ---------//

	for( i=0 ; i<unit_info_count ; i++ )
	{
		unitInfo = unit_info_array+i;

		if( unitInfo->vehicle_unit_id )
		{
			unit_info_array[ unitInfo->vehicle_unit_id-1 ].vehicle_id = unitInfo->vehicle_id;
			unit_info_array[ unitInfo->vehicle_unit_id-1 ].solider_id = i+1;
		}
	}
}
//-------- End of function UnitRes::load_info ---------//


//------- Begin of function UnitRes::load_attack_info ---------//

void UnitRes::load_attack_info()
{
	Database  		 *dbUnitAttack = game_set.open_db(UNIT_ATTACK_DB);
	UnitAttackRec 	 *attackRec;
	AttackInfo  	 *attackInfo;
	int		 		 i;

	attack_info_count = dbUnitAttack->rec_count();
	attack_info_array = (AttackInfo*) mem_add(sizeof(AttackInfo) * attack_info_count);

	memset( attack_info_array, 0, sizeof(AttackInfo)*attack_info_count );

	//--------- read in frame information ---------//

	for( i=0 ; i<dbUnitAttack->rec_count() ; i++ )
	{
		attackRec  = (UnitAttackRec*) dbUnitAttack->read(i+1);
		attackInfo = attack_info_array+i;

		attackInfo->combat_level     = m.atoi(attackRec->combat_level 	  , attackRec->COMBAT_LEVEL_LEN);
		attackInfo->attack_delay     = m.atoi(attackRec->attack_delay 	  , attackRec->UNIT_PARA_LEN);
		attackInfo->attack_range     = m.atoi(attackRec->attack_range 	  , attackRec->UNIT_PARA_LEN);
		attackInfo->attack_damage    = m.atoi(attackRec->attack_damage	  , attackRec->UNIT_PARA_LEN);
		attackInfo->pierce_damage 	  = m.atoi(attackRec->pierce_damage	  , attackRec->UNIT_PARA_LEN);
		attackInfo->bullet_out_frame = m.atoi(attackRec->bullet_out_frame, attackRec->UNIT_PARA_LEN);
		attackInfo->bullet_speed 	  = m.atoi(attackRec->bullet_speed	  , attackRec->UNIT_PARA_LEN);
		attackInfo->bullet_radius    = m.atoi(attackRec->bullet_radius       , attackRec->UNIT_PARA_LEN);
		attackInfo->bullet_sprite_id = m.atoi(attackRec->bullet_sprite_id, attackRec->UNIT_PARA_LEN);
		attackInfo->dll_bullet_sprite_id = m.atoi(attackRec->dll_bullet_sprite_id, attackRec->UNIT_PARA_LEN);
		attackInfo->eqv_attack_next  = m.atoi(attackRec->eqv_attack_next, attackRec->UNIT_PARA_LEN);
		attackInfo->min_power        = m.atoi(attackRec->min_power, attackRec->UNIT_PARA_LEN);
		attackInfo->consume_power    = m.atoi(attackRec->consume_power, attackRec->UNIT_PARA_LEN);
		attackInfo->fire_radius      = m.atoi(attackRec->fire_radius, attackRec->UNIT_PARA_LEN);
		attackInfo->effect_id        = m.atoi(attackRec->effect_id, attackRec->UNIT_PARA_LEN);
	}
}
//-------- End of function UnitRes::load_attack_info ---------//


//-------- Begin of function UnitRes::mobile_type_to_mask --------//

char UnitRes::mobile_type_to_mask(int mobileType)
{
	switch( mobileType)
	{
		case UNIT_LAND:
			return 1;

		case UNIT_SEA:
			return 2;

		case UNIT_AIR:
			return 3;

		default:
			err_here();
			return 0;
	}
}
//-------- End of function UnitRes::mobile_type_to_mask --------//


//-------- Begin of function UnitInfo::is_loaded -------//

int UnitInfo::is_loaded()
{
	return sprite_res[sprite_id]->is_loaded(); 
}
//--------- End of function UnitInfo::is_loaded --------//


//---- Begin of function UnitInfo::inc_nation_unit_count ----//

void UnitInfo::inc_nation_unit_count(int nationRecno)
{
	err_when( nationRecno<1 || nationRecno>nation_array.size() );

	nation_unit_count_array[nationRecno-1]++;

	//------- increase the nation's unit count ---------//

	Nation* nationPtr = nation_array[nationRecno];

	nationPtr->total_unit_count++;

	if( unit_class==UNIT_CLASS_WEAPON )
	{
		nationPtr->total_weapon_count++;
	}
	else if( unit_class==UNIT_CLASS_SHIP )
	{
		nationPtr->total_ship_count++;
	}
	else if( race_id )
	{
		nationPtr->total_human_count++;
	}
}
//----- End of function UnitInfo::inc_nation_unit_count -----//


//---- Begin of function UnitInfo::dec_nation_unit_count ----//

void UnitInfo::dec_nation_unit_count(int nationRecno)
{
	err_when( nationRecno<0 || nationRecno>MAX_NATION );

	if( nationRecno )
	{
		nation_unit_count_array[nationRecno-1]--;

		err_when( nation_unit_count_array[nationRecno-1] < 0 );

		//------ decrease the nation's unit count -------//

		Nation* nationPtr = nation_array[nationRecno];

		nationPtr->total_unit_count--;

		err_when( nationPtr->total_unit_count < 0 );

		if( unit_class==UNIT_CLASS_WEAPON )
		{
			nationPtr->total_weapon_count--;

			err_when( nationPtr->total_weapon_count < 0 );
		}
		else if( unit_class==UNIT_CLASS_SHIP )
		{
			nationPtr->total_ship_count--;

			err_when( nationPtr->total_ship_count < 0 );
		}
		else if( race_id )
		{
			nationPtr->total_human_count--;

			err_when( nationPtr->total_human_count < 0 );

			if( nationPtr->total_human_count < 0 )
				nationPtr->total_human_count = 0;
		}
	}
}
//----- End of function UnitInfo::dec_nation_unit_count -----//


//---- Begin of function UnitInfo::inc_nation_general_count ----//

void UnitInfo::inc_nation_general_count(int nationRecno)
{
	err_when( nationRecno<1 || nationRecno>nation_array.size() );

	nation_general_count_array[nationRecno-1]++;

	nation_array[nationRecno]->total_general_count++;
}
//----- End of function UnitInfo::inc_nation_general_count -----//


//---- Begin of function UnitInfo::dec_nation_general_count ----//

void UnitInfo::dec_nation_general_count(int nationRecno)
{
	err_when( nationRecno<0 || nationRecno>MAX_NATION );

	if( nationRecno )
	{
		nation_general_count_array[nationRecno-1]--;

		nation_array[nationRecno]->total_general_count--;
	}

	err_when( nation_general_count_array[nationRecno-1] < 0 );
}
//----- End of function UnitInfo::dec_nation_general_count -----//


//-------- Begin of function UnitInfo::unit_change_nation -------//
//
// Call this function when a unit changes its nation. This
// function updates the unit count vars in the UnitInfo.
//
// <int> newNationRecno - the new nation recno
// <int> oldNationRecno - the original nation recno
// <int> rankId			- the rank of the unit.
//
void UnitInfo::unit_change_nation(int newNationRecno, int oldNationRecno, int rankId)
{
	//---- update nation_unit_count_array[] ----//

	if( oldNationRecno )
	{
		if( rankId != RANK_KING )
			dec_nation_unit_count(oldNationRecno);

		if( rankId == RANK_GENERAL )
			dec_nation_general_count(oldNationRecno);
	}

	if( newNationRecno )
	{
		if( rankId != RANK_KING )
			inc_nation_unit_count(newNationRecno);

		if( rankId == RANK_GENERAL )		// if the new rank is general
			inc_nation_general_count(newNationRecno);
	}
}
//--------- End of function UnitInfo::unit_change_nation --------//

#ifdef DEBUG

//-------- Begin of function UnitRes::get_attack_info -------//

AttackInfo* UnitRes::get_attack_info(int attackId)
{
	if( attackId<1 || attackId>attack_info_count )
		err.run( "UnitRes::get_attack_info[]" );

	return attack_info_array+attackId-1;
}

//--------- End of function UnitRes::get_attack_info --------//


//-------- Begin of function UnitRes::operator[] -------//

UnitInfo* UnitRes::operator[](int unitId)
{
	if( unitId<1 || unitId>unit_info_count )
		err.run( "UnitRes::operator[]" );

	return unit_info_array+unitId-1;
}

//--------- End of function UnitRes::operator[] --------//

#endif

//-------- Begin of function UnitInfo::get_large_icon_ptr -------//

char *UnitInfo::get_large_icon_ptr(char rankId)
{
	switch( rankId )
	{
	case RANK_KING:
		return king_icon_ptr;
	case RANK_GENERAL:
		return general_icon_ptr;
	case RANK_SOLDIER:
	default:
		return soldier_icon_ptr;
	}
}
//-------- End of function UnitInfo::get_large_icon_ptr -------//

// ##### begin Gilbert 17/10 ########//
// -------- Begin of function UnitInfo::get_small_icon_ptr --------//
char *UnitInfo::get_small_icon_ptr(char rankId)
{
	switch( rankId )
	{
	case RANK_KING:
		return king_small_icon_ptr;
	case RANK_GENERAL:
		return general_small_icon_ptr;
	case RANK_SOLDIER:
	default:
		return soldier_small_icon_ptr;
	}
}
// -------- End of function UnitInfo::get_small_icon_ptr --------//
// ##### end Gilbert 17/10 ########//
