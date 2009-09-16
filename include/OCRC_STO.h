// Filename    : OCRC_STO.H
// Description : store of crc of objects


#ifndef __OCRC_STO_H
#define __OCRC_STO_H

#include <OVQUEUE.H>
#include <OSTR.H>

class CrcStore
{
public:
	VLenQueue nations;
	VLenQueue units;
	VLenQueue firms;
	VLenQueue towns;
	VLenQueue bullets;
	VLenQueue rebels;
	VLenQueue spies;
	VLenQueue talk_msgs;

	// #### patch begin Gilbert 23/1 #####//
	String	crc_error_string;
	// #### patch end Gilbert 23/1 #####//

public:
	CrcStore();
	void	init();
	void	deinit();

	void	record_nations();
	void	record_units();
	void	record_firms();
	void	record_towns();
	void	record_bullets();
	void	record_rebels();
	void	record_spies();
	void	record_talk_msgs();

	void	record_all();
	void	send_all();
	int	compare_remote(DWORD remoteMsgId, char *);
};

extern CrcStore crc_store;

#endif 