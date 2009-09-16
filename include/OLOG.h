// Filename    : LOG.H
// Description : Logging to locate hang

#ifndef __LOG_H
#define __LOG_H

#include <OSTR.h>
#include <OVQUEUE.h>

//#define ENABLE_LOG

#define DEBUG_LOG_GLOBAL 0
#include <OLOG.h>
// 1 to force enable debug log of that module, even though DEBUG_LOG_GLOBAL is 0
// -1 to force disable debug log of that module, even though DEBUG_LOG_GLOBAL is 1
// 0 to follow DEBUG_LOG_GLOBAL

class Log
{
private:
	enum { LOG_VERSION = 12 };
	VLenQueue text_buffer[LOG_VERSION];
	String	log_text;
	String	log_file;
	int		log_line;

public:
	Log();
	~Log();
	void mark_begin();
	void mark_end();
	void mark(char *msg, char *file, int line);
	void mark(int n, char *file, int line);
	void dump();

	static void debug_log( char *msg );
	static void debug_log( int n );
};

extern Log msg_log;

#if defined(DEBUG) && defined(ENABLE_LOG)
	#define LOG_BEGIN		msg_log.mark_begin()
	#define LOG_MSG(c)	msg_log.mark(c, __FILE__, __LINE__)
	#define LOG_END		msg_log.mark_end()
	#define LOG_DUMP		msg_log.dump()
#else
	#define LOG_BEGIN
	#define LOG_MSG(c)
	#define LOG_END
	#define LOG_DUMP
#endif


#ifndef DEBUG_LOG_LOCAL
	#define DEBUG_LOG_LOCAL 0
#endif

#if (DEBUG_LOG_GLOBAL + DEBUG_LOG_LOCAL > 0)
	#define DEBUG_LOG(c)	Log::debug_log(c)
#else
	#define DEBUG_LOG(c)
#endif


#endif