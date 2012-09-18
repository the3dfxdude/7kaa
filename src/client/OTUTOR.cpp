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

//Filename    : OTUTOR.CPP
//Description : Class Tutor

#include <OVGA.h>
#include <vga_util.h>
#include <OSYS.h>
#include <OAUDIO.h>
#include <OBATTLE.h>
#include <OFONT.h>
#include <OF_MONS.h>
#include <OMONSRES.h>
#include <OIMGRES.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <OGFILE.h>
#include <OGAME.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OFILETXT.h>
#include <OTUTOR.h>

//---------- define constant ------------//

#define TUTOR_DB   		"TUT_LIST.RES"
#define TUTOR_TEXT_RES  "TUT_TEXT.RES"
#define TUTOR_INTRO_RES "TUT_INTR.RES"

//---------- define coordinations ------------//

enum { TUTOR_X1 = ZOOM_X1,
		 TUTOR_Y1 = ZOOM_Y1,
		 TUTOR_X2 = ZOOM_X2,
		 TUTOR_Y2 = TUTOR_Y1+120
	  };

enum { TUTOR_BUTTON_X1 = TUTOR_X2-66,
		 TUTOR_BUTTON_Y1 = TUTOR_Y1+20
	  };

//-------- Define static vars ----------//

static Button button_new_tutor, button_quit_tutor;
static Button button_restart, button_prev, button_next;
static Button3D button_sample;

//------- Begin of function Tutor::Tutor -----------//

Tutor::Tutor()
{
	init_flag = 0;
	tutor_count = 0;
	tutor_info_array = NULL;
	cur_tutor_id = 0;
	cur_text_block_id = 0;
	last_text_block_id = 0;
	cur_speech_wav_id = 0;
	tutor_text_buf = NULL;
	tutor_text_buf_size = 0;
	tutor_intro_buf = NULL;
	tutor_intro_buf_size = 0;
	text_block_array = NULL;
	text_block_count = 0;
}
//--------- End of function Tutor::Tutor -----------//


//------- Begin of function Tutor::~Tutor -----------//

Tutor::~Tutor()
{
	deinit();
}
//--------- End of function Tutor::~Tutor -----------//


//---------- Begin of function Tutor::init -----------//
//
// This function must be called after a map is generated.
//
void Tutor::init()
{
	deinit();

	//------- allocate text_block_array ----------//

	text_block_array = (TutorTextBlock*) mem_add( sizeof(TutorTextBlock) * MAX_TUTOR_TEXT_BLOCK );

	//----- open tutorial text resource file -------//

	String str;

	str  = DIR_RES;
	str += TUTOR_TEXT_RES;

	res_tutor_text.init(str, 0);  	// 0-don't read all into buffer

	//----- open tutorial introduction resource file -------//

	str  = DIR_RES;
	str += TUTOR_INTRO_RES;

	res_tutor_intro.init(str, 0);  	// 0-don't read all into buffer

	//------- load database information --------//

	load_tutor_info();

	init_flag=1;
}
//---------- End of function Tutor::init -----------//


//---------- Begin of function Tutor::deinit -----------//

void Tutor::deinit()
{
	if( init_flag )
	{
		mem_del(tutor_info_array);
      tutor_info_array = NULL;

		mem_del(text_block_array);
		text_block_array = NULL;

		if( tutor_text_buf )
		{
			mem_del(tutor_text_buf);
			tutor_text_buf = NULL;
		}

		if( tutor_intro_buf )
		{
			mem_del(tutor_intro_buf);
			tutor_intro_buf = NULL;
		}

		res_tutor_text.deinit();
		res_tutor_intro.deinit();

		init_flag=0;
	}
}
//---------- End of function Tutor::deinit -----------//


//------- Begin of function Tutor::load_tutor_info -------//
//
// Read in information of GOD.DBF into memory array
//
void Tutor::load_tutor_info()
{
	TutorRec  *tutorRec;
	TutorInfo *tutorInfo;

	String str;

	str = DIR_RES;
	str += TUTOR_DB;

	Database  dbTutor(str, 1);

	tutor_count      = (short) dbTutor.rec_count();
	tutor_info_array = (TutorInfo*) mem_add( sizeof(TutorInfo)*tutor_count );

	//------ read in tutor information array -------//

	memset( tutor_info_array, 0, sizeof(TutorInfo) * tutor_count );

	for( int i=0 ; i<tutor_count ; i++ )
	{
		tutorRec  = (TutorRec*) dbTutor.read(i+1);
		tutorInfo = tutor_info_array+i;

		misc.rtrim_fld( tutorInfo->code, tutorRec->code, tutorRec->CODE_LEN );
		misc.rtrim_fld( tutorInfo->des , tutorRec->des , tutorRec->DES_LEN  );
#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
		translate.multi_to_win(tutorInfo->des, tutorInfo->DES_LEN);
#endif
	}

	//--- exclude the Fryhtan and Seat of Power tutorials in the demo ---//

	#ifdef DEMO
		tutor_count -= 2;
	#endif
}
//--------- End of function Tutor::load_tutor_info ---------//


//------------ Begin of function Tutor::load -------------//
//
// <int> tutorId - id. of the tutorial
//
void Tutor::load(int tutorId)
{
	cur_tutor_id = tutorId;

	//------- get the tutor msgs from the resource file -------//

	int   dataSize;
	File* filePtr = res_tutor_text.get_file( tutor[tutorId]->code, dataSize);

	if( !filePtr )       // if error getting the tutor resource
	{
		err_here();
		return;
	}

	//------ Open the file and allocate buffer -----//

	FileTxt fileTxt( filePtr, dataSize );  // initialize fileTxt with an existing file stream

	if( dataSize > tutor_text_buf_size )
	{
		tutor_text_buf      = mem_resize( tutor_text_buf, dataSize );       // allocate a buffer larger than we need for the largest size possible
		tutor_text_buf_size = dataSize;
	}

	//-------- read in tutor info one by one --------//

	TutorTextBlock* tutorTextBlock = text_block_array;
	char*     textPtr = tutor_text_buf;
	int       readLen, totalReadLen=0;    // total length of text read into the tutor_text_buf
	int		 loopCount=0;
	char*		 tokenStr;

	text_block_count=0;

	fileTxt.next_line();    // by pass the first two lines of file description
	fileTxt.next_line();

	while( !fileTxt.is_eof() )
	{
		err_when( loopCount++ > 10000 );

		tokenStr = fileTxt.get_token(0);		// don't advance the pointer

		if( !tokenStr )
			break;

		//------ read in the display button code of the tutorial segment -------//

		if( strcmpi( tokenStr, "Button" ) == 0 )
		{
			fileTxt.get_token(1);		// advance the pointer

			tokenStr = fileTxt.get_token(1);

			strncpy( tutorTextBlock->button_code, tokenStr, tutorTextBlock->BUTTON_CODE_LEN );
			tutorTextBlock->button_code[tutorTextBlock->BUTTON_CODE_LEN] = '\0';
		}
		else
		{
			tutorTextBlock->button_code[0] = '\0';
		}

		//------- read in the tutorial text -------//

		readLen = fileTxt.read_paragraph(textPtr, tutor_text_buf_size-totalReadLen);

		tutorTextBlock->text_ptr = textPtr;
		tutorTextBlock->text_len = readLen;

		textPtr      += readLen;
		totalReadLen += readLen;

		err_when( totalReadLen>tutor_text_buf_size );

		//----------- next tutor block -------------//

		fileTxt.next_line();      // pass the page break line

		text_block_count++;
		tutorTextBlock++;

		err_when( text_block_count >= MAX_TUTOR_TEXT_BLOCK );
	}

	cur_text_block_id = 1;
	last_text_block_id = 0;
}
//------------- End of function Tutor::load -------------//


//------------ Begin of function Tutor::get_intro -------------//
//
// <int> tutorId - id. of the tutorial
//
// return: <char*> return the introduction text of the tutorial.
//
char* Tutor::get_intro(int tutorId)
{
	//------- get the tutor msgs from the resource file -------//

	int   dataSize;
	File* filePtr = res_tutor_intro.get_file( tutor[tutorId]->code, dataSize);

	if( !filePtr )       // if error getting the tutor resource
	{
		err_here();
		return NULL;
	}

	//------ Open the file and allocate buffer -----//

	FileTxt fileTxt( filePtr, dataSize );  // initialize fileTxt with an existing file stream

	if( dataSize > tutor_intro_buf_size )
	{
		tutor_intro_buf      = mem_resize( tutor_intro_buf, dataSize );       // allocate a buffer larger than we need for the largest size possible
		tutor_intro_buf_size = dataSize;
	}

	// #### begin Gilbert 23/9 #######//
	fileTxt.read_paragraph(tutor_intro_buf, tutor_intro_buf_size);
	// #### end Gilbert 23/9 #######//

	return tutor_intro_buf;
}
//------------- End of function Tutor::get_intro -------------//


//-------- Begin of function Tutor::select_run_tutor --------//
//
// <int> inGameCall - whether it's called from the main menu or from in-game tutorial.
//
void Tutor::select_run_tutor(int inGameCall)
{
	if( inGameCall )
		select_tutor(-2);

	int tutorId = select_tutor(1);

	if( tutorId > 0 )
		run(tutorId, inGameCall);

	if( inGameCall )
		select_tutor(-1);
}
//--------- End of function Tutor::select_run_tutor ---------//


//---------- Begin of function Tutor::run -----------//

void Tutor::run(int tutorId, int inGameCall)
{
	//--- don't load pre-saved game when selecting a new tutorial in the game ---//

	if( !inGameCall )
	{
		String str;

		str  = DIR_TUTORIAL;
		str += tutor[tutorId]->code;
		str += ".TUT";

		if( misc.is_file_exist(str) )
		{
			game_file.load_game("", str);
		}
		else
		{
			str = DIR_TUTORIAL;
			str += "STANDARD.TUT";

			if( misc.is_file_exist(str) )
				game_file.load_game("", str);
		}

		//------ fix firm_build_id problem -----//

		Firm* firmPtr;

		for( int i=firm_array.size() ; i>0 ; i-- )
		{
			if( firm_array.is_deleted(i) )
				continue;

			firmPtr = firm_array[i];

			if( firmPtr->firm_id != FIRM_MONSTER )
				continue;

			int monsterId = ((FirmMonster*)firmPtr)->monster_id;

			firmPtr->firm_build_id = firm_res[FIRM_MONSTER]->get_build_id( monster_res[monsterId]->firm_build_code );
		}

		//----- set the gaming speed to normal -----//

		sys.set_speed(9, COMMAND_AUTO);
	}

	//--------- load the tutorial text ------------//

	tutor.load(tutorId);			

	game.game_mode = GAME_TUTORIAL;

	//------------------------------------------//

	if( !inGameCall )
	{
		battle.run_loaded();
		game.deinit();
	}
}
//----------- End of function Tutor::run ------------//


//------------ Begin of function Tutor::disp ------------//

void Tutor::disp()
{
	vga.use_back();

	Vga::opaque_flag = config.opaque_report;
	vga_util.d3_panel_down( TUTOR_X1, TUTOR_Y1, TUTOR_X2, TUTOR_Y2 );
	Vga::opaque_flag = 0;

	TutorTextBlock* tutorTextBlock = text_block_array+cur_text_block_id-1;

	//------- display button bitmap ------//

	int textX2 = TUTOR_X2-10;

	if( tutorTextBlock->button_code[0] )
	{
		button_sample.paint( TUTOR_BUTTON_X1, TUTOR_BUTTON_Y1, 'A', tutorTextBlock->button_code );
		textX2 = TUTOR_BUTTON_X1-16;
	}

	//-------- display tutorial text --------//

	font_san.put_paragraph( TUTOR_X1+10, TUTOR_Y1+10, textX2, TUTOR_Y2-10,
									tutorTextBlock->text_ptr, 4 );

	//--------- display controls ---------//

	int x=TUTOR_X1+10, y=TUTOR_Y2-22;

	#ifdef GERMAN
		button_new_tutor.paint_text( x, y, "Next Training" );
		button_quit_tutor.paint_text( x+120, y, "Quit Training" );
	#else
		button_new_tutor.paint_text( x, y, "Next Training" );
		button_quit_tutor.paint_text( x+100, y, "Quit Training" );
	#endif

	//---- display current text block position ----//

	x += 360;

	String str;

	str  = cur_text_block_id;
	str += " / ";
	str += text_block_count;

	font_san.put( x, y+3, str );

	x += 60;

	//------- display other controls --------//

	button_restart.paint_text( x, y, "|<<" );

	if( cur_text_block_id > 1 )
		button_prev.paint_text( x+45, y, " < " );

	if( cur_text_block_id < text_block_count )
		button_next.paint_text( x+88, y, " > " );

	vga.use_front();

	//------ play speech of the tutorial -------//

	if( last_text_block_id != cur_text_block_id )
	{
		last_text_block_id = cur_text_block_id;
		play_speech();
   }
}
//------------ End of function Tutor::disp ------------//


//----------- Begin of function Tutor::play_speech ------------//

void Tutor::play_speech()
{
	if( !sys.dir_tutorial[0] )
		return;

	if( cur_speech_wav_id )
		stop_speech();

	//---------------------------------//

	String str;

	str  = sys.dir_tutorial;
	str += tutor[cur_tutor_id]->code;
	str += "\\TUT";

	if( cur_text_block_id < 10 )		// Add a zero. e.g. "TUT01"
		str += "0";

	str += cur_text_block_id;

	if( !misc.is_file_exist(str) )
		return;

	// ##### begin Gilbert 25/9 ######//
	cur_speech_wav_id = audio.play_long_wav(str, DEF_REL_VOLUME);		// cur_speech_wav_id is the WAV id that is needed for stopping playing of the WAV file
	// ##### end Gilbert 25/9 ######//
}
//------------ End of function Tutor::play_speech ------------//


//----------- Begin of function Tutor::stop_speech ------------//

void Tutor::stop_speech()
{
	audio.stop_long_wav(cur_speech_wav_id);
	cur_speech_wav_id = 0;
}
//------------ End of function Tutor::stop_speech ------------//


//---------- Begin of function Tutor::detect ----------//

int Tutor::detect()
{
	if( button_new_tutor.detect() )
	{
   	stop_speech();
		select_run_tutor(1);		// select and run tutorial, 1-called from in-game, not from the main menu
		return 1;
	}

	if( button_quit_tutor.detect() )
	{
		stop_speech();
		game.game_mode = GAME_SINGLE_PLAYER;
		return 1;
	}

	if( button_restart.detect() )
	{
		cur_text_block_id = 1;
		return 1;
	}

	if( cur_text_block_id > 1 && button_prev.detect() )
	{
		prev_text_block();
		return 1;
	}

	if( cur_text_block_id < text_block_count && button_next.detect() )
	{
		next_text_block();
		return 1;
	}

	return 0;
}
//----------- End of function Tutor::detect ------------//


//---------- Begin of function Tutor::prev_text_block ----------//

void Tutor::prev_text_block()
{
	if( cur_text_block_id > 1 )
		cur_text_block_id--;
}
//----------- End of function Tutor::prev_text_block ------------//


//---------- Begin of function Tutor::next_text_block ----------//

void Tutor::next_text_block()
{
	if( cur_text_block_id < text_block_count )
		cur_text_block_id++;
}
//----------- End of function Tutor::next_text_block ------------//


//---------- Begin of function Tutor::operator[] -----------//

TutorInfo* Tutor::operator[](int recNo)
{
	err_when( recNo < 1 || recNo > tutor_count );

	return tutor_info_array + recNo - 1;
}
//----------- End of function Tutor::operator[] ------------//
