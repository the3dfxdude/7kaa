// Filename    : OIMMPLAY.H
// Description : header file of MultiPlayerIm (I'Magic Multiplayer SDK)
// Owner       : Gilbert

#ifndef __OIMMPLAY_H
#define __OIMMPLAY_H

#ifdef IMAGICMP

#include <ODYNARRB.H>
#include <immlib.h>

extern GUID GAME_GUID;
extern char *GAME_GUID_STR;

#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
#define MP_FRIENDLY_NAME_LEN 20
#define MP_FORMAL_NAME_LEN 64
#define MP_RECV_BUFFER_SIZE 0x2000

struct IMServiceProvider : public IMMSERVICEINFO
{
	IMServiceProvider(const IMMSERVICEINFO &);

	char *name_str();
	DWORD service_id() { return dwServiceID; }
	char *name_str_long();
};

struct IMSessionDesc : public IMMSESSIONINFO
{
	IMSessionDesc() {}
	IMSessionDesc(const IMMSESSIONINFO &);
	void after_copy() {}
	IMSessionDesc *before_use() { return this; }

	char *name_str() { return szSessionName; };
	DWORD session_id() { return dwSessionID; }
};

struct IMPlayer : public IMMPLAYERINFO
{
	char	connecting;		// initially set to 1, 
	                     // clear after disconnect
	IMPlayer(const IMMPLAYERINFO &);

	IMMPID	pid()			{ return imID; }
	char *friendly_name_str() { return szFriendly; }
	char *formal_name_str() { return szFormal; }
};

class MultiPlayerIM
{
public:
	int						init_flag;
	DynArrayB				service_providers;		// array of IMServiceProvider
	DynArrayB				current_sessions;			// array of IMSessionDesc
	PCIMMID					imm_ptr;
	IMSessionDesc			joined_session;
	char						skipped_session;			// create/join session during create player
	char						lobbied_flag;
	int						use_receive_thread;

	IMMPID					my_player_id;
	int						host_flag;
	DynArrayB				player_pool;				// array of IMPlayer

	char *					recv_buffer;
	DWORD						recv_buffer_size;

public:
	MultiPlayerIM();
	~MultiPlayerIM();
	void pre_init();
	void init(DWORD serviceProvideCode);
	void deinit();

	// ------- functions on lobby ----------//
	void	init_lobbied(int maxPlayers, char *cmdLine);
	int	is_lobbied();		// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char *get_lobbied_name()		{ return 0; }		// not available

	// ------- functions on service provider ------ //
	void	poll_service_providers();								// can be called before init
	IMServiceProvider *get_service_provider(int i);		// can be called before init

	// ------- functions on session --------//
	int	poll_sessions();
	void	sort_sessions(int sortType);
	IMSessionDesc *get_session(int i);
	int	create_session(char *sessionName, int maxPlayers);
	int	join_session(IMSessionDesc* sessionDesc);
	int	join_session(int currentSessionIndex );
	void	close_session();
	void	disable_join_session();		// so that new player cannot join

	// -------- functions on player management -------//
	int	create_player(char *friendlyName, char *formalName,
		LPVOID lpData=NULL, DWORD dataSize=0, DWORD flags=0);
	void	destroy_player( IMMPID playerId );
	void	poll_players();
	IMPlayer *get_player(int i);
	IMPlayer *search_player(IMMPID playerId);
	IMPlayer *search_player(char *name);
	int	is_host(IMMPID playerId);
	int	am_I_host();
	int	is_player_connecting(IMMPID playerId);

	// ------- functions on message passing ------//
	int	send(IMMPID toId, LPVOID lpData, DWORD dataSize);
	void	begin_stream(IMMPID toID);
	int	send_stream(IMMPID toId, LPVOID lpData, DWORD dataSize);
	void	end_stream(IMMPID toID);
	int	get_msg_count();
	char *receive(IMMPID *from, IMMPID *to, LPDWORD recvLen, int *sysMsgCount=0);

	void	before_receive();				// call before receive
	void	after_send();					// call after send

protected:
	void	handle_system_msg(LPVOID, DWORD );
	// void	handle_lobby_system_msg(LPVOID, DWORD);
};

extern MultiPlayerIM mp_im;

#include <MPTYPES.H>

#endif // IMAGICMP
#endif // __OIMMPLAY_H