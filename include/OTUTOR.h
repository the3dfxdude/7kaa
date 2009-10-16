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

//Filename    : OTUTOR.H
//Description : Header file of object RaceRes

#ifndef __OH
#define __OH

#ifndef __ORESX_H
#include <ORESX.h>
#endif

//------------ Define constant -----------//

#define 	MAX_TUTOR_TEXT_BLOCK	100

//------------ Define struct TutorRec ---------------//

struct TutorRec
{
	enum { CODE_LEN=8, DES_LEN=40 };

	char  code[CODE_LEN];
	char  des[DES_LEN];
};

//------------- Define struct TutorInfo --------------//

struct TutorInfo
{
	enum { CODE_LEN=8, DES_LEN=40 };

	char  code[CODE_LEN+1];
	char  des[DES_LEN+1];
};

//---------- Define struct TutorTextBlock ------------//

struct TutorTextBlock
{
	enum { BUTTON_CODE_LEN=8 };

	char* text_ptr;          // offset of the help text in the text buffer
	short text_len;          // length of the help text

	char	button_code[BUTTON_CODE_LEN+1];
};

//----------- Define class Tutor ---------------//

class Tutor
{
public:
	char        init_flag;

	short       tutor_count;
	TutorInfo*  tutor_info_array;

	ResourceIdx res_tutor_text, res_tutor_intro;

	//-------------------------------//

	short				cur_tutor_id;
	short				cur_text_block_id;
	short				last_text_block_id;
	int				cur_speech_wav_id;

	char        	*tutor_text_buf;
	int				tutor_text_buf_size;
	char        	*tutor_intro_buf;
	int				tutor_intro_buf_size;

	TutorTextBlock *text_block_array;
	int				text_block_count;

public:
	Tutor();
	~Tutor();

	void        init();
	void        deinit();

	int			select_tutor(int);
	void			select_run_tutor(int inGameCall);
	void 			run(int tutorId, int inGameCall);
	void			load(int tutorId);

	char* 		get_intro(int tutorId);

	void			disp();
	int			detect();

	void 			prev_text_block();
	void 			next_text_block();

	void			play_speech();
	void			stop_speech();

	int         write_file(File*);
	int         read_file(File*);

	TutorInfo*  operator[](int tutorId);      // pass tutorId  as recno

private:
	void        load_tutor_info();
};

extern Tutor tutor;

//----------------------------------------------------//

#endif
