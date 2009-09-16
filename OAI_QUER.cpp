//Filename   : OAI_QUER.CPP
//Description: AI - query functions

#include <ALL.H>
#include <OTOWN.H>
#include <OFIRM.H>
#include <ONATION.H>


//--------- Begin of function Nation::check_firm_ready --------//
//
// <short> xLoc, yLoc - locatino of the firm.
// [int]   firmId		 - id. of the firm. If not given, don't
//								verify the firm id. (default: 0)
//
// return 1 means firm exists and belongs to  our nation
// return 0 otherwise
//
int Nation::check_firm_ready(short xLoc, short yLoc, int firmId)
{
	Location *locPtr = world.get_loc(xLoc, yLoc);

	if(!locPtr->is_firm())
		return 0;	// no firm there

	short firmRecno = locPtr->firm_recno();

	if(firm_array.is_deleted(firmRecno))
		return 0;	// firm deleted

	Firm *firmPtr = firm_array[firmRecno];

	if(firmPtr->nation_recno!=nation_recno)
		return 0;	// firm changed nation

	if( firmId && firmPtr->firm_id!=firmId )
		return 0;

	return 1;
}
//---------- End of function Nation::check_firm_ready --------//


//--------- Begin of function Nation::check_town_ready --------//
//
// return 1 means town exists and belongs to  our nation
// return 0 otherwise
//
int Nation::check_town_ready(short xLoc, short yLoc)
{
	Location *locPtr = world.get_loc(xLoc, yLoc);

	if(!locPtr->is_town())
		return 0;	// no town there

	short townRecno = locPtr->town_recno();

	if(town_array.is_deleted(townRecno))
		return 0;	// town deleted

	Town *townPtr = town_array[townRecno];

	if(townPtr->nation_recno!=nation_recno)
		return 0;	// town changed nation

	return 1;
}
//---------- End of function Nation::check_town_ready --------//


//--------- Begin of function Nation::can_ai_build --------//
//
// Whether the AI can build the specific firm type next to
// the current firm.
//
int Nation::can_ai_build(int firmId)
{
	//------- check whether the AI has enough cash -----//

	if( cash < firm_res[firmId]->setup_cost )
		return 0;

	return 1;
}
//--------- End of function Nation::can_ai_build --------//



