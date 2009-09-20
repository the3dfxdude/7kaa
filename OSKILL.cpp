//Filename    : OSKILL.CPP
//Description : Skill class

#include <OSTR.h>
#include <OFIRMID.h>
#include <OSITE.h>
#include <OSKILL.h>

//----------- define static vars -----------//

char* Skill::skill_str_array[MAX_SKILL] =
{
	"Construction",
	"Leadership",
	"Mining",
	"Manufacture",
	"Research",
	"Spying",
	"Praying",
};

char* Skill::skill_code_array[MAX_SKILL] =
{
	"CONS",
	"LEAD",
	"MINE",
	"MANU",
	"RESE",
	"SPY",
	"PRAY",
};

char Skill::skilled_race_id_array[MAX_SKILL];	// the id. of the race that specialized in this skill.

//-------- Begin of function Skill::Skill -------//

Skill::Skill()
{
	memset( this, 0, sizeof(Skill) );
}

//-------- End of function Skill::Skill -------//


//-------- Begin of function Skill::skill_des -------//
//
// [int] shortWord - use short word (default:0)
//
char* Skill::skill_des(int shortWord)
{
	if( skill_id==0 )
		return "";
	else
		return skill_str_array[skill_id-1];
}
//-------- End of function Skill::skill_des -------//


//-------- Begin of function Skill::get_skill -------//
//
// Check if the this Skill structure has the
// specific skill.
//
// <int> skillId - skill id.
//
int Skill::get_skill(int skillId)
{
	if( skill_id==skillId )
		return skill_level;
	else
		return 0;
}
//-------- End of function Skill::get_skill -------//


