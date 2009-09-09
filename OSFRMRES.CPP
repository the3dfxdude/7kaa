//Filename    : OSFRMRES.CPP
//Description : Object Sprite Frame Resource

#include <ALL.H>
#include <OSTR.H>
#include <OSYS.H>
#include <OGAMESET.H>
#include <OSFRMRES.H>

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
		delete sprite_frame_array;

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

		spriteFrame->offset_x = m.atoi(frameRec->offset_x, frameRec->OFFSET_LEN);
		spriteFrame->offset_y = m.atoi(frameRec->offset_y, frameRec->OFFSET_LEN);

		spriteFrame->width  = m.atoi(frameRec->width , frameRec->WIDTH_LEN);
		spriteFrame->height = m.atoi(frameRec->height, frameRec->HEIGHT_LEN);

		memcpy( &spriteFrame->bitmap_offset, frameRec->bitmap_offset, sizeof(long) );
	}
}
//-------- End of function SpriteFrameRes::load_info ---------//


#ifdef DEBUG

//-------- Begin of function SpriteFrameRes::operator[] -------//

SpriteFrame* SpriteFrameRes::operator[](int recNo)
{
	if( recNo<1 || recNo>sprite_frame_count )
		err.run( "SpriteFrameRes::operator[]" );

	return sprite_frame_array+recNo-1;
}

//--------- End of function SpriteFrameRes::operator[] --------//

#endif
