// Filename    : OB_PROJ.H
// Description : header file for Projectile

#ifndef __OB_PROJ_H
#define __OB_PROJ_H

#include <OBULLET.h>

#pragma pack(1)
class Projectile : public Bullet
{
public:
	float z_coff;			// height = z_coff * (cur_step) * (total_step - cur_step)
	Sprite	act_bullet;
	Sprite	bullet_shadow;

public:
	Projectile();
	~Projectile();
	void	init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType); // virtual function from obullet.h
	void	deinit();
	char	display_layer();
	void	draw();

	// int	write_derived_file(File *);
	int	read_derived_file(File *);

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();
};
#pragma pack()

#endif
