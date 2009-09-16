// Filename    : OSERES.CPP
// Description : Sound resource
//  search a sound effect id from subject verb object
//  pass the id to se_ctrl.request to trigger the sound effect
// Owner       : Gilbert


#include <OSERES.h>
#include <OSE.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OCONFIG.h>
#include <string.h>
#include <stdlib.h>

// ---------- Define constant ----------//
#define SERES_DB "SOUNDRES"
unsigned long SERes::last_select_time = 0;
unsigned long SERes::last_command_time = 0;
unsigned long SERes::select_sound_length = 600;	// time between successive select sound


// --------- begin of function SEInfo::match ----------//
int SEInfo::match(char subjectType, short subjectId, char *act,
						char objectType, short objectId)
{
	// object_type == -1, match all object type
	// object_id == -1, match all object id of that type
	return subject_type == subjectType && 
		(subject_id == -1 || subject_id == subjectId) &&
		strncmp(action, act, VERB_LEN )== 0 &&
		(object_type == -1 || (object_type == objectType &&
		 (object_id == -1 || object_id == objectId)));
}
// --------- end of function SEInfo::match ----------//


// --------- begin of function SERes::SERes ----------//
SERes::SERes()
{
	init_flag = 0;
	se_array_count = 0;
	se_array = NULL;
	se_index_count = 0;
	se_index_array = NULL;
	type_index_count = 0;
	type_index_array = NULL;
}
// --------- end of function SERes::SERes ----------//


// --------- begin of function SERes::~SERes ----------//
SERes::~SERes()
{
	deinit();
}
// --------- end of function SERes::~SERes ----------//


// --------- begin of function SERes::init1 ----------//
void SERes::init1()
{
	deinit();
	seed = m.get_time();
	load_info();
	sort_info();
	build_index();
	init_flag = 1;
}
// --------- end of function SERes::init1 ----------//


// --------- begin of function SERes::init2 ----------//
// called after se_ctrl is init
void SERes::init2(SECtrl *seCtrl)
{
	err_when( !seCtrl || !seCtrl->init_flag );
	err_when( init_flag == 0 );
	se_output = seCtrl;
	int i;
	SEInfo *seInfo;
	for(i = 0, seInfo = se_array; i < se_array_count; ++i, ++seInfo )
	{
		seInfo->effect_id = seCtrl->search_effect_id(seInfo->file_name);
	}

	init_flag = 2;
}
// --------- end of function SERes::init2 ----------//


// --------- begin of function SERes::deinit ----------//
void SERes::deinit()
{
	if( init_flag )
	{
		mem_del(se_array);
		se_array = NULL;
		se_array_count = 0;

		mem_del(se_index_array);
		se_index_array = NULL;
		se_index_count = 0;

		mem_del(type_index_array);
		type_index_array = NULL;
		type_index_count = 0;

		init_flag = 0;
	}
}
// --------- end of function SERes::deinit ----------//


// --------- begin of function SERes::load_info ----------//
void SERes::load_info()
{
	SERec *seRec;
	SEInfo *seInfo;
	int	i;
	Database *dbSE = game_set.open_db(SERES_DB);

	se_array_count = dbSE->rec_count();
	se_array = (SEInfo *)mem_add(sizeof(SEInfo) * se_array_count);
	memset( se_array, 0, sizeof(SEInfo) * se_array_count );

	for( i = 0; i < se_array_count; ++i )
	{
		seRec = (SERec *) dbSE->read(i+1);
		seInfo = se_array+i;

		// ------- copy subject ---------//
		seInfo->subject_type = seRec->subject_type;
		seInfo->subject_id = m.atoi(seRec->subject_id, seRec->RECNO_LEN);

		// -------- copy verb ---------//
		memcpy( seInfo->action, seRec->action, seRec->VERB_LEN );
		seInfo->action[seInfo->VERB_LEN] = '\0';
		m.rtrim( seInfo->action );

		// --------- copy object ---------//
		if( seRec->object_type == ' ' || seRec->object_type == '\0')
		{
			seInfo->object_type = 0;
			seInfo->object_id = 0;
		}
		else if( seRec->object_type == '*' )
		{
			seInfo->object_type = -1;		// all object
			seInfo->object_id = -1;
		}
		else
		{
			seInfo->object_type = seRec->object_type;
			if( seRec->object_id[0] != '*' )
				seInfo->object_id = m.atoi(seRec->object_id, seRec->RECNO_LEN);
			else
				seInfo->object_id = -1;		// all of the objectType
		}
		
		// -------- copy out frame ---------//
		seInfo->out_frame = m.atoi(seRec->out_frame, seRec->OUT_FRAME_LEN);
		err_when(seInfo->out_frame <= 0);

		// -------- copy file name --------//
		memcpy(seInfo->file_name, seRec->file_name, seRec->FILE_NAME_LEN);
		seInfo->file_name[seInfo->FILE_NAME_LEN] = '\0';
		m.rtrim(seInfo->file_name);
		seInfo->effect_id = 0;
	}
}
// --------- end of function SERes::load_info ----------//


// --------- begin of function SERes::sort_info --------//
static int seinfo_cmp(const void *r1, const void *r2)
{
	return memcmp(r1, r2, sizeof(SEInfo));
}

void SERes::sort_info()
{
	// notice object_id -1 are put after any other object_id 
	qsort(se_array, se_array_count, sizeof(SEInfo), seinfo_cmp);
}
// --------- end of function SERes::sort_info --------//


// --------- begin of function SERes::build_index -------//
// build index on (subject_type, subject_id)
void SERes::build_index()
{
	// ---------- first pass, count the size of index ---------//

	int i,j,k;
	SEInfo *seInfo;
	char lastType = -1;
	short lastId;

	type_index_count = 0;
	se_index_count = 0;

	for(i = 0, seInfo = se_array; i < se_array_count; ++i, ++seInfo)
	{
		if( lastType != seInfo->subject_type)
		{
			type_index_count++;
			se_index_count++;
			lastType = seInfo->subject_type;
			lastId = seInfo->subject_id;
		}
		else if( lastId != seInfo->subject_id)
		{
			se_index_count++;
			lastId = seInfo->subject_id;
		}
	}

	// --------- allocate memory for index ----------//

	SEInfoIndex *seIndex = se_index_array = (SEInfoIndex *)
		mem_resize( se_index_array, sizeof(SEInfoIndex) * se_index_count);
	memset( se_index_array, 0, sizeof(SEInfoIndex) * se_index_count);

	SETypeIndex *typeIndex = type_index_array = (SETypeIndex *)
		mem_resize( type_index_array, sizeof(SETypeIndex) * type_index_count);
	memset( type_index_array, 0, sizeof(SETypeIndex) * type_index_count);

	// ---------- pass 2, build indices -----------//
	seIndex--;					// move one step backward
	typeIndex--;
	j = -1;
	k = -1;
	lastType = -1;
	for(i = 0, seInfo = se_array; i < se_array_count; ++i, ++seInfo)
	{
		if( lastType != seInfo->subject_type)
		{
			// ----------- new (type,Id) ---------//
			++seIndex;
			++j;
			seIndex->subject_type = seInfo->subject_type;
			seIndex->subject_id = seInfo->subject_id;
			seIndex->start_rec = seIndex->end_rec = i;

			// ----------- new type ---------//
			++typeIndex;
			++k;
			typeIndex->subject_type = seInfo->subject_type;
			typeIndex->start_rec = typeIndex->end_rec = j;

			lastType = seInfo->subject_type;
			lastId = seInfo->subject_id;
		}
		else
		{
			err_when(typeIndex < type_index_array);	// must not enter here for the first time

			if( lastId != seInfo->subject_id)
			{
				// ----------- new (type,Id) ---------//
				++seIndex;
				++j;
				seIndex->subject_type = seInfo->subject_type;
				seIndex->subject_id = seInfo->subject_id;
				seIndex->start_rec = i;
				seIndex->end_rec = i;

				lastId = seInfo->subject_id;
			}
			else
			{
				err_when(seIndex < se_index_array);
				seIndex->end_rec = i;
			}
			typeIndex->end_rec = j;
		}
	}
}
// --------- end of function SERes::build_index -------//


// --------- begin of function SERes::scan ----------//
// search the SEInfo whose subject, action and object match
// <char> subjectType        type of subject 'S'= sprite,
//                           'U'=unit, 'R'=race, 'F'=firm, 'T'=town
// <short> subjectId         sprite_id, unit_id, race_id or firm_id ...
// <char *> act              name of the action, first four chars are significant
// [char] objectType         type of object 'S'= sprite,
//                           'U'=unit, 'R'=race, 'F'=firm, 'T'=town
//                           default : 0 (none)
// [short] objectId          sprite_id, unit_id, race_id or firm_id ...
//                           default : 0 (none)
//
// return NULL if not found
//
SEInfo* SERes::scan(char subjectType, short subjectId, char *act, 
						  char objectType, short objectId, int findFirst)
{
	err_when(!init_flag);
	
	int startRec, endRec, i;
	SETypeIndex *typeIndex;
	SEInfoIndex *seIndex;
	SEInfo *seInfo;
	
	// ---------- search the type_index_array ---------//
	int foundFlag = 0;
	for( i = 0, typeIndex = type_index_array; i < type_index_count; ++i, ++typeIndex)
	{
		if( subjectType == typeIndex->subject_type)
		{
			startRec = typeIndex->start_rec;
			endRec = typeIndex->end_rec;
			foundFlag = 1;
			break;
		}
	}
	if( !foundFlag )
		return NULL;

	// ---------- search the se_index_array ---------//
	foundFlag = 0;
	for( i = startRec, seIndex = se_index_array + startRec; i <= endRec;
		++i, ++seIndex)
	{
		if( subjectId == seIndex->subject_id && subjectType == seIndex->subject_type)
		{
			startRec = seIndex->start_rec;
			endRec = seIndex->end_rec;
			foundFlag = 1;
			break;
		}
	}
	if( !foundFlag )			// not found
		return NULL;

	// ----------- search the se_array ----------//
	for(i = startRec, seInfo = se_array + startRec; i <= endRec;
		++i, ++seInfo)
	{
		if( seInfo->match(subjectType, subjectId, act, objectType, objectId) )
			break;
	}
	if( i <= endRec )			// found
	{
		if( findFirst )
			return seInfo;
		int found = 1;
		SEInfo *retSEInfo = seInfo;
		for( ++i, ++seInfo ; i <= endRec && found <= 4 && 
			seInfo->match(subjectType, subjectId, act, objectType, objectId);
			++i, ++seInfo )
		{
			++found;
			if( random(found+1) == 0)
				retSEInfo = seInfo;
		}
		return retSEInfo;
	}
	return NULL;
}
// --------- end of function SERes::scan ----------//


// --------- begin of function SERes::scan_id ----------//
short SERes::scan_id(char subjectType, short subjectId, char *act, 
						  char objectType, short objectId, int findFirst)
{
	SEInfo *seInfo;
	if( (seInfo = scan(subjectType, subjectId, act, objectType, objectId, findFirst))
		!= NULL)
		return seInfo->effect_id;
	return 0;
}
// --------- end of function SERes::scan_id ----------//


// --------- begin of function SERes::operator[] ----------//
SEInfo* SERes::operator[] (int i)
{
	err_when(!init_flag || i <= 0 || i > se_array_count);
	return se_array + i - 1;
}
// --------- end of function SERes::operator[] ----------//


// --------- begin of function SERes::sound ----------//
void SERes::sound(short xLoc, short yLoc, short frame,
	char subjectType,short subjectId, char *action, char objectType,short objectId)
{
	//### begin trevor 20/8 ###//
	if( !config.sound_effect_flag )
		return;
	//### end trevor 20/8 ###//

	short relXLoc = xLoc - (world.zoom_matrix->top_x_loc + world.zoom_matrix->disp_x_loc/2);
	short relYLoc = yLoc - (world.zoom_matrix->top_y_loc + world.zoom_matrix->disp_y_loc/2);
	RelVolume relVolume( PosVolume(relXLoc, relYLoc) );
	if( !config.pan_control )
		relVolume.ds_pan = 0;
	SEInfo *seInfo;

	if( relVolume.rel_vol < 5)
		return;

	if( (seInfo=scan(subjectType, subjectId, action, objectType, objectId))
		!= NULL && frame == seInfo->out_frame)
	{
		se_output->request(seInfo->effect_id, relVolume );
	}
}
// --------- end of function SERes::sound ----------//


// --------- begin of function SERes::far_sound ----------//
//
// as same as far_sound, but no cut_off volume
// usually used in acknowlege voice
//
void SERes::far_sound(short xLoc, short yLoc, short frame,
	char subjectType,short subjectId, char *action, char objectType,short objectId)
{
	//### begin trevor 20/8 ###//
	if( !config.sound_effect_flag )
		return;
	//### end trevor 20/8 ###//

	short relXLoc = xLoc - (world.zoom_matrix->top_x_loc + world.zoom_matrix->disp_x_loc/2);
	short relYLoc = yLoc - (world.zoom_matrix->top_y_loc + world.zoom_matrix->disp_y_loc/2);
	RelVolume relVolume( PosVolume(relXLoc, relYLoc), 200, MAX_MAP_WIDTH );
	if( !config.pan_control )
		relVolume.ds_pan = 0;
	SEInfo *seInfo;

	if( relVolume.rel_vol < 80)
		relVolume.rel_vol = 80;

	if( (seInfo=scan(subjectType, subjectId, action, objectType, objectId))
		!= NULL && frame == seInfo->out_frame)
	{
		se_output->request(seInfo->effect_id, relVolume );
	}
}
// --------- end of function SERes::far_sound ----------//


// --------- begin of function SERes::mark_select_object_time ----------//
int SERes::mark_select_object_time()		// return false if this sound should be skipped due to too frequent
{
	unsigned long t = m.get_time();
	if( t - last_select_time >= select_sound_length )
	{
		last_select_time = t;
		return 1;
	}
	return 0;
}
// --------- end of function SERes::mark_select_object_time ----------//


// --------- begin of function SERes::mark_select_object_time ----------//
int SERes::mark_command_time()		// return false if this sound should be skipped due to too frequent
{
	unsigned long t = m.get_time();
//	if( t - last_command_time >= select_sound_length )
//	{
//		last_command_time = t;
//		return 1;
//	}
	if( t - last_select_time >= select_sound_length )
	{
		last_select_time = t;
		return 1;
	}
	return 0;
}
// --------- end of function SERes::mark_select_object_time ----------//


//-------------- Begin Function SERes::random ---------//
unsigned SERes::random(unsigned bound)
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   seed = MULTIPLIER * seed + INCREMENT;
	return seed % bound;
}
//-------------- End Function SERes::random ---------//
