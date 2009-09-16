//Filename   : OSPINNER.H
//Description: Hander file for class Spinner

#ifndef __OSPINNER_H
#define __OSPINNER_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OINFO_H
#include <OINFO.h>
#endif

#ifndef __OFONT_H
#include <OFONT.h>
#endif

//------------- Define constant --------------//

#define MAX_SPINNER_OPTION		16

//----------- Define class Spinner -----------//

class Spinner
{
public:
	char	init_flag;
	short	x1, y1, x2, x3, y2;

	char* option_des_array[MAX_SPINNER_OPTION];
	char  option_count;

	Font* font_ptr;
	char* spinner_icon;
	char* spinner_des;

	char	selected_id;		// selected option id.

	//------ static member vars ------//

	static Font* default_font_ptr;
	static char* default_spinner_icon;

public:
	Spinner()		{ init_flag=0; }

	void	init(int x1, int y1, char* spinnerDes, int x2, int x3, Font* fontPtr);
	void	add_option(char* optionDes);
	void	set_selected(int selectedId)		{ selected_id = selectedId; }

	void 	disp(int refreshFlag=INFO_REPAINT);
	int  	detect();

	void	set_default_font(Font* fontPtr)	{ default_font_ptr = fontPtr; }
	void	set_default_icon(char* iconName);
};

//----------------------------------------------------//

#endif