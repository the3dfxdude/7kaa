// Filename    : OSNOWRES.H
// Description : Header file of snow resource
// Owner       : Gilbert

#ifndef __OSNOWRES_H
#define __OSNOWRES_H

#ifndef __ORESDB_H
#include <ORESDB.h>
#endif

// ----------- Define struct SnowRec --------- //
//
struct SnowRec
{
	enum { FILE_NAME_LEN=8, OFFSET_LEN=3, BITMAP_PTR_LEN=4, RECNO_LEN=4 };
	char	file_name[FILE_NAME_LEN];
	char	offset_x[OFFSET_LEN];
	char	offset_y[OFFSET_LEN];
	char	next_file1[FILE_NAME_LEN];
	char	next_file2[FILE_NAME_LEN];
	char	next_file3[FILE_NAME_LEN];
	char	next_file4[FILE_NAME_LEN];
	char	prev_file1[FILE_NAME_LEN];
	char	prev_file2[FILE_NAME_LEN];
	char	bitmap_ptr[BITMAP_PTR_LEN];
	char	next_ptr1[RECNO_LEN];
	char	next_ptr2[RECNO_LEN];
	char	next_ptr3[RECNO_LEN];
	char	next_ptr4[RECNO_LEN];
	char	prev_ptr1[RECNO_LEN];
	char	prev_ptr2[RECNO_LEN];
};

// ---------- Define struct SnowInfo -------//
//
struct SnowInfo
{
	enum { MAX_NEXT_PTR=4, MAX_PREV_PTR=2 };
	int				snow_map_id;
	char*				bitmap_ptr;
	SnowInfo*		next_file[MAX_NEXT_PTR];
	SnowInfo*		prev_file[MAX_PREV_PTR];
	short				offset_x;
	short				offset_y;
	unsigned short next_count;
	unsigned short prev_count;

	int				is_root()	{ return prev_count == 0; }
	int				is_leaf()	{ return next_count == 0; }
	int				rand_next(unsigned rand)	{ return is_leaf() ? snow_map_id : next_file[rand % next_count]->snow_map_id; }
	int				rand_prev(unsigned rand)	{ return is_root() ? snow_map_id : prev_file[rand % prev_count]->snow_map_id; }
	SnowInfo *		rand_next_ptr(unsigned rand)	{ return is_leaf() ? this : next_file[rand % next_count]; }
	SnowInfo *		rand_prev_ptr(unsigned rand)	{ return is_root() ? NULL : prev_file[rand % prev_count]; }
	short				bitmap_width() { return *(short *)bitmap_ptr; }
	short				bitmap_height() { return *(((short *)bitmap_ptr)+1); }
	void				draw_at(short absX, short absY);
};


// --------- Define class SnowRes -----//
//
class SnowRes
{
public:
	SnowInfo*	snow_info_array;
	int			snow_info_count;
	SnowInfo**	root_info_array;
	unsigned		root_count;

	int			init_flag;
	ResourceDb	res_bitmap;

public:
	SnowRes();
	~SnowRes();

	void init();
	void deinit();

	int			rand_root(unsigned rand);
#ifdef DEBUG
	SnowInfo*	operator[](int);
#else
	SnowInfo*	operator[](int snowMapId) { return snow_info_array+snowMapId-1; }
#endif

private:
	void load_info();
};

extern SnowRes snow_res;

#endif