// Filename   : OLONGLOG.CPP

#include <OLONGLOG.H>
#include <stdio.h>


LongLog::LongLog(char suffix)
{
	char filename[] = "LLONGx.LOG";
	filename[5] = suffix;
	file_create(filename);
}

LongLog::~LongLog()
{
	file_close();
}

void LongLog::printf(char *format, ...)
{
	//---- translate the message and the arguments into one message ----//

	char strBuf[150];

	va_list argPtr;        // the argument list structure

	va_start( argPtr, format );
	vsprintf( strBuf, format, argPtr );
	file_write(strBuf, strlen(strBuf));
	va_end( argPtr );
}


