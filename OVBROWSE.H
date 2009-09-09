//Filename    : OVBROWSE.H
//Description : Object List Box

#ifndef __OVBROWSE_H
#define __OVBROWSE_H

#ifndef __OSCROLL_H
#include <OSCROLL.H>
#endif

//------------ Define macro constant --------------//

#define MINI_INDICATOR_HEIGHT  8  // Minimum height of scroll bar indicator
#define SCROLL_BAR_WIDTH      15

#define VBROWSE_COLOR_BACK      Vga::active_buf->color_down
#define VBROWSE_COLOR_FONT      V_BLACK
#define VBROWSE_COLOR_HIGH      VGA_YELLOW

//----------- Define variable type -----------//

typedef void (*BrowDispFP)(int recNo,int x,int y,int refreshFlag); // user defined function to be called

//---------- Define class ListBox ------------//

class VBrowse
{
public:
   short rec_no;            // current record no.

	char  double_click;	    // double clicked on record
	char	detect_pull_flag;		 // whether detect pulling records or not

	short rec_height;        // record height (font.height()+line space)
	short rec_width;

	short top_rec_no;        // the record no. of the record on the top of the list box
	short disp_max_rec;

public:
	enum  {DISP_REC=1,DISP_HIGH=2,CLEAR_HIGH=3};
	enum  {MIN_INTER_SPACE=3};	// minimum inter-record space

	char  init_flag;
	char  state;             // state of the list box, 0=closed, 1=open
	char  disp_frame;	    	 // whether display item highlight frame or not

	short x1,y1,x2,y2;       // coordinations of the list box
	short ix1,iy1,ix2,iy2;   // content area coordination

	short rec_x_space;       // space between records
	short rec_y_space;

	char  none_record;	    // true if no record available
	short total_rec_num;     // total no. of records

	short x_max_rec;	    	 // maximum records can be displayed on screen
	short y_max_rec;	    	 // in a row, a column and a box

	BrowDispFP disp_rec_func;    // user defined function to be called
	ScrollBar  scroll_bar;

	//--------- Define static member vars ---------//

	static char 	 press_record;     	 // for detecting pulling records upwards and downloads
	static VBrowse* press_vbrowse_ptr;

public :
	VBrowse()	  { init_flag=0; }

	virtual void  init(int,int,int,int,int,int,int,BrowDispFP,int=1,int=MIN_INTER_SPACE);

	virtual void  open(int recNo=1, int newTotalRec= -1);
	virtual void  close();

	virtual int   recno()	    	 { return rec_no; }
	virtual int   total_rec()	    { return none_record ? 0 : total_rec_num; }

	virtual int   detect();
	virtual int   detect_right();
	virtual void  refresh(int= -1,int= -1);
	virtual void  update();
	virtual void  paint();

protected:
	virtual void  init_var(int,int);

	virtual void  disp_all();
	virtual void  disp_one(int,int);
	virtual void	disp_rec(int,int,int,int);
	virtual int   detect_pull();
};

//--------------------------------------------------//

#endif

