//Filename    : OSLIDER.H
//Description : Header of object Slider

#ifndef __OSLIDER_H
#define __OSLIDER_H

//--------- Define class Slider --------//

class Slider
{
public:
   short slider_x1, slider_y1, slider_x2, slider_y2;
   short bar_width;

   char  bar_color, bar_color2;

   int*  var_ptr;
   int   max_value, std_value;

public:
   void init( int,int,int,int,int,int*,int,int=0,int=0 );

   void paint();
   void refresh();
   int  detect();
};


//------- Define class SliderGroup --------//

class SliderGroup
{
public:
   int     slider_num;
   Slider* slider_array;

public:
   SliderGroup(int);
   ~SliderGroup()                { delete slider_array; }

   void paint();
   int  detect();

   Slider& operator[](int recNo) { if( recNo>slider_num ) recNo=0; return slider_array[recNo]; }
};

//-----------------------------------------//

#endif
