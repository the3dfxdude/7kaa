// Filename    : OU_CART.CPP
// Description : Explosive Cart

#include <OU_CART.h>


// --------- define constant --------//

#define EXPLODE_RANGE 1
#define EXPLODE_DAMAGE 50
#define CHAIN_TRIGGER_RANGE 2


UnitExpCart::UnitExpCart() : Unit()
{
	triggered = 0;
}

UnitExpCart::~UnitExpCart()
{
}


int UnitExpCart::process_die()
{
	if(triggered && cur_frame == 3)
	{
		short x, y;
		short x1 = next_x_loc();
		short x2 = x1;
		short y1 = next_y_loc();
		short y2 = y1;
		x1 -= CHAIN_TRIGGER_RANGE;
		x2 += CHAIN_TRIGGER_RANGE;
		y1 -= CHAIN_TRIGGER_RANGE;
		y2 += CHAIN_TRIGGER_RANGE;
		if(x1 < 0)
			x1 = 0;
		if(x2 >= world.max_x_loc)
			x2 = world.max_x_loc-1;
		if(y1 < 0)
			y1 = 0;
		if(y2 >= world.max_y_loc)
			y2 = world.max_y_loc-1;

		for( y = y1; y <= y2; ++y)
		{
			for( x = x1; x <= x2; ++x)
			{
				Location *locPtr = world.get_loc(x,y);
				if( locPtr->has_unit(UNIT_LAND) )
				{
					Unit *unitPtr = unit_array[locPtr->unit_recno(UNIT_LAND)];
					if( unitPtr->unit_id == UNIT_EXPLOSIVE_CART )
						((UnitExpCart *)unitPtr)->trigger_explode();
				}
			}
		}
	}

	if(triggered && (cur_frame == 3 || cur_frame == 7) )
	{
		short x, y;
		short x1 = next_x_loc();
		short x2 = x1;
		short y1 = next_y_loc();
		short y2 = y1;
		x1 -= EXPLODE_RANGE;
		x2 += EXPLODE_RANGE;
		y1 -= EXPLODE_RANGE;
		y2 += EXPLODE_RANGE;

		if(x1 < 0)
			x1 = 0;
		if(x2 >= world.max_x_loc)
			x2 = world.max_x_loc-1;
		if(y1 < 0)
			y1 = 0;
		if(y2 >= world.max_y_loc)
			y2 = world.max_y_loc-1;

		if( cur_frame == 3)
		{
			for( y = y1; y <= y2; ++y)
			{
				for( x = x1; x <= x2; ++x)
				{
					Location *locPtr = world.get_loc(x,y);
					if( locPtr->has_unit(UNIT_LAND) )
					{
						hit_target(this, unit_array[locPtr->unit_recno(UNIT_LAND)], (float) EXPLODE_DAMAGE);
					}
					else if( locPtr->has_unit(UNIT_SEA) )
					{
						hit_target(this, unit_array[locPtr->unit_recno(UNIT_SEA)], (float) EXPLODE_DAMAGE);
					}
					else if( locPtr->is_wall() )
					{
						hit_wall(this, x, y, (float) EXPLODE_DAMAGE);
					}
					else if( locPtr->is_plant() )
					{
						locPtr->remove_plant();
						world.plant_count--;
					}
					else
					{
						hit_building(this, x, y, (float) EXPLODE_DAMAGE);
					}
				}
			}
		}
		else if(cur_frame == 7)
		{
			for( y = y1; y <= y2; ++y)
			{
				for( x = x1; x <= x2; ++x)
				{
					Location *locPtr = world.get_loc(x,y);
					// ##### begin Gilbert 30/10 ######//
					int fl = (abs(x - next_x_loc()) + abs(y - next_y_loc())) * -30 + 80;
					if( locPtr->can_set_fire() && locPtr->fire_str() < fl )
						locPtr->set_fire_str(fl);
					if( locPtr->fire_src() > 0 )
						locPtr->set_fire_src(1);		// such that the fire will be put out quickly
					// ##### end Gilbert 30/10 ######//
				}
			}
		}
	}

	return Unit::process_die();
}


void UnitExpCart::trigger_explode()
{
	if( hit_points > 0)	// so dying cart cannot be triggered
	{
		triggered = 1;
		hit_points = (float) 0;
	}
}