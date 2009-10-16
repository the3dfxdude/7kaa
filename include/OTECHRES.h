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

//Filename    : OTECHRES.H
//Description : class Tech

#ifndef __OTECHRES_H
#define __OTECHRES_H

#ifndef __ORESX_H
#include <ORESX.h>
#endif

//--------- Define constant -----------//

enum { TECH_SMALL_ICON_WIDTH=24,
		 TECH_SMALL_ICON_HEIGHT=20,
		 TECH_LARGE_ICON_WIDTH=46,
		 TECH_LARGE_ICON_HEIGHT=38
	  };

//------ define tech classes ----------//

enum { TECH_CLASS_COUNT=2 };

enum { TECH_WEAPON=1,
		 TECH_SHIP,
	  };

//------ define struct TechClassRec ---------//

struct TechClassRec
{
	enum { CODE_LEN=8, ICON_NAME_LEN=8 };

	char class_code[CODE_LEN];
	char icon_name[ICON_NAME_LEN];
};

//------ define struct TechRec ---------//

struct TechRec
{
	enum { CODE_LEN=8, MAX_TECH_LEVEL_LEN=3, COMPLEX_LEVEL_LEN=3, ID_LEN=3, ICON_NAME_LEN=8 };

	char class_code[CODE_LEN];

	char max_tech_level[MAX_TECH_LEVEL_LEN];
	char complex_level[COMPLEX_LEVEL_LEN];

	char unit_code[CODE_LEN];
	char firm_code[CODE_LEN];
	char parent_unit_code[CODE_LEN];
	char parent_firm_code[CODE_LEN];
	char parent_level;

	char icon_name[ICON_NAME_LEN];

	char class_id[ID_LEN];
	char unit_id[ID_LEN];
	char firm_id[ID_LEN];
	char parent_unit_id[ID_LEN];
	char parent_firm_id[ID_LEN];
};

//-------- define struct TechClass ----------//

#pragma pack(1)
class TechClass
{
public:
	short class_id;
	short first_tech_id;
	short tech_count;

	short icon_index;
	char* tech_icon();

	//-------- dynamic game vars --------//

	short  nation_research_firm_recno_array[MAX_NATION];	// the recno of each nation's research firm currently researching this technology

public:
	void   set_nation_research_firm_recno(int nationRecno, int firmRecno)	{ nation_research_firm_recno_array[nationRecno-1] = firmRecno; }
	int    get_nation_research_firm_recno(int nationRecno)						{ return nation_research_firm_recno_array[nationRecno-1]; }
	int	 is_nation_researching(int nationRecno)								   { return nation_research_firm_recno_array[nationRecno-1]; }
};
#pragma pack()

//-------- define struct TechInfo ----------//

#pragma pack(1)
class TechInfo
{
public:
	short tech_id;
	short class_id;

	short max_tech_level;		// maximum level for this technology
	short complex_level;

	short unit_id;
	short firm_id;
	short parent_unit_id;
	short parent_firm_id;
	char  parent_level;

	short icon_index;
	char* tech_large_icon();
	char* tech_small_icon();
	char* tech_des();			// description of the technology

	//-------- dynamic game vars --------//

	char   nation_tech_level_array[MAX_NATION];			// each nation's the technology level of this unit,
	char   nation_is_researching_array[MAX_NATION];		// whether the nation is researching this technology, it stores the number of firms of each nation researching on this technology.
	float  nation_research_progress_array[MAX_NATION];	// the progresses of each nation researching this technology, when it reaches complex_level, the research is done.

public:
	void   set_nation_tech_level(int nationRecno, int techLevel);
	int    get_nation_tech_level(int nationRecno)						{ return nation_tech_level_array[nationRecno-1]; }

	void	 inc_nation_is_researching(int nationRecno);
	void	 dec_nation_is_researching(int nationRecno);
	int	 is_nation_researching(int nationRecno)		   { return nation_is_researching_array[nationRecno-1]; }

	int 	 is_parent_tech_invented(int nationRecno);
	int 	 can_research(int nationRecno);
	int  	 progress(int nationRecno, float progressPoint);
	float	 get_progress(int nationRecno);
};
#pragma pack()

//------ define class TechRes ----------//

class TechRes
{
public:
	char        init_flag;

	short       tech_class_count;
	short       tech_count;
	short			total_tech_level;		// the sum of research levels of all technology

	TechClass* 	tech_class_array;
	TechInfo*  	tech_info_array;

	ResourceIdx	res_bitmap;

public:
	TechRes();

	void        init();
	void        deinit();

	void 			init_nation_tech(int nationRecno);
	void 			inc_all_tech_level(int nationRecno);

	int         write_file(File*);
	int         read_file(File*);

   #ifdef DEBUG
		TechInfo*   operator[](int techId);      // pass techId as recno
		TechClass*  tech_class(int techClassId);
	#else
		TechInfo*   operator[](int techId)			{ return tech_info_array+techId-1; }
		TechClass*  tech_class(int techClassId)   { return tech_class_array+techClassId-1; }
	#endif

private:
	void        load_tech_class();
	void        load_tech_info();
};

extern TechRes tech_res;

//------------------------------------//

#endif
