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

//Filename    : OHELP.H
//Description : Header file of object Help

#ifndef __OHELP_H
#define __OHELP_H


//----------- Define constant -------------//

enum { NO_HELP, BRIEF_HELP, DETAIL_HELP };	 	// for config.help_mode  

enum { MAX_HELP_INFO=200 };     // Maximum no. of help info allowed

//------------------------------------------//
//
// Structure of the HELP.RES :
//
// <Area X1>, <Area Y1>, <Area X2>, <Area Y2>
// <Help Title>
// <Help Text>
//
// <Page Break Code, CTRL-L>
//
// <Area X1>, <Area Y1>, <Area X2>, <Area Y2>
// <Help Title>
// <Help Text>
//      .
//		  .
// (Another help block, continue)
//
//------------------------------------------//

//---------- Define struct HelpInfo ------------//

struct HelpInfo
{
	enum { HELP_CODE_LEN=8, HELP_TITLE_LEN=60 };

	char  help_code[HELP_CODE_LEN+1];			// either identify the help message by help code or area position
	short area_x1, area_y1, area_x2, area_y2;

	char  help_title[HELP_TITLE_LEN+1];

	char* help_text_ptr;          // offset of the help text in the text buffer
	short help_text_len;          // length of the help text
};

//-----------------------------------------------//

class Help
{
public:
	enum 			{ SCREEN_CODE_LEN=8 };
	enum			{ CUSTOM_HELP_TITLE_LEN=80, CUSTOM_HELP_DETAIL_LEN=200 };

	char			custom_help_title[CUSTOM_HELP_TITLE_LEN+1];
	char			custom_help_detail[CUSTOM_HELP_DETAIL_LEN+1];

	char 			help_code[HelpInfo::HELP_CODE_LEN+1];
	short			help_x, help_y;

	short       help_info_count;

	HelpInfo    *help_info_array;
	char        *help_text_buf;
	int			help_text_buf_size;

	char        *save_scr_buf;      // have a private save screen buffer for displaying the helpial text
	short       save_scr_x1, save_scr_y1, save_scr_x2, save_scr_y2;

	short			last_mouse_x;
	short			last_mouse_y;
	int			mouse_still_time;

public:
	Help();
	~Help();

	void       init(char* resName);
	void       deinit();

	void       load(char*);

	void 		  push_screen(char* screenCode);
	void		  pop_screen();
	void		  set_screen(char* screenCode);

	void		  set_help(int x1, int y1, int x2, int y2, const char* helpCode);
	void		  set_unit_help(int unitId, int rankId, int x1, int y1, int x2, int y2);
	void		  set_custom_help(int x1, int y1, int x2, int y2, char* helpTitle, char* helpDetail=NULL);

	int		  should_disp();
	void       disp();
	void 		  disp_help(int centerX, int centerY, char* helpTitle, char* helpDetail);

	void       save_scr(int,int,int,int);   // save the screen to the save screen buffer in Help
	void       rest_scr();
};

//-----------------------------------------------//

extern Help help;

#endif
