// Filename    : OSLIDCUS.H
// Description : Custom slide bar 


#ifndef __OSLIDCUS_H
#define __OSLIDCUS_H


// -------- define type SlideBarFP ---------//

class SlideBar;
class SlideVBar;

typedef void (*SlideBarFP)(SlideBar *, int repaintBody);
typedef void (*SlideVBarFP)(SlideVBar *, int repaintBody);

// --------- define clas SlideBar

class SlideBar
{
public:

	// screen representation
	short scrn_x1;
	short scrn_y1;
	short scrn_x2;
	short scrn_y2;
	short scrn_bar_width;
	short scroll_type;			// 0 = slide, 1 = scroll bar

	// assume the recno of the first is min_recno and
	// last record is max_recno
	// and you can view (view_size) records at a time
	// and first record on the viewing area is view_recno
	int	min_recno;
	int	max_recno;
	int	view_size;
	int	view_recno;

	char	drag_flag;
	// valid if drag_flag is true
	short	drag_cur_x;		// mouse.cur_x when start dragging the mouse
	short drag_rect_left;		// rect_left() when start dragging
	int	drag_last_view_recno;

	SlideBarFP	disp_func;

public:
	void	init_slide(short x1, short y1, short x2, short y2, short barWidth, SlideBarFP dispFunc);
	void	init_scroll(short x1, short y1, short x2, short y2, int viewSize, SlideBarFP dispFunc);

	void	set(int minRecno, int maxRecno, int viewRecno);
	int	set_view_recno(int viewRecno);
	void	set_min_recno(int minRecno);
	void	set_max_recno(int maxRecno);
	int	is_empty();

	int	detect();
	void	paint();
	void	paint(int newViewRecno);

public:
	// function for disp_func
	short	rect_left();
	short	rect_right();

	int	calc_view_recno(short scrnX);
	int	max_view_recno();
};



class SlideVBar
{
public:

	// screen representation
	short scrn_x1;
	short scrn_y1;
	short scrn_x2;
	short scrn_y2;
	short scrn_bar_height;
	short scroll_type;			// 0 = slide, 1 = scroll bar

	// assume the recno of the first is min_recno and
	// last record is max_recno
	// and you can view (view_size) records at a time
	// and first record on the viewing area is view_recno
	int	min_recno;
	int	max_recno;
	int	view_size;
	int	view_recno;

	char	drag_flag;
	// valid if drag_flag is true
	short	drag_cur_y;		// mouse.cur_y when start dragging the mouse
	short drag_rect_top;		// rect_top() when start dragging
	int	drag_last_view_recno;

	SlideVBarFP	disp_func;

public:
	void	init_slide(short x1, short y1, short x2, short y2, short barHeight, SlideVBarFP dispFunc);
	void	init_scroll(short x1, short y1, short x2, short y2, int viewSize, SlideVBarFP dispFunc);

	void	set(int minRecno, int maxRecno, int viewRecno);
	int	set_view_recno(int viewRecno);
	void	set_min_recno(int minRecno);
	void	set_max_recno(int maxRecno);
	int	is_empty();

	int	detect();
	void	paint();
	void	paint(int newViewRecno);

public:
	// function for disp_func
	short	rect_top();
	short	rect_bottom();

	int	calc_view_recno(short scrnY);
	int	max_view_recno();
};


#endif
