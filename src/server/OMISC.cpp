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

//Filename    : OMISC.CPP
//Description : Object of Misc useful functions

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <ALL.h>
#include <OSTR.h>
#include <OMISC.h>

#define	MOVE_AROUND_TABLE_SIZE	900

static char		move_around_table_x[MOVE_AROUND_TABLE_SIZE] = {0};
static char		move_around_table_y[MOVE_AROUND_TABLE_SIZE] = {0};
static short	move_around_table_size = 0;

#if(defined(SPANISH))
	#define THOUSAND_SEPARATOR '.'
	#define DECIMAL_SEPARATOR ','
#else
	#define THOUSAND_SEPARATOR ','
	#define DECIMAL_SEPARATOR '.'
#endif
//-------- Start of function Misc::delay -------------//
//
// Misc::delay for specified seconds
//
// float wait = the no. of second to wait
//
void Misc::delay(float wait)
{
   clock_t stopTime;

   stopTime = (long) (clock() + (wait * CLOCKS_PER_SEC));

   while( clock() < stopTime );
}
//--------- End of function Misc::delay ---------------//


//-------- BEGIN OF FUNCTION Misc::str_shorten --------//
//
// Shorten the string with words exceed the dest. length cut.
//
// <char *> = destination string
// <char *> = source string      (source should be longer than dest)
// <int>    = MAX. no. of characters in the dest. string.
//            ( destStr should be allocated as destStr[destLen+1] )
//
void Misc::str_shorten(char* destStr, char* srcStr, int destLen)
{
   strncpy( destStr, srcStr, destLen );

   destStr[destLen]=NULL;

   //------ no need to cut characters if it fit preciously ----//
   //
   // e.g. "One Two Three" ---> "One Two"
   //      (srcStr)             (destStr, return it as the result)
   //----------------------------------------------------------//

   if( (int)strlen(srcStr) < destLen || srcStr[destLen] == ' ' )
      return;

   //--- if there is only one word in the string, don't cut it ----//
   //
   // e.g. "VeryLongWord" --> "VeryLongWo"
   //
   //--------------------------------------------------------------//

   if( !str_chr( destStr, ' ' ) )
      return;

   //------ if there is more than one word, cut it ------//
   //
   // e.g. "One Two Three" ----> "One Two Thr" ---> "One Two"
   //      (srcStr)               (destStr before)   (destStr after)
   //
   //----------------------------------------------------//

   int i;

   for( i=destLen-1 ; i>0 && destStr[i] != ' ' ; i-- )
      destStr[i] = NULL;

   destStr[i] = NULL;	// trim the space also
}
//------------ END OF FUNCTIN Misc::str_shorten -----------------//


//-------- BEGIN OF FUNCTION Misc::str_cut --------//
//
// Syntax : Misc::str_cut( <char*>,<char*>,<int>,<int> )
//
// Description : cut up string1 and place the resuit in string 2
//
// <char *> = destination string
// <char *> = source string
// <int>    = the start position of the new string
// <int>    = the no. of characters to copied.        (default:to the end of the string)
//

int Misc::str_cut(char* dstr, char* sstr, int schar, int charnum )
{
   int si,di,forever;

   forever = (charnum < 0);

   for (si=schar-1,di=0 ; ( di<charnum || forever ) && sstr[si] ; si++,di++)
      dstr[di] = sstr[si] ;

   dstr[di] = NULL;      // terminating NULL sign

   return 1;
}
//------------ END OF FUNCTIN Misc::str_cut -----------------//


//---------- BEGIN OF FUNCTION Misc::str_chr ----------------//
//
// Description : return the position of the first occurance of the character
//               in the string
//
// Syntax      : Misc::str_chr( <char*>, <char), [int], [int] )
//
// <char*> = the string to the scanned.
// <char>  = the key character
// [int]   = the start position of the string to be scanned (1)
// [int]   = the end position of the string to be scanned   (-1)
//
// return : a positive number indicating the position of the first occurance
//          FAIL / NULL if not found
//

int Misc::str_chr( char* str, char chr, int spos, int epos )
{
   int i;

   epos--;

   for (i=spos-1 ; str[i] && ( i<=epos || epos==-2 ) ; i++)
   {
      if ( str[i] == chr )
         return(i+1);
   }

   return 0;
}

//----------- END OF FUNCTION Misc::str_chr -------------//


//---------- BEGIN OF FUNCTION Misc::str_str ----------------//
//
// Description : return the position of the first occurance of the character
//               in the string
//
// Syntax      : Misc::str_str( <char*>, <char*>, [int], [int] )
//
// <char*> = the string to the scanned.
// <char*> = the key string
// [int]   = the start position of the string to be scanned (1)
// [int]   = the end position of the string to be scanned   (-1)
//
// return : a positive number indicating the position of the first occurance
//          FAIL / NULL if not found
//

int Misc::str_str( char* str, char* fstr, int spos, int epos )
{
   int i,j,flen;

   if (epos == -1)
      epos = strlen(str);

   flen = strlen(fstr);
   epos -= flen ;

   for (i=spos-1 ; str[i] && i<=epos ; i++)
   {
      for (j= 0; j < flen && str[i+j] ; j++ )
      {
         if ( str[i+j] != fstr[j] )     // exactly equal
            break;
      }
      if ( j==flen )     // all equal
         return (i+1) ;
   }

   return 0;
}

//----------- END OF FUNCTION Misc::str_str -------------//


//------ BEGIN OF FUNCTION Misc::upper ---------//
//
// Description : Convert the character to upper case
//
// Syntax      : Misc::upper(<char>)
//
// return : the converted character
//
//
int Misc::upper(int inchar)
{
	if ( inchar >= 'a' && inchar <= 'z' )
		inchar -= 32;

	return( inchar );
}

//----- END OF FUNCTION Misc::upper ------------//


//------ BEGIN OF FUNCTION Misc::lower ---------//
//
// Description : Convert the character to lower case
//
// Syntax      : Misc::lower(<char>)
//
// return : the converted character
//
//
int Misc::lower(int inchar)
{
   if ( inchar >= 'A' && inchar <= 'Z' )
      inchar += 32;

   return( inchar );
}

//----- END OF FUNCTION Misc::lower ------------//



//--------- BEGIN OF FUNCTION Misc::ltrim_len ---------//
//
// Description : return the number of character in the string
//               with left space cut
//
int Misc::ltrim_len(char* inStr,int spos,int len)
{
   int i,j;

   if ( len == -1 )
      len = strlen(inStr);

   for( i = spos-1,j=0 ; j<len && inStr[i]==' ' ; i++ )
      j++;

   return len-j;
}
//--------- END OF FUNCTION Misc::ltrim_len -------------//


//--------- BEGIN OF FUNCTION Misc::rtrim_len ---------//
//
// Description : return the number of character in the string
//               with right space cut
//
// <char*> inStr = the pointer to the string
// [int]   spos  = start position of the string from the pointer
//                 (default : 1)
// [int]   len   = length of the string
//                 (default : until NULL)
//
int Misc::rtrim_len(char* inStr,int spos,int len)
{
   int i,j;

   if ( len == -1 )
      len = strlen(inStr);

   for( i = spos+len-2,j=0 ; j<len && inStr[i]==' ' ; i-- )
      j++;

   return len-j;
}
//--------- END OF FUNCTION Misc::rtrim_len -------------//


//------ BEGIN OF FUNCTION Misc::rtrim ---------//
//
// <char*> = the destination
// <char*> = the source
//
// Note : the destination memory must be allocated
//
void Misc::rtrim( char* des, char* src )
{
   int i;

   for ( i=strlen(src)-1 ; src[i]==' ' && i>=0 ; i-- )
      des[i] = src[i];

   des[i+1] = NULL;
}
//------- END OF FUNCTION Misc::rtrim --------//


//------ BEGIN OF FUNCTION Misc::ltrim ---------//
//
// <char*> = the destination
// <char*> = the source
//
// Note : the destination memory must be allocated
//
void Misc::ltrim( char* des, char* src )
{
   int i,j;

   for ( i=0 ; src[i]==' ' && src[i] ; )
      i++ ;

   for ( j=0 ; src[i] ; i++,j++ )
      des[j] = src[i];

   des[j] = NULL;
}
//------- END OF FUNCTION Misc::ltrim --------//


//------ BEGIN OF FUNCTION Misc::alltrim ---------//
//
// <char*> = the destination
// <char*> = the source
//
// Note : the destination memory must be allocated
//
void Misc::alltrim( char* des, char* src)
{
   int i,j;

   for ( i=0 ; src[i]==' ' && src[i] ; )
      i++ ;

   for ( j=0 ; src[i] && src[i]!=' ' ; i++,j++ )
      des[j] = src[i];

   des[j] = NULL;
}
//------- END OF FUNCTION Misc::alltrim --------//


//------ BEGIN OF FUNCTION Misc::rtrim ---------//
//
// <char*> = the string, the result is put back into the original string pointer
//
char* Misc::rtrim( char* str )
{
   int i;

   for ( i=strlen(str)-1 ; str[i]==' ' && i>=0 ; i-- );

   str[i+1] = NULL;

   return str;
}
//------- END OF FUNCTION Misc::rtrim --------//


//------ BEGIN OF FUNCTION Misc::ltrim ---------//
//
// <char*> = the string
//
char* Misc::ltrim( char* str )
{
   int i,j;

   for ( i=0 ; str[i]==' ' && str[i] ; i++ );

   for ( j=0 ; str[i] ; i++,j++ )
      str[j] = str[i];

   str[j] = NULL;

   return str;
}
//------- END OF FUNCTION Misc::ltrim --------//


//------ BEGIN OF FUNCTION Misc::alltrim ---------//
//
// <char*> = the string
//
char* Misc::alltrim( char* str )
{
   int i,j;

   for ( i=0 ; str[i]==' ' && str[i] ; i++ );

   for ( j=0 ; str[i] && str[i]!=' ' ; i++,j++ )
      str[j] = str[i];

   str[j] = NULL;

   return str;
}
//------- END OF FUNCTION Misc::alltrim --------//


//------- BEGIN OF FUNCTION Misc::empty ------------//
//
// Description : empty a string
//
// Syntax      : Misc::empty(<*char>,<int>)
//
// <*char> = the pointer of the string
// <int>   = the length of the string (not include the null string )
//
void Misc::empty(char *inStr, int strLen )
{
   memset( inStr,' ',strLen);
   inStr[strLen] = NULL;
}

//-------- END OF FUNCTION Misc::empty ---------//


//------- BEGIN OF FUNCTION Misc::is_empty ------------//
//
// Description : empty a string
//
// Syntax      : Misc::is_empty(<*char>,[int],[int])
//
// <*char> = the pointer of the string
// [int]   = the length of the string (not include the null string )
//
int Misc::is_empty(char *inStr, int strLen)
{
   int i;

   if( !strLen )
      strLen = strlen(inStr);

   for( i=0 ; i<strLen ; i++ )
   {
      if( inStr[i] != ' ' )
         return 0;
   }

   return 1;
}
//-------- END OF FUNCTION Misc::is_empty ---------//


//------- BEGIN OF FUNCTION Misc::fix_str ------//
//
// format the word to the fixed length
//
// e.g. Misc::fix_str("ABC",10) -> "ABC     "
//
// <char*> str     = the string to be formated
// <int>   len     = the deserved length after formatted
// [char]  endChar = the end character of the string ( default : NULL terminator )
//
void Misc::fix_str(char* str,int len,char endChar)
{
   int oldLen;

   if ( endChar == NULL )
      oldLen = strlen(str);
   else
   {
      oldLen = Misc::str_chr(str,endChar)-1;
      err_if ( oldLen == -1 )              // the end character not found
         err_now("Misc::fix_str");
   }

   if ( len > oldLen )
      memset( str+oldLen, ' ', len-oldLen );

   str[len] = NULL;
}

//--------- END OF FUNCTION Misc::fix_str ---------//


//------- BEGIN OF FUNCTION Misc::valid_char --------//
//
// Description : test if the character is valid for field and file name
//
// Syntax      : Misc::valid_char(<char>)
//
// <char> = the character to be validify
//
// return      : SUCCEED or FAIL
//

int Misc::valid_char( char ch )
{
   return ( ch>='a' && ch<='z'  ||
            ch>='A' && ch<='Z'  ||
            ch>='0' && ch<='9'  ||
            ch=='\\'  ||  ch=='.'  || ch=='_'  ||  ch==':' ) ;
}

//--------- END OF FUNCTION Misc::valid_char ----------//


//------- BEGIN OF FUNCTION Misc::str_cmp -----------//
//
// Description : compare string
//
// Return : SUCCEED if the same
//          FAIL    if different
//
// e.g "ABCDE"    <> "ABC"
//     "ABCDE   " =  "ABCDE"
//     "ABCDE"    <> "ABCDEF"
//     "ABCDE"    <> "ABCDE "

int Misc::str_cmp( char* str1, char* str2 )
{
   err_when( !str1 || !str2 );

   int i;

   for (i=0 ; str1[i] && str2[i] ; i++)
      if ( str1[i] != str2[i] )
         return 0;

   return ( str2[i] == NULL && (str1[i]==NULL || str1[i]==' ') );
}

//--------- END OF FUNCTION Misc::str_cmp -----------//


//------- BEGIN OF FUNCTION Misc::str_cmpx -----------//
//
// Description : String Inexact comparsion
//
// Return : SUCCEED if the same
//          FAIL    if different
//
// e.g "ABCDE"    =  "ABC"
//     "ABCDE   " =  "ABCDE"
//     "ABCDE"    <> "ABCDEF"
//     "ABCDE"    <> "ABCDE "
//
int Misc::str_cmpx( char* str1, char* str2 )
{
   err_when( !str1 || !str2 );

   int i;

   for (i=0 ; str1[i] && str2[i] ; i++)
      if ( str1[i] != str2[i] )
         return 0;

   return ( str2[i] == NULL );
}

//--------- END OF FUNCTION Misc::str_cmpx -----------//


//------- BEGIN OF FUNCTION Misc::str_icmpx -----------//
//
// Description : String Inexact comparsion without case sensitive
//
// Return : SUCCEED if the same
//          FAIL    if different
//
// e.g "ABCDE"    =  "ABC"
//     "abcde   " =  "ABCDE"
//     "ABCDE"    <> "ABCDEF"
//     "ABCDE"    <> "ABCDE "

int Misc::str_icmpx( char* str1, char* str2 )
{
   err_when( !str1 || !str2 );

   int i;
   register int a,b;


   for (i=0 ; (a=str1[i]) != NULL && (b=str2[i]) != NULL ; i++)
   {
      if ( a >= 'a' && a <= 'z' )
         a -= 32;

      if ( b >= 'a' && b <= 'z' )
         b -= 32;

      if ( a != b )
         return 0;
   }

   return ( str2[i] == NULL );
}

//--------- END OF FUNCTION Misc::str_icmpx -----------//


//-------- BEGIN OF FUNCTION Misc::check_sum ----------//
//
// Return the checksum of the string
//
// <char*> = the string
// [int]   = length of the string

int Misc::check_sum(char* str, int len)
{
   int i,checksum;

   if ( len == -1 )
      len = strlen( str );

   for( checksum=0,i=0 ; i<len ; i++,str++ )
      checksum += (*str) * (i+checksum+1) ;

   return checksum;
}

//--------- END OF FUNCTION Misc::check_sum -----------//



//---------- Begin of function Misc::format --------//
//
// Format a number to a string with specified format
// Note : the formated string is right justified
//
// int   inNum    = the number to be formated
//
// [int] formatType = 1 - 1,000,000  add thousand seperator
//                    2 - $1,000,000 add thousand seperator and dollar sign
//                    3 - 56% add percentage sign % at the end of the number
//                    4 - 1000000, no thousand seperator
//                    (default : 1)
//
// return <char*> the pointer to the converted string, the string
//                is stored in static variable which will be overwritten
//                in next call.
//
char* Misc::format( int inNum, int formatType )
{
   static char outBuf[35];
   static char longBuf[25];
   char   *outPtr=outBuf;
   char   *longStr;
   int    i, intDigit, sign;

   if( inNum < 0 )
   {
      sign  = -1;
      inNum = -inNum;
   }
   else
      sign  = 0;

   longStr  = ltoa( inNum, longBuf, 10 );
   intDigit = strlen(longStr);  // no. of integer digits

   //--------- negetive bracket ------------//

   if( sign < 0 )
      *outPtr++ = '(';

   //--------- dollar sign ------------//

	if( formatType == 2 )
      *outPtr++ = '$';

	//-------- integer number -----------//

	for( i=intDigit ; i>0 ; i-- )
	{
		if( formatType != 4 )		// no thousand separators for format 4
		{
			if( i%3 == 0 && i < intDigit )
				*outPtr++ = THOUSAND_SEPARATOR;
		}

		*outPtr++ = *longStr++;
	}

	//--------- percent sign (%) ------------//

	if( formatType == 3 )
      *outPtr++ = '%';

   //--------- negetive bracket ----------//

   if( sign < 0 )
      *outPtr++ = ')';

   *outPtr++ = NULL;

   return outBuf;
}
//---------- End of function Misc::format ---------//


//---------- Begin of function Misc::format --------//
//
// Format a number to a float number to format string
// Note : the formated string is right justified
//
// <double> inNum = the number to be formated
//                  use <double> instead of <float> because
//                  fcvt() only accept <double>
//
// [int] formatType = 1 - 1,000,000  add thousand seperator
//                    2 - $1,000,000 add thousand seperator and dollar sign
//                    3 - 56% add percentage sign % at the end of the number
//
// return <char*> the pointer to the converted string, the string
//                is stored in static variable which will be overwritten
//                in next call.
//
char* Misc::format(double inNum, int formatType)
{
   enum { MONEY_DEC_PLACE = 2 };

   static char outBuf[35];
   char   *outPtr=outBuf;
   char   *floatStr;
   int    i, intDigit, sign;    // intDigit = no. of integer digits

   floatStr = fcvt( inNum, MONEY_DEC_PLACE, &intDigit, &sign );

   #ifdef DEBUG
      if( intDigit > 29 )            // integer digits can't exceed 29
         err.run( "Misc::format(), inNum : %e, formatType : %d", inNum, formatType );
   #endif

   //--------- negetive bracket ------------//

   if( inNum < 0 )
      *outPtr++ = '(';

   //--------- dollar sign ($) ------------//

   if( formatType == 2 )
      *outPtr++ = '$';

   //------- integer number -----------//

   for( i=intDigit ; i>0 ; i-- )
   {
      if( i%3 == 0 && i < intDigit )
         *outPtr++ = THOUSAND_SEPARATOR;

      *outPtr++ = *floatStr++;
   }

   if( intDigit <= 0 )
      *outPtr++ = '0';

   //------- dec. place number -----------//

   if( inNum > -1000 && inNum < 1000 )    // if the number is less than 1000, add dec. places
   {                                      // if the number is greater than 1000, truncate any dec. places
      *outPtr++ = DECIMAL_SEPARATOR;

      if( *floatStr && intDigit >= 0 )    // e.g. 0.03 --> str:"3", intDight:-1
         *outPtr++ = *floatStr++;         // 1st dec. place
      else
         *outPtr++ = '0';                 // add a '0' when intDigit < 0

      if( *floatStr )                     // 2nd dec. place
         *outPtr++ = *floatStr++;
      else
         *outPtr++ = '0';
   }

   //--------- percent sign (%) ------------//

   if( formatType == 3 )
      *outPtr++ = '%';

   //--------- negetive bracket ------------//

   if( inNum < 0 )
      *outPtr++ = ')';

   *outPtr++ = NULL;

   return outBuf;
}
//---------- End of function Misc::format ---------//


//---------- Begin of function Misc::num_to_str --------//
//
// Convert a number into string.
//
// int inNum = the number to be converted
//
// return : <char*> the converted string.
//
char* Misc::num_to_str(int inNum)
{
   static char strBuf[25];

   return ltoa( inNum, strBuf, 10 );
}
//---------- End of function Misc::format ---------//


//---------- Begin of function Misc::nullify -------//
//
// Nullify and right trim a string field in the record
//
// <char*> strPtr = string pointer
// <int>   strLen = string length
//
// Return : <char*> the nullied string which only stored in static
//                  buffer temporary until next call to nullify()
//
// note : if the string is longer than the buffer space, the string
//        will be truncated
//
char* Misc::nullify(char* strPtr, int strLen)
{
   int i;

   if( strLen > STR_BUF_LEN )
      strLen = STR_BUF_LEN;

   memcpy( str_buf, strPtr, strLen );

   for( i=strLen-1 ; i>=0 ; i-- )    // Right Trim
   {
      if( str_buf[i] != ' ' )
      {
         str_buf[i+1] = NULL;
         break;
      }
   }

   if( i<0 )              // Empty value
      str_buf[0] = NULL;

   return str_buf;
}
//----------- End of function Misc::nullify ----------//



//-------- Begin of function Misc::rtrim_fld ---------//
//
// Rtrim a text field and copy it to variable
//
// <char*> varPtr = pointer to the variable
// <char*> fldPtr = pointer to the field
// <int>   fldLen = length of the field
//
// Note <varPtr> must be pre-allocated with a len > fldLen+1
//
void Misc::rtrim_fld(char* varPtr, char* fldPtr, int fldLen)
{
   int rtrimLen = rtrim_len( fldPtr, 1, fldLen );

   memcpy( varPtr, fldPtr, rtrimLen );
   varPtr[rtrimLen] = NULL;
}
//---------- End of function Misc::rtrim_fld ---------//



//------- Begin of function Misc::atoi ---------//
//
// Same as atoi() in stdlib.h instead it allows you to specify the
// length of the string
//
// <char*> str    = the string to be converted to integer
// <int>   strLen = length of the string

int Misc::atoi( char *str, int strLen )
{
   if ( strLen >= sizeof( str_buf ) )
      strLen = sizeof( str_buf ) - 1 ;

   memcpy( str_buf, str, (size_t)strLen ) ;
   str_buf[strLen] = '\0' ;
   return ::atoi( str_buf ) ;
}
//---------- End of function Misc::atoi ---------//


//------- Begin of function Misc::sqrt ---------//
//
// Find the square root of an long integer
//
// <int> x = the value for calculating its square root
//
int Misc::sqrt(long x)
{
   err_when( x < 0 );

   long OddInt, OldArg, FirstSqrt;

   OddInt=1;
   OldArg=x;

   while(x>=0)
   {
     x-=OddInt;
     OddInt+=2;
   }

   FirstSqrt=OddInt >> 1;

   if( FirstSqrt*FirstSqrt - FirstSqrt + 1 > OldArg)
     return(FirstSqrt-1);
   else
     return(FirstSqrt);
}
//---------- End of function Misc::sqrt ---------//


//------- Begin of function Misc::diagonal_distance ---------//
//
// Given two lengths in x and y coordination, then find the diagonal
// distance between them
// result = the square root of X*X + Y*Y
//
// <int> x1, y1 = the starting point of the diagonal line
// <int> x2, y2 = the ending point of the diagonal line
//
int Misc::diagonal_distance(int x1, int y1, int x2, int y2)
{
	int x = abs(x1-x2);
	int y = abs(y1-y2);

	return Misc::sqrt( x*x + y*y );
}
//---------- End of function Misc::diagonal_distance ---------//


//------- Begin of function Misc::points_distance ---------//
//
// Given two lengths in x and y coordination, then find the
// distance between two points, taking diagonal distance
// the same as the horizontal and vertical distances.
//
// <int> x1, y1 = the starting point of the diagonal line
// <int> x2, y2 = the ending point of the diagonal line
//
int Misc::points_distance(int x1, int y1, int x2, int y2)
{
	int x = abs(x1-x2);
	int y = abs(y1-y2);

	return MAX(x, y);
}
//---------- End of function Misc::points_distance ---------//


//------- Begin of function Misc::get_random_seed --------//
//
long Misc::get_random_seed()
{
   return random_seed;
}
//---------- End of function Misc::get_random_seed ---------//


//------- Begin of function Misc::randomize --------//
//
void Misc::randomize()
{
   set_random_seed(time(NULL));
}
//---------- End of function Misc::randomize ---------//


//------- Begin of function Misc::set_random_seed --------//
//
void Misc::set_random_seed(long randomSeed)
{
	// ###### begin Gilbert 19/6 ######//
	err_when( is_seed_locked() );
	// ###### end Gilbert 19/6 ######//
   random_seed = randomSeed;
}
//---------- End of function Misc::set_random_seed ---------//


//------- Begin of function Misc::random ---------//
//
// <int> maxNum = maximum random number, it must <= 0x7FFF
//                in 32 bit compiler <int> = <long>
//
// return : <int> the random number
//
int Misc::random(int maxNum)
{
   err_if( maxNum < 0 || maxNum > 0x7FFF )
      err_now( "Misc::random()" );

	// ###### begin Gilbert 19/6 ######//
	err_when( is_seed_locked() );
	// ###### end Gilbert 19/6 ######//
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   #define RANDOM_MAX      0x7FFFU

   random_seed = MULTIPLIER * random_seed + INCREMENT;

   return maxNum * ((random_seed >> 16) & RANDOM_MAX) / (RANDOM_MAX+1);
}
//---------- End of function Misc::random ---------//


//------- Begin of function Misc::rand ---------//
//
// Return a random number from 0 to 0x7FFF
//
// return : <int> a random number from from 0 to 0x7FFF
//
int Misc::rand()
{
   #define MULTIPLIER      0x015a4e35L
   #define INCREMENT       1
   #define RANDOM_MAX      0x7FFFU

	// ###### begin Gilbert 19/6 ######//
	err_when( is_seed_locked() );
	// ###### end Gilbert 19/6 ######//
   random_seed = MULTIPLIER * random_seed + INCREMENT;

   return ((random_seed >> 16) & RANDOM_MAX);
}
//---------- End of function Misc::rand ---------//


//------- Begin of function Misc::round ---------//
//
// Round the float no. to the 2 signicant values :
//
// Since this function is not called frequently, we can afford
// to use a slower algorithm.
//
// <float> inValue        = the number to be rounded.
// <int>   signPlace      = round to no. of signicant places.
// [int]   roundDirection = 0-if <= 4, round to a smaller, if >=5 round to a bigger
//                          1-round to a smaller number
//                          2-round to a bigger number
//                          (default : 0)
//
// return : <float> the rounded number
//
float Misc::round(float inValue, int signPlace, int roundDirection)
{
   int   i;
   float baseValue = (float) 1;
   float minValue  = (float) 10;

   for( i=2 ; i<=signPlace ; i++ )
      minValue *= 10;

   while( inValue > minValue )
   {
      inValue   /= 10;
      baseValue *= 10;
   }

   float outValue = (float)((int)inValue) * baseValue;

   //-----------------------------------------------//
   //
   // If the result number is smaller than the given number
   // and the roundDirection is rounding to a bigger number,
   // than increase result number.
   //
   //-----------------------------------------------//

   if( outValue!=inValue )
   {
		if( (roundDirection==0 && ((int)inValue)%10 >= 5) ||
	  roundDirection==2 )
      {
	 outValue = (float)((int)inValue+1) * baseValue;
      }
   }

   return outValue;
}
//---------- End of function Misc::round ---------//


//------- Begin of function Misc::round_dec ---------//
//
// Round the number to its nearest 2 decimal places.
//
// <float> inNum - the input number.
//
// Return : <float> the output number
//
float Misc::round_dec(float inNum)
{
   return (float)((int)(inNum * 100)) / 100;
}
//---------- End of function Misc::round_dec ---------//


//------- Begin of function Misc::is_file_exist ---------//
//
// Check whether the given file exists in the current directory or not
//
// <char*> fileName = the name of the file
//
// return : <int> 1 - the file exists
//                0 - doesn't exist
//
int Misc::is_file_exist(const char* fileName)
{
	WIN32_FIND_DATA	findData;

   HANDLE findHandle = FindFirstFile( fileName, &findData );

   return findHandle!=INVALID_HANDLE_VALUE;
}
//---------- End of function Misc::is_file_exist ---------//


//------- Begin of function Misc::change_file_ext ---------//
//
// Change file extension.
//
// <char*> desFileName = the destination file name to be written
// <char*> srcFileName = the source file name
// <char*> newExt      = the new extension.
//
void Misc::change_file_ext(char* desFileName, char* srcFileName, const char* newExt)
{
   int nameLen = m.str_chr(srcFileName, '.');	// include the '.' in the nameLen

   err_when( nameLen<1 || nameLen>9 || strlen(newExt)>3 );

   memcpy( desFileName, srcFileName, nameLen );
   strcpy( desFileName+nameLen, newExt );        // extension for scenarion text file
}
//---------- End of function Misc::change_file_ext ---------//


//------- Begin of function Misc::extract_file_name ---------//
//
// Extract the file name from a full file path.
//
// <char*> desFileName = the destination buffer to be written
// <char*> srcFileName = the source file name
//
void Misc::extract_file_name(char* desFileName, char* srcFileName)
{
	int i;
	for( i=strlen(srcFileName) ; i>=0 ; i-- )
	{
		if( srcFileName[i]=='\\' )			// get last '\' before the file name
			break;
	}

	strncpy(desFileName, srcFileName+i+1, MAX_PATH);
	desFileName[MAX_PATH]=NULL;
}
//---------- End of function Misc::extract_file_name ---------//


//------- Begin of function Misc::num_th ---------//
//
// Convert the number into 99th format.
//
// <int> inNum = the input number
//
// return : <char*> the result string
//
char* Misc::num_th(int inNum)
{
   static String str;

   str = format(inNum);

   if( inNum >=11 && inNum <= 13 )
   {
      str += "th";
   }
   else
   {
      switch( inNum%10 )
      {
	 case 1:
	    str += "st";
	    break;

	 case 2:
	    str += "nd";
	    break;

	 case 3:
	    str += "rd";
	    break;

	 default:
	    str += "th";
		 break;
      }
   }

   return str;
}
//---------- End of function Misc::num_th ---------//


//------- Begin of function Misc::get_time ---------//
//
unsigned long Misc::get_time()
{
	return timeGetTime();
}
//---------- End of function Misc::get_time ---------//


//------- Begin of function Misc::del_array_rec ---------//
//
// Delete a record in an array.
//
// <void*> arrayBody - the array pointer
// <int>	  arraySize	- size of the array
// <int>   recSize	- record size
// <int>   delRecno  - recno to be deleted.
//
void Misc::del_array_rec(void* arrayBody, int arraySize, int recSize, int delRecno)
{
	err_when( arraySize<1 );

	err_when( delRecno<1 || delRecno>arraySize);

	int t = delRecno-1;

	char* arrayPtr = (char*) arrayBody;

	memmove( arrayPtr+recSize*t, arrayPtr+recSize*(t+1),
				recSize * (arraySize-t-1) );
}
//---------- End of function Misc::del_array_rec ---------//


//-------- Begin of function Misc::cal_move_around_a_point -------//
//	This function is used to return the x_Offset and y_Offset from the
//	center of the square.  The detail is shown in the following figure.
//
//
//  21 20 19 18 17
//  22  7  6  5 16
//  23  8  1  4 15
//  24  9  2  3 14
//  25 10 11 12 13
//
//   Assume the square is 5x5, 1 is the center of the square.  The input
// value of num determine which point is processed.  For instance, 9 is
// processed if num=9.
//
//   using the center as the reference point, (xShift, yShift) = (-1, -1)
// for num=7, (xShift, yShift) = (0,2) for num=11.
//
//   For num>25 in this case, multiply of 25 will be cut out until num<=25.
//
void Misc::cal_move_around_a_point(short num, short width, short height, int& xShift, int& yShift)
{
	short maxSqtSize = (width>height) ? height+1 : width+1;
	//short num2 = num%(maxSqtSize*maxSqtSize) + 1;
	short num2 = (num-1)%(maxSqtSize*maxSqtSize) + 1;
	
	if(num2<=MOVE_AROUND_TABLE_SIZE)
	{
		xShift = int(*(move_around_table_x+num2-1));
		yShift = int(*(move_around_table_y+num2-1));

		/*#ifdef DEBUG2
			int xShift2, yShift2;
			cal_move_around_a_point_v2(num, width, height, xShift2, yShift2);
			err_when(xShift!=xShift2);
			err_when(yShift!=yShift2);
		#endif*/
		return;
	}
	else
		cal_move_around_a_point_v2(num, width, height, xShift, yShift);
}
//------ End of function Misc::cal_move_around_a_point ---------//


//-------- Begin of function Misc::cal_move_around_a_point_v2 -------//
void Misc::cal_move_around_a_point_v2(short num, short width, short height, int& xShift, int& yShift)
{
	short maxSqtSize = (width>height) ? height+1 : width+1;
	//short num2 = num%(maxSqtSize*maxSqtSize) + 1;
	short num2 = (num-1)%(maxSqtSize*maxSqtSize) + 1;
	
	if(num2<=1)
	{
		xShift = yShift = 0;
		return;
	}

	int sqtCount = 1;

	while(sqtCount<210)	// the MAX. size of the map is 200x200
	{
		if(num2<=sqtCount*sqtCount)
			break;
		else
			sqtCount += 2;
	}

	int filter = (sqtCount-1)/2;	// is an integer
	int refNum = num2 - (sqtCount-2)*(sqtCount-2);

	//=====================================//
	// some adjustment to the refNum can
	// generate different mode of result
	//=====================================//
	// note: sqtCount>=3 for this mode
	refNum = (refNum-1-(sqtCount-3)/2)%(4*(sqtCount-1)) + 1;

	//-------------------------------------------------//
	// determine xMag
	//-------------------------------------------------//
	int xMag;
	if(refNum < sqtCount)
		xMag = refNum - 1;
	else
	{
		if(refNum>=sqtCount && refNum<=3*(sqtCount-1))
			xMag = (sqtCount<<1) - 1 - refNum; //(sqtCount-1) - (refNum-sqtCount);
		else if(refNum >= sqtCount+2*(sqtCount-1))
			xMag = refNum + 3 - (sqtCount<<2); //(refNum-sqtCount-2*(sqtCount-1)) - (sqtCount-1);
		else
			err_here();
	}

	//-------------------------------------------------//
	// calculate xShift
	//-------------------------------------------------//
	if(xMag>0)	// +ve
		xShift = (xMag>filter) ? filter : xMag;
	else			// -ve
		xShift = (-xMag>filter) ? -filter : xMag;

	//-------------------------------------------------//
	// calculate yShift
	//-------------------------------------------------//
	//ySign = (refNum>sqtCount && refNum<=3*sqtCount-3) ? -1 : 1;
	int yMag = (sqtCount-1) - abs(xMag);		// abs(xMag) + abs(yMag) always = (sqtCount-1)
	if(refNum>sqtCount && refNum<=3*sqtCount-3)	// -ve
		yShift = (yMag>filter) ? -filter : -yMag;
	else	// +ve
		yShift = (yMag>filter) ? filter : yMag;
}
//------ End of function Misc::cal_move_around_a_point_v2 ---------//


//-------- Begin of function Misc::construct_move_around_table -------//
void Misc::construct_move_around_table()
{
	if(move_around_table_size==MOVE_AROUND_TABLE_SIZE)
		return; // table already created

	int xShift, yShift;
	char *xPtr = move_around_table_x;
	char *yPtr = move_around_table_y;
	for(int i=1; i<=MOVE_AROUND_TABLE_SIZE; ++i, xPtr++, yPtr++)
	{
		cal_move_around_a_point_v2(i, MOVE_AROUND_TABLE_SIZE, MOVE_AROUND_TABLE_SIZE, xShift, yShift);
		*xPtr = char(xShift);
		*yPtr = char(yShift);
	}

	move_around_table_size = MOVE_AROUND_TABLE_SIZE;
}
//------ End of function Misc::construct_move_around_table ---------//


//-------- Begin of function Misc::set_surround_bit -------//
void Misc::set_surround_bit(long int& flag, int bitNo)
{
	static long int bitFlag[20] = {0x000001, 0x000002, 0x000004, 0x000008, 0x000010, 0x000020, 0x000040, 0x000080,
											 0x000100, 0x000200, 0x000400, 0x000800, 0x001000, 0x002000, 0x004000, 0x008000,
											 0x010000, 0x020000, 0x040000, 0x080000};

	err_when(bitNo<0 || bitNo>=20);
	flag |= bitFlag[bitNo];
}
//------ End of function Misc::set_surround_bit ---------//


//------- Begin of function Misc::roman_number -------//

char* Misc::roman_number(int inNum)
{
	err_when( inNum<1 || inNum >= 1000 );

	static const char* roman_number_array[] =
	{ "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X" };

	static String str;

	str = "";

	if( inNum > 100 )
	{
		str += roman_number_array[inNum/100-1];
		inNum = inNum - inNum/100*100;
	}

	if( inNum > 10 )
	{
		str += roman_number_array[(inNum-1)/10-1];
		inNum = inNum - (inNum-1)/10*10;
	}

	err_when( inNum<1 || inNum>10 );

	str += roman_number_array[inNum-1];

	return str;
}
//------ End of function Misc::roman_number ---------//

// ###### begin Gilbert 19/6 ########//
Misc::Misc()
{
	freeze_seed = 0;
	construct_move_around_table();
}


void Misc::lock_seed()
{
	freeze_seed = 1;
}

void Misc::unlock_seed()
{
	freeze_seed = 0;
}

int Misc::is_seed_locked()
{
	return freeze_seed > 0;
}
// ###### end Gilbert 19/6 ########//
