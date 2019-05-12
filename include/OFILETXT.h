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

//Filename    : OFILETXT.H
//Description : Header file for text file stream

#ifndef __FILETXT_H
#define __FILETXT_H

#ifndef __OFILE_H
#include <OFILE.h>
#endif

//--------- Define control characters ------------//

#define CHAR_EOF        '\x1A'     // End of file character
#define CHAR_PAGE_BREAK '~' 	     // Page Break
#define CHAR_RETURN     '\r'       // Carriage return
#define CHAR_LINE_FEED  '\n'       // Line feed

//--------------------------------------//

class FileTxt : public File
{
public:
   char *data_ptr;

private:
   enum { MAX_TOKEN_LEN=30 };

   char *data_buf;
   char token_buf[MAX_TOKEN_LEN];

   long file_length;

public:
	FileTxt(char* fileName);
	FileTxt(File* filePtr, int dataSize);
	~FileTxt();

   char*  next_line();
   char*  locate_word(char*);

   char   get_char(int=1);
   char*  get_token(int=1);
   double get_num();
   void   read_line(char*,int);
   int    read_paragraph(char*,int);

   long   file_size()   { return file_length; }
   int    is_eof()      { return *data_ptr == CHAR_EOF; }

   // Match and advance data_ptr functions
   size_t match_chars(const char *cc);
   size_t match_chars_ex(const char *cc);
   size_t match_str(const char *str);
};

// Match an unordered class of characters of cc beginning at data_ptr. Matches
// of the same characters may happen multiple times. Advance data_ptr if there
// is a match. Return the character match count.
inline size_t FileTxt::match_chars(const char *cc)
{
   size_t c = strspn(data_ptr, cc);
   data_ptr += c;
   return c;
}

// Match an unordered class of characters of exlucsive of the list in cc
// beginning at data_ptr. Matches of the same characters may happen multiple
// times. Advance data_ptr if there is a match. Return the match count.
inline size_t FileTxt::match_chars_ex(const char *cc)
{
   size_t c = strcspn(data_ptr, cc);
   data_ptr += c;
   return c;
}

// Match character sequence of str, stopping before null, beginning at
// data_ptr. Advance data_ptr if there is an exact match. Return the match
// count.
inline size_t FileTxt::match_str(const char *str)
{
   size_t c = strlen(str);
   if( !strncmp(data_ptr, str, c) )
   {
      data_ptr += c;
      return c;
   }
   return 0;
}

#endif

