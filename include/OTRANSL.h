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

//Filename    : OTRANSL.H
//Description : Multi-lingual Translation Class Header

#ifndef __OTRANSL_H
#define __OTRANSL_H


//--------- Define struct TranslateRec -------//

struct TranslateRec
{
   char* from_text_ptr;
   char* to_text_ptr;
};

//---------- Define class Translate -----------//

class Translate
{
private:
   char	  	 init_flag;
   int		 rec_count;	// no. of translation records.

   char* 	 translate_text_buf;
   TranslateRec* translate_table;

   short 	 quick_seek_table[256];	  // ascii from 0 to 255, total 256 records, storing record numbers pointing to translate_table

public:
   Translate();
   ~Translate() 	{ deinit(); }

   void init();
   void deinit();

   const char* process(const char*);

	static void multi_to_win(char *c, int len);
};

extern Translate translate;

//--------------------------------------------------//

#endif
