// Filename    : OLONGLOG.H
// Description : 

#ifndef __OLONGLOG_H
#define __OLONGLOG_H

#include <OFILE.H>

class LongLog : public File
{
public:
	LongLog(char suffix);
	~LongLog();

	void printf(char*,...);
};

#ifdef DEBUG
extern LongLog *long_log;
#endif

#endif