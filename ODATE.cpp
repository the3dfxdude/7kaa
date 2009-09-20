//Filename    : ODATE.CPP
//Description : Date Information Object

#include <stdlib.h>
#include <string.h>
#include <windowsx.h>

#include <OSTR.h>
#include <OMISC.h>
#include <OTRANSL.h>
#include <ODATE.h>


//--------- Define static member variables -----------//


#define  JULIAN_ADJUSTMENT    1721425L

static char month_str_array[][10] =
{
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December"
} ;

static int month_tot[]=
    { 0, 0,  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 } ;

// Jan Feb Mar  Apr  May  Jun   Jul  Aug  Sep  Oct  Nov  Dec
// 31  28  31   30   31   30     31   31   30   31   30   31


//------------ Begin of function DateInfo::julain ----------//
//
// Convert from year, month, day integer format to julian date format
//
// Julian day is the number of days since the date  Jan 1, 4713 BC
// Ex.  Jan 1, 1981 is  2444606
//
// <int> year, month, day = the components of the date
//
// Return : <long> the julian date
//          -1     illegal given date
//
long DateInfo::julian( int year, int month, int day )
{
	long  total, dayYear ;

	dayYear    =  day_year( year, month, day) ;

	if (dayYear < 1)
		return( -1) ;  /* Illegal Date */

	total =  ytoj(year) ;
	total+=  dayYear ;
	total+=  JULIAN_ADJUSTMENT ;

	return total;
}
//-------------- End of function DateInfo::julian ------------//


//--------- Begin of function DateInfo::julian ---------//
//
// Convert a string such as "07/03/93" to long number
//
// Returns:
// <int> >0  -  Julian day
//              That is the number of days since the date  Jan 1, 4713 BC
//              Ex.  Jan 1, 1981 is  2444606
//        0  -  NULL Date (dbf_date is all blank)
//       -1  -  Illegal Date
//
long DateInfo::julian( char *dateStr )
{
   return julian( m.atoi( dateStr,4 ), m.atoi( dateStr+4,2 ),
                  m.atoi( dateStr+6,2 ) );
}
//---------- End of function DateInfo::julian ----------//


//------------ Begin of function DateInfo::get_date ---------//
//
// Return year, month or day of the given julian date
//
// <long> julianDate = the julian date to be converted
// <char> returnType = 'Y'-year, 'M'-month, 'D'-day
//
// Return : the year, month or day of the julian date
//          -1 if the julian date passed is invalid
//
int DateInfo::get_date( long julianDate, char returnType )
{
   int   year, month, day, nDays, maxDays ;
   long   totalDays ;

   if ( julianDate > 5373484 || julianDate < JULIAN_ADJUSTMENT )
      return -1;

   totalDays  =  (long) (julianDate) - JULIAN_ADJUSTMENT ;
   year       =  (int) ((double)totalDays/365.2425) + 1 ;
   nDays      =  (int) (totalDays -  ytoj(year)) ;

   if ( nDays <= 0 )
   {
      year-- ;
      nDays   =  (int) (totalDays - ytoj(year)) ;
   }

   if (year%4 == 0 && year%100 != 0 || year%400 == 0)
      maxDays =  366 ;
   else
      maxDays =  365 ;

   if ( nDays > maxDays )
   {
      year++ ;
      nDays -= maxDays ;
   }

   if ( month_day( year, nDays, month, day ) < 0 )
      return -1;

   //............................................//

   switch( returnType )
   {
      case 'Y':
         return year;

      case 'M':   // return the month
         return month;

      case 'D':
         return day;
   }

   return 0;
}
//------------- End of function DateInfo::get_date --------//



//------------ Begin of function DateInfo::date_str ---------//
//
// Convert from julian date format to string date format
// Return a date string in the format of DD MMM YYYY e.g. 15 Jan 1992
//
// Julian day is the number of days since the date  Jan 1, 4713 BC
// Ex.  Jan 1, 1981 is  2444606
//
// <long> julianDate    = the julian date to be converted
// [int]  shortMonthStr = short month string or not (e.g. Jan instead of January)
//                        (default : 0)
//
// Return : <char*> the formated date string
//
char* DateInfo::date_str( long julianDate, int shortMonthStr)
{
   int    year, month, day, nDays, maxDays ;
   long   totalDays ;
   static char strBuf[16];

   if ( julianDate > 5373484 || julianDate < JULIAN_ADJUSTMENT )
   {
      strBuf[0]=NULL;
      return strBuf;
   }

   totalDays  =  (long) (julianDate) - JULIAN_ADJUSTMENT ;
   year       =  (int) ((double)totalDays/365.2425) + 1 ;
   nDays      =  (int) (totalDays -  ytoj(year)) ;

   if ( nDays <= 0 )
   {
      year-- ;
      nDays   =  (int) (totalDays - ytoj(year)) ;
   }

   if (year%4 == 0 && year%100 != 0 || year%400 == 0)
      maxDays =  366 ;
   else
      maxDays =  365 ;

   if ( nDays > maxDays )
   {
      year++ ;
      nDays -= maxDays ;
   }

   if ( month_day( year, nDays, month, day ) < 0 )
   {
      strBuf[0]=NULL;
      return strBuf;
   }

   //--------------------------------------------//

   static String str;

#if(defined(SPANISH))
	if( shortMonthStr )
	{
		str  = itoa(day,strBuf,10);		// day
		str += "-";
		strcpy(strBuf, translate.process(month_str_array[month-1]));
		if(strlen(strBuf) > 3)
			strBuf[3] = '\0';		// limit month to 3 chars
		str += strBuf;							// month
		str += "-";
	   str += itoa(year,strBuf,10);		// year
	}
	else
	{
		str  = itoa(day,strBuf,10);		// day
		str += " de ";
		str += translate.process(month_str_array[month-1]);
		str += " de ";
	   str += itoa(year,strBuf,10);		// year
	}
#elif(defined(FRENCH))
	str  = itoa(day,strBuf,10);		// day
	str += " ";
	if( shortMonthStr )
	{
		strcpy(strBuf, translate.process(month_str_array[month-1]));
		if(strlen(strBuf) > 3)
			strBuf[3] = '\0';		// limit month to 4 chars
		if(month == 7)				// Juillet(July) abbreviated to Jul.
			strBuf[2] = 'l';
		str += strBuf;							// month
	}
	else
	{
		str += translate.process(month_str_array[month-1]);
	}
	str += " ";
	str += itoa(year,strBuf,10);		// year
#else
	// GERMAN and US
	str = translate.process(month_str_array[month-1]);
   if( shortMonthStr )
      str = str.left(3);
   str += " ";
   str += itoa(day,strBuf,10);
   str += ", ";
   str += itoa(year,strBuf,10);
#endif

   return str;
}
//------------- End of function DateInfo::date_str --------//


//------------ Begin of function DateInfo::month_str ---------//
//
// <int> monthNo = the month (1-12)
//
// Return : <char*> the month string
//
char* DateInfo::month_str(int monthNo)
{
	return translate.process(month_str_array[monthNo-1]);
}
//------------- End of function DateInfo::month_str --------//


//---------- Begin of function DateInfo::day_year ----------//
//
// Returns an (int) day of the year starting from 1.
// Ex.    Jan 1, returns  1
//
// Returns      -1  if it is an illegal date.
//

int DateInfo::day_year( int year, int month, int day )
{
   int  isLeap, monthDays ;

   isLeap =   ( year%4 == 0 && year%100 != 0 || year%400 == 0 ) ?  1 : 0 ;

   monthDays =  month_tot[ month+1 ] -  month_tot[ month] ;
   if ( month == 2 )  monthDays += isLeap ;

   if ( year  < 0  ||
        month < 1  ||  month > 12  ||
        day   < 1  ||  day   > monthDays )
        return( -1 ) ;  /* Illegal Date */

   if ( month <= 2 )  isLeap = 0 ;

   return(  month_tot[month] + day + isLeap ) ;
}

//------------- End of function DateInfo::day_year -----------//


//----------- Begin of function DateInfo::ytoj ----------//
//
// c4ytoj -  Calculates the number of days to the year
//
//     This calculation takes into account the fact that
// 1)  Years divisible by 400 are always leap years.
// 2)  Years divisible by 100 but not 400 are not leap years.
// 3)  Otherwise, years divisible by four are leap years.
//
//     Since we do not want to consider the current year, we will
//     subtract the year by 1 before doing the calculation.

long DateInfo::ytoj( int yr )
{
   yr-- ;
   return( yr*365L +  yr/4L - yr/100L + yr/400L ) ;
}

//--------------- End of function DateInfo::ytoj -----------//


//--------- Begin of function DateInfo::month_day ---------//
//
//  Given the year and the day of the year, returns the
//  month and day of month.
//
int DateInfo::month_day( int year, int days,  int &monthRef,  int &dayRef )
{
   int isLeap, i ;

   isLeap =  ( year%4 == 0 && year%100 != 0 || year%400 == 0 ) ?  1 : 0 ;
   if ( days <= 59 )  isLeap = 0 ;

   for( i = 2; i <= 13; i++)
   {
      if ( days <=  month_tot[i] + isLeap )
      {
         monthRef =  --i ;
         if ( i <= 2) isLeap = 0 ;

         dayRef   =  days - month_tot[ i] - isLeap ;
         return( 0) ;
      }
   }
   dayRef   =  0 ;
   monthRef =  0 ;

   return( -1 ) ;
}

//----------- End of function DateInfo::month_day -----------//



//--------- Begin of function DateInfo::time_str ---------//
//
// Given a integer as time and then return the time string
//
// <int> inTime = the time in integer format (e.g. 1600 --> "16:00" )
//
char* DateInfo::time_str(int inTime)
{
	// #### begin Gilbert 18/8 ####//
	static char strBuf[6] = "00:00";
	//itoa( inTime/100, strBuf, 10 );
	//itoa( inTime%100, strBuf+3, 10 );
	strBuf[4] = '0' + inTime % 10;
	strBuf[3] = '0' + (inTime/10) % 10;
	strBuf[1] = '0' + (inTime/100) % 10;
	strBuf[0] = '0' + (inTime/1000) % 10;
	// #### end Gilbert 18/8 ####//

   return strBuf;
}
//---------- End of function DateInfo::time_str ----------//


//-------- Begin of function DateInfo::days_in_month ------//
//
// return the no. of days in a given month
//
// <int> inMonth = the specified month
//
// return : <int> = the no. of days in a given monthh
//
int DateInfo::days_in_month(int inMonth)
{
   return month_tot[inMonth+1] - month_tot[inMonth];
}
//---------- End of function DateInfo::days_in_month ------//


//-------- Begin of function DateInfo::add_months ------//
//
// Add months to the given julian date.
//
// <int> inDate   = the input julian date
// <int> addMonth = the no. of months to add
//
// return : <int> - the result julian date
//
int DateInfo::add_months(int inDate, int addMonth)
{
	int inYear  = year(inDate);
	int inMonth = month(inDate);
	int inDay   = day(inDate);

	inMonth += addMonth;

	if( inMonth > 12 )
	{
		inMonth -= 12;
		inYear++;
	}

	if( inDay > days_in_month(inMonth) )
		inDay = days_in_month(inMonth);

	return julian( inYear, inMonth, inDay );
}
//---------- End of function DateInfo::add_months ------//


//-------- Begin of function DateInfo::file_time_to_julian ------//
//
int DateInfo::file_time_to_julian(FILETIME& fileTime)
{
/*
	WORD dosTime, dosDate;

	CoFileTimeToDosDateTime( &fileTime, &dosDate, &dosTime );

	//--------------------------------------------//
	// Bits	Contents
	// 0-4	Days of the month (1-31).
	// 5-8	Months (1 = January, 2 = February, and so forth).
	// 9-15	Year offset from 1980 (add 1980 to get actual year).
	//--------------------------------------------//

	return julian( 1980 + (dosDate>>9), (dosDate>>5) & 0x0F, dosDate & 0x0F );
*/
	return 0;
}
//---------- End of function DateInfo::file_time_to_julian ------//

