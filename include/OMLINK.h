//Filename    : OMLINK.H
//Description : Header file of object Multiple Linker

#ifndef __OMLINK_H
#define __OMLINK_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.H>
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
