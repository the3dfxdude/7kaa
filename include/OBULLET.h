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

//Filename    : OBULLET.H
//Description : Header file of Object Bullet
//Owner		  : Alex

#ifndef __OBULLET_H
#define __OBULLET_H

#ifndef __OSPRITE_H
#include <OSPRITE.h>
#endif

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//------ define the parent type ------//
enum	{	BULLET_BY_UNIT = 1,
			BULLET_BY_FIRM,
		};

//----------- Define class Bullet -----------//

class Unit;
struct BulletCrc;

#pragma pack(1)
class Bullet : public Sprite
{
public:
	char	parent_type;
	short	parent_recno;

	//char	mobile_type;			// mobile type of the bullet
	char	target_mobile_type;
	float attack_damage;
	short damage_radius;
	short nation_recno;
	char	fire_radius;

	short	origin_x, origin_y;
	short target_x_loc, target_y_loc;
	char  cur_step, total_step;

public:
	Bullet();

	virtual void 	init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType);
	void 	process_move();
	int	process_die();
	void 	hit_target(short x, short y);
	void 	hit_building(short x, short y);
	void 	hit_wall(short x, short y);
	float	attenuated_damage(short curX, short curY);
	int	check_hit();
	int	warn_target();
	virtual char display_layer();

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

	virtual int	write_derived_file(File* filePtr);
	virtual int	read_derived_file(File* filePtr);

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(BulletCrc *c);
};
#pragma pack()

//------- Define class BulletArray ---------//

class BulletArray : public SpriteArray
{
public:
	BulletArray(int initArraySize);

	int	create_bullet(short spriteId, Bullet** =NULL);

	short add_bullet(Unit* parentUnit, Unit* targetUnit);		// unit attacks unit
	short add_bullet(Unit* parentUnit, short xLoc, short yLoc);	// unit attacks firm, town
	short add_bullet(Firm* parentFirm, Unit* targetUnit);		// firm attacks unit
	short add_bullet(Firm* parentFirm, Firm* targetFirm);		// firm attacks firm

	int	add_bullet_possible(short startXLoc, short startYLoc, char attackerMobileType,
									  short targetXLoc, short targetYLoc, char targetMobileType,
									  short targetWidth, short targetHeight, short& resultXLoc, short& resultYLoc,
									  char bulletSpeed, short bulletSpriteId);
	int	bullet_path_possible(short startXLoc, short startYLoc, char attackerMobileType,
										short destXLoc, short destYLoc, char targetMobileType,
										char bulletSpeed, short bulletSpriteId);

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

	int	bullet_class_size(int spriteId);

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		Bullet* operator[](int recNo);
	#else
		Bullet* operator[](int recNo)   { return (Bullet*) get_ptr(recNo); }
	#endif
};

extern BulletArray bullet_array;

//-----------------------------------------//

#endif

