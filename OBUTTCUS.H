//Filename    : OBUTTCUS.H
//Description : Header file of button object

#ifndef __OBUTTCUS_H
#define __OBUTTCUS_H


//------- Define type ButtonCustomFP -------//

class ButtonCustom;

typedef void (*ButtonCustomFP)(ButtonCustom *, int repaintBody);

//------- Define struct ButtonCustomFPPara -------//

struct ButtonCustomPara
{
	void*	ptr;
	int	value;

	ButtonCustomPara( void *p, int v) : ptr(p), value(v) {} 
};

//------- Define class ButtonCustom -------//

class ButtonCustom
{
public:
	char     	   init_flag;
	short 	  		x1,y1,x2,y2;   // some function will need to access the button's coordination for area detection

	char  	  		pushed_flag;
	char  	  		enable_flag;   // either 1(Yes) or 0(No)
	char  	  		elastic_flag;

	unsigned short button_key;     // button is pressed when user press this key
	
	ButtonCustomFP	body_fp;
	ButtonCustomPara custom_para;		// let body_fp to read a parameter

public:
	ButtonCustom();

	void create(int pX1, int pY1, int pX2, int pY2, ButtonCustomFP funcPtr,
		ButtonCustomPara funcPara, char elasticFlag=1, char defIsPushed=0);

	void paint(int pX1, int pY1, int pX2, int pY2, ButtonCustomFP funcPtr,
		ButtonCustomPara funcPara, char elasticFlag=1, char defIsPushed=0)
		{ create(pX1, pY1, pX2, pY2, funcPtr, funcPara, elasticFlag, defIsPushed); paint();}

	void paint(int defIsPushed= -1, int repaintBody=1);
	void reset()                         { init_flag=0; }
	void hide();

	int  detect(unsigned=0,unsigned=0,int=0,int=0);

	void push()        { if(!pushed_flag) paint(1); }
	void pop()         { if(pushed_flag)  paint(0);  }

	void disable()     { if(enable_flag)  { enable_flag=0; paint(); } }
	void enable()      { if(!enable_flag) { enable_flag=1; paint(); } }

	static void disp_text_button_func(ButtonCustom *, int repaintBody);

};

//------- Define class ButtonCustomGroup ----------------//

class ButtonCustomGroup
{
public:
   int     button_num;
   int     button_pressed;
   ButtonCustom* button_array;

public:
	ButtonCustomGroup(int);
	~ButtonCustomGroup();

   void paint(int= -1);
   int  detect();
   void push(int, int paintFlag=1);

   ButtonCustom& operator[](int);
   int     operator()()          { return button_pressed; }
};
//-------------------------------------------------//

#endif
