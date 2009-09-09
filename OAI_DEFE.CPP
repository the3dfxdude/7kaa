//Filename   : OAI_DEFE.CPP
//Description: AI on defense

#include <ALL.H>
#include <OTALKRES.H>
#include <ONATION.H>

//----- Begin of function Nation::ai_defend -----//
//
// <int> attackerUnitRecno - unit recno of the attacker.
//
int Nation::ai_defend(int attackerUnitRecno)
{
	//--- don't call for defense too frequently, only call once 7 days (since this function will be called every time our king/firm/town is attacked, so this filtering is necessary ---//

	if( info.game_date < ai_last_defend_action_date+7 )
		return 0;

	ai_last_defend_action_date = info.game_date;

	//---------- analyse the situation first -----------//

	Unit* attackerUnit = unit_array[attackerUnitRecno];

	err_when( attackerUnit->nation_recno == nation_recno );

	int attackerXLoc = attackerUnit->next_x_loc();
	int attackerYLoc = attackerUnit->next_y_loc();

	int hasWar;

	int enemyCombatLevel = mobile_defense_combat_level( attackerXLoc, attackerYLoc,
		attackerUnit->nation_recno, 0, hasWar );		// 0-don't return immediately even if there is war around this town

	//-- the value returned is enemy strength minus your own strength, so if it's positive, it means that your enemy is stronger than you, otherwise you're stronger than your enemy --//

	int attackCombatLevel = ai_attack_target(attackerXLoc, attackerYLoc, enemyCombatLevel, 1);		// 1-defense mode

	//------ request military aid from allies ----//

	if( attackCombatLevel < enemyCombatLevel && attackerUnit->nation_recno )
	{
		ai_request_military_aid();
	}

	return 1;
}
//----- End of function Nation::ai_defend -----//


//----- Begin of function Nation::ai_request_military_aid -----//
//
// Request allied nations to provide immediate military aid.
//
int Nation::ai_request_military_aid()
{
	return 0;		//**BUGHERE, multiplayer sync error, disabled temporarily

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( get_relation(i)->status != NATION_ALLIANCE )
			continue;

		if( should_diplomacy_retry(TALK_REQUEST_MILITARY_AID, i) )
		{
			talk_res.ai_send_talk_msg(i, nation_recno, TALK_REQUEST_MILITARY_AID);
			return 1;
		}
	}

	return 0;
}
//----- End of function Nation::ai_request_military_aid -----//
