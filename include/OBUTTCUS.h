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

	char			button_wait; // user pushed button and waiting for release
	uint32_t		button_wait_timeout;

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
	bool detect_hover();
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
