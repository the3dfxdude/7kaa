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

//Filename    : OSFRMRES.CPP
//Description : Object Sprite Frame Resource

#include <ALL.h>
#include <OSTR.h>
#include <OSYS.h>
#include <OGAMESET.h>
#include <OSFRMRES.h>

//-------- define file name -----------//

#define SPRITE_FRAME_DB    "SFRAME"


//-------- Begin of function SpriteFrameRes::init ---------//

void SpriteFrameRes::init()
{
	deinit();

	//------- load database information --------//

	load_info();

	init_flag=1;
}
//--------- End of function SpriteFrameRes::init ----------//


//-------- Begin of function SpriteFrameRes::deinit ---------//

void SpriteFrameRes::deinit()
{
	if( init_flag )
	{
		delete[] sprite_frame_array;

		init_flag=0;
	}
}
//--------- End of function SpriteFrameRes::deinit ----------//


//------- Begin of function SpriteFrameRes::load_info ---------//

void SpriteFrameRes::load_info()
{
	Database   		*dbSpriteFrame = game_set.open_db(SPRITE_FRAME_DB);
	SpriteFrameRec *frameRec;
	SpriteFrame 	*spriteFrame;
	int		  		i;

	sprite_frame_count = dbSpriteFrame->rec_count();
	sprite_frame_array = new SpriteFrame[sprite_frame_count];

	memset( sprite_frame_array, 0, sizeof(SpriteFrame)*sprite_frame_count );

	//--------- read in frame information ---------//

	for( i=0 ; i<dbSpriteFrame->rec_count() ; i++ )
	{
		frameRec  = (SpriteFrameRec*) dbSpriteFrame->read(i+1);
		spriteFrame = sprite_frame_array+i;

		spriteFrame->offset_x = misc.atoi(frameRec->offset_x, frameRec->OFFSET_LEN);
		spriteFrame->offset_y = misc.atoi(frameRec->offset_y, frameRec->OFFSET_LEN);

		spriteFrame->width  = misc.atoi(frameRec->width , frameRec->WIDTH_LEN);
		spriteFrame->height = misc.atoi(frameRec->height, frameRec->HEIGHT_LEN);

		memcpy( &spriteFrame->bitmap_offset, frameRec->bitmap_offset, sizeof(uint32_t) );
	}
}
//-------- End of function SpriteFrameRes::load_info ---------//


#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS

//-------- Begin of function SpriteFrameRes::operator[] -------//

SpriteFrame* SpriteFrameRes::operator[](int recNo)
{
	if( recNo<1 || recNo>sprite_frame_count )
		err.run( "SpriteFrameRes::operator[]" );

	return sprite_frame_array+recNo-1;
}

//--------- End of function SpriteFrameRes::operator[] --------//

#endif
