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

//Filename    : OSKILL.H
//Description : Skill class

#ifndef __OSKILL_H
#define __OSKILL_H

//--- Define the default skill of the citizens ---//

enum { CITIZEN_COMBAT_LEVEL = 10,
		 CITIZEN_SKILL_LEVEL  = 10,
		 CITIZEN_HIT_POINTS   = 20  };

//---------- Define constant ------------//

enum { MAX_SKILL=7,
		 MAX_TRAINABLE_SKILL=MAX_SKILL-1 };		// exclude praying

//---------- Define skill types -----------//

enum { SKILL_CONSTRUCTION=1,
		 SKILL_LEADING,
		 SKILL_MINING,
		 SKILL_MFT,
		 SKILL_RESEARCH,
		 SKILL_SPYING,
		 SKILL_PRAYING,
	  };

//-------- Define struct Skill ----------//

#pragma pack(1)
class Skill
{
public:
	char combat_level;
	char skill_id;
	char skill_level;		// if the unit is a town defender, this var is temporary used for storing the loyalty that will be added back to the town if the defender returns to the town

	unsigned char combat_level_minor;		// when combat_level_mirror >= 100, combat_level + 1
	unsigned char skill_level_minor;
	unsigned char skill_potential;		// skill potential

	static const char* skill_str_array[MAX_SKILL];
	static char* skill_code_array[MAX_SKILL];
	static char  skilled_race_id_array[MAX_SKILL];	// the id. of the race that specialized in this skill.
	static char  skill_train_cost_array[MAX_SKILL];

public:
	Skill();

	const char* skill_des(int shortWord=0);
	int 	get_skill(int skillId);
	void	set_skill(int skillId)		{ skill_id = skillId; }
};
#pragma pack()

//---------------------------------------//

#endif
