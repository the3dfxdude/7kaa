//Filename    : OERR.CPP
//Description : Object Error Handling

#include <new.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include <OSYS.H>
#include <OBOX.H>
#include <OVGA.H>
#include <ALL.H>

//------------------------------------------------//
//
// There are several types of errors :
//
// 1. Internal Errors caused by bugs of the program
// 2. Runtime Errors caused by using up of resources (memory) or
//    unexpected environment errors. (disk error)
//
//------------------------------------------------//

static int new_func_handler(size_t);

//---------- define static variable ----------//

static char error_flag=0;		// prevent error message dead loop

//------- Begin of function Error::Error ------------//
//
// Set new() operator error handler, new_handler() is called when
// new cannot allocate sufficient memory required.
//
Error::Error()
{
	_set_new_handler(new_func_handler);        // set_new_handler() is a C++ function

	extra_handler = NULL;
}
//-------- End of function Error::Error --------------//


//------- Begin of function new_func_handler ------------//
//
static int new_func_handler(size_t allocSize)
{
	err.mem();

	return 0;
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
void Error::internal(char* errMsg,char* fileName,int lineNum)
{
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

	OutputDebugString( strBuf );

	if( vga.is_inited() )
		box.msg( strBuf, 0 );

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
	if( error_flag )	// prevent error message dead loop
		return;

	error_flag=1;

	//-------------------------------------------------//

	if( extra_handler )
		(*extra_handler)();

	char* strBuf = "Insufficient Memory, execution interrupt.";

	//-------- display error message -------//

	OutputDebugString( strBuf );

	if( vga.is_inited() )
		box.msg( strBuf, 0 );

	sys.deinit_directx();

	exit( -2 );
}
//--------- END OF FUNCTION Error::mem ----------//


//------- BEGIN OF FUNCTION Error::msg -----------//
//
// <char*> formated erorr message with % argument
// <....>  the argument list
//
void Error::msg( char *format, ... )
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

	OutputDebugString( strBuf );

	if( vga.is_inited() )
		box.msg( strBuf, 0 );

	sys.deinit_directx();

	error_flag = 0;		// this error does not exit program
}
//--------- END OF FUNCTION Error::msg ----------//


//------- BEGIN OF FUNCTION Error::run --------//
//
// <char*> formated erorr message with % argument
// <....>  the argument list
//
void Error::run( char *format, ... )
{
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

	OutputDebugString( strBuf );

	if( vga.is_inited() )
		box.msg( strBuf, 0 );

	sys.deinit_directx();

	exit( -2 );
}
//---------- END OF FUNCTION Error::run -----------//

