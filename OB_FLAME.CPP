// Filename    : OB_FLAME.CPP
// Description : non-moving bullet attack (flamethrower)

#include <OB_FLAME.H>


BulletFlame::BulletFlame() : Bullet()
{
}


//### begin alex 3/5 ###//
void BulletFlame::init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType)
{
	// note : BulletFlame should have at least one dummy moving frame for each direction
	Bullet::init(parentType, parentRecno, targetXLoc, targetYLoc, targetMobileType);
//#### end alex 3/5 ####//

	cur_action = SPRITE_IDLE;
	
	// sound effect here
}


void BulletFlame::process_idle()
{

	// Sprite::process_idle();
	if( ++cur_frame <= cur_sprite_stop()->frame_count )
	{
		// ----- warn/ attack target every frame -------//
		warn_target();
		check_hit();
	}
	else
	{
		cur_action = SPRITE_DIE;
		cur_frame = 1;
	}
}


//--------- Begin of function BulletFlame::display_layer -------//
char BulletFlame::display_layer()
{
	switch(mobile_type)
	{
	case UNIT_LAND:
	case UNIT_SEA:
		return 2;
	case UNIT_AIR:
		return 8;
	default:
		err_here();
		return 1;
	}
}
//--------- End of function BulletFlame::display_layer -------//
