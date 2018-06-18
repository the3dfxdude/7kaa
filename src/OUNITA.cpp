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

//Filename    : OUNITA.CPP
//Description : Object UnitArray

#include <ALL.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OSTR.h>
#include <OSPY.h>
#include <OREMOTE.h>
#include <ONATION.h>
#include <OREBEL.h>
#include <OGODRES.h>
#include <OUNITALL.h>
#include <OWORLD.h>
#include <OCONFIG.h>
#include <OGAME.h>
#include <OANLINE.h>
#include <OFONT.h>
#include <CRC.h>

#ifdef NO_DEBUG_UNIT
#undef err_when
#undef err_here
#undef err_if
#undef err_else
#undef err_now
#define err_when(cond)
#define err_here()
#define err_if(cond)
#define err_else
#define err_now(msg)
#undef DEBUG
#endif

//--------- Declare static functions ---------//

static void put_profile(int y, const char* dispDes, int dispValue);

//--------- Begin of function UnitArray::UnitArray ---------//
//
// <int> initArraySize - the initial size of this array.
//
UnitArray::UnitArray(int initArraySize) : SpriteArray(initArraySize)
{
}
//----------- End of function UnitArray::UnitArray -----------//


//--------- Begin of function UnitArray::init ---------//
//
void UnitArray::init()
{
	//-------- initialize group selection parameter -------//

	cur_group_id = 1;
	cur_team_id  = 1;

	selected_land_unit_count = 0;
	selected_sea_unit_count = 0;
	selected_air_unit_count = 0;
	idle_blocked_unit_reset_count = 0;
	selected_land_unit_array = NULL;
	selected_sea_unit_array = NULL;
	selected_air_unit_array = NULL;

	visible_unit_count = 0;

	//------------ init sprite_array ------------//

	SpriteArray::init();
}
//----------- End of function UnitArray::init -----------//


//--------- Begin of function UnitArray::add_unit ---------//
//
// <int> unitId               - the id. of the unit
// <int> nationRecno          - the recno of the nation that the unit belongs to
// [int] rankId					- rank id. of the unit (none for non-human unit)
// [int] unitLoyalty			   - loyalty of the unit  (none for non-human unit)
// [int] startXLoc, startYLoc - the starting location of the unit
//										  (if startXLoc < 0, this is a unit for hire, and is not a unit of the game yet. init_sprite() won't be called for this unit)
//										  (default: -1, -1)
// [int] remoteAction			- whether this is a remote action or not.
//
// return : <int> - the recno of the unit added.
//
int UnitArray::add_unit(int unitId, int nationRecno, int rankId, int unitLoyalty, int startXLoc, int startYLoc)
{
	//-------- create and initialize Unit -------//

	Unit* unitPtr = create_unit(unitId);

	//----------- add to SpriteArray ------------//

	unitPtr->init(unitId, nationRecno, rankId, unitLoyalty, startXLoc, startYLoc);

	//-------------------------------------------//

	return unitPtr->sprite_recno;
}
//----------- End of function UnitArray::add_unit -----------//


//-------- Begin of function UnitArray::create_unit --------//
//
Unit* UnitArray::create_unit(int unitId)
{
	Unit* 	 unitPtr;
	UnitInfo* unitInfo = unit_res[unitId];

	switch(unitId)
	{
		case UNIT_CARAVAN:
			unitPtr = new UnitCaravan;
			break;

		case UNIT_VESSEL:
		case UNIT_TRANSPORT:
		case UNIT_CARAVEL:
		case UNIT_GALLEON:
			unitPtr = new UnitMarine;
			break;

		case UNIT_EXPLOSIVE_CART:
			unitPtr = new UnitExpCart;
			break;

		default:
			if( unitInfo->is_monster )
				unitPtr = new UnitMonster;

			else if( unitInfo->solider_id )		// if it is a vehicle unit
				unitPtr = new UnitVehicle;

			else if( god_res.is_god_unit(unitId) )
				unitPtr = new UnitGod;

			else
				unitPtr = new Unit;
	}

	SpriteArray::add(unitPtr);			// it set Sprite::sprite_recno

 	return unitPtr;
}
//----------- End of function UnitArray::create_unit ---------//


//-------- Begin of function UnitArray::unit_class_size --------//
//
// Return the size of the class.
//
int UnitArray::unit_class_size(int unitId)
{
	UnitInfo* unitInfo = unit_res[unitId];

	switch(unitId)
	{
		case UNIT_CARAVAN:
			return sizeof(UnitCaravan);

		case UNIT_VESSEL:
		case UNIT_TRANSPORT:
		case UNIT_CARAVEL:
		case UNIT_GALLEON:
			return sizeof(UnitMarine);

		case UNIT_EXPLOSIVE_CART:
			return sizeof(UnitExpCart);

		default:
			if( unitInfo->is_monster )
				return sizeof(UnitMonster);

			else if( unitInfo->solider_id )		// if it is a vehicle unit
				return sizeof(UnitVehicle);

			else if( god_res.is_god_unit(unitId) )
				return sizeof(UnitGod);

			else
				return sizeof(Unit);
	}
}
//----------- End of function UnitArray::unit_class_size ---------//


//-------- Begin of function UnitArray::disappear_in_town --------//
//
// The unit disappear from the map because it has moved into a town.
//
void UnitArray::disappear_in_town(int unitRecno, int townRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	if( unitPtr->unit_mode == UNIT_MODE_REBEL )
		rebel_array.settle_town(unitRecno, townRecno);

	del(unitRecno);		// delete it from unit_array
}
//----------- End of function UnitArray::disappear_in_town ---------//


//-------- Begin of function UnitArray::disappear_in_firm --------//
//
// The unit disappear from the map because it has moved into a town.
//
void UnitArray::disappear_in_firm(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	err_when( unitPtr->unit_mode == UNIT_MODE_REBEL );		// this shouldn't happen

	del(unitRecno);		// delete it from unit_array
}
//----------- End of function UnitArray::disappear_in_firm ---------//


//-------- Begin of function UnitArray::die --------//
//
void UnitArray::die(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	if( unitPtr->unit_mode == UNIT_MODE_REBEL )
		rebel_array.drop_rebel_identity(unitRecno);

	unitPtr->die();

	del(unitRecno);		// delete it from unit_array
}
//----------- End of function UnitArray::die ---------//


//--------- Begin of function UnitArray::draw_dot ---------//
//
// Draw 2x2 tiny squares on map window representing the location of the unit ---//
//
void UnitArray::draw_dot()
{
	char*	  vgaBufPtr = vga_back.buf_ptr();
	char*	  writePtr;
	int	  i, j, mapX, mapY;
	Unit*	  unitPtr;
	char*   nationColorArray = nation_array.nation_color_array;
	char	  nationColor;
	int		vgaBufPitch = vga_back.buf_pitch();
	const unsigned int excitedColorCount = 4;
	char excitedColorArray[MAX_NATION+1][excitedColorCount];
	short playerNationRecno = nation_array.player_recno;
	short lineFromX, lineFromY, lineToX, lineToY;
	int	resultNodeRecno, resultNodeCount;
	ResultNode *resultNode1, *resultNode2;

	for( i = 0; i <= MAX_NATION; ++i )
	{
		if( i == 0 || !nation_array.is_deleted(i) )
		{
			char *remapTable = game.get_color_remap_table(i, 0);
			excitedColorArray[i][0] = remapTable[0xe0];
			excitedColorArray[i][1] = remapTable[0xe1];
			excitedColorArray[i][2] = remapTable[0xe2];
			excitedColorArray[i][3] = remapTable[0xe3];
		}
		else
		{
			excitedColorArray[i][0] = 
			excitedColorArray[i][1] = 
			excitedColorArray[i][2] = 
			excitedColorArray[i][3] = (char) V_WHITE;
		}
	}

	int arraySize = size();
	int lineColor;

	//### begin alex 27/10 ###//
	//------------- set boundary of anim_line to mini-map ------------//
	short animLineBoundX1 = anim_line.bound_x1;
	short animLineBoundY1 = anim_line.bound_y1;
	short animLineBoundX2 = anim_line.bound_x2;
	short animLineBoundY2 = anim_line.bound_y2;
	anim_line.bound_x1 = MAP_X1;
	anim_line.bound_y1 = MAP_Y1;
	anim_line.bound_x2 = MAP_X2;
	anim_line.bound_y2 = MAP_Y2;
	//#### end alex 27/10 ####//

	for(i=1; i<=arraySize; i++)
	{
		unitPtr = (Unit*)get_ptr(i);

		if( !unitPtr || !unitPtr->is_visible() || unitPtr->is_shealth())
			continue;

		if( unitPtr->mobile_type == UNIT_SEA )
			lineColor = V_WHITE;
		else
			lineColor = V_BLACK;

		//---------------- draw unit path in mini_map ------------//
		if(unitPtr->selected_flag && (config.show_unit_path & 2) )
		{
			//#ifdef DEBUG
			//if(config.show_ai_info || unitPtr->nation_recno==playerNationRecno)
			//#else
			//if(unitPtr->nation_recno==playerNationRecno)
			//#endif
			if( config.show_ai_info || !playerNationRecno || unitPtr->is_nation(playerNationRecno) )
			{
				resultNodeRecno = unitPtr->result_node_recno;
				resultNodeCount = unitPtr->result_node_count;
				if(resultNodeCount && resultNodeRecno<=resultNodeCount)
				{
					//-----------------------------------------------------------//
					if(unitPtr->cur_x_loc()!=unitPtr->go_x_loc() || unitPtr->cur_y_loc()!=unitPtr->go_y_loc())
					{
						lineFromX = MAP_X1 + unitPtr->go_x_loc();
						lineFromY = MAP_Y1 + unitPtr->go_y_loc();
						lineToX = MAP_X1 + unitPtr->next_x_loc();
						lineToY = MAP_Y1 + unitPtr->next_y_loc();
						vga_back.line(lineFromX, lineFromY, lineToX, lineToY, lineColor);
					}

					//-----------------------------------------------------------//
					err_when(resultNodeRecno<1);
					resultNode1 = unitPtr->result_node_array + resultNodeRecno - 1;
					resultNode2 = resultNode1 + 1;
					for(j=resultNodeRecno+1; j<=resultNodeCount; j++, resultNode1++, resultNode2++)
					{
						lineFromX = MAP_X1 + resultNode2->node_x;
						lineFromY = MAP_Y1 + resultNode2->node_y;
						lineToX = MAP_X1 + resultNode1->node_x;
						lineToY = MAP_Y1 + resultNode1->node_y;
						vga_back.line(lineFromX, lineFromY, lineToX, lineToY, lineColor);
					}
				}

				//### begin alex 27/10 ###//
				//-------------- draw way_point on mini map ------------//
				if(unitPtr->way_point_count>1)
				{
					resultNode1 = unitPtr->way_point_array;
					resultNode2 = resultNode1+1;
					lineToX = MAP_X1 + resultNode1->node_x;
					lineToY = MAP_Y1 + resultNode1->node_y;
					for(j=unitPtr->way_point_count-1; j>0; j--, resultNode1++, resultNode2++)
					{
						lineFromX = MAP_X1 + resultNode2->node_x;
						lineFromY = MAP_Y1 + resultNode2->node_y;
						anim_line.draw_line(&vga_back, lineFromX, lineFromY, lineToX, lineToY, 0, 2);
						lineToX = lineFromX;
						lineToY = lineFromY;
					}
				}
				//#### end alex 27/10 ####//
			}
		}
	}

	//### begin alex 27/10 ###//
	//---------------- restore boundary setting of anim_line --------------//
	anim_line.bound_x1 = animLineBoundX1;
	anim_line.bound_y1 = animLineBoundY1;
	anim_line.bound_x2 = animLineBoundX2;
	anim_line.bound_y2 = animLineBoundY2;
	//#### end alex 27/10 ####//

	for(i=1; i<=arraySize; i++)
	{
		unitPtr = (Unit*)get_ptr(i);

		if( !unitPtr || !unitPtr->is_visible() || unitPtr->is_shealth())
			continue;

		mapX = MAP_X1 + unitPtr->cur_x_loc();
		mapY = MAP_Y1 + unitPtr->cur_y_loc();

		if( mapX == MAP_WIDTH-1 )
			mapX = MAP_WIDTH-2;

		if( mapY == MAP_HEIGHT-1 )
			mapY = MAP_HEIGHT-2;

		writePtr = vgaBufPtr + mapY*vgaBufPitch + mapX;

		nationColor = unitPtr->cur_action != SPRITE_ATTACK ?
			nationColorArray[unitPtr->nation_recno] :
			excitedColorArray[unitPtr->nation_recno][sys.frame_count % excitedColorCount];

		// ###### begin Gilbert 3/11 #######//
		int dotSize = unitPtr->mobile_type == UNIT_LAND ? 2 : 3;
		if( dotSize == 2 )
		{
			if( writePtr[0] != UNEXPLORED_COLOR )
				writePtr[0] = nationColor;

			if( writePtr[1] != UNEXPLORED_COLOR )
				writePtr[1] = nationColor;

			if( writePtr[vgaBufPitch] != UNEXPLORED_COLOR )
				writePtr[vgaBufPitch] = nationColor;

			if( writePtr[vgaBufPitch+1] != UNEXPLORED_COLOR )
				writePtr[vgaBufPitch+1] = nationColor;
		}
		else if( dotSize == 3 )
		{
			if( writePtr[-vgaBufPitch-1] != UNEXPLORED_COLOR )
				writePtr[-vgaBufPitch-1] = nationColor;

			if( writePtr[-vgaBufPitch] != UNEXPLORED_COLOR )
				writePtr[-vgaBufPitch] = nationColor;

			if( writePtr[-vgaBufPitch+1] != UNEXPLORED_COLOR )
				writePtr[-vgaBufPitch+1] = nationColor;

			if( writePtr[-1] != UNEXPLORED_COLOR )
				writePtr[-1] = nationColor;

			if( writePtr[0] != UNEXPLORED_COLOR )
				writePtr[0] = nationColor;

			if( writePtr[1] != UNEXPLORED_COLOR )
				writePtr[1] = nationColor;

			if( writePtr[vgaBufPitch-1] != UNEXPLORED_COLOR )
				writePtr[vgaBufPitch-1] = nationColor;

			if( writePtr[vgaBufPitch] != UNEXPLORED_COLOR )
				writePtr[vgaBufPitch] = nationColor;

			if( writePtr[vgaBufPitch+1] != UNEXPLORED_COLOR )
				writePtr[vgaBufPitch+1] = nationColor;
		}
		// ###### end Gilbert 3/11 #######//
	}
}
//----------- End of function UnitArray::draw_dot -----------//


//--------- Begin of function UnitArray::draw_profile ---------//
void UnitArray::draw_profile()
{
#ifdef DEBUG
	static unsigned long lastDrawTime = misc.get_time();

	if(misc.get_time() >= lastDrawTime + 1000)
	{
		//--------- update unit process profile ---------//
		last_unit_ai_profile_time = unit_ai_profile_time;
		unit_ai_profile_time = 0L;
		last_unit_profile_time = unit_profile_time;
		unit_profile_time = 0L;
		//--------- update seek path profile ---------//
		last_seek_path_profile_time = seek_path_profile_time;
		seek_path_profile_time = 0L;
		//--------- update sprite process profile --------//
		last_sprite_array_profile_time = sprite_array_profile_time;
		sprite_array_profile_time = 0L;
		//------- update sprite cur_action profile ---------//
		last_sprite_idle_profile_time = sprite_idle_profile_time;
		sprite_idle_profile_time = 0L;
		last_sprite_move_profile_time = sprite_move_profile_time;
		sprite_move_profile_time = 0L;
		last_sprite_wait_profile_time = sprite_wait_profile_time;
		sprite_wait_profile_time = 0L;

		last_sprite_attack_profile_time = sprite_attack_profile_time;
		sprite_attack_profile_time = 0L;

		last_unit_attack_profile_time = unit_attack_profile_time;
		unit_attack_profile_time = 0L;

		last_unit_assign_profile_time = unit_assign_profile_time;
		unit_assign_profile_time = 0L;

		lastDrawTime = misc.get_time();
	}

	//------------ draw unit process profile -------------//

	int y=ZOOM_Y1+120;

	put_profile(y	  , "Unit AI:"	   	, last_unit_ai_profile_time );
	put_profile(y+=20, "Unit:"				, last_unit_profile_time );
	put_profile(y+=20, "Sprite:"	   	, last_sprite_array_profile_time );
	put_profile(y+=20, "SeekPath:"   	, last_seek_path_profile_time );
	put_profile(y+=20, "Sprite Idle:"	, last_sprite_idle_profile_time );
	put_profile(y+=20, "Sprite Move:"	, last_sprite_move_profile_time );
	put_profile(y+=20, "Sprite Wait:"	, last_sprite_wait_profile_time );
	put_profile(y+=20, "Sprite Attack:" , last_sprite_attack_profile_time );
	put_profile(y+=20, "Unit Attack:"   , last_unit_attack_profile_time );
	put_profile(y+=20, "Unit Assign:"   , last_unit_assign_profile_time );
#endif
}
//----------- End of function UnitArray::draw_profile -----------//


//--------- Begin of static function put_profile ---------//
//
static void put_profile(int y, const char* dispDes, int dispValue)
{
	font_news.put( ZOOM_X1+10, y, dispDes );
	font_news.put( ZOOM_X1+120, y, dispValue );
}
//----------- End of static function put_profile -----------//


//--------- Begin of function UnitArray::process ---------//
//
// This function is called every frame.
//
void UnitArray::process()
{
	#define SYS_YIELD_INTERVAL	50
	#define YEAR_FRAME_COUNT	365*FRAMES_PER_DAY // choose a value that is multiply of FRAMES_PER_DAY
	Unit* unitPtr;
	int i;
	int arraySize = size();

	int sysFrameCount = int(sys.frame_count%FRAMES_PER_DAY);
	int yearFrameCount = int(sys.frame_count%YEAR_FRAME_COUNT); 
	int compareI = arraySize%FRAMES_PER_DAY;
	if(compareI < sysFrameCount)
		compareI += FRAMES_PER_DAY;
	int sysYieldCount = arraySize - arraySize%SYS_YIELD_INTERVAL;

	for(i=arraySize; i; --i, compareI--) // for(i=arraySize; i>0; --i, compareI++) or //for(i=1; i<=arraySize; i++, compareI++)
	{
		//-------- system yield ---------//
		if(i==sysYieldCount)
		{
			sysYieldCount -= SYS_YIELD_INTERVAL;
			sys.yield();
		}

		//-------------------------------//
		if(compareI == sysFrameCount)
		{
			compareI += FRAMES_PER_DAY;

			if( is_deleted(i) )
				continue;

			unitPtr = (Unit*) get_ptr(i);

			//-------------- reset ignore_power_nation ------------//
			if(compareI==yearFrameCount && unitPtr->unit_id!=UNIT_CARAVAN)
				unitPtr->ignore_power_nation = 0;

			unitPtr->next_day();       // although this function is called every frame, next_day() & process_ai() is only called once per day

			if( is_deleted(i) )
				continue;

			#ifdef DEBUG
			if(config.disable_ai_flag==0 && unitPtr->ai_unit)
			#else
			if( unitPtr->ai_unit )
			#endif
			{
				#ifdef DEBUG
				unsigned long profileAiStartTime = misc.get_time();
				#endif
				
				unitPtr->process_ai();

				#ifdef DEBUG
				unit_ai_profile_time += misc.get_time() - profileAiStartTime;
				#endif
			}

			//----- if it's an independent unit -------//

			else if( !unitPtr->nation_recno && unitPtr->race_id && !unitPtr->spy_recno)
				unitPtr->think_independent_unit();
		}
	}

	if(idle_blocked_unit_reset_count<50)
		idle_blocked_unit_reset_count++; // the ability to restart idle blocked attacking unit

	//------- process Sprite ---------//

	SpriteArray::process();
}
//----------- End of function UnitArray::process -----------//


//----------- Begin of function UnitArray::return_camp -----------//
//
// Order all units that are currently selected to return to their
// home camp.
//
void UnitArray::return_camp(int remoteAction, short *selectedUnitArray, int selectedCount)
{
	Unit*	unitPtr;

	if( !selectedUnitArray )
	{
		if( !remoteAction && remote.is_enable() )
		{
			selectedCount = 0;

			int i;
			for( i=unit_array.size() ; i>0 ; i-- )
			{
				if( unit_array.is_deleted(i) )
					continue;
				unitPtr = this->operator[](i);
				if( !unitPtr->selected_flag || !unitPtr->is_visible() )
					continue;
				if(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE)
					continue;
				if( !unitPtr->is_own() )				// only if the unit belongs to us (a spy is also okay if true_nation_recno is ours)
					continue;
				//---------------------------------//
				if( unitPtr->home_camp_firm_recno )
					selectedCount++;
			}

			// packet structure : <no. of units> <unit recno>...
			short *shortPtr = (short *) remote.new_send_queue_msg(MSG_UNITS_RETURN_CAMP, (1+selectedCount)*sizeof(short) );
			*shortPtr = selectedCount;
			shortPtr++;

			int reCount = 0;
			for( i=unit_array.size() ; i>0 ; i-- )
			{
				if( unit_array.is_deleted(i) )
					continue;
				unitPtr = this->operator[](i);
				if( !unitPtr->selected_flag || !unitPtr->is_visible() )
					continue;
				if(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE)
					continue;
				if( !unitPtr->is_own() )				// only if the unit belongs to us (a spy is also okay if true_nation_recno is ours)
					continue;
				//---------------------------------//
				if( unitPtr->home_camp_firm_recno )
				{
					err_when( reCount > selectedCount );
					*shortPtr = i;
					shortPtr++;
					reCount++;
				}
			}
			err_when( reCount != selectedCount );
			return;
		}
		else
		{
			for( int i=unit_array.size() ; i>0 ; i-- )
			{
				if( unit_array.is_deleted(i) )
					continue;
				unitPtr = this->operator[](i);
				if( !unitPtr->selected_flag || !unitPtr->is_visible() )
					continue;
				if(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE)
					continue;
				if( !unitPtr->is_own() )				// only if the unit belongs to us (a spy is also okay if true_nation_recno is ours)
					continue;
				//---------------------------------//
				if( unitPtr->home_camp_firm_recno )
					unitPtr->return_camp();
			}
		}
	}
	else
	{
		if( !remoteAction && remote.is_enable() )
		{
			if( selectedCount > 0)
			{
				// packet structure : <no. of units> <unit recno>...
				short *shortPtr = (short *) remote.new_send_queue_msg(MSG_UNITS_RETURN_CAMP, (1+selectedCount)*sizeof(short) );
				*shortPtr = selectedCount;
				shortPtr++;
				memcpy(shortPtr, selectedUnitArray, sizeof(short)*selectedCount );
			}
			return;
		}
		else
		{
			for( int j = selectedCount-1; j >= 0; j--)
			{
				// descending unit recno
				int i = selectedUnitArray[j];
				if( unit_array.is_deleted(i) )
					continue;
				unitPtr = this->operator[](i);
				if( !unitPtr->is_visible() )
					continue;
				if(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE)
					continue;
				//---------------------------------//
				if( unitPtr->home_camp_firm_recno )
					unitPtr->return_camp();
			}
		}
	}
}
//----------- End of function UnitArray::return_camp -----------//


#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS

//------- Begin of function UnitArray::operator[] -----//

Unit* UnitArray::operator[](int recNo)
{
	Unit* unitPtr = (Unit*) get_ptr(recNo);

	if( !unitPtr )
		err.run( "UnitArray[] is deleted" );

	return unitPtr;
}

//--------- End of function UnitArray::operator[] ----//

#endif

//------- Begin of function UnitArray::is_deleted -----//

int UnitArray::is_deleted(int recNo)
{
	Unit *unitPtr = (Unit*) get_ptr(recNo);

	if( !unitPtr )
		return 1;

	if( unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE )
		return 1;
	
	return 0;
}
//--------- End of function UnitArray::is_deleted ----//


//------- Begin of function UnitArray::is_truly_deleted -----//

int UnitArray::is_truly_deleted(int recNo)
{
	return get_ptr(recNo) == NULL;
}
//--------- End of function UnitArray::is_truly_deleted ----//
