// Filename    : OFIRMDIE.H
// Description : destructing firm


#ifndef __OFIRMDIE_H
#define __OFIRMDIE_H

#include <ODYNARRB.h>
#include <ORESDB.h>

struct FirmBuild;
struct FirmBitmap;
struct FirmDieBitmap;

// -------- define class FirmDieRes --------//

class FirmDieRes
{
public:
	short			firm_build_count;
	short			firm_bitmap_count;
	FirmBuild *firm_build_array;
	FirmDieBitmap *firm_bitmap_array;

	char	   	init_flag;
	ResourceDb	res_bitmap;

public:
	FirmDieRes();
	~FirmDieRes();
	void	init();
	void	deinit();

	FirmBuild*	get_build(int buildId);
	FirmDieBitmap* get_bitmap(int bitmapId);

private:
	void	load_build_info();
	void	load_bitmap_info();
};

// -------- define class FirmDie --------//

class FirmDie
{
public:
	short	firm_id;
	short	firm_build_id;
	short	nation_recno;
	short	frame;
	short	frame_delay_count;
	short	loc_x1, loc_y1, loc_x2, loc_y2;

public:
	void	init(short firmId, short firmBuildId, short nationRecno, 
		short	locX1, short locY1, short locX2, short locY2);
	void	init(Firm *firmPtr);	
	void	pre_process();
	int	process();
	void	draw(int displayLayer);
};

// -------- define class FirmDieArray --------//

class FirmDieArray : public DynArrayB
{
public:
	FirmDieArray();
	~FirmDieArray();
	void	init();
	void	deinit();

	int	add(FirmDie *r);
	void	del(int i);
	int	is_deleted(int recno);
	void	process();

	FirmDie *operator[](int recNo);
};

extern FirmDieRes firm_die_res;
extern FirmDieArray firm_die_array;

#endif