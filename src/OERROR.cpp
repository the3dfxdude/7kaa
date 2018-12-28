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

//Filename    : OERR.CPP
//Description : Object Error Handling

#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef USE_WINDOWS
#include <windows.h> // OutputDebugString
#endif

#include <OSYS.h>
#include <OBOX.h>
#include <OVGA.h>
#include <ALL.h>

#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(fatal);

//------------------------------------------------//
//
// There are several types of errors :
//
// 1. Internal Errors caused by bugs of the program
// 2. Runtime Errors caused by using up of resources (memory) or
//    unexpected environment errors. (disk error)
//
//------------------------------------------------//

static void new_func_handler();

//---------- define static variable ----------//

static char error_flag=0;		// prevent error message dead loop

//------- Begin of function Error::Error ------------//
//
// Set new() operator error handler, new_handler() is called when
// new cannot allocate sufficient memory required.
//
Error::Error()
{
	std::set_new_handler(new_func_handler);        // set_new_handler() is a C++ function

	extra_handler = NULL;
}
//-------- End of function Error::Error --------------//


//------- Begin of function new_func_handler ------------//
//
static void new_func_handler()
{
	err.mem();
}
//-------- End of function new_func_handler --------------//


//------- BEGIN OF FUNCTION Error::internal -----------//
//
// sample error message :
//
// Exit : Insufficient Memory
// File : ODYNARR.CPP
// Line : 453
//
// Continue ?
//
// <char*> errMsg   - the error message
// <char*> fileName - the file name of the CPP function cause error
//                    usually is __FILE__
// <int>   lineNum  - the line number of program cause error
//                    usually is __LINE__
//
void Error::internal(char* errMsg,const char* fileName,int lineNum)
{
#if defined(_MSC_VER) && defined(_DEBUG)
	// Let the debugger have a first shot at the error when in debug build, before clean-up happens.
	__debugbreak();
#endif

 	if( error_flag )	// prevent error message dead loop
		return;

	error_flag=1;

	//-------------------------------------------------//

	char strBuf[100];

	if( extra_handler )		// all the extra error handler first
		(*extra_handler)();

	if( errMsg )
		sprintf(strBuf, "Error : %s\nFile : %s\nLine : %d\n", errMsg,fileName,lineNum );
	else
		sprintf(strBuf, "Error on File : %s\nLine : %d\n",fileName,lineNum );

	//-------- display error message -------//

	ERR("%s\n", strBuf);
#ifdef USE_WINDOWS
	OutputDebugString( strBuf );
#endif

	if( sys.init_flag )
		box.msg( strBuf, 0 );
	else
		sys.show_error_dialog( "%s\n", strBuf );

	sys.deinit_directx();

	exit( -2 );
}
//--------- END OF FUNCTION Error::internal ----------//


//------- BEGIN OF FUNCTION Error::mem -----------//
//
// There is no memory left to save the screen, so don't use v_pop.ask(),
// direct output to screen.
//
void Error::mem()
{
#if defined(_MSC_VER) && defined(_DEBUG)
	// Let the debugger have a first shot at the error when in debug build, before clean-up happens.
	__debugbreak();
#endif

	if( error_flag )	// prevent error message dead loop
		return;

	error_flag=1;

	//-------------------------------------------------//

	if( extra_handler )
		(*extra_handler)();

	const char* strBuf = "Insufficient Memory, execution interrupt.";

	//-------- display error message -------//

	ERR("%s\n", strBuf);
#ifdef USE_WINDOWS
	OutputDebugString( strBuf );
#endif

	if( sys.init_flag )
		box.msg( strBuf, 0 );
	else
		sys.show_error_dialog( "%s\n", strBuf );

	sys.deinit_directx();

	exit( -2 );
}
//--------- END OF FUNCTION Error::mem ----------//


//------- BEGIN OF FUNCTION Error::msg -----------//
//
// <char*> formated erorr message with % argument
// <....>  the argument list
//
void Error::msg( const char *format, ... )
{
	if( error_flag )	// prevent error message dead loop
		return;

	error_flag=1;

	//-------------------------------------------------//

	//---- translate the message and the arguments into one message ----//

	char strBuf[100];

	va_list argPtr;        // the argument list structure

	va_start( argPtr, format );
	vsprintf( strBuf, format, argPtr );

	va_end( argPtr );

	//-------- display error message -------//

	ERR("%s\n", strBuf);
#ifdef USE_WINDOWS
	OutputDebugString( strBuf );
#endif

	if( sys.init_flag )
		box.msg( strBuf, 0 );
	else
		sys.show_error_dialog( "%s\n", strBuf );

	sys.deinit_directx();

	error_flag = 0;		// this error does not exit program
}
//--------- END OF FUNCTION Error::msg ----------//


//------- BEGIN OF FUNCTION Error::run --------//
//
// <char*> formated error message with % argument
// <....>  the argument list
//
void Error::run( const char *format, ... )
{
#if defined(_MSC_VER) && defined(_DEBUG)
	// Let the debugger have a first shot at the error when in debug build, before clean-up happens.
	__debugbreak();
#endif

	if( error_flag )	// prevent error message dead loop
		return;

	error_flag=1;

	//-------------------------------------------------//

	if( extra_handler )
		(*extra_handler)();

	//---- translate the message and the arguments into one message ----//

	char strBuf[100];

	va_list argPtr;        // the argument list structure

	va_start( argPtr, format );
	vsprintf( strBuf, format, argPtr );

	va_end( argPtr );

	//-------- display error message -------//

	ERR("%s\n", strBuf);
#ifdef USE_WINDOWS
	OutputDebugString( strBuf );
#endif

	if( sys.init_flag )
		box.msg( strBuf, 0 );
	else
		sys.show_error_dialog( "%s\n", strBuf );

	// Richard 17-1-2014: Set exit flag to signal termination (for atexit cleanup)
	sys.signal_exit_flag = 1;

	sys.deinit_directx();

	exit( -2 );
}
//---------- END OF FUNCTION Error::run -----------//

