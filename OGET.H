//Filename    : OGET.H
//Description : Header file of OGET.CPP

#ifndef __OGET_H
#define __OGET_H


//-------- Define macros -------------//

enum { MAX_GET_WIDTH = 258 };

//---------- Define data structure ----------//

struct GetFld
{
   short x;
   short y;
   short x2;

   void* data;
   char  type;

   short width_data;
   short width_scr;
   short dec_num;

   char* picture;
   char  upper_convert;

   char  (*valid_ptr)() ;
   char  (*call_ptr) (GetFld*, char*, int);
   int   call_data;      // parameter used in calling (*call_ptr)()
};

typedef char GetCall( GetFld *, char *, int ) ;


//--------- Define Class Structure ------------//

class Font;

class Get
{
public:
   char   mask_flag;	   // whether mask the display (e.g. for displaying password)

private:

   //--------- Calling parameters ---------//

   short  get_num;          // the total no. of gets
   short  max_get_num;       // maximum no. of gets, allocated memory

   Font*  font_ptr;

   char   up_down_exit;    // exit when UP is press at the first field or DOWN is press at the last field

   short  fld_x1;          // for field() and detect() only
   short  fld_y1;
   short  fld_x2;
   short  fld_y2;

   //--------- Internal variables ----------//

   GetFld *fld_array;
   GetFld *fld_ptr;

   char   insert_flag;          // insert flag

public:

   Get();
   ~Get()         { deinit(); }

   void init(int,int);
   void deinit();

   void set_font(Font*);

   void add(int,int,void*,char,int);
   void picture(char*,int=0);

   void call(GetCall*,int);
   void valid( char(*)()) ;
   void width(int,int);

   char select(int,char **sel_opt);

   void display();
   int  read(int=1);

   //--- high level function for creating & detecting field ---//

	void field(int,int,char*,int,char*,int,int);
	void field(int x1, int y1, char* dataPtr, int dataWidth, int x2);

   int  detect();

private:
   void check_data(GetFld*);
   void display_get(GetFld*,int);
   int  load_data(GetFld*,char*);
   void store_data( GetFld *, char *, char * ) ;
   int  right_pos( int, int, char * ) ;
   int  num_pos( int, int, char * ) ;
   void cursor_type(int);

   //-------- called by read() only ----------//

   int process_field_key( int keyCode, GetFld* getPtr, char* dataBuf,
                               char* pictBuf, int& curPos, int& bufPos);

   int valid_pict( int keyCode, GetFld* getPtr, char* dataBuf,
                        char* pictBuf, int& bufPos );

   int process_inter_field_key( int keyCode, GetFld* getPtr, int& firstFlag,
                                int& firstKey, int& fieldId, int& bufPos );

   void init_field(char* dataBuf, char* pictBuf  , GetFld* getPtr,
                   int&  bufPos , int&  bufOffset, int &bufWidth );

   int get_next_key( char* dataBuf, char* pictBuf, GetFld* getPtr,
                          int&  bufPos , int&  bufOffset, int &bufWidth );

   int store_and_valid( int keyCode, char* dataBuf, char* pictBuf, GetFld* getPtr,
                        int& firstFlag );
};

//---------------------------------------------------//

#endif




