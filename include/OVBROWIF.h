//Filename    : OVBROWIF.H
//Description : Object List Box

#ifndef __OVBROWIF_H
#define __OVBROWIF_H

#ifndef __OVBROWSE_H
#include <OVBROWSE.h>
#endif

//----------- Define variable type -----------//

typedef void (*BrowDispFP)(int recNo,int x,int y,int refreshFlag); // user defined function to be called

//---------- Define class ListBox ------------//

class VBrowseIF : public VBrowse
{
public:
	char	vga_front_only;

public:
	VBrowseIF();

	void  set_vga_front_only()		{ vga_front_only=1; }
	void	init_var(int totalRec, int recNo);
	void  refresh(int= -1,int= -1);
	void  paint();

protected:
	void	disp_all();
	void	disp_one(int recNo, int dispType);
	void	disp_rec(int,int,int,int);
};

//--------------------------------------------------//

#endif

