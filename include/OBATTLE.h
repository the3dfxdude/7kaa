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

//Filename    : OBATTLE.H
//Description : Header file for Battle object

#ifndef __OBATTLE_H
#define __OBATTLE_H

//---------- Define class Battle --------//

// ##### begin Gilbert 18/8 ######//
struct NewNationPara;
// ##### end Gilbert 18/8 ######//

class Battle
{
public:
	void  init();
	void	deinit();

	// ##### begin Gilbert 18/8 ######//
	void	run(NewNationPara* mpGame, int mpPlayerCount=0);
	// ##### end Gilbert 18/8 ######//

	#ifdef DEBUG
		void	run_sim();
	#endif

	void	run_loaded();
	void	run_replay();
	void	run_test();

private:
	void 	create_ai_nation(int aiNationCount);
	void 	create_pregame_object();

	int 	create_town(int nationRecno, int raceId, int& xLoc, int& yLoc);
	int 	create_unit(int townRecno, int unitId, int rankId);
	void	create_test_unit(int nationRecno);
};

extern Battle battle;

//---------------------------------------//

#endif
