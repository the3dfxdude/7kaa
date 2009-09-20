//Filename    : MISC.H
//Description : Header file for MISC function object

#ifndef __MISC_H
#define __MISC_H

//-------- Define macro constant ---------//

#define LINE_FEED    0xA        // Line feed ascii code
#define RETURN       0xD

//-----------------------------------------//

class Misc
{
public:
	enum  { STR_BUF_LEN = 120 };

	char  str_buf[STR_BUF_LEN+1];
	char	freeze_seed;
	long  random_seed;

public:
	Misc();

   int   str_cut(char*,char*,int,int=-1);
   int   str_chr(char*,char,int=1,int=-1);
   int   str_str(char*,char*,int=1,int=-1);
   int   str_cmp(char*,char*);
   int   str_cmpx(char*,char*);
   int   str_icmpx(char*,char*);
   void  str_shorten(char*,char*,int);

	int   upper(int);
	int   lower(int);

   int   rtrim_len(char*,int=1,int=-1);
   int   ltrim_len(char*,int=1,int=-1);
   void  rtrim(char*,char*);
   void  ltrim(char*,char*);
   void  alltrim(char*,char*);
   char* rtrim(char*);
   char* ltrim(char*);
   char* alltrim(char*);

   char* nullify(char*,int);
   void  rtrim_fld(char*,char*,int);
   int   atoi(char*,int);

   void  empty(char*,int);
   int   is_empty(char*,int=0);

   int   valid_char(char);
   void  fix_str(char*,int,char=0);
   int   check_sum(char*,int=-1);

   char* format(double,int=1);
   char* format_percent(double);
   char* format(int,int=1);

   char* format(short a,int b=1) { return format((int)a, b); }
   char* format(long  a,int b=1) { return format((int)a, b); }
   char* num_th(int);
	char* num_to_str(int);
	char* roman_number(int);

   int   get_key();
   int   key_pressed();
   int   is_touch(int,int,int,int,int,int,int,int);

   int   sqrt(long);
	int   diagonal_distance(int,int,int,int);
	int   points_distance(int,int,int,int);
	float round(float,int,int=0);
   float round_dec(float);

	void  delay(float wait);
	unsigned long get_time();

   void  randomize();
   void  set_random_seed(long);
   long  get_random_seed();
   int   rand();
   int   random(int);

   int   is_file_exist(char*);
   void  change_file_ext(char*,char*,char*);
	void  extract_file_name(char*, char*);

   void  put_text_scr(char*);
	void  del_array_rec(void* arrayBody, int arraySize, int recSize, int delRecno);

	void	cal_move_around_a_point(short num, short width, short height, int& xShift, int& yShift);
	void	cal_move_around_a_point_v2(short num, short width, short height, int& xShift, int& yShift);
	void	set_surround_bit(long int& flag, int bitNo);

	void	lock_seed();
	void	unlock_seed();
	int	is_seed_locked();

private:
	void	construct_move_around_table();
};

extern Misc m, m2;      // two instance for separate random_seed

//---------- End of define class ---------------//


//--------- Begin of inline function Misc::is_touch ------------//
//
// Check if the given two area touch each other
//
// Return : 1 or 0
//
inline int Misc::is_touch(int x1, int y1, int x2, int y2, int a1, int b1, int a2, int b2 )
{
	return (( b1 <=  y1 && b2 >=  y1 ) ||
			  (  y1 <= b1 &&  y2 >= b1 )) &&
			 (( a1 <=  x1 && a2 >=  x1 ) ||
			  (  x1 <= a1 &&  x2 >= a1 ));
}
//--------- End of inline function Misc::is_touch -----------//

#endif
