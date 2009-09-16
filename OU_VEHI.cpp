//Filename    : OU_VEHI.CPP
//Description : Unit Vehicle

#include <OWORLD.h>
#include <OUNITRES.h>
#include <OU_VEHI.h>

//--------- Begin of function UnitVehicle::set_combat_level ---------//
//
void UnitVehicle::set_combat_level(int combatLevel)
{
	skill.combat_level = combatLevel;

	UnitInfo* unitInfo = unit_res[unit_id];

	max_hit_points = unit_res[unitInfo->vehicle_id]->hit_points +
						  unit_res[unitInfo->solider_id]->hit_points * combatLevel / 100;

	hit_points = min(hit_points, max_hit_points);
}
//-------- End of function UnitVehicle::set_combat_level -----------//


//--------- Begin of function UnitVehicle::dismount ---------//
//
// The solider unit dismount from the vehicle.
//
// This unit is decomposed into two units: the solider and the vehicle.
//
void UnitVehicle::dismount()
{
	err_when( !unit_res[unit_id]->solider_id );

	UnitInfo* unitInfo = unit_res[unit_id];

	SpriteInfo* soliderSpriteInfo = sprite_res[ unit_res[unitInfo->solider_id]->sprite_id ];

	//--- calc the hit points of the solider and the vehicle after deforming ---//

	float soliderHitPoints = hit_points * solider_hit_points / (solider_hit_points+vehicle_hit_points);
	float vehicleHitPoints = hit_points * vehicle_hit_points / (solider_hit_points+vehicle_hit_points);

	soliderHitPoints = max(1, soliderHitPoints);
	vehicleHitPoints = max(1, vehicleHitPoints);

	//-------- add the solider unit ---------//

	//---- look for an empty location for the unit to stand ----//

	int xLoc=next_x_loc(), yLoc=next_y_loc();

	if( !world.locate_space( xLoc, yLoc, xLoc+sprite_info->loc_width-1,
									 yLoc+sprite_info->loc_height-1, soliderSpriteInfo->loc_width, soliderSpriteInfo->loc_height ) )
	{
		return;
	}

	int unitRecno = unit_array.add_unit(unitInfo->solider_id, nation_recno,
							 rank_id, loyalty, xLoc, yLoc);

	Unit* unitPtr = unit_array[unitRecno];

	unitPtr->skill = skill;
	unitPtr->set_combat_level(skill.combat_level);
	unitPtr->hit_points = soliderHitPoints;

	//-------- delete current unit ----------//

	int curXLoc = next_x_loc(), curYLoc = next_y_loc();

	unit_array.del(sprite_recno);		// delete the vehicle (e.g. horse)

	//------- add the vehicle unit ----------//

	unitRecno = unit_array.add_unit(unitInfo->vehicle_id, 0, 0, 0, curXLoc, curYLoc);

	unit_array[unitRecno]->hit_points = vehicleHitPoints;
}
//-------- End of function UnitVehicle::dismount -----------//

