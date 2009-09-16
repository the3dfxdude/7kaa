// Filename    : OSERES.H
// Description : Header file of sound resource
// Onwer       : Gilbert


#ifndef __OSERES_H
#define __OSERES_H


// ------------- Define struct SERec -------------//

struct SERec
{
	enum	{ RECNO_LEN=3, CODE_LEN=12, VERB_LEN=4, OUT_FRAME_LEN=3, FILE_NAME_LEN=8};
	char	subject_type;
	char	subject_code[CODE_LEN];
	char	subject_id[RECNO_LEN];
	char	action[VERB_LEN];
	char	object_type;
	char	object_id[RECNO_LEN];
	char	out_frame[OUT_FRAME_LEN];
	char	file_name[FILE_NAME_LEN];

};

// ------------- Define struct SEInfo -------------//

struct SEInfo
{
	enum { VERB_LEN=4, FILE_NAME_LEN=8 };
	char	subject_type;	// S=sprite, U=unit, R=race, F=firm, T=town
	short	subject_id;
	char	action[VERB_LEN+1];		// '\0' padding
	char	object_type;
	short	object_id;
	short out_frame;
	char	file_name[FILE_NAME_LEN+1];
	short	effect_id;				// id returned from se_ctrl.scan

	int match(char, short, char *, char, short);
};

// ---------- Define struct SEInfoIndex -------//

struct SEInfoIndex
{
	char	subject_type;
	char	dummy;
	short	subject_id;
	int	start_rec;
	int	end_rec;
};

// ---------- Define struct SETypeIndex -------//

struct SETypeIndex
{
	char	subject_type;
	char	dummy;
	int	start_rec;
	int	end_rec;
};

// ------------- Define class SERes -------------//

class SECtrl;

class SERes
{
public:
	int		init_flag;
	SECtrl*	se_output;
	int		se_array_count;
	SEInfo*	se_array;
	int		se_index_count;
	SEInfoIndex *se_index_array;
	int		type_index_count;
	SETypeIndex *type_index_array;

	static unsigned long last_select_time;
	static unsigned long last_command_time;
	static unsigned long select_sound_length;
	unsigned seed;

public:
	SERes();
	~SERes();

	void		init1();		// init before se_ctrl.init
	void		init2(SECtrl*);		// init after se_ctrl.init
	void		deinit();

	SEInfo*	scan(char,short, char *, char,short, int findFirst=0);
	short		scan_id(char,short, char *, char,short, int findFirst=0);
	SEInfo*	operator[] (int);
	void		sound(short xLoc, short yLoc, short frame, char,short, char *, char=0,short=0);
	void		far_sound(short xLoc, short yLoc, short frame, char,short, char *, char=0,short=0);

	static int mark_select_object_time();		// return false if this sound should be skipped due to too frequent
	static int mark_command_time();				// return false if this sound should be skipped due to too frequent

private:
	void load_info();
	void sort_info();
	void build_index();
	unsigned	random(unsigned);
};

extern SERes se_res;

#endif

