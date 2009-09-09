// Filename    : OB_FLAME.H
// Description : non-moving bullet attack (flamethrower)


#ifndef __OB_FLAME_H
#define __OB_FLAME_H

#include	<OBULLET.H>

// cur_action of BulletFlame is SPRITE_STOP before die

class BulletFlame : public Bullet
{
public:
	BulletFlame();

	void 	init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType);
	void	process_idle();
	char	display_layer();

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();
};

#endif