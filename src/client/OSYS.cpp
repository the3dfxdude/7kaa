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

//Filename    : OSYS.CPP
//Description : System resource management object

#include <RESOURCE.h>
#include <stdio.h>
#include <ALL.h>
#include <OAUDIO.h>
#include <ODATE.h>
#include <OBOX.h>
#include <OFONT.h>
#include <OSTR.h>
#include <OVGA.h>
#include <OGAME.h>
#include <ONEWS.h>
#include <OTRANSL.h>
#include <OGAMESET.h>
#include <OGFILE.h>
#include <OINFO.h>
#include <OVBROWSE.h>
#include <OIMGRES.h>
#include <OMOUSE.h>
#include <OMOUSE2.h>
#include <KEY.h>
#include <OMOUSECR.h>
#include <OUNIT.h>
#include <OSITE.h>
#include <OSPATH.h>
#include <OSPATHS2.h>
#include <OSPREUSE.h>
#include <OSPY.h>
#include <OSYS.h>
#include <syswin.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OTECHRES.h>
#include <OTALKRES.h>
#include <OGODRES.h>
#include <OHELP.h>
#include <OTUTOR.h>
#include <OF_BASE.h>
#include <OTOWN.h>
#include <OBULLET.h>
#include <ONATION.h>
#include <OFLAME.h>
#include <OPOWER.h>
#include <OTERRAIN.h>
#include <OWORLD.h>
#include <OANLINE.h>
#include <OSE.h>
#include <OLOG.h>
#ifdef USE_DPLAY
#include <OERRCTRL.h>
#endif
#include <OMUSIC.h>
#include <OLZW.h>
#include <OLONGLOG.h>
#include <OVGALOCK.h>
#include <OGRPSEL.h>
#include <OCRC_STO.h>
#include <OF_HARB.h>
// ##### begin Gilbert 23/10 ######//
#include <OOPTMENU.h>
#include <OINGMENU.h>
// ##### end Gilbert 23/10 ######//

//----------- Declare static functions -----------//

static void test_lzw();

static void locate_king_general(int rankId);
static void locate_spy();
static void locate_ship();
static void locate_camp();
static int  locate_ship_in_harbor();
static int  locate_visible_ship();
static int  detect_scenario_cheat_key(unsigned scanCode, unsigned skeyState);
static int  get_mouse_loc_in_zoom_map(int &x, int &y);

//----------- Define static variables ------------//

static unsigned long last_frame_time=0, last_resend_time=0;
static char          remote_send_success_flag=1;
static char          scenario_cheat_flag=0;

//----------- Begin of function Sys::Sys -----------//

Sys::Sys()
{
   memset(this, 0, sizeof(Sys) );

   common_data_buf = mem_add( COMMON_DATA_BUF_SIZE );

   view_mode = MODE_NORMAL;         // the animation mode
}
//----------- End of function Sys::Sys -----------//


//----------- Begin of function Sys::~Sys -----------//

Sys::~Sys()
{
   mem_del(common_data_buf);

   deinit();
}
//----------- End of function Sys::~Sys -----------//


//------------ Begin of function Sys::init ----------//
//
int Sys::init()
{
   err_when( init_flag );

   //------- initialize basic vars --------//

	#ifdef BETA
		debug_session       = m.is_file_exist("DEBUG.SYS");
		testing_session     = m.is_file_exist("TESTING.SYS");
		scenario_cheat_flag = m.is_file_exist("CHEAT.SYS");
	#endif

	#ifdef DEBUG
		debug_session       = m.is_file_exist("DEBUG.SYS");
		testing_session     = m.is_file_exist("TESTING.SYS");
		scenario_cheat_flag = m.is_file_exist("CHEAT.SYS");
	#endif

//	debug_session       = m.is_file_exist("DEBUG.SYS");

	set_game_dir();      // set game directories names and game version

   //------- initialize more stuff ---------//

   if( !window.init() )
      return FALSE;

   if( !init_directx() )
      return FALSE;

   if( !init_objects() )   // initialize system objects which do not change from games to games.
      return FALSE;

   init_flag = 1;

   return TRUE;
}
//------------ End of function Sys::init ----------//


//-------- Begin of function Sys::deinit --------//
//
// Finished with all objects we use; release them
//
void Sys::deinit()
{
   if( !init_flag )
      return;

   game.deinit();    // actually game.deinit() will be called by main_win_proc() and calling it here will have no effect

   deinit_objects();

   //-------------------------------------//

   if( vga_back.buf_locked )
      vga_back.unlock_buf();

   if( vga_front.buf_locked )
      vga_front.unlock_buf();

   //-------------------------------------//

   window.deinit();

   init_flag = 0;
}
//--------- End of function Sys::deinit ---------//


//-------- Begin of function Sys::init_directx --------//
//
int Sys::init_directx()
{
   DEBUG_LOG("Attempt audio.init()");
   audio.init();
   DEBUG_LOG(audio.wav_init_flag);
   music.init();
   se_ctrl.init();

   //---------------------------------------//

   ShowCursor(FALSE);

   //-------- initialize DirectDraw --------//

   DEBUG_LOG("Attempt vga.init()");
   if( !vga.init() )
      return FALSE;
   DEBUG_LOG("vga.init() ok");

   DEBUG_LOG("Attempt vga.load_pal()");
   vga.load_pal(DIR_RES"PAL_STD.RES");
   DEBUG_LOG("vga.load_pal() finish");

   if( sys.debug_session )                // if we are currently in a debug session, don't lock the front buffer otherwise the system will hang up
   {
      DEBUG_LOG("Attempt vga_front.init_back()");
      vga.init_surface( &vga_front, VGA_BACK );
      vga_front.is_front = 1;       // set it to 1, overriding the setting in init_back()
      DEBUG_LOG("Attempt vga_true_front.init_front()");
      vga.init_surface( &vga_true_front, VGA_FRONT );
      DEBUG_LOG("Attempt vga.activate_pal()");
      vga.activate_pal(&vga_true_front);
      DEBUG_LOG("vga.activate_pal() finish");
   }
   else
   {
      vga.init_surface( &vga_front, VGA_FRONT );
      vga.activate_pal(&vga_front);
   }

   DEBUG_LOG("Attempt vga_back.init_back()");
   vga.init_surface( &vga_back, VGA_BACK );
   DEBUG_LOG("vga_back.init_back() finish");

   DEBUG_LOG("Attempt vga_front.lock_buf()");
   vga_front.lock_buf();
   DEBUG_LOG("vga_front.lock_buf() finish");

   DEBUG_LOG("Attempt vga_back.lock_buf()");
   vga_back.lock_buf();
   DEBUG_LOG("vga_back.lock_buf() finish");

   return TRUE;
}
//-------- End of function Sys::init_directx --------//


//-------- Begin of function Sys::deinit_directx --------//
//
void Sys::deinit_directx()
{
   DEBUG_LOG("Attempt vga_back.deinit()");
   vga_back.deinit();
   DEBUG_LOG("vga_back.deinit() finish");

   if( sys.debug_session )
   {
      DEBUG_LOG("Attempt vga_true_front.deinit()");
      vga_true_front.deinit();
      DEBUG_LOG("vga_true_front.deinit() finish");
   }

   DEBUG_LOG("Attempt vga_front.deinit()");
   vga_front.deinit();
   DEBUG_LOG("Attempt vga_front.deinit() finish");

   DEBUG_LOG("Attempt vga.deinit()");
   vga.deinit();
   DEBUG_LOG("vga.deinit() finish");

   //------------------------------//

   se_ctrl.deinit();
   music.deinit();
   DEBUG_LOG("Attempt audio.deinit()");
   audio.deinit();
   DEBUG_LOG("audio.deinit() finish");
}
//--------- End of function Sys::deinit_directx ---------//


//------- Begin of function Sys::init_objects -----------//
//
// Initialize system objects which do not change from games to games.
//
int Sys::init_objects()
{
   //--------- init system class ----------//

   mouse_cursor.init();
   mouse_cursor.set_frame_border(ZOOM_X1,ZOOM_Y1,ZOOM_X2,ZOOM_Y2);

   mouse.init();

   //------- init resource class ----------//

	#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
		font_std.init("SAN", 1);
		font_hall.init("HALL", 1);
	#else
		font_std.init("STD", 2);
	#endif

	font_san.init("SAN", 0);      // 0-zero inter-character space
	font_mid.init("MID");
	font_small.init("SMAL");
	font_news.init("NEWS");
	font_hitpoint.init("HITP");
	font_bible.init("CASA", 1, 3);
	font_bard.init("CASA", 0);

   image_icon.init(DIR_RES"I_ICON.RES",1,0);       // 1-read into buffer
   image_interface.init(DIR_RES"I_IF.RES",0,0);    // 0-don't read into the buffer, don't use common buffer

   #ifndef DEMO         // do not load these in the demo verison
      image_menu.init(DIR_RES"I_MENU.RES",0,0);       // 0-don't read into the buffer, don't use common buffer
      image_encyc.init(DIR_RES"I_ENCYC.RES",0,0); // 0-don't read into the buffer, don't use common buffer
   #endif

   image_button.init(DIR_RES"I_BUTTON.RES",1,0);
   image_spict.init(DIR_RES"I_SPICT.RES",1,0);
   image_tutorial.init(DIR_RES"TUT_PICT.RES",0,0);

	#ifdef AMPLUS
		#ifndef DEMO         // do not load these in the demo verison
			image_menu_plus.init(DIR_RES"I_MENU2.RES",0,0);       // 0-don't read into the buffer, don't use common buffer
		#endif
	#endif

   seek_path.init(MAX_BACKGROUND_NODE);
   seek_path_s2.init(1);//seek_path_s2.init(MAX_BACKGROUND_NODE);
   seek_path_reuse.init(MAX_BACKGROUND_NODE);
   group_select.init();

   //------------ init flame ------------//

   for(int i = 0; i < FLAME_GROW_STEP; ++i)
      flame[i].init(Flame::default_width(i), Flame::default_height(i), Flame::base_width(i), FLAME_WIDE);

   //------------ init animated line drawer -------//

   anim_line.init(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2);

   //---------- init other objects ----------//

   game_set.init();     // this must be called before game.init() as game.init() assume game_set has been initialized
   help.init("HELP.RES");

#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
   translate.init();
#endif

   tutor.init();
   game_file_array.init("*.SAV");

   //---------- init game_set -----------//

   DEBUG_LOG("Sys::init_objects finish");

   return TRUE;
}
//------- End of function Sys::init_objects -----------//


//------- Begin of function Sys::deinit_objects -----------//

void Sys::deinit_objects()
{
   //--------- deinit system class ----------//

   mouse.deinit();            // mouse must be deinitialized first
   mouse_cursor.deinit();

   //------- deinit resource class ----------//

   font_std.deinit();
   font_san.deinit();
   font_mid.deinit();
   font_small.deinit();
   font_news.deinit();
   font_hitpoint.deinit();
   font_bible.deinit();
	font_bard.deinit();

	#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
		font_hall.deinit();
	#endif

   image_icon.deinit();
   image_interface.deinit();
   image_menu.deinit();
   image_button.deinit();
   image_spict.deinit();
   image_encyc.deinit();
   image_tutorial.deinit();

   seek_path.deinit();
   seek_path_s2.deinit();
   seek_path_reuse.deinit();
   group_select.deinit();

   for(int i = 0; i < FLAME_GROW_STEP; ++i)
      flame[i].deinit();

   //--------- deinit other objects ----------//

   game_set.deinit();
   help.deinit();

#if( defined(GERMAN) || defined(FRENCH) || defined(SPANISH) )
   translate.deinit();
#endif

   tutor.deinit();
   config.deinit();
   game_file_array.deinit();
}
//------- End of function Sys::deinit_objects -----------//


//-------- Begin of function Sys::run --------//
//
void Sys::run(int isLoadedGame)
{
   //-*********** simulate aat ************-//
   #ifdef DEBUG
      //--------- enable only when simulation    -------//
      debug_sim_game_type = (m.is_file_exist("sim.sys")) ? 2 : 0;
   #endif
   //-*********** simulate aat ************-//

   sys_flag  = SYS_RUN;
   view_mode = MODE_NORMAL;

   //------- test LZW compression ---------//

#ifdef DEBUG_LZW
   test_lzw();
#endif

   //------ reset mouse ---------//

   mouse.reset_click();
   mouse_cursor.set_frame(0);

   //-- enable power after the game objets has been initialized --//

   power.enable();      // enable power, which handle mouse inputs

   //----- sys::disp_frame() will redraw everything when this flag is set to 1 ----//

   sys.need_redraw_flag = 1;

   option_menu.active_flag = 0;
   in_game_menu.active_flag = 0;

   sys.disp_frame();
   disp_view_mode();

   //----------- run the main loop -----------//

   main_loop(isLoadedGame);

   //-----------------------------------------//

   m.unlock_seed();
}
//--------- End of function Sys::run --------//


//-------- Begin of static function test_lzw --------//
//
static void test_lzw()
{
   // test lzw compress
   if( m.is_file_exist("NORMAL.SAV"))
   {
      File f,g;
      Lzw lzw_c, lzw_d;    // one for compress, the other for decompress
      f.file_open("NORMAL.SAV");

      // read into buffer
      long fileSize = f.file_size();
      unsigned char *srcPtr = (unsigned char *) mem_add(fileSize);
      f.file_read(srcPtr, fileSize);

      // find compressed size to allocate space
      long compSize = lzw_c.compress(srcPtr, fileSize);
      unsigned char *destPtr = (unsigned char *) mem_add( compSize/8+ 4 );       // alloc 4 bytes more
      if( compSize != lzw_c.compress(srcPtr, fileSize, destPtr) )
      {
         err_here();
      }

      // decompress again
      long backSize = lzw_d.expand(destPtr, compSize, NULL);
      err_when(backSize != fileSize);
      unsigned char *backPtr = (unsigned char *) mem_add( backSize+4 );
      if( backSize != lzw_d.expand(destPtr, compSize, backPtr) )
      {
         err_here();
      }

      // finally compare srcPtr and backPtr
      err_when( memcmp(srcPtr, backPtr, fileSize) );

      f.file_close();

      // write it to a file
      {
         unsigned char *writePtr = destPtr;
         long writeSize = (compSize +7) / 8;
         g.file_create("NORMAL.LZ1");
         for( ; writeSize > 0; writeSize -= 0x4000)
         {
            g.file_write(writePtr, writeSize > 0x4000 ? 0x4000 : writeSize);
            writePtr += 0x4000;
         }
         g.file_close();
      }

      // test two, compress to a file
      g.file_create("NORMAL.LZW");
      compSize = lzw_c.compress(srcPtr, fileSize, &g);
      g.file_close();

      g.file_open("NORMAL.LZW");
      backSize = lzw_d.expand(&g, NULL);
      err_when(backSize != fileSize);
      backPtr = (unsigned char *) mem_resize(backPtr, backSize+4 );
      g.file_close();

      g.file_open("NORMAL.LZW");
      if( backSize != lzw_d.expand(&g, backPtr))
      {
         err_here();
      }
      err_when( memcmp(srcPtr,backPtr, fileSize) );

      mem_del(destPtr);
      mem_del(backPtr);
      mem_del(srcPtr);
   }
}
//--------- End of static function test_lzw --------//


//-------- Begin of function Sys::main_loop --------//
//
void Sys::main_loop(int isLoadedGame)
{
   MSG msg;
   // #### begin Gilbert 31/10 #####//
   // int rc;
   // #### end Gilbert 31/10 #####//

   //-------- reset day_frame_count -------//

   if( !isLoadedGame )
   {
      day_frame_count = 0;       // for determining when the day counter should be increased.
      frame_count = 1;
   }

   //----- initialize these vars for every game -----//

   for( int i=nation_array.size() ; i>0 ; i-- )
   {
      if( !nation_array.is_deleted(i) )
         nation_array[i]->next_frame_ready = 0;
   }

#ifdef USE_DPLAY
   remote.packet_send_count    = 0;
   remote.packet_receive_count = 0;
#endif

   last_frame_time = m.get_time()+60000;     // plus 60 seconds buffer for game loading/starting time
   //frame_count     = 1;
   is_sync_frame   = 0;

   //----------------------------------------------//
#ifdef USE_DPLAY
   mp_clear_request_save();
   remote.enable_poll_msg();
   remote.enable_process_queue();
   remote_send_success_flag = 1;
#endif

#ifdef DEBUG
   char longLogSuffix = 'A';
#ifdef USE_DPLAY
   if( remote.is_enable() )
   {
      if(long_log)
         delete long_log;
      long_log = new LongLog(longLogSuffix);
   }
#endif // USE_DPLAY
#endif // DEBUG

   //-*********** syn game test ***********-//
   #ifdef DEBUG
   if(debug_seed_status_flag==DEBUG_SYN_LOAD_AND_COMPARE_ONCE)
      sp_load_seed_file();
   #endif
   //-*********** syn game test ***********-//

   vga_front.unlock_buf();

   // ------- establish_contact again --------//
   // if the game saved after NationRelation::contact_msg_flag set, but
   // remote players may have not receive MSG_NATION_CONTACT
   //
   // send MSG_NATION_CONTACT now
   if( !config.explore_whole_map && nation_array.player_recno &&
      !nation_array.is_deleted(nation_array.player_recno) )
   {
      for(short nationRecno = 1; nationRecno <= nation_array.size(); ++nationRecno )
      {
         if( nationRecno == nation_array.player_recno ||
            nation_array.is_deleted(nationRecno) )
            continue;

         NationRelation *relation = (~nation_array)->get_relation(nationRecno);
         if( relation->contact_msg_flag && !relation->has_contact)
         {
#ifdef USE_DPLAY
            // packet structure : <player nation> <explored nation>
            short *shortPtr = (short *)remote.new_send_queue_msg(MSG_NATION_CONTACT, 2*sizeof(short));
            *shortPtr = nation_array.player_recno;
            shortPtr[1] = nationRecno;
#endif
         }
      }
   }

   // #### begin Gilbert 23/10 #######//
   option_menu.active_flag = 0;
   in_game_menu.active_flag = 0;
   // #### end Gilbert 23/10 #######//

   // ##### begin Gilbert 4/11 ######//
   DWORD lastDispFrameTime = m.get_time();
   // ##### end Gilbert 4/11 ######//

	// ##### patch begin Gilbert 17/11 #######//
	DWORD firstUnreadyTime = 0;
	// ##### patch end Gilbert 17/11 #######//

   while( 1 )
   {
      if ( !paused_flag && active_flag )
      {
         // #### begin Gilbert 31/10 ######//
         int rc = 0;
         // #### end Gilbert 31/10 ######//
         if( sys.signal_exit_flag )
            break;

         vga_front.lock_buf();

         yield();       // could be improved, give back the control to Windows, so it can do some OS management. Maybe call WaitMessage() here and set up a timer to get messages regularly.

         detect();

         //--------------------------------//

         // ###### begin Gilbert 4/11 ######//
         DWORD markTime = m.get_time();      // a time taken earlier than should_next_frame takes
         // ###### end Gilbert 4/11 ######//

			// ##### patch begin Gilbert 17/11 #######//
			int unreadyPlayerFlag = 0;
			// ##### patch end Gilbert 17/11 #######//

         if( config.frame_speed>0 )              // 0-frozen
         {
#ifdef USE_DPLAY
            if( remote.is_enable() )      // && is_sync_frame )
            {
               remote.poll_msg();
               m.unlock_seed();
               rc = is_mp_sync(&unreadyPlayerFlag);         // if all players are synchronized
               m.lock_seed();
            }
            else
#endif
               rc = should_next_frame();

            if( rc )
            {
               LOG_BEGIN;
               m.unlock_seed();

#if (defined(DEBUG) && defined(USE_DPLAY))
               if( remote.is_enable() )
               {
                  long_log->printf("begin process frame %d\n", frame_count);
               }
#endif

               process();

#ifdef USE_DPLAY
               if(remote.is_enable() )
                  m.lock_seed();    // such that random seed is unchanged outside sys::process()
#endif
               LOG_END;

#ifdef USE_DPLAY
               // -------- compare objects' crc --------- //
					// ###### patch begin Gilbert 20/1 ######//
					if( remote.is_enable() && (remote.sync_test_level & 2) &&(frame_count % (remote.get_process_frame_delay()+3)) == 0)
               {
                  // cannot compare every frame, as PROCESS_FRAME_DELAY >= 1
                  crc_store.record_all();
                  crc_store.send_all();
               }
					// ###### patch end Gilbert 20/1 ######//
#endif

            }
         }

         // ###### begin Gilbert 4/11 #######//
         // ------- display graduately, keep on displaying --------- //
         if( rc )
         {
            lastDispFrameTime = m.get_time();
				// ####### patch begin Gilbert 17/11 ######//
				// reset firstUnreadyTime
				firstUnreadyTime = 0;
				// ####### patch end Gilbert 17/11 ######//
         }
         else
         {
				// ####### patch begin Gilbert 17/11 ######//
				// set firstUnreadyTime, begin of a delay
				if( !firstUnreadyTime )
					firstUnreadyTime = m.get_time();
				// ####### patch end Gilbert 17/11 ######//

            if( config.frame_speed == 0 || markTime-lastDispFrameTime >= DWORD(1000/config.frame_speed) 
#ifdef AMPLUS
					|| zoom_need_redraw || map_need_redraw
#endif
					)
            {
               // on second condition, it should be happened when
               // in multiplayer, where should_next_frame passed
               // but is_mp_sync not passed
               disp_frame();
               lastDispFrameTime = markTime;

					// ####### patch begin Gilbert 17/11 ######//
					// display player not ready
					if( firstUnreadyTime && m.get_time() - firstUnreadyTime > 5000 )
					{
						int y = ZOOM_Y1 + 10;
						int x = ZOOM_X1 + 10;
						for( int nationRecno = 1; nationRecno <= MAX_NATION; ++nationRecno )
						{
							if( unreadyPlayerFlag & (1 << (nationRecno-1)) )
							{
								if( !nation_array.is_deleted(nationRecno) )
								{
									int x2 = font_news.put( x, y, "Waiting for ");
									x2 = font_news.put( x2, y, nation_array[nationRecno]->nation_name() );
									y += font_news.height() + 5;
								}
							}
						}
					}
					// ####### patch end Gilbert 17/11 ######//
            }
         }
         // ###### end Gilbert 4/11 #######//

         // ----------- detect sond is ended, play another -----------//

         if( config.frame_speed == 0 || day_frame_count == 0)
            music.yield();

#if (defined(DEBUG) && defined(USE_DPLAY))
         if( rc && remote.is_enable() && day_frame_count == 0 )
         {
            if( long_log)
               delete long_log;

            if( ++longLogSuffix > 'Z' )
            {
               longLogSuffix = 'A';
            }
            long_log = new LongLog(longLogSuffix);
            long_log->printf("Game Date : %d/%d/%d\n", info.game_month, info.game_day, info.game_year);
         }
#endif

         if(rc)
         {
            //-*********** syn game test ***********-//
            //-------------------------------------------------------------//
            // record random seed for comparison
            //-------------------------------------------------------------//
            #ifdef DEBUG
               if(debug_seed_status_flag==DEBUG_SYN_LOAD_AND_COMPARE_ONCE ||
                  debug_seed_status_flag==DEBUG_SYN_AUTO_LOAD)
                  sp_compare_seed();
               else if(debug_seed_status_flag==DEBUG_SYN_AUTO_SAVE)
                  sp_record_seed();
            #endif
            //-*********** syn game test ***********-//

            //------ auto save -------//

            auto_save();
         }

         //------ detect save game triggered by remote player ------//

#ifdef USE_DPLAY
         if( mp_save_flag && mp_save_frame == frame_count )
         {
            mp_clear_request_save();            // clear request first before save game

            if( nation_array.player_recno )     // only save then the player is still in the game
            {
               game_file.save_game( remote.save_file_name );

               // ####### begin Gilbert 24/10 ######//
               //static String str;
               //str  = "The current game has been saved to ";
               //str += remote.save_file_name;
               //str += ".";
               //box.msg( str );
               news_array.multi_save_game();
               // ####### end Gilbert 24/10 ######//
            }
         }
#endif

         vga_front.unlock_buf();

         if( sys.signal_exit_flag )
            break;
      }
      else
      {
         window.handle_messages();
      }
   }

   // #### begin Gilbert 23/10 #######//
   in_game_menu.active_flag = 0;
   option_menu.active_flag = 0;
   // #### end Gilbert 23/10 #######//

   vga_front.lock_buf();

#if (defined(DEBUG) && defined(USE_DPLAY))
   if(remote.is_enable())
   {
      if(long_log)
         delete long_log;
      long_log = NULL;
   }
#endif

   music.stop();
#ifdef USE_DPLAY
   remote.disable_process_queue();
   remote.disable_poll_msg();
   mp_clear_request_save();
#endif
}
//--------- End of function Sys::main_loop --------//


//-------- Begin of function Sys::auto_save --------//
//
void Sys::auto_save()
{
   if( nation_array.player_recno == 0 )
      return;

   //---------- single player auto save ----------//

#ifdef USE_DPLAY
   if( !remote.is_enable() &&          // no auto save in a multiplayer game
       info.game_month%2==0 && info.game_day==1 && day_frame_count==0)
#else
   if(info.game_month%2==0 && info.game_day==1 && day_frame_count==0)
#endif
   {
      #ifdef DEBUG2
      if(1)
      #else
      if( sys.debug_session || sys.testing_session )
      #endif
      {
         static int saveCount = 0;
         switch(saveCount)
         {
            case 0:  game_file.save_game( "AUTO1.SAV" );
                     break;
            case 1:  game_file.save_game( "AUTO2.SAV" );
                     break;
            case 2:  game_file.save_game( "AUTO3.SAV" );
                     break;
         }

         if( ++saveCount>3 )
            saveCount = 0;
      }
      else
      {
         //--- rename the existing AUTO.SAV to AUTO2.SAV and save a new game ---//

         if( m.is_file_exist( "AUTO.SAV" ) )
         {
            if( m.is_file_exist( "AUTO2.SAV" ) )      // if there is already an AUTO2.SAV, delete it
               remove( "AUTO2.SAV" );

            rename( "AUTO.SAV", "AUTO2.SAV" );
         }
      }

      game_file.save_game( "AUTO.SAV" );

      //-*********** syn game test ***********-//
      #ifdef DEBUG
         if(debug_seed_status_flag==DEBUG_SYN_AUTO_SAVE)
         {
            sp_write_seed();
            sp_close_seed_file();

            debug_seed_status_flag = NO_DEBUG_SYN;
            mouse.add_key_event(DIK_BACKSLASH, m.get_time()); // load file for comparison
         }

         //debug_seed_status_flag = 2;
         //sp_seed_pos_reset();
         //sp_record_match_seed();
      #endif
      //-*********** syn game test ***********-//
   }

#ifdef USE_DPLAY
   // --------- multiplayer autosave game --------//

	// ###### patch begin Gilbert 23/1 #######//
   if( remote.is_enable() && remote.sync_test_level >= 0 &&			// disable autosave after un-sync
      day_frame_count==0 && info.game_day==1 && info.game_month%2==0 )
	// ###### patch end Gilbert 23/1 #######//
   {
      //--- rename the existing AUTO.SVM to AUTO2.SVM and save a new game ---//

      if( m.is_file_exist( "AUTO.SVM" ) )
      {
         if( m.is_file_exist( "AUTO2.SVM" ) )      // if there is already an AUTO2.SVM, delete it
            remove( "AUTO2.SVM" );

         rename( "AUTO.SVM", "AUTO2.SVM" );
      }

      game_file.save_game( "AUTO.SVM" );
   }
#endif
}
//-------- End of function Sys::auto_save --------//


//-------- Begin of function Sys::pause --------//
//
void Sys::pause()
{
   if( paused_flag )
      return;

   window.flag_redraw();

   paused_flag = TRUE;
}
//--------- End of function Sys::pause ---------//


//-------- Begin of function Sys::unpause --------//
//
void Sys::unpause()
{
   if( !paused_flag )
      return;

   // ####### begin Gilbert 3/11 #######//
   //if( GetForegroundWindow() != main_hwnd )
     // return;
   // ####### end Gilbert 3/11 #######//

   if( !restore() )
   {
      //-----------------------------------------------------//
      //  we are unable to restore, this can happen when
      //  the screen resolution or bitdepth has changed
      //  we just reload all the art again and re-create
      //  the front and back buffers.  this is a little
      //  overkill we could handle a screen res change by
      //  just recreating the front and back buffers we dont
      //  need to redo the art, but this is way easier.
      //-----------------------------------------------------//

      if (init_directx())
      {
         if( !restore() )     // if still not successful, quit
            return;
      }
   }

   // ####### begin Gilbert 31/10 #######//
   mouse.update_skey_state();       // update ctrl/shift/alt key state after switch task
   // ####### end Gilbert 31/10 #######//

   //---- restore the saved screen before killing the focus ----//

   paused_flag = FALSE;
}
//--------- End of function Sys::unpause ---------//


//-------- Begin of function Sys::restore --------//
//
int Sys::restore()
{
   if( !vga_front.restore_buf() )
      return 0;

   if( !vga_back.restore_buf() )
      return 0;

   if( sys.debug_session )
   {
      if( !vga_true_front.restore_buf() )
         return 0;
   }
   // ####### begin Gilbert 16/9 ########//
   //else
   // vga_front.lock_buf();
   // ####### end Gilbert 16/9 ########//

   return 1;
}
//--------- End of function Sys::restore ---------//


//-------- Begin of function Sys::yield --------//
//
void Sys::yield()
{
   static int isYielding=0;

   if( isYielding )
      return;

   isYielding=1;

   window.handle_messages();

   mouse.poll_event();

   audio.yield();

#ifdef USE_DPLAY
   if( remote.is_enable() )
   {
      //yield_wsock_msg();
      remote.poll_msg();

      remote.process_specific_msg(MSG_SET_SPEED);        // need to test it here for restoring the speed from frozen to normal

      if( config.frame_speed > 0 )
      {
         remote.process_specific_msg(MSG_TELL_SEND_TIME);
         remote.process_specific_msg(MSG_REQUEST_RESEND);
      }

      //-------- display debug info -----------//

      if( power.enable_flag && (testing_session || debug_session) )
      {
         String str;

         str  = "Player: ";
         str += nation_array.player_recno;
         str += "/";
         str += nation_array.size();

         str += " Send:";
         str += remote.packet_send_count;
         str += " Recv:";
         str += remote.packet_receive_count;
         str += " Frame:";
         str += frame_count;

         font_san.disp( ZOOM_X1, 4, str, ZOOM_X1+300);
      }
   }
#endif

   isYielding=0;
}
//--------- End of function Sys::yield ---------//


//-------- Begin of function Sys::yield_wsock_msg --------//
//
void Sys::yield_wsock_msg()
{
   // MSG msg;

   //------ only get WinSock messages (WSA_ACCEPT & WSA_READ) ------//

   // if( PeekMessage(&msg, NULL, WSA_ACCEPT, WSA_READ, PM_NOREMOVE) )
   // {
   //   if (!GetMessage( &msg, NULL, WSA_ACCEPT, WSA_READ))
   //       return;

   //   TranslateMessage(&msg);
   //   DispatchMessage(&msg);
   //}
}
//--------- End of function Sys::yield_wsock_msg ---------//


//-------- Begin of function Sys::is_mp_sync --------//
//
// Multiplayer synchronization.
//
// Check all players are ready to proceed to the next frame.
//
int Sys::is_mp_sync(int *unreadyPlayerFlag)
{
#ifdef USE_DPLAY
   #define RESEND_TIME_OUT            2000    // if the other machines still aren't ready after 2 seconds, send the notification again
   #define RESEND_AGAIN_TIME_OUT      1000    // keep resending if no responses
   #define CONNECTION_LOST_TIME_OUT  20000    // ask for connection lost handling aftering waiting for 5 seconds.

   //---- if we haven't been ready for the next frame yet ----//

#ifdef DEBUG
   int n;
   DEBUG_LOG("begin nation's next_frame_ready");
   for (n = 1; n <= nation_array.size(); ++n)
   {
      DEBUG_LOG(nation_array[n]->next_frame_ready);
   }
   DEBUG_LOG("end nation's next_frame_ready");
#endif

	// ####### patch begin Gilbert 17/11 ######//
	if( unreadyPlayerFlag )
		*unreadyPlayerFlag = 0;
	// ####### end begin Gilbert 17/11 ######//

   // if last remote.send was fail, attempt to send it again
   if( !nation_array.player_recno )
   {
      // observation mode
      if( !should_next_frame() )
         return 0;
      remote_send_success_flag = 1;
   }
   else if( remote_send_success_flag 
		&& remote.has_send_frame(nation_array.player_recno, frame_count)
		&& (~nation_array)->next_frame_ready==0 )
   {
      DEBUG_LOG("Local player not ready");
      if( !should_next_frame() )    // not ready to proceed yet
         return 0;

      //------------ queue MSG_NEXT_FRAME ----------//

      short* shortPtr = (short*) remote.new_send_queue_msg(MSG_NEXT_FRAME, sizeof(short));

      shortPtr[0] = nation_array.player_recno;  // short_para1 is the nation recno of the current player

      //------------ queue MSG_QUEUE_TRAILER ----------//

      shortPtr = (short*) remote.new_send_queue_msg(MSG_QUEUE_TRAILER, sizeof(short));
      shortPtr[0] = nation_array.player_recno;  // short_para1 is the nation recno of the current player

      //------ copy all queued action to our own receive buffer to merge with other player's action ----//

      remote.append_send_to_receive();

      //--- copy the whole queue to a buffer in case of resend request from other players ---//

      remote.copy_send_to_backup();

      //----------- queue MSG_TELL_SEND_TIME ----------//
/*
      unsigned long* longPtr = (unsigned long*) remote.new_send_queue_msg(MSG_TELL_SEND_TIME, sizeof(unsigned long));

      longPtr[0] = m.get_time();
*/
      //---------- send out all messages in the queue ---------//

      remote_send_success_flag = remote.send_queue_now();               // if not sent successfully, try again next time

      if( remote_send_success_flag )      // still failed, try again next time
      {
         DEBUG_LOG("first send sucess" );
         remote.init_send_queue(frame_count+1, nation_array.player_recno);    // frame_count, initialize for next frame's send queue
         // sent random seed
         char *p = (char *)remote.new_send_queue_msg(MSG_TELL_RANDOM_SEED, sizeof(short)+sizeof(long));
         *(short *)p = nation_array.player_recno;
         p += sizeof(short);
         *(long *)p = m.get_random_seed();
      }
      else
      {
         // re_transmit as quickly as possible
         ec_remote.re_transmit(5);
      }
   }
   else
   {
      DEBUG_LOG("Local player nation ready");
   }

   //----- if previous sending was not successful, send again now -----//

   if( !remote_send_success_flag )
   {
      remote_send_success_flag = remote.send_queue_now();               // if not sent successfully, try again next time

      if( remote_send_success_flag )      // still failed, try again next time
      {
         DEBUG_LOG("resending ok");
         remote.init_send_queue(frame_count+1, nation_array.player_recno);    // frame_count, initialize for next frame's send queue
         // sent random seed
         char *p = (char *)remote.new_send_queue_msg(MSG_TELL_RANDOM_SEED, sizeof(short)+sizeof(long));
         *(short *)p = nation_array.player_recno;
         p += sizeof(short);
         *(long *)p = m.get_random_seed();
      }
      else
      {
         // re_transmit as quickly as possible
         ec_remote.re_transmit(5);
         DEBUG_LOG("resending not ok");
         return 0;
      }
   }

   //------ pre_process MSG_NEXT_FRAME in the queue -----//

   remote.process_specific_msg(MSG_NEXT_FRAME);

#ifdef DEBUG
   DEBUG_LOG("begin nation's next_frame_ready");
   for (n = 1; n <= nation_array.size(); ++n)
   {
      DEBUG_LOG(nation_array[n]->next_frame_ready);
   }
   DEBUG_LOG("end nation's next_frame_ready");
#endif

   //------ check if all remote players are ready to proceed -----//

   int     nationRecno;
   Nation* nationPtr;

   for( nationRecno=nation_array.size() ; nationRecno>0 ; nationRecno-- )
   {
      if( nation_array.is_deleted(nationRecno) )
         continue;

      nationPtr = nation_array[nationRecno];

      //------- if some remote machines are not ready yet -------//

		// ###### patch begin Gilbert 17/11 ######//
      if( nationPtr->is_remote() &&
			(remote.has_send_frame(nationRecno, frame_count) && !nationPtr->next_frame_ready) )
		{
			if( unreadyPlayerFlag )
				*unreadyPlayerFlag |= ( 1 << (nationRecno-1) );
         break;
		}
		// ###### end begin Gilbert 17/11 ######//
   }

   //------- if some remote machines are not ready yet -------//

   if( nationRecno>0 )
   {
      DEBUG_LOG("a nation not ready");
      DEBUG_LOG(nationRecno);

      if( m.get_time() >= last_frame_time+RESEND_TIME_OUT )
      {
         //---- if it has been time out for too long, carry out connection lost handling ---//

         if( // m.get_time() >= last_frame_time+CONNECTION_LOST_TIME_OUT ||
            !ec_remote.is_player_valid(nationRecno))
         {
            DEBUG_LOG( "Connection Lost" );
            // ###### begin Gilbert 24/10 ######//
            // box.msg( "Connection Lost!" );         //**BUGHERE, should have a function to set all units, structures of this nation to AI
            news_array.multi_connection_lost(nationRecno);
            // may allow save game here, ask user whether to save the game
            // ###### end Gilbert 24/10 ######//
				nationPtr->nation_type = NATION_AI;    // let computer take over the nation
				nation_array.ai_nation_count++;
			}

         //------- re-send the message -------//

         /* re-send is now done by ec_remote
         else if( m.get_time() >= last_resend_time + RESEND_AGAIN_TIME_OUT )     // resent once per half second
         {
            DEBUG_LOG( "send retransmit request" );
            RemoteMsg* remoteMsg = remote.new_msg(MSG_REQUEST_RESEND, sizeof(int) + sizeof(DWORD) );

            DWORD* dwordPtr = (DWORD*) remoteMsg->data_buf;

            dwordPtr[0] = (~nation_array)->player_id;          // send to us
            dwordPtr[1] = frame_count;                         // request it to send us queue of this frame

            remote.send_free_msg( remoteMsg, nation_array[nationRecno]->player_id );      // send to that nationRecno only

            last_resend_time = m.get_time();
         }
         */
      }

      return 0;
   }

   //--------------------------------------------------------//
   //
   // When all players are ready to proceed to the next frame
   //
   // As we have already know all players are ready, we can
   // reset the next_frame_ready flag for all nations.
   //
   //--------------------------------------------------------//

   DEBUG_LOG("all nation ready");
   for( int i=nation_array.size() ; i>0 ; i-- )
   {
      if( nation_array.is_deleted(i) )
         continue;

      nation_array[i]->next_frame_ready=0;      // -- instead of set to 0, set it may be 2 if it has just received an notifying signal for the further next frame from a player as it also sent out a next frame ready msg to all other players
   }

   //--------- process msgs in the receive queue ----------//

   remote.process_receive_queue();

#ifdef DEBUG
   DEBUG_LOG("begin nation's next_frame_ready");
   for (n = 1; n <= nation_array.size(); ++n)
   {
      DEBUG_LOG(nation_array[n]->next_frame_ready);
   }
   DEBUG_LOG("end nation's next_frame_ready");
#endif

   //-------- record this frame's time -------//

   last_frame_time  = m.get_time();
   last_resend_time = 0;

#endif // USE_DPLAY

   return 1;
}
//---------- End of function Sys::is_mp_sync --------//


//-------- Begin of function Sys::should_next_frame --------//
//
// Check if it's now the time for processing the next frame.
//
int Sys::should_next_frame()
{
   //----- special modes: 0-frozen, 9-fastest possible -----//

   if( config.frame_speed==99 )
      return 1;

   if( config.frame_speed==0 )
      return 0;

   //---- check if it's now the time for processing the next frame ----//

   DWORD curTime = m.get_time();

   if( next_frame_time )      // if next_frame_time==0, it's the first frame of the game
   {
      if( next_frame_time < 1000 )  // the DWORD variable has been overflow
      {
         if( curTime < next_frame_time || curTime >= 1000 )    // >= 1000 if the curTime has been overflow yet, wait for it to overflow so we can compare it when next_frame_time
            return 0;
      }
      else     // normal non-overflow case
      {
         if( curTime < next_frame_time )
            return 0;
      }
   }

   //--- Time between frames = 1000 milliseconds / frames per second ---//

   next_frame_time = curTime + 1000 / config.frame_speed;

   return 1;
}
//--------- End of function Sys::should_next_frame ---------//


//-------- Begin of function Sys::process_key --------//
//
void Sys::process_key(unsigned scanCode, unsigned skeyState)
{
   detect_function_key(scanCode, skeyState);

   //----- don't detect letter keys when in chat mode ----//

   if( !(view_mode == MODE_NATION &&
         info.nation_report_mode == NATION_REPORT_CHAT) )
   {
      if( sys.debug_session || sys.testing_session || scenario_cheat_flag )
      {
         detect_cheat_key(scanCode, skeyState);
         detect_debug_cheat_key(scanCode, skeyState);

         if( detect_scenario_cheat_key(scanCode, skeyState) )
            return;
      }
      else
      {
#ifdef USE_DPLAY
         if( nation_array.player_recno && !remote.is_enable() )      // not allowed in multiplayer mode
#else
         if( nation_array.player_recno )
#endif
         {
            if( (~nation_array)->cheat_enabled_flag )
            {
               detect_cheat_key(scanCode, skeyState);
            }
            else
				{
					#ifdef GERMAN
						if( detect_key_str(1, "!!!###") )
					#else
						if( detect_key_str(1, "!!!@@@###") )
					#endif
					{
                  box.msg( "Cheat Mode Enabled." );
                  (~nation_array)->cheat_enabled_flag = 1;
               }
            }
         }
      }

      detect_letter_key(scanCode, skeyState);

      detect_set_speed(scanCode, skeyState);                     // set the speed of the game
   }
}
//--------- End of function Sys::process_key ---------//


//-------- Begin of function Sys::detect_letter_key --------//
//
void Sys::detect_letter_key(unsigned scanCode, unsigned skeyState)
{
   int keyCode;

   if((keyCode = mouse.is_key(scanCode, skeyState, (WORD) 0, K_IS_CTRL)))
   {
      int groupId;
      switch(keyCode)
      {
         case '1': case '2': case '3': case '4': case '5':
         case '6': case '7': case '8': case '9':
            groupId = keyCode-'0';
            group_select.group_units(groupId);
            break;
      }
   }

   if((keyCode = mouse.is_key(scanCode, skeyState, (WORD) 0, K_IS_ALT)))
   {
      int groupId;
      switch(keyCode)
      {
         case '1': case '2': case '3': case '4': case '5':
         case '6': case '7': case '8': case '9':
            groupId = keyCode-'0';
            group_select.select_grouped_units(groupId);
            break;
      }
   }

   if( (keyCode = mouse.is_key(scanCode, skeyState, (WORD) 0, K_UNIQUE_KEY)) )
   {
      keyCode = m.lower(keyCode);

      switch(keyCode)
      {
      case KEY_ESC:
         set_view_mode(MODE_NORMAL);
         break;

      //---- keys for toggling map mode ----//

      case 'q':
         world.map_matrix->toggle_map_mode(0);
         break;

      case 'w':
         world.map_matrix->toggle_map_mode(1);
         break;

      case 'e':
         world.map_matrix->toggle_map_mode(2);
         break;

      //--------- opaque report mode --------//

      case 'p':
         config.opaque_report = !config.opaque_report;

         if( config.opaque_report )
            box.msg( "Opaque report mode." );
         else
            box.msg( "Transparent report mode." );
         break;

      //------ clear news messages ------//

      case 'x':
         news_array.clear_news_disp();
         break;

      //------ jump to a location with natural resource ---//

      case 'j':
         site_array.go_to_a_raw_site();
         break;

      //--------- bring up the option menu  ----------//

      case 'o':
         // ##### begin Gilbert 5/11 #######//
         // game.in_game_option_menu();
#ifdef USE_DPLAY
         option_menu.enter(!remote.is_enable());
#else
         option_menu.enter(1);
#endif
         // ##### end Gilbert 5/11 #######//
         break;

      //--------- forward/backward tutorial text block --------//

      case ',':
         if( game.game_mode == GAME_TUTORIAL )
            tutor.prev_text_block();
         break;

      case '.':
         if( game.game_mode == GAME_TUTORIAL )
            tutor.next_text_block();
         break;

      //---- keys for saving and loading game -----//

      case 's':
         save_game();
         break;

      case 'l':
         load_game();
         break;

      case KEY_UP:
         world.disp_next(-1, 0);    // previous same object type of any nation
         break;

      case KEY_DOWN:
         world.disp_next(1, 0);     // next same object type of any nation
         break;

      case KEY_LEFT:
         world.disp_next(-1, 1);    // prevous same object type of the same nation
         break;

      case KEY_RIGHT:
         world.disp_next(1, 1);     // next same object type of the same nation
         break;

      //---- key for quick locate -----//

      case 'k':
         locate_king_general(RANK_KING);
         break;

      case 'g':
         locate_king_general(RANK_GENERAL);
         break;

      case 'y':
         locate_spy();
         break;

      case 'h':
         locate_ship();
         break;

      case 'f':
         locate_camp();
         break;
      }
   }
}
//--------- End of function Sys::detect_letter_key ---------//


//-------- Begin of function Sys::detect_function_key --------//
//
void Sys::detect_function_key(unsigned scanCode, unsigned skeyState)
{
   int keyCode;

   if( (keyCode = mouse.is_key(scanCode, skeyState, (WORD) 0, K_UNIQUE_KEY)) )
   {
      switch(keyCode)
      {
      case KEY_ESC:
         set_view_mode(MODE_NORMAL);
         break;

      case KEY_F1:
         if( view_mode==MODE_NATION )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_NATION);
         break;
      case KEY_F2:
         if( view_mode==MODE_TOWN )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_TOWN);
         break;
      case KEY_F3:
         if( view_mode==MODE_ECONOMY )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_ECONOMY);
         break;
      case KEY_F4:
         if( view_mode==MODE_TRADE )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_TRADE);
         break;
      case KEY_F5:
         if( view_mode==MODE_MILITARY )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_MILITARY);
         break;
      case KEY_F6:
         if( view_mode==MODE_TECH )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_TECH);
         break;
      case KEY_F7:
         if( view_mode==MODE_SPY )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_SPY);
         break;
      case KEY_F8:
         if( view_mode==MODE_RANK )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_RANK);
         break;
      case KEY_F9:
         if( view_mode==MODE_NEWS_LOG )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_NEWS_LOG);
         break;

      case KEY_F10:
         // ##### begin Gilbert 5/11 ######//
         //game.in_game_menu();
#ifdef USE_DPLAY
         in_game_menu.enter(!remote.is_enable());
#else
         in_game_menu.enter(1);
#endif
         // ##### end Gilbert 5/11 ######//
         break;

      case KEY_F11:
         capture_screen();
         break;

#ifdef BETA
      case KEY_F12:
         sys.signal_exit_flag = 1;
         break;
#endif
      }
   }
}
//--------- End of function Sys::detect_function_key ---------//


//-------- Begin of function Sys::detect_cheat_key --------//
//
void Sys::detect_cheat_key(unsigned scanCode, unsigned skeyState)
{
#ifdef USE_DPLAY
   if( remote.is_enable() )      // no cheat keys in multiplayer games
      return;
#endif

   int keyCode = mouse.is_key( scanCode, skeyState, (WORD) 0, K_CHAR_KEY );

   if( !keyCode )    // since all keys concern are printable
      return;

   keyCode = m.lower(keyCode);

   switch( keyCode )
   {
      //-------- cheat keys ---------//

      case 'c':      // add cash
         if( nation_array.player_recno )
            (~nation_array)->add_cheat((float)1000);
         break;

      case '\\':     // add food
         if( nation_array.player_recno )
            (~nation_array)->add_food((float)1000);
         break;

      case 't':
         tech_res.inc_all_tech_level(nation_array.player_recno);
         god_res.enable_know_all(nation_array.player_recno);
         box.msg( "Your technology has advanced.\nYou can now invoke all Greater Beings." );
         break;

      case 'm':
         world.unveil(0, 0, MAX_WORLD_X_LOC-1, MAX_WORLD_Y_LOC-1);
         world.visit(0, 0, MAX_WORLD_X_LOC-1, MAX_WORLD_Y_LOC-1, 0, 0);
         break;

      case ';':   // increase town population
         if( town_array.selected_recno )
         {
            Town* townPtr = town_array[town_array.selected_recno];
            #ifdef DEBUG2
               for(int di=0; di<MAX_RACE; di++)
               {
                  if(townPtr->race_pop_array[di])
                  {
                     townPtr->init_pop(di+1, 10, 100);
                     break;
                  }
               }
            #else
               townPtr->init_pop( m.random(MAX_RACE)+1, 10, 100 );
            #endif
            townPtr->auto_set_layout();
         }
         break;

      case 'u':
         config.king_undie_flag = !config.king_undie_flag;

         if( config.king_undie_flag )
            box.msg( "Your king is now immortal." );
         else
            box.msg( "King immortal mode is now disabled." );
         break;

      case '=':
         if( firm_array.selected_recno )
         {
            Firm* firmPtr = firm_array[firm_array.selected_recno];

            if( firmPtr->firm_id == FIRM_BASE )
            {
               ((FirmBase*)firmPtr)->pray_points = (float) MAX_PRAY_POINTS;
               info.disp();
            }
         }
         break;

      case 'b':      // finish building a firm instantly or increase the hit points of a firm to its MAX
         if( firm_array.selected_recno )
         {
            Firm* firmPtr = firm_array[firm_array.selected_recno];
            firmPtr->hit_points = firmPtr->max_hit_points;
         }
         break;

      case 'z':      // toggle fast_build
         config.fast_build = !config.fast_build;

         if( !config.fast_build )
            box.msg( "Fast build is now disabled" );
         else
            box.msg( "Fast build is now enabled" );
         break;

      //----- increase the combat level -------//

      case '[':
         if( unit_array.selected_recno )
         {
            Unit* unitPtr = unit_array[unit_array.selected_recno];

            unitPtr->set_combat_level( MIN(100, unitPtr->skill.combat_level+20) );
         }
         break;

      //----- increase the skill level of the unit -------//

      case ']':
         if( unit_array.selected_recno )
         {
            Unit* unitPtr = unit_array[unit_array.selected_recno];

            if( unitPtr->skill.skill_id )
               unitPtr->skill.skill_level = MIN(100, unitPtr->skill.skill_level+20);
         }
         break;

      //----- increase the spying skill -------//

      case '\'':
         if( unit_array.selected_recno )
         {
            Unit* unitPtr = unit_array[unit_array.selected_recno];

            if( unitPtr->spy_recno )
            {
               Spy* spyPtr = spy_array[unitPtr->spy_recno];

               spyPtr->spy_skill = MIN(100, spyPtr->spy_skill+20);
            }
         }
         break;
   }
}
//--------- End of function Sys::detect_cheat_key ---------//


//-------- Begin of function Sys::detect_debug_cheat_key --------//
//
void Sys::detect_debug_cheat_key(unsigned scanCode, unsigned skeyState)
{
#ifdef USE_DPLAY
   if( remote.is_enable() )      // no cheat keys in multiplayer games
      return;
#endif

   int keyCode = mouse.is_key( scanCode, skeyState, (WORD) 0, K_UNIQUE_KEY );

   if( !keyCode )    // since all keys concern are printable
      return;

   keyCode = m.lower(keyCode);

   switch( keyCode )
   {
/*
      case 'j':      // allow all nations to have all god creatures
         for( i=1; i<=nation_array.size() ; i++ )
         {
            if( !nation_array.is_deleted(i) )
               god_res.enable_know_all(i);
         }
         box.msg( "Now knowledge of seat of power is available to all nations." );
         break;
*/
      case 'n':
         config.blacken_map = !config.blacken_map;
         config.fog_of_war  = config.blacken_map;
         break;

      case 'r':      // set default report nation
         if( firm_array.selected_recno )
         {
            info.default_viewing_nation_recno = firm_array[firm_array.selected_recno]->nation_recno;
         }
         else if( town_array.selected_recno )
         {
            int nationRecno = town_array[town_array.selected_recno]->nation_recno;

            if( nationRecno )
               info.default_viewing_nation_recno = nationRecno;
         }
         else if( unit_array.selected_recno )
         {
            int nationRecno = unit_array[unit_array.selected_recno]->nation_recno;

            if( nationRecno )
               info.default_viewing_nation_recno = nationRecno;
         }
         break;

      case 'a':
         if( view_mode==MODE_AI_ACTION )
            set_view_mode(MODE_NORMAL);
         else
            set_view_mode(MODE_AI_ACTION);
         break;

      //-----------------------------------//
/*
      case 't':   // next town layout
         if( town_array.selected_recno )
            town_array[town_array.selected_recno]->auto_set_layout();
         break;
*/
      //-------------------------------//

      case 'i':
         config.disable_ai_flag = !config.disable_ai_flag;

         if( config.disable_ai_flag )
            box.msg( "AI is now disabled" );
         else
            box.msg( "AI is now enabled" );
         break;

      case 'd':
         config.show_ai_info = !config.show_ai_info;
         info.disp();

         if( config.show_ai_info )
            box.msg( "Now AI info will be displayed." );
         else
            box.msg( "Now AI info will not be displayed." );
         break;

      case '/':
         config.show_all_unit_icon = !config.show_all_unit_icon;

         if( config.show_all_unit_icon )
            box.msg( "Now all unit icons will be displayed." );
         else
            box.msg( "Now all unit icons will not be displayed." );
         break;

#ifdef DEBUG
      case '~':
         sys.testing_session = !sys.testing_session;

         if( sys.testing_session )
            box.msg( "sys.testing_session is now 1." );
         else
            box.msg( "sys.testing_session is now 0." );
         break;

      case '\r':
         if(debug2_enable_flag)
            debug2_enable_flag = 0;
         else
            debug2_enable_flag = 1;
         break;

/*    //-*********** syn game test ***********-//
      case '\'':
         //if(debug2_enable_flag && debug_sim_game_type)
         //game_file_array[0]->load_game("syn.sav");
         game_file.load_game("syn.sav");
         sp_load_seed_file();
         debug_seed_status_flag = DEBUG_SYN_AUTO_LOAD;
         break;

      case '[':
         if(m.is_file_exist("SYN.SYS"))
         {
            debug_seed_status_flag = DEBUG_SYN_AUTO_SAVE;
            sp_seed_pos_reset();
            sp_record_match_seed();
            sp_create_seed_file("nseed.rs");

            game_file.save_game("syn.sav");
         }
         break;

      case ']':
         if(debug_seed_status_flag==NO_DEBUG_SYN)
         {
            if(m.is_file_exist("SYN.SYS"))
            {
               debug_seed_status_flag = DEBUG_SYN_LOAD_AND_COMPARE_ONCE;
               game_file.load_game("syn.sav");
               sp_load_seed_file();
            }
            else
               debug_seed_status_flag = NO_DEBUG_SYN;
         }
         break;
*/       //-*********** syn game test ***********-//
#endif
   }
}
//--------- End of function Sys::detect_debug_cheat_key ---------//


//-------- Start of function detect_scenario_cheat_key -------------//

static int detect_scenario_cheat_key(unsigned scanCode, unsigned skeyState)
{
#ifdef USE_DPLAY
   if( remote.is_enable() )      // no cheat keys in multiplayer games
      return 0;
#endif

   int keyCode = mouse.is_key(scanCode, skeyState, (WORD) 0, K_IS_CTRL);

   if( !keyCode )
      return 0;

   //------------------------------------------//

   int keyProcessed = 0;

   Firm *firmPtr;
   Unit *unitPtr;
   Town *townPtr;
   Nation *nationPtr;
   Site *sitePtr;
   Spy *spyPtr;
   Location *locPtr;
   int i, j, curXLoc, curYLoc;

   switch(keyCode)
   {
      case 'p': //-------- get scroll of power for the race of selected unit --------//
         if(unit_array.selected_recno)
         {
            unitPtr = unit_array[unit_array.selected_recno];
            if(unitPtr->nation_recno==nation_array.player_recno && unitPtr->race_id)
            {
               god_res[unitPtr->race_id]->enable_know(unitPtr->nation_recno);
               //box.msg( "Get Scroll of Power of selected unit for his race" );
               keyProcessed++;
            }
         }
         keyProcessed++;
         break;

      case 't': //-------- get all technology except scrolls of power -------//
         tech_res.inc_all_tech_level(nation_array.player_recno);
         //box.msg( "Your technology has advanced." );
         keyProcessed++;
         break;

      case 'g': //------- get galleon and cannon technologies -------//
         err_when(tech_res[4]->unit_id != UNIT_CANNON);
         err_when(tech_res[7]->unit_id != UNIT_GALLEON);

         i = tech_res[4]->get_nation_tech_level(nation_array.player_recno);
         if(i < tech_res[4]->max_tech_level)
            tech_res[4]->set_nation_tech_level(nation_array.player_recno, i+1);

         i = tech_res[7]->get_nation_tech_level(nation_array.player_recno);
         if(i < tech_res[7]->max_tech_level)
            tech_res[7]->set_nation_tech_level(nation_array.player_recno, i+1);

         //box.msg( "Get technologies of Galleon and Cannon." );
         keyProcessed++;
         break;

      case 'q': //-- decrease population of a selected race in a selected village by 10 --//
         if(town_array.selected_recno)
         {
            townPtr = town_array[town_array.selected_recno];
            if(townPtr->nation_recno == nation_array.player_recno)
            {
               i = townPtr->get_selected_race();
               if(i && townPtr->race_pop_array[i-1])
               {
                  for(j=10; j>0 && !town_array.is_deleted(townPtr->town_recno); --j)
                     townPtr->kill_town_people(i);

                  //box.msg( "Population decrease by 10." );
                  keyProcessed++;
               }
               townPtr->auto_set_layout();
            }
         }
         keyProcessed++;
         break;

      case 'w': //-- increase population of a selected race in a selected village by 10 --//
         if(town_array.selected_recno)
         {
            townPtr = town_array[town_array.selected_recno];
            if(townPtr->nation_recno == nation_array.player_recno)
            {
               i = townPtr->get_selected_race();
               if(i && townPtr->race_pop_array[i-1])
               {
                  townPtr->init_pop(i, 10, 100);
                  //box.msg( "Population increase by 10." );
                  keyProcessed++;
               }
               townPtr->auto_set_layout();
            }
         }
         keyProcessed++;
         break;

      case 'e': //-------- decrease the reputation by 10 -----------//
         nationPtr = nation_array[nation_array.player_recno];
         nationPtr->reputation -= 10;
         if(nationPtr->reputation < -100)
            nationPtr->reputation = (float) -100;

         //box.msg( "Reputation decrease by 10." );
         keyProcessed++;
         break;

      case 'r': //-------- increase the reputation by 10 -----------//
         nationPtr = nation_array[nation_array.player_recno];
         nationPtr->reputation += 10;
         if(nationPtr->reputation > 100)
            nationPtr->reputation = (float) 100;

         //box.msg( "Reputation increase by 10." );
         keyProcessed++;
         break;

      case 'j': //--------- damage a building by 20 pt -----------//
         if(firm_array.selected_recno)
         {
            firmPtr = firm_array[firm_array.selected_recno];
            if(firmPtr->nation_recno==nation_array.player_recno)
            {
               firmPtr->hit_points -= 20;
               if(firmPtr->hit_points < 1)
                  firmPtr->hit_points = (float) 1;
               //box.msg( "damage firm by 20 points." );
               keyProcessed++;
            }
         }
         keyProcessed++;
         break;

      case 'k': //--------- repair a building by 20 pt -----------//
         if(firm_array.selected_recno)
         {
            firmPtr = firm_array[firm_array.selected_recno];
            if(firmPtr->nation_recno==nation_array.player_recno)
            {
               firmPtr->hit_points += 20;
               if(firmPtr->hit_points > firmPtr->max_hit_points)
                  firmPtr->hit_points = firmPtr->max_hit_points;
               //box.msg( "Repair firm by 20 points." );
               keyProcessed++;
            }
         }
         keyProcessed++;
         break;

      case 'x': //------ decrease cash by 1000 --------//
         nationPtr = nation_array[nation_array.player_recno];
         nationPtr->cash -= 1000;
         if(nationPtr->cash < 0)
            nationPtr->cash = (float) 0;
         //box.msg( "Decrease cash by 1000." );
         keyProcessed++;
         break;

      case 'c': //------ decrease food by 1000 --------//
         nationPtr = nation_array[nation_array.player_recno];
         nationPtr->food -= 1000;
         if(nationPtr->food < 0)
            nationPtr->food = (float) 0;
         //box.msg( "Decrease food by 1000." );
         keyProcessed++;
         break;

      case 'm': //----- add natural resource to cursor pos / remove existing resource ------//
         if(get_mouse_loc_in_zoom_map(curXLoc, curYLoc))
         {
            locPtr = world.get_loc(curXLoc, curYLoc);
            if(locPtr->has_site()) // remove site
            {
               i = locPtr->site_recno();
               sitePtr = site_array[i];
               if(!sitePtr->has_mine)
               {
                  site_array.del_site(i);
                  //box.msg( "Site deleted." );
                  keyProcessed++;
               }
            }
            else if(locPtr->can_build_site(1) && !locPtr->is_power_off()) // add site
            {
               i = MAX_RAW_RESERVE_QTY * (50 + m.random(50)) / 100;
               site_array.add_site(curXLoc, curYLoc, SITE_RAW, m.random(MAX_RAW)+1, i);
               //box.msg( "Site added." );
               keyProcessed++;
            }
         }
         keyProcessed++;
         break;

      case 'b': //------------ add reserve of natural resource by 100 ----------//
         if(get_mouse_loc_in_zoom_map(curXLoc, curYLoc))
         {
            locPtr = world.get_loc(curXLoc, curYLoc);
            if(locPtr->has_site())
            {
               i = locPtr->site_recno();
               sitePtr = site_array[i];
               if(!sitePtr->has_mine)
               {
                  sitePtr->reserve_qty += 100;
                  //box.msg( "increase reserve by 100." );
                  keyProcessed++;

                  info.disp();
               }
            }
         }
         keyProcessed++;
         break;

      case 'v': //------------ reduce reserve of natural resource by 100 ----------//
         if(get_mouse_loc_in_zoom_map(curXLoc, curYLoc))
         {
            locPtr = world.get_loc(curXLoc, curYLoc);
            if(locPtr->has_site())
            {
               i = locPtr->site_recno();
               sitePtr = site_array[i];
               if(!sitePtr->has_mine && sitePtr->reserve_qty>100)
               {
                  sitePtr->reserve_qty -= 100;
                  //box.msg( "reduce reserve by 100." );
                  keyProcessed++;

                  info.disp();
               }
            }
         }
         keyProcessed++;
         break;

      case 'h': //-------- hide map except for areas around your village, people --------//
         if( config.explore_whole_map )      // no action if the setting of the map is explored
            break;

         vga_back.bar(MAP_X1, MAP_Y1, MAP_X2, MAP_Y2, UNEXPLORED_COLOR);
         for(j=0; j<MAX_WORLD_Y_LOC; ++j)
         {
            locPtr = world.get_loc(0, j);
            for(i=0; i<MAX_WORLD_X_LOC; ++i, locPtr++)
               locPtr->explored_off();
         }

         for(i=town_array.size(); i>0; --i)
         {
            if(town_array.is_deleted(i))
               continue;

            townPtr = town_array[i];
            if(townPtr->nation_recno == nation_array.player_recno)
               world.unveil(townPtr->loc_x1, townPtr->loc_y1, townPtr->loc_x2, townPtr->loc_y2);
         }

         for(i=firm_array.size(); i>0; --i)
         {
            if(firm_array.is_deleted(i))
               continue;

            firmPtr = firm_array[i];
            if(firmPtr->nation_recno == nation_array.player_recno)
               world.unveil(firmPtr->loc_x1, firmPtr->loc_y1, firmPtr->loc_x2, firmPtr->loc_y2);
         }

         for(i=unit_array.size(); i>0; --i)
         {
            if(unit_array.is_deleted(i))
               continue;

            unitPtr = unit_array[i];
            if(unitPtr->nation_recno == nation_array.player_recno)
               world.unveil(unitPtr->next_x_loc(), unitPtr->next_y_loc(), unitPtr->next_x_loc(), unitPtr->next_y_loc());
         }

         for(i=spy_array.size(); i>0; --i)
         {
            if(spy_array.is_deleted(i))
               continue;

            spyPtr = spy_array[i];
            if(spyPtr->true_nation_recno!=nation_array.player_recno)
               continue;

            if(spyPtr->spy_place == SPY_FIRM)
            {
               if(!firm_array.is_deleted(spyPtr->spy_place_para))
               {
                  firmPtr = firm_array[spyPtr->spy_place_para];
                  world.unveil(firmPtr->loc_x1, firmPtr->loc_y1, firmPtr->loc_x2, firmPtr->loc_y2);
               }
            }
            else if(spyPtr->spy_place == SPY_TOWN)
            {
               if(!town_array.is_deleted(spyPtr->spy_place_para))
               {
                  townPtr = town_array[spyPtr->spy_place_para];
                  world.unveil(townPtr->loc_x1, townPtr->loc_y1, townPtr->loc_x2, townPtr->loc_y2);
               }
            }
         }

         for(i=2; i<=nation_array.size(); ++i) // assume player_nation_recno = 1
         {
            if( nation_array.is_deleted(i) )
               continue;

            (~nation_array)->init_relation(i);
            nation_array[i]->init_relation(1);
         }

         keyProcessed++;
         break;

      case 'z': //------------ put the selected unit to the cursor position ------------//
         if(unit_array.selected_recno)
         {
            unitPtr = unit_array[unit_array.selected_recno];
            if(get_mouse_loc_in_zoom_map(curXLoc, curYLoc))
            {
               if(unitPtr->mobile_type!=UNIT_LAND)
               {
                  curXLoc = (curXLoc/2) * 2;
                  curYLoc = (curYLoc/2) * 2;
               }

               locPtr = world.get_loc(curXLoc, curYLoc);
               if(locPtr->can_move(unitPtr->mobile_type))
               {
                  world.set_unit_recno(unitPtr->next_x_loc(), unitPtr->next_y_loc(), unitPtr->mobile_type, 0);
                  unitPtr->stop2();
                  unitPtr->next_x = curXLoc << ZOOM_X_SHIFT_COUNT;
                  unitPtr->next_y = curYLoc << ZOOM_Y_SHIFT_COUNT;
                  unitPtr->cur_x = unitPtr->go_x = unitPtr->next_x;
                  unitPtr->cur_y = unitPtr->go_y = unitPtr->next_y;
                  unitPtr->move_to_x_loc = curXLoc;
                  unitPtr->move_to_y_loc = curYLoc;
                  world.set_unit_recno(curXLoc, curYLoc, unitPtr->mobile_type, unitPtr->sprite_recno);
                  //box.msg( "move unit." );
                  keyProcessed++;
               }
            }
         }
         keyProcessed++;
         break;

      case 's':
         if( info.default_viewing_nation_recno &&
             nation_array.player_recno &&
             nation_array[info.default_viewing_nation_recno]->is_ai() )
         {
            nation_array[info.default_viewing_nation_recno]->surrender(nation_array.player_recno);
         }
         keyProcessed++;
         break;
   }

   return keyProcessed;
}
//--------- End of function detect_scenario_cheat_key ---------------//


//-------- Begin of function Sys::detect_set_speed --------//
//
int Sys::detect_set_speed(unsigned scanCode, unsigned skeyState)
{
   int keyCode = mouse.is_key( scanCode, skeyState, (WORD) 0, K_CHAR_KEY );

   if( !keyCode )    // since all keys concern are printable
      return 0;

   //------- determine the speed to set of the key pressed -------//

   if( keyCode >= '0' && keyCode <= '8' )
   {
      set_speed( (keyCode-'0') * 3 );
      return 1;
   }

   else if( keyCode == '9' )
   {
      set_speed( 99 );           // highest possible speed
      return 1;
   }

   return 0;
}
//--------- End of function Sys::detect_set_speed ---------//


//--------- Begin of function Sys::detect_key_str --------//
//
// Detect for continous input of a string from the keyboard
//
// <int>   keyStrId = the id. of the key string
//                    each id has its individual key_str_pos
// <char*> keyStr   = the string to detect
//
// return : <int> 1 - complete string detected
//                0 - not detected
//
int Sys::detect_key_str(int keyStrId, const char* keyStr)
{
   err_when( keyStrId < 0 || keyStrId >= MAX_KEY_STR );

   unsigned char* keyStr2 = (unsigned char*) keyStr;

   if( mouse.key_code == keyStr2[key_str_pos[keyStrId]] )
      key_str_pos[keyStrId]++;
   else
      key_str_pos[keyStrId]=0;    // when one key unmatched, reset the counter

   if( key_str_pos[keyStrId] >= (int) strlen(keyStr) )
   {
      key_str_pos[keyStrId]=0;    // the full string has been entered successfully without any mistakes
      return 1;
   }
   else
      return 0;
}
//----------- End of function Sys::detect_key_str --------//


//-------- Begin of function Sys::set_speed --------//
//
void Sys::set_speed(int frameSpeed, int remoteCall)
{
   //--------- if multiplayer, update remote players setting -------//

#ifdef USE_DPLAY
   if( remote.is_enable() && !remoteCall )
   {
      RemoteMsg *remoteMsg = remote.new_msg(MSG_SET_SPEED, sizeof(short));

      *((short*)remoteMsg->data_buf) = frameSpeed;

      remote.send_free_msg( remoteMsg );     // send out the message and free it after finishing sending
   }
#endif

   //---------- set the speed now ----------//

   if( config.frame_speed==0 )                   // if it's currently frozen, set last_frame_time to avoid incorrect timeout
      last_frame_time = m.get_time();

   config.frame_speed = frameSpeed;
}
//--------- End of function Sys::set_speed ---------//


//-------- Begin of function Sys::capture_screen --------//
//
void Sys::capture_screen()
{
   String str("7K");

   int i;
   for( i=0 ; i<=99 ; i++ )
   {
      str  = "7K";

      if( i<10 )
         str += "0";

      str += i;
      str += ".BMP";

      if( !m.is_file_exist(str) )
         break;
   }

   if( i>99 )        // all file names from DWORLD00 to DWORLD99 have been occupied
      return;

   if( sys.debug_session )    // in debug session, the buffer is not locked, we need to lock it for capturing the screen
   {
      vga_true_front.lock_buf();
      vga_true_front.write_bmp_file(str);
      vga_true_front.unlock_buf();
   }
   else
   {
      vga_front.write_bmp_file(str);
   }

   //------ display msg --------//

   String str2;

#if(defined(SPANISH))
	str2  = "Pantalla actual guardada en el archivo ";
	str2 += str;
	str2 += ".";
#elif(defined(FRENCH))
	str2  = "Cet écran a été sauvegardé dans le fichier ";
   str2 += str;
   str2 += ".";
#else
   str2  = "The current screen has been written to file ";
   str2 += str;
   str2 += ".";
#endif

   box.msg( str2 );
}
//--------- End of function Sys::capture_screen ---------//


//-------- Begin of function Sys::load_game --------//
//
void Sys::load_game()
{
#ifdef USE_DPLAY
   //--- load game not enabled in multiplayer game ---//

   if( remote.is_enable() )
      return;
#endif

   signal_exit_flag=1;     // for deinit functions to recognize that this is an end game deinitialization instead of a normal deinitialization

   int rc=0;

   game_file_array.init("*.SAV");                  // reload any save game file
   game_file_array.menu(-2);               // save screen area to back buffer
   switch( game_file_array.menu(2) )
   {
      case 1:
         rc = 1;                 // fall through to case 0

      case 0:
         signal_exit_flag = 0;
         break;
         // case -1 and otherwise, left sys.signal_exit_flag 1 to exit the game
   }

   game_file_array.menu(-1);               // restore screen area from back buffer

   //-----------------------------------//
   if( rc == -1)
   {
      box.msg( "Fail Loading Game" );
      return;
   }

   if( rc )    // if rc==0, leave signal_exit_flag 1, which the game will then quit
   {
      need_redraw_flag = 1;
      disp_frame();
      // #### begin Gilbert 22/10 ######//
      disp_view_mode();
      // #### end Gilbert 22/10 ######//
      box.msg( "Game Loaded Successfully" );
      signal_exit_flag=0;
      info.disp();
   }
}
//--------- End of function Sys::load_game ---------//


//-------- Begin of function Sys::save_game --------//
//
void Sys::save_game()
{
   if( nation_array.player_recno==0 )     // cannot save game when the player's kingdom has been destroyed
      return;

#ifdef USE_DPLAY
   if( remote.is_enable() )
   {
      DWORD *dwordPtr = (DWORD *)remote.new_send_queue_msg( MSG_REQUEST_SAVE, sizeof(DWORD) );
      *dwordPtr = remote.next_send_frame(nation_array.player_recno, sys.frame_count+remote.process_frame_delay)+2;
      return;
   }
#endif

   game_file_array.init("*.SAV");                  // reload any save game file
   game_file_array.menu(-2);               // save screen area to back buffer

   if( game_file_array.menu(1) == 1 )
   {
      if( GameFile::last_read_success_flag )
         box.msg( "Game Saved Successfully" );
   }

   game_file_array.menu(-1);               // restore screen area from back buffer

	// ##### patch begin Gilbert 16/3 #######//
	info.disp();
	// ##### patch end Gilbert 16/3 #######//
}
//-------- End of function Sys::save_game --------//


// --------- begin of function Sys::mp_request_save ----------//
void Sys::mp_request_save(DWORD frame)
{
   if( !mp_save_flag )
   {
      mp_save_flag = 1;
      mp_save_frame = frame;
   }
}
// --------- end of function Sys::mp_request_save ----------//


// --------- begin of function Sys::mp_clear_request_save ----------//
void Sys::mp_clear_request_save()
{
   mp_save_flag = 0;
   mp_save_frame = 0;
}
// --------- end of function Sys::mp_clear_request_save ----------//


//-------- Begin of function Sys::set_game_dir ----------//
//
// Set all game directories.
//
void Sys::set_game_dir()
{
   //------- If it should run from the CDROM ------//

   get_cdrom_drive();

   set_one_dir( "IMAGE\\HALLFAME.ICN"    , "IMAGE\\", dir_image );
   set_one_dir( "ENCYC\\SEAT\\NORMAN.ICN", "ENCYC\\", dir_encyc );
#ifdef AMPLUS
   set_one_dir( "ENCYC2\\SEAT\\EGYPTIAN.ICN", "ENCYC2\\", dir_encyc2 );
#endif
   set_one_dir( "MOVIE\\INTRO.AVI"       , "MOVIE\\", dir_movie );

#ifdef DEMO
   set_one_dir( "MUSIC\\DEMO.WAV"        , "MUSIC\\", dir_music );
   set_one_dir( "TUTORIAL\\STANDARD.TUT" , "TUTORIAL\\", dir_tutorial );
   set_one_dir( "SCENARIO\\DEMO.SCN"     , "SCENARIO\\", dir_scenario );
#else
   set_one_dir( "MUSIC\\NORMAN.WAV"      , "MUSIC\\", dir_music );
   set_one_dir( "TUTORIAL\\1BAS_MIL.TUT" , "TUTORIAL\\", dir_tutorial );
   set_one_dir( "SCENARIO\\7FOR7.SCN"    , "SCENARIO\\", dir_scenario );
#endif

#if(MAX_SCENARIO_PATH >= 2)
   set_one_dir( "SCENARI2\\SCN_01.SCN"    , "SCENARI2\\", dir_scenario_path[1] );
#endif

   //-------- set game version ---------//

   #ifdef BETA
      game_version = VERSION_FULL;
   #else
      #ifdef DEMO
         game_version = VERSION_DEMO;
      #else
         if( 1 ) // no longer checkcd if( cdrom_drive )
            game_version = VERSION_FULL;     // single player game is not available when game_version == VERSION_FULL
         else
            game_version = VERSION_MULTIPLAYER_ONLY;
      #endif
   #endif
}
//----------- End of function Sys::set_game_dir ----------//


//-------- Begin of function Sys::set_one_dir ----------//
//
int Sys::set_one_dir( const char* checkFileName, const char* defaultDir, char* trueDir )
{
   if( m.is_file_exist( checkFileName ) )
   {
      strcpy( trueDir, defaultDir );
   }
   else
   {
      if( cdrom_drive )
      {
         strcpy( trueDir, "D:\\" );
         strcat( trueDir, defaultDir );

         trueDir[0] = cdrom_drive;
      }
      else
      {
         strcpy( trueDir, "" );
         return 0;
      }
   }

   return 1;
}
//----------- End of function Sys::set_one_dir ----------//


//-------- Start of function Sys::get_cdrom_drive -------------//
//
// Get the drive letter of the CDROM and restore the result in cdrom_drive.
//
void Sys::get_cdrom_drive()
{
   unsigned char  i;
   char  driveStr[4];
   static char checkFileName[30] = "D:\\7K.EXE";        // check this file to identify the disc

   cdrom_drive = 0;

   driveStr[1] = ':';
   driveStr[2] = '\\';
   driveStr[3] = 0;

	// ##### patch begin Gilbert 14/10 ######//
   for(i='C'; i<='Z'; i++)
	// ##### patch end Gilbert 14/10 ######//
   {
      checkFileName[0] = i;

      driveStr[0] = i;

      if(GetDriveType(driveStr) == DRIVE_CDROM)
      {
         if( m.is_file_exist(checkFileName) )
         {
            cdrom_drive = i;
            break;
         }
      }
   }
}
//--------- End of function Sys::get_cdrom_drive ---------------//


//-------- Start of function locate_king_general -------------//
//
static void locate_king_general(int rankId)
{
   if( !nation_array.player_recno )
      return;

   int unitRecno = 0;
   if(unit_array.selected_recno)
      unitRecno = unit_array.selected_recno;
   else if(rankId!=RANK_KING && firm_array.selected_recno)
   {
      Firm *firmPtr = firm_array[firm_array.selected_recno];
      if((firmPtr->firm_id==FIRM_CAMP || firmPtr->firm_id==FIRM_BASE) && firmPtr->overseer_recno)
         unitRecno = firmPtr->overseer_recno;
   }

   for( int i=unit_array.size() ; i>0 ; i-- )
   {
      if( ++unitRecno > unit_array.size() )
         unitRecno = 1;

      if( unit_array.is_deleted(unitRecno) )
         continue;

      Unit* unitPtr = unit_array[unitRecno];

      if( unitPtr->nation_recno == nation_array.player_recno &&
          unitPtr->rank_id == rankId )
      {
         short xLoc, yLoc;

         if( unitPtr->get_cur_loc(xLoc, yLoc) )
         {
            world.go_loc(xLoc, yLoc, 1);
            return;
         }
      }
   }
}
//--------- End of function locate_king_general ---------------//


//-------- Start of function locate_spy -------------//
//
static void locate_spy()
{
   if( !nation_array.player_recno )
      return;

   int unitRecno = unit_array.selected_recno;

   for( int i=unit_array.size() ; i>0 ; i-- )
   {
      if( ++unitRecno > unit_array.size() )
         unitRecno = 1;

      if( unit_array.is_deleted(unitRecno) )
         continue;

      Unit* unitPtr = unit_array[unitRecno];

      if( unitPtr->true_nation_recno() == nation_array.player_recno &&
          unitPtr->spy_recno && unitPtr->is_visible() )
      {
         short xLoc, yLoc;

         if( unitPtr->get_cur_loc(xLoc, yLoc) )
         {
            world.go_loc(xLoc, yLoc, 1);
            return;
         }
      }
   }
}
//--------- End of function locate_spy ---------------//


//-------- Start of function locate_ship -------------//
//
static void locate_ship()
{
   if( !nation_array.player_recno )
      return;

   //----------------------------------------------------------------------------//
   // The order is
   // 1) visible ships by their sprite_recno in ascending order
   // 2) FirmHarbors with ships (num of ships >= 1) by their firm_recno in ascending
   //    order
   //
   // Start the scaning in one of the following cases
   // 1) a unit is selected
   // 2) a firm is selected
   // 3) neither unit or firm is selected
   //----------------------------------------------------------------------------//

   if(firm_array.selected_recno)
   {
      if(!locate_ship_in_harbor()) // harbor first, then unit
         locate_visible_ship();
   }
   else // if(unit_array.selected_recno) or neither of them is selected
   {
      if(!locate_visible_ship()) // unit first, then harbor
         locate_ship_in_harbor();
   }
}
//--------- End of function locate_ship ---------------//


//-------- Start of function locate_ship_in_harbor -------------//
// return 1 if found
// return 0 otherwise
//
static int locate_ship_in_harbor()
{
   int firmRecno = firm_array.selected_recno;
   int checkSize = firm_array.size();
   if(firmRecno)
      checkSize--; // not include the selected firm

   for( int i=checkSize ; i>0 ; i-- )
   {
      if( ++firmRecno > firm_array.size() )
         firmRecno = 1;

      if( firm_array.is_deleted(firmRecno) )
         continue;

      Firm* firmPtr = firm_array[firmRecno];
      if(firmPtr->firm_id!=FIRM_HARBOR ||
         firmPtr->nation_recno != nation_array.player_recno)
         continue;

      if(((FirmHarbor*)firmPtr)->ship_count==0)
         continue; // not interested

      world.go_loc(firmPtr->center_x, firmPtr->center_y, 1);
      return 1;
   }
   return 0;
}
//--------- End of function locate_ship_in_harbor ---------------//


//-------- Start of function locate_visible_ship -------------//
// return 1 if found
// return 0 otherwise
//
static int locate_visible_ship()
{
   int unitRecno = unit_array.selected_recno;
   int checkSize = unit_array.size();
   if(unitRecno)
      checkSize--; // not include the selected unit

   for( int i=checkSize ; i>0 ; i-- )
   {
      if( ++unitRecno > unit_array.size() )
         unitRecno = 1;

      if( unit_array.is_deleted(unitRecno) )
         continue;

      Unit* unitPtr = unit_array[unitRecno];
      if(!unitPtr->is_visible()) // skip the case unit_mode==UNIT_MODE_IN_HARBOR in calling Unit::get_cur_loc()
         continue;

      if( unitPtr->nation_recno == nation_array.player_recno &&
          unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_SHIP )
      {
         short xLoc, yLoc;

         if( unitPtr->get_cur_loc(xLoc, yLoc) )
         {
            world.go_loc(xLoc, yLoc, 1);
            return 1;
         }
      }
   }

   return 0;
}
//--------- End of function locate_visible_ship ---------------//


//-------- Start of function locate_camp -------------//
//
static void locate_camp()
{
   if( !nation_array.player_recno )
      return;

   int firmRecno = firm_array.selected_recno;

   for( int i=firm_array.size() ; i>0 ; i-- )
   {
      if( ++firmRecno > firm_array.size() )
         firmRecno = 1;

      if( firm_array.is_deleted(firmRecno) )
         continue;

      Firm* firmPtr = firm_array[firmRecno];

      if( firmPtr->nation_recno == nation_array.player_recno &&
          firmPtr->firm_id == FIRM_CAMP )
      {
         world.go_loc(firmPtr->center_x, firmPtr->center_y, 1);
         return;
      }
   }
}
//--------- End of function locate_camp ---------------//


//-------- Start of function get_mouse_loc_in_zoom_map -------------//
static int get_mouse_loc_in_zoom_map(int &x, int &y)
{
   int mouseX = mouse.cur_x;
   int mouseY = mouse.cur_y;
   if(mouseX >= ZOOM_X1 && mouseX <= ZOOM_X2 && mouseY >= ZOOM_Y1 && mouseY <= ZOOM_Y2)
   {
      x = world.zoom_matrix->top_x_loc + (mouseX-ZOOM_X1)/ZOOM_LOC_WIDTH;
      y = world.zoom_matrix->top_y_loc + (mouseY-ZOOM_Y1)/ZOOM_LOC_HEIGHT;
      return 1;
   }

   return 0; // out of zoom map boundary
}
//--------- End of function get_mouse_loc_in_zoom_map ---------------//
