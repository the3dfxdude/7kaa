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