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

//Filename    : OREMOTE.H
//Description : Header file of object Remote

#ifndef __OREMOTE_H
#define __OREMOTE_H

#include <MPTYPES.h>
#include <OREMOTEQ.h>
#include <ReplayFile.h>

//---------- Define message id. ---------//

enum { FIRST_REMOTE_MSG_ID = 0x25D3 };

enum { MSG_QUEUE_HEADER=FIRST_REMOTE_MSG_ID,
		 MSG_QUEUE_TRAILER,
		 MSG_NEW_NATION,
		 MSG_UPDATE_GAME_SETTING,
		 MSG_START_GAME,
		 MSG_NEXT_FRAME,
		 MSG_REQUEST_RESEND,
		 MSG_TELL_SEND_TIME,
		 MSG_SET_SPEED,
		 MSG_TELL_RANDOM_SEED,
		 MSG_REQUEST_SAVE,
		 MSG_PLAYER_QUIT,

		 MSG_UNIT_STOP,
		 MSG_UNIT_MOVE,
		 MSG_UNIT_SET_FORCE_MOVE,
		 MSG_UNIT_ATTACK,
		 MSG_UNIT_ASSIGN,
		 MSG_UNIT_CHANGE_NATION,
		 MSG_UNIT_BUILD_FIRM,
		 MSG_UNIT_BURN,
		 MSG_UNITS_SETTLE,
		 MSG_UNIT_SET_GUARD,
		 MSG_UNIT_SET_RANK,
		 MSG_UNIT_DISMOUNT,
		 MSG_UNIT_REWARD,
		 MSG_UNITS_TRANSFORM,
		 MSG_UNIT_RESIGN,
		 MSG_UNITS_ASSIGN_TO_SHIP,
		 MSG_UNITS_SHIP_TO_BEACH,
		 MSG_UNIT_SUCCEED_KING,
		 MSG_UNITS_RETURN_CAMP,
		 MSG_U_CARA_CHANGE_GOODS,
		 MSG_U_CARA_SET_STOP,
		 MSG_U_CARA_DEL_STOP,
		 MSG_U_CARA_SELECTED,
		 MSG_U_SHIP_UNLOAD_UNIT,
		 MSG_U_SHIP_UNLOAD_ALL_UNITS,
		 MSG_U_SHIP_CHANGE_GOODS,
		 MSG_U_SHIP_SET_STOP,
		 MSG_U_SHIP_DEL_STOP,
		 MSG_U_SHIP_CHANGE_MODE,
		 MSG_U_SHIP_SELECTED,
		 MSG_U_GOD_CAST,
		 MSG_UNIT_SPY_NATION,
		 MSG_UNIT_SPY_NOTIFY_CLOAKED_NATION,
		 MSG_UNIT_CHANGE_AGGRESSIVE_MODE,
		 MSG_SPY_CHANGE_NOTIFY_FLAG,

		 //##### trevor 15/10 ######//
		 MSG_SPY_ASSASSINATE,
		 //##### trevor 15/10 ######//

		 //### begin alex 14/10 ###//
		 MSG_UNIT_ADD_WAY_POINT,
		 //#### end alex 14/10 ####//

		 MSG_FIRM_SELL,
		 MSG_FIRM_CANCEL,
		 MSG_FIRM_DESTRUCT,
		 MSG_FIRM_SET_REPAIR,
		 MSG_FIRM_TRAIN_LEVEL,
		 MSG_FIRM_MOBL_WORKER,
		 MSG_FIRM_MOBL_ALL_WORKERS,
		 MSG_FIRM_MOBL_OVERSEER,
		 MSG_FIRM_MOBL_BUILDER,
		 MSG_FIRM_TOGGLE_LINK_FIRM,
		 MSG_FIRM_TOGGLE_LINK_TOWN,
		 MSG_FIRM_PULL_TOWN_PEOPLE,
		 MSG_FIRM_SET_WORKER_HOME,
		 MSG_FIRM_BRIBE,
		 MSG_FIRM_CAPTURE,

		//### trevor 2/10 ###//
		 MSG_FIRM_REWARD,
		 MSG_F_CAMP_PATROL,
		 MSG_F_CAMP_TOGGLE_PATROL,
		 MSG_F_INN_HIRE,
		//### trevor 2/10 ###//
		 MSG_F_MARKET_SCRAP,
		 MSG_F_MARKET_HIRE_CARA,
		 MSG_F_RESEARCH_START,
		 MSG_F_WAR_BUILD_WEAPON,
		 MSG_F_WAR_CANCEL_WEAPON,
		 MSG_F_WAR_SKIP_WEAPON,
		 MSG_F_HARBOR_BUILD_SHIP,
		 MSG_F_HARBOR_SAIL_SHIP,
		 MSG_F_HARBOR_SKIP_SHIP,
		 MSG_F_FACTORY_CHG_PROD,
		 MSG_F_BASE_MOBL_PRAYER,
		 MSG_F_BASE_INVOKE_GOD,

		 MSG_TOWN_RECRUIT,
		 MSG_TOWN_SKIP_RECRUIT,
		 MSG_TOWN_MIGRATE,
		 MSG_TOWN_COLLECT_TAX,
		 MSG_TOWN_REWARD,
		 MSG_TOWN_TOGGLE_LINK_FIRM,
		 MSG_TOWN_TOGGLE_LINK_TOWN,
		 MSG_TOWN_AUTO_TAX,
		 MSG_TOWN_AUTO_GRANT,
		 MSG_TOWN_GRANT_INDEPENDENT,

		 MSG_WALL_BUILD,
		 MSG_WALL_DESTRUCT,

		 MSG_SPY_CYCLE_ACTION,
		 MSG_SPY_LEAVE_TOWN,
		 MSG_SPY_LEAVE_FIRM,
		 MSG_SPY_CAPTURE_FIRM,
		 MSG_SPY_DROP_IDENTITY,
		 MSG_SPY_REWARD,
		 MSG_SPY_EXPOSED,

		 MSG_SEND_TALK_MSG,			// for diplomacy
		 MSG_REPLY_TALK_MSG,
		 MSG_NATION_CONTACT,
		 MSG_NATION_SET_SHOULD_ATTACK,
		 MSG_CHAT,

		 MSG_COMPARE_NATION,
		 MSG_COMPARE_UNIT,
		 MSG_COMPARE_FIRM,
		 MSG_COMPARE_TOWN,
		 MSG_COMPARE_BULLET,
		 MSG_COMPARE_REBEL,
		 MSG_COMPARE_SPY,
		 MSG_COMPARE_TALK,

		//### jesse 2021 ###//
		 MSG_U_CARA_COPY_ROUTE,
		 MSG_COMPARE_CRC,
		 MSG_U_SHIP_COPY_ROUTE,
		 MSG_FIRM_REQ_BUILDER,
		 MSG_F_MARKET_RESTOCK,

		 LAST_REMOTE_MSG_ID			// keep this item last
	  };

enum { REMOTE_MSG_TYPE_COUNT= LAST_REMOTE_MSG_ID - FIRST_REMOTE_MSG_ID };



//--------- Define struct RemoteMsg ---------//

struct RemoteMsg
{
public:
	uint32_t	id;
	char  data_buf[1];

public:
	void	process_msg();

	//------ remote message processing functions ------//

	void	queue_header();
	void	queue_trailer();
	void 	new_nation();
	void	update_game_setting();
	void	start_game();
	void	next_frame();
	void  request_resend();
	void	tell_send_time();
	void  set_speed();
	void	tell_random_seed();
	void	request_save_game();
	void	player_quit();

	void	unit_stop();
	void	unit_move();
	void	unit_set_force_move();
	void	unit_attack();
	void	unit_assign();
	void  unit_change_nation();

	void	unit_build_firm();
	void	unit_burn();
	void	units_settle();
	void	unit_set_guard();
	void	unit_set_rank();
	void	unit_dismount();
	void	unit_reward();
	void	units_transform();
	void	unit_resign();
	void	units_assign_to_ship();
	void	units_ship_to_beach();
	void	unit_succeed_king();
	void	units_return_camp();
	void	caravan_change_goods();
	void	caravan_set_stop();
	void	caravan_del_stop();
	void	caravan_selected();
	void	ship_unload_unit();
	void	ship_unload_all_units();
	void	ship_change_goods();
	void	ship_set_stop();
	void	ship_del_stop();
	void	ship_change_mode();
	void	ship_selected();
	void	ship_copy_route();
	void	god_cast();
	void	change_spy_nation();
	void	notify_cloaked_nation();
	void  unit_change_aggressive_mode();
	void  spy_change_notify_flag();

	//##### trevor 15/10 #####//
	void  spy_assassinate();
	//##### trevor 15/10 #####//

	//### begin alex 14/10 ###//
	void	unit_add_way_point();
	//#### end alex 14/10 ####//

	void	firm_sell();
	void	firm_cancel();				// cancel construction
	void	firm_destruct();
	void	firm_set_repair();
	void	firm_train_level();
	void	mobilize_worker();
	void	mobilize_all_workers();
	void	mobilize_overseer();
	void	mobilize_builder();
	void	firm_toggle_link_firm();
	void	firm_toggle_link_town();
	void	firm_pull_town_people();
	void	firm_set_worker_home();
	void	firm_bribe();
	void	firm_capture();
	void	camp_patrol();
	void	toggle_camp_patrol();
	void	firm_reward();
	void	inn_hire();
	void	market_scrap();
	void	market_hire_caravan();
	void	research_start();
	void	build_weapon();
	void	cancel_weapon();
	void	skip_build_weapon();
	void	build_ship();
	void	sail_ship();
	void	skip_build_ship();
	void	factory_change_product();
	void	base_mobilize_prayer();
	void	invoke_god();

	void	town_recruit();
	void	town_skip_recruit();
	void	town_migrate();
	void	town_collect_tax();
	void	town_reward();
	void	town_toggle_link_firm();
	void	town_toggle_link_town();
	void	town_auto_tax();
	void	town_auto_grant();
	void	town_grant_independent();

	void	wall_build();
	void	wall_destruct();

	void	spy_cycle_action();
	void	spy_leave_town();
	void	spy_leave_firm();
	void	spy_capture_firm();
	void	spy_drop_identity();
	void	spy_reward();
	void	spy_exposed();

	void	send_talk_msg();
	void	reply_talk_msg();
	void	nation_contact();
	void	nation_set_should_attack();

	 //##### trevor 30/9 #######//
	void	chat();
	 //##### trevor 30/9 #######//

	void	compare_remote_object();
	void	compare_remote_crc();

	void	caravan_copy_route();
	void	firm_request_builder();
	void	market_switch_restock();
};

//----------- Define class Remote -----------//

class MultiPlayer;

class Remote
{
public:
	enum { COMMON_MSG_BUF_SIZE	    	 = 1024,			// Remote allocates a common RemoteMsg object with a data buffer of this size.
			 SEND_QUEUE_BUF_SIZE		  	 = 1024, 		// The default queue buffer size
			 SEND_QUEUE_BUF_INC_SIZE 	 = 1024,			// If the queue is full, expand with this size
			 RECEIVE_QUEUE_BUF_SIZE		 = 8192, 		// The default queue buffer size
			 RECEIVE_QUEUE_BUF_INC_SIZE = 2048,			// If the queue is full, expand with this size
//			 MAX_PROCESS_FRAME_DELAY = 5,					// process player action 1 frame later
			 MAX_PROCESS_FRAME_DELAY = 8,					// process player action 1 frame later
			 SEND_QUEUE_BACKUP = MAX_PROCESS_FRAME_DELAY+4,
			 RECEIVE_QUEUE_BACKUP = (MAX_PROCESS_FRAME_DELAY+1)*2,
		  };

	enum { MODE_DISABLED = 0, MODE_MP_ENABLED, MODE_REPLAY, MODE_REPLAY_END };

public:
	char				is_host;
	char				handle_vga_lock;
	int				connectivity_mode;
	int				poll_msg_flag;
	//	Wsock*			wsock_ptr;
	MultiPlayer *mp_ptr;

	//--------- send queue -----------//

	RemoteQueue		send_queue[SEND_QUEUE_BACKUP];		// 0 for the latest, other for backup
	uint32_t	send_frame_count[SEND_QUEUE_BACKUP];
/*
	char*				send_queue_buf;
	char*				send_queue_ptr;
	int				send_queue_buf_size;
	int			   send_queued_size;

	//------- send backup queue --------//

	char*				backup_queue_buf;			// backup copy of the send_queue_buf
	int				backup_queued_size;		// for handling resend request
	DWORD         	backup_frame_count;

	char*				backup2_queue_buf;		// backup copy of the send_queue_buf
	int				backup2_queued_size;		// for handling resend request
	DWORD         	backup2_frame_count;
*/
	//------- receive queue ---------//

	RemoteQueue		receive_queue[RECEIVE_QUEUE_BACKUP];
	uint32_t	receive_frame_count[RECEIVE_QUEUE_BACKUP];
/*
	char*				receive_queue_buf;
	char*				receive_queue_ptr;
	int				receive_queue_buf_size;
	int			   receive_queued_size;

	char*				receive2_queue_buf;
	char*				receive2_queue_ptr;
	int				receive2_queue_buf_size;
	int			   receive2_queued_size;
*/
	char				process_queue_flag;

	//-------------------------------//

	int				packet_send_count;
	int				packet_receive_count;

	//-------------------------------//
	short				nation_processing;		// used in process_receive_queue

	char				save_file_name[FilePath::MAX_FILE_PATH];

	char				*common_msg_buf;
	// ###### patch begin Gilbert 22/1 #######//
	char				sync_test_level;			// 0=disable, bit0= random seed, bit1=crc
	// ###### patch end Gilbert 22/1 #######//
	int				process_frame_delay;

	// --------- alternating send frame --------//
	int				alternating_send_rate;	// 1=every frame, 2=send one frame per two frames...

	ReplayFile			replay;

public:
	Remote();
	~Remote();

	void			init(MultiPlayer *mp);
	int 			init_replay_load(char *full_path, NewNationPara *mpGame, int *playerCount);
	void			init_replay_save(NewNationPara *mpGame, int playerCount);
	void			deinit();

	void			init_start_mp();
	int				is_enable();
	int			is_replay();
	int			is_replay_end();
	// int			can_start_game();
	int				number_of_opponent();
	PID_TYPE    	self_player_id();
	// void			set_disconnect_handler(DisconnectFP disconnectFP);

	int				create_game();
	int				connect_game();
	void			start_game();

	void 			send_msg(RemoteMsg* remoteMsgPtr, uint32_t receiverId=0);
	void 			send_free_msg(RemoteMsg* remoteMsgPtr, uint32_t receiverId=0);

	RemoteMsg* 		new_msg(uint32_t msgId, int dataSize);
	void 			free_msg(RemoteMsg* remoteMsgPtr);

	char* 	 		new_send_queue_msg(uint32_t msgId, int msgSize);
	int				send_queue_now(uint32_t receiverId=0);
	int				send_backup_now(uint32_t receiverId, uint32_t requestFrameCount);
	void 			append_send_to_receive();
	void			copy_send_to_backup();
	// int			poll_msg(UINT message, UINT wParam, LONG lParam);
	int				poll_msg();
	void			enable_poll_msg();
	void			disable_poll_msg();


	void 			process_receive_queue();
	void 			process_specific_msg(uint32_t msgId);

	void			init_send_queue(uint32_t,short);
	void			init_receive_queue(uint32_t);

	void			enable_process_queue();
	void			disable_process_queue();

	void			reset_process_frame_delay();
	int				get_process_frame_delay();
	void			set_process_frame_delay(int);
	int				calc_process_frame_delay(int milliSecond);

	// ------- alternating send frame -------//
	void			set_alternating_send(int rate);
	int				get_alternating_send();
	int				has_send_frame(int nationRecno, uint32_t frameCount);
	uint32_t		next_send_frame(int nationRecno, uint32_t frameCount);
};

extern Remote remote;

//------------------------------------//

#endif
