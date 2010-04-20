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

//Filename   : OAI_ECO.CPP
//Description: AI economy functions

#include <ALL.h>
#include <OUNIT.h>
#include <OFIRMALL.h>
#include <ONATION.h>


//--------- Begin of function Nation::think_reduce_expense --------//

void Nation::think_reduce_expense()
{
	if( true_profit_365days() > 0 || cash > 5000 * pref_cash_reserve / 100 )
		return;

	//-------- close down firms ---------//

	int  curRating, bestRating=0;
	Firm *firmPtr, *bestFirm=NULL;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->nation_recno != nation_recno )
			continue;

		if( firmPtr->firm_id == FIRM_WAR_FACTORY ||
			 firmPtr->firm_id == FIRM_RESEARCH )
		{
			curRating = 100-(int)firmPtr->productivity;
		}
		else
			continue;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestFirm = firmPtr;
		}
	}

	if( bestFirm )
		bestFirm->ai_del_firm();
}
//---------- End of function Nation::think_reduce_expense --------//


//----- Begin of function Nation::ai_should_spend -----//
//
// This function returns whether this nation should make
// a new spending.
//
// <int>   importanceRating - how important is the spending to the
//								    	nation.
// [float] spendAmt			 - if this is not given, then it will
//										consider generally - whether the
//										nation should spend money generally.
//
int Nation::ai_should_spend(int importanceRating, float spendAmt)
{
	if( cash < spendAmt )
		return 0;

	float fixedExpense = fixed_expense_365days();
	float stdCashLevel = MAX(fixedExpense,2000) * (150+pref_cash_reserve) / 100;
	float trueProfit = true_profit_365days();

	//----- if we are losing money, don't spend on non-important things -----//

	if( trueProfit < 0 )
	{
		if( 400 * (-trueProfit) / fixedExpense > importanceRating )
			return 0;
	}

	//--------------------------------------//

	float curCashLevel = 100 * (cash-spendAmt) / (stdCashLevel*2);

	return importanceRating >= (100-curCashLevel);
}
//------ End of function Nation::ai_should_spend ------//


//----- Begin of function Nation::ai_should_spend_war -----//
//
// This function returns whether the nation should spend money
// or war.
//
// <int> enemyMilitaryRating - the military_rank_rating() of the enemy
// [int] considerCeaseFire	  - whether this function is called when considering ceasing fire
//										 (default:0)
//
int Nation::ai_should_spend_war(int enemyMilitaryRating, int considerCeaseFire)
{
	int importanceRating = 30 + pref_military_development/5;		// 30 to 50

	importanceRating += military_rank_rating() - enemyMilitaryRating*2;

	if( considerCeaseFire )    	// only when we are very powerful, we will start a battle. So won't cease fire too soon after declaring war
		importanceRating += 20;		// less eary to return 0, for cease fire

	return ai_should_spend(importanceRating);
}
//------ End of function Nation::ai_should_spend_war ------//

