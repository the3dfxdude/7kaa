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

//Filename    : OSTR.H
//Description : Header file of object String

#ifndef __OSTR_H
#define __OSTR_H

#include <string.h>

//--------- Define macro ant ------------//

#define NOCLIP          0
#define CLIP            1

#define MAX_STR_LEN  200

//------- Define class String --------------//

class String
{
public:
   char   str_buf[MAX_STR_LEN+1];

public:
   String();                            // default ructor;
   String( char *s);
   String( const char *s);
   String( String& s);

   char*  upper();
   char*  lower();
   int    len()         { return strlen(str_buf); }
   int    at(char*);
   void   catf(char *format, ...);

   char*  right(int needLen)           { return substr( len()-needLen ); }
   char*  left(int needLen)             { return substr( 0, needLen ); }
   char*  substr(int, int=0);

   //-------- class overloaded operators -----------//

   operator char*() { return str_buf; }

   String& operator=( String&); // str1 = str
   String& operator=( char*);   // str1 = char*
   String& operator=( const char*);   // str1 = char*
   String& operator=( long );   // str1 = long

   String& operator+=( String&);        // str1 += str
   String& operator+=( char*);  // str1 += char*
   String& operator+=( const char*);  // str1 += const char*
   String& operator+=( long );  // str1 += long

   String& operator*=(int n);   // str1 *= n
   char    operator[](int n)    { return str_buf[n]; }

   //----------- string comparsion functions ----------//

   friend int operator == ( String& s1, String& s2) { return strcmp(s1.str_buf,s2.str_buf)==0; }
   friend int operator == ( String& s1, char*   s2) { return strcmp(s1.str_buf,s2)==0; }
   friend int operator == ( char*   s1, String& s2) { return strcmp(s1,s2.str_buf)==0; }
   friend int operator != ( String& s1, String& s2) { return strcmp(s1.str_buf,s2.str_buf)!=0; }
   friend int operator != ( String& s1, char*   s2) { return strcmp(s1.str_buf,s2)!=0; }
   friend int operator != ( char*   s1, String& s2) { return strcmp(s1,s2.str_buf)!=0; }
   friend int operator <  ( String& s1, String& s2) { return strcmp(s1.str_buf,s2.str_buf)<0; }
   friend int operator <  ( String& s1, char*   s2) { return strcmp(s1.str_buf,s2)<0; }
   friend int operator <  ( char*   s1, String& s2) { return strcmp(s1,s2.str_buf)<0; }
   friend int operator >  ( String& s1, String& s2) { return strcmp(s1.str_buf,s2.str_buf)>0; }
   friend int operator >  ( String& s1, char*   s2) { return strcmp(s1.str_buf,s2)>0; }
   friend int operator >  ( char*   s1, String& s2) { return strcmp(s1,s2.str_buf)>0; }
   friend int operator <= ( String& s1, String& s2) { return strcmp(s1.str_buf,s2.str_buf)<=0; }
   friend int operator <= ( String& s1, char*   s2) { return strcmp(s1.str_buf,s2)<=0; }
   friend int operator <= ( char*   s1, String& s2) { return strcmp(s1,s2.str_buf)<=0; }
   friend int operator >= ( String& s1, String& s2) { return strcmp(s1.str_buf,s2.str_buf)>=0; }
   friend int operator >= ( String& s1, char*   s2) { return strcmp(s1.str_buf,s2)>=0; }
   friend int operator >= ( char*   s1, String& s2) { return strcmp(s1,s2.str_buf)>=0; }
};

//------------------------------------------------------//

#endif
