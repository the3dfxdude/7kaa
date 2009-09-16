//Filename    : OSTR.CPP
//Description : Object String

#include <OMISC.h>
#include <ctype.h>
#include <OSTR.h>


//------- Define static variable --------//

static char work_buf[MAX_STR_LEN+1];

//-------- Begin of constructor/destructor functions --------//

String::String()
{
   str_buf[0] = NULL;
}

String::String( char *s )
{
   strncpy(str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = NULL;
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
  work_buf[0] = NULL;

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
  work_buf[len] = NULL;

  return work_buf;
}

//------- End of function String::substr ------------//


//------- Begin of function String::upper/lower -------//

char* String::upper()
{
   memcpy( work_buf, str_buf, len()+1 );
   strupr( work_buf );

   return work_buf;
}

char* String::lower(void)
{
   memcpy( work_buf, str_buf, len()+1 );
   strlwr( work_buf );

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


//-------- Begin of operator= functions -----------//

String& String::operator=(String& s)
{
   memcpy(str_buf, s.str_buf, MAX_STR_LEN+1 );

   return *this;
}

String& String::operator=(char *s)
{
   strncpy(str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = NULL;

   return *this;
}

String& String::operator=(long value)
{
   strncpy(str_buf, m.format(value), MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = NULL;

   return *this;
}
//---------- End of operator= functions -----------//


//-------- Begin of operator+= functions -----------//

String& String::operator+=(String& s)
{
   strncat( str_buf, s.str_buf, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = NULL;
   return *this;
}

String& String::operator+=(char *s)
{
   strncat( str_buf, s, MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = NULL;
   return *this;
}

String& String::operator+=(long value)
{
   strncat( str_buf, m.format(value), MAX_STR_LEN );
   str_buf[MAX_STR_LEN] = NULL;
   return *this;
}

//---------- End of operator+= functions -----------//


//-------- Begin of operator*= functions -----------//

String& String::operator*=(int n)
{
  memcpy( work_buf, str_buf, len()+1 );

  for(int i=1; i<n; i++)
     strncat(str_buf, work_buf, MAX_STR_LEN);

  str_buf[MAX_STR_LEN] = NULL;

  return *this;
}

//---------- End of operator*= functions -----------//

