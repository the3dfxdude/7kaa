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

//Filename    : OHELP.CPP
//Description : Object Help

#include <ALL.h>
#include <OSYS.h>
#include <OSTR.h>
#include <OVGA.h>
#include <OMUSIC.h>
#include <OFONT.h>
#include <OINFO.h>
#include <OGAME.h>
#include <OUNIT.h>
#include <OBOX.h>
#include <OMOUSE.h>
#include <OVBROWSE.h>
#include <OFILETXT.h>
#include <OHELP.h>


//---------- Define constant -------------//

#define HELP_BOX_COLOR      (VGA_GRAY+5)
#define HELP_INACTIVE_TIME  ((float) 0.8)		// when the mouse is inactive for one second, display help

enum { HELP_SCR_BUF_WIDTH  = 400,      // 400
		 HELP_SCR_BUF_HEIGHT = 200,      // 300
		 HELP_SCR_BUF_SIZE   = HELP_SCR_BUF_WIDTH * HELP_SCR_BUF_HEIGHT };

enum { MSG_LINE_SPACE = 4 };

enum { X_MARGIN = 10,
		 Y_MARGIN = 6  };

enum { MSG_WIN_WIDTH   = 390,
		 MSG_HEAD_HEIGHT = 16   };


//------- Begin of function Help::Help ----------//

Help::Help()
{
	memset( this, 0, sizeof(Help) );

	help_info_array = (HelpInfo*) mem_add( sizeof(HelpInfo) * MAX_HELP_INFO );

	save_scr_x1 = -1;
}
//------- Begin of function Help::Help ----------//


//------- Begin of function Help::~Help ----------//

Help::~Help()
{
	deinit();
}
//------- Begin of function Help::~Help ----------//


//------- Begin of function Help::init ----------//

void Help::init(const char* resName)
{
	String str;

	str  = DIR_RES;
	str += resName;

	load( str );

	save_scr_buf = mem_add(HELP_SCR_BUF_SIZE);
}
//------- Begin of function Help::init ----------//


//------- Begin of function Help::deinit ----------//

void Help::deinit()
{
   if( save_scr_buf )
   {
      mem_del( save_scr_buf );
		save_scr_buf = NULL;
   }

   if( help_info_array )
   {
		mem_del( help_info_array );
		help_info_array = NULL;
   }

	if( help_text_buf )
	{
		mem_del( help_text_buf );
		help_text_buf = NULL;
	}
}
//------- Begin of function Help::deinit ----------//


//------- Begin of function Help::load ----------//
//
// <char*> helpFileName = name of the help file.
//
void Help::load(char* helpFileName)
{
	//------ Open the file and allocate buffer -----//

	FileTxt fileTxt( helpFileName );

	int dataSize = fileTxt.file_size();

	if( dataSize > help_text_buf_size )
	{
		help_text_buf      = mem_resize( help_text_buf, dataSize );       // allocate a buffer larger than we need for the largest size possible
		help_text_buf_size = dataSize;
	}

	//-------- read in help info one by one --------//

	HelpInfo* iptr    = help_info_array;
	char*     textPtr = help_text_buf;
	int       readLen, totalReadLen=0;    // total length of text read into the help_text_buf
	int		 loopCount=0;
	char*		 tokenStr;

	help_info_count=0;

	while( !fileTxt.is_eof() )
	{
		err_when( loopCount++ > 10000 );

		tokenStr = fileTxt.get_token(0);		// don't advance the pointer

		if( !tokenStr )
			break;

		//--------- if it's a help code ----------//

		if( tokenStr[0] >= 'A' && tokenStr[0] <= 'Z' )
		{
			strncpy( iptr->help_code, tokenStr, iptr->HELP_CODE_LEN );
			iptr->help_code[iptr->HELP_CODE_LEN] = NULL;
		}

		//------ if it's a help area position ------//

		else if( tokenStr[0] >= '0' && tokenStr[0] <= '9' )
		{
			iptr->area_x1 = (short) fileTxt.get_num();
			iptr->area_y1 = (short) fileTxt.get_num();
			iptr->area_x2 = (short) fileTxt.get_num();
			iptr->area_y2 = (short) fileTxt.get_num();
		}
		else
			err_here();

		//---------- next line -----------//

		fileTxt.next_line();

		if( fileTxt.is_eof() )
			break;

		//--------------------------------------------//
		// get help title
		//--------------------------------------------//

		fileTxt.read_line( iptr->help_title, iptr->HELP_TITLE_LEN );

		//---------------------------------------------------------//
		// get help description
		//---------------------------------------------------------//

		readLen = fileTxt.read_paragraph(textPtr, help_text_buf_size-totalReadLen);

		iptr->help_text_ptr = textPtr;
		iptr->help_text_len = readLen;

		textPtr      += readLen;
		totalReadLen += readLen;

		err_when( totalReadLen>help_text_buf_size );

		//----------- next help block -------------//

		fileTxt.next_line();      // pass the page break line

		help_info_count++;
		iptr++;

		err_when( help_info_count >= MAX_HELP_INFO );
	}
}
//--------- End of function Help::load ----------//


//---------- Begin of function Help::save_scr ---------//
//
// Save the specified porton of the screen
//
// <int> x1,y1,x2,y2 = the area of the screen
//
void Help::save_scr(int x1, int y1, int x2, int y2)
{
	if( save_scr_x1 >= 0 )		// there is already a screen saved
		return;

	err_when( x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=VGA_WIDTH || y2>=VGA_HEIGHT );

	long saveSize = (long)(x2-x1+1) * (y2-y1+1);

	err_when( saveSize > HELP_SCR_BUF_SIZE );

	save_scr_x1 = x1;
	save_scr_y1 = y1;
	save_scr_x2 = x2;
	save_scr_y2 = y2;

	mouse.hide_area( x1, y1, x2, y2 );

	vga_front.read_bitmap( x1, y1, x2, y2, save_scr_buf );

	mouse.show_area();
}
//------------ End of function Help::save_scr ---------//


//----------- Begin of function Help::rest_scr --------//
//
// Restore previously saved screen
//
void Help::rest_scr()
{
	if( save_scr_x1 < 0 )        // already restored, or not saved yet
		return;

	err_when( save_scr_x1>save_scr_x2 || save_scr_y1>save_scr_y2 ||
				 save_scr_x1<0 || save_scr_y1<0 || save_scr_x2>=VGA_WIDTH || save_scr_y2>=VGA_HEIGHT );

//	mouse.hide_area( save_scr_x1, save_scr_y1, save_scr_x2, save_scr_y2 );

	vga_front.put_bitmap( save_scr_x1, save_scr_y1, save_scr_buf );
	sys.blt_virtual_buf();

//	mouse.show_area();

	mouse.show();

	save_scr_x1 = -1;       // state that it has been restored.
}
//------------ End of function Help::rest_scr ----------//


//----------- Begin of function Help::disp --------//
//
// Display help message on the given screen location.
//
void Help::disp()
{
	//---- first check if we should disp the help now ------//

	if( !should_disp() )
	{
		help_code[0] = NULL;	// reset it everytime after displaying, if the mouse is still on the button, help_code will be set again.
		custom_help_title[0] = NULL;
		return;
	}

	//-------- button help ---------//

	if( help_code[0] )
	{
		//--------- locate the help and display it ----------//

		int i;
		HelpInfo* helpInfo = help_info_array;

		for( i=0 ; i<help_info_count ; i++, helpInfo++ )
		{
			if( helpInfo->help_code[0] == help_code[0] &&
				 strcmp( helpInfo->help_code, help_code )==0 )
			{
				disp_help( help_x, help_y,
							  helpInfo->help_title, helpInfo->help_text_ptr );
				break;
			}
		}

		help_code[0] = NULL;		// reset it everytime after displaying, if the mouse is still on the button, help_code will be set again.
	}

	//-------- custom help ---------//

	else if( custom_help_title[0] )
	{
		disp_help(help_x, help_y, custom_help_title, custom_help_detail);
		custom_help_title[0] = NULL;
	}

	//-------- other interface help ---------//

	else
	{
		//--------- locate the help and display it ----------//

		int i;
		HelpInfo* helpInfo = help_info_array;

		int spotX = mouse.cur_x;
		int spotY = mouse.cur_y;

		for( i=0 ; i<help_info_count ; i++, helpInfo++ )
		{
			if( spotX >= helpInfo->area_x1 && spotY >= helpInfo->area_y1 &&
				 spotX <= helpInfo->area_x2 && spotY <= helpInfo->area_y2 )
			{
				disp_help( (helpInfo->area_x1+helpInfo->area_x2)/2,
							  (helpInfo->area_y1+helpInfo->area_y2)/2,
							  helpInfo->help_title, helpInfo->help_text_ptr );
				break;
			}
		}
	}
}
//------------ End of function Help::disp ----------//


//---------- Begin of function Help::disp_help ----------//
//
// <int>   centerX, centerY - the center position of the help area.
// <char*> helpTitle  - title of the help
// [char*] helpDetail - detail of the help.
//
void Help::disp_help(int centerX, int centerY, char* helpTitle, char* helpDetail)
{
	if( config.help_mode == NO_HELP )
		return;

	mouse.hide();

	//------ calculate the position of the help box ------//

	int winWidth, winHeight, dispHelpDetail=0;

	if( helpDetail && helpDetail[0] &&		  // with detailed help
		 config.help_mode == DETAIL_HELP )		  // Detailed Help
	{
		winWidth  = font_san.text_width(helpDetail, -1, MSG_WIN_WIDTH-X_MARGIN*2) + X_MARGIN*2;
		winHeight = Y_MARGIN*2 + font_san.height() + 8 + font_san.text_height(MSG_LINE_SPACE);    // text_width() must be called before calling text_height()

		dispHelpDetail = 1;
	}
	else		// Help title only
	{
		winWidth  = font_san.text_width(helpTitle, -1, MSG_WIN_WIDTH-X_MARGIN*2) + X_MARGIN*2;
		winHeight = Y_MARGIN*2 + font_san.height();
	}

	//--- if the text is bigger than one text box can hold, use a scrollable text box ---//

	int x1, y1, x2, y2;

	if( winWidth * winHeight > HELP_SCR_BUF_SIZE )
	{
		x1 = MAX( 2, centerX - HELP_SCR_BUF_WIDTH  / 2 );
		y1 = MAX( 2, centerY - HELP_SCR_BUF_HEIGHT / 2 );

		x2 = x1 + HELP_SCR_BUF_WIDTH - 1;
		y2 = y1 + HELP_SCR_BUF_HEIGHT - 1;
	}
	else
	{
		x1 = MAX( 2, centerX - winWidth  / 2 );
		y1 = MAX( 2, centerY - winHeight / 2 );

		x2 = x1 + winWidth - 1;
		y2 = y1 + winHeight - 1;
	}

	if( x2 >= VGA_WIDTH )
	{
		x2 = VGA_WIDTH-10;
		x1 = x2-winWidth+1;
	}

	if( y2 >= VGA_HEIGHT )
	{
		y2 = VGA_HEIGHT-3;
		y1 = y2-winHeight+1;
	}

	//------------- save the area --------------//

	help.save_scr( x1, y1, x2, y2 ); 	// save the screen to the private buffer in Help

	//------- Draw box (and arrow if specified object) ------//

	vga_front.bar( x1, y1, x2, y2, V_WHITE );

	vga_front.bar( x1, y1, x2, y1+1, HELP_BOX_COLOR );        // Top border
	vga_front.bar( x1, y2-1, x2, y2, HELP_BOX_COLOR );        // Bottom border
	vga_front.bar( x1, y1, x1+1, y2, HELP_BOX_COLOR );        // Left border
	vga_front.bar( x2-1, y1, x2, y2, HELP_BOX_COLOR );        // Right border

	//--------- disp help detail -----------//

	font_san.put( x1+X_MARGIN, y1+Y_MARGIN, helpTitle );

	if( dispHelpDetail )
	{
		int y = y1 + Y_MARGIN + font_san.height() + 4;

		vga_front.bar( x1, y, x2, y+1, HELP_BOX_COLOR );  // line between description and help text

		font_san.put_paragraph( x1+X_MARGIN, y+4, x2-X_MARGIN, y2-Y_MARGIN, helpDetail, MSG_LINE_SPACE );
	}

	if( sys.debug_session )
		sys.blt_virtual_buf();

	//--- in a single player game, pause the game when a help message is disp_helplayed ---//

	while( help.should_disp() )
	{
		sys.yield();
		music.yield();

		mouse.get_event();
	}

	help.rest_scr();
}
//--------- End of function Help::disp_help ----------//


//--------- Begin of function Help::should_disp --------//
//
int Help::should_disp()
{
	if( config.help_mode == NO_HELP )
		return 0;

	if( VBrowse::press_record )
		return 0;

	if( last_mouse_x==mouse.cur_x && last_mouse_y==mouse.cur_y &&
		 !mouse.left_press && !mouse.right_press && !mouse.any_click(2) )
	{
		if( m.get_time() >= mouse_still_time + HELP_INACTIVE_TIME * 1000 )
		{
			return 1;
		}
	}
	else
	{
		last_mouse_x = mouse.cur_x;
		last_mouse_y = mouse.cur_y;
		mouse_still_time = m.get_time();
	}

	return 0;
}
//---------- End of function Help::should_disp ---------//


//--------- Begin of function Help::set_help --------//
//
void Help::set_help(int x1, int y1, int x2, int y2, const char* helpCode)
{
	err_when( strlen(helpCode) > 8 );

	if( !mouse.in_area(x1, y1, x2, y2) )
		return;

	strcpy( help_code, helpCode );

	help_x = (x1+x2)/2;
	help_y = (y1+y2)/2;
}
//---------- End of function Help::set_help ---------//


//--------- Begin of function Help::set_unit_help --------//
//
void Help::set_unit_help(int unitId, int rankId, int x1, int y1, int x2, int y2)
{
	if( !mouse.in_area(x1, y1, x2, y2) )
		return;

	//-------- compose the help string --------//

	static String str;

#if(defined(SPANISH) || defined(FRENCH))
	str = "";
	if( rankId==RANK_KING )
		str = translate.process("King ");
	else if( rankId==RANK_GENERAL )
		str = translate.process("General ");
	str += unit_res[unitId]->name;
#else
	str = unit_res[unitId]->name;

	#if( !defined(GERMAN) && !defined(FRENCH) && !defined(SPANISH) )		 // english
		if( rankId>=RANK_GENERAL && unitId==UNIT_MAYA )
			str += "n";			// "Mayan"
	#endif

	if( rankId==RANK_KING )
	{
		str += " ";
		str += translate.process( "King" );
	}
	else if( rankId==RANK_GENERAL )
	{
		str += " ";
		str += translate.process( "General" );
	}
#endif

	set_custom_help( x1, y1, x2, y2, str );
}
//---------- End of function Help::set_unit_help ---------//


//--------- Begin of function Help::set_custom_help --------//
//
// <int>   x1, y1, x2, y2 - the coordination of the help
// <char*> helpTitle  - the title of the help
// [char*] helpDetail - the detailed text of the help
//
void Help::set_custom_help(int x1, int y1, int x2, int y2, char* helpTitle, char* helpDetail)
{
	if( !mouse.in_area(x1, y1, x2, y2) )
		return;

	help_x = (x1+x2)/2;
	help_y = (y1+y2)/2;

	strncpy( custom_help_title, helpTitle, CUSTOM_HELP_TITLE_LEN );
	custom_help_title[CUSTOM_HELP_TITLE_LEN] = NULL;

	if( helpDetail )
	{
		strncpy( custom_help_detail, helpDetail, CUSTOM_HELP_DETAIL_LEN );
		custom_help_detail[CUSTOM_HELP_DETAIL_LEN] = NULL;
	}
	else
	{
		custom_help_detail[0] = NULL;
	}
}
//---------- End of function Help::set_custom_help ---------//


