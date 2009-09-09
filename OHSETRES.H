// Filename    : OHSETRES.H
// Description : Header file for class HillRes
// Ownership   : Gilbert


#ifndef __OHSETRES_H
#define __OHSETRES_H

#ifndef __ORESDB_H
#include <ORESDB.H>
#endif

//------------ Define struct EqPatternRec ---------------//

struct EqPatternRec
{
	enum	{ PATTERN_ID_LEN = 3};
	char	pattern_id[PATTERN_ID_LEN];
	char	eqv_pattern_id[PATTERN_ID_LEN];
};

//------------ Define struct HSetRec ------------//

struct HSetRec
{
	enum { SET_ID_LEN=4, SIDE_LEN=2, LENGTH_LEN=2, STEP_LEN=2 };
	char	set_id[SET_ID_LEN];
	char	side[SIDE_LEN];
	char	replaced_length[LENGTH_LEN];
	char	replaced_length2[LENGTH_LEN];
	char	replaced_width[LENGTH_LEN];
	char	replaced_width2[LENGTH_LEN];
	char	step_num[STEP_LEN];
};


//---------- Define struct HSetInfo ------------//

struct HillSetInfo;
struct HSetInfo
{
	short	set_id;
	char	side;				// 1 = N, 2=NE, 3=E... 8=NW
	char	replaced_length;
	char	replaced_length2;
	char	replaced_width;
	char	replaced_width2;
	char	step_num;				// start from 1
	HillSetInfo	*first_hill_set;
};

//---------- Define struct HillSetRec ----------//
struct HillSetRec
{
	enum { SET_ID_LEN=4, STEP_LEN=2, PATTERN_ID_LEN=3, POST_MOVE_LEN=2 };
	char	set_id[SET_ID_LEN];
	char	step[STEP_LEN];
	char	pattern_id[PATTERN_ID_LEN];
	char	post_move[POST_MOVE_LEN];
};

//----------- Define struct HillSetInfo ----------//

struct HillSetInfo
{
	short set_id;
	char step;
	unsigned char pattern_id;
	char post_move;	// 0 = don't move, 1=N, 2=NE, 3=E ... 8=NW

	HillSetInfo *next_hill_set;
};

//----------- Define class HSetRes ---------------//

class HSetRes
{
public:

	int	hset_info_count;
	HSetInfo *hset_info_array;

	int	hill_set_info_count;
	HillSetInfo *hill_set_info_array;

	// similar pattern
	// if a pattern (pattern representative) has another similar pattern,
	// eqv_pattern_count[pattern_id-1] > 0
	// the first similar pattern is eqv_pattern_id[4*(pattern_id-1) + 0]
	// and 4th (last) one is eqv_pattern_id[4*(pattern_id-1) + 3]
	enum { MAX_EQV_PATTERN =4 };
	unsigned max_pattern_id;
	unsigned char	*eqv_pattern_count;
	unsigned char	*eqv_pattern_id;

	char			init_flag;
	ResourceDb	res_bitmap;

public:
	HSetRes();

	void			init(unsigned maxPatternId);
	void			deinit();

	//-------- function related to Pattern ---------//
	unsigned char	get_eqv_pattern_count(unsigned char patternId);
	unsigned char	get_eqv_pattern_id(unsigned char patternId, unsigned char seqno); // seqno = 0 to 3
	unsigned char	get_random_pattern(unsigned char patternId);


	//-------- function related to HSet and HillSet -------//
	HSetInfo *get_hset(short hSetId)						{ return hset_info_array+(hSetId-1);}
	HillSetInfo *get_first_hill_set(short hSetId)	{ return hset_info_array[hSetId-1].first_hill_set;}
	HillSetInfo *get_first_hill_set(HSetInfo *hSetInfo)		{ return hSetInfo->first_hill_set;}
	HillSetInfo *get_next_hill_set(HillSetInfo *hillSetInfo)	{ return hillSetInfo->next_hill_set; }

	//  select HillSetInfo where side = ?? and length <= ? and width <= ? and
	//  length2 <= ? and width2 <= ?
	// for N,E,S,W side, set maxLength2 and maxWidth2 to 0
	// isSide = 9 for standalone hill
	HSetInfo *random_hset(char isSide, char maxLength, char maxWidth, char maxLength2, char maxWidth2);

private:
	void			load_eqv_pattern_info(unsigned maxPatternId);
	void			load_hset_info();
	void			load_hill_set_info();
};

//----------------------------------------------------//

#endif
