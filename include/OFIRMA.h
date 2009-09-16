//Filename    : OFIRMA.H
//Description : Object Firm Array

#ifndef __OFIRMA_H
#define __OFIRMA_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.H>
#endif

//--------- Define class FirmArray ----------//

class Firm;

class FirmArray : public DynArrayB
{
public:
	int	 selected_recno;		// the firm current being selected

private:
	int    process_recno;

public:
   FirmArray();
   ~FirmArray();

	void  init();
   void  deinit();

	int 	build_firm(int xLoc, int yLoc, int nationRecno, int firmId, char* buildCode=NULL, short builderRecno=0);
	int   create_firm(int);
   void  del_firm(int);
   int   firm_class_size(int);

	int   process();
	void  next_day();
	void  next_month();
	void  next_year();

	void	draw_dot();
	//### begin alex 12/9 ###//
	void	draw_profile();
	//#### end alex 12/9 ####//
	void  skip(int);

	int   write_file(File*);
	int   read_file(File*);

	#ifdef DEBUG
		Firm* operator()();             // reference to current Firm record
		Firm* operator[](int recNo);
	#else
		Firm* operator()()	    		{ return (Firm*) get_ptr(); }
		Firm* operator[](int recNo)   { return (Firm*) get_ptr(recNo); }
	#endif

	int   is_deleted(int recNo)    // whether the item is deleted or not
			{ return get_ptr(recNo) == NULL; }

};

//---------------------------------------------//

class  MLink;

extern MLink     firm_mlink;
extern FirmArray firm_array;

//---------------------------------------------//

#endif
