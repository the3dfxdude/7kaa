/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
	int   mouse_over(int *x1=NULL, int *y1=NULL, int *x2=NULL, int *y2=NULL);

protected:
	void	disp_all();
	void	disp_one(int recNo, int dispType);
	void	disp_rec(int,int,int,int);
};

//--------------------------------------------------//

#endif

