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

//Filename    : OTUTOR2.CPP
//Description : Class Tutor


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
#include <OTUTOR.h>
#include <OINFO.h>
// ####### begin Gilbert 4/11 #######//
#include <OMUSIC.h>
// ####### end Gilbert 4/11 #######//
#include "gettext.h"


// --------- declare static funtion --------//

static void disp_scroll_bar_func(SlideVBar *scroll, int);


enum { TUTOR_MENU_X1 = 0,
		 TUTOR_MENU_Y1 = 0,
		 TUTOR_MENU_WIDTH = VGA_WIDTH,
		 TUTOR_MENU_HEIGHT = VGA_HEIGHT };

enum { SCROLL_X1 = 757,
		 SCROLL_Y1 = 309,
		 SCROLL_X2 = 770,
		 SCROLL_Y2 = 492 };

enum { BROWSE_X1 = 30,
		 BROWSE_Y1 = 292,
		 BROWSE_REC_WIDTH  = 725,
		 BROWSE_REC_HEIGHT = 44,
		 BROWSE_X2 = BROWSE_X1 + BROWSE_REC_WIDTH - 1,
		 MAX_BROWSE_DISP_REC = 5 };

enum { TEXT_AREA_X1 = 40,
		 TEXT_AREA_Y1 = 198,
		 TEXT_AREA_X2 = 768,
		 TEXT_AREA_Y2 = 260,
		 TEXT_AREA_WIDTH = TEXT_AREA_X2 - TEXT_AREA_X1 + 1,
		 TEXT_AREA_HEIGHT = TEXT_AREA_Y2 - TEXT_AREA_Y1 + 1,
};

enum { TEXT_OFFSET_X = 11,
		 TEXT_OFFSET_Y = 9 };

#define TU_USE_BACKUP_SURFACE

#define TUOPTION_BROWSE(s)     (1 << (s))
#define TUOPTION_ALL_BROWSE    0x0000ffff
#define TUOPTION_PAGE          0x00010000
#define TUOPTION_TEXT_AREA     0x00020000
#define TUOPTION_PIC_AREA      0x00040000
#define TUOPTION_SCROLL        0x00080000
#define TUOPTION_ALL           0xffffffff


// actionMode = -2 save to back buffer
//              -1 restore from back buffer
// 
// return 0 for cancelled
//       -1 for failure
//       >0 recno selected
int Tutor::select_tutor(int actionMode)
{
	if( actionMode == -2 || actionMode == -1)
	{
		// copy or restore screen to back buffer
		int scrnX1, scrnY1, scrnX2, scrnY2;
		scrnX1 = TUTOR_MENU_X1;
		scrnY1 = TUTOR_MENU_Y1;
		scrnX2 = scrnX1 + TUTOR_MENU_WIDTH-1;
		scrnY2 = scrnY1 + TUTOR_MENU_HEIGHT-1;

		mouse.hide_area( scrnX1, scrnY1, scrnX2, scrnY2);

		if( actionMode == -2 )
		{
			info.save_game_scr();
			// save to back buffer
			IMGcopy(vga_back.buf_ptr(), vga_back.buf_pitch(),
				vga_front.buf_ptr(), vga_front.buf_pitch(),
				scrnX1, scrnY1, scrnX2, scrnY2);
		}
		else
		{
			// restore from back buffer
			IMGcopy(vga_front.buf_ptr(), vga_front.buf_pitch(),
				vga_back.buf_ptr(), vga_back.buf_pitch(),
				scrnX1, scrnY1, scrnX2, scrnY2);
			info.rest_game_scr();
		}

		mouse.show_area();

		return 1;
	}

	if( tutor_count==0 )
	{
		box.msg( _("Tutorial files not found.") );
		return 0;
	}

	//-------------------------------------//

	// ##### begin Gilbert 4/11 ########//
	// stop any music
	music.stop();
	// ##### end Gilbert 4/11 ########//

	int menuX1 = TUTOR_MENU_X1;
	int menuY1 = TUTOR_MENU_Y1;

	// int x=menuX1, y=menuY1+17;

	// vga_back.adjust_brightness( x, y, x+menuX1-1, y+menuY1-1, -6 );
	// vga_util.blt_buf( x, y, x+menuX1-1, y+menuY1-1, 0 );

	mouse_cursor.set_icon(CURSOR_NORMAL);

	power.win_opened = 1;

	int minRecno = 1;
	int browseRecno = minRecno;
	// ###### begin Gilbert 29/9 #######//
	// if called during the game, set the current tutorial selected
	if( cur_tutor_id >= 1 && cur_tutor_id <= tutor_count )
	{
		browseRecno = cur_tutor_id;
	}
	// ###### end Gilbert 29/9 #######//

	//--------------------------------------//
	Button3D scrollUp, scrollDown, startButton, cancelButton;
	int retFlag = 0;
	int refreshFlag = TUOPTION_ALL;

	scrollUp.create(menuX1+SCROLL_X1,menuY1+SCROLL_Y1-17, "SV-UP-U", "SV-UP-D", 1, 0);
	scrollDown.create(menuX1+SCROLL_X1,menuY1+SCROLL_Y2+1, "SV-DW-U", "SV-DW-D", 1, 0);
	startButton.create(menuX1+170, menuY1+529, "START-U", "START-D",1, 0);
	cancelButton.create(menuX1+465, menuY1+529, "CANCEL-U", "CANCEL-D", 1, 0);

	SlideVBar scrollBar;
	scrollBar.init_scroll(menuX1+SCROLL_X1, menuY1+SCROLL_Y1, menuX1+SCROLL_X2, menuY1+SCROLL_Y2,
		MAX_BROWSE_DISP_REC, disp_scroll_bar_func);
	scrollBar.set(minRecno, tutor_count, minRecno);

	// try to centre the selected record on the browser
//	int newBrowseTopRecno = browseRecno - MAX_BROWSE_DISP_REC/2;
//	if( newBrowseTopRecno > scrollBar.max_view_recno() )
//		newBrowseTopRecno = scrollBar.max_view_recno();
//	if( newBrowseTopRecno < scrollBar.min_recno )
///		newBrowseTopRecno = scrollBar.min_recno;
	scrollBar.set_view_recno(browseRecno - MAX_BROWSE_DISP_REC/2);

#ifdef TU_USE_BACKUP_SURFACE
	// create temporary surface
	Blob browseArea[MAX_BROWSE_DISP_REC];
	Blob scrollArea;
	Blob textArea;
#endif

	while(1)
	{
		//---------- yield --------//

		sys.yield();

		mouse.get_event();

		// --------- display ----------//

		if( refreshFlag )
		{
#ifndef TU_USE_BACKUP_SURFACE
			refreshFlag = TUOPTION_ALL;
#endif

			if( refreshFlag & TUOPTION_PAGE )
			{
				mouse.hide_area(menuX1, menuY1, menuX1+TUTOR_MENU_WIDTH, menuY1+TUTOR_MENU_HEIGHT);

				image_interface.put_front( menuX1, menuY1, "TUTORIAL" );
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
#endif
				scrollUp.paint();
				scrollDown.paint();
				startButton.paint();
				cancelButton.paint();

				mouse.show_area();
			}

			if( refreshFlag & TUOPTION_PIC_AREA )
			{
				if( browseRecno && image_tutorial.get_index(this->operator[](browseRecno)->code) )
				{
					image_tutorial.put_large(&vga_front, 21,19, this->operator[](browseRecno)->code);
				}
				else
				{
					// draw the background
#ifdef TU_USE_BACKUP_SURFACE
					// copy from ?
#endif
				}
			}

			if( refreshFlag & TUOPTION_TEXT_AREA )
			{
#ifdef TU_USE_BACKUP_SURFACE
				// copy from back buffer
				vga_front.put_bitmap(menuX1+TEXT_AREA_X1, menuY1+TEXT_AREA_Y1, 
					textArea.ptr);
#endif
				if( browseRecno ) 
				{
					#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
						int lineSpace = 2;
					#else
						int lineSpace = 4;
					#endif

					font_std.put_paragraph(menuX1+TEXT_AREA_X1, menuY1+TEXT_AREA_Y1, menuX1+TEXT_AREA_X2, menuY1+TEXT_AREA_Y2,
						get_intro(browseRecno), lineSpace );		// 4 - space between lines
				}
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
						if( rec >= 1 && rec <= tutor_count )
						{
							int textX = font_bible.put(browseSlotX1+TEXT_OFFSET_X,
								browseSlotY1+TEXT_OFFSET_Y, misc.format(rec), 0, browseSlotX2 );
							textX = font_bible.put(textX, browseSlotY1+TEXT_OFFSET_Y,
								". ", 0, browseSlotX2 );
							textX = font_bible.put(textX, browseSlotY1+TEXT_OFFSET_Y,
								this->operator[](rec)->des, 0, browseSlotX2 );

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
			//if( scrollBar.view_recno > minRecno )
			//{
			//	scrollBar.set_view_recno(oldValue-1);
			//	refreshFlag = 1;
			//}
			if( oldValue != scrollBar.set_view_recno(oldValue-1) )
				refreshFlag |= TUOPTION_ALL_BROWSE | TUOPTION_SCROLL;
		}
		else if( scrollDown.detect() )
		{
			// click on scroll down
			int oldValue = scrollBar.view_recno;
			//if( scrollBar.view_recno+scrollBar.view_size <= tutor_count )
			//{
			//	scrollBar.set_view_recno(oldValue+1);
			//	refreshFlag = 1;
			//}
			if( oldValue != scrollBar.set_view_recno(oldValue+1) )
				refreshFlag |= TUOPTION_ALL_BROWSE | TUOPTION_SCROLL;
		}
		else if( mouse.double_click( menuX1+BROWSE_X1, menuY1+BROWSE_Y1, 
			menuX1+BROWSE_X1+BROWSE_REC_WIDTH-1, 
			menuY1+BROWSE_Y1+ BROWSE_REC_HEIGHT*MAX_BROWSE_DISP_REC -1) )
		{
			// double click on game slot
			int oldValue = browseRecno;
			int newValue = scrollBar.view_recno + (mouse.click_y(0) - BROWSE_Y1 - menuY1) / BROWSE_REC_HEIGHT;
			if( newValue <= tutor_count && newValue == oldValue)
			{
				browseRecno = newValue;
				retFlag = newValue;
				break;
			}
		}
		else if( mouse.single_click( menuX1+BROWSE_X1, menuY1+BROWSE_Y1, 
			menuX1+BROWSE_X1+BROWSE_REC_WIDTH-1, 
			menuY1+BROWSE_Y1+ BROWSE_REC_HEIGHT*MAX_BROWSE_DISP_REC -1) )
		{
			// click on game slot
			int oldValue = browseRecno;
			int newValue = scrollBar.view_recno + (mouse.click_y(0) - BROWSE_Y1 - menuY1) / BROWSE_REC_HEIGHT;
			// ##### begin Gilbert 31/10 ########//
			if( newValue <= tutor_count )
			{
				if( oldValue != newValue )
				{
					browseRecno = newValue;
					refreshFlag |= TUOPTION_BROWSE(newValue-scrollBar.view_recno)
						| TUOPTION_TEXT_AREA | TUOPTION_PIC_AREA;
					if( oldValue-scrollBar.view_recno >= 0 && oldValue-scrollBar.view_recno < MAX_BROWSE_DISP_REC )
						refreshFlag |= TUOPTION_BROWSE(oldValue-scrollBar.view_recno);
				}
			}
			// ##### end Gilbert 31/10 ########//
		}
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
