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

//Filename    : OFILETXT.CPP
//Description : Object Text file

#include <stdlib.h>

#include <ALL.h>
#include <KEY.h>
#include <OFILETXT.h>


//---- marco function for advancing to next token, bypassing space, ',' and ':' ---//

#define next_token()  for( ; *data_ptr==' ' || *data_ptr==',' || *data_ptr==':' ; data_ptr++ )


//-------- Begin of function FileTxt::FileTxt ----------//
//
// <char*> fileName = name of the file
//
FileTxt::FileTxt(char* fileName)
{
   file_open(fileName);

   //-----------------------------------//

   file_length = File::file_size();

   data_buf = mem_add( file_length+1 );

   data_buf[file_length] = CHAR_EOF;
   data_ptr = data_buf;

   //-----------------------------------//

   file_read( data_buf, file_length );

   file_close();
}
//---------- End of function FileTxt::FileTxt ----------//


//-------- Begin of function FileTxt::FileTxt ----------//
//
// Initialize this FileTxt structure with a file stream.
//
// <File*> filePtr  = pointer to a file class for reading in the text
// <int>	  dataSize = size of the data.
//
FileTxt::FileTxt(File* filePtr, int dataSize)
{
	//-----------------------------------//

	file_length = dataSize;

	data_buf = mem_add( file_length+1 );

	data_buf[file_length] = CHAR_EOF;
	data_ptr = data_buf;

	//-----------------------------------//

	filePtr->file_read( data_buf, file_length );
}
//---------- End of function FileTxt::FileTxt ----------//


//-------- Begin of function FileTxt::~FileTxt ----------//
//
FileTxt::~FileTxt()
{
   mem_del( data_buf );
}
//---------- End of function FileTxt::~FileTxt ----------//


//--------- Begin of function FileTxt::get_char ------//
//
// Get one character from the file stream
//
// [int] advancePointer = whether advance the text pointer after getting the char or not
//                        set this to 0, if you only want to get a char for testing only
//                        (default : 1)
//
// return : <char> the character gotten
//          if EOF, return NULL
//
char FileTxt::get_char(int advancePointer)
{
   while( *data_ptr == CHAR_RETURN )
      next_line();

   if( *data_ptr == CHAR_EOF )
      return 0;

   char dataChar = *data_ptr;

   if( advancePointer )
      data_ptr++;

   return dataChar;
}
//----------- End of function FileTxt::get_char ------//


//--------- Begin of function FileTxt::next_line ------//
//
// Description : move the pointer of the character buffer to the
//               first character (not space) of the next line
//
// Return : <int> 1 - jump to next line
//                0 - end of file
//
char* FileTxt::next_line()
{
   for( ; *data_ptr != CHAR_RETURN && *data_ptr != CHAR_EOF ; data_ptr++ );

   if( *data_ptr == CHAR_RETURN )
      data_ptr++;

   if( *data_ptr == CHAR_LINE_FEED )
      data_ptr++;

   next_token();

   if( *data_ptr == CHAR_EOF )
      return NULL;
   else
      return data_ptr;
}
//----------- End of function FileTxt::next_line ------//



//----------- End of function FileTxt::locate_word ------//
//
// locate the specified word
//
// <char*> wordPtr = pointer to the word
//
// Return : the pointer to next word
//          NULL if not found, now pointed to EOF
//
char* FileTxt::locate_word(char* wordPtr)
{
   for( ; *data_ptr != CHAR_EOF ; data_ptr++ )
   {
      if( *data_ptr == *wordPtr )       // match first character first
      {
         for( ; *data_ptr == *wordPtr && *wordPtr ; data_ptr++, wordPtr++ );

         if( *wordPtr == NULL )
         {
            next_token();
            return data_ptr;
         }
      }
   }

   return NULL;
}
//----------- End of function FileTxt::locate_word ------//


//--------- Begin of function FileTxt::get_token ------//
//
// [int] advancePointer = whether we should advance the pointer
//                        after getting the token
//                        (default : 1)
//                        It it is 0, the pointer will stay there
//
// Return : the pointer to token buffer
//          NULL if not found, now pointed to EOF
//
char* FileTxt::get_token(int advancePointer)
{
   int   i;
   char  pc;
   char* savePtr, *tokenPtr = token_buf;

   if( !advancePointer )
       savePtr = data_ptr;

   next_token();

   while( *data_ptr == CHAR_RETURN )    // bypass all space lines
   {
      if( !next_line() )         // End of File
         return NULL;
   }

   //.........................................//

   for( i=0 ; ; i++,data_ptr++ )
   {
      pc = *data_ptr;

      if ( pc == ' ' || pc == '=' || pc == ',' || pc == ':' || pc==CHAR_RETURN )
         break;

      else if( pc == CHAR_EOF )
      {
         if( !advancePointer )    // don't advance the pointer after getting the token
            data_ptr = savePtr;

         return NULL;
      }

      if ( i<MAX_TOKEN_LEN )
         *tokenPtr++ = *data_ptr;
   }

   //................................//

   *tokenPtr = NULL;

   next_token();

   if( !advancePointer )
      data_ptr = savePtr;

   return token_buf;
}
//----------- End of function FileTxt::get_token ------//


//----------- End of function FileTxt::get_num ------//
//
// Convert current token to number and return the number
//
double FileTxt::get_num()
{
   return atof( get_token() );
}
//----------- End of function FileTxt::get_num ------//


//--------- Begin of function FileTxt::read_line ------//
//
// Read a line from this text file to the given buffer
//
// <char*> textBuf = a pre-allocated text buffer with a len of bufLen+1
// <int>   bufLen  = MAX no. of chars the buffer can store
//
void FileTxt::read_line(char* textBuf, int bufLen)
{
   next_token();        // skip all leading space

   if( *data_ptr == CHAR_RETURN )
      next_line();

   //-----------------------------------------//

   int i;

   for( i=0 ; *data_ptr!=CHAR_RETURN && *data_ptr!=CHAR_EOF && i<bufLen ; i++ )
      *textBuf++ = *data_ptr++;

   *textBuf=NULL;

   next_line();
}
//----------- End of function FileTxt::read_line ------//


//--------- Begin of function FileTxt::read_paragraph ------//
//
// Read a paragraph from the text file to the given buffer
// It will read all coming text until it encounter PAGE BREAK or EOF character
//
// <char*> textPtr = a pre-allocated text buffer with a len of bufLen+1
// <int>   bufLen  = MAX no. of chars the buffer can store
//
// return : <int> textReadLen = the length of text actually read into textPtr
//
int FileTxt::read_paragraph(char* textPtr, int bufLen)
{
   next_token();        // skip all leading space

   if( *data_ptr == CHAR_RETURN )
      next_line();

   //-----------------------------------------//

   char ch;
   int  textReadLen=0;

   while( *data_ptr != CHAR_EOF )
   {
      ch = *data_ptr++;

		if( ch==CHAR_PAGE_BREAK || ch=='~' )
			break;

      // RETURN + LINE_FEED = word wrap
      // LINE_FEED only     = new line

      if( ch==CHAR_RETURN )
      {
         if( *data_ptr==CHAR_LINE_FEED )
         {
            data_ptr++;
            ch = ' ';      // Convert RETURN + LINE_FEED to a space
         }
		}

		// ##### begin Gilbert 24/7 #######//
		if( ch== 0xb ) //KEY_CTRL_K )
		// ##### end Gilbert 24/7 #######//
		{
			ch = '\n';          // CTRL-K means new line (CTRL-J)

			if( *data_ptr == CHAR_RETURN )    // by pass RETURN + LINE_FEED which may be turned into space if not cancelled
				data_ptr++;

			if( *data_ptr == CHAR_LINE_FEED )
				data_ptr++;
		}

		*textPtr++ = ch;
		textReadLen++;

		if( textReadLen>=bufLen-1 )       // when in non-debug mode
			break;
	}

   *textPtr++ = NULL;
   textReadLen++;

   err_when( data_ptr-data_buf > file_length );

   return textReadLen;
}
//----------- End of function FileTxt::read_paragraph ------//
