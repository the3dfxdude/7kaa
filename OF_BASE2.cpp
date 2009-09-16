//Filename    : OF_BASE2.CPP
//Description : Firm Base - part 2, the AI functions

#include <ONATION.h>
#include <OTOWN.h>
#include <OF_BASE.h>

//--------- Begin of function FirmBase::process_ai ---------//

void FirmBase::process_ai()
{
	think_assign_unit();

	think_invoke_god();
}
//----------- End of function FirmBase::process_ai -----------//


//--------- Begin of function FirmBase::think_assign_unit ---------//

void FirmBase::think_assign_unit()
{
	Nation* nationPtr = nation_array[nation_recno];

	//-------- assign overseer ---------//

	if( info.game_date%15==firm_recno%15 )		// do not call too often because when an AI action is queued, it will take a while to carry it out
	{
		if( !overseer_recno )
			nationPtr->add_action(loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_BASE);
	}

	//------- recruit workers ---------//

	if( info.game_date%15==firm_recno%15 )
	{
		if( worker_count < MAX_WORKER )
			ai_recruit_worker();
	}
}
//----------- End of function FirmBase::think_assign_unit -----------//


//--------- Begin of function FirmBase::think_invoke_god ---------//

void FirmBase::think_invoke_god()
{
	if( pray_points < MAX_PRAY_POINTS || !can_invoke() )
		return;

	invoke_god();
}
//----------- End of function FirmBase::think_invoke_god -----------//


