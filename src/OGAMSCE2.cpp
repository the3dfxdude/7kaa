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

// Filename    : OGAMSCE2.CPP
// Description : select scenario menu


#include <OSYS.h>
#include <OGAME.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OVGABUF.h>
#include <OIMGRES.h>
#include <OBUTT3D.h>
#include <OSLIDCUS.h>
#include <OBOX.h>
#include <OPOWER.h>
#include <OFONT.h>
#include <KEY.h>
#include <OBLOB.h>
#include <OINFO.h>
#include <OFILETXT.h>
#include <OVQUEUE.h>
#include <OGETA.h>
#include <PlayerStats.h>
#include <OGFILE.h>
// ####### begin Gilbert 4/11 #######//
#include <OMUSIC.h>
// ####### end Gilbert 4/11 #######//
#include "gettext.h"

// --------- declare static funtion --------//

static void disp_scroll_bar_func(SlideVBar *scroll, int);
enum CHECKBOX_STATE { UNCHECKED = 0, PART_CHECKED = 1, CHECKED = 2 };
static void draw_checkbox(int x, int y, CHECKBOX_STATE checked);

enum { TUTOR_MENU_X1 = 0,
		 TUTOR_MENU_Y1 = 0,
		 TUTOR_MENU_WIDTH = VGA_WIDTH,
		 TUTOR_MENU_HEIGHT = VGA_HEIGHT };

enum { SCROLL_X1 = 757,
		 SCROLL_Y1 = 352,
		 SCROLL_X2 = 770,
		 SCROLL_Y2 = 492 };

enum { BROWSE_X1 = 30,
		 BROWSE_Y1 = 336,
		 BROWSE_REC_WIDTH  = 725,
		 BROWSE_REC_HEIGHT = 44,
		 BROWSE_X2 = BROWSE_X1 + BROWSE_REC_WIDTH - 1,
		 MAX_BROWSE_DISP_REC = 4 };

enum { TEXT_AREA_X1 = 43,
		 TEXT_AREA_Y1 = 198,
		 TEXT_AREA_X2 = 745,
		 TEXT_AREA_Y2 = 302,
		 TEXT_AREA_WIDTH = TEXT_AREA_X2 - TEXT_AREA_X1 + 1,
		 TEXT_AREA_HEIGHT = TEXT_AREA_Y2 - TEXT_AREA_Y1 + 1,
};

enum { TEXT_SCROLL_X1 = SCROLL_X1,
	    TEXT_SCROLL_Y1 = 202,
		 TEXT_SCROLL_X2 = SCROLL_X2,
		 TEXT_SCROLL_Y2 = 289,
};

enum { TEXT_OFFSET_X = 11,
		 TEXT_OFFSET_Y = 9 };

// ####### begin Gilbert 1/11 #########//
enum { 
		 NAME_FIELD_X1 = 180,
		 NAME_FIELD_Y1 = 551,
		 NAME_FIELD_X2 = 351,
//		 NAME_FIELD_Y2 = 566,
};
// ####### end Gilbert 1/11 #########//


#define TU_USE_BACKUP_SURFACE

#define TUOPTION_BROWSE(s)     (1 << (s))
#define TUOPTION_ALL_BROWSE    0x0000ffff
#define TUOPTION_PAGE          0x00010000
#define TUOPTION_TEXT_AREA     0x00020000
#define TUOPTION_PIC_AREA      0x00040000
#define TUOPTION_SCROLL        0x00080000
#define TUOPTION_TEXT_SCROLL   0x00100000
#define TUOPTION_TEXT_BUFFER   0x00200000
// ##### begin Gilbert 1/11 ########//
#define TUOPTION_NAME_FIELD    0x00400000
// ##### end Gilbert 1/11 ########//
#define TUOPTION_ALL           0xffffffff

//---------- Begin of function draw_checkbox ----------//
//
// Though we check for PlayStatus here, this is a very general
// purpose function and can be used anywhere with ints instead.
//
void draw_checkbox(int x, int y, CHECKBOX_STATE checked)
{
	// UNPLAYED  = Empty checkbox
	// PLAYED    = Line in checkbox
	// COMPLETED = Checked

	if (checked == CHECKBOX_STATE::CHECKED) {
		image_menu.put_front(x, y, "NMPG-RCH");             // Checked checkbox
	}
	else                                                    // Unchecked checkbox
	{
		//
		// We need an empty checkbox to start. It stays that way for an
		// UNCHECKED box. We'll draw a line inside for PART_CHECKED.
		//

		//---- draw a pseudo-3D empty checkbox ----//
		vga_front.rect(x, y, x + 14, y + 14, 10, V_WHITE); // white background
		vga_front.line(x, y, x + 14, y, V_WHITE - 8);      // top
		vga_front.line(x, y, x, y + 14, V_WHITE - 8);      // left
		x += 1;
		y += 1;
		vga_front.line(x, y, x + 13, y, V_WHITE - 6);	   // inner shadow top
		vga_front.line(x, y, x, y + 13, V_WHITE - 6);	   // inner shadow left
		x += 1;
		y += 1;
		vga_front.rect(x, y, x + 12, y + 12, 1, V_WHITE - 1);  // another inner shadow

		if (checked == CHECKBOX_STATE::PART_CHECKED)   // Line in the middle
		{
			vga_front.line(x + 1, y + 4, x + 11, y + 4, VGA_GRAY + 4);  // Dark gray line
			vga_front.line(x + 1, y + 5, x + 11, y + 5, VGA_GRAY + 4);  // Dark gray line
			vga_front.line(x + 1, y + 6, x + 11, y + 6, VGA_GRAY + 4);  // Dark gray line
			vga_front.line(x + 2, y + 7, x + 12, y + 7, V_WHITE - 8);   // slight shadow bottom-right
		}
	}
}
//---------- End of function draw_checkbox ----------//

//---------- Begin of function Game::select_scenario ----------//
//
// Select a scenario. 
//
// <int>    scenCount		   - no. of available scenarios. 
// <char**> scenFileNameArray - an array of <char*> pointing to the file name of the
//										  available scenarios. 
//
// return : <int> >0 - id. of the scenario selected. 
//                 0 - cancel
//
int Game::select_scenario(int scenCount, ScenInfo* scenInfoArray)
{
	char* scenFileName;
	char	pictName[20];
	char	textName[20];
	char* pathName;

	if( scenCount==0 )
	{
		box.msg( _("Scenario files not found.") );
		return 0;
	}

	//-------------------------------------//

	// ##### begin Gilbert 4/11 ########//
	// stop any music
	music.stop();
	// ##### end Gilbert 4/11 ########//

	int menuX1 = TUTOR_MENU_X1;
	int menuY1 = TUTOR_MENU_Y1;

	mouse_cursor.set_icon(CURSOR_NORMAL);

	power.win_opened = 1;

	int minRecno = 1;
	int browseRecno = minRecno;

	//--------------------------------------//
	Button3D scrollUp, scrollDown, startButton, cancelButton;
	int retFlag = 0;
	int refreshFlag = TUOPTION_ALL;

	scrollUp.create(menuX1+SCROLL_X1,menuY1+SCROLL_Y1-17, "SV-UP-U", "SV-UP-D", 1, 0);
	scrollDown.create(menuX1+SCROLL_X1,menuY1+SCROLL_Y2+1, "SV-DW-U", "SV-DW-D", 1, 0);
	// ###### begin Gilbert 1/11 ########//
//	startButton.create(menuX1+170, menuY1+529, "START-U", "START-D",1, 0);
//	cancelButton.create(menuX1+465, menuY1+529, "CANCEL-U", "CANCEL-D", 1, 0);
	startButton.create(menuX1+373, menuY1+529, "START-U", "START-D",1, 0);
	cancelButton.create(menuX1+548, menuY1+529, "CANCEL-U", "CANCEL-D", 1, 0);
	// ###### end Gilbert 1/11 ########//

	SlideVBar scrollBar;
	scrollBar.init_scroll(menuX1+SCROLL_X1, menuY1+SCROLL_Y1, menuX1+SCROLL_X2, menuY1+SCROLL_Y2,
		MAX_BROWSE_DISP_REC, disp_scroll_bar_func);
	scrollBar.set(minRecno, scenCount, minRecno);

	scrollBar.set_view_recno(browseRecno - MAX_BROWSE_DISP_REC/2);
	Button3D textScrollUp, textScrollDown;
	textScrollUp.create(menuX1+TEXT_SCROLL_X1, menuY1+TEXT_SCROLL_Y1-17,
		"SV-UP-U", "SV-UP-D", 1, 0);
	textScrollDown.create(menuX1+TEXT_SCROLL_X1,menuY1+TEXT_SCROLL_Y2+1, 
		"SV-DW-U", "SV-DW-D", 1, 0);
	VLenQueue textBuffer;
	*(textBuffer.reserve(1)) = '\0';

	Font &textFont = font_std;
	const int TEXT_LINE_SPACE = 4;
	const int ESTIMATED_LINE_IN_TEXT_AREA = 
		(TEXT_AREA_Y2 - TEXT_AREA_Y1 + 1) / (textFont.font_height + TEXT_LINE_SPACE);

	SlideVBar textScrollBar;
	textScrollBar.init_scroll(menuX1+TEXT_SCROLL_X1, menuY1+TEXT_SCROLL_Y1, 
		menuX1+TEXT_SCROLL_X2, menuY1+TEXT_SCROLL_Y2,
		ESTIMATED_LINE_IN_TEXT_AREA, disp_scroll_bar_func);

	// ###### begin Gilbert 1/11 ########//
	GetA playerNameField;
	playerNameField.init( menuX1+NAME_FIELD_X1, menuY1+NAME_FIELD_Y1, menuX1+NAME_FIELD_X2, 
		config.player_name, HUMAN_NAME_LEN, &font_san, 0, 1);
	// ###### end Gilbert 1/11 ########//

#ifdef TU_USE_BACKUP_SURFACE
	// create temporary surface
	Blob browseArea[MAX_BROWSE_DISP_REC];
	Blob scrollArea;
	Blob textArea;
	Blob textScrollArea;
	// ##### begin Gilbert 1/11 #######//
	Blob nameFieldArea;
	// ##### end Gilbert 1/11 #######//
#endif

	while(1)
	{
		//---------- yield --------//

		sys.yield();
		vga.flip();

		mouse.get_event();

		if( sys.signal_exit_flag == 1 )
		{
			retFlag = 0;
			break;
		}

		// --------- display ----------//

		if( refreshFlag )
		{
#ifndef TU_USE_BACKUP_SURFACE
			refreshFlag = TUOPTION_ALL;
#endif
			scenFileName = scenInfoArray[browseRecno-1].file_name;
			misc.change_file_ext( pictName, scenFileName, "SCP" );
			misc.change_file_ext( textName, scenFileName, "SCT" );
			pathName = DIR_SCENARIO_PATH(scenInfoArray[browseRecno-1].dir_id);
			err_when( ! *pathName );
			if( refreshFlag & TUOPTION_PAGE )
			{
				mouse.hide_area(menuX1, menuY1, menuX1+TUTOR_MENU_WIDTH, menuY1+TUTOR_MENU_HEIGHT);

				image_interface.put_front( menuX1, menuY1, "SCENARIO" );
#ifdef TU_USE_BACKUP_SURFACE
				// capture into browseArea, scrollArea, textArea
				for( int j = 0; j < MAX_BROWSE_DISP_REC; ++j)
				{
					browseArea[j].resize(2*sizeof(short) + BROWSE_REC_WIDTH*BROWSE_REC_HEIGHT);
					vga_front.read_bitmap(
						menuX1+BROWSE_X1, menuY1+BROWSE_Y1 + j*BROWSE_REC_HEIGHT,
						menuX1+BROWSE_X2, menuY1+BROWSE_Y1 + j*BROWSE_REC_HEIGHT+BROWSE_REC_HEIGHT-1,
						browseArea[j].ptr);
				}

				scrollArea.resize(2*sizeof(short)+(SCROLL_X2-SCROLL_X1+1)*(SCROLL_Y2-SCROLL_Y1+1));
				vga_front.read_bitmap(menuX1+SCROLL_X1,menuY1+SCROLL_Y1,menuX1+SCROLL_X2,menuY1+SCROLL_Y2, scrollArea.ptr);

				textArea.resize(2*sizeof(short)+TEXT_AREA_WIDTH*TEXT_AREA_HEIGHT);
				vga_front.read_bitmap(menuX1+TEXT_AREA_X1, menuY1+TEXT_AREA_Y1,
					menuX1+TEXT_AREA_X2, menuY1+TEXT_AREA_Y2, textArea.ptr);

				textScrollArea.resize( 2*sizeof(short)+
					(TEXT_SCROLL_X2-TEXT_SCROLL_X1+1)*(TEXT_SCROLL_Y2-TEXT_SCROLL_Y1+1));
				vga_front.read_bitmap(menuX1+TEXT_SCROLL_X1, menuY1+TEXT_SCROLL_Y1,
					menuX1+TEXT_SCROLL_X2, menuY1+TEXT_SCROLL_Y2, textScrollArea.ptr);

				// ###### begin Gilbert 1/11 ########//
				nameFieldArea.resize( 2*sizeof(short) + 
					(playerNameField.x_limit-playerNameField.x+1)*playerNameField.height());
				vga_front.read_bitmap(playerNameField.x, playerNameField.y,
					playerNameField.x_limit, playerNameField.y+playerNameField.height()-1,
					nameFieldArea.ptr);
				playerNameField.back_ground_bitmap = nameFieldArea.ptr;
				// ###### end Gilbert 1/11 ########//
#endif
				scrollUp.paint();
				scrollDown.paint();
				startButton.paint();
				cancelButton.paint();

				textScrollUp.paint();
				textScrollDown.paint();

				mouse.show_area();
			}

			if( refreshFlag & TUOPTION_PIC_AREA )
			{
				String str;

				str = pathName;
				str += pictName;

				if( browseRecno && misc.is_file_exist(str) )
				{
					File pictFile;
					pictFile.file_open(str);
					vga_front.put_large_bitmap(menuX1+21,menuY1+19, &pictFile);
					pictFile.file_close();
				}
				else
				{
					// draw the background
#ifdef TU_USE_BACKUP_SURFACE
					// copy from ?
#endif
				}
			}

			if( refreshFlag & TUOPTION_TEXT_BUFFER )
			{
				// load text buffer
				String str;

				str = pathName;
				str += textName;

				if( browseRecno && misc.is_file_exist(str) )
				{
					File textFile;
					int dataSize;
					textFile.file_open(str);
					// ##### patch begin Gilbert 2/2 ####//
					dataSize = textFile.file_size();

					FileTxt fileTxt( &textFile, dataSize );  // initialize fileTxt with an existing file stream

					fileTxt.next_line();		// skip the title lines
					fileTxt.next_line();
					fileTxt.next_line();
					fileTxt.next_line();

					textBuffer.clear();
					fileTxt.read_paragraph(textBuffer.reserve(dataSize+8), dataSize);
					// ##### end begin Gilbert 2/2 ####//

			      int dispLines;    // no. of lines can be displayed on the area
			      int totalLines;   // total no. of lines of the text

					textFont.count_line( menuX1+TEXT_AREA_X1, menuY1+TEXT_AREA_Y1,
						menuX1+TEXT_AREA_X2, menuY1+TEXT_AREA_Y2,
						_(textBuffer.queue_buf), TEXT_LINE_SPACE, dispLines, totalLines );

					// textScrollBar.view_size = dispLines;
					textScrollBar.set(1, totalLines ,1);
					refreshFlag |= TUOPTION_TEXT_SCROLL;
				}
			}

			if( refreshFlag & TUOPTION_TEXT_AREA )
			{
#ifdef TU_USE_BACKUP_SURFACE
				// copy from back buffer
				vga_front.put_bitmap(menuX1+TEXT_AREA_X1, menuY1+TEXT_AREA_Y1, 
					textArea.ptr);
#endif
					textFont.put_paragraph(menuX1+TEXT_AREA_X1, menuY1+TEXT_AREA_Y1, menuX1+TEXT_AREA_X2, menuY1+TEXT_AREA_Y2,
						_(textBuffer.queue_buf), TEXT_LINE_SPACE, textScrollBar.view_recno );		// 4 - space between lines
			}

			if( refreshFlag & TUOPTION_TEXT_SCROLL )
			{
#ifdef TU_USE_BACKUP_SURFACE
				vga_front.put_bitmap(menuX1+TEXT_SCROLL_X1, menuY1+TEXT_SCROLL_Y1, 
					textScrollArea.ptr);
#endif
				// display scroll bar
				textScrollBar.paint();
			}

			if( refreshFlag & TUOPTION_SCROLL )
			{
#ifdef TU_USE_BACKUP_SURFACE
				// copy from back buffer
				vga_front.put_bitmap(menuX1+SCROLL_X1, menuY1+SCROLL_Y1, 
					scrollArea.ptr);
#endif
				// display scroll bar
				scrollBar.paint();
			}

			if( refreshFlag & TUOPTION_ALL_BROWSE )
			{
				int rec, slot;
				for( slot = 0; slot < scrollBar.view_size; ++slot)
				{
					int browseSlotX1 = menuX1+BROWSE_X1;
					int browseSlotY1 = menuY1+BROWSE_Y1+slot*BROWSE_REC_HEIGHT;
					int browseSlotX2 = menuX1+BROWSE_X2;
					int browseSlotY2 = menuY1+BROWSE_Y1+(slot+1)*BROWSE_REC_HEIGHT-1;

					rec = scrollBar.view_recno + slot;
					if( refreshFlag & TUOPTION_BROWSE(slot) )
					{
#ifdef TU_USE_BACKUP_SURFACE
						vga_front.put_bitmap(browseSlotX1, browseSlotY1,
							browseArea[rec%MAX_BROWSE_DISP_REC].ptr);
#endif
						if( rec >= 1 && rec <= scenCount )
						{
							int textX = font_bible.put(browseSlotX1+TEXT_OFFSET_X,
								browseSlotY1+TEXT_OFFSET_Y, misc.format(rec), 0, browseSlotX2 );

							//----- display the scenario name -----//

							textX = font_bible.put(textX, browseSlotY1+TEXT_OFFSET_Y,
								". ", 0, browseSlotX2 );

							textX = font_bible.put(textX, browseSlotY1+TEXT_OFFSET_Y,
								_(scenInfoArray[rec-1].scen_name), 0, browseSlotX2 );

							//---- display the scenario's play status ----//

							draw_checkbox(browseSlotX1 + TEXT_OFFSET_X + 675,
										  browseSlotY1 + TEXT_OFFSET_Y + 5,
										  static_cast<CHECKBOX_STATE>(scenInfoArray[rec - 1].play_status));

							//---- display the scenario difficulty and bonus points ----//

							String str(_("Difficulty : "));
							str += scenInfoArray[rec-1].goal_difficulty;

							font_bible.put(browseSlotX1+TEXT_OFFSET_X+400, browseSlotY1+TEXT_OFFSET_Y,
												str, 0, browseSlotX2 );

							str  = _("Score Bonus: ");
							str += scenInfoArray[rec-1].goal_score_bonus;

							font_bible.put(browseSlotX1+TEXT_OFFSET_X+530, browseSlotY1+TEXT_OFFSET_Y,
												str, 0, browseSlotX2 );

							//--------------------------------------//

							if( rec == browseRecno )
							{
								vga_front.adjust_brightness(browseSlotX1, browseSlotY1, browseSlotX2, browseSlotY2, -2);

								//vga_front.put_bitmap_trans_decompress( menuX1+BROWSE_X1, menuY1+BROWSE_Y1+slot*BROWSE_REC_HEIGHT,
								//	image_button.read("LS-DWN"));
							}
						}
					}
				}
			}

			// ###### begin Gilbert 1/11 #######//
			if( refreshFlag & TUOPTION_NAME_FIELD )
				playerNameField.paint();
			// ###### end Gilbert 1/11 #######//

			refreshFlag = 0;
		}

		sys.blt_virtual_buf();

		if( scrollBar.detect() == 1)
		{
			refreshFlag |= TUOPTION_SCROLL | TUOPTION_ALL_BROWSE;
		}
		else if( scrollUp.detect() )
		{
			// click on scroll up
			int oldValue = scrollBar.view_recno;
			if( oldValue != scrollBar.set_view_recno(oldValue-1) )
				refreshFlag |= TUOPTION_ALL_BROWSE | TUOPTION_SCROLL;
		}
		else if( scrollDown.detect() )
		{
			// click on scroll down
			int oldValue = scrollBar.view_recno;
			if( oldValue != scrollBar.set_view_recno(oldValue+1) )
				refreshFlag |= TUOPTION_ALL_BROWSE | TUOPTION_SCROLL;
		}
		else if( textScrollBar.detect() == 1 )
		{
			refreshFlag |= TUOPTION_TEXT_SCROLL | TUOPTION_TEXT_AREA;
		}
		else if( textScrollUp.detect() )
		{
			// click on scroll up
			int oldValue = textScrollBar.view_recno;
			if( oldValue != textScrollBar.set_view_recno(oldValue-1) )
				refreshFlag |= TUOPTION_TEXT_SCROLL | TUOPTION_TEXT_AREA;
		}
		else if( textScrollDown.detect() )
		{
			// click on scroll down
			int oldValue = textScrollBar.view_recno;
			if( oldValue != textScrollBar.set_view_recno(oldValue+1) )
				refreshFlag |= TUOPTION_TEXT_SCROLL | TUOPTION_TEXT_AREA;
		}
		else if( mouse.double_click( menuX1+BROWSE_X1, menuY1+BROWSE_Y1, 
			menuX1+BROWSE_X1+BROWSE_REC_WIDTH-1, 
			menuY1+BROWSE_Y1+ BROWSE_REC_HEIGHT*MAX_BROWSE_DISP_REC -1) )
		{
			// double click on game slot
			int oldValue = browseRecno;
			int newValue = scrollBar.view_recno + (mouse.click_y(0) - BROWSE_Y1 - menuY1) / BROWSE_REC_HEIGHT;
			if( newValue <= scenCount && newValue == oldValue )
			{
				browseRecno = newValue;
				retFlag = newValue;
				break;
			}
		}
		else if( mouse.single_click( menuX1+BROWSE_X1, menuY1+BROWSE_Y1, 
			menuX1+BROWSE_X1+BROWSE_REC_WIDTH-1, 
			menuY1+BROWSE_Y1+ BROWSE_REC_HEIGHT*MAX_BROWSE_DISP_REC -1, 2) )
		{
			int btn;
			if (mouse.left_press) { btn = 0; }
			else if (mouse.right_press) { btn = 1; }
			// click on game slot
			int oldValue = browseRecno;
			int newValue = scrollBar.view_recno + (mouse.click_y(btn) - BROWSE_Y1 - menuY1) / BROWSE_REC_HEIGHT;
			if( newValue <= scenCount )
			{
				if( oldValue != newValue )
				{
					browseRecno = newValue;
					refreshFlag |= TUOPTION_BROWSE(newValue-scrollBar.view_recno)
						| TUOPTION_TEXT_BUFFER
						| TUOPTION_TEXT_AREA | TUOPTION_PIC_AREA;
					if( oldValue-scrollBar.view_recno >= 0 && oldValue-scrollBar.view_recno < MAX_BROWSE_DISP_REC)
						refreshFlag |= TUOPTION_BROWSE(oldValue-scrollBar.view_recno);
				}
			}

			if (mouse.right_press)
			{
				int status = scenInfoArray[browseRecno - 1].play_status;
				status++;
				if(status > nsPlayerStats::PlayStatus::COMPLETED)
				{
					status = nsPlayerStats::PlayStatus::UNPLAYED;
				}
				scenInfoArray[browseRecno - 1].play_status = status;
				int browseSlotX1 = menuX1 + BROWSE_X1;
				int browseSlotY1 = menuY1 + BROWSE_Y1 + (newValue - scrollBar.view_recno) * BROWSE_REC_HEIGHT;
				draw_checkbox(browseSlotX1 + TEXT_OFFSET_X + 675,
							  browseSlotY1 + TEXT_OFFSET_Y + 5,
							  static_cast<CHECKBOX_STATE>(status));

				// update PLAYSTAT.DAT
				String path;
				path += DIR_SCENARIO_PATH(scenInfoArray[browseRecno - 1].dir_id);;
				path += scenInfoArray[browseRecno - 1].file_name;
				playerStats.set_scenario_play_status(scenInfoArray[browseRecno - 1].file_name, static_cast<nsPlayerStats::PlayStatus>(status));
			}
		}
		// ######## begin Gilbert 1/11 #########//
		else if( playerNameField.detect() )
		{
			// load button
			refreshFlag = TUOPTION_NAME_FIELD;
		}
		// ######## end Gilbert 1/11 #########//
		else if( cancelButton.detect(KEY_ESC) || mouse.any_click(RIGHT_BUTTON) > 0)		// also when ESC key is pressed or right button
		{
			// cancel button or escape key
			refreshFlag = TUOPTION_ALL;
			retFlag = 0;
			break;		// break while(1)
		}
		else if( startButton.detect() )
		{
			// load button
			refreshFlag = TUOPTION_ALL;
			retFlag = browseRecno;
			break;
		}
	}

	power.win_opened = 0;

	return retFlag;
}
//------------ End of function Game::select_scenario -----------//


static void disp_scroll_bar_func(SlideVBar *scroll, int)
{
	short rectTop = scroll->rect_top();
	short rectBottom = scroll->rect_bottom();
	vga_front.bar( scroll->scrn_x1, rectTop, scroll->scrn_x2, rectBottom, VGA_YELLOW+1);
	if( rectBottom - rectTop > 6 )
	{
		vga_front.d3_panel_up(scroll->scrn_x1, rectTop, scroll->scrn_x2, rectBottom,2,0);
	}
}


