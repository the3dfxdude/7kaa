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

//Filename    : ORESTXT.H
//Description : Header file of Object Text ResTxt

#ifndef __ORESTXT_H
#define __ORESTXT_H

#ifndef __OFILE_H
#include <OFILE.h>
#endif

//------ Begin of function TxtIndex ------//

struct TxtIndex
{
   enum { TITLE_LEN=40, PICT_NAME_LEN=8 };

   char  title[TITLE_LEN+1];       // Title of the text article
   char  pict_name[PICT_NAME_LEN+1];
   short text_size;		   // size of the text body
   long  file_pos;
};


//--------- Define class ResTxt ----------//

class ResTxt
{
public:
   int	      rec_count;

private:
   enum     { DEF_TXT_BUF_SIZE  = 5120 };  // default buffer size : 5K
					   // this size should be large enough for average text unit, also we don't need realloc very frequently
   char       init_flag;
   char	     res_file_name[MAX_PATH+1];	// e.g. text file name, e.g. BOOK.RTX
							// we need to keep the file name, because the file is opened only when reading the content, and close when not necessary
   TxtIndex*  txt_index_array;
   char*      txt_buf;
   int        txt_buf_size;

   File	      file_txt;
   char	      always_open_flag;		// always open the text resource file or not 

public:
   ResTxt()	  { init_flag=0; }
   ~ResTxt()	  { deinit(); }

   void  init(char*,int=1);
   void  deinit();

   char* get_title(int);
   char* get_body(int);
   char* get_pict_name(int);

   int   locate_topic(char*);
};

//-------------------------------------------//

#endif
