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

//Filename    : OGAMEMP.CPP
//Description : Main Game Object - Multiplayer Game (using Imagic multiplayer SDK)

#include <OSYS.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OBOX.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OFONT.h>
#include <OBUTT3D.h>
#include <ONATION.h>
#include <ORACERES.h>
#include <OREMOTE.h>
#include <OBATTLE.h>
#include <OGAME.h>
#include <netplay.h>
#include <OERRCTRL.h>
#include <OGFILE.h>
#include <OCONFIG.h>
#include <OIMGRES.h>
#include <OGET.h>
#include <KEY.h>
#include <OMUSIC.h>
#include <OBUTTCUS.h>
#include <OCOLTBL.h>
#include <OGETA.h>
#include <OSLIDCUS.h>
#include <OBLOB.h>

// --------- Define constant --------//

#define PLAYER_RATIO_CDROM 3
#define PLAYER_RATIO_NOCD -1
#define PLAYER_RATIO_STRING "four"
#define FORCE_MAX_FRAME_DELAY 5

enum { OPTION_BASIC,
		 OPTION_ADVANCED,
		 OPTION_ADVANCE2,
		 OPTION_GOAL,
	  };

enum { BASIC_OPTION_X_SPACE = 78,
		 BASIC_OPTION_HEIGHT = 32 };

enum { COLOR_OPTION_X_SPACE = 35,
		 COLOR_OPTION_HEIGHT = 32 };

enum { SERVICE_OPTION_X_SPACE = 180,
		 SERVICE_OPTION_HEIGHT = 139 };

static char race_table[MAX_RACE] =		// race translation table
#if(MAX_RACE == 10)
{
	RACE_CHINESE, RACE_EGYPTIAN, RACE_GREEK, RACE_JAPANESE, RACE_MAYA,
	RACE_INDIAN, RACE_NORMAN, RACE_PERSIAN, RACE_VIKING, RACE_ZULU
};
#else
{
	RACE_CHINESE, RACE_GREEK, RACE_JAPANESE, RACE_MAYA,
	RACE_PERSIAN, RACE_NORMAN, RACE_VIKING
};
#endif

static char reverse_race_table[MAX_RACE] =		// race translation table
#if(MAX_RACE == 10)
{
	6, 4, 2, 8, 7, 0, 3, 1, 5, 9
};
#else
{
	5, 3, 1, 6, 4, 0, 2
};
#endif

static char sub_game_mode;		// 0 = new multiplayer game, 1 = load multiplayer game
static void disp_virtual_button(ButtonCustom *, int);
static void disp_virtual_tick(ButtonCustom *, int);
static void disp_scroll_bar_func(SlideVBar *scroll, int);


enum
{
	MPMSG_START_GAME = 0x1f5a0001,
	MPMSG_ABORT_GAME,
	MPMSG_RANDOM_SEED,			// see MpStructSeed
	MPMSG_RANDOM_SEED_STR,
	MPMSG_DECLARE_NATION,		// see MpStructNation
	MPMSG_END_SETTING,
	MPMSG_START_LOAD_GAME,
	MPMSG_ACCEPT_DECLARE_NATION,		// used in loading multiplayer game
	MPMSG_REFUSE_DECLARE_NATION,
	MPMSG_SEND_CONFIG,

	MPMSG_NEW_PLAYER,
	MPMSG_ACCEPT_NEW_PLAYER,
	MPMSG_REFUSE_NEW_PLAYER,
	MPMSG_ACQUIRE_COLOR,
	MPMSG_ACCEPT_COLOR,
	MPMSG_REFUSE_COLOR,
	MPMSG_ACQUIRE_RACE,
	MPMSG_ACCEPT_RACE,
	MPMSG_REFUSE_RACE,
	MPMSG_PLAYER_READY,
	MPMSG_PLAYER_UNREADY,
	MPMSG_LOAD_GAME_NEW_PLAYER,
	MPMSG_SEND_CHAT_MSG,

	// ### patch begin Gilbert 22/1 ######//
	// introduced since version 111
	MPMSG_SEND_SYNC_TEST_LEVEL,
	// ### patch begin Gilbert 22/1 ######//
	MPMSG_TEST_LATENCY_SEND,
	MPMSG_TEST_LATENCY_ECHO,
	MPMSG_SET_PROCESS_FRAME_DELAY,
};

struct MpStructBase
{
	DWORD msg_id;
	MpStructBase(DWORD msgId) : msg_id(msgId) {}
};

struct MpStructSeed : public MpStructBase
{
	long	seed;
	MpStructSeed(long s) : MpStructBase(MPMSG_RANDOM_SEED), seed(s) {}
};

struct MpStructSeedStr : public MpStructBase
{
	enum { RANDOM_SEED_MAX_LEN = 11 };
	char	seed_str[RANDOM_SEED_MAX_LEN+1];
	MpStructSeedStr(char *s) : MpStructBase(MPMSG_RANDOM_SEED_STR)
	{
		if(s)
			strcpy(seed_str, s);
		else
			seed_str[0] = '\0';
	}

	MpStructSeedStr(long l) : MpStructBase(MPMSG_RANDOM_SEED_STR)
	{
		ltoa(l, seed_str, 10);
	}
};

struct MpStructNation : public MpStructBase
{
	short nation_recno;
	DWORD dp_player_id;
	short color_scheme;
	short race_id;
	char  player_name[NationArray::HUMAN_NAME_LEN+1];

	MpStructNation() : MpStructBase(MPMSG_DECLARE_NATION) {}
	MpStructNation(short n, DWORD playerId, short scheme, short race,
		char *playerName):
		MpStructBase(MPMSG_DECLARE_NATION), nation_recno(n),
		dp_player_id(playerId), color_scheme(scheme), race_id(race)
		{
			strcpy(player_name, playerName);
		}
	void init(short n, DWORD playerId, short scheme, short race,
		char *playerName)
	{
		msg_id = MPMSG_DECLARE_NATION;
		nation_recno = n;
		dp_player_id = playerId;
		color_scheme = scheme;
		race_id = race;
		strcpy(player_name, playerName);
	}
};

struct MpStructConfig : public MpStructBase
{
	Config game_config;

	MpStructConfig(Config &c) : MpStructBase(MPMSG_SEND_CONFIG), game_config(c) {}
};


struct MpStructNewPlayer : public MpStructBase
{
	PID_TYPE player_id;
	short player_balance;	// 2 for CD-ROM version, -1 for non CD-ROM version
	MpStructNewPlayer(PID_TYPE p, short bal) : MpStructBase(MPMSG_NEW_PLAYER), player_id(p),
		player_balance(bal) {}
};

struct MpStructAcceptNewPlayer : public MpStructBase
{
	PID_TYPE player_id;
	MpStructAcceptNewPlayer(PID_TYPE p) : MpStructBase(MPMSG_ACCEPT_NEW_PLAYER), player_id(p) {}
};

struct MpStructRefuseNewPlayer : public MpStructBase
{
	PID_TYPE player_id;
	MpStructRefuseNewPlayer(PID_TYPE p) : MpStructBase(MPMSG_REFUSE_NEW_PLAYER), player_id(p) {}
};

struct MpStructAcquireColor : public MpStructBase
{
	short	color_scheme_id;
	MpStructAcquireColor(short c) : MpStructBase(MPMSG_ACQUIRE_COLOR), color_scheme_id(c) {}
};

struct MpStructAcceptColor : public MpStructBase
{
	PID_TYPE  request_player_id;
	short	color_scheme_id;
	MpStructAcceptColor(PID_TYPE p,short c) : MpStructBase(MPMSG_ACCEPT_COLOR),
		request_player_id(p), color_scheme_id(c) {}
};

struct MpStructRefuseColor : public MpStructBase
{
	PID_TYPE  request_player_id;
	short	color_scheme_id;
	MpStructRefuseColor(PID_TYPE p, short c) : MpStructBase(MPMSG_REFUSE_COLOR), 
		request_player_id(p), color_scheme_id(c) {}
};

struct MpStructAcquireRace : public MpStructBase
{
	short	race_id;
	MpStructAcquireRace(short raceId) : MpStructBase(MPMSG_ACQUIRE_RACE), race_id(raceId) {}
};

struct MpStructAcceptRace : public MpStructBase
{
	PID_TYPE  request_player_id;
	short	race_id;
	MpStructAcceptRace(PID_TYPE p,short raceId) : MpStructBase(MPMSG_ACCEPT_RACE),
		request_player_id(p), race_id(raceId) {}
};

struct MpStructRefuseRace : public MpStructBase
{
	PID_TYPE  request_player_id;
	short	race_id;
	MpStructRefuseRace(PID_TYPE p, short raceId) : MpStructBase(MPMSG_REFUSE_RACE), 
		request_player_id(p), race_id(raceId) {}
};

struct MpStructPlayerReady : public MpStructBase
{
	PID_TYPE player_id;
	MpStructPlayerReady(PID_TYPE p) : MpStructBase(MPMSG_PLAYER_READY), player_id(p) {}
};

struct MpStructPlayerUnready : public MpStructBase
{
	PID_TYPE player_id;
	MpStructPlayerUnready(PID_TYPE p) : MpStructBase(MPMSG_PLAYER_UNREADY), player_id(p) {}
};

struct MpStructLoadGameNewPlayer : public MpStructBase
{
	PID_TYPE player_id;
	short nation_recno;
	short color_scheme_id;
	short race_id;
	DWORD frame_count;			// detail to test save game from the same game
	long	random_seed;
	short player_balance;

	MpStructLoadGameNewPlayer(PID_TYPE p, Nation *n, DWORD frame, long seed, short bal) : 
		MpStructBase(MPMSG_LOAD_GAME_NEW_PLAYER), player_id(p),
		nation_recno(n->nation_recno), color_scheme_id(n->color_scheme_id),
		race_id(n->race_id), frame_count(frame), random_seed(seed), player_balance(bal) 
		{
		}
};

struct MpStructChatMsg : public MpStructBase
{
	enum { MSG_LENGTH = 60 };
	char sender[MP_FRIENDLY_NAME_LEN+1];
	char content[MSG_LENGTH+1];
	MpStructChatMsg(char *fromName, char *message) : MpStructBase(MPMSG_SEND_CHAT_MSG)
	{
		if( fromName )
			strcpy(sender, fromName);
		else
			sender[0] = '\0';
		if( message )
			strcpy(content, message);
		else
			content[0] = '\0';
	}
};

struct MpStructSyncLevel : public MpStructBase
{
	char sync_test_level;
	MpStructSyncLevel(char syncLevel) : MpStructBase(MPMSG_SEND_SYNC_TEST_LEVEL), sync_test_level(syncLevel)
	{
	}
};


struct MpStructLatencySend : public MpStructBase
{
	int test_id;
	DWORD send_time;
	MpStructLatencySend(int testId, DWORD sendTime) : MpStructBase(MPMSG_TEST_LATENCY_SEND),
		test_id(testId), send_time(sendTime)
	{
	}
};

struct MpStructLatencyReturn : public MpStructLatencySend
{
	MpStructLatencyReturn(const MpStructLatencySend &ls) : MpStructLatencySend(ls)
	{
		msg_id = MPMSG_TEST_LATENCY_ECHO;
	}
};


struct MpStructProcessFrameDelay : public MpStructBase
{
	int	common_process_frame_delay;
	MpStructProcessFrameDelay(int newFrameDelay) : MpStructBase(MPMSG_SET_PROCESS_FRAME_DELAY),
		common_process_frame_delay(newFrameDelay)
	{
	}
};

//--------- Define static functions ------------//

static void pregame_disconnect_handler(DWORD playerId);
static void ingame_disconnect_handler(DWORD playerId);

/*
//--------- Begin of function Game::mp_disp_player ---------//
void Game::mp_disp_players()
{
	enum { BUTTON_WIDTH=80, BUTTON_HEIGHT=22, BUTTON_Y_SPACE=26 };

	enum { BUTTON_NUM=8, BUTTON_TOP_X=620, BUTTON_TOP_Y=10 };

	//--------- display buttons -------//

	int 	 x=BUTTON_TOP_X;
	int 	 y=BUTTON_TOP_Y;
	Button buttonArray[BUTTON_NUM];

	mp_obj.poll_players();
	for( int i=0 ; i<BUTTON_NUM && mp_obj.get_player(i+1); i++, y+=BUTTON_Y_SPACE )
	{
		buttonArray[i].paint_text(x, y, x+BUTTON_WIDTH-1, y+BUTTON_HEIGHT-1, mp_obj.get_player(i+1)->friendly_name );
	}

}
//--------- End of function Game::mp_disp_players ---------//
*/

//-------- Begin of function Game::mp_broadcast_setting --------//
//
// Broadcast the latest game settings from the host to all clients.
// This function should be called by the host only
//
// see also to RemoteMsg::update_game_setting
//
void Game::mp_broadcast_setting()
{
	// send (long) random seed
	// send (short) no. of nations
	// for each nation, send :
	//	(short) nation recno
	// (DWORD) directPlay player id
	// (short) color scheme
	// (short) race id
	//
	short i;
	int msgSize = sizeof(long)	+sizeof(short) + 
		nation_array.size()*(3*sizeof(short)+sizeof(PID_TYPE));
	RemoteMsg *remoteMsg = remote.new_msg( MSG_UPDATE_GAME_SETTING, msgSize );

	char* dataPtr = remoteMsg->data_buf;

	*(long*)dataPtr   = m.get_random_seed();
	dataPtr 		      += sizeof(long);

	*(short*)dataPtr  = nation_array.size();
	dataPtr           += sizeof(short);

	for( i=1 ; i<=nation_array.size() ; i++)
	{
		*(short *)dataPtr = i;
		dataPtr += sizeof(short);
		*(PID_TYPE *)dataPtr = nation_array[i]->player_id;
		dataPtr += sizeof(PID_TYPE);
		*(short *)dataPtr = nation_array[i]->color_scheme_id;
		dataPtr += sizeof(short);
		*(short *)dataPtr = nation_array[i]->race_id;
		dataPtr += sizeof(short);
	}

	err_when(dataPtr - remoteMsg->data_buf > msgSize);

	remote.send_free_msg(remoteMsg);		// send out the message and free it after finishing sending
}
//--------- End of function Game::mp_broadcast_setting ---------//


//-------- Begin of function pregame_disconnect_handler --------//
//
// Host disconnection handler, called by Remote when one of the players
// disconnects from the game when it's still in multiplayer game setting
// menu.
//
static void pregame_disconnect_handler(DWORD playerId)
{
	int i;

	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array[i]->player_id == playerId )
		{
			((DynArray*)&nation_array)->linkout(i);
			break;
		}
	}
}
//--------- End of function pregame_disconnect_handler ---------//


//-------- Begin of function ingame_disconnect_handler --------//
//
// Host disconnection handler, called by Remote when one of the players
// disconnects from the game after the game has started.
//
static void ingame_disconnect_handler(DWORD playerId)
{
	int i;

	for( i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( nation_array[i]->player_id == playerId )
		{
			nation_array[i]->nation_type = NATION_AI;		//**BUGHERE, should have a function to set all units, structures of this nation to AI
			nation_array.ai_nation_count++;
      }
	}
}
//--------- End of function ingame_disconnect_handler ---------//


// --------- Begin of static function multi_player_game ----------//
// avoid creating local variable in this function
// ###### begin Gilbert 13/2 #######//
void Game::multi_player_game(char *cmdLine)
// ###### end Gilbert 13/2 #######//
{
	sub_game_mode = 0;
	info.init_random_seed(0);			// initialize the random seed

	int choice, p;
	ProtocolType selected_protocol = TCPIP;

	if (!cmdLine) {
		// not launched from lobby

		mp_obj.poll_supported_protocols();
		choice = mp_select_service();
		if( !choice )
		{
			mp_obj.deinit();
			return;
		}

		switch(choice)
		{
		case 1:	// IPX
			selected_protocol = IPX;
			break;
		case 2:	// TCP/IP
			selected_protocol = TCPIP;
			break;
		case 3:	// Modem
			selected_protocol = Modem;
			break;
		case 4:	// Serial
			selected_protocol = Serial;
			break;
		default:
			selected_protocol = None;
		}
	}

	if (mp_obj.is_protocol_supported(selected_protocol))
		mp_obj.init(selected_protocol);

	if (cmdLine)
		mp_obj.init_lobbied(MAX_NATION, cmdLine);

	// do not call remote.init here, or sys.yield will call remote.poll_msg
	if(!mp_obj.is_initialized())
	{
		// BUGHERE : display error message
		box.msg("Cannot initialize DirectPlay.");
		mp_obj.deinit();
		return;
	}

	// create game or join game
	switch( mp_select_mode(NULL) )
	{
	case 1:		// create game
		// BUGHERE : enter session name here and player name here
		if( mp_obj.create_session(config.player_name, MAX_NATION) 
			&& mp_obj.create_player(config.player_name, config.player_name) )
		{
			remote.init(&mp_obj);
			remote.create_game();
		}
		else
		{
			mp_obj.deinit();
			return;
		}
		break;
	case 2:		// join game
		if( (choice = mp_select_session()) )
		{
			if( mp_obj.join_session(choice) 
				&& mp_obj.create_player(config.player_name, config.player_name) )
			{
				remote.init(&mp_obj);
				remote.connect_game();
			}
			else
			{
				// can't join session
				// BUGHERE : display error message here
				mp_obj.deinit();
				return;
			}
		}
		else
		{
			mp_obj.deinit();
			return;
		}
		break;
	default:			// cancel
		mp_obj.deinit();
		return;
	}

	// config game session ...
	NewNationPara *nationPara = (NewNationPara *)mem_add(sizeof(NewNationPara)*MAX_NATION);
	int mpPlayerCount = 0;
	if( !mp_select_option(nationPara, &mpPlayerCount) )
	{
		mem_del(nationPara);
		if( remote.is_host )
			mp_obj.close_session();
		remote.deinit();
		mp_obj.deinit();
		return;
	}

	// assign nation is done is mp_select_option

	// ---------- initialize ec_remote --------- //
	// find itself
	for( p = 0; p < mpPlayerCount; ++p )
	{
		if( nationPara[p].dp_player_id == mp_obj.get_my_player_id() )
		{
			ec_remote.init( &mp_obj, char(p+1) );
			break;
		}
	}

	err_when( p >= mpPlayerCount );

	for( p = 0; p < mpPlayerCount; ++p )
	{
		if( nationPara[p].dp_player_id != mp_obj.get_my_player_id() )
		{
			ec_remote.set_dp_id(char(p+1), nationPara[p].dp_player_id );
		}
	}

	//---------- start game now ----------//

//	vga_front.bar(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,V_LIGHT_BLUE);
//	sys.blt_virtual_buf();

	remote.init_start_mp();

	// suppose inital sys.frame_count is 1
	// find nation_array.player_recno from nationPara
	for( p = 0; p < mpPlayerCount; ++p )
	{
		if( nationPara[p].dp_player_id == mp_obj.get_my_player_id() )
		{
			// remote.init_send_queue(1, nation_array.player_recno);	// but nation_array.player_recno is not set
			remote.init_send_queue(1, nationPara[p].nation_recno);   // initialize the send queue for later sending
			break;
		}
	}
	err_when( p >= mpPlayerCount );

	remote.init_receive_queue(1);

	init();
	remote.handle_vga_lock = 0;	// disable lock handling

	battle.run(nationPara, mpPlayerCount);

	mem_del(nationPara);
	remote.deinit();
	mp_obj.deinit();
	deinit();
}
// --------- End of static function multi_player_game ----------//


// --------- Begin of static function load_mp_game ----------//
// avoid creating local variable in this function
void Game::load_mp_game(char *fileName, char *cmdLine)
{
	sub_game_mode = 1;

	int nationRecno;
	int choice, p;
	ProtocolType selected_protocol = TCPIP;

	if (!cmdLine) {
		// not launched from lobby

		mp_obj.poll_supported_protocols();
		choice = mp_select_service();
		if( !choice )
		{
			mp_obj.deinit();
			return;
		}

		switch(choice)
		{
		case 1:	// IPX
			selected_protocol = IPX;
			break;
		case 2:	// TCP/IP
			selected_protocol = TCPIP;
			break;
		case 3:	// Modem
			selected_protocol = Modem;
			break;
		case 4:	// Serial
			selected_protocol = Serial;
			break;
		default:
			selected_protocol = None;
		}
	}

	if (mp_obj.is_protocol_supported(selected_protocol))
		mp_obj.init(selected_protocol);

	if (cmdLine)
		mp_obj.init_lobbied(MAX_NATION, cmdLine);

	// do not call remote.init here, or sys.yield will call remote.poll_msg
	if(!mp_obj.is_initialized())
	{
		// BUGHERE : display error message
		box.msg("Cannot initialize DirectPlay.");
		mp_obj.deinit();
		return;
	}

	// load game
	//if( !game_file.load_game(fileName) )
	//{
	//	mp_obj.deinit();
	//	return;
	//}

	// count required player
	int gamePlayerCount = 0;
	for(nationRecno = 1; nationRecno <= nation_array.size(); ++nationRecno)
		if( !nation_array.is_deleted(nationRecno) && 
			(nation_array[nationRecno]->is_own() || nation_array[nationRecno]->is_remote()) )
			++gamePlayerCount;

	// create game or join game
	switch( mp_select_mode(fileName) )
	{
	case 1:		// create game
		// BUGHERE : enter session name here and player name here
		if( mp_obj.create_session(config.player_name, gamePlayerCount)	// player name as session name
			&& mp_obj.create_player(config.player_name, config.player_name) )
		{
			remote.init(&mp_obj);
			remote.create_game();
		}
		else
		{
			mp_obj.deinit();
			return;
		}
		break;
	case 2:		// join game
		if( (choice = mp_select_session()) )
		{
			// BUGHERE : enter session name here and player name here
			if( mp_obj.join_session(choice) && mp_obj.create_player(config.player_name, config.player_name) )
			{
				// count required player
				gamePlayerCount = 0;
				for(nationRecno = 1; nationRecno <= nation_array.size(); ++nationRecno)
					if( !nation_array.is_deleted(nationRecno) && !nation_array[nationRecno]->is_ai() )
						++gamePlayerCount;

				remote.init(&mp_obj);
				remote.connect_game();
			}
			else
			{
				// can't join session
				// BUGHERE : display error message here
				mp_obj.deinit();
				return;
			}
		}
		else
		{
			mp_obj.deinit();
			return;
		}
		break;
	default:			// cancel
		mp_obj.deinit();
		return;
	}

	// config game session ...
	if( !mp_select_load_option(fileName) )
	{
		if( remote.is_host )
			mp_obj.close_session();
		remote.deinit();
		mp_obj.deinit();
		return;
	}

	// assign nation is done is mp_select_load_option

	// ---------- initialize ec_remote --------- //
	// find itself
	ec_remote.init( &mp_obj, (char) (~nation_array)->nation_recno );
	for( p = 1; p <= nation_array.size(); ++p )
	{
		if( !nation_array.is_deleted(p) )
		{
			Nation *nationPtr = nation_array[p];
			if( nationPtr->is_remote() )
			{
				ec_remote.set_dp_id((char) p, nationPtr->player_id );
			}
		}
	}

	//---------- start game now ----------//

//	vga_front.bar(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,V_LIGHT_BLUE);
//	sys.blt_virtual_buf();

	remote.init_start_mp();
	remote.init_send_queue(sys.frame_count, nation_array.player_recno);   // initialize the send queue for later sending
	remote.init_receive_queue(sys.frame_count);

//	init();
	remote.handle_vga_lock = 0;	// disable lock handling

	battle.run_loaded();		// 1-multiplayer game

	remote.deinit();
	mp_obj.deinit();
	deinit();
}
// --------- End of static function load_mp_game ----------//


//-------- Begin of function Game::mp_select_service --------//
//
// Select multiplayer mode. Create a new game or connect
// to an existing game ?
//
// return : <int> service selected, starting from 1
// 
int Game::mp_select_service()
{
	enum { BUTTON_NUM = 4 };
	static short buttonX[BUTTON_NUM] = { 206, 412, 206, 412 };
	static short buttonY[BUTTON_NUM] = { 94, 94, 254, 254 };
	#define SERVICE_BUTTON_WIDTH SERVICE_OPTION_X_SPACE
	#define SERVICE_BUTTON_HEIGHT SERVICE_OPTION_HEIGHT

#define SVOPTION_PAGE        0x00000001
#define SVOPTION_ALL         0x0fffffff

	int refreshFlag = SVOPTION_ALL;

	// -------- display button ---------//
	Button3D returnButton;
	returnButton.create(520, 538, "CANCEL-U", "CANCEL-D", 1, 0);

	ButtonCustom serviceButton[BUTTON_NUM];
	int b;
//	for( b = 0; b < BUTTON_NUM && mp_obj.get_service_provider(b+1); ++b )
	for( b = 0; b < BUTTON_NUM; ++b )
	{
		serviceButton[b].create(buttonX[b], buttonY[b], 
			buttonX[b]+SERVICE_BUTTON_WIDTH-1, buttonY[b]+SERVICE_BUTTON_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL,0) );
	}

	int buttonCount = b;
	int choice = 0;

	vga_front.unlock_buf();

	//-------- detect buttons ---------//

	while(1)
	{
		if( sys.need_redraw_flag )
		{
			refreshFlag = SVOPTION_ALL;
			sys.need_redraw_flag = 0;
		}

		vga_front.lock_buf();

		sys.yield();
		mouse.get_event();

		if( refreshFlag )
		{
			if( refreshFlag & SVOPTION_PAGE )
			{
				//--------- display interface screen -------//

				image_menu.put_to_buf( &vga_back, "MPG-PG1" );
				image_menu.put_back( 234, 15,
					sub_game_mode == 0 ? (char*)"TOP-NMPG" : (char*)"TOP-LMPG" );
				vga_util.blt_buf(0, 0, vga_back.buf_width()-1, vga_back.buf_height()-1, 0);

				returnButton.paint();
				for( b = 0; b < buttonCount; ++b )
				{
					serviceButton[b].paint();
				}
			}

			refreshFlag = 0;
		}

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();

		// ------- detect next button -------//
		if( returnButton.detect() )
		{
			choice = 0;
			break;
		}

		// ------- detect protocol button ---------//

		for( b = 0; b < buttonCount; ++b )
		{
			if( serviceButton[b].detect())
			{
				// let the button look pushed
				serviceButton[b].paint(1);
				choice = b+1;
				break;
			}
		}

		if( choice)
			break;

		vga_front.unlock_buf();
	}

	if( !vga_front.buf_locked )
		vga_front.lock_buf();

	return choice;
}
//--------- End of function Game::mp_select_service ---------//


//-------- Begin of function Game::mp_select_mode --------//
// return 0 = cancel, 1 = create, 2 = join
int Game::mp_select_mode(char *defSaveFileName)
{
	enum { BUTTON_NUM = 4 };
	static short buttonX[BUTTON_NUM] = { 206, 412, 206, 412 };
	static short buttonY[BUTTON_NUM] = { 94, 94, 254, 254 };
	#define SERVICE_BUTTON_WIDTH SERVICE_OPTION_X_SPACE
	#define SERVICE_BUTTON_HEIGHT SERVICE_OPTION_HEIGHT

#define SMOPTION_GETA(n)   (1 << n)
#define SMOPTION_GETA_ALL  0x0000000f
#define SMOPTION_PAGE      0x08000000
#define SMOPTION_ALL       0x0fffffff

	int refreshFlag = SMOPTION_ALL;

	Button3D createButton, joinButton, returnButton;

	// ####### begin Gilbert 13/2 ##########//
	if( mp_obj.get_lobbied_name() )
	{
		strncpy(config.player_name, mp_obj.get_lobbied_name(), Config::PLAYER_NAME_LEN );
		config.player_name[Config::PLAYER_NAME_LEN] = '\0';
	}

	createButton.create(120, 538, "CREATE-U", "CREATE-D", 1, 0);
	if( mp_obj.is_lobbied() == 2 )		// join only
		createButton.enable_flag = 0;		// avoid paint();
	joinButton.create(320, 538, "JOIN-U", "JOIN-D", 1, 0);
	if( mp_obj.is_lobbied() == 1 )		// join only
		joinButton.enable_flag = 0;		// avoid paint();
	// ####### end Gilbert 13/2 ##########//
	returnButton.create(520, 538, "CANCEL-U", "CANCEL-D", 1, 0);

	//Get get_name;
	//get_name.field( 374, 470, config.player_name, Config::PLAYER_NAME_LEN, 574 );
	char saveFileName[8+1];		// save game name without path or extension
	if( defSaveFileName )
	{
		int newLen = m.str_str(defSaveFileName, "." );
		if( newLen > 1)
			m.str_cut(saveFileName, defSaveFileName, 1, newLen-1);
		else
			err_here();
		if( m.str_icmpx(saveFileName, "AUTO") || m.str_icmpx(saveFileName, "AUTO2") )
		{
			strcpy(saveFileName, "MULTI");
		}
	}
	else
	{
		strcpy(saveFileName, "MULTI");
	}


	//GetA getName, getSaveFile;
	//getName.init( 319, 423, 582, config.player_name, Config::PLAYER_NAME_LEN, &font_san, 0);
	//getName.enable_flag = !sub_game_mode;		// disable the first input, all input
	//getName.paint();
	//getSaveFile.init( 394, 447, 582, saveFileName, 8, &font_san, 0);
	//getSaveFile.enable_flag = 0;
	//getSaveFile.paint();
	GetAGroup keyInField(2);
	GetA &getName = keyInField[0];
	GetA &getSaveFile = keyInField[1];
	getName.init( 319, 423, 582, config.player_name, Config::PLAYER_NAME_LEN, &font_san, 0 ,1);
	getSaveFile.init( 394, 447, 582, saveFileName, 8, &font_san, 0, 1);
	keyInField.set_focus(0, 0);		// 0 in 2nd parameter - don't display

	int rc = 0;

	vga_front.unlock_buf();

	while(1)
	{
		if( sys.need_redraw_flag )
		{
			refreshFlag = SMOPTION_ALL;
			sys.need_redraw_flag = 0;
		}

		vga_front.lock_buf();

		sys.yield();
		mouse.get_event();

		if( refreshFlag )
		{
			if( refreshFlag & SMOPTION_PAGE )
			{
				//--------- display interface screen -------//

				image_menu.put_to_buf( &vga_back, "MPG-PG1" );
				image_menu.put_back( 234, 15,
					sub_game_mode == 0 ? (char*)"TOP-NMPG" : (char*)"TOP-LMPG" );

				vga_util.blt_buf(0, 0, vga_back.buf_width()-1, vga_back.buf_height()-1, 0);

				if( createButton.enable_flag )
					createButton.paint();
				if( joinButton.enable_flag )
					joinButton.paint();
				returnButton.paint();
			}

			if( refreshFlag & SMOPTION_GETA_ALL )
			{
				keyInField.paint();
			}

			refreshFlag = 0;
		}

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();

		if( keyInField.detect() )
		{
			// refreshFlag |= SMOPTION_GETA_ALL;
		}
		else if( returnButton.detect() )
		{
			rc = 0;
			break;
		}
		else if( createButton.detect() )
		{
			rc = 1;
		}
		else if( joinButton.detect() )
		{
			rc = 2;
		}

		if( rc )
		{
			// check saveFileName is AUTO*
			if( m.str_icmpx(getSaveFile.input_field, "AUTO") ||
				m.str_icmpx(getSaveFile.input_field, "AUTO2") )
			{
				if( !box.ask("It is not recommended to use this save game file name, do you wish to continue?") )
				{
					rc = 0;
				}
			}
		}

		if( rc )
		{
			// correct saveFileName
			int newLen;
			if( (newLen = m.str_str(getSaveFile.input_field, ".")) > 0 )
			{
				m.str_cut(getSaveFile.input_field, getSaveFile.input_field, 1, newLen-1);
			}
			if( strlen(getSaveFile.input_field) == 0 )
			{
				// empty, set back to default save game name
				strcpy( getSaveFile.input_field, "MULTI" );
			}

			strcpy(remote.save_file_name, saveFileName);
			strcat(remote.save_file_name, ".SVM");
			break;
		}

		vga_front.unlock_buf();
	}
	if( !vga_front.buf_locked )
		vga_front.lock_buf();

	return rc;
}
//-------- End of function Game::mp_select_mode --------//


//-------- Begin of function Game::mp_select_session --------//
int Game::mp_select_session()
{

#define SSOPTION_PAGE           0x00000010
#define SSOPTION_POLL_SESSION   0x00000001
#define SSOPTION_DISP_SESSION   0x00000002
#ifdef AMPLUS
#define SSOPTION_SCROLL_BAR     0x00000004
#endif
#define SSOPTION_ALL            0x7fffffff

	enum { JOIN_BUTTON_X = 320, JOIN_BUTTON_Y = 538 };
	enum { CANCEL_BUTTON_X = 520, CANCEL_BUTTON_Y = 538 };

#ifdef AMPLUS
	enum { SESSION_BUTTON_X1 = 30, SESSION_BUTTON_Y1 = 160 };
	enum { SESSION_BUTTON_Y_SPACING = 44, MAX_BUTTON = 8 };
	enum { SESSION_BUTTON_X2 = 754,
		SESSION_DESC_Y1 = SESSION_BUTTON_Y1+14,
		SESSION_DESC_X1 = SESSION_BUTTON_X1 + 14, SESSION_DESC_X2 = SESSION_BUTTON_X2,
		SESSION_BUTTON_Y2 = SESSION_BUTTON_Y1 + MAX_BUTTON * SESSION_BUTTON_Y_SPACING-1};

	enum { SCROLL_X1=757, SCROLL_Y1 = 176, SCROLL_X2 = 770, SCROLL_Y2 = 493 };
#else
	enum { SESSION_BUTTON_X1 = 108, SESSION_BUTTON_Y1 = 125 };
	enum { SESSION_BUTTON_Y_SPACING = BASIC_OPTION_HEIGHT, MAX_BUTTON = 6 };
	enum { SESSION_BUTTON_X2 = SESSION_BUTTON_X1 + BASIC_OPTION_X_SPACE-1,
		SESSION_DESC_Y1 = SESSION_BUTTON_Y1+10,
		SESSION_DESC_X1 = SESSION_BUTTON_X2 + 8, SESSION_DESC_X2 = 600,
		SESSION_BUTTON_Y2 = SESSION_BUTTON_Y1 + MAX_BUTTON * SESSION_BUTTON_Y_SPACING-1};
#endif

	// ####### begin Gilbert 13/2 ########//
	if( mp_obj.is_lobbied() == 2 )		// join session, but selected in lobby
	{
		return 1;
	}
	// ####### end Gilbert 13/2 ########//

	unsigned long refreshTime;
	int refreshFlag = SSOPTION_ALL;
	int choice = 0;
	SessionIdType sessionGuid;
	memset(&sessionGuid, 0, sizeof(sessionGuid));

	// ------- initialized button -----------//

	Button3D joinButton;
	joinButton.create(JOIN_BUTTON_X, JOIN_BUTTON_Y, "JOIN-U", "JOIN-D", 1, 0);
	joinButton.disable();

	Button3D cancelButton;
	cancelButton.create(CANCEL_BUTTON_X, CANCEL_BUTTON_Y, "CANCEL-U", "CANCEL-D", 1, 0);

#ifdef AMPLUS
	SlideVBar scrollBar;

	scrollBar.init_scroll(SCROLL_X1, SCROLL_Y1, SCROLL_X2, SCROLL_Y2,
		MAX_BUTTON, disp_scroll_bar_func);
	scrollBar.set(1, 1, 1);		// assume it has one record

	Button3D scrollUp;
	scrollUp.create(SCROLL_X1,SCROLL_Y1-17, "SV-UP-U", "SV-UP-D", 1, 0);

	Button3D scrollDown;
	scrollDown.create(SCROLL_X1,SCROLL_Y2+1, "SV-DW-U", "SV-DW-D", 1, 0);

	Blob browseArea[MAX_BUTTON];

	#define BASE_SESSION scrollBar.view_recno
#else
	#define BASE_SESSION 1
#endif

	unsigned long pollTime = 1000;
	vga_front.unlock_buf();

	while(1)
	{
		int s;
		int b;

		if( sys.need_redraw_flag )
		{
			refreshFlag = SSOPTION_ALL;
			sys.need_redraw_flag = 0;
		}

		vga_front.lock_buf();

		sys.yield();
		mouse.get_event();

		if( refreshFlag )
		{
			if( refreshFlag & SSOPTION_PAGE )
			{
				// --------- display interface screen -------//
				image_menu.put_to_buf( &vga_back, "MPG-PG2" );
#ifdef AMPLUS
				image_menu2.put_to_buf( &vga_back, "MPG-PG2" );
#endif
				image_menu.put_back( 234, 15,
					sub_game_mode == 0 ? (char*)"TOP-NMPG" : (char*)"TOP-LMPG" );
				vga_util.blt_buf(0, 0, vga_back.buf_width()-1, vga_back.buf_height()-1, 0);

#ifdef AMPLUS
				scrollBar.paint();
				scrollUp.paint();
				scrollDown.paint();

				// capture into browseArea, scrollArea, textArea
				for( b = 0; b < MAX_BUTTON; ++b)
				{
					browseArea[b].resize(2*sizeof(short) + (SESSION_BUTTON_X2-SESSION_BUTTON_X1+1)*SESSION_BUTTON_Y_SPACING);
					vga_front.read_bitmap(
						SESSION_BUTTON_X1, b*SESSION_BUTTON_Y_SPACING+SESSION_BUTTON_Y1,
						SESSION_BUTTON_X2, (b+1)*SESSION_BUTTON_Y_SPACING+SESSION_BUTTON_Y1-1,
						browseArea[b].ptr);
				}
#endif
				joinButton.paint();
				cancelButton.paint();
			}

			if( refreshFlag & SSOPTION_POLL_SESSION )
			{
				pollTime = m.get_time();
				if( !mp_obj.poll_sessions() )
				{
					// return fail if poll_sessions fails or cancel the dialogue box
					choice = 0;
					break;
				}

				// limit the pollTime between 1 sec to 10 sec
				pollTime = m.get_time() - pollTime + 1000;
				if( pollTime > 10000 )
					pollTime = 10000;

				refreshTime = m.get_time();

#ifdef AMPLUS
				// ------- sort by name ---------//
				mp_obj.sort_sessions(2);		// sort by session name
#endif

				// ------- update choice ---------- //
				choice = 0;
				for( s = 1; mp_obj.get_session(s); ++s )
				{
					if( sessionGuid == mp_obj.get_session(s)->session_id() )
						choice = s;
				}
				if( choice > 0)
					joinButton.enable();
				else
					joinButton.disable();

				//------- update scroll bar --------//
				// BUGHERE : error if empty
#ifdef AMPLUS
				scrollBar.set(1, s-1, scrollBar.view_recno);
				refreshFlag |= SSOPTION_SCROLL_BAR;
#endif
				refreshFlag |= SSOPTION_DISP_SESSION;
			}

			if( refreshFlag & SSOPTION_DISP_SESSION )
			{
#ifndef AMPLUS
				vga_util.blt_buf(SESSION_BUTTON_X1, SESSION_BUTTON_Y1, SESSION_DESC_X2, SESSION_BUTTON_Y2, 0);
#endif
				for( b = 0, s = BASE_SESSION; b < MAX_BUTTON; ++b, ++s )
				{

#ifdef AMPLUS
					vga_back.put_bitmap(
						SESSION_BUTTON_X1, b*SESSION_BUTTON_Y_SPACING+SESSION_BUTTON_Y1,
						browseArea[(s-1)%MAX_BUTTON].ptr);
					vga_util.blt_buf(
						SESSION_BUTTON_X1, b*SESSION_BUTTON_Y_SPACING+SESSION_BUTTON_Y1,
						SESSION_BUTTON_X2, (b+1)*SESSION_BUTTON_Y_SPACING+SESSION_BUTTON_Y1-1, 0);
#endif

					if( mp_obj.get_session(s) )
					{
						// display session description
						font_san.put( SESSION_DESC_X1, SESSION_DESC_Y1 + b*SESSION_BUTTON_Y_SPACING,
							mp_obj.get_session(s)->name_str(), 0, SESSION_DESC_X2 );

						// display cursor 
						if( s == choice )
						{
							vga_front.adjust_brightness( SESSION_BUTTON_X1, SESSION_BUTTON_Y1 + b*SESSION_BUTTON_Y_SPACING,
								SESSION_BUTTON_X2, SESSION_BUTTON_Y1 + (b+1)*SESSION_BUTTON_Y_SPACING-1, -2);
#ifndef AMPLUS
							image_interface.put_front(SESSION_BUTTON_X1, SESSION_BUTTON_Y1 + b*SESSION_BUTTON_Y_SPACING, "BAS_DOWN" );
#endif
						}
					}
				}
			}

#ifdef AMPLUS
			if( refreshFlag & SSOPTION_SCROLL_BAR )
			{
				scrollBar.paint();
			}
#endif

			refreshFlag = 0;
		}

		sys.blt_virtual_buf();		// blt the virtual front buffer to the screen

		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();

#ifdef AMPLUS
		int scrollRc = scrollBar.detect();
		if( scrollRc )
		{
			// refreshFlag |= SSOPTION_SCROLL_BAR;
			// dragging scroll bar, don't poll session
			refreshFlag &= ~SSOPTION_POLL_SESSION;

			// suspend the refreshTime, so session list won't update immediate after release dragging
			refreshTime = m.get_time();

			if( scrollRc == 1)
				refreshFlag |= SSOPTION_DISP_SESSION;
		}
		else
#endif
		{
			for( b = 0, s = BASE_SESSION; b < MAX_BUTTON && mp_obj.get_session(s) ; ++b, ++s )
			{
				if( mouse.single_click( SESSION_BUTTON_X1, SESSION_BUTTON_Y1 + (b)*SESSION_BUTTON_Y_SPACING,
					SESSION_DESC_X2, SESSION_BUTTON_Y1 + (b+1)*SESSION_BUTTON_Y_SPACING -1 ) )
				{
					choice = s;
					sessionGuid = mp_obj.get_session(s)->session_id();
					refreshFlag |= SSOPTION_DISP_SESSION;
					joinButton.enable();

					// suspend the refreshTime, so session list won't update immediate after release dragging
					refreshTime = m.get_time();
				}
			}

#ifdef AMPLUS
			if( scrollUp.detect() )
			{
				int oldValue = scrollBar.view_recno;
				if( oldValue != scrollBar.set_view_recno(oldValue-1) )
					refreshFlag |= SSOPTION_DISP_SESSION | SSOPTION_SCROLL_BAR;
			}
		
			if( scrollDown.detect() )
			{
				int oldValue = scrollBar.view_recno;
				if( oldValue != scrollBar.set_view_recno(oldValue+1) )
					refreshFlag |= SSOPTION_DISP_SESSION | SSOPTION_SCROLL_BAR;
			}
#endif

			if( choice > 0 && joinButton.detect() )
				break;

			if( cancelButton.detect() )
			{
				choice = 0;
				break;
			}

			if( !(mouse.skey_state & SHIFT_KEY_MASK) && m.get_time() - refreshTime > pollTime )
				refreshFlag |= SSOPTION_POLL_SESSION | SSOPTION_DISP_SESSION;
		}

		vga_front.unlock_buf();
	}

	if( !vga_front.buf_locked )
		vga_front.lock_buf();

	return choice;
}
//-------- End of function Game::mp_select_session --------//


// define bit flag for refreshFlag
#define SGOPTION_PAGE           0x40000000
#define SGOPTION_RACE           0x00000001
#define SGOPTION_COLOR          0x00000002
#define SGOPTION_AI_NATION      0x00000004
#define SGOPTION_DIFFICULTY     0x00000008
#define SGOPTION_TERRAIN        0x00000010
#define SGOPTION_LAND_MASS      0x00000020
// #### begin Gilbert 25/10 #######//
// #define SGOPTION_NAME_FIELD     0x00000040
#define SGOPTION_MAP_ID         0x00000080
// #### end Gilbert 25/10 #######//
#define SGOPTION_EXPLORED       0x00000100
#define SGOPTION_FOG            0x00000200
#define SGOPTION_TREASURE       0x00000400
#define SGOPTION_AI_TREASURE    0x00000800
#define SGOPTION_AI_AGGRESSIVE  0x00001000
#define SGOPTION_FRYHTANS       0x00002000
#define SGOPTION_RANDOM_STARTUP 0x00004000
#define SGOPTION_RAW            0x00010000
#define SGOPTION_NEAR_RAW       0x00020000
#define SGOPTION_START_TOWN     0x00040000
#define SGOPTION_TOWN_STRENGTH  0x00080000
#define SGOPTION_TOWN_EMERGE    0x00100000
#define SGOPTION_KINGDOM_EMERGE 0x00200000
#define SGOPTION_RANDOM_EVENT   0x00400000
#define SGOPTION_CLEAR_ENEMY    0x01000000
#define SGOPTION_CLEAR_MONSTER  0x02000000
#define SGOPTION_ENOUGH_PEOPLE  0x04000000
#define SGOPTION_ENOUGH_INCOME  0x08000000
// ##### begin Gilbert 25/10 #######//
#define SGOPTION_ENOUGH_SCORE   0x10000000
#define SGOPTION_TIME_LIMIT     0x20000000
// ##### end Gilbert 25/10 #######//
#define SGOPTION_ALL            0x7fffffff
#define SGOPTION_ALL_OPTIONS    (SGOPTION_ALL & ~SGOPTION_PAGE & ~SGOPTION_MAP_ID)

#define MGOPTION_PLAYERS        0x00000001
#define MGOPTION_IN_MESSAGE     0x00000002
#define MGOPTION_OUT_MESSAGE    0x00000004
#define MGOPTION_ALL            0x7fffffff


//-------- Begin of function Game::mp_select_option -----------//
// return 0 = cancel, 1 = ok
int Game::mp_select_option(NewNationPara *nationPara, int *mpPlayerCount)
{
	const int offsetY = 212;
	char optionMode = OPTION_BASIC;
	char menuTitleBitmap[] = "TOP-NMPG";
	
	Config tempConfig = config;
	tempConfig.reset_cheat_setting();

	// some setting may be modified in the last game
	if( tempConfig.difficulty_level != OPTION_CUSTOM )
		tempConfig.change_difficulty(tempConfig.difficulty_level);

	PID_TYPE from, to;
	uint32_t recvLen;
	int sysMsgCount;
	char *recvPtr;

	char raceAssigned[MAX_RACE];		// master copy belongs to host's
	char colorAssigned[MAX_COLOR_SCHEME];		// master copy belongs to host's
	//static short raceX[MAX_RACE] = {390, 313, 236, 467, 390, 236, 313};
	//static short raceY[MAX_RACE] = {276, 276, 276, 276, 243, 243, 243};
	//const raceWidth = 77;
	//const raceHeight = 33;
	//static short colorX[MAX_COLOR_SCHEME] = {545, 581, 581, 617, 617, 545, 653};
	//static short colorY[MAX_COLOR_SCHEME] = {243, 276, 243, 276, 243, 276, 276};
	//const colorWidth = 36;
	//const colorHeight = 33;
	static short tickX[MAX_NATION] = { 103, 254, 405, 556, 103, 254, 405 };
	static short tickY[MAX_NATION] = {  53,  53,  53,  53,  73,  73,  73 };
	const int nameOffsetX = 23;
	const int nameOffsetY = 3;

	DynArray messageList(sizeof(MpStructChatMsg));
	MpStructChatMsg typingMsg(tempConfig.player_name, NULL);
	
	memset( raceAssigned, 0, sizeof(raceAssigned) );
	memset( colorAssigned, 0, sizeof(colorAssigned) );

	PID_TYPE hostPlayerId = 0;
	PID_TYPE regPlayerId[MAX_NATION];
	memset( regPlayerId, 0, sizeof(regPlayerId) );
	char playerReadyFlag[MAX_NATION];
	memset( playerReadyFlag, 0, sizeof(playerReadyFlag) );
	short playerRace[MAX_NATION];	// host only
	memset( playerRace, 0, sizeof(playerRace) );
	short playerColor[MAX_NATION];	// host only
	memset( playerColor, 0, sizeof(playerColor) );
	short playerBalance[MAX_NATION];
	memset( playerBalance, 0, sizeof(playerBalance) );

	int regPlayerCount = 0;
	int selfReadyFlag = 0;
	int shareRace = 1;		// host only, 0= exclusive race of each player

	mp_obj.poll_players();
	int p;
	for( p = 1; p <= MAX_NATION && mp_obj.get_player(p); ++p )
	{
		// host only identify himself
		if( !remote.is_host || mp_obj.get_player(p)->pid() == mp_obj.get_my_player_id() )
		{
			regPlayerId[regPlayerCount] = mp_obj.get_player(p)->pid();
			playerReadyFlag[regPlayerCount] = 0;
			playerColor[regPlayerCount] = 0;
			playerRace[regPlayerCount] = 0;
			playerBalance[regPlayerCount] = 0;
			++regPlayerCount;
		}
	}

	int i;
	long refreshFlag = SGOPTION_ALL;
	// ####### begin Gilbert 25/10 #####//
	long mRefreshFlag = MGOPTION_ALL;
	// ####### end Gilbert 25/10 #####//
	int retFlag = 0;

	// randomly select a race
	if( remote.is_host )
	{
		tempConfig.race_id = char(mp_obj.get_my_player_id() % MAX_RACE + 1);
		raceAssigned[tempConfig.race_id-1] = 1;

		tempConfig.player_nation_color = 1;
		colorAssigned[tempConfig.player_nation_color-1] = 1;
		for( p = 0; p < regPlayerCount && regPlayerId[p] != mp_obj.get_my_player_id(); ++p );
		if( p < regPlayerCount )
		{
			playerRace[p] = tempConfig.race_id;
			playerColor[p] = tempConfig.player_nation_color;
			playerBalance[p] = sys.cdrom_drive ? PLAYER_RATIO_CDROM : PLAYER_RATIO_NOCD;
		}
	}
	else
	{
		tempConfig.race_id = 0;
		tempConfig.player_nation_color = 0;
		// ask host for a race and color code
		MpStructNewPlayer msgNewPlayer( mp_obj.get_my_player_id(), 
			sys.cdrom_drive ? PLAYER_RATIO_CDROM : PLAYER_RATIO_NOCD );
		mp_obj.send_stream(BROADCAST_PID, &msgNewPlayer, sizeof(msgNewPlayer) );
	}

	// --------- initialize race button group ---------- //

	ButtonCustomGroup raceGroup(MAX_RACE);
	for( i = 0; i < MAX_RACE; ++i )
	{
#if(MAX_RACE == 10)
		raceGroup[i].create(220+(i%5)*BASIC_OPTION_X_SPACE, (i/5)*BASIC_OPTION_HEIGHT+offsetY+70,
			220+(i%5+1)*BASIC_OPTION_X_SPACE-1, (i/5+1)*BASIC_OPTION_HEIGHT+offsetY+70-1,
			disp_virtual_button, ButtonCustomPara(&raceGroup, race_table[i]), 0, 0);
		#define Y_SHIFT_FLAG 1
#else
		raceGroup[i].create(118+i*BASIC_OPTION_X_SPACE, offsetY+93,
			118+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+93+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&raceGroup, race_table[i]), 0, 0);
		#define Y_SHIFT_FLAG 0
#endif
	}

	// --------- initialize color button group ---------- //

	ButtonCustomGroup colorGroup(MAX_COLOR_SCHEME);
	for( i = 0; i < MAX_COLOR_SCHEME; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 13
		#else
			#define Y_SHIFT 0
		#endif
		colorGroup[i].create(195+i*COLOR_OPTION_X_SPACE, offsetY+139+Y_SHIFT,
			195+(i+1)*COLOR_OPTION_X_SPACE-1, offsetY+139+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&colorGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	// ---------- initialize ai_nation_count buttons --------//

	ButtonCustom aiNationInc, aiNationDec;
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 13
		#else
			#define Y_SHIFT 0
		#endif
		aiNationInc.create(595, offsetY+139+Y_SHIFT, 
			595+COLOR_OPTION_X_SPACE-1, offsetY+139+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, +1) );
		aiNationDec.create(630, offsetY+139+Y_SHIFT,
			630+COLOR_OPTION_X_SPACE-1, offsetY+139+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, -1) );
		#undef Y_SHIFT
	}

	// ---------- initialize difficulty_level button group -------//

	ButtonCustomGroup diffGroup(6);
	for( i = 0; i < 6; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 16
		#else
			#define Y_SHIFT 0
		#endif
		diffGroup[i].create( 205+i*BASIC_OPTION_X_SPACE, offsetY+184+Y_SHIFT,
			205+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+184+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&diffGroup, i), 0, 0);
		#undef Y_SHIFT
	}

	// --------- initialize terrain_set button group -------//

	// ##### begin Gilbert 25/10 #######//
	ButtonCustomGroup terrainGroup(3);
	for( i = 0; i < 3; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 16
		#else
			#define Y_SHIFT 0
		#endif
#ifdef AMPLUS
		terrainGroup[i].create(166+i*BASIC_OPTION_X_SPACE, offsetY+248+Y_SHIFT, 
			166+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+248+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&terrainGroup, i+1), 0, 0);
#else
		int k = i;
		switch(i)
		{
		case 1: k = 2; break;
		case 2: k = 1; break;
		}
		terrainGroup[i].create(205+k*BASIC_OPTION_X_SPACE, offsetY+248+Y_SHIFT, 
			205+(k+1)*BASIC_OPTION_X_SPACE-1, offsetY+248+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&terrainGroup, i+1), 0, 0);
		if( i == 1 )
			terrainGroup[i].enable_flag = 0;
#endif
		#undef Y_SHIFT
	}
	// ##### end Gilbert 25/10 #######//

	// --------- initialize land_mass button group -------//

	ButtonCustomGroup landGroup(3);
	for( i = 0; i < 3; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 16
		#else
			#define Y_SHIFT 0
		#endif
		landGroup[i].create(439+i*BASIC_OPTION_X_SPACE, offsetY+248+Y_SHIFT,
			439+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+248+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&landGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	GetA messageField;
	messageField.init( 190, 116, 700, typingMsg.content,
		MpStructChatMsg::MSG_LENGTH, &font_san, 0, 1);

	// ###### begin Gilbert 25/10 #######//
	// --------- initialize info.random_seed field ----------//

	MpStructSeedStr msgSeedStr(info.random_seed);
	GetA mapIdField;
#if(defined(SPANISH))
	#define MAPID_X1 588
#elif(defined(FRENCH))
	#define MAPID_X1 578
#else
	#define MAPID_X1 564
#endif

	mapIdField.init( MAPID_X1, offsetY+83, 700, msgSeedStr.seed_str, msgSeedStr.RANDOM_SEED_MAX_LEN, &font_san, 0, 1);
#undef MAPID_X1
	// ###### end Gilbert 25/10 #######//

	// --------- initialize explore_whole_map button group -------//

	ButtonCustomGroup exploreGroup(2);
	for( i = 0; i < 2; ++i )
	{
		exploreGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+74, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+74+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&exploreGroup, 1-i), 0, 0);
	}

	// --------- initialize fog_of_war button group -------//

	ButtonCustomGroup fogGroup(2);
	for( i = 0; i < 2; ++i )
	{
		fogGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+106, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+106+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&fogGroup, 1-i), 0, 0);
	}

	// --------- initialize start_up_cash/start_up_food button group -------//

	ButtonCustomGroup treasureGroup(4);
	for( i = 0; i < 4; ++i )
	{
		treasureGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+138,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+138+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&treasureGroup, i+1), 0, 0);
	}

	// --------- initialize ai_start_up_cash/food button group -------//

	ButtonCustomGroup aiTreasureGroup(4);
	for( i = 0; i < 4; ++i )
	{
		aiTreasureGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+170,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+170+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&aiTreasureGroup, i+1), 0, 0);
	}

	// --------- initialize ai_aggressiveness -------//

	ButtonCustomGroup aiAggressiveGroup(4);
	for( i = 0; i < 4; ++i )
	{
		aiAggressiveGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+202,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+202+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&aiAggressiveGroup, i+1), 0, 0);
	}

	// --------- initialize monster_type -------//

	ButtonCustomGroup monsterGroup(3);
	for( i = 0; i < 3; ++i )
	{
		monsterGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+234, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+234+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&monsterGroup, i), 0, 0);
	}

	// --------- initialize random startup button group -------//

	ButtonCustomGroup randomStartUpGroup(2);
	for( i = 0; i < 2; ++i )
	{
		randomStartUpGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+266, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+266+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&randomStartUpGroup, 1-i), 0, 0);
	}

	//  -------- initialize start_up_raw_site buttons --------- //

	ButtonCustom rawSiteInc, rawSiteDec;
	rawSiteInc.create( 358, offsetY+72, 
		358+COLOR_OPTION_X_SPACE-1, offsetY+72+COLOR_OPTION_HEIGHT-1,
		disp_virtual_button, ButtonCustomPara(NULL,0));
	rawSiteDec.create( 393, offsetY+72, 
		393+COLOR_OPTION_X_SPACE-1, offsetY+72+COLOR_OPTION_HEIGHT-1,
		disp_virtual_button, ButtonCustomPara(NULL,0));

	// --------- initialize start_up_has_mine_nearby button group --------//

	ButtonCustomGroup nearRawGroup(2);
	for( i = 0; i < 2; ++i )
	{
		nearRawGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+104,
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+104+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&nearRawGroup, 1-i), 0, 0);
	}

	// --------- initialize start_up_independent_town button group --------//

	static short startTownArray[3] = { 7, 15, 30 };

	ButtonCustomGroup townStartGroup(3);
	for( i = 0; i < 3; ++i )
	{
		townStartGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+136, 
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+136+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townStartGroup, startTownArray[i]), 0, 0);
	}

	// --------- initialize independent_town_resistance button group --------//

	ButtonCustomGroup townResistGroup(3);
	for( i = 0; i < 3; ++i )
	{
		townResistGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+168, 
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+168+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townResistGroup, i+1), 0, 0);
	}

	// --------- initialize new_independent_town_emerge button group --------//

	ButtonCustomGroup townEmergeGroup(2);
	for( i = 0; i < 2; ++i )
	{
		townEmergeGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+200,
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+200+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townEmergeGroup, 1-i), 0, 0);
	}

	// --------- initialize new_nation_emerge button group --------//

	ButtonCustomGroup nationEmergeGroup(2);
	for( i = 0; i < 2; ++i )
	{
		nationEmergeGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+232,
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+232+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&nationEmergeGroup, 1-i), 0, 0);
	}

	// --------- initialize random_event_frequency button group --------//

	ButtonCustomGroup randomEventGroup(4);
	for( i = 0; i < 4; ++i )
	{
		randomEventGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+264, 
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+264+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&randomEventGroup, i), 0, 0);
	}

	// ---------- initialize goal buttons ----------//

	// ##### begin Gilbert 25/10 #######//
	ButtonCustom clearEnemyButton, clearMonsterButton, enoughPeopleButton, enoughIncomeButton, enoughScoreButton, timeLimitButton;
	ButtonCustom peopleInc, peopleDec, incomeInc, incomeDec, scoreInc, scoreDec, yearInc, yearDec;
	
	clearEnemyButton.create( 209, offsetY+109, 209+19, offsetY+109+19,	// -3
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 1);
	clearEnemyButton.enable_flag = 0;;
	clearMonsterButton.create( 209, offsetY+142, 209+19, offsetY+142+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 
		tempConfig.goal_destroy_monster);
	enoughPeopleButton.create( 209, offsetY+175, 209+19, offsetY+175+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 
		tempConfig.goal_population_flag);
	enoughIncomeButton.create( 209, offsetY+208, 209+19, offsetY+208+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_economic_score_flag);
	enoughScoreButton.create( 209, offsetY+241, 209+19, offsetY+241+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_total_score_flag);
	timeLimitButton.create( 209, offsetY+273, 209+19, offsetY+273+19,	// +29
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_year_limit_flag);

	peopleInc.create( 524, offsetY+170,
		524+COLOR_OPTION_X_SPACE-1, offsetY+170+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	peopleDec.create( 559, offsetY+170, 
		559+COLOR_OPTION_X_SPACE-1, offsetY+170+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	incomeInc.create( 524, offsetY+202,
		524+COLOR_OPTION_X_SPACE-1, offsetY+202+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	incomeDec.create( 559, offsetY+202,
		559+COLOR_OPTION_X_SPACE-1, offsetY+202+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	scoreInc.create( 524, offsetY+234,
		524+COLOR_OPTION_X_SPACE-1, offsetY+234+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	scoreDec.create( 559, offsetY+234,
		559+COLOR_OPTION_X_SPACE-1, offsetY+234+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	yearInc.create( 524, offsetY+266,
		524+COLOR_OPTION_X_SPACE-1, offsetY+266+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	yearDec.create( 559, offsetY+266,
		559+COLOR_OPTION_X_SPACE-1, offsetY+266+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	// ##### end Gilbert 25/10 #######//

	Button3D startButton, readyButton, returnButton;
	readyButton.create(120, 538, "READY-U", "READY-D", 1, 0);
	startButton.create(320, 538, "START-U", "START-D", 1, 0);
	returnButton.create(520, 538, "CANCEL-U", "CANCEL-D", 1, 0);

	// ###### begin Gilbert 24/10 #######//
	vga_front.unlock_buf();
	// ###### end Gilbert 24/10 #######//

	while(1)
	{
		// ####### begin Gilbert 23/10 #######//
		if( sys.need_redraw_flag )
		{
			refreshFlag = SGOPTION_ALL;
			mRefreshFlag = MGOPTION_ALL;
			sys.need_redraw_flag = 0;
		}

		vga_front.lock_buf();
		// ####### end Gilbert 23/10 #######//

		sys.yield();
		mouse.get_event();

		// -------- display ----------//
		// ##### begin Gilbert 25/10 #####//
		if( refreshFlag || mRefreshFlag )
		// ##### end Gilbert 25/10 #####//
		{
			// ------- display basic option ---------//
			if( optionMode == OPTION_BASIC )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-BSC");
#if(MAX_RACE == 10)
					// protection : image_menu.put_to_buf( &vga_back, "MPG-BSC");
					// ensure the user has the release version (I_MENU.RES)
					// image_menu2.put_to_buf( &vga_back, "MPG-BSC") get the real one
					image_menu2.put_to_buf( &vga_back, "MPG-BSC");
#endif
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_RACE )
					raceGroup.paint( reverse_race_table[tempConfig.race_id-1] );
				if( refreshFlag & SGOPTION_COLOR )
					colorGroup.paint( tempConfig.player_nation_color-1 );
				if( refreshFlag & SGOPTION_AI_NATION )
				{
					#if(Y_SHIFT_FLAG)
						#define Y_SHIFT 13
					#else
						#define Y_SHIFT 0
					#endif
					font_san.center_put(564, offsetY+144+Y_SHIFT, 564+25, offsetY+144+Y_SHIFT+21,
						m.format(tempConfig.ai_nation_count), 1);
					aiNationInc.paint();
					aiNationDec.paint();
					#undef Y_SHIFT
				}
				if( refreshFlag & SGOPTION_DIFFICULTY )
					diffGroup.paint(tempConfig.difficulty_level);
				if( refreshFlag & SGOPTION_TERRAIN )
					terrainGroup.paint(tempConfig.terrain_set-1);
				if( refreshFlag & SGOPTION_LAND_MASS )
					landGroup.paint(tempConfig.land_mass-1);
			}

			// ------- display advanced option ---------//
			if( optionMode == OPTION_ADVANCED )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-O1");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				// ###### begin Gilbert 25/10 #######//
				if( refreshFlag & SGOPTION_MAP_ID )
					mapIdField.paint();
				// ###### end Gilbert 25/10 #######//
				if( refreshFlag & SGOPTION_EXPLORED )
					exploreGroup.paint(1-tempConfig.explore_whole_map);
				if( refreshFlag & SGOPTION_FOG )
					fogGroup.paint(1-tempConfig.fog_of_war);
				if( refreshFlag & SGOPTION_TREASURE )
					treasureGroup.paint( tempConfig.start_up_cash-1 );
				if( refreshFlag & SGOPTION_AI_TREASURE )
					aiTreasureGroup.paint( tempConfig.ai_start_up_cash-1 );
				if( refreshFlag & SGOPTION_AI_AGGRESSIVE )
					aiAggressiveGroup.paint(tempConfig.ai_aggressiveness-1);
				if( refreshFlag & SGOPTION_FRYHTANS )
					monsterGroup.paint(tempConfig.monster_type);
				if( refreshFlag & SGOPTION_RANDOM_STARTUP )
					randomStartUpGroup.paint(1-tempConfig.random_start_up);
			}

			// ------- display advanced option ---------//
			if( optionMode == OPTION_ADVANCE2 )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-O2");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_RAW )
				{
					font_san.center_put(327, offsetY+77, 327+25, offsetY+77+21,
						m.format(tempConfig.start_up_raw_site), 1);
					rawSiteInc.paint();
					rawSiteDec.paint();
				}
				if( refreshFlag & SGOPTION_NEAR_RAW )
					nearRawGroup.paint(1-tempConfig.start_up_has_mine_nearby);
				if( refreshFlag & SGOPTION_START_TOWN )
					townStartGroup.paint(
					tempConfig.start_up_independent_town >= 30 ? 2 :
					tempConfig.start_up_independent_town <= 7 ? 0 :
					1
					);
				if( refreshFlag & SGOPTION_TOWN_STRENGTH )
					townResistGroup.paint(tempConfig.independent_town_resistance-1);
				if( refreshFlag & SGOPTION_TOWN_EMERGE )
					townEmergeGroup.paint(1-tempConfig.new_independent_town_emerge);
				if( refreshFlag & SGOPTION_KINGDOM_EMERGE )
					nationEmergeGroup.paint(1-tempConfig.new_nation_emerge);
				if( refreshFlag & SGOPTION_RANDOM_EVENT )
					randomEventGroup.paint(tempConfig.random_event_frequency);
			}

			// ------- display goal option ---------//
			if( optionMode == OPTION_GOAL )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-GOAL");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_CLEAR_ENEMY )
					clearEnemyButton.paint();
				if( refreshFlag & SGOPTION_CLEAR_MONSTER )
					clearMonsterButton.paint(tempConfig.goal_destroy_monster);
				// ####### begin Gilbert 25/10 ########//
				if( refreshFlag & SGOPTION_ENOUGH_PEOPLE )
				{
					enoughPeopleButton.paint(tempConfig.goal_population_flag);
					font_san.center_put( 446, offsetY+176, 446+67, offsetY+176+21,
						m.format(tempConfig.goal_population), 1);
					peopleInc.paint();
					peopleDec.paint();
				}
				if( refreshFlag & SGOPTION_ENOUGH_INCOME )
				{
					enoughIncomeButton.paint(tempConfig.goal_economic_score_flag);
					font_san.center_put( 446, offsetY+207, 446+67, offsetY+207+21,
						m.format(tempConfig.goal_economic_score), 1);
					incomeInc.paint();
					incomeDec.paint();
				}
				if( refreshFlag & SGOPTION_ENOUGH_SCORE )
				{
					enoughScoreButton.paint(tempConfig.goal_total_score_flag);
					font_san.center_put( 446, offsetY+239, 446+67, offsetY+239+21,
						m.format(tempConfig.goal_total_score), 1);
					scoreInc.paint();
					scoreDec.paint();
				}
				if( refreshFlag & SGOPTION_TIME_LIMIT )
				{
					timeLimitButton.paint(tempConfig.goal_year_limit_flag);
					font_san.center_put( 446, offsetY+271, 446+33, offsetY+271+21,
						m.format(tempConfig.goal_year_limit), 1);
					yearInc.paint();
					yearDec.paint();
				}
				// ####### end Gilbert 25/10 ########//
			}

			// -------- refresh players in the session --------//
			if( mRefreshFlag & MGOPTION_PLAYERS )
			{
				vga_util.blt_buf( 96, 46, 702, 100, 0 );
				for( p = 0; p < regPlayerCount; ++p)
				{
					if( playerReadyFlag[p] )
					{
						image_menu.put_front( tickX[p]+3, tickY[p]+3, "NMPG-RCH" );
					}
					PlayerDesc *dispPlayer = mp_obj.search_player(regPlayerId[p]);
					font_san.put( tickX[p]+nameOffsetX, tickY[p]+nameOffsetY, dispPlayer?dispPlayer->friendly_name_str():(char*)"?anonymous?" );
				}
			}

			// ------------ display chat message --------//
			if( mRefreshFlag & MGOPTION_OUT_MESSAGE )
			{
				messageField.paint();
			}

			// ------------- display incoming chat message --------//
			if( mRefreshFlag & MGOPTION_IN_MESSAGE )
			{
				vga_util.blt_buf( 101, 135, 700, 202, 0 );
				for( p = 1; p <= 4 && p <= messageList.size() ; ++p)
				{
					int ny = 136+(p-1)*16;
					int nx = font_san.put( 102, ny, ((MpStructChatMsg *)messageList.get(p))->sender );
					nx = font_san.put( nx, ny, " : ");
					nx = font_san.put( nx, ny, ((MpStructChatMsg *)messageList.get(p))->content, 0, 700);
				}
			}

			// ------- display difficulty -------//
			if( (refreshFlag & SGOPTION_DIFFICULTY) || (mRefreshFlag & MGOPTION_PLAYERS) )
			{
				font_san.center_put( 718, offsetY+74, 780, offsetY+108, 
					m.format(tempConfig.multi_player_difficulty(regPlayerCount-1)), 1 );
			}

			// -------- repaint button -------//
			if( refreshFlag & SGOPTION_PAGE )
			{
				if( remote.is_host )
					startButton.paint();
				readyButton.paint();
				returnButton.paint();
			}

			refreshFlag = 0;
			mRefreshFlag = 0;
		}
		sys.blt_virtual_buf();

		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();

		// --------- detect remote message -------//
		recvPtr = mp_obj.receive(&from, &to, &recvLen, &sysMsgCount);

		if( sysMsgCount )
		{
			// see any player disconnect
			for(int q = 0; q < regPlayerCount; ++q)
			{
				if( !mp_obj.is_player_connecting(regPlayerId[q]) )
				{
					mRefreshFlag |= MGOPTION_PLAYERS;

					memmove( regPlayerId+q, regPlayerId+q+1, (MAX_NATION-1-q)*sizeof(regPlayerId[0]) );
					regPlayerId[MAX_NATION-1] = 0;
					memmove( playerReadyFlag+q, playerReadyFlag+q+1, (MAX_NATION-1-q)*sizeof(playerReadyFlag[0]) );
					playerReadyFlag[MAX_NATION-1] = 0;
					short freeColor = playerColor[q];
					memmove( playerColor+q, playerColor+q+1, (MAX_NATION-1-q)*sizeof(playerColor[0]) );
					playerColor[MAX_NATION-1] = 0;
					if(freeColor > 0 && freeColor <= MAX_COLOR_SCHEME)
						colorAssigned[freeColor-1] = 0;
					short freeRace = playerRace[q];
					memmove( playerRace+q, playerRace+q+1, (MAX_NATION-1-q)*sizeof(playerRace[0]) );
					playerRace[MAX_NATION-1] = 0;
					if(freeRace > 0 && freeRace <= MAX_RACE)
						raceAssigned[freeRace-1]--;
					memmove( playerBalance+q, playerBalance+q+1, (MAX_NATION-1-q)*sizeof(playerBalance[0]) );
					playerBalance[MAX_NATION-1] = 0;
					--regPlayerCount;
					--q;
				}
			}
		}

		if( recvPtr )
		{
			if( ((MpStructBase *)recvPtr)->msg_id == MPMSG_START_GAME )
			{
				retFlag = 1;
				break;		// break while(1) loop
			}
			else
			{
				switch( ((MpStructBase *)recvPtr)->msg_id )
				{
				case MPMSG_ABORT_GAME:
					return 0;
				case MPMSG_SEND_CONFIG:
					tempConfig.change_game_setting( ((MpStructConfig *)recvPtr)->game_config );
					refreshFlag |= SGOPTION_ALL_OPTIONS;
					break;
				case MPMSG_RANDOM_SEED:
					info.init_random_seed( ((MpStructSeed *)recvPtr)->seed );
					break;
					// ####### begin Gilbert 25/10 #######//
				case MPMSG_RANDOM_SEED_STR:
					msgSeedStr = *(MpStructSeedStr *)recvPtr;
					mapIdField.select_whole();
					refreshFlag |= SGOPTION_MAP_ID;
					break;
					// ####### end Gilbert 25/10 #######//
				case MPMSG_NEW_PLAYER:
					mp_obj.poll_players();
					if( remote.is_host )
					{
						if( regPlayerCount < MAX_NATION )
						{
							MpStructNewPlayer *newPlayerMsg = (MpStructNewPlayer *)recvPtr;
							regPlayerId[regPlayerCount] = newPlayerMsg->player_id;
							playerReadyFlag[regPlayerCount] = 0;

							// send accept new player to all player
							MpStructAcceptNewPlayer msgAccept(from);
							mp_obj.send_stream( BROADCAST_PID, &msgAccept, sizeof(msgAccept) );

							// assign initial race
							int c = m.get_time() % MAX_RACE;
							int t;
							for( t = 0; t < MAX_RACE; ++t, ++c )
							{
								c %= MAX_RACE;
								if( raceAssigned[c] == 0 )
								{
									raceAssigned[c]++;
									playerRace[regPlayerCount] = c+1;
									MpStructAcceptRace msgAcceptRace(from, c+1);
									mp_obj.send_stream( from, &msgAcceptRace, sizeof(msgAcceptRace) );
									break;
								}
							}
							err_when( t >= MAX_RACE );		// not found

							// assign initial color
							c = m.get_time() % MAX_COLOR_SCHEME;
							for( t = 0; t < MAX_COLOR_SCHEME; ++t, ++c )
							{
								c %= MAX_COLOR_SCHEME;
								if( !colorAssigned[c] )
								{
									colorAssigned[c]=1;
									playerColor[regPlayerCount] = c+1;
									MpStructAcceptColor msgAcceptColor(from, c+1);
									mp_obj.send_stream( from, &msgAcceptColor, sizeof(msgAcceptColor) );
									break;
								}
							}
							err_when( t >= MAX_COLOR_SCHEME );		// not found

							// send random seed
							// ###### begin Gilbert 25/10 #######//
							// MpStructSeed msgRandomSeed(m.get_random_seed());
							// mp_obj.send_stream( from, &msgRandomSeed, sizeof(msgRandomSeed) );
							mp_obj.send_stream( from, &msgSeedStr, sizeof(msgSeedStr) );
							// ###### end Gilbert 25/10 #######//

							// send config 
							MpStructConfig msgConfig( tempConfig );
							mp_obj.send_stream( from, &msgConfig, sizeof(msgConfig) );

							// send ready flag
							for( c = 0; c < regPlayerCount; ++c)
							{
								if( playerReadyFlag[c] )
								{
									MpStructPlayerReady msgReady(regPlayerId[c]);
									mp_obj.send_stream(from, &msgReady, sizeof(msgReady));
								}
							}

							// ###### patch begin Gilbert 22/1 ######//
							// send remote.sync_test_level
							MpStructSyncLevel msgSyncTest(remote.sync_test_level);
							mp_obj.send_stream( from, &msgSyncTest, sizeof(msgSyncTest) );
							// ###### patch end Gilbert 22/1 ######//

							// update balance
							playerBalance[regPlayerCount] = newPlayerMsg->player_balance;

							regPlayerCount++;
							mRefreshFlag |= MGOPTION_PLAYERS;
						}
						else
						{
							// reply refuse new player
							MpStructRefuseNewPlayer msgRefuse(from);
							mp_obj.send_stream( BROADCAST_PID, &msgRefuse, sizeof(msgRefuse) );
						}
					}
					break;
				case MPMSG_LOAD_GAME_NEW_PLAYER:
					{
						// incorrect message, reject
						MpStructRefuseNewPlayer msgRefuse(from);
						mp_obj.send_stream(BROADCAST_PID, &msgRefuse, sizeof(msgRefuse) );
					}
					break;
				case MPMSG_ACCEPT_NEW_PLAYER:
					hostPlayerId = from;
					if( regPlayerCount < MAX_NATION && ((MpStructAcceptNewPlayer *)recvPtr)->player_id != mp_obj.get_my_player_id() )
					{
						// search if this player has existed
						for( p=0; p < regPlayerCount && regPlayerId[p] != ((MpStructAcceptNewPlayer *)recvPtr)->player_id; ++p );
						regPlayerId[p] = ((MpStructAcceptNewPlayer *)recvPtr)->player_id;
						playerReadyFlag[p] = 0;
						if( p >= regPlayerCount )
						{
							regPlayerCount++;		// now regPlayerCount == p
							err_when( p != regPlayerCount );
						}
						mRefreshFlag |= MGOPTION_PLAYERS;
					}
					break;
				case MPMSG_ACQUIRE_RACE:
					if( remote.is_host )
					{
						short cl = ((MpStructAcquireRace *)recvPtr)->race_id;
						if( !cl )
						{
							for( cl = 1; cl <= MAX_RACE; ++cl)
							{
								if( raceAssigned[cl-1] == 0 )
									break;
							}
						}
						int p;
						for( p = 0; p < regPlayerCount && regPlayerId[p] != from; ++p );
						if( cl <= MAX_RACE && p < regPlayerCount &&
							(shareRace || raceAssigned[cl-1] == 0 ) )		// more than one player can use the same race
						{
							// unassign current race
							if( playerRace[p] > 0)
								raceAssigned[playerRace[p]-1]--;

							// mark race assigned
							raceAssigned[cl-1]++;
							playerRace[p] = cl;

							// reply accept race
							MpStructAcceptRace msgAcceptRace(from, cl );
							mp_obj.send_stream( from, &msgAcceptRace, sizeof(msgAcceptRace) );
						}
						else
						{
							// reply refuse race
							MpStructRefuseRace msgRefuseRace(from, ((MpStructAcquireRace *)recvPtr)->race_id );
							mp_obj.send_stream( from, &msgRefuseRace, sizeof(msgRefuseRace) );
						}
					}
					break;
				case MPMSG_ACCEPT_RACE:
					if( ((MpStructAcceptRace *)recvPtr)->request_player_id == mp_obj.get_my_player_id() )
					{
						tempConfig.race_id = char(((MpStructAcceptRace *)recvPtr)->race_id);
						refreshFlag |= SGOPTION_RACE;
					}
					break;
				case MPMSG_REFUSE_RACE:
					if( ((MpStructRefuseRace *)recvPtr)->request_player_id == mp_obj.get_my_player_id() )
					{
						refreshFlag |= SGOPTION_RACE;
						// sound effect here
					}
					break;
				case MPMSG_ACQUIRE_COLOR:
					if( remote.is_host )
					{
						short cl = ((MpStructAcquireColor *)recvPtr)->color_scheme_id;
						if( !cl )
						{
							for( cl = 1; cl <= MAX_COLOR_SCHEME && colorAssigned[cl-1]; ++cl);
						}
						int p;
						for( p = 0; p < regPlayerCount && regPlayerId[p] != from; ++p );
						if( cl <= MAX_COLOR_SCHEME && !colorAssigned[cl-1] && p < regPlayerCount )
						{
							if( playerColor[p] > 0 )
								colorAssigned[playerColor[p]-1] = 0;

							// mark color assigned
							colorAssigned[cl-1] = 1;
							playerColor[p] = cl;

							// reply accept color
							MpStructAcceptColor msgAcceptColor(from, cl );
							mp_obj.send_stream( from, &msgAcceptColor, sizeof(msgAcceptColor) );
						}
						else
						{
							// reply refuse color
							MpStructRefuseColor msgRefuseColor(from, ((MpStructAcquireColor *)recvPtr)->color_scheme_id );
							mp_obj.send_stream( from, &msgRefuseColor, sizeof(msgRefuseColor) );
						}
					}
					break;
				case MPMSG_ACCEPT_COLOR:
					if( ((MpStructAcceptColor *)recvPtr)->request_player_id == mp_obj.get_my_player_id() )
					{
						tempConfig.player_nation_color = char(((MpStructAcceptColor *)recvPtr)->color_scheme_id);
						refreshFlag |= SGOPTION_COLOR;
					}
					break;
				case MPMSG_REFUSE_COLOR:
					if( ((MpStructRefuseColor *)recvPtr)->request_player_id == mp_obj.get_my_player_id() )
					{
						refreshFlag |= SGOPTION_COLOR;
						// sound effect here
					}
					break;
				case MPMSG_PLAYER_READY:
					{
						for( int p = 0; p < regPlayerCount; ++p)
						{
							if( regPlayerId[p] == ((MpStructPlayerReady *)recvPtr)->player_id )
							{
								playerReadyFlag[p] = 1;
								mRefreshFlag |= MGOPTION_PLAYERS;
							}
						}
					}
					break;
				case MPMSG_PLAYER_UNREADY:
					{
						for( int p = 0; p < regPlayerCount; ++p)
						{
							if( regPlayerId[p] == ((MpStructPlayerUnready *)recvPtr)->player_id )
							{
								playerReadyFlag[p] = 0;
								mRefreshFlag |= MGOPTION_PLAYERS;
							}
						}
					}
					break;
				case MPMSG_SEND_CHAT_MSG:
					while( messageList.size() >= 4 )
						messageList.linkout(1);
					messageList.linkin(recvPtr);
					mRefreshFlag |= MGOPTION_IN_MESSAGE;
					break;
				// ###### patch begin Gilbert 22/1 ######//
				case MPMSG_SEND_SYNC_TEST_LEVEL:
					remote.sync_test_level = ((MpStructSyncLevel *)recvPtr)->sync_test_level;
					break;
				// ###### patch end Gilbert 22/1 ######//
				case MPMSG_REFUSE_NEW_PLAYER:
					if( ((MpStructRefuseNewPlayer *)recvPtr)->player_id == mp_obj.get_my_player_id() )
					{
						box.msg("You cannot join the game because the multiplayer saved game you selected is different from those of other human players.");
						return 0;
					}
					break;
				default:		// if the game is started, any other thing is received
					return 0;
				}
			}
		}

		// --------- detect basic option -------- //
		if( optionMode == OPTION_BASIC )
		{
			if( !selfReadyFlag )
			{
				if( raceGroup.detect() >= 0)
				{
					int r = tempConfig.race_id = raceGroup[raceGroup()].custom_para.value;
					if( remote.is_host )
					{
						int p;
						for( p = 0; p < regPlayerCount && regPlayerId[p] != mp_obj.get_my_player_id() ; ++p );
						if( r <= MAX_RACE && p < regPlayerCount &&
							(shareRace || raceAssigned[r-1] == 0 ) )		// more than one player can use the same race
						{
							// unassign current race
							if( playerRace[p] > 0)
								raceAssigned[playerRace[p]-1]--;

							// mark race assigned
							raceAssigned[r-1]++;
							playerRace[p] = r;

							tempConfig.race_id = r;
						}
						refreshFlag |= SGOPTION_RACE;
					}
					else
					{
						MpStructAcquireRace msgAcquire(r);
						mp_obj.send_stream(hostPlayerId, &msgAcquire, sizeof( msgAcquire) );
					}
					//refreshFlag |= SGOPTION_RACE;
				}
				else if( colorGroup.detect() >= 0)
				{
					int r = colorGroup[colorGroup()].custom_para.value;
					if( remote.is_host )
					{
						if( !colorAssigned[r-1] )
						{
							int p;
							for( p = 0; p < regPlayerCount && regPlayerId[p] != mp_obj.get_my_player_id(); ++p );
							if( r <= MAX_COLOR_SCHEME && !colorAssigned[r-1] && p < regPlayerCount )
							{
								// unmark current color
								if( playerColor[p] > 0 )
									colorAssigned[playerColor[p]-1] = 0;
								// mark color assigned
								colorAssigned[r-1] = 1;
								playerColor[p] = r;
								tempConfig.player_nation_color = r;
							}
						}
						refreshFlag |= SGOPTION_COLOR;
					}
					else
					{
						MpStructAcquireColor msgAcquire(r);
						mp_obj.send_stream(hostPlayerId, &msgAcquire, sizeof( msgAcquire) );
					}
					//refreshFlag |= SGOPTION_COLOR;
				}
			}
		}

		// -------- detect other option, only host can change ---------//

		unsigned keyCode = 0;

		if( remote.is_host && !selfReadyFlag)
		{
			int configChange = 0;

			// ------- detect basic option ---------//
			if( optionMode == OPTION_BASIC )
			{
				if( aiNationInc.detect() )
				{
					tempConfig.ai_nation_count++;
					if( tempConfig.ai_nation_count >= MAX_NATION )
						tempConfig.ai_nation_count = MAX_NATION-1;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					refreshFlag |= SGOPTION_AI_NATION | SGOPTION_DIFFICULTY;
				}
				else if( aiNationDec.detect() )
				{
					tempConfig.ai_nation_count--;
					if( tempConfig.ai_nation_count < 0 )
						tempConfig.ai_nation_count = 0;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					refreshFlag |= SGOPTION_AI_NATION | SGOPTION_DIFFICULTY;
				}
				else if( diffGroup.detect() >= 0)
				{
					if( diffGroup[diffGroup()].custom_para.value != OPTION_CUSTOM )
					{
						tempConfig.change_difficulty(diffGroup[diffGroup()].custom_para.value);
						configChange = 1;
						// all but SGOPTION_PAGE;
						refreshFlag |= SGOPTION_ALL & ~SGOPTION_PAGE;
					}
				}
				else if( terrainGroup.detect() >= 0)
				{
					tempConfig.terrain_set = terrainGroup[terrainGroup()].custom_para.value;
					static short latitudeArray[3] = { 45, 70, 20 };
					err_when( tempConfig.terrain_set <= 0 || tempConfig.terrain_set > 3 );
					tempConfig.latitude = latitudeArray[tempConfig.terrain_set-1];
					configChange = 1;
					//refreshFlag |= SGOPTION_TERRAIN;
				}
				else if( landGroup.detect() >= 0)
				{
					tempConfig.land_mass = landGroup[landGroup()].custom_para.value;
					configChange = 1;
					//refreshFlag |= SGOPTION_LAND_MASS;
				}
			}

			// ------- detect advanced option ---------//

			else if( optionMode == OPTION_ADVANCED )
			{
				// ###### begin Gilbert 24/10 ######//
				MpStructSeedStr oldMapStr(msgSeedStr);
				if( keyCode = mapIdField.detect() )
				{
					if( strcmp(oldMapStr.seed_str, msgSeedStr.seed_str) )
						mp_obj.send_stream( BROADCAST_PID, &msgSeedStr, sizeof(msgSeedStr) );
					refreshFlag |= SGOPTION_MAP_ID;
				}
				else
				// ###### end Gilbert 24/10 ######//
				if( exploreGroup.detect() >= 0 )
				{
					tempConfig.explore_whole_map = exploreGroup[exploreGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_EXPLORED;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( fogGroup.detect() >= 0 )
				{
					tempConfig.fog_of_war = fogGroup[fogGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_FOG
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( treasureGroup.detect() >= 0 )
				{
					tempConfig.start_up_cash = treasureGroup[treasureGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_TREASURE;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( aiTreasureGroup.detect() >= 0 )
				{
					tempConfig.ai_start_up_cash = aiTreasureGroup[aiTreasureGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_AI_TREASURE;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( aiAggressiveGroup.detect() >= 0 )
				{
					tempConfig.ai_aggressiveness = 
						aiAggressiveGroup[aiAggressiveGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_AI_AGGRESSIVE;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( monsterGroup.detect() >= 0 )
				{
					tempConfig.monster_type = monsterGroup[monsterGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_FRYHTANS;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( randomStartUpGroup.detect() >= 0)
				{
					tempConfig.random_start_up = randomStartUpGroup[randomStartUpGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_RANDOM_STARTUP;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}

			}

			// -------- detect advanced option ---------//

			else if( optionMode == OPTION_ADVANCE2 )
			{
				if( rawSiteInc.detect() )
				{
					if( ++tempConfig.start_up_raw_site > 7 )
						tempConfig.start_up_raw_site = 7;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					refreshFlag |= SGOPTION_RAW | SGOPTION_DIFFICULTY;
				}
				else if( rawSiteDec.detect() )
				{
					if( --tempConfig.start_up_raw_site < 1 )
						tempConfig.start_up_raw_site = 1;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					refreshFlag |= SGOPTION_RAW | SGOPTION_DIFFICULTY;
				}
				else if( nearRawGroup.detect() >= 0)
				{
					tempConfig.start_up_has_mine_nearby = nearRawGroup[nearRawGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_NEAR_RAW;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( townStartGroup.detect() >= 0)
				{
					tempConfig.start_up_independent_town = townStartGroup[townStartGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// resfreshFlag |= SGOPTION_START_TOWN;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( townResistGroup.detect() >= 0)
				{
					tempConfig.independent_town_resistance = townResistGroup[townResistGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// resfreshFlag |= SGOPTION_TOWN_RESIST;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( townEmergeGroup.detect() >= 0)
				{
					tempConfig.new_independent_town_emerge = townEmergeGroup[townEmergeGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_TOWN_EMERGE;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( nationEmergeGroup.detect() >= 0)
				{
					tempConfig.new_nation_emerge = nationEmergeGroup[nationEmergeGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_NATION_EMERGE;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
				else if( randomEventGroup.detect() >= 0)
				{
					tempConfig.random_event_frequency = randomEventGroup[randomEventGroup()].custom_para.value;
					tempConfig.difficulty_level = OPTION_CUSTOM;
					configChange = 1;
					// refreshFlag |= SGOPTION_RANDOM_EVENT;
					refreshFlag |= SGOPTION_DIFFICULTY;
				}
			}	
			
			// -------- detect goal option ----------//

			else if( optionMode == OPTION_GOAL )
			{
				if( clearEnemyButton.detect() )
				{
				}
				else if( clearMonsterButton.detect() )
				{
					tempConfig.goal_destroy_monster = clearMonsterButton.pushed_flag;
					configChange = 1;
				}
				else if( enoughPeopleButton.detect() )
				{
					tempConfig.goal_population_flag = enoughPeopleButton.pushed_flag;
					configChange = 1;
				}
				else if( enoughIncomeButton.detect() )
				{
					tempConfig.goal_economic_score_flag = enoughIncomeButton.pushed_flag;
					configChange = 1;
				}
				// ##### begin Gilbert 24/10 #######//
				else if( enoughScoreButton.detect() )
				{
					tempConfig.goal_total_score_flag = enoughScoreButton.pushed_flag;
					configChange = 1;
				}
				// ##### end Gilbert 24/10 #######//
				else if( timeLimitButton.detect() )
				{
					tempConfig.goal_year_limit_flag = timeLimitButton.pushed_flag;
					configChange = 1;
				}
				else if( peopleInc.detect() )
				{
					tempConfig.goal_population += 100;
					if( tempConfig.goal_population > 5000 )
						tempConfig.goal_population = 5000;
					configChange = 1;
					refreshFlag |= SGOPTION_ENOUGH_PEOPLE;
				}
				else if( peopleDec.detect() )
				{
					tempConfig.goal_population -= 100;
					if( tempConfig.goal_population < 100 )
						tempConfig.goal_population = 100;
					configChange = 1;
					refreshFlag |= SGOPTION_ENOUGH_PEOPLE;
				}
				// ###### begin Gilbert 24/10 #######//
				else if( incomeInc.detect() )
				{
					tempConfig.goal_economic_score += 100;
					if( tempConfig.goal_economic_score > 5000 )
					{
						tempConfig.goal_economic_score = 5000;
					}
					configChange = 1;
					refreshFlag |= SGOPTION_ENOUGH_INCOME;
				}
				else if( incomeDec.detect() )
				{
					tempConfig.goal_economic_score -= 100;
					if( tempConfig.goal_economic_score < 100 )
					{
						tempConfig.goal_economic_score = 100;
					}
					configChange = 1;
					refreshFlag |= SGOPTION_ENOUGH_INCOME;
				}
				else if( scoreInc.detect() )
				{
					if( tempConfig.goal_total_score >= 2000 )
						tempConfig.goal_total_score += 500;
					else
						tempConfig.goal_total_score += 100;
					if( tempConfig.goal_total_score > 10000 )
						tempConfig.goal_total_score = 10000;
					configChange = 1;
					refreshFlag |= SGOPTION_ENOUGH_SCORE;
				}
				else if( scoreDec.detect() )
				{
					if( tempConfig.goal_total_score > 2000 )
						tempConfig.goal_total_score -= 500;
					else
						tempConfig.goal_total_score -= 100;
					if( tempConfig.goal_total_score < 100 )
						tempConfig.goal_total_score = 100;
					configChange = 1;
					refreshFlag |= SGOPTION_ENOUGH_SCORE;
				}
				// ###### end Gilbert 24/10 #######//
				else if( yearInc.detect() )
				{
					if( tempConfig.goal_year_limit >= 20 )
						tempConfig.goal_year_limit += 5;
					else
						tempConfig.goal_year_limit++;
					if( tempConfig.goal_year_limit > 100 )
					{
						tempConfig.goal_year_limit = 100;
					}
					configChange = 1;
					refreshFlag |= SGOPTION_TIME_LIMIT;
				}
				else if( yearDec.detect() )
				{
					if( tempConfig.goal_year_limit > 20 )
						tempConfig.goal_year_limit -= 5;
					else
						tempConfig.goal_year_limit--;
					if( tempConfig.goal_year_limit < 1 )
					{
						tempConfig.goal_year_limit = 1;
					}
					configChange = 1;
					refreshFlag |= SGOPTION_TIME_LIMIT;
				}
			}

			if( configChange )
			{
				MpStructConfig msgConfig(tempConfig);
				mp_obj.send_stream( BROADCAST_PID, &msgConfig, sizeof(msgConfig) );
			}
		}

		// --------- detect switch option button ------//

		if( mouse.single_click(96, offsetY+12, 218, offsetY+54) )
		{
			if( optionMode != OPTION_BASIC )
			{
				optionMode = OPTION_BASIC;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}
		else if( mouse.single_click(236, offsetY+12, 363, offsetY+54) )
		{
			if( optionMode != OPTION_ADVANCED )
			{
				optionMode = OPTION_ADVANCED;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}
		else if( mouse.single_click(380, offsetY+12, 506, offsetY+54) )
		{
			if( optionMode != OPTION_ADVANCE2 )
			{
				optionMode = OPTION_ADVANCE2;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}
		else if( mouse.single_click(523, offsetY+12, 649, offsetY+54) )
		{
			if( optionMode != OPTION_GOAL )
			{
				optionMode = OPTION_GOAL;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}

		// --------- detect ready button button --------//

		if( readyButton.detect() )
		{
			mRefreshFlag |= MGOPTION_PLAYERS;
			for(p = 0; p < regPlayerCount && regPlayerId[p] != mp_obj.get_my_player_id(); ++p);
			if( p < regPlayerCount )
			{
				if( !selfReadyFlag ) 
				{
					playerReadyFlag[p] = selfReadyFlag = 1;
					MpStructPlayerReady msgReady(mp_obj.get_my_player_id());
					mp_obj.send_stream(BROADCAST_PID, &msgReady, sizeof(msgReady));
				}
				else
				{
					// else un-ready this player
					playerReadyFlag[p] = selfReadyFlag = 0;
					MpStructPlayerUnready msgUnready(mp_obj.get_my_player_id());
					mp_obj.send_stream(BROADCAST_PID, &msgUnready, sizeof(msgUnready));
				}
			}
		}
		if( remote.is_host && startButton.detect() && regPlayerCount >= 2 )
		{
			// see if all player is ready
			short sumBalance = 0;
			int q;
			for( q = 0; q < regPlayerCount && playerReadyFlag[q]; ++q)
			{
				err_when( playerBalance[q] == 0 );
				sumBalance += playerBalance[q];
			}
			if( q >= regPlayerCount )		// not all playerReadyFlag[p] = 1;
			{
#ifdef DEBUG
				sumBalance = 0;
#endif
#ifdef BETA
				sumBalance = 0;
#endif
				if( sumBalance >= 0 )
				{
//					MpStructBase msgStart(MPMSG_START_GAME);
//					mp_obj.send_stream(BROADCAST_PID, &msgStart, sizeof(msgStart));
					retFlag = 1;
					break;							// break while(1)
				}
				else
				{
					// too many non-CD version
					box.msg("There are not enough Seven Kingdoms CDROMs to start the game. Each CDROM supports "PLAYER_RATIO_STRING" players.");
				}
			}
		}
		else if( returnButton.detect() )
		{
			if( remote.is_host )
			{
				MpStructBase msgAbort(MPMSG_ABORT_GAME);
				mp_obj.send_stream(BROADCAST_PID, &msgAbort, sizeof(msgAbort) );
			}
			retFlag = 0;
			break;			// break while(1)
		}
		else if( !keyCode && (keyCode = messageField.detect()) != 0)		// keyCode may be non-zero if after mapIdField.detect()
		{
			mRefreshFlag |= MGOPTION_OUT_MESSAGE;
			if(keyCode == KEY_RETURN && strlen(typingMsg.content) > 0)
			{
				// send message
				mp_obj.send_stream(BROADCAST_PID, &typingMsg, sizeof(typingMsg) );

				// clear the string
				messageField.clear();
			}
			else if( keyCode == KEY_ESC )
			{
				messageField.clear();
			}
		}

		// ####### begin Gilbert 24/10 #######//
		vga_front.unlock_buf();
		// ####### end Gilbert 24/10 #######//
	}

	// ###### begin Gilbert 24/10 #######//
	if( !vga_front.buf_locked )
		vga_front.lock_buf();
	// ###### end Gilbert 24/10 #######//

	// ---------- final setup to start multiplayer game --------//

	if( retFlag )
	{
		retFlag = 0;

		if( remote.is_host )
			mp_obj.disable_join_session();

		// mp_obj.poll_players();
		nation_array.init();
		nation_array.zap();
			
		int trial;
		unsigned long startTime;
		int playerCount = 0;

		if( remote.is_host )
		{
			VLenQueue setupString;

			// ------- put start game string -------//

			{
				MpStructBase msgStart(MPMSG_START_GAME);
				memcpy( setupString.reserve(sizeof(msgStart)), &msgStart, sizeof(msgStart) );
			}

			// -------- put random seed -------- //
			do
			{
				info.init_random_seed( atol(msgSeedStr.seed_str) );
			} while (m.get_random_seed() == 0L);
			{
				MpStructSeed msgSeed(m.get_random_seed());
				memcpy( setupString.reserve(sizeof(msgSeed)), &msgSeed, sizeof(msgSeed) );
			}

			// -------- send config ------------//
			{
				MpStructConfig msgConfig(tempConfig);
				memcpy( setupString.reserve(sizeof(msgConfig)), &msgConfig, sizeof(msgConfig) );
			}

			// BUGHERE : terrain_set 2 is not available
#ifndef AMPLUS
			if( tempConfig.terrain_set == 2)
				tempConfig.terrain_set = 1;
#endif

			config = tempConfig;		// nation_array.new_nation reads setting from config

			// -------- setup nation now ------------ //

			info.init();

			playerCount = 0;

			for( p = 0; p < regPlayerCount; ++p )
			{
				// ensure it is a valid player
				PID_TYPE playerId = regPlayerId[p];
				PlayerDesc *player = mp_obj.search_player(playerId);
				if( !playerId || !player || !player->connecting )
					continue;

				nationPara[playerCount].init(playerCount+1, playerId, playerColor[p], playerRace[p], player->friendly_name_str());
				((MpStructNation *)setupString.reserve(sizeof(MpStructNation)))->init(
					playerCount+1, playerId, playerColor[p], playerRace[p], player->friendly_name_str());

				playerCount++;
			}

			*mpPlayerCount = playerCount;
			config.difficulty_rating = config.multi_player_difficulty(playerCount-1);

			// ---- force set to the lowest frame delay -------//

#ifdef AMPLUS
			remote.set_process_frame_delay(FORCE_MAX_FRAME_DELAY);
			{
				MpStructProcessFrameDelay msgFrameDelay(remote.get_process_frame_delay());
				memcpy( setupString.reserve(sizeof(msgFrameDelay)), &msgFrameDelay, sizeof(msgFrameDelay));
			}
#endif

			// -------- send sync test level ----------//

			{
				MpStructSyncLevel msgSyncTest(remote.sync_test_level);
				memcpy( setupString.reserve(sizeof(msgSyncTest)), &msgSyncTest, sizeof(msgSyncTest));
			}

			mp_obj.send_stream(BROADCAST_PID, setupString.queue_buf, setupString.length() );
		}
		else
		{
			// use the message recving MPMSG_START_GAME

			err_when( !recvPtr);

			DWORD offset = 0;
			int recvStartMsg = 0;
			int recvSeed = 0;
			int recvConfig = 0;
			int ownPlayerFound = 0;
			playerCount = 0;
			char *oriRecvPtr = recvPtr;
			int recvSetFrameDelay = 0;
			int recvSyncTestLevel = 0;

			// process the string received
			while( offset < recvLen )
			{
				DWORD oldOffset = offset;
				recvPtr = oriRecvPtr + offset;

				switch( ((MpStructBase *)(recvPtr))->msg_id )
				{
				case MPMSG_START_GAME:
					recvStartMsg = 1;
					offset += sizeof( MpStructBase );
					break;

				case MPMSG_RANDOM_SEED:
					err_when( ((MpStructSeed *) recvPtr)->seed == 0L);
					info.init_random_seed( ((MpStructSeed *) recvPtr)->seed );
					offset += sizeof( MpStructSeed );
					++recvSeed;
					break;

				case MPMSG_SEND_CONFIG:
					tempConfig.change_game_setting( ((MpStructConfig *) recvPtr)->game_config );
					offset += sizeof( MpStructConfig );
					++recvConfig;
#ifndef AMPLUS
					if( tempConfig.terrain_set == 2)
						tempConfig.terrain_set = 1;
#endif
					config = tempConfig;		// nation_array.new_nation reads setting from config
					break;

				case MPMSG_DECLARE_NATION:
					{
						if( playerCount == 0 )
							info.init();			// info.init for the first time

						MpStructNation *msgNation = (MpStructNation *)recvPtr;
						nationPara[playerCount].init( msgNation->nation_recno,
							msgNation->dp_player_id, msgNation->color_scheme, 
							msgNation->race_id, msgNation->player_name);

						if( msgNation->dp_player_id == mp_obj.get_my_player_id() )
							ownPlayerFound++;
						*mpPlayerCount = ++playerCount;
						offset += sizeof( MpStructNation );
						config.difficulty_rating = config.multi_player_difficulty(playerCount-1);
					}
					break;

				case MPMSG_SET_PROCESS_FRAME_DELAY:
					remote.set_process_frame_delay(((MpStructProcessFrameDelay *)recvPtr)->common_process_frame_delay);
					++recvSetFrameDelay;
					offset += sizeof( MpStructProcessFrameDelay );
					break;

				case MPMSG_SEND_SYNC_TEST_LEVEL:
					remote.sync_test_level = ((MpStructSyncLevel *)recvPtr)->sync_test_level;
					++recvSyncTestLevel;
					offset += sizeof( MpStructSyncLevel );
					break;
				}  // end switch

				if( !recvStartMsg || offset <= oldOffset )
				{
					err_here();
					box.msg( "Connection string from host is corrupted" );
					return 0;
				}
			} // end while

			if( !recvSeed )
			{
				box.msg( "Cannot get random seeds from the host." );
				return 0;
			}
			err_when( recvSeed > 1 );
			if( !recvConfig )
			{
				box.msg( "Cannot get game configuration info from the host." );
				return 0;
			}
			err_when( recvConfig > 1 );
			if( playerCount == 0 )
			{
				box.msg( "Cannot get kingdom information from the host." );
				return 0;
			}
			err_when( playerCount > MAX_NATION );
			if( !ownPlayerFound )
			{
				box.msg( "The host cannot recognize your machine." );
				return 0;
			}
			err_when( ownPlayerFound > 1 );
			if( !recvSetFrameDelay || !recvSyncTestLevel )
			{
				box.msg("Cannot receive important information from the host");
				return 0;
			}
			err_when( recvSetFrameDelay > 1 );
			err_when( recvSyncTestLevel > 1 );
		}

		if( remote.sync_test_level == 0)
		{
			remote.set_alternating_send(playerCount > 4 ? 4 : playerCount);		// automatic setting
		}

		{
			// ------- broadcast end setting string ------- //

			MpStructBase mpEndSetting(MPMSG_END_SETTING);
			mp_obj.send_stream( BROADCAST_PID, &mpEndSetting, sizeof(mpEndSetting) );

			// ------ wait for MPMSG_END_SETTING ----------//
			// ---- to filter other all message until MP_MSG_END_SETTING ---//

			trial = 5000;
			startTime = m.get_time();
			int recvEndSetting = 0;
			while( --trial > 0 || m.get_time() - startTime < 10000 )
			{
				if( recvEndSetting >= playerCount-1)
					break;
				recvPtr = mp_obj.receive( &from, &to, &recvLen);
				if( recvPtr )
				{
					trial = MAX(trial, 1000);
					startTime = m.get_time();
					if( ((MpStructBase *)recvPtr)->msg_id == MPMSG_END_SETTING )
					{
						recvEndSetting++;
					}
				}
			}
			if( recvEndSetting < playerCount-1 )
			{
				box.msg("Some player(s) encountered errors when initializing the game.");
				// but continue
			}
		}

		retFlag = 1;
	}		// end if(retFlag)

	return retFlag;
}
#ifdef Y_SHIFT
	#error
#endif
#undef Y_SHIFT_FLAG
//-------- End of function Game::mp_select_option -----------//


//-------- Begin of function Game::mp_select_load_option -----------//
// return 0 = cancel, 1 = ok
int Game::mp_select_load_option(char *fileName)
{
	const int offsetY = 212;
	char optionMode = OPTION_BASIC;
	char menuTitleBitmap[] = "TOP-LMPG";
	
	Config &tempConfig = config;

	PID_TYPE from, to;
	uint32_t recvLen;
	int sysMsgCount;
	char *recvPtr;

	char raceAssigned[MAX_RACE];		// master copy belongs to host's
	char colorAssigned[MAX_COLOR_SCHEME];		// master copy belongs to host's
	//static short raceX[MAX_RACE] = {390, 313, 236, 467, 390, 236, 313};
	//static short raceY[MAX_RACE] = {276, 276, 276, 276, 243, 243, 243};
	//const raceWidth = 77;
	//const raceHeight = 33;
	//static short colorX[MAX_COLOR_SCHEME] = {545, 581, 581, 617, 617, 545, 653};
	//static short colorY[MAX_COLOR_SCHEME] = {243, 276, 243, 276, 243, 276, 276};
	//const colorWidth = 36;
	//const colorHeight = 33;
	static short tickX[MAX_NATION] = { 103, 254, 405, 556, 103, 254, 405 };
	static short tickY[MAX_NATION] = {  53,  53,  53,  53,  73,  73,  73 };
	const int nameOffsetX = 23;
	const int nameOffsetY = 3;

	DynArray messageList(sizeof(MpStructChatMsg));
	MpStructChatMsg typingMsg(tempConfig.player_name, NULL);
	
	memset( raceAssigned, 0, sizeof(raceAssigned) );
	memset( colorAssigned, 0, sizeof(colorAssigned) );

	PID_TYPE hostPlayerId = 0;
	PID_TYPE regPlayerId[MAX_NATION];
	memset( regPlayerId, 0, sizeof(regPlayerId) );
	char playerReadyFlag[MAX_NATION];
	memset( playerReadyFlag, 0, sizeof(playerReadyFlag) );
	short playerRace[MAX_NATION];	// host only
	memset( playerRace, 0, sizeof(playerRace) );
	short playerColor[MAX_NATION];	// host only
	memset( playerColor, 0, sizeof(playerColor) );
	short playerBalance[MAX_NATION];
	memset( playerBalance, 0, sizeof(playerBalance) );

	int regPlayerCount = 0;
	int selfReadyFlag = 0;
	int maxPlayer;
	int shareRace = 1;		// host only, 0= exclusive race of each player

	mp_obj.poll_players();
	int p;
	for( p = 1; p <= MAX_NATION && mp_obj.get_player(p); ++p )
	{
		// host only identify himself
		if( !remote.is_host || mp_obj.get_player(p)->pid() == mp_obj.get_my_player_id() )
		{
			regPlayerId[regPlayerCount] = mp_obj.get_player(p)->pid();
			playerReadyFlag[regPlayerCount] = 0;
			playerColor[regPlayerCount] = 0;
			playerRace[regPlayerCount] = 0;
			playerBalance[regPlayerCount] = 0;
			++regPlayerCount;
		}
	}

	err_when( tempConfig.race_id != (~nation_array)->race_id );
	err_when( tempConfig.player_nation_color != (~nation_array)->color_scheme_id );

	int i;
	long refreshFlag = SGOPTION_ALL;
	long mRefreshFlag = MGOPTION_ALL;
	int retFlag = 0;

	if( remote.is_host )
	{
		colorAssigned[tempConfig.player_nation_color-1] = 1;
		for( p = 0; p < regPlayerCount && regPlayerId[p] != mp_obj.get_my_player_id(); ++p );
		if( p < regPlayerCount )
		{
			playerColor[p] = tempConfig.player_nation_color;
			playerBalance[p] = sys.cdrom_drive ? PLAYER_RATIO_CDROM : PLAYER_RATIO_NOCD;
		}

		// initialize other colorAssigned, only free for those of the remote players
		maxPlayer = 1;			// host
		for( p = 1; p <= nation_array.size(); ++p )
		{
			if( !nation_array.is_deleted(p) && nation_array[p]->is_remote() )
			{
				colorAssigned[nation_array[p]->color_scheme_id-1] = 0;
				maxPlayer++;
			}
		}
		err_when( maxPlayer > MAX_NATION );
	}
	else
	{
		memset( colorAssigned, 0, sizeof(colorAssigned) );		// assume all color are unassigned
		MpStructLoadGameNewPlayer msgNewPlayer( mp_obj.get_my_player_id(), ~nation_array, sys.frame_count,
			m.get_random_seed(), sys.cdrom_drive ? PLAYER_RATIO_CDROM : PLAYER_RATIO_NOCD );
		mp_obj.send_stream(BROADCAST_PID, &msgNewPlayer, sizeof(msgNewPlayer) );
	}

	// --------- initialize race button group ---------- //

	ButtonCustomGroup raceGroup(MAX_RACE);
	for( i = 0; i < MAX_RACE; ++i )
	{
#if(MAX_RACE == 10)
		raceGroup[i].create(220+(i%5)*BASIC_OPTION_X_SPACE, (i/5)*BASIC_OPTION_HEIGHT+offsetY+70,
			220+(i%5+1)*BASIC_OPTION_X_SPACE-1, (i/5+1)*BASIC_OPTION_HEIGHT+offsetY+70-1,
			disp_virtual_button, ButtonCustomPara(&raceGroup, race_table[i]), 0, 0);
		#define Y_SHIFT_FLAG 1
#else
		raceGroup[i].create(118+i*BASIC_OPTION_X_SPACE, offsetY+93,
			118+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+93+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&raceGroup, race_table[i]), 0, 0);
		#define Y_SHIFT_FLAG 0
#endif
	}

	// --------- initialize color button group ---------- //

	ButtonCustomGroup colorGroup(MAX_COLOR_SCHEME);
	for( i = 0; i < MAX_COLOR_SCHEME; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 13
		#else
			#define Y_SHIFT 0
		#endif
		colorGroup[i].create(195+i*COLOR_OPTION_X_SPACE, offsetY+139+Y_SHIFT,
			195+(i+1)*COLOR_OPTION_X_SPACE-1, offsetY+139+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&colorGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	// ---------- initialize ai_nation_count buttons --------//

	ButtonCustom aiNationInc, aiNationDec;
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 13
		#else
			#define Y_SHIFT 0
		#endif
		aiNationInc.create(595, offsetY+139+Y_SHIFT, 
			595+COLOR_OPTION_X_SPACE-1, offsetY+139+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, +1) );
		aiNationDec.create(630, offsetY+139+Y_SHIFT,
			630+COLOR_OPTION_X_SPACE-1, offsetY+139+Y_SHIFT+COLOR_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(NULL, -1) );
		#undef Y_SHIFT
	}

	// ---------- initialize difficulty_level button group -------//

	ButtonCustomGroup diffGroup(6);
	for( i = 0; i < 6; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 16
		#else
			#define Y_SHIFT 0
		#endif
		diffGroup[i].create( 205+i*BASIC_OPTION_X_SPACE, offsetY+184+Y_SHIFT,
			205+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+184+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&diffGroup, i), 0, 0);
		#undef Y_SHIFT
	}

	// --------- initialize terrain_set button group -------//

	// ####### begin Gilbert 25/10 #######//
	ButtonCustomGroup terrainGroup(3);
	for( i = 0; i < 3; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 16
		#else
			#define Y_SHIFT 0
		#endif
#ifdef AMPLUS
		terrainGroup[i].create(166+i*BASIC_OPTION_X_SPACE, offsetY+248+Y_SHIFT, 
			166+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+248+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&terrainGroup, i+1), 0, 0);
#else
		int k = i;
		switch(i)
		{
		case 1: k = 2; break;
		case 2: k = 1; break;
		}
		terrainGroup[i].create(205+k*BASIC_OPTION_X_SPACE, offsetY+248+Y_SHIFT, 
			205+(k+1)*BASIC_OPTION_X_SPACE-1, offsetY+248+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&terrainGroup, i+1), 0, 0);
		if( i == 1 )
			terrainGroup[i].enable_flag = 0;
#endif
		#undef Y_SHIFT
	}
	// ####### end Gilbert 25/10 #######//

	// --------- initialize land_mass button group -------//

	ButtonCustomGroup landGroup(3);
	for( i = 0; i < 3; ++i )
	{
		#if(Y_SHIFT_FLAG)
			#define Y_SHIFT 16
		#else
			#define Y_SHIFT 0
		#endif
		landGroup[i].create(439+i*BASIC_OPTION_X_SPACE, offsetY+248+Y_SHIFT,
			439+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+248+Y_SHIFT+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&landGroup, i+1), 0, 0);
		#undef Y_SHIFT
	}

	GetA messageField;
	messageField.init( 190, 116, 700, typingMsg.content,
		MpStructChatMsg::MSG_LENGTH, &font_san, 0, 1);
	
	// ###### begin Gilbert 25/10 #######//
	// --------- initialize info.random_seed field ----------//

	MpStructSeedStr msgSeedStr(info.random_seed);
	GetA mapIdField;
#if(defined(SPANISH))
	#define MAPID_X1 588
#elif(defined(FRENCH))
	#define MAPID_X1 578
#else
	#define MAPID_X1 564
#endif
	mapIdField.init( MAPID_X1, offsetY+83, 700, msgSeedStr.seed_str, msgSeedStr.RANDOM_SEED_MAX_LEN, &font_san, 0, 1);
#undef MAPID_X1
	// ###### end Gilbert 25/10 #######//

	// --------- initialize explore_whole_map button group -------//

	ButtonCustomGroup exploreGroup(2);
	for( i = 0; i < 2; ++i )
	{
		exploreGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+74, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+74+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&exploreGroup, 1-i), 0, 0);
	}

	// --------- initialize fog_of_war button group -------//

	ButtonCustomGroup fogGroup(2);
	for( i = 0; i < 2; ++i )
	{
		fogGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+106, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+106+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&fogGroup, 1-i), 0, 0);
	}

	// --------- initialize start_up_cash/start_up_food button group -------//

	ButtonCustomGroup treasureGroup(4);
	for( i = 0; i < 4; ++i )
	{
		treasureGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+138,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+138+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&treasureGroup, i+1), 0, 0);
	}

	// --------- initialize ai_start_up_cash/food button group -------//

	ButtonCustomGroup aiTreasureGroup(4);
	for( i = 0; i < 4; ++i )
	{
		aiTreasureGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+170,
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+170+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&aiTreasureGroup, i+1), 0, 0);
	}

	// --------- initialize ai_aggressiveness -------//

	ButtonCustomGroup aiAggressiveGroup(4);
	for( i = 0; i < 4; ++i )
	{
		aiAggressiveGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+202, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+202+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&aiAggressiveGroup, i+1), 0, 0);
	}

	// --------- initialize monster_type -------//

	ButtonCustomGroup monsterGroup(3);
	for( i = 0; i < 3; ++i )
	{
		monsterGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+234, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+234+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&monsterGroup, i), 0, 0);
	}

	// --------- initialize random startup button group -------//

	ButtonCustomGroup randomStartUpGroup(2);
	for( i = 0; i < 2; ++i )
	{
		randomStartUpGroup[i].create(335+i*BASIC_OPTION_X_SPACE, offsetY+266, 
			335+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+266+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&randomStartUpGroup, 1-i), 0, 0);
	}

	//  -------- initialize start_up_raw_site buttons --------- //

	ButtonCustom rawSiteInc, rawSiteDec;
	rawSiteInc.create( 358, offsetY+72, 
		358+COLOR_OPTION_X_SPACE-1, offsetY+72+COLOR_OPTION_HEIGHT-1,
		disp_virtual_button, ButtonCustomPara(NULL,0));
	rawSiteDec.create( 393, offsetY+72, 
		393+COLOR_OPTION_X_SPACE-1, offsetY+72+COLOR_OPTION_HEIGHT-1,
		disp_virtual_button, ButtonCustomPara(NULL,0));

	// --------- initialize start_up_has_mine_nearby button group --------//

	ButtonCustomGroup nearRawGroup(2);
	for( i = 0; i < 2; ++i )
	{
		nearRawGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+104,
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+104+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&nearRawGroup, 1-i), 0, 0);
	}

	// --------- initialize start_up_independent_town button group --------//

	static short startTownArray[3] = { 7, 15, 30 };

	ButtonCustomGroup townStartGroup(3);
	for( i = 0; i < 3; ++i )
	{
		townStartGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+136, 
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+136+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townStartGroup, startTownArray[i]), 0, 0);
	}

	// --------- initialize independent_town_resistance button group --------//

	ButtonCustomGroup townResistGroup(3);
	for( i = 0; i < 3; ++i )
	{
		townResistGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+168, 
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+168+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townResistGroup, i+1), 0, 0);
	}

	// --------- initialize new_independent_town_emerge button group --------//

	ButtonCustomGroup townEmergeGroup(2);
	for( i = 0; i < 2; ++i )
	{
		townEmergeGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+200,
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+200+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&townEmergeGroup, 1-i), 0, 0);
	}

	// --------- initialize new_nation_emerge button group --------//

	ButtonCustomGroup nationEmergeGroup(2);
	for( i = 0; i < 2; ++i )
	{
		nationEmergeGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+232,
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+232+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&nationEmergeGroup, 1-i), 0, 0);
	}

	// --------- initialize random_event_frequency button group --------//

	ButtonCustomGroup randomEventGroup(4);
	for( i = 0; i < 4; ++i )
	{
		randomEventGroup[i].create(322+i*BASIC_OPTION_X_SPACE, offsetY+264, 
			322+(i+1)*BASIC_OPTION_X_SPACE-1, offsetY+264+BASIC_OPTION_HEIGHT-1,
			disp_virtual_button, ButtonCustomPara(&randomEventGroup, i), 0, 0);
	}

	// ---------- initialize goal buttons ----------//

	// ##### begin Gilbert 25/10 #######//
	ButtonCustom clearEnemyButton, clearMonsterButton, enoughPeopleButton, enoughIncomeButton, enoughScoreButton, timeLimitButton;
	ButtonCustom peopleInc, peopleDec, incomeInc, incomeDec, scoreInc, scoreDec, yearInc, yearDec;
	
	clearEnemyButton.create( 209, offsetY+109, 209+19, offsetY+109+19,	// -3
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 1);
	clearEnemyButton.enable_flag = 0;;
	clearMonsterButton.create( 209, offsetY+142, 209+19, offsetY+142+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 
		tempConfig.goal_destroy_monster);
	enoughPeopleButton.create( 209, offsetY+175, 209+19, offsetY+175+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0, 
		tempConfig.goal_population_flag);
	enoughIncomeButton.create( 209, offsetY+208, 209+19, offsetY+208+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_economic_score_flag);
	enoughScoreButton.create( 209, offsetY+241, 209+19, offsetY+241+19,
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_total_score_flag);
	timeLimitButton.create( 209, offsetY+273, 209+19, offsetY+273+19,	// +29
		disp_virtual_tick, ButtonCustomPara(NULL, 0), 0,
		tempConfig.goal_year_limit_flag);

	peopleInc.create( 524, offsetY+170,
		524+COLOR_OPTION_X_SPACE-1, offsetY+170+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	peopleDec.create( 559, offsetY+170, 
		559+COLOR_OPTION_X_SPACE-1, offsetY+170+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	incomeInc.create( 524, offsetY+202,
		524+COLOR_OPTION_X_SPACE-1, offsetY+202+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	incomeDec.create( 559, offsetY+202,
		559+COLOR_OPTION_X_SPACE-1, offsetY+202+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	scoreInc.create( 524, offsetY+234,
		524+COLOR_OPTION_X_SPACE-1, offsetY+234+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	scoreDec.create( 559, offsetY+234,
		559+COLOR_OPTION_X_SPACE-1, offsetY+234+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	yearInc.create( 524, offsetY+266,
		524+COLOR_OPTION_X_SPACE-1, offsetY+266+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	yearDec.create( 559, offsetY+266,
		559+COLOR_OPTION_X_SPACE-1, offsetY+266+COLOR_OPTION_HEIGHT-1, 
		disp_virtual_button, ButtonCustomPara(NULL, 0) );
	// ##### end Gilbert 25/10 #######//

	Button3D startButton, readyButton, returnButton;
	readyButton.create(120, 538, "READY-U", "READY-D", 1, 0);
	startButton.create(320, 538, "START-U", "START-D", 1, 0);
	returnButton.create(520, 538, "CANCEL-U", "CANCEL-D", 1, 0);

	// ###### begin Gilbert 24/10 #######//
	vga_front.unlock_buf();
	// ###### end Gilbert 24/10 #######//

	while(1)
	{
		// ####### begin Gilbert 23/10 #######//
		if( sys.need_redraw_flag )
		{
			refreshFlag = SGOPTION_ALL;
			mRefreshFlag = MGOPTION_ALL;
			sys.need_redraw_flag = 0;
		}

		vga_front.lock_buf();
		// ####### begin Gilbert 24/10 ########//

		sys.yield();
		mouse.get_event();

		// -------- display ----------//
		if( refreshFlag || mRefreshFlag )
		{
			// ------- display basic option ---------//
			if( optionMode == OPTION_BASIC )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-BSC");
#if(MAX_RACE == 10)
					// protection : image_menu.put_to_buf( &vga_back, "MPG-BSC");
					// ensure the user has the release version (I_MENU.RES)
					// image_menu2.put_to_buf( &vga_back, "MPG-BSC") get the real one
					image_menu2.put_to_buf( &vga_back, "MPG-BSC");
#endif
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_RACE )
					raceGroup.paint( reverse_race_table[tempConfig.race_id-1] );
				if( refreshFlag & SGOPTION_COLOR )
					colorGroup.paint( tempConfig.player_nation_color-1 );
				if( refreshFlag & SGOPTION_AI_NATION )
				{
					#if(Y_SHIFT_FLAG)
						#define Y_SHIFT 13
					#else
						#define Y_SHIFT 0
					#endif
					font_san.center_put(564, offsetY+144+Y_SHIFT, 564+25, offsetY+144+Y_SHIFT+21,
						m.format(tempConfig.ai_nation_count), 1);
					aiNationInc.paint();
					aiNationDec.paint();
					#undef Y_SHIFT
				}
				if( refreshFlag & SGOPTION_DIFFICULTY )
					diffGroup.paint(tempConfig.difficulty_level);
				if( refreshFlag & SGOPTION_TERRAIN )
					terrainGroup.paint(tempConfig.terrain_set-1);
				if( refreshFlag & SGOPTION_LAND_MASS )
					landGroup.paint(tempConfig.land_mass-1);
			}

			// ------- display advanced option ---------//
			if( optionMode == OPTION_ADVANCED )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-O1");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				// ###### begin Gilbert 24/10 #######//
				if( refreshFlag & SGOPTION_MAP_ID )
					mapIdField.paint();
				// ###### end Gilbert 24/10 #######//
				if( refreshFlag & SGOPTION_EXPLORED )
					exploreGroup.paint(1-tempConfig.explore_whole_map);
				if( refreshFlag & SGOPTION_FOG )
					fogGroup.paint(1-tempConfig.fog_of_war);
				if( refreshFlag & SGOPTION_TREASURE )
					treasureGroup.paint( tempConfig.start_up_cash-1 );
				if( refreshFlag & SGOPTION_AI_TREASURE )
					aiTreasureGroup.paint( tempConfig.ai_start_up_cash-1 );
				if( refreshFlag & SGOPTION_AI_AGGRESSIVE )
					aiAggressiveGroup.paint(tempConfig.ai_aggressiveness-1);
				if( refreshFlag & SGOPTION_FRYHTANS )
					monsterGroup.paint(tempConfig.monster_type);
				if( refreshFlag & SGOPTION_RANDOM_STARTUP )
					randomStartUpGroup.paint(1-tempConfig.random_start_up);
			}

			// ------- display advanced option ---------//
			if( optionMode == OPTION_ADVANCE2 )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-O2");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_RAW )
				{
					font_san.center_put(327, offsetY+77, 327+25, offsetY+77+21,
						m.format(tempConfig.start_up_raw_site), 1);
					rawSiteInc.paint();
					rawSiteDec.paint();
				}
				if( refreshFlag & SGOPTION_NEAR_RAW )
					nearRawGroup.paint(1-tempConfig.start_up_has_mine_nearby);
				if( refreshFlag & SGOPTION_START_TOWN )
					townStartGroup.paint(
					tempConfig.start_up_independent_town >= 30 ? 2 :
					tempConfig.start_up_independent_town <= 7 ? 0 :
					1
					);
				if( refreshFlag & SGOPTION_TOWN_STRENGTH )
					townResistGroup.paint(tempConfig.independent_town_resistance-1);
				if( refreshFlag & SGOPTION_TOWN_EMERGE )
					townEmergeGroup.paint(1-tempConfig.new_independent_town_emerge);
				if( refreshFlag & SGOPTION_KINGDOM_EMERGE )
					nationEmergeGroup.paint(1-tempConfig.new_nation_emerge);
				if( refreshFlag & SGOPTION_RANDOM_EVENT )
					randomEventGroup.paint(tempConfig.random_event_frequency);
			}

			// ------- display goal option ---------//
			if( optionMode == OPTION_GOAL )
			{
				if( refreshFlag & SGOPTION_PAGE )
				{
					image_menu.put_to_buf( &vga_back, "MPG-GOAL");
					image_menu.put_back( 234, 15, menuTitleBitmap);
					vga_util.blt_buf(0,0,VGA_WIDTH-1,VGA_HEIGHT-1,0);
				}
				if( refreshFlag & SGOPTION_CLEAR_ENEMY )
					clearEnemyButton.paint();
				if( refreshFlag & SGOPTION_CLEAR_MONSTER )
					clearMonsterButton.paint(tempConfig.goal_destroy_monster);
				// ####### begin Gilbert 25/10 ########//
				if( refreshFlag & SGOPTION_ENOUGH_PEOPLE )
				{
					enoughPeopleButton.paint(tempConfig.goal_population_flag);
					font_san.center_put( 446, offsetY+176, 446+67, offsetY+176+21,
						m.format(tempConfig.goal_population), 1);
					peopleInc.paint();
					peopleDec.paint();
				}
				if( refreshFlag & SGOPTION_ENOUGH_INCOME )
				{
					enoughIncomeButton.paint(tempConfig.goal_economic_score_flag);
					font_san.center_put( 446, offsetY+207, 446+67, offsetY+207+21,
						m.format(tempConfig.goal_economic_score), 1);
					incomeInc.paint();
					incomeDec.paint();
				}
				if( refreshFlag & SGOPTION_ENOUGH_SCORE )
				{
					enoughScoreButton.paint(tempConfig.goal_total_score_flag);
					font_san.center_put( 446, offsetY+239, 446+67, offsetY+239+21,
						m.format(tempConfig.goal_total_score), 1);
					scoreInc.paint();
					scoreDec.paint();
				}
				if( refreshFlag & SGOPTION_TIME_LIMIT )
				{
					timeLimitButton.paint(tempConfig.goal_year_limit_flag);
					font_san.center_put( 446, offsetY+271, 446+33, offsetY+271+21,
						m.format(tempConfig.goal_year_limit), 1);
					yearInc.paint();
					yearDec.paint();
				}
				// ####### end Gilbert 25/10 ########//
			}

			// -------- refresh players in the session --------//
			if( mRefreshFlag & MGOPTION_PLAYERS )
			{
				vga_util.blt_buf( 96, 46, 702, 100, 0 );
				for( p = 0; p < regPlayerCount; ++p)
				{
					if( playerReadyFlag[p] )
					{
						image_menu.put_front( tickX[p]+3, tickY[p]+3, "NMPG-RCH" );
					}
					PlayerDesc *dispPlayer = mp_obj.search_player(regPlayerId[p]);
					font_san.put( tickX[p]+nameOffsetX, tickY[p]+nameOffsetY, dispPlayer?dispPlayer->friendly_name_str():(char*)"?anonymous?" );
				}
			}

			// ------------ display chat message --------//
			if( mRefreshFlag & MGOPTION_OUT_MESSAGE )
			{
				messageField.paint();
			}

			// ------------- display incoming chat message --------//
			if( mRefreshFlag & MGOPTION_IN_MESSAGE )
			{
				vga_util.blt_buf( 101, 135, 700, 202, 0 );
				for( p = 1; p <= 4 && p <= messageList.size() ; ++p)
				{
					int ny = 136+(p-1)*16;
					int nx = font_san.put( 102, ny, ((MpStructChatMsg *)messageList.get(p))->sender );
					nx = font_san.put( nx, ny, " : ");
					nx = font_san.put( nx, ny, ((MpStructChatMsg *)messageList.get(p))->content, 0, 700);
				}
			}

			// ------- display difficulty -------//
			if( (refreshFlag & SGOPTION_DIFFICULTY) || (mRefreshFlag & MGOPTION_PLAYERS) )
			{
				font_san.center_put( 718, offsetY+84, 774, offsetY+108, 
					m.format(tempConfig.difficulty_rating), 1 );
			}

			// -------- repaint button -------//
			if( refreshFlag & SGOPTION_PAGE )
			{
				if( remote.is_host )
					startButton.paint();
				readyButton.paint();
				returnButton.paint();
			}

			refreshFlag = 0;
			mRefreshFlag = 0;
		}
		sys.blt_virtual_buf();

		if( config.music_flag )
		{
			if( !music.is_playing() )
				music.play(1, sys.cdrom_drive ? MUSIC_CD_THEN_WAV : 0 );
		}
		else
			music.stop();

		// --------- detect remote message -------//
		recvPtr = mp_obj.receive(&from, &to, &recvLen, &sysMsgCount);

		if( sysMsgCount )
		{
			// see any player disconnect
			for(int q = 0; q < regPlayerCount; ++q)
			{
				if( !mp_obj.is_player_connecting(regPlayerId[q]) )
				{
					mRefreshFlag |= MGOPTION_PLAYERS;

					memmove( regPlayerId+q, regPlayerId+q+1, (MAX_NATION-1-q)*sizeof(regPlayerId[0]) );
					regPlayerId[MAX_NATION-1] = 0;
					memmove( playerReadyFlag+q, playerReadyFlag+q+1, (MAX_NATION-1-q)*sizeof(playerReadyFlag[0]) );
					playerReadyFlag[MAX_NATION-1] = 0;
					short freeColor = playerColor[q];
					memmove( playerColor+q, playerColor+q+1, (MAX_NATION-1-q)*sizeof(playerColor[0]) );
					playerColor[MAX_NATION-1] = 0;
					if(freeColor > 0 && freeColor <= MAX_COLOR_SCHEME)
						colorAssigned[freeColor-1] = 0;
					short freeRace = playerRace[q];
					memmove( playerRace+q, playerRace+q+1, (MAX_NATION-1-q)*sizeof(playerRace[0]) );
					playerRace[MAX_NATION-1] = 0;
					if(freeRace > 0 && freeRace <= MAX_RACE)
						raceAssigned[freeRace-1]--;
					memmove( playerBalance+q, playerBalance+q+1, (MAX_NATION-1-q)*sizeof(playerBalance[0]) );
					playerBalance[MAX_NATION-1] = 0;
					--regPlayerCount;
					--q;
				}
			}
		}

		if( recvPtr )
		{
			if( ((MpStructBase *)recvPtr)->msg_id == MPMSG_START_GAME )
			{
				retFlag = 1;
				break;		// break while(1) loop
			}
			else
			{
				switch( ((MpStructBase *)recvPtr)->msg_id )
				{
				case MPMSG_ABORT_GAME:
					return 0;
				case MPMSG_SEND_CONFIG:
					tempConfig.change_game_setting( ((MpStructConfig *)recvPtr)->game_config );
					refreshFlag |= SGOPTION_ALL_OPTIONS;
					break;
				case MPMSG_RANDOM_SEED:
					info.init_random_seed( ((MpStructSeed *)recvPtr)->seed );
					break;
					// ####### begin Gilbert 25/10 #######//
				case MPMSG_RANDOM_SEED_STR:
					msgSeedStr = *(MpStructSeedStr *)recvPtr;
					mapIdField.select_whole();
					refreshFlag |= SGOPTION_MAP_ID;
					break;
					// ####### end Gilbert 25/10 #######//
				case MPMSG_NEW_PLAYER:
					{
						// incorrect message, reject
						MpStructRefuseNewPlayer msgRefuse(from);
						mp_obj.send_stream(BROADCAST_PID, &msgRefuse, sizeof(msgRefuse) );
					}
					break;
				case MPMSG_LOAD_GAME_NEW_PLAYER:
					mp_obj.poll_players();
					if( remote.is_host )
					{
						MpStructLoadGameNewPlayer *newPlayerMsg = (MpStructLoadGameNewPlayer *)recvPtr;
						if( regPlayerCount < MAX_NATION 
							&& !nation_array.is_deleted(newPlayerMsg->nation_recno)
							&& nation_array[newPlayerMsg->nation_recno]->color_scheme_id == newPlayerMsg->color_scheme_id
							&& nation_array[newPlayerMsg->nation_recno]->race_id == newPlayerMsg->race_id
							&& !colorAssigned[newPlayerMsg->color_scheme_id-1] 
							&& newPlayerMsg->frame_count == sys.frame_count
							&& newPlayerMsg->random_seed == m.get_random_seed() )
						{
							regPlayerId[regPlayerCount] = newPlayerMsg->player_id;
							playerReadyFlag[regPlayerCount] = 0;
							raceAssigned[newPlayerMsg->race_id]++;
							playerRace[regPlayerCount] = newPlayerMsg->race_id;
							colorAssigned[newPlayerMsg->color_scheme_id-1]=1;
							playerColor[regPlayerCount] = newPlayerMsg->color_scheme_id;

							// send accept new player to all player
							MpStructAcceptNewPlayer msgAccept(from);
							mp_obj.send_stream( BROADCAST_PID, &msgAccept, sizeof(msgAccept) );

							// send ready flag
							for( int c = 0; c < regPlayerCount; ++c)
							{
								if( playerReadyFlag[c] )
								{
									MpStructPlayerReady msgReady(regPlayerId[c]);
									mp_obj.send_stream(from, &msgReady, sizeof(msgReady));
								}
							}

							// ###### patch begin Gilbert 22/1 ######//
							// send remote.sync_test_level
							MpStructSyncLevel msgSyncTest(remote.sync_test_level);
							mp_obj.send_stream( from, &msgSyncTest, sizeof(msgSyncTest) );
							// ###### patch end Gilbert 22/1 ######//

							// update balance
							playerBalance[regPlayerCount] = newPlayerMsg->player_balance;

							regPlayerCount++;
							mRefreshFlag |= MGOPTION_PLAYERS;
						}
						else
						{
							// reply refuse new player
							MpStructRefuseNewPlayer msgRefuse(from);
							mp_obj.send_stream( BROADCAST_PID, &msgRefuse, sizeof(msgRefuse) );
						}
					}
					break;
				case MPMSG_ACCEPT_NEW_PLAYER:
					hostPlayerId = from;
					if( regPlayerCount < MAX_NATION && ((MpStructAcceptNewPlayer *)recvPtr)->player_id != mp_obj.get_my_player_id() )
					{
						// search if this player has existed
						for( p=0; p < regPlayerCount && regPlayerId[p] != ((MpStructAcceptNewPlayer *)recvPtr)->player_id; ++p );
						regPlayerId[p] = ((MpStructAcceptNewPlayer *)recvPtr)->player_id;
						playerReadyFlag[p] = 0;
						if( p >= regPlayerCount )
						{
							regPlayerCount++;		// now regPlayerCount == p
							err_when( p != regPlayerCount );
						}
						mRefreshFlag |= MGOPTION_PLAYERS;
					}
					break;
				case MPMSG_PLAYER_READY:
					{
						for( int p = 0; p < regPlayerCount; ++p)
						{
							if( regPlayerId[p] == ((MpStructPlayerReady *)recvPtr)->player_id )
							{
								playerReadyFlag[p] = 1;
								mRefreshFlag |= MGOPTION_PLAYERS;
							}
						}
					}
					break;
				case MPMSG_PLAYER_UNREADY:
					{
						for( int p = 0; p < regPlayerCount; ++p)
						{
							if( regPlayerId[p] == ((MpStructPlayerUnready *)recvPtr)->player_id )
							{
								playerReadyFlag[p] = 0;
								mRefreshFlag |= MGOPTION_PLAYERS;
							}
						}
					}
					break;
				case MPMSG_SEND_CHAT_MSG:
					while( messageList.size() >= 4 )
						messageList.linkout(1);
					messageList.linkin(recvPtr);
					mRefreshFlag |= MGOPTION_IN_MESSAGE;
					break;
				// ###### patch begin Gilbert 22/1 ######//
				case MPMSG_SEND_SYNC_TEST_LEVEL:
					remote.sync_test_level = ((MpStructSyncLevel *)recvPtr)->sync_test_level;
					break;
				// ###### patch end Gilbert 22/1 ######//
				case MPMSG_REFUSE_NEW_PLAYER:
					if( ((MpStructRefuseNewPlayer *)recvPtr)->player_id == mp_obj.get_my_player_id() )
					{
						box.msg("You cannot join the game because the multiplayer saved game you selected is different from those of other human players.");
						return 0;
					}
					break;
				default:		// if the game is started, any other thing is received
					return 0;
				}
			}
		}

		// --------- detect switch option button ------//

		if( mouse.single_click(96, offsetY+12, 218, offsetY+54) )
		{
			if( optionMode != OPTION_BASIC )
			{
				optionMode = OPTION_BASIC;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}
		else if( mouse.single_click(236, offsetY+12, 363, offsetY+54) )
		{
			if( optionMode != OPTION_ADVANCED )
			{
				optionMode = OPTION_ADVANCED;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}
		else if( mouse.single_click(380, offsetY+12, 506, offsetY+54) )
		{
			if( optionMode != OPTION_ADVANCE2 )
			{
				optionMode = OPTION_ADVANCE2;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}
		else if( mouse.single_click(523, offsetY+12, 649, offsetY+54) )
		{
			if( optionMode != OPTION_GOAL )
			{
				optionMode = OPTION_GOAL;
				refreshFlag = SGOPTION_ALL;
				mRefreshFlag = MGOPTION_ALL;
			}
		}

		// --------- detect ready button button --------//

		unsigned keyCode;

		if( readyButton.detect() )
		{
			mRefreshFlag |= MGOPTION_PLAYERS;
			for(p = 0; p < regPlayerCount && regPlayerId[p] != mp_obj.get_my_player_id(); ++p);
			if( p < regPlayerCount )
			{
				if( !selfReadyFlag ) 
				{
					playerReadyFlag[p] = selfReadyFlag = 1;
					MpStructPlayerReady msgReady(mp_obj.get_my_player_id());
					mp_obj.send_stream(BROADCAST_PID, &msgReady, sizeof(msgReady));
				}
				else
				{
					// else un-ready this player
					playerReadyFlag[p] = selfReadyFlag = 0;
					MpStructPlayerUnready msgUnready(mp_obj.get_my_player_id());
					mp_obj.send_stream(BROADCAST_PID, &msgUnready, sizeof(msgUnready));
				}
			}
		}
		if( remote.is_host && startButton.detect() )
		{

			// see if all player is ready
			short sumBalance = 0;
			int q;
			for( q = 0; q < regPlayerCount && playerReadyFlag[q]; ++q)
			{
				err_when( playerBalance[q] == 0 );
				sumBalance += playerBalance[q];
			}
			if( q >= regPlayerCount )		// not all playerReadyFlag[p] = 1;
			{
#ifdef DEBUG
				sumBalance = 0;
#endif
#ifdef BETA
				sumBalance = 0;
#endif
				if( regPlayerCount != maxPlayer )
				{
					String str;

					str  = "This multiplayer saved game needs ";
					str += maxPlayer;
					str += " human players while now there ";

					if( regPlayerCount > 1 )
						str += " are";
					else
						str += " is";

					str += " only ";
					str += regPlayerCount;

					if( regPlayerCount > 1 )
						str += " human players.";
					else
						str += " human player.";

					str += " The game cannot start.";

					box.msg(str);
				}
				else if( sumBalance < 0 )
				{
					// too many non-CD version
					box.msg("There are not enough Seven Kingdoms CDROMs to start the game. Each CDROM supports "PLAYER_RATIO_STRING" players.");
				}
				else
				{
//					MpStructBase msgStart(MPMSG_START_GAME);
//					mp_obj.send_stream(BROADCAST_PID, &msgStart, sizeof(msgStart));
					retFlag = 1;
					break;							// break while(1)
				}
			}
		}
		else if( returnButton.detect() )
		{
			if( remote.is_host )
			{
				MpStructBase msgAbort(MPMSG_ABORT_GAME);
				mp_obj.send_stream(BROADCAST_PID, &msgAbort, sizeof(msgAbort) );
			}
			retFlag = 0;
			break;			// break while(1)
		}
		else if( (keyCode = messageField.detect()) != 0 )
		{
			mRefreshFlag |= MGOPTION_OUT_MESSAGE;
			if(keyCode == KEY_RETURN && strlen(typingMsg.content) > 0)
			{
				// send message
				mp_obj.send_stream(BROADCAST_PID, &typingMsg, sizeof(typingMsg) );

				// clear the string
				messageField.clear();
			}
			else if( keyCode == KEY_ESC )
			{
				messageField.clear();
			}
		}

		// ####### begin Gilbert 24/10 #######//
		vga_front.unlock_buf();
		// ####### end Gilbert 24/10 #######//
	}

	// ###### begin Gilbert 24/10 #######//
	if( !vga_front.buf_locked )
		vga_front.lock_buf();
	// ###### end Gilbert 24/10 #######//

	// ---------- final setup to start multiplayer game --------//

	if( retFlag )
	{
		retFlag = 0;

		if( remote.is_host )
			mp_obj.disable_join_session();

		// mp_obj.poll_players();
			
		int trial;
		unsigned long startTime;
		int playerCount = 0;

		if( &config != &tempConfig )
			config = tempConfig;

		if( remote.is_host )
		{
			// setup nation now
			VLenQueue setupString;

			// ------- put start game string -------//

			{
				MpStructBase msgStart(MPMSG_START_GAME);
				memcpy( setupString.reserve(sizeof(msgStart)), &msgStart, sizeof(msgStart) );
			}

			// ------- put nations -------//

			playerCount = 0;
			for( p = 0; p < regPlayerCount; ++p )
			{
				PID_TYPE playerId = regPlayerId[p];
				PlayerDesc *player = mp_obj.search_player(playerId);
				if( !playerId || !player || !player->connecting )
					continue;

				// match nation color
				for( short nationRecno = 1; nationRecno <= nation_array.size(); ++nationRecno)
				{
					Nation *nationPtr;
					if( !nation_array.is_deleted(nationRecno) && (nationPtr = nation_array[nationRecno]) 
						&& (nationPtr->is_own() || nationPtr->is_remote())
						&& playerColor[p] == nation_array[nationRecno]->color_scheme_id )
					{
						nationPtr->player_id = playerId;
						nationPtr->next_frame_ready = 0;
						((MpStructNation *)setupString.reserve(sizeof(MpStructNation)))->init(
							nationRecno, nationPtr->player_id,
							nationPtr->color_scheme_id, nationPtr->race_id, player->friendly_name_str());
						playerCount++;
						break;
					}
				}
			}
			/*
			for( int d = 1; playerCount<MAX_NATION && mp_obj.get_player(d); ++d)
			{
				// ensure it is a valid player
				PlayerDesc *player = mp_obj.get_player(d);
				PID_TYPE playerId = player ? player->pid() : 0;
				if( !playerId || !player->connecting )
					continue;
				for( p = 0; p < regPlayerCount && regPlayerId[p] != playerId; ++p);
				if( p >= regPlayerCount )		// not found
					continue;

				// match nation color
				for( short nationRecno = 1; nationRecno <= nation_array.size(); ++nationRecno)
				{
					Nation *nationPtr;
					if( !nation_array.is_deleted(nationRecno) && (nationPtr = nation_array[nationRecno]) 
						&& (nationPtr->is_own() || nationPtr->is_remote())
						&& playerColor[p] == nation_array[nationRecno]->color_scheme_id )
					{
						nationPtr->player_id = playerId;
						nationPtr->next_frame_ready = 0;
						((MpStructNation *)setupString.reserve(sizeof(MpStructNation)))->init(
							nationRecno, nationPtr->player_id,
							nationPtr->color_scheme_id, nationPtr->race_id, player->friendly_name_str());
						playerCount++;
						break;
					}
				}
			}
			*/

			//--- if the current number of players < original number of players ---//

			if( playerCount < maxPlayer )
			{
				String str;

				str  = "This multiplayer saved game needs ";
				str += maxPlayer;
				str += " human players while now there ";

				if( playerCount > 1 )
					str += " are";
				else
					str += " is";

				str += " only ";
				str += playerCount;

				if( playerCount > 1 )
					str += " human players.";
				else
					str += " human player.";

				str += " The game cannot start.";

				box.msg(str);
				return 0;
			}

			//--- if the current number of players > original number of players ---//

			if( playerCount > maxPlayer )
			{
				String str;

				str  = "This multiplayer saved game can only support ";
				str += maxPlayer;
				str += " human players while now there are ";
				str += playerCount;
				str += " human players. The game cannot start.";

				box.msg(str);
				return 0;
			}

			// ---- force set to the lowest frame delay -------//

#ifdef AMPLUS
			remote.set_process_frame_delay(FORCE_MAX_FRAME_DELAY);
			{
				MpStructProcessFrameDelay msgFrameDelay(remote.get_process_frame_delay());
				memcpy( setupString.reserve(sizeof(msgFrameDelay)), &msgFrameDelay, sizeof(msgFrameDelay));
			}
#endif

			// -------- send sync test level ----------//

			{
				MpStructSyncLevel msgSyncTest(remote.sync_test_level);
				memcpy( setupString.reserve(sizeof(msgSyncTest)), &msgSyncTest, sizeof(msgSyncTest));
			}

			mp_obj.send_stream( BROADCAST_PID, setupString.queue_buf, setupString.length() );
		}
		else
		{
			// use the message recving MPMSG_START_GAME

			err_when( !recvPtr );

			DWORD offset = 0;
			int recvStartMsg = 0;
			int ownPlayerFound = 0;
			playerCount = 0;
			char *oriRecvPtr = recvPtr;
			int recvSetFrameDelay = 0;
			int recvSyncTestLevel = 0;

			// process the string received
			while( offset < recvLen )
			{
				DWORD oldOffset = offset;
				recvPtr = oriRecvPtr + offset;

				switch( ((MpStructBase *)(recvPtr))->msg_id )
				{
				case MPMSG_START_GAME:
					recvStartMsg = 1;
					offset += sizeof( MpStructBase );
					break;

				case MPMSG_DECLARE_NATION:
					{
						MpStructNation *msgNation = (MpStructNation *)recvPtr;
						short nationRecno = msgNation->nation_recno;
						Nation *nationPtr;
						if( nation_array.is_deleted(nationRecno) ||
							!(nationPtr = nation_array[nationRecno]) ||
							!nationPtr->is_own() && !nationPtr->is_remote() ||
							nationPtr->color_scheme_id != msgNation->color_scheme )
						{
							box.msg( "Incorrect kingdom information received from the host." );
							return 0;
						}
						nationPtr->player_id = msgNation->dp_player_id;
						if( nationPtr->is_own() && msgNation->dp_player_id == mp_obj.get_my_player_id())
						{
							ownPlayerFound = 1;
						}
						nationPtr->next_frame_ready = 0;
						nation_array.set_human_name(nationRecno, msgNation->player_name);
						++playerCount;
						offset += sizeof( MpStructNation );
					}
					break;

				case MPMSG_SET_PROCESS_FRAME_DELAY:
					remote.set_process_frame_delay(((MpStructProcessFrameDelay *)recvPtr)->common_process_frame_delay);
					++recvSetFrameDelay;
					offset += sizeof( MpStructProcessFrameDelay );
					break;

				case MPMSG_SEND_SYNC_TEST_LEVEL:
					remote.sync_test_level = ((MpStructSyncLevel *)recvPtr)->sync_test_level;
					++recvSyncTestLevel;
					offset += sizeof( MpStructSyncLevel );
					break;
				}  // end switch

				if( !recvStartMsg || offset <= oldOffset )
				{
					err_here();
					box.msg( "Connection string from host is corrupted" );
					return 0;
				}
			} // end while

			if( playerCount == 0 )
			{
				box.msg( "Cannot get kingdom information from the host." );
				return 0;
			}
			err_when( playerCount > MAX_NATION );
			if( !ownPlayerFound )
			{
				box.msg( "The host cannot recognize your machine." );
				return 0;
			}
			err_when( ownPlayerFound > 1 );
			if( !recvSetFrameDelay || !recvSyncTestLevel )
			{
				box.msg("Cannot receive important information from the host");
				return 0;
			}
			err_when( recvSetFrameDelay > 1 );
			err_when( recvSyncTestLevel > 1 );

			// ------- send end setting string ------- //

			MpStructBase mpEndSetting(MPMSG_END_SETTING);
			mp_obj.send_stream( from, &mpEndSetting, sizeof(mpEndSetting) );
		}	

		if( remote.sync_test_level == 0)
		{
			remote.set_alternating_send(playerCount > 4 ? 4 : playerCount);		// automatic setting
		}

		{
			// ------- broadcast end setting string ------- //

			MpStructBase mpEndSetting(MPMSG_END_SETTING);
			mp_obj.send_stream( BROADCAST_PID, &mpEndSetting, sizeof(mpEndSetting) );

			// ------ wait for MPMSG_END_SETTING ----------//
			// ---- to filter other all message until MP_MSG_END_SETTING ---//

			trial = 5000;
			startTime = m.get_time();
			int recvEndSetting = 0;
			while( --trial > 0 || m.get_time() - startTime < 10000 )
			{
				if( recvEndSetting >= playerCount-1)
					break;
				recvPtr = mp_obj.receive( &from, &to, &recvLen);
				if( recvPtr )
				{
					trial = MAX(trial, 1000);
					startTime = m.get_time();
					if( ((MpStructBase *)recvPtr)->msg_id == MPMSG_END_SETTING )
					{
						recvEndSetting++;
					}
				}
			}
			if( recvEndSetting < playerCount-1 )
			{
				box.msg("Some player(s) encountered errors when initializing the game.");
				// but continue
			}
		}

		retFlag = 1;
	}

	return retFlag;
}
#ifdef Y_SHIFT
	#error
#endif
#undef Y_SHIFT_FLAG
// --------- End of function Game::mp_select_load_option --------- //



static void disp_virtual_button(ButtonCustom *button, int)
{
	mouse.hide_area(button->x1, button->y1, button->x2, button->y2);
	if( !button->pushed_flag )
	{
		// copy from back buffer to front buffer
		IMGcopy(vga_front.buf_ptr(), vga_front.buf_pitch(),
			vga_back.buf_ptr(), vga_back.buf_pitch(),
			button->x1, button->y1, button->x2, button->y2 );
	}
	else
	{
		// copy from back buffer to front buffer, but the area is
		// darkened by 2 scale
		IMGcopyRemap(vga_front.buf_ptr(), vga_front.buf_pitch(),
			vga_back.buf_ptr(), vga_back.buf_pitch(),
			button->x1, button->y1, button->x2, button->y2,
			vga.vga_color_table->get_table(-2) );

		// draw black frame
		if( button->x2-button->x1+1 == BASIC_OPTION_X_SPACE &&
			button->y2-button->y1+1 == BASIC_OPTION_HEIGHT )
		{
			image_interface.put_front(button->x1, button->y1, "BAS_DOWN");
		}
		else if( button->x2-button->x1+1 == COLOR_OPTION_X_SPACE &&
			button->y2-button->y1+1 == COLOR_OPTION_HEIGHT )
		{
			image_interface.put_front(button->x1, button->y1, "COL_DOWN");
		}
		else if( button->x2-button->x1+1 == SERVICE_OPTION_X_SPACE &&
			button->y2-button->y1+1 == SERVICE_OPTION_HEIGHT )
		{
			image_menu.put_front(button->x1, button->y1, "NMPG-1BD");
		}
	}
	mouse.show_area();
}


static void disp_virtual_tick(ButtonCustom *button, int )
{
	mouse.hide_area(button->x1, button->y1, button->x2, button->y2);

	// copy from back buffer to front buffer
	IMGcopy(vga_front.buf_ptr(), vga_front.buf_pitch(),
		vga_back.buf_ptr(), vga_back.buf_pitch(),
		button->x1, button->y1, button->x2, button->y2 );

	if( button->pushed_flag )
		image_menu.put_front( button->x1+3, button->y1+3, "NMPG-RCH" );

	mouse.show_area();
}


static void disp_scroll_bar_func(SlideVBar *scroll, int)
{
	short rectTop = scroll->rect_top();
	short rectBottom = scroll->rect_bottom();
	vga_util.blt_buf(scroll->scrn_x1, scroll->scrn_y1, scroll->scrn_x2, scroll->scrn_y2, 0);
	vga_front.bar( scroll->scrn_x1, rectTop, scroll->scrn_x2, rectBottom, VGA_YELLOW+1);
	if( rectBottom - rectTop > 6 )
	{
		vga_front.d3_panel_up(scroll->scrn_x1, rectTop, scroll->scrn_x2, rectBottom,2,0);
	}
}

