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

// Filename    : OLOG.CPP
// Description : logging class
// Owner       : Gilbert


#include <OLOG.h>
#include <stdio.h>
#include <OFILE.h>


// -------- begin of function Log::Log ----------//
Log::Log()
{
}
// -------- end of function Log::Log ----------//

Log::~Log()
{
}

// -------- begin of function Log::mark_begin ----------//
void Log::mark_begin()
{
	// text_buffer[0] stores the latest log
	text_buffer[0].clear();
	*(text_buffer[0].reserve(sizeof(char))) = '\0';	// put a null character in the buffer
}
// -------- end of function Log::mark_begin ----------//


// -------- begin of function Log::mark_end ----------//
void Log::mark_end()
{
	// enable next line to trace hang
	// dump();

	// rotate buffers
	text_buffer[LOG_VERSION-1].clear();
	*(text_buffer[LOG_VERSION-1].reserve(sizeof(char))) = '\0';	// put a null character in the buffer
	for(int n = LOG_VERSION-1; n > 0; --n)
	{
		text_buffer[n].swap(text_buffer[n-1]);
	}
}
// -------- end of function Log::mark_end ----------//


// -------- begin of function Log::mark ----------//
void Log::mark(char *msg, char *file, int line)
{
	log_text = msg;
	log_file = file;
	log_line = line;

	String t(log_text);
	t += "\r\n";
	strcpy( text_buffer[0].reserve(t.len())-1, t );		// minus 1 to remove the '\0' at the end 
}
// -------- end of function Log::mark ----------//


// -------- begin of function Log::mark ----------//
void Log::mark(int n , char *file, int line)
{
	log_text = n;
	log_file = file;
	log_line = line;

	String t(log_text);
	t += "\r\n";
	strcpy( text_buffer[0].reserve(t.len())-1, t );
}
// -------- end of function Log::mark ----------//


// -------- begin of function Log::dump ----------//
void Log::dump()
{
	// write old_buffer
	char filename[20];
	strcpy(filename, "AM_A.LOG");
	File f;
	for(int n = LOG_VERSION-1; n >= 0; --n, filename[3]++)	// AM_A.LOG, AM_B.LOG ...
	{
		f.file_create(filename);
		f.file_write( text_buffer[n].queue_buf, text_buffer[n].length() );
		f.file_close();
	}
}
// -------- end of function Log::dump ----------//


// -------- begin of function Log::debug_log ----------//
void Log::debug_log(char *msg)
{
	String s;
	s = msg; 
	s += "\r\n"; 
	OutputDebugString(s);
}


void Log::debug_log(int n)
{
	String s;
	s = n; 
	s += "\r\n"; 
	OutputDebugString(s);
}
// -------- end of function Log::debug_log ----------//
