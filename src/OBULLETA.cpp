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

//Filename    : OBULLETA.CPP
//Description : Object Bullet Array
//Owner		  : Alex

#include <OVGA.h>
#include <OUNIT.h>
#include <OBULLET.h>
#include <OWORLD.h>
#include <OTOWN.h>
#include <OB_PROJ.h>
#include <OB_HOMIN.h>
#include <OB_FLAME.h>


//--------- Begin of function BulletArray::BulletArray ---------//
//
// <int> initArraySize - the initial size of this array.
//
BulletArray::BulletArray(int initArraySize) : SpriteArray(initArraySize)
{
}
//----------- End of function BulletArray::BulletArray -----------//


//--------- Begin of function BulletArray::create_bullet ---------//
//
// return: <int> the recno of the bullet created.
//
int BulletArray::create_bullet(short spriteId, Bullet** bpp)
{
	Bullet* bulletPtr;
	SpriteInfo *spriteInfo = sprite_res[spriteId];
	err_when(!spriteInfo);
	err_when(spriteInfo->sprite_type != 'B');
	switch(spriteInfo->sprite_sub_type)
	{
	case 0:
	case ' ':
		bulletPtr = new Bullet;
		break;
	case 'P':
		bulletPtr = new Projectile;
		break;
	case 'H':
		bulletPtr = new BulletHoming;
		break;
	case 'F':
		bulletPtr = new BulletFlame;
		break;
	default:
		err_here();			// undefined bullet type
		if( bpp )
			*bpp = NULL;
		return 0;
	}

	add(bulletPtr);
	if( bpp )
		*bpp = bulletPtr;
	return recno();
}
//----------- End of function BulletArray::create_bullet -----------//


//--------- Begin of function BulletArray::bullet_class_size ---------//
int BulletArray::bullet_class_size(int spriteId)
{
	SpriteInfo *spriteInfo = sprite_res[spriteId];
	err_when(spriteInfo->sprite_type != 'B');
	switch(spriteInfo->sprite_sub_type)
	{
	case 0:
	case ' ':
		return sizeof(Bullet);
	case 'P':
		return sizeof(Projectile);
	case 'H':
		return sizeof(BulletHoming);
	case 'F':
		return sizeof(BulletFlame);
	default:
		err_here();			// undefined bullet type
		return 0;
	}
}
//----------- End of function BulletArray::bullet_class_size -----------//


//--------- Begin of function BulletArray::bullet_path_possible ---------//
// For default bullet:
//		if the bullet is not blocked by building in its path, this
//		function return 1 for success, otherwise return 0.
// For projectile bullet:
//		always return 1
//
int BulletArray::bullet_path_possible(short startXLoc, short startYLoc, char attackerMobileType,
												  short destXLoc, short destYLoc, char targetMobileType,
												  char bulletSpeed, short bulletSpriteId)
{
	if(attackerMobileType==UNIT_AIR || targetMobileType==UNIT_AIR)
		return 1;

	//-------- skip the checking for projectile -----------//
	SpriteInfo *spriteInfo = sprite_res[bulletSpriteId];
	if(spriteInfo->sprite_sub_type == 'P')
		return 1;

	err_when(spriteInfo->sprite_sub_type == 'P');

	//----------------------- define variables ---------------//

	int originX	= startXLoc*ZOOM_LOC_WIDTH;
	int originY	= startYLoc*ZOOM_LOC_HEIGHT;
	int goX		= destXLoc*ZOOM_LOC_WIDTH;
	int goY		= destYLoc*ZOOM_LOC_HEIGHT;

	int xStep	= (goX - originX)/bulletSpeed;
	int yStep	= (goY - originY)/bulletSpeed;

	int totalStep = MAX(1, MAX(abs(xStep), abs(yStep)));
	int curStep = 0;

	//------------------------------------------------------//
	// if the path of the bullet is blocked, return 0
	//------------------------------------------------------//
	int curX = originX + ZOOM_LOC_WIDTH/2;
	int curY = originY + ZOOM_LOC_HEIGHT/2;
	int curXLoc, curYLoc;
	Location *locPtr;

	while(curStep++<totalStep)
	{
		err_when(curStep > 200);

		curX += xStep;
		curY += yStep;

		curXLoc = curX/ZOOM_LOC_WIDTH;
		curYLoc = curY/ZOOM_LOC_HEIGHT;

		if(curXLoc==startXLoc && curYLoc==startYLoc)
			continue;

		if(curXLoc==destXLoc && curYLoc==destYLoc)
			break;	// is destination

		locPtr = world.get_loc(curXLoc, curYLoc);

		//### begin alex 2/6 ###//
		//if(!locPtr->walkable(3) || locPtr->has_unit(UNIT_LAND) || locPtr->has_unit(UNIT_SEA))
		if(!locPtr->walkable(3))
		//#### end alex 2/6 ####//
			return 0;
	}

	return 1;
}
//----------- End of function BulletArray::bullet_path_possible -----------//


//--------- Begin of function BulletArray::add_bullet_possible ---------//
// call bullet_possible to check whether bullets emitted from this position
// can reach the target location with blocking
//
// return 1 if bullets are able to reach the target location
// return 0 if not
//
int BulletArray::add_bullet_possible(short startXLoc, short startYLoc, char attackerMobileType,
												 short targetXLoc, short targetYLoc, char targetMobileType,
												 short targetWidth, short targetHeight, short& resultXLoc, short& resultYLoc,
												 char bulletSpeed, short bulletSpriteId)
{
	resultXLoc = resultYLoc = -1;

	//----------------------------------------------------------------------//
	// for target with size 1x1
	//----------------------------------------------------------------------//
	if(targetWidth==1 && targetHeight==1)
	{
		if(bullet_path_possible(startXLoc, startYLoc, attackerMobileType, targetXLoc, targetYLoc, targetMobileType, bulletSpeed, bulletSpriteId))
		{
			resultXLoc = targetXLoc;
			resultYLoc = targetYLoc;
			return 1;
		}
		else 
			return 0;
	}

	err_when(targetWidth==1 && targetHeight==1);
	//----------------------------------------------------------------------//
	// choose the closest corner to be the default attacking point of range attack
	//
	// generalized case for range-attack is coded below. Work for target with
	// size > 1x1 
	//----------------------------------------------------------------------//

	//-------------- define parameters --------------------//
	short	adjWidth	= targetWidth-1;	// adjusted width;
	short	adjHeight = targetHeight-1; // adjusted height
	short xOffset=0, yOffset=0;	// the 1st(default) location for range attack
	short atEdge = 0;	// i.e. the attacking point is at the corner or edge of the target
							// 1 for at the edge, 0 for at corner
		
	//----------------------------------------------------------------------//
	// determine initial xOffset
	//----------------------------------------------------------------------//
	if(startXLoc <= targetXLoc)
		xOffset = 0; // the left hand side of the target
	else if(startXLoc >= targetXLoc+adjWidth)
		xOffset = adjWidth; // the right hand side of the target
	else
	{
		xOffset = startXLoc - targetXLoc; // in the middle(vertical) of the target
		atEdge++;
	}

	//----------------------------------------------------------------------//
	// determine initial yOffset
	//----------------------------------------------------------------------//
	if(startYLoc <= targetYLoc)
		yOffset = 0;	// the upper of the target
	else if(startYLoc >= targetYLoc+adjHeight)
		yOffset = adjHeight;
	else
	{
		yOffset = startYLoc - targetYLoc; // in the middle(horizontal) of the target
		atEdge++;
	}

	//----------------------------------------------------------------------//
	// checking whether it is possible to add bullet
	//----------------------------------------------------------------------//
	if(bullet_path_possible(startXLoc, startYLoc, attackerMobileType, targetXLoc+xOffset, targetYLoc+yOffset, targetMobileType, bulletSpeed, bulletSpriteId))
	{
		resultXLoc = targetXLoc+xOffset;
		resultYLoc = targetYLoc+yOffset;
		return 1;
	}

	short	leftXOffset, leftYOffset;
	short	rightXOffset, rightYOffset;
	short leftX, leftY, rightX, rightY;
	short found=0;	// found a location to attack the target and the path is not blocked
	short end=0;
	
	if(atEdge)	// only check one edge of the target
	{
		if(xOffset==0 || xOffset==adjWidth) // horizontal edge
		{
			leftYOffset = rightYOffset = 0;
			leftXOffset = 1;
			rightXOffset = -1;
		}
		else if(yOffset==0 || yOffset==adjHeight) // vertical edge
		{
			leftXOffset = rightXOffset = 0;
			leftYOffset = 1;
			rightYOffset = -1;
		}
		else
			err_here();	// the sprite is within the target ???
	}
	else	// at the corner,	need to check two edges of the target
	{
		leftYOffset = rightXOffset = 0;
		leftXOffset = (xOffset==0) ? 1 : -1;
		rightYOffset = (yOffset==0) ? 1 : -1;
	}

	leftX = rightX = xOffset;
	leftY = rightY = yOffset;

	while(!found)
	{
		end = 0;
		
		//-------------------------------------------//
		// for the leftX, leftY
		//-------------------------------------------//
		leftX += leftXOffset;
		leftY += leftYOffset;
		if(leftX>=0 && leftX<targetWidth && leftY>=0 && leftY<targetHeight)
		{
			if(bullet_path_possible(startXLoc, startYLoc, attackerMobileType, targetXLoc+leftX, targetYLoc+leftY, targetMobileType, bulletSpeed, bulletSpriteId))
			{
				resultXLoc = targetXLoc+leftX;
				resultYLoc = targetYLoc+leftY;
				return 1;
			}
		}
		else
			end++;

		//-------------------------------------------//
		// for the rightX, rightY
		//-------------------------------------------//
		rightX += rightXOffset;
		rightY += rightYOffset;
		if(rightX>=0 && rightX<targetWidth && rightY>=0 && rightY<targetHeight)
		{
			if(bullet_path_possible(startXLoc, startYLoc, attackerMobileType, targetXLoc+rightX, targetYLoc+rightY, targetMobileType, bulletSpeed, bulletSpriteId))
			{
				resultXLoc = targetXLoc+rightX;
				resultYLoc = targetYLoc+rightY;
				return 1;
			}
		}
		else
		{
			if(end)	// all locations have been checked, all are blocked
				return 0;
		}
	}

	return 0;
}
//----------- End of function BulletArray::add_bullet_possible -----------//


//--------- Begin of function BulletArray::add_bullet ---------//
// unit attacks unit by bullet
//
// <Unit*> parentUnit - pointer to the attacking target unit
// <Unit*> targetUnit - pointer to the target unit
//
//	return 1 if bullet is added successfully otherwise return 0.
//
short BulletArray::add_bullet(Unit* parentUnit, Unit* targetUnit)
{
	//------------------------------------------------------//
	// define parameters
	//------------------------------------------------------//
	SpriteInfo *targetSpriteInfo = targetUnit->sprite_info;
	AttackInfo *attackInfo = parentUnit->attack_info_array+parentUnit->cur_attack;
	short attackXLoc	= parentUnit->range_attack_x_loc;
	short attackYLoc	= parentUnit->range_attack_y_loc;
	short targetXLoc = targetUnit->next_x_loc();
	short targetYLoc = targetUnit->next_y_loc();

	if(attackXLoc >= targetXLoc && attackXLoc < targetXLoc+targetSpriteInfo->loc_width &&
		attackYLoc >= targetYLoc && attackYLoc < targetYLoc+targetSpriteInfo->loc_height)
	{
		//-------------------------------------------------------//
		// the previous used range attack destination can be reused,
		// time is saved 'cos no need to check for bullet_path_possible()
		//-------------------------------------------------------//
		short bulletId = (parentUnit->attack_info_array+parentUnit->cur_attack)->bullet_sprite_id;
		Bullet* bulletPtr;
		create_bullet( bulletId, &bulletPtr);
		bulletPtr->init(BULLET_BY_UNIT, parentUnit->sprite_recno, attackXLoc, attackYLoc, targetUnit->mobile_type);
		err_when(parentUnit->cur_dir!=parentUnit->final_dir);
		//parentUnit->get_dir(parentUnit->next_x_loc(), parentUnit->next_y_loc(), attackXLoc, attackYLoc);
		return 1;
	}

	err_here();
	return 0;
}
//----------- End of function BulletArray::add_bullet -----------//


//--------- Begin of function Bullet::add_bullet ---------//
// Overload of add_bullet(), unit attack firm/town/wall
//
// <Unit*> parentUnit - pointer to the attacking target unit
// <short> xLoc - the target upper left corner x loc
//					   note: taregt can be firm, town or wall
// <short> yLoc - the target upper left corner y loc
//
//	return 1 if bullet is added successfully otherwise return 0.
//
//	note: unit attacks firm or town by bullet will call this
//			function. The location of the firm or town is
//			specified by xLoc and yLoc.
//
short BulletArray::add_bullet(Unit* parentUnit, short xLoc, short yLoc)
{
	//------------------------------------------------------//
	// define parameters
	//------------------------------------------------------//
	AttackInfo *attackInfo = parentUnit->attack_info_array+parentUnit->cur_attack;
	short	attackXLoc = parentUnit->range_attack_x_loc;
	short	attackYLoc = parentUnit->range_attack_y_loc;
	short	targetXLoc = xLoc;
	short	targetYLoc = yLoc;
	short	width, height;
	Firm*			targetFirm = NULL;
	Town*    	targetTown = NULL;
	FirmInfo*	firmInfo = NULL;
	Location*	locPtr = world.get_loc(xLoc, yLoc);
	
	if(locPtr->is_firm())
	{
		targetFirm = firm_array[locPtr->firm_recno()];
		firmInfo = firm_res[targetFirm->firm_id];
		width = firmInfo->loc_width;
		height = firmInfo->loc_height;
	}
	else if(locPtr->is_town())
	{
		targetTown = town_array[locPtr->town_recno()];
		width = targetTown->loc_width();
		height = targetTown->loc_height();
	}
	else if(locPtr->is_wall())
		width = height = 1;
	else
		err_here();

	if(attackXLoc >= targetXLoc && attackXLoc < targetXLoc+width &&
		attackYLoc >= targetYLoc && attackYLoc < targetYLoc+height)
	{
		//-------------------------------------------------------//
		// the previous used range attack destination can be reused,
		// time is saved 'cos no need to check for bullet_path_possible()
		//-------------------------------------------------------//
		short bulletId = (parentUnit->attack_info_array+parentUnit->cur_attack)->bullet_sprite_id;
		Bullet* bulletPtr;
		create_bullet( bulletId, &bulletPtr);
		bulletPtr->init(BULLET_BY_UNIT, parentUnit->sprite_recno, attackXLoc, attackYLoc, UNIT_LAND);
		err_when(parentUnit->cur_dir!=parentUnit->final_dir);
		//parentUnit->get_dir(parentUnit->next_x_loc(), parentUnit->next_y_loc(), attackXLoc, attackYLoc);
		return 1;
	}

	err_here();
	return 0;
}
//----------- End of function BulletArray::add_bullet -----------//


//--------- Begin of function Bullet::add_bullet ---------//
// Overload of add_bullet()
//
// <Unit*> parentFirm - pointer to the attacking target Firm
// <Unit*> targetUnit - pointer to the target Unit
//
//	return 1 if bullet is added successfully otherwise return 0.
short BulletArray::add_bullet(Firm* parentFirm, Unit* targetUnit)
{
	return 0;
}
//----------- End of function BulletArray::add_bullet -----------//


//--------- Begin of function Bullet::add_bullet ---------//
// Overload of add_bullet()
//
// <Unit*> parentFirm - pointer to the attacking target Firm
// <Unit*> targetFirm - pointer to the target Firm
//
//	return 1 if bullet is added successfully otherwise return 0.
short BulletArray::add_bullet(Firm* parentFirm, Firm* targetFirm)
{
	return 0;
}
//----------- End of function BulletArray::add_bullet -----------//