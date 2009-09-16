//Filename    : OTRANSL.CPP
//Description : Multi-lingual Translation Class

#include <string.h>
#include <stdlib.h>

#include <ALL.h>
#include <OGAME.h>
#include <OSYS.h>
#include <OFILETXT.h>
#include <OTRANSL.h>


//--------- Define constant ---------//

#define TRANSLATE_FILE_NAME   DIR_RES"TRANSLAT.RES"

//------- Format of the translation resource file --------//
//
// <Original Text>|<Translated Text>~
//
// | - separator between the original and the translated text
// ~ - separator between text blocks
//
//--------------------------------------------------------//

//----------- Define static functions --------------//

static int sort_translate_table( const void *a, const void *b );

//--------- Begin of function Translate:Translate --------//

Translate::Translate()
{
   memset( this, 0, sizeof(this) );
}

//---------- End of function Translate::Translate --------//


//--------- Begin of function Translate:init --------//

void Translate::init()
{
   //---- read the whole file into the buf --------//

   if( !m.is_file_exist(TRANSLATE_FILE_NAME) )
      return;

	File fileTranslate;

	fileTranslate.file_open(TRANSLATE_FILE_NAME);

	int textBufSize=fileTranslate.file_size();

	translate_text_buf = mem_add(textBufSize);

	fileTranslate.file_read(translate_text_buf, textBufSize);

	fileTranslate.file_close();

   //----- count the no. of records ----------//

   char* textPtr=translate_text_buf;
   int   i;

   for( i=0, rec_count=0 ; i<textBufSize ; i++, textPtr++ )
   {
      if( *textPtr == '|' )
			rec_count++;
   }

   //------ allocate the translation table ------//

   translate_table = (TranslateRec*) mem_add( sizeof(TranslateRec) * rec_count );

   //---- generate the translation table from the translation text ----//

   int recNo=1;

   textPtr=translate_text_buf;

   translate_table[recNo-1].from_text_ptr = textPtr;

	// ###### patch begin Gilbert 2/2 #####//
//   for( i=0 ; i<textBufSize ; i++, textPtr++ )
   for( i=0 ; textPtr-translate_text_buf<textBufSize ; i++, textPtr++ )
	// ###### end begin Gilbert 2/2 #####//
   {
      if( *textPtr == '|' )
      {
			*textPtr = NULL;	// Nullify the sentence
			translate_table[recNo-1].to_text_ptr = textPtr+1;
      }

      if( *textPtr == '~' )
      {
			*textPtr = NULL;	// Nullify the sentence

			recNo++;

			if( recNo>rec_count )	// end of the file
				break;

			//------- skip to the next line ----------//

			for( ; *textPtr != CHAR_RETURN && *textPtr != CHAR_EOF ; textPtr++ );

			if( *textPtr == CHAR_RETURN )
				textPtr++;

			if( *textPtr == CHAR_LINE_FEED )
				textPtr++;

			translate_table[recNo-1].from_text_ptr = textPtr;
      }
   }

   //-------- sort the translation table ----------//

   qsort( translate_table, rec_count, sizeof(TranslateRec), sort_translate_table );

   //-------- create the quick_seek_table ----------//

   memset( quick_seek_table, 0, sizeof(quick_seek_table) );

   int lastSeekChar=0;

   for( i=0 ; i<rec_count ; i++ )
   {
      if( translate_table[i].from_text_ptr[0] > lastSeekChar )
      {
			lastSeekChar = translate_table[i].from_text_ptr[0];
			quick_seek_table[lastSeekChar] = i+1;
      }
   }

   //----------- set init_flag ------------//

   init_flag=1;
}

//---------- End of function Translate::init --------//


//--------- Begin of function Translate:deinit --------//

void Translate::deinit()
{
   if( translate_text_buf )
   {
      mem_del( translate_text_buf );
      translate_text_buf = NULL;
   }

   if( translate_table )
   {
      mem_del( translate_table );
      translate_table = NULL;
   }
}

//---------- End of function Translate::deinit --------//


//--------- Begin of function Translate:process --------//
//
// Translate the given text into the foreign language.
//
// <char*> originalText - the original text to be translated
//
// return : <char*> the translated text.
//		    if not found, return the original text.
//
char* Translate::process(char* originalText)
{
   if( !init_flag )
      return originalText;

   //------ look up the quick_seek_table --------//

   int tableRecno = quick_seek_table[ *((unsigned char*)originalText) ];

   if( !tableRecno )      	// not in the translation table
      return originalText;

   int i;

   for( i=tableRecno ; i<=rec_count ; i++ )
   {
      if( strcmp(translate_table[i-1].from_text_ptr, originalText)==0 )
			return translate_table[i-1].to_text_ptr;
		else
		{
			if( translate_table[i-1].from_text_ptr[0] != originalText[0] )		// all original text begins with this letter has been scanned and none has been found 
				break;
		}
	}

   return originalText;
}
//---------- End of function Translate::process --------//


//------ Begin of function sort_translate_table ------//
//
static int sort_translate_table( const void *a, const void *b )
{
   return strcmp( ((TranslateRec*)a)->from_text_ptr, ((TranslateRec*)b)->from_text_ptr );
}
//------- End of function sort_translate_table ------//


//------ Begin of function Translate::multi_to_win ----------//
void Translate::multi_to_win(char *c, int len)
{
	// look up table to convert multilingual char set to windows char set
	static unsigned char multi_to_win_table[] = 
		"ÇüéâäàåçêëèïîìÄÅ"
		"ÉæÆôöòûùÿÖÜø£Ø×\x83"
		"áíóúñÑªº¿\xae¬½¼¡«»"
		"?????ÁÂÀ©????¢¥?"
		"??????ãÃ???????¤"
		"ðÐÊËÈ'ÍÎÏ????¦Ì?"
		"ÓßÔÒõÕµÞþÚÛÙýÝ?´"
		"·±=¾¶§÷?°¨?¹³²??";


	unsigned char *textPtr = (unsigned char *)c;
	for( ; len > 0 && *textPtr; --len, ++textPtr )
	{
		if( *textPtr >= 0x80 && multi_to_win_table[*textPtr - 0x80] != '?' )
			*textPtr = multi_to_win_table[*textPtr - 0x80];
	}
}
//------ End of function Translate::multi_to_win ----------//
