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

//Filename    : OSTR.CPP
//Description : Object String

#include <OMISC.h>
#include <OSTR.h>
#include <stdarg.h>
#include <stdio.h>

#include <cctype>


//------- Define static variable --------//

static char work_buf[MAX_STR_LEN+1];

//-------- Begin of constructor/destructor functions --------//

String::String()
{
   str_buf[0] = '\0';
}

String::String( char *s )
{
   strncpy(str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = '\0';
}

String::String( const char *s )
{
   strncpy(str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = '\0';
}

String::String( String& s )
{
   memcpy(str_buf, s.str_buf, MAX_STR_LEN+1 );
}

//---------- End of constructor/destructor functions --------//


//------- Begin of function String::substr --------//
//
// <int> pos = the start position of the sub string you want
//             start from 0, 0-aligned
// [int] len = the length of the sub string
//             (default : till the end of the string
//
char* String::substr(int pos, int len)
{
  work_buf[0] = '\0';

  int strLen = strlen(str_buf);

  if(pos >= strLen)                     // check for boundary errors
     return work_buf;

  if( len <= 0 )
     len = strLen - pos;
  else
  {
     if(len > strLen - pos)
        return work_buf;
  }

  strncpy( work_buf, str_buf+pos, len );
  work_buf[len] = '\0';

  return work_buf;
}

//------- End of function String::substr ------------//


//------- Begin of function String::upper/lower -------//

char* String::upper()
{
   memcpy( work_buf, str_buf, len()+1 );

   char* str = work_buf;
   while (*str)
   {
	   *str = std::toupper(*str);
	   str++;
   }

   return work_buf;
}

char* String::lower(void)
{
   memcpy( work_buf, str_buf, len()+1 );

   char* str = work_buf;
   while (*str)
   {
	   *str = std::tolower(*str);
	   str++;
   }

   return work_buf;
}

//--------- End of function String::upper/lower --------//


//-------- Begin of function String::at ---------------//
//
// <char*> searchStr = the string you want to search for
//
// return : >= 0 the position of the search str in the string
//          = -1 if not found
//
int String::at(char* searchStr)
{
  int pos;
  char *tmp;

  if( (tmp = strstr(str_buf, searchStr)) != NULL)
    pos = (int)(tmp-str_buf);
  else
    pos = -1;

  return pos;
}
//---------- End of function String::at ---------------//


//-------- Begin of function String::catf ---------------//
//
// A formatted string concatenation
//
void String::catf(const char *format, ...)
{
   int l = strlen(str_buf);
   int left = MAX_STR_LEN-l;
   if( left <= 0 )
      return;
   va_list valist;
   va_start(valist, format);
   vsnprintf(str_buf+l, left+1, format, valist);
   va_end(valist);
}
//---------- End of function String::catf ---------------//


//-------- Begin of operator= functions -----------//

String& String::operator=(String& s)
{
   memcpy(str_buf, s.str_buf, MAX_STR_LEN+1 );

   return *this;
}

String& String::operator=(char *s)
{
   strncpy(str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = '\0';

   return *this;
}

String& String::operator=(const char *s)
{
   strncpy(str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = '\0';

   return *this;
}

String& String::operator=(long value)
{
   strncpy(str_buf, misc.format(value), MAX_STR_LEN);
   str_buf[MAX_STR_LEN] = '\0';

   return *this;
}
//---------- End of operator= functions -----------//


//-------- Begin of operator+= functions -----------//

String& String::operator+=(String& s)
{
   strncat( str_buf, s.str_buf, MAX_STR_LEN-strlen(str_buf) );
   str_buf[MAX_STR_LEN] = '\0';
   return *this;
}

String& String::operator+=(char *s)
{
   strncat( str_buf, s, MAX_STR_LEN-strlen(str_buf) );
   str_buf[MAX_STR_LEN] = '\0';
   return *this;
}

String& String::operator+=(const char *s)
{
   strncat( str_buf, s, MAX_STR_LEN-strlen(str_buf) );
   str_buf[MAX_STR_LEN] = '\0';
   return *this;
}

String& String::operator+=(long value)
{
   (*this) += misc.format(value);
   return *this;
}

//---------- End of operator+= functions -----------//


//-------- Begin of operator*= functions -----------//

String& String::operator*=(int n)
{
   int l = strlen(str_buf);
   if( !l )
      return *this;

   int left = MAX_STR_LEN-l;
   char *end = str_buf+l;

   for( int i=1; i<n && left > 0; i++ )
   {
      int w = left>l ? l : left;
      memcpy(end, str_buf, w);
      left -= w;
      end += w;
   }
   *end = '\0';

   return *this;
}

//---------- End of operator*= functions -----------//

