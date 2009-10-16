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

//Filename    : OMLINK.H
//Description : Header file of object Multiple Linker

#ifndef __OMLINK_H
#define __OMLINK_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.h>
#endif

//--------- Define struct MLinkNode -----------//

struct MLinkNode
{
   int  dest_recno;    // destination recno of the linkage
   int  next_link;     // next linkage of the client
   char link_type;     // client defined linkage type
};

//----------- Define class MLink -----------//

class MLink : public DynArrayB
{
private:
   MLinkNode* cur_node;	 // for temporary storage, allow first_dest(), next_dest()
			 // first_node() & next_node() to have quicker access
public:
   MLink();

   void linkin(int&,int,int=0);
   int  linkout(int&,int);
   void clean_link(int&);
	void update(int,int,int,int=0);
	void zap();

   int  check_exist(int,int);
   int  dest_count(int);

   int  first_dest(int);
   int  next_dest();

   // write_file() and read_file() use DynArrayB::write_file() and read_file() directly

   MLinkNode* first_node(int);
   MLinkNode* next_node();
};

//---------------------------------------------//

#endif
