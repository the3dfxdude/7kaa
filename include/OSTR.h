//Filename    : OSTR.H
//Description : Header file of object String

#ifndef __OSTR_H
#define __OSTR_H

#ifndef __STRING_H
#include <string.h>
#endif

//--------- Define macro ant ------------//

#define LEFT_JUSTIFY    0
#define CENTER_JUSTIFY  1
#define RIGHT_JUSTIFY   2
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
   String( String& s);

   char*  upper();
   char*  lower();
   int    len()         { return strlen(str_buf); }
   int    at(char*);

   char*  right(int needLen)           { return substr( len()-needLen ); }
   char*  left(int needLen)             { return substr( 0, needLen ); }
   char*  substr(int, int=0);

   //-------- class overloaded operators -----------//

   operator char*() { return str_buf; }

   String& operator=( String&); // str1 = str
   String& operator=( char*);   // str1 = char*
   String& operator=( long );   // str1 = long

   String& operator+=( String&);        // str1 += str
   String& operator+=( char*);  // str1 += char*
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
