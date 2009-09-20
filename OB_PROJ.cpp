// Filename   : OB_PROJ.CPP
// Description : header file for Projectile
// Owner       : Gilbert


#include <OB_PROJ.h>


// the height of projectile is as follow :
// z = z_coff * (cur_step+1) * (total_step+1 - cur_step ));

// the gradient of projectile is
// dz/d(cur_step) = z_coff * (total_step - 2 * (cur_step) );
// if dz/d(cur_step) >= 10.0 use upward frame,
// if dz/d(cur_step) <= -10.0 use downward frame,

// actual bullet (x,y,z) is drawn at (x, y - z)
// shadow is drawn at set_cur(x - z/8, y - z/6);


// --------- Begin of function Projectile::Projectile --------//
Projectile::Projectile() : Bullet(), act_bullet(), bullet_shadow()
{
	act_bullet.sprite_recno = 0;
	bullet_shadow.sprite_recno = 0;
}
// --------- End of function Projectile::Projectile --------//


// --------- Begin of function Projectile::~Projectile --------//
Projectile::~Projectile()
{
	deinit();
}
// --------- End of function Projectile::~Projectile --------//


// --------- Begin of function Projectile::init --------//
//### begin alex 3/5 ###//
void Projectile::init(char parentType, short parentRecno, short targetXLoc, short targetYLoc, char targetMobileType)
{
	Bullet::init(parentType, parentRecno, targetXLoc, targetYLoc, targetMobileType);
//#### end alex 3/5 ####//
	short spriteId = sprite_info->get_sub_sprite_info(1)->sprite_id;
	act_bullet.init( spriteId, cur_x_loc(), cur_y_loc() );
	short shadowSpriteId = sprite_info->get_sub_sprite_info(2)->sprite_id;
	bullet_shadow.init( shadowSpriteId, cur_x_loc(), cur_y_loc() );

	// calculate z_coff;
	z_coff = (float)1.0;
	/*
	float dz = z_coff * total_step;
	if( dz >= 10.0)
		cur_dir = cur_dir & 7 | 8;					// pointing up
	else if( dz <= -10.0)
		cur_dir = cur_dir & 7 | 16;				// pointing down
	else
		cur_dir &= 7;
	*/
	
	// --------- recalcuate spriteFrame pointer ----------//
	SpriteFrame* spriteFrame = cur_sprite_frame();
}
// --------- End of function Projectile::init --------//


// --------- Begin of function Projectile::deinit --------//
void Projectile::deinit()
{
}
// --------- End of function Projectile::deinit --------//


// --------- Begin of function Projectile::draw --------//
void Projectile::draw()
{
	short z = short(z_coff * (cur_step+1) * (total_step+1 - cur_step ));
	if(z < 0)
		z = 0;

	// does not draw itself but the shadow and the actual bullet
	if( cur_action == SPRITE_MOVE )
	{
		// -------- update cur_frame ----------//
		float dz = z_coff * (total_step - 2 * (cur_step) );
		if( dz >= 10.0)
			final_dir = (final_dir & 7 )| 8;					// pointing up
		else if( dz <= -10.0)
			final_dir = (final_dir & 7 )| 16;				// pointing down
		else
			final_dir &= 7;

		// ------- update bullet_shadow coordinate --------//
		err_when((bullet_shadow.sprite_info)->need_turning);
		bullet_shadow.set_dir(final_dir);
		bullet_shadow.cur_frame = cur_frame;
		bullet_shadow.cur_action = SPRITE_MOVE;
		bullet_shadow.set_cur(cur_x - z / 8, cur_y - z / 6);

		// ------- update act_bullet coordinate --------//
		err_when((act_bullet.sprite_info)->need_turning);
		act_bullet.set_dir(final_dir);
		act_bullet.cur_frame = cur_frame;
		act_bullet.cur_action = SPRITE_MOVE;
		act_bullet.set_cur(cur_x, cur_y - z);
		
		bullet_shadow.draw();
		act_bullet.draw();
		
		// restore cur_dir
		set_dir((final_dir&7));
	}
	else
	{
		// ###### begin Gilbert 17/10 #########//
		char finalDirBackup = final_dir;
		if( cur_action == SPRITE_DIE )
			set_dir((final_dir & 7) | 16);		// downward direction
		Bullet::draw();
		if( cur_action == SPRITE_DIE )
			set_dir(finalDirBackup);		// downward direction
		// ###### end Gilbert 17/10 #########//
	}
}
// --------- End of function Projectile::draw --------//


// --------- Begin of function Projectile::display_layer --------//
char Projectile::display_layer()
{
	//### begin alex 3/5 ###//
	if(mobile_type == UNIT_AIR || target_mobile_type==UNIT_AIR)
	//#### end alex 3/5 ####//
		return 8;
	else
		return 2;
}
// --------- End of function Projectile::display_layer --------//
