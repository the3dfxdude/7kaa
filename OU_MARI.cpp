// Filename    : OU_MARI.CPP
// Description : sea unit

#include <OSYS.h>
#include <OTERRAIN.h>
#include <OU_CARA.h>
#include <OPOWER.h>
#include <OU_MARI.h>
#include <OREMOTE.h>
#include <ONATIONA.h>
#include <OCONFIG.h>
#ifdef DEBUG2
#include <OFONT.h>
#endif

//------- Define constant ----------//

const int WAVE_CYCLE = 8;

//------- Begin of function UnitMarine::UnitMarine -------//

UnitMarine::UnitMarine()
{
	menu_mode  = 0;
	extra_move_in_beach = NO_EXTRA_MOVE;
	in_beach = 0;
	selected_unit_id = 0;

	//------- transporting units vars ---------//

	unit_count = 0;
	memset(unit_recno_array, 0, sizeof(short)*MAX_UNIT_IN_SHIP);

	//------- transporting goods vars ---------//

	memset( stop_array, 0, MAX_STOP_FOR_SHIP * sizeof(ShipStop) );

	journey_status		= ON_WAY_TO_FIRM;
	dest_stop_id		= 0;
	stop_defined_num	= 0;
	wait_count			= 0;
	stop_x_loc			= 0;
	stop_y_loc			= 0;

	memset(raw_qty_array, 0, sizeof(short)*MAX_RAW);
	memset(product_raw_qty_array, 0, sizeof(short)*MAX_PRODUCT);

	auto_mode			= 1;	// there should be no button to toggle it if the ship is only for trading
	cur_firm_recno		= 0;
}
//------- End of function UnitMarine::UnitMarine -------//


//------- Begin of function UnitMarine::~UnitMarine -------//

UnitMarine::~UnitMarine()
{
	//-------- del those units in the ship -------//

	for(int i=0; i<unit_count; i++)
	{
		if( !unit_array.SpriteArray::is_deleted(unit_recno_array[i]) )
			unit_array.del(unit_recno_array[i]);
	}
}
//------- End of function UnitMarine::~UnitMarine -------//


//------- Begin of function UnitMarine::init -------//

void UnitMarine::init(int unitId, int nationRecno, int rankId, int unitLoyalty, int startX, int startY)
{
	attack_mode_selected = 0;       // for fix_attack_info() to set attack_info_array

	Unit::init(unitId, nationRecno, rankId, unitLoyalty, startX, startY);

	short spriteId = sprite_info->get_sub_sprite_info(1)->sprite_id;
	splash.init( spriteId, cur_x_loc(), cur_y_loc() );
	splash.cur_frame = 1;

	//------- set carry_goods_capacity -------//

	carry_goods_capacity = unit_res[unitId]->carry_goods_capacity;

	//------- set menu mode of the unit -------//

	UnitInfo* unitInfo = unit_res[unitId];

	if( unitInfo->carry_unit_capacity==0 && unitInfo->carry_goods_capacity>0 )		// if this ship only carries goods
		menu_mode = SHIP_MENU_GOODS;
	else
		menu_mode = SHIP_MENU_UNIT;
}
//------- End of function UnitMarine::init -------//


//------- Begin of function UnitMarine::update_abs_pos ------//

void UnitMarine::update_abs_pos(SpriteFrame *spriteFrame)
{
	Unit::update_abs_pos(spriteFrame);
	short h = wave_height(6);
	abs_y1 -= h;
	abs_y2 -= h;
}
//------- End of function UnitMarine::update_abs_pos -------//


//------- Begin of function UnitMarine::draw -------//

void UnitMarine::draw()
{
	// -------- update splash parameter --------//
	// ###### begin Gilbert 8/9 #######//
	char oldSplashAction = splash.cur_action;

	switch(cur_action)
	{
	case SPRITE_MOVE:
		splash.cur_action = SPRITE_MOVE;
		if( splash.cur_action != oldSplashAction)
			splash.cur_frame = 1;
		else
		{
			++splash.cur_frame;
			if( splash.cur_frame < 1 || splash.cur_frame > splash.cur_sprite_move()->frame_count )
				splash.cur_frame = 1;
		}
		break;

	default:
		splash.cur_action = SPRITE_IDLE;
		if( splash.cur_action != oldSplashAction)
			splash.cur_frame = 1;
		else
		{
			++splash.cur_frame;
			if( splash.cur_frame < 1 || splash.cur_frame > splash.cur_sprite_stop()->frame_count)
				splash.cur_frame = 1;
		}
		break;
	}
	// ###### end Gilbert 8/9 #######//

	splash.cur_x = cur_x;
	splash.cur_y = cur_y - wave_height(7);
	splash.cur_dir = cur_dir;
	splash.final_dir = final_dir;
	splash.turn_delay = turn_delay;

	// --------- draw splash and then the unit --------//
	// ###### begin Gilbert 24/9 #######//
	if( cur_action != SPRITE_DIE )
		splash.draw();
	// ###### end Gilbert 24/9 #######//
	Unit::draw();

	#ifdef DEBUG2
		if(selected_flag && 0)
		{
			vga.d3_panel_up( INFO_X1, INFO_Y1+144, INFO_X2, INFO_Y1+144+87 );

			int 	 x=INFO_X1+4, y=INFO_Y1+200, refreshFlag=INFO_REPAINT;
			font_san.field( x, y, " " , x+2, sprite_recno, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+20, y, " " , x+22, next_x_loc(), 1, INFO_X2-2, refreshFlag);
			font_san.field( x+50, y, " " , x+52, next_y_loc(), 1, INFO_X2-2, refreshFlag);
			font_san.field( x+70, y, " " , x+72, nation_recno, 1, INFO_X2-2, refreshFlag);

			font_san.field( x+100, y, " " , x+102, action_mode, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+120, y, " " , x+122, action_para, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+140, y, " " , x+142, action_x_loc, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+160, y, " " , x+162, action_y_loc, 1, INFO_X2-2, refreshFlag);
			y-=20;
			font_san.field( x+100, y, " " , x+102, action_mode2, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+120, y, " " , x+122, action_para2, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+140, y, " " , x+142, action_x_loc2, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+160, y, " " , x+162, action_y_loc2, 1, INFO_X2-2, refreshFlag);
			y-=20;
			font_san.field( x+160, y, " " , x+162, cur_action, 1, INFO_X2-2, refreshFlag);
		}
	#endif
}
//------- End of function UnitMarine::draw -------//


//------- Begin of function UnitMarine::draw_outlined -------//

void UnitMarine::draw_outlined()
{
	// -------- update splash parameter --------//
	// ###### begin Gilbert 8/9 #######//
	char oldSplashAction = splash.cur_action;

	switch(cur_action)
	{
	case SPRITE_MOVE:
		splash.cur_action = SPRITE_MOVE;
		if( splash.cur_action != oldSplashAction)
			splash.cur_frame = 1;
		else
		{
			++splash.cur_frame;
			if( splash.cur_frame < 1 || splash.cur_frame > splash.cur_sprite_move()->frame_count )
				splash.cur_frame = 1;
		}
		break;

	default:
		splash.cur_action = SPRITE_IDLE;
		if( splash.cur_action != oldSplashAction)
			splash.cur_frame = 1;
		else
		{
			++splash.cur_frame;
			if( splash.cur_frame < 1 || splash.cur_frame > splash.cur_sprite_stop()->frame_count)
				splash.cur_frame = 1;
		}
		break;
	}
	// ###### end Gilbert 8/9 #######//

	splash.cur_x = cur_x;
	splash.cur_y = cur_y - wave_height(7);
	splash.cur_dir = cur_dir;
	splash.final_dir = final_dir;
	splash.turn_delay = turn_delay;

	// --------- draw splash and then the unit --------//
	// ###### begin Gilbert 24/9 #######//
	if( cur_action != SPRITE_DIE )
		splash.draw();
	// ###### end Gilbert 24/9 #######//
	Unit::draw_outlined();

	#ifdef DEBUG2
		if(selected_flag && 0)
		{
			vga.d3_panel_up( INFO_X1, INFO_Y1+144, INFO_X2, INFO_Y1+144+87 );

			int 	 x=INFO_X1+4, y=INFO_Y1+200, refreshFlag=INFO_REPAINT;
			font_san.field( x, y, " " , x+2, sprite_recno, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+20, y, " " , x+22, next_x_loc(), 1, INFO_X2-2, refreshFlag);
			font_san.field( x+50, y, " " , x+52, next_y_loc(), 1, INFO_X2-2, refreshFlag);
			font_san.field( x+70, y, " " , x+72, nation_recno, 1, INFO_X2-2, refreshFlag);

			font_san.field( x+100, y, " " , x+102, action_mode, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+120, y, " " , x+122, action_para, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+140, y, " " , x+142, action_x_loc, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+160, y, " " , x+162, action_y_loc, 1, INFO_X2-2, refreshFlag);
			y-=20;
			font_san.field( x+100, y, " " , x+102, action_mode2, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+120, y, " " , x+122, action_para2, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+140, y, " " , x+142, action_x_loc2, 1, INFO_X2-2, refreshFlag);
			font_san.field( x+160, y, " " , x+162, action_y_loc2, 1, INFO_X2-2, refreshFlag);
			y-=20;
			font_san.field( x+160, y, " " , x+162, cur_action, 1, INFO_X2-2, refreshFlag);
		}
	#endif
}
//------- End of function UnitMarine::draw_outlined -------//


//------- Begin of function UnitMarine::wave_height -------//

short UnitMarine::wave_height(int phase)
{
	err_when( phase < 0);
	static short height[WAVE_CYCLE] = { 4,3,2,1,0,1,2,3 };
	return height[((sys.frame_count /4) + phase) % WAVE_CYCLE];
}
//------- End of function UnitMarine::wave_height -------//


//--------- Begin of function UnitMarine::del_unit ---------//
//
// Delete a unit from the ship. This function is called by
// Unit::deinit() when the unit is killed.
//
// <int> unitRecno - recno of the unit to be loaded.
//
void UnitMarine::del_unit(int unitRecno)
{
	for( int i=0 ; i<unit_count ; i++ )
	{
		if( unit_recno_array[i] == unitRecno )
		{
			err_when( unit_count > MAX_UNIT_IN_SHIP );

			m.del_array_rec(unit_recno_array, unit_count, sizeof(unit_recno_array[0]), i+1);
			return;
		}
	}

	err_here();
}
//----------- End of function UnitMarine::del_unit -----------//


//--------- Begin of function UnitMarine::load_unit ---------//
//
// Load an unit to the ship.
//
// <int> unitRecno - recno of the unit to be loaded.
//
void UnitMarine::load_unit(int unitRecno)
{
	if(unit_array.is_deleted(unitRecno))
		return;

	Unit *unitPtr = unit_array[unitRecno];

	if(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode2==ACTION_DIE)
		return;

	if( unit_count == MAX_UNIT_IN_SHIP )
		return;

	unit_recno_array[unit_count++] = unitRecno;

	unitPtr->set_mode(UNIT_MODE_ON_SHIP, sprite_recno);	// set unit mode

	if(unitPtr->selected_flag)
	{
		unitPtr->selected_flag = 0;
		unit_array.selected_count--;
	}
	unitPtr->deinit_sprite();

	//--- if this marine unit is currently selected ---//

	if(unit_array.selected_recno==sprite_recno)
	{
		if(!remote.is_enable() || nation_recno==nation_array.player_recno || config.show_ai_info)
			disp_info(INFO_UPDATE);
	}
}
//----------- End of function UnitMarine::load_unit -----------//


//--------- Begin of function UnitMarine::unload_unit ---------//
//
// Unload an unit from the ship.
//
// <int> unitSeqId - sequence id. of the unit in unit_recno_array[]
//
void UnitMarine::unload_unit(int unitSeqId, char remoteAction)
{
	err_when(unitSeqId > unit_count);

	if(!remoteAction && remote.is_enable() )
	{
		// packet structure : <unit recno> <unitSeqId>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_U_SHIP_UNLOAD_UNIT, 2*sizeof(short));
		*shortPtr = sprite_recno;
		shortPtr[1] = unitSeqId;
		return;
	}

	//-------- unload unit now -------//

	if( unloading_unit(0, unitSeqId-1) ) 	// unit is unloaded
	{
		err_when( unit_count+1 > MAX_UNIT_IN_SHIP );

		m.del_array_rec(unit_recno_array, unit_count+1, sizeof(unit_recno_array[0]), unitSeqId);
	}
}
//----------- End of function UnitMarine::unload_unit -----------//


//--------- Begin of function UnitMarine::unload_all_units ---------//
void UnitMarine::unload_all_units(char remoteAction)
{
	if(!remoteAction && remote.is_enable() )
	{
		// packet structure : <unit recno>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_U_SHIP_UNLOAD_ALL_UNITS, sizeof(short));
		*shortPtr = sprite_recno;
		return;
	}

	unloading_unit(1);	// unload all units
}
//----------- End of function UnitMarine::unload_all_units -----------//


//--------- Begin of function UnitMarine::unloading_unit ---------//
//
//	<int> isAll			- 1 for unload all the units
//							- otherwise 0
// <int>	unitSeqId	- if(isAll==0) unitSeqId+1 is the recno of the
//							  selected unit in unit_recno_array[]
//
int UnitMarine::unloading_unit(int isAll, int unitSeqId)
{
	if( !is_on_coast() )
		return 0;

	//-------------------------------------------------------------------------//
	// return if no territory is nearby the ship
	//-------------------------------------------------------------------------//

	int curXLoc = next_x_loc();	// ship location
	int curYLoc = next_y_loc();
	int unprocess = isAll ? unit_count : 1;
	Unit *unitPtr = isAll ? unit_array[unit_recno_array[unprocess-1]] : unit_array[unit_recno_array[unitSeqId]];
	Location *locPtr;
	int xShift, yShift, checkXLoc, checkYLoc;
	int regionId = 0; // unload all the units in the same territory
	int found, i = 2;
	int sqtSize = 3, sqtArea = sqtSize*sqtSize;

	#ifdef DEBUG
		long debugCount = 0L;
	#endif

	if(isAll &&  nation_recno == nation_array.player_recno )		// for player's camp, patrol() can only be called when the player presses the button.
		power.reset_selection();

	while(unprocess) // using the calculated 'i' to reduce useless calculation
	{
		err_when(debugCount++ > 4*long(MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC));

		m.cal_move_around_a_point(i, MAX_WORLD_X_LOC, MAX_WORLD_Y_LOC, xShift, yShift);
		checkXLoc = curXLoc+xShift;
		checkYLoc = curYLoc+yShift;
		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
		{
			i++;
			continue;
		}
		
		locPtr = world.get_loc(checkXLoc, checkYLoc);
	
		//-------------------------------------------------------------------------//
		// check for space to unload the unit
		//-------------------------------------------------------------------------//
		if(!regionId || locPtr->region_id == regionId)
		{
			if(locPtr->walkable())
				found = 1;

			if(locPtr->can_move(UNIT_LAND))//unitPtr->mobile_type))
			{
				regionId = locPtr->region_id;

				unitPtr->init_sprite(checkXLoc, checkYLoc);
				unitPtr->set_mode(0);

				if( isAll && nation_recno == nation_array.player_recno )		// for player's camp, patrol() can only be called when the player presses the button.
				{
					unitPtr->selected_flag = 1; // mark selected if unload all
					unit_array.selected_count++;

					if( !unit_array.selected_recno )
						unit_array.selected_recno = unitPtr->sprite_recno;
				}

				unprocess--;
				unit_count--;

				if(unprocess)
					unitPtr = unit_array[unit_recno_array[unprocess-1]]; // point to next unit
				else
					break;	// finished, all have been unloaded
			}
		}

		//-------------------------------------------------------------------------//
		// stop checking if there is totally bouned by unacessible location
		//-------------------------------------------------------------------------//
		if(i==sqtArea)
		{
			if(found)
			{
				found = 0;	// reset found
				sqtSize += 2;
				sqtArea = sqtSize*sqtSize;
			}
			else // no continuous location for the unit to unload, some units can't be unloaded
				return 0;
		}

		i++;
	}

	//-------- display info --------//

	if( nation_recno == nation_array.player_recno )		// for player's camp, patrol() can only be called when the player presses the button.
		info.disp();

	return 1;
}
//----------- End of function UnitMarine::unloading_unit -----------//


//--------- Begin of function UnitMarine::is_on_coast ---------//
//
// Return whether the unit is on the coast and ready for unloading.
//
int UnitMarine::is_on_coast()
{
	Location *locPtr;
	int 		xShift, yShift, checkXLoc, checkYLoc, found=0;
	int 		curXLoc = next_x_loc();	// ship location
	int 		curYLoc = next_y_loc();

	for(int i=2; i<=9; i++) // checking for the surrouding location
	{
		m.cal_move_around_a_point(i, 3, 3, xShift, yShift);

		checkXLoc = curXLoc + xShift;
		checkYLoc = curYLoc + yShift;

		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
			continue;

		locPtr = world.get_loc(checkXLoc, checkYLoc);

		if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN && // a territory nearby
			locPtr->walkable())
		{
			return 1;
		}
	}

	return 0;
}
//----------- End of function UnitMarine::is_on_coast -----------//


//--------- Begin of function UnitMarine::extra_move ---------//
void UnitMarine::extra_move()
{
	static char offset[3] = {0, 1, -1};

	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	
	int vecX = action_x_loc2 - curXLoc;
	int vecY = action_y_loc2 - curYLoc;
	int checkXLoc, checkYLoc, i, found=0;

	if(vecX==0 || vecY==0)
	{
		if(vecX==0)
		{
			vecY /= abs(vecY);
			checkYLoc = curYLoc + vecY;
		}
		else // vecY==0
		{
			vecX /= abs(vecX);
			checkXLoc = curXLoc + vecX;
		}

		for(i=0; i<3; i++)
		{
			if(vecX==0)
				checkXLoc = curXLoc + offset[i];
			else
				checkYLoc = curYLoc + offset[i];

			if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
				continue;

			if(world.get_loc(checkXLoc, checkYLoc)->can_move(mobile_type))
			{
				found++;
				break;
			}
		}
	}
	else
	{
		vecX /= abs(vecX);
		vecY /= abs(vecY);
		checkXLoc = curXLoc + vecX;
		checkYLoc = curYLoc + vecY;

		if(world.get_loc(checkXLoc, checkYLoc)->can_move(mobile_type))
			found++;
	}

	if(!found)
		return;

	set_dir(curXLoc, curYLoc, checkXLoc, checkYLoc);
	cur_action = SPRITE_SHIP_EXTRA_MOVE;
	go_x = checkXLoc*ZOOM_LOC_WIDTH;
	go_y = checkYLoc*ZOOM_LOC_HEIGHT;
	err_when(cur_x==go_x && cur_y==go_y);
	//extra_move_in_beach = EXTRA_MOVING_IN;
}
//----------- End of function UnitMarine::extra_move -----------//


//------- Begin of function UnitMarine::process_extra_move ------//

void UnitMarine::process_extra_move()
{
	static short vector_x_array[] = { 0,  1, 1, 1, 0, -1, -1, -1};	// default vectors, temporary only
	static short vector_y_array[] = {-1, -1, 0, 1, 1,  1,  0, -1};

	if(!match_dir()) // process turning
		return;

	if(cur_x!=go_x || cur_y!=go_y)
	{
		//------------------------------------------------------------------------//
		// set cargo_recno, extra_move_in_beach
		//------------------------------------------------------------------------//
		if(cur_x==next_x && cur_y==next_y)
		{
			int goXLoc = go_x>>ZOOM_X_SHIFT_COUNT;
			int goYLoc = go_y>>ZOOM_Y_SHIFT_COUNT;
			if(!world.get_loc(goXLoc, goYLoc)->can_move(mobile_type))
			{
				go_x = next_x;
				go_y = next_y;
				return;
			}

			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();
			world.set_unit_recno(curXLoc, curYLoc, mobile_type, 0);
			world.set_unit_recno(goXLoc, goYLoc, mobile_type, sprite_recno);
			next_x = go_x;
			next_y = go_y;
			
			err_when( ((curXLoc%2)|(curYLoc%2)) + ((goXLoc%2)|(goYLoc%2)) != 1); // one pair location must be even, another is not even
			in_beach = !(curXLoc%2 || curYLoc%2);
			
			if(goXLoc%2 || goYLoc%2) // not even location
				extra_move_in_beach = EXTRA_MOVING_IN;
			else // even location
				extra_move_in_beach = EXTRA_MOVING_OUT;
		}
		//else
		//	int debug = 0;
		
		//---------- process moving -----------//
		short stepX = sprite_info->speed;
		short stepY = sprite_info->speed;
		short vectorX = vector_x_array[final_dir] * sprite_info->speed;	// cur_dir may be changed in the above set_next() call
		short vectorY = vector_y_array[final_dir] * sprite_info->speed;

		if(abs(cur_x-go_x) <= stepX)
			cur_x = go_x;
		else
			cur_x += vectorX;

		if(abs(cur_y-go_y) <= stepY)
			cur_y = go_y;
		else
			cur_y += vectorY;

		err_when(extra_move_in_beach!=EXTRA_MOVING_IN && extra_move_in_beach!=EXTRA_MOVING_OUT);
		err_when(cur_action!=SPRITE_SHIP_EXTRA_MOVE);
	}

	if(cur_x==go_x && cur_y==go_y)
	{
		if(result_node_array==NULL)
		{
			cur_action = SPRITE_IDLE;
			cur_frame  = 1;
			move_to_x_loc = next_x_loc();
			move_to_y_loc = next_y_loc();
		}
		else
		{
			cur_action = SPRITE_MOVE;
			next_move();
		}

		if(in_beach)
		{
			extra_move_in_beach = EXTRA_MOVE_FINISH;
			err_when(move_to_x_loc%2==0 && move_to_y_loc%2==0);
			err_when(result_node_array || result_path_dist);
		}
		else
		{
			extra_move_in_beach = NO_EXTRA_MOVE;
			err_when(move_to_x_loc%2 || move_to_y_loc%2);
			err_when(result_node_array || result_path_dist);
		}

		err_when(cur_action==SPRITE_IDLE && (extra_move_in_beach==EXTRA_MOVING_IN || extra_move_in_beach==EXTRA_MOVING_OUT));
	}
}
//----------- End of function UnitMarine::process_extra_move -----------//


//------------ Begin of function UnitMarine::actual_damage --------------//
//
// This function returns the actual hit damage this unit can do to a target.
//
float UnitMarine::actual_damage()
{
	float attackDamage = Unit::actual_damage();

	//-----------------------------------------//
	//
	// If there is units on the ship, the units
	// leadership will increase the attacking damage.
	//
	// actual damage = normal damage X (100+highest leadership unit on the ship) / 100
	//
	//-----------------------------------------//

	Unit* unitPtr;
	int 	t, highestLeadership=0;

	for( int i=0 ; i<unit_count ; i++ )
	{
		unitPtr = unit_array[ unit_recno_array[i] ];

		if( unitPtr->skill.skill_id == SKILL_LEADING )
		{
			if( (t=unitPtr->skill.skill_level) > highestLeadership )
				highestLeadership = t;
		}
	}

	return attackDamage * (100+highestLeadership) / 100;
}
//------------ End of function UnitMarine::actual_damage --------------//


//------- Begin of function UnitMarine::total_carried_goods -------//

int UnitMarine::total_carried_goods()
{
	int totalQty=0;

	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		totalQty += raw_qty_array[i];
		totalQty += product_raw_qty_array[i];
	}

   return totalQty;
}
//------- End of function UnitMarine::total_carried_goods -------//


//------- Begin of function UnitMarine::can_resign -------//
int UnitMarine::can_resign()
{
	err_when(unit_count < 0);
   return unit_count == 0;
}
//------- End of function UnitMarine::can_resign -------//
