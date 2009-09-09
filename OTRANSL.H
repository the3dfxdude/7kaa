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

   char* process(char*);

	static void multi_to_win(char *c, int len);
};

extern Translate translate;

//--------------------------------------------------//

#endif
