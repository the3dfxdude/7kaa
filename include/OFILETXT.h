//Filename    : OFILETXT.H
//Description : Header file for text file stream

#ifndef __FILETXT_H
#define __FILETXT_H

#ifndef __OFILE_H
#include <OFILE.H>
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
};

#endif


