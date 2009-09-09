//Filename    : ORAWRES.H
//Description : Header file of object RawRes

#ifndef __ORAWRES_H
#define __ORAWRES_H

#ifndef __ORES_H
#include <ORES.H>
#endif

#ifndef __ODYNARR_H
#include <ODYNARR.H>
#endif

//------------- define constant --------------//

#define MAX_RAW			 		    3
#define MAX_PRODUCT					 3
#define MAX_RAW_RESERVE_QTY	20000

//------------ define icon size -------------//

enum { RAW_SMALL_ICON_WIDTH=10,
		 RAW_SMALL_ICON_HEIGHT=10,
		 RAW_LARGE_ICON_WIDTH=32,
		 RAW_LARGE_ICON_HEIGHT=32
	  };

//----------- Define raw material types ---------//

enum { RAW_CLAY=1,
		 RAW_COPPER,
		 RAW_IRON,		 };

//------------ Define struct RawRec ---------------//

struct RawRec
{
	enum { NAME_LEN=12, TERA_TYPE_LEN=1 };

	char name[NAME_LEN];
	char tera_type[TERA_TYPE_LEN];
};

//------------- Define struct RawInfo --------------//

struct RawInfo
{
public:
	enum { NAME_LEN=20 };

	char	raw_id;
	char 	name[NAME_LEN+1];
	char  tera_type;

	DynArray raw_supply_firm_array;
	DynArray product_supply_firm_array;

public:
	RawInfo();

	void 		add_raw_supply_firm(short firmRecno);
	void 		add_product_supply_firm(short firmRecno);

	short		get_raw_supply_firm(short recNo) 		{ return *( (short*) raw_supply_firm_array.get(recNo) ); }
	short		get_product_supply_firm(short recNo) 	{ return *( (short*) product_supply_firm_array.get(recNo) ); }
};

//----------- Define class RawRes ---------------//

class RawRes
{
public:
	short    	raw_count;
	RawInfo* 	raw_info_array;

	char	   	init_flag;

	Resource    res_icon;

public:
	RawRes();

	void 		init();
	void 		deinit();

	void		next_day();
	void		update_supply_firm();

	void  	put_small_product_icon(int x, int y, int rawId);
	void		put_small_raw_icon(int x, int y, int rawId);

	char* 	large_product_icon(int rawId)	{ return res_icon.read(rawId); }
	char* 	small_product_icon(int rawId) { return res_icon.read(MAX_RAW+rawId); }
	char* 	large_raw_icon(int rawId)		{ return res_icon.read(MAX_RAW*2+rawId); }
	char* 	small_raw_icon(int rawId)		{ return res_icon.read(MAX_RAW*3+rawId); }

	int  		write_file(File*);
	int  		read_file(File*);

	RawInfo* operator[](int rawId);      // pass rawId  as recno

private:
	void 		load_all_info();
};

extern RawRes raw_res;

//----------------------------------------------------//

#endif
