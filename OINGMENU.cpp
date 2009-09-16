// Filename    : OINGMENU.H
// Description : in-game menu (async version)

#include <OVGA.h>
#include <OVGABUF.h>
#include <OSYS.h>
#include <OIMGRES.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <KEY.h>
#include <OPOWER.h>
#include <OBOX.h>
#include <OREMOTE.h>
#include <OTUTOR.h>
#include <ONATIONA.h>
#include <OWORLDMT.h>
#include <OGAME.h>
#include <OOPTMENU.h>
#include <OINGMENU.h>
#include <OFONT.h>
#include <OMUSIC.h>



#ifdef AMPLUS
enum { GAME_MENU_WIDTH  = 350,
       GAME_MENU_HEIGHT = 400  };
#else
enum { GAME_MENU_WIDTH  = 350,
       GAME_MENU_HEIGHT = 386  };
#endif

enum { GAME_MENU_X1 = ZOOM_X1 + ( (ZOOM_X2-ZOOM_X1+1) - GAME_MENU_WIDTH ) / 2,
       GAME_MENU_Y1 = ZOOM_Y1 + ( (ZOOM_Y2-ZOOM_Y1+1) - GAME_MENU_HEIGHT ) / 2 };

enum { GAME_OPTION_WIDTH  = 170,
       GAME_OPTION_HEIGHT = 34   };

// ####### begin Gilbert 29/10 #########//
#ifdef AMPLUS
enum { GAME_OPTION_X1 = GAME_MENU_X1+90,
       GAME_OPTION_Y1 = GAME_MENU_Y1+93  };

enum { MAP_ID_X1 = GAME_MENU_X1 + 18,
       MAP_ID_Y1 = GAME_MENU_Y1 + 362,
       MAP_ID_X2 = GAME_MENU_X1 + 330,
       MAP_ID_Y2 = GAME_MENU_Y1 + 382 };
#else
enum { GAME_OPTION_X1 = GAME_MENU_X1+90,
       GAME_OPTION_Y1 = GAME_MENU_Y1+76  };

enum { MAP_ID_X1 = GAME_MENU_X1 + 18,
       MAP_ID_Y1 = GAME_MENU_Y1 + 350,
       MAP_ID_X2 = GAME_MENU_X1 + 330,
       MAP_ID_Y2 = GAME_MENU_Y1 + 373 };
#endif
// ####### end Gilbert 29/10 #########//

unsigned InGameMenu::menu_hot_key[GAME_OPTION_COUNT] = {'o','s','l', 0,0,0,0,KEY_ESC };

InGameMenu::InGameMenu()
{
   active_flag = 0;
}


void InGameMenu::enter(char untilExitFlag)
{
   if( active_flag )
      return;

   refresh_flag = 1;
   active_flag = 1;

   memset(game_menu_option_flag, 1, sizeof(game_menu_option_flag) );

   if( !nation_array.player_recno)
   {
      // when in observe mode
      game_menu_option_flag[1] = 0;    // disable save game
      game_menu_option_flag[4] = 0;    // disable retire
   }

   if( remote.is_enable() )
   {
      // when in when in multi-player mode,
      game_menu_option_flag[2] = 0;    // disable load game
      game_menu_option_flag[3] = 0;    // disable training
      game_menu_option_flag[4] = 0;    // disable retire
   }

   mouse_cursor.set_icon(CURSOR_NORMAL);

   power.win_opened = 1;

   if( untilExitFlag )
   {
      while( is_active() )
      {
         sys.yield();
         mouse.get_event();

         // display on front buffer
         char useBackBuf = vga.use_back_buf;
         vga.use_front();
         disp();
         if(useBackBuf)
            vga.use_back();

         sys.blt_virtual_buf();
         music.yield();
         detect();
      }
   }
}

void InGameMenu::disp(int needRepaint)
{
   if( !active_flag )
      return;

   // since use back buffer, always refresh
   if( Vga::use_back_buf || needRepaint )
      refresh_flag = 1;

   if( refresh_flag )
   {
      int x=GAME_MENU_X1+20, y=GAME_MENU_Y1+17;

      if( Vga::use_back_buf )
         image_interface.put_back( GAME_MENU_X1, GAME_MENU_Y1, "GAMEMENU" );
      else
         image_interface.put_front( GAME_MENU_X1, GAME_MENU_Y1, "GAMEMENU" );


      for( int b = 0; b < GAME_OPTION_COUNT; ++b)
      {
         if( !game_menu_option_flag[b])
         {
            // darked disabled button
            Vga::active_buf->adjust_brightness(
               GAME_OPTION_X1, GAME_OPTION_Y1 + b*GAME_OPTION_HEIGHT,
               GAME_OPTION_X1+GAME_OPTION_WIDTH-1,
               GAME_OPTION_Y1 + (b+1)*GAME_OPTION_HEIGHT-1, -8);
         }
      }

		String str;
		#if(defined(SPANISH))
			str = "I.D del Mapa: ";
		#elif(defined(FRENCH) )
			str = "Nº de la carte: ";
		#elif(defined(GERMAN))
			str = "Karten-I.D.: ";
		#else
			str = "Map I.D.: ";
		#endif

      str += info.random_seed;
      font_bible.center_put( MAP_ID_X1, MAP_ID_Y1, MAP_ID_X2, MAP_ID_Y2, str);

      refresh_flag = 0;
   }
}


int InGameMenu::detect()
{
   if( !active_flag )
      return 0;

   int i, y=GAME_OPTION_Y1, x2, y2;

   for( i=1 ; i<=GAME_OPTION_COUNT ; i++, y+=GAME_OPTION_HEIGHT )
   {
      x2 = GAME_OPTION_X1+GAME_OPTION_WIDTH-1;
      y2 = y+GAME_OPTION_HEIGHT-1;

      if( game_menu_option_flag[i-1] == 1 &&
         (menu_hot_key[i-1] && mouse.key_code == menu_hot_key[i-1] ||
         mouse.single_click( GAME_OPTION_X1, y, x2, y2 )) )
         break;

      if( i == GAME_OPTION_COUNT &&    // assume last option is 'continue'
         (mouse.any_click(1) || mouse.key_code==KEY_ESC) )
         break;
   }

   if( i>GAME_OPTION_COUNT )
      return 0;

   //------ display the pressed down button -----//

   vga_front.save_area_common_buf( GAME_OPTION_X1, y, x2, y2 );

   image_interface.put_front( GAME_OPTION_X1, y, "MENU-DWN" );

   while( mouse.left_press )  // holding down the button
   {
      sys.yield();
      mouse.get_event();
   }

   vga_front.rest_area_common_buf();         // restore the up button

   //--------- run the option -------//

   exit(0);

   switch(i)
   {
      case 1:     // options
         option_menu.enter(!remote.is_enable());
         break;

      case 2:     // save game
         sys.save_game();
         break;

      case 3:     // load game
         sys.load_game();
         break;

      case 4:     // training
         tutor.select_run_tutor(1);
         break;

      case 5:     // retire
         if( nation_array.player_recno )     // only when the player's kingdom still exists
         {
            if( box.ask("Do you really want to retire?", "Yes", "No", 175, 320) )
            {
               if( remote.is_enable() )
               {
                  // BUGHERE : message will not be sent out
                  short *shortPtr = (short *)remote.new_send_queue_msg( MSG_PLAYER_QUIT, 2*sizeof(short));
                  shortPtr[0] = nation_array.player_recno;
                  shortPtr[1] = 1;     // retire
               }
               game.game_end(0, 0, 0, 1);          // 1 - retire
            }
         }
         break;

      case 6:     // quit to main menu
      {
         int boxX1;

         #ifdef GERMAN
            boxX1 = 125;
         #else
            boxX1 = 115;
         #endif

         if( !nation_array.player_recno ||
             box.ask( "Do you really want to quit to the Main Menu?", "Yes", "No", boxX1, 350 ) )
         {
            if( remote.is_enable() && nation_array.player_recno )
            {
               // BUGHERE : message will not be sent out
               short *shortPtr = (short *)remote.new_send_queue_msg( MSG_PLAYER_QUIT, 2*sizeof(short));
               shortPtr[0] = nation_array.player_recno;
               shortPtr[1] = 0;     // not retire
            }
            sys.signal_exit_flag = 2;
         }
         break;
      }

      case 7:     // quit to Windows
         if( !nation_array.player_recno ||
             box.ask( "Do you really want to quit to Windows?", "Yes", "No", 130, 400 ) )
         {
            if( remote.is_enable() && nation_array.player_recno )
            {
               // BUGHERE : message will not be sent out
               short *shortPtr = (short *)remote.new_send_queue_msg( MSG_PLAYER_QUIT, 2*sizeof(short));
               shortPtr[0] = nation_array.player_recno;
               shortPtr[1] = 1;     // retire
            }
            sys.signal_exit_flag = 1;
         }
         break;
   }

   return 1;
}


void InGameMenu::exit(int action)
{
   power.win_opened = 0;
   active_flag = 0;
}


void InGameMenu::abort()
{
   power.win_opened = 0;
   active_flag = 0;
}
