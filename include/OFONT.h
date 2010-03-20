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

//Filename    : OFONT.H
//Description : Header file of Object Font

#ifndef __OFONT_H
#define __OFONT_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OVGABUF_H
#include <OVGABUF.h>
#endif

#ifndef __OTRANSL_H
#include <OTRANSL.h>
#endif

//------- Define constant ------------//

#define DEFAULT_LINE_SPACE  2   // 2 pixels space between lines

// ###### patch begin Gilbert 20/2 ########//
//#if( defined(FRENCH) || defined(SPANISH) )
	// change to 80-8f in french
//	#define FIRST_NATION_COLOR_CODE_IN_TEXT  0x80
//	#define LAST_NATION_COLOR_CODE_IN_TEXT   0x8f
//#else
	// english and german uses f0-ff
//	#define FIRST_NATION_COLOR_CODE_IN_TEXT  0xF0	// 0xF0 to 0xFF are nation color code, must be this range as other ranges are used by foreign characters
//	#define LAST_NATION_COLOR_CODE_IN_TEXT   0xFF
//#endif
#define FIRST_NATION_COLOR_CODE_IN_TEXT  0x80
#define LAST_NATION_COLOR_CODE_IN_TEXT   0x8f
// ###### end begin Gilbert 20/2 ########//

#define NATION_COLOR_BAR_WIDTH 			  13
#define NATION_COLOR_BAR_HEIGHT			  13

//------ Define struct HyperField ------//

#define MAX_HYPER_FIELD 30      // Maximum no. of hyper-text fields in a single page

struct HyperField
{
	short x1, y1, x2, y2;
	const char* text_ptr;        // pointer to the hyper-field name in the text
	short text_len;        // length of the hyper-field name
};

//--------- Define class Font ----------//

struct FontInfo;

class Font
{
public:
	char	  init_flag;

	const char*   next_text_ptr;      // these 3 vars are used for storing
	short   next_text_y;        // the result parameters after calling
	short   line_count;         // put_paragraph(). Refer to put_paragraph()

	short   font_height;        // height of a character
	short	  std_font_height;
	short	  max_font_height;
	short   max_font_width;     // width of the widest character in the font
	short   space_width;        // width of the space character
	short   inter_char_space;   // inter-character space

	unsigned short first_char;         // the starting available character
	unsigned short last_char;           // the ending available character

	FontInfo* font_info_array;
	char* 	 font_bitmap_buf;        // pointer to the buffer of the font

public:
	static short      hyper_field_count;
	static HyperField hyper_field_array[MAX_HYPER_FIELD];

//---------------------------------------//

public:
	Font(char* =NULL);
	~Font();

	void init(const char* fontName, int interCharSpace=1, int italicShift=0);
	void deinit();

	int  height()                    { return font_height; }
	int  width()                     { return space_width; }

	void use_std_height()		{ font_height = std_font_height; }
	void use_max_height()		{ font_height = max_font_height; }

	int  text_width(const char*, int= -1, int=0);
	int  text_height(int=DEFAULT_LINE_SPACE);

	int  put(int,int,const char*,char=0,int= -1);

	int  put(int x, int y, int value, char clearBack=0, int x2= -1)
		  { return put( x, y, m.format(value), clearBack, x2 ); }

	void put_char(int,int,unsigned short);
	void right_put(int,int,char*);

	int  d3_put(int,int,const char*);
	void d3_put(int,int,int,int,const char*);
	int  center_put(int,int,int,int,const char*,char clearBack=0);

	void put_paragraph(int,int,int,int,const char*,int=DEFAULT_LINE_SPACE,int=1,char=1);

	void count_line(int x1, int y1, int x2, int y2, const char *text,
						 int lineSpace, int& totalLines, int& dispLines);

	void put_char_to_buffer(char* dest, int destPitch, int x1, int y1, unsigned short text);
	void put_to_buffer(char* dest, int destPitch, int x1, int y1, const char *text);
	void center_put_to_buffer(char* dest, int destPitch, int x1, int y1, int x2, int y2, char *text);

	short translate_german_char(short textChar);

	//----------- <int> version -------------//

	void put_field(int,int,const char*,int,int,int=1);
	void update_field(int,int,int,int,int);
	void field(int,int,const char*,int,int,int,int,int,const char* helpCode=NULL);
	int  disp(int,int,int,int,int);

	//----- <short> version, call <int> version directly -----//

	void put_field(int a,int b,const char* c,int d,short e,int f=1)
		  { put_field(a,b,c,d,(int)e,f); }

	void update_field(int a,int b,short c,int d,int e)
		  { update_field(a,b,(int)c,d,e); }

	void field(int a,int b,const char* c,int d,short e,int f,int g,int h,char* helpCode=NULL)
		  { field(a,b,c,d,(int)e,f,g,h); }

	int  disp(int a,int b,short c,int d,int e)
		  { return disp(a,b,(int)c,d,e); }

	//---- <long> version, call <int> version directly ------//

	void put_field(int a,int b,const char* c,int d,long e,int f=1)
		  { put_field(a,b,c,d,(int)e,f); }

	void update_field(int a,int b,long c,int d,int e)
		  { update_field(a,b,(int)c,d,e); }

	void field(int a,int b,const char* c,int d,long e,int f,int g,int h,char* helpCode=NULL)
		  { field(a,b,c,d,(int)e,f,g,h); }

	int  disp(int a,int b,long c,int d,int e)
		  { return disp(a,b,(int)c,d,e); }

	//------------- <double> version ---------------//

	void put_field(int,int,const char*,int,double,int=1);
	void update_field(int,int,double,int,int);
	void field(int,int,const char*,int,double,int,int,int,char* helpCode=NULL);
	int  disp(int,int,double,int,int);

	//-------------- <char*> version ---------------//

	void put_field(int,int,const char*,int,char*);
	void update_field(int,int,const char*,int);
	void field(int,int,const char*,int,const char*,int,int,const char* helpCode=NULL);
	int  disp(int,int,const char*,int);
};

extern Font font_san, font_std, font_small, font_mid, font_news;
extern Font font_hitpoint, font_bible, font_bard;

#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
extern Font font_hall;
#endif

//------------------------------------------------//

#endif
