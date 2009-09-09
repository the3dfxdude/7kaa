//Filename    : OSITE.H
//Description : Object Site 

#ifndef __OSITE_H
#define __OSITE_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.H>
#endif

#ifndef __ORAWRES_H
#include <ORAWRES.H>
#endif

//-------- Define Site Type --------//

enum { MAX_SITE_TYPE=3 };

enum { SITE_RAW=1,
		 SITE_SCROLL,
		 SITE_GOLD_COIN };

//--------- Define class Site ----------//

class Site
{
public:
	short site_recno;

	char  site_type;		// SITE_RAW, SITE_ARTIFACT or SITE_SCROLL

	short object_id;		// id. of the object,
	int   reserve_qty;	// for raw material only
	char  has_mine;		// whether there is a mine on this site

	short map_x_loc;
	short map_y_loc;

	BYTE	region_id;

public:
	void 	init(int siteRecno, int siteType, int xLoc, int yLoc);
	void 	deinit();

	void 	disp_info(int refreshFlag);
	void 	detect_info();

	void  draw(int x, int y);
	void	draw_selected();

	int   get_site_object(int unitRecno);
	int  	ai_get_site_object();
};

//--------- Define class SiteArray ----------//

class SiteArray : public DynArrayB
{
public:
	short	 selected_recno;			// the firm current being selected
	short	 untapped_raw_count;		// no. of unoccupied raw site available
	short	 scroll_count;
	short	 gold_coin_count;
	short	 std_raw_site_count;		// standard no. of raw site in one game, based on this number, new mines pop up when existing mines run out of deposit

public:
	SiteArray();
	~SiteArray();

	void	init();
	void 	deinit();

	int 	add_site(int xLoc, int yLoc, int siteType, int objectId, int reserveQty=0);
	void 	del_site(int siteRecno);

	void	generate_raw_site(int stdRawSiteCount=0);
	int  	create_raw_site(int regionId, int townRecno=0);

	int 	scan_site(int xLoc, int yLoc, int siteType=0);
	void 	go_to_a_raw_site();
	void 	ai_get_site_object();

   void	next_day();
	void	draw_dot();

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

	//--------------------------------------//

	int  is_deleted(int recNo);

	#ifdef DEBUG
		Site* operator()();            // reference to current Site record
		Site* operator[](int recNo);
	#else
		Site* operator()()  	   	 { return (Site*) get(); }
		Site* operator[](int recNo)  { return (Site*) get(recNo); }
	#endif
};

extern SiteArray site_array;

//--------------------------------------------//

#endif