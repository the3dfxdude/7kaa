//Filename    : ORES.H
//Description : Header file of Object Resource

#ifndef __ORES_H
#define __ORES_H

#ifndef __OFILE_H
#include <OFILE.H>
#endif

//--------- Define class Resource ----------//

class Resource : public File
{
public:
   int      rec_count;          // total no. of records

private:
   enum     { DEF_BUF_SIZE = 5120 };   // default buffer size : 5K

   long     *index_buf;         // index buffer pointer
   char     *data_buf;          // data buffer pointer
   unsigned data_buf_size;      // size of the data buffer

   char     init_flag;
   char     read_all;           // read all data from resource file to memory
   char     use_common_buf;        // use vga's buffer as data buffer or not
   int      cur_rec_no;         // current record no

public:
   Resource()	  { init_flag=0; }
   ~Resource()	  { deinit(); }

   Resource(char* resFile, int readAll, int useCommonBuf=0)
	 { init_flag=0; init(resFile, readAll, useCommonBuf); }

   void  init(char* resFile, int readAll, int useCommonBuf=0);
   void  deinit();

	int   is_inited() 	{ return init_flag; }

   char* read(int= -1);
   File* get_file(int, int&);
};

//-------------------------------------------//

#endif
