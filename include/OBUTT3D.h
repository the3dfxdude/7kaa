//Filename    : OBUTT3D.H
//Description : Header file of button object

#ifndef __OBUTT3D_H
#define __OBUTT3D_H

//------- Define button type -------//

enum { BUTTON_TYPE_SHELL,		// use the button standard shells combining with bitmap for each button
		 BUTTON_TYPE_BITMAP,		// display the button using a single bitmap
	  };

//------- Define button styles -------//

enum { BUTTON_STYLE_ACTION='A' };		// Action button

enum { BUTTON_ACTION_WIDTH=52,
		 BUTTON_ACTION_HEIGHT=43,
	  };

//------- Define class Button3D -------//

class Button3D
{
public:
	enum { HELP_CODE_LEN=8 };

	char     	   init_flag;
	short 	  		x1,y1,x2,y2;   // some function will need to access the button's coordination for area detection
	char				button_type;
	char     	   button_style;

	char  	  		pushed_flag;
	char  	  		enable_flag;   // either 1(Yes) or 0(No)
	char  	  		elastic_flag;

	unsigned short button_key;     // button is pressed when user press this key

	char* 	  		icon_ptr;      // can be a text pointer, bitmap pointer or a pointer to user defined function
	char*				button_up_ptr;
	char*				button_down_ptr;
	char*				button_disabled_ptr;

	char				help_code[HELP_CODE_LEN+1];

public:
	Button3D();

	void create(int pX1, int pY1, char buttonStyle,
					char* buttonName, char elasticFlag=1, char defIsPushed=0);

	void create(int pX1, int pY1, char* upButtonName,
					char* downButtonName, char elasticFlag, char defIsPushed);

	void set_key(unsigned keyCode)       { button_key = keyCode; }

	void paint(int pX1, int pY1, char buttonStyle,
					char* buttonName, char elasticFlag=1, char defIsPushed=0)
		  { create( pX1, pY1, buttonStyle, buttonName, elasticFlag, defIsPushed ); paint(); }

	void paint(int pX1, int pY1, char* upButtonName,
					char* downButtonName, char elasticFlag=1, char defIsPushed=0)
		  { create( pX1, pY1, upButtonName, downButtonName, elasticFlag, defIsPushed ); paint(); }

	void paint(int defIsPushed= -1);

	void set_help_code(char* helpCode);
	void update_bitmap(char* buttonName);

	void reset()                         { init_flag=0; }
	void hide();

	int  detect(unsigned=0,unsigned=0,int=0,int=0);

	void push()        { if(!pushed_flag) paint(1); }
	void pop()         { if(pushed_flag)  paint(0);  }

	void disable()     { if(enable_flag)  { enable_flag=0; paint(); } }
	void enable()      { if(!enable_flag) { enable_flag=1; paint(); } }
};

//-------------------------------------------------//

#endif
