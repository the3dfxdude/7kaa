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

//Filename	  : OPOWER.CPP
//Description : Object Power

#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OUNIT.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OSITE.h>
#include <OSYS.h>
#include <ONATION.h>
#include <OBOX.h>
#include <OIMGRES.h>
#include <OMOUSECR.h>
#include <OWORLD.h>
#include <OINFO.h>
#include <OU_CARA.h>
#include <OU_MARI.h>
#include <OPOWER.h>
#include <OWALLRES.h>
#include <OSERES.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif

//-------------- Define constant -----------------//

#define DETECT_SPREAD 	3

static short nation_divide_order[MAX_NATION+1] = {1, 2, 3, 4, 5, 6, 7, 0};

//-------- Begin of static function divide_by_nation --------//
static void divide_by_nation(short *nationUnitCount, short *selectedArray, int selectedCount)
{
	short *orderedArray = (short*) mem_add(sizeof(short)*selectedCount);
	short *nationOrderPtr = nation_divide_order;
	short unitCount = 0;

	short *selectedUnitPtr;
	Unit	*unitPtr;
	int i, j;

	for(i=0; i<=MAX_NATION; ++i, nationOrderPtr++)
	{
		for(j=0, selectedUnitPtr=selectedArray; j<selectedCount; ++j, selectedUnitPtr++)
		{
			err_when(unit_array.is_deleted(*selectedUnitPtr));
			unitPtr = unit_array[*selectedUnitPtr];

			if(unitPtr->nation_recno == *nationOrderPtr)
				orderedArray[unitCount++] = *selectedUnitPtr;
		}

		err_when(unitCount>selectedCount);
		nationUnitCount[i] = unitCount;
		if(i<MAX_NATION && unitCount==selectedCount)
		{
			for(int k=i+1; k<=MAX_NATION; ++k)
				nationUnitCount[k] = selectedCount;
			
			break;
		}
	}

	memcpy(selectedArray, orderedArray, sizeof(short) * selectedCount);
	mem_del(orderedArray);
	orderedArray = NULL;
}
//--------- End of static function divide_by_nation ---------//


//----------- Begin of function Power::Power -----------//

Power::Power()
{
	memset( this, 0, sizeof(Power) );
}
//----------- End of function Power::Power -----------//


//----------- Begin of function Power::~Power -----------//

Power::~Power()
{
}
//----------- End of function Power::~Power -----------//


//----------- Begin of function Power::init -----------//

void Power::init()
{
	memset( this, 0, sizeof(Power) );

	reset_selection();		// there may be selections left by the previous game.
}
//----------- End of function Power::init -----------//


//-------- Begin of function Power::mouse_handler --------//
//
// React immediately when the mouse is moved or clicked.
//
void Power::mouse_handler()
{
	if( sys.view_mode != MODE_NORMAL )
	{
		mouse_cursor.set_icon(CURSOR_NORMAL);
		return;
	}

	if( !enable_flag || win_opened || Box::opened_flag ||
		 sys.signal_exit_flag  )		// a window or box is opened upon the current interface
	{
		return;
	}

	//---- if there is an object at where the mouse cursor is pointing at, change the cursor shape ----//

	Location *pointingLoc;
	short selectedRecno = 0;
	ScreenObjectType selectedType = find_selected_type(&selectedRecno);
	if( mouse.cur_x >= ZOOM_X1 && mouse.cur_x <= ZOOM_X2 &&
		 mouse.cur_y >= ZOOM_Y1 && mouse.cur_y <= ZOOM_Y2 &&
		 (pointingLoc = test_detect(mouse.cur_x, mouse.cur_y))!= NULL)
	{
		// ------- pointing something -------//
		short pointingRecno;
		ScreenObjectType pointingType = find_pointing_type(pointingLoc, &pointingRecno);
		mouse_cursor.set_icon( choose_cursor(mouse.cur_x, mouse.cur_y,
			selectedType, selectedRecno, pointingType, pointingRecno) );
	}
	else
		mouse_cursor.set_icon( choose_cursor(mouse.cur_x, mouse.cur_y,
			selectedType, selectedRecno, SCREEN_OBJECT_NONE, 0) );

	//---- pressing right button in command mode -> cancel command mode ----//

	if( mouse.right_press && command_id )
	{
		command_id = 0;
		info.disp();
		return;
	}
/*
	//------ detect selecting objects and laying tracks ------//

	if( detect_frame() )
		return;

	//----- detect right mouse button to select defined unit groups -----//

	int shiftPressed = GetKeyState(VK_SHIFT) & 0X0100;		// return the current real-time key state

	if( mouse.right_press && !shiftPressed )		// shift key to override the standard selection action and go for attacking own units
	{
		if( mouse.cur_x >= ZOOM_X1 && mouse.cur_x <= ZOOM_X2 &&		// if the mouse is inside the zoom area
			 mouse.cur_y >= ZOOM_Y1 && mouse.cur_y <= ZOOM_Y2 )
		{
			if( detect_select( mouse.cur_x, mouse.cur_y,
									 mouse.cur_x, mouse.cur_y, 1 ) )		// 1-recall group
			{
				return;
			}
		}
	}

	//----------- detect action ------------//

	// ##### begin Gilbert 29/5 ########//
	if( detect_action() && unit_array.selected_recno )
	{
		if( se_res.mark_command_time() )
		{
			Unit *unitPtr = unit_array[unit_array.selected_recno];
			se_res.far_sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1, 'S',
				unitPtr->sprite_id, "ACK");
		}
	}
	// ##### end Gilbert 29/5 ########//
	*/
}
//--------- End of function Power::mouse_handler ---------//


//-------- Begin of function Power::detect_frame --------//
//
// Detect selecting objects or laying tracks with the mouse
// selection frame.
//
// return: <int> 1 - objects selected or tracks built.
//					  0 - nothing selected or built.
//
int Power::detect_frame()
{
	//----- detect left mouse button to activate frame selection -----//

	int rc=0, selectedCount=0;
	Location* locPtr;

	if( mouse.is_mouse_event() )
	{
		if( mouse.mouse_event_type == LEFT_BUTTON )
		{
			int mouseX = mouse.click_x(LEFT_BUTTON);
			int mouseY = mouse.click_y(LEFT_BUTTON);

			if( !mouse_cursor.frame_flag )
			{
				if( mouseX >= ZOOM_X1 && mouseX <= ZOOM_X2 &&		// if the mouse is inside the zoom area
					 mouseY >= ZOOM_Y1 && mouseY <= ZOOM_Y2 )
				{
					int curXLoc = world.zoom_matrix->top_x_loc + (mouseX-ZOOM_X1)/ZOOM_LOC_WIDTH;
					int curYLoc = world.zoom_matrix->top_y_loc + (mouseY-ZOOM_Y1)/ZOOM_LOC_HEIGHT;

					locPtr = world.get_loc(curXLoc, curYLoc);

					//-------- set boundary of mouse -------//

					mouse.set_boundary(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2);

					//------- activate frame selection --------//

					switch(command_id)
					{
						case COMMAND_BUILD_FIRM:
							if(!unit_array.is_deleted(command_unit_recno) && unit_array[command_unit_recno]->is_visible())
							{
								Unit *cmdUnit = unit_array[command_unit_recno];
								if( se_res.mark_command_time() )
								{
									se_res.far_sound( cmdUnit->cur_x_loc(), cmdUnit->cur_y_loc(), 1,
										'S', cmdUnit->sprite_id, "ACK" );
								}
								cmdUnit->build_firm(curXLoc, curYLoc, command_para, COMMAND_PLAYER);
							}
							command_id = 0;
							rc = 1;
							info.disp();
							break;

						case COMMAND_BURN:
							if(!unit_array.is_deleted(command_unit_recno) && unit_array[command_unit_recno]->is_visible())
							{
								Unit *cmdUnit = unit_array[command_unit_recno];
								if( se_res.mark_command_time() )
								{
									se_res.far_sound( cmdUnit->cur_x_loc(), cmdUnit->cur_y_loc(), 1,
										'S', cmdUnit->sprite_id, "ACK" );
								}
								cmdUnit->burn(curXLoc, curYLoc, COMMAND_PLAYER);
							}
							command_id = 0;
							rc = 1;
							break;

						case COMMAND_SETTLE:
							if( se_res.mark_command_time() )
							{
								Unit *repUnit = unit_array[unit_array.selected_recno];
								se_res.far_sound( repUnit->cur_x_loc(), repUnit->cur_y_loc(), 1,
									'S', repUnit->sprite_id, "ACK");
							}
							if( locPtr->is_town() && town_array[locPtr->town_recno()]->nation_recno == unit_array[unit_array.selected_recno]->nation_recno )
							{
								Town *townPtr = town_array[locPtr->town_recno()];
								unit_array.assign(townPtr->loc_x1, townPtr->loc_y1, 0, COMMAND_PLAYER );		// assign to an existing town
							}
							else
								unit_array.settle(curXLoc, curYLoc, 0, COMMAND_PLAYER);		// settle as a new town
							command_id = 0;
							rc = 1;
							info.disp();
							break;

						case COMMAND_SET_CARAVAN_STOP:
							if(unit_array[command_unit_recno]->is_visible())
							{
								UnitCaravan* unitPtr = (UnitCaravan*) unit_array[command_unit_recno];
								err_when( unitPtr->unit_id != UNIT_CARAVAN );
								// find the firm it is pointing
								Location *locPtr = world.get_loc(curXLoc, curYLoc);
								Firm *firmPtr;
								if( locPtr->is_firm() && (firmPtr = firm_array[locPtr->firm_recno()])
									&& unitPtr->can_set_stop(firmPtr->firm_recno) )
								{
									if( se_res.mark_command_time() )
									{
										se_res.far_sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1,
											'S', unitPtr->sprite_id, "ACK");
									}
									unitPtr->set_stop(command_para, curXLoc, curYLoc, COMMAND_PLAYER);		// command_para is the id. of the stop
								}
							}
							command_id = 0;
							rc = 1;
							break;

						case COMMAND_SET_SHIP_STOP:
							if(unit_array[command_unit_recno]->is_visible())
							{
								UnitMarine* unitPtr = (UnitMarine*) unit_array[command_unit_recno];
								err_when( unit_res[unitPtr->unit_id]->mobile_type != UNIT_SEA );
								// find the firm it is pointing
								Location *locPtr = world.get_loc(curXLoc, curYLoc);
								Firm *firmPtr;
								if( locPtr->is_firm() && (firmPtr = firm_array[locPtr->firm_recno()])
									&& unitPtr->can_set_stop(firmPtr->firm_recno) )
								{
									if( se_res.mark_command_time() )
									{
										se_res.far_sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1,
											'S', unitPtr->sprite_id, "ACK");
									}
									unitPtr->set_stop(command_para, curXLoc, curYLoc, COMMAND_PLAYER);		// command_para is the id. of the stop
								}
							}
							command_id = 0;
							rc = 1;
							break;

						case COMMAND_BUILD_WALL:
							world.build_wall_tile(curXLoc, curYLoc, nation_array.player_recno, COMMAND_PLAYER);
							rc = 1;
							break;

						case COMMAND_DESTRUCT_WALL:
							world.destruct_wall_tile(curXLoc, curYLoc, nation_array.player_recno, COMMAND_PLAYER);
							rc = 1;
							break;

						case COMMAND_GOD_CAST_POWER:
							if(!unit_array.is_deleted(command_unit_recno) && unit_array[command_unit_recno]->is_visible())
							{
								Unit *cmdUnit = unit_array[command_unit_recno];
								if( se_res.mark_command_time() )
								{
									Unit *repUnit = unit_array[unit_array.selected_recno];
									se_res.far_sound( repUnit->cur_x_loc(), repUnit->cur_y_loc(), 1,
									'S', repUnit->sprite_id, "ACK");
								}
								// ###### begin Gilbert 14/10 ########//
								cmdUnit->go_cast_power(curXLoc, curYLoc, command_para, COMMAND_PLAYER);
								// ###### end Gilbert 14/10 ########//
							}
							command_id = 0;
							rc = 1;
							break;

						default:
							if( !mouse_cursor.frame_flag )
							{
								mouse_cursor.set_frame(1, 0);
								mouse_cursor.process(mouseX, mouseY);
							}
							else
								mouse_cursor.set_frame(1, 0);
					}
				}
			}

			if( mouseX >= MAP_X1 && mouseX <= MAP_X2 &&		// if the mouse is inside the zoom area
				 mouseY >= MAP_Y1 && mouseY <= MAP_Y2 )
			{
				mouse.set_boundary(MAP_X1, MAP_Y1, MAP_X2, MAP_Y2);
			}

		}

		//------- the selection action is complete --------//
		else if( mouse.mouse_event_type == LEFT_BUTTON_RELEASE)
		{
			int mouseX = mouse.click_x(LEFT_BUTTON);
			int mouseY = mouse.click_y(LEFT_BUTTON);
			int mouseReleaseX = mouse.release_x(LEFT_BUTTON);
			int mouseReleaseY = mouse.release_y(LEFT_BUTTON);

			//-------- reset boundary of mouse -------//
			mouse.reset_boundary();

			if( mouse_cursor.frame_flag )
			{
				mouse_cursor.process(mouseReleaseX, mouseReleaseY);
				mouse_cursor.set_frame(0);
				rc = detect_select( mouse_cursor.frame_x1, mouse_cursor.frame_y1,
										  mouse_cursor.frame_x2, mouse_cursor.frame_y2,
										  0, mouse.event_skey_state & SHIFT_KEY_MASK);
			}
		}
	}
	else
	{
		// no mouse event but keep pressing left button
		if( mouse.left_press )
		{
			if( mouse_cursor.frame_flag )
			{
				mouse_cursor.process(mouse.cur_x, mouse.cur_y);
			}
		}
		else
		{
			mouse_cursor.set_frame(0);
		}
	}

	return rc;
}
//--------- End of function Power::detect_frame ---------//


//-------- Begin of function Power::detect_action --------//
//
// return: <int> 1 - action executed.
//					  0 - no action ordered.
//
int Power::detect_action()
{
	if( !mouse.has_mouse_event || mouse.mouse_event_type != RIGHT_BUTTON || !nation_array.player_recno)
		return 0;

	int curXLoc, curYLoc;
	// int shiftPressed = GetKeyState(VK_SHIFT) & 0X0100;		// return the current real-time key state
	int shiftPressed = mouse.event_skey_state & SHIFT_KEY_MASK;
	int mouseX = mouse.click_x(RIGHT_BUTTON);
	int mouseY = mouse.click_y(RIGHT_BUTTON);

	//--------- if click on the zoom window --------//

	if( mouseX >= ZOOM_X1 && mouseX <= ZOOM_X2 &&		// if the mouse is inside the zoom area
		 mouseY >= ZOOM_Y1 && mouseY <= ZOOM_Y2 )
	{
		curXLoc = world.zoom_matrix->top_x_loc + (mouseX-ZOOM_X1)/ZOOM_LOC_WIDTH;
		curYLoc = world.zoom_matrix->top_y_loc + (mouseY-ZOOM_Y1)/ZOOM_LOC_HEIGHT;
	}

	//---------- if click on the map window -----------//

	else if( mouseX >= MAP_X1 && mouseX <= MAP_X2 &&		// if the mouse is inside the zoom area
				mouseY >= MAP_Y1 && mouseY <= MAP_Y2 )
	{
		curXLoc = world.map_matrix->top_x_loc + (mouseX-MAP_X1);
		curYLoc = world.map_matrix->top_y_loc + (mouseY-MAP_Y1);
	}

	else
	{
		return 0;
	}

	//-------- find out how many units have been selected --------//

	int 	 selectedCount=0;
	short* selectedArray;
	Unit*  unitPtr;

	selectedArray = (short*) mem_add( sizeof(short) * unit_array.size() );

	int i;
	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE)
			continue;

		if( !unitPtr->selected_flag || !unitPtr->is_visible() )
			continue;

		if( !unitPtr->is_own() )				// only if the unit belongs to us (a spy is also okay if true_nation_recno is ours)
			continue;

		selectedArray[selectedCount++] = i;
	}

	//### begin alex 16/10 ###//
	if((mouse.event_skey_state & ALT_KEY_MASK))
	{
		unit_array.add_way_point(curXLoc, curYLoc, selectedArray, selectedCount, COMMAND_PLAYER);
		mem_del(selectedArray);
		return 1;
	}
	//#### end alex 16/10 ####//

	if( selectedCount==0 )		// no unit selected
	{
		mem_del(selectedArray);
		return 0;
	}

	//----- get the info of the clicked location ------//

	Location* locPtr=test_detect(mouse.cur_x, mouse.cur_y);
	Unit		 *targetUnit=NULL, *targetShip=NULL, *targetAirUnit=NULL;
	Firm*		 targetFirm=NULL;
	Town*     targetTown=NULL;
	char	 	 targetWall=0;
	Unit*		 activeUnit = unit_array[selectedArray[0]];
	int		 mobileType = activeUnit->mobile_type;		// mobile type of the selected units
	int		 targetMobileType=0;
	short		 nationRecno = activeUnit->nation_recno;
	int		 retFlag = 0;

	if( locPtr )
	{
		//targetMobileType = locPtr->has_any_unit(mobileType);
		targetMobileType = locPtr->has_any_unit();
		if(targetMobileType)
		{
			switch(targetMobileType)
			{
				case UNIT_LAND:
						targetUnit = unit_array[locPtr->unit_recno(targetMobileType)];
						err_when(!targetUnit->is_visible());
						break;

				case UNIT_SEA:
						targetShip = unit_array[locPtr->unit_recno(targetMobileType)];
						err_when(!targetShip->is_visible());
						break;

				case UNIT_AIR:
						targetAirUnit = unit_array[locPtr->unit_recno(targetMobileType)];
						err_when(!targetAirUnit->is_visible());
						break;

				default: err_here();
							break;
			}
		}
		else if( locPtr->is_firm() )
			targetFirm = firm_array[locPtr->firm_recno()];
		else if( locPtr->is_town() )
			targetTown = town_array[locPtr->town_recno()];
		else if( locPtr->is_wall() )
			targetWall = 1;
	}

	//--------------- divide by nation ---------------//
	static short nationUnitCount[MAX_NATION+1]; // plus one for independent nation
	short *nationUnitCountPtr = nationUnitCount;
	divide_by_nation(nationUnitCountPtr, selectedArray, selectedCount);

	//------------- process action for each nation -------------//
	nationUnitCountPtr = nationUnitCount;

	Nation	*nationPtr;
	short		*nationSelectedArray;
	short		nationSelectedCount;

	int		isHuman;

	for(int natCount=0, preNatCount=0; natCount<=MAX_NATION; natCount++, nationUnitCountPtr++)
	{
		if(!(*nationUnitCountPtr) || (*nationUnitCountPtr)==preNatCount)
			continue; // no unit in this nation

		nationSelectedArray = selectedArray + preNatCount;
		nationSelectedCount = *nationUnitCountPtr - preNatCount;
		preNatCount = *nationUnitCountPtr;
		nationPtr = (nation_divide_order[natCount]) ? nation_array[nation_divide_order[natCount]] : NULL;

		//---------- if the target is a unit -----------//
		//### begin alex 19/3 ###//
		//activeUnit = unit_array[nationSelectedArray[0]];			// update activeUnit for each nation
		activeUnit = unit_array[select_active_unit(nationSelectedArray, nationSelectedCount)]; // update activeUnit for each nation
		//#### end alex 19/3 ####//
		isHuman = unit_res[activeUnit->unit_id]->race_id > 0;		// whether the unit can be assigned to firms and towns

		if(targetMobileType)
		{
			if(targetUnit && targetUnit->hit_points>0)
			{
				//--- embarking horse/elephants, when targetUnit->nation_recno==0, the unit is an animal ---//
				if( unit_res[activeUnit->unit_id]->vehicle_id == targetUnit->unit_id )
				{
					unit_array.assign(targetUnit->next_x_loc(), targetUnit->next_y_loc(), 0, COMMAND_PLAYER, nationSelectedArray, nationSelectedCount );
					retFlag = 1;
				}
				else if( !targetUnit->is_own() )
				{
					if( nationPtr && nationPtr->get_relation_should_attack(targetUnit->nation_recno)
						 && activeUnit->attack_count > 0 )			// Units like Phoenix that can't attack will call move_to() instead of calling attack()
					{
						if(nation_array.player_recno==nation_divide_order[natCount])
							unit_array.attack(targetUnit->next_x_loc(), targetUnit->next_y_loc(), 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, targetUnit->sprite_recno);
					}
					else
					{
						unit_array.move_to(curXLoc, curYLoc, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER);
					}
					retFlag = 1;
				}
				else if( targetUnit->is_own() && targetUnit->unit_id == UNIT_EXPLOSIVE_CART && shiftPressed)
				{
					if(nation_array.player_recno==nation_divide_order[natCount])
						unit_array.attack(targetUnit->next_x_loc(), targetUnit->next_y_loc(), 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, targetUnit->sprite_recno);
					retFlag = 1;
				}
				// else, right click on other land unit, it is team select
			}
			else if(targetShip && targetShip->hit_points>0)
			{
				if(targetShip->is_own() )
				{
					if(unit_res[targetShip->unit_id]->carry_unit_capacity>0)
					{
						// ##### patch begin Gilbert 5/8 #######//
						unit_array.assign_to_ship(targetShip->next_x_loc(), targetShip->next_y_loc(), 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, targetShip->sprite_recno );
						// ##### patch end Gilbert 5/8 #######//
					}
				}
				else
				{
					if( nationPtr && nationPtr->get_relation_should_attack(targetShip->nation_recno)
						 && activeUnit->attack_count > 0 )			// Units like Phoenix that can't attack will call move_to() instead of calling attack()
					{
						if(nation_array.player_recno==nation_divide_order[natCount])
							unit_array.attack(targetShip->next_x_loc(), targetShip->next_y_loc(), 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, targetShip->sprite_recno);
					}
					else
					{
						unit_array.move_to(curXLoc, curYLoc, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER);
					}
				}
				retFlag = 1;
			}
			else if(targetAirUnit && targetAirUnit->hit_points>0)
			{
				if( nationPtr && nationPtr->get_relation_should_attack(targetAirUnit->nation_recno)
					 && activeUnit->attack_count > 0 )			// Units like Phoenix that can't attack will call move_to() instead of calling attack()
				{
					if(nation_array.player_recno==nation_divide_order[natCount])
						unit_array.attack(targetAirUnit->next_x_loc(), targetAirUnit->next_y_loc(), 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, targetAirUnit->sprite_recno);
				}
				else
				{
					unit_array.move_to(curXLoc, curYLoc, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER);
				}
				retFlag = 1;
			}
		}
		else if(targetFirm && targetFirm->hit_points>0)
		{
			//-------- if this firm does not belong to the player -------//
			int assignedFlag=0;

			if( ( ( nationPtr && nationPtr->get_relation_should_attack(targetFirm->nation_recno) )
				  || shiftPressed ) && activeUnit->attack_count > 0 )			// Units like Phoenix that can't attack will call move_to() instead of calling attack()
			{
				//------------ attack the firm -------------//
				if(nation_array.player_recno==nation_divide_order[natCount])
					unit_array.attack(targetFirm->loc_x1, targetFirm->loc_y1, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, 0);
			}
			else
			{
				//------------ filtering for firm_can_assign() ---------------//
				int canAssign;
				if(targetFirm->firm_id==FIRM_BASE)
				{
					canAssign = 0;
					Unit *selectedUnit;					
					for(int i=0; i<nationSelectedCount; i++)
					{
						selectedUnit = unit_array[nationSelectedArray[i]];
						// ####### begin Gilbert 22/10 #########//
						// if(selectedUnit->firm_can_assign(targetFirm->firm_recno))
						if( unit_can_assign_firm(nationSelectedArray[i], targetFirm->firm_recno, nation_array.player_recno) )
						// ####### end Gilbert 22/10 #########//
						{
							canAssign = 1;
							break;
						}
					}
				}
				else
					// ####### begin Gilbert 22/10 #########//
					// canAssign = activeUnit->firm_can_assign(targetFirm->firm_recno);
					canAssign = unit_can_assign_firm(activeUnit->sprite_recno, targetFirm->firm_recno, nation_array.player_recno);
					// ####### begin Gilbert 22/10 #########//

				if( canAssign && (isHuman || targetFirm->firm_id==FIRM_CAMP) )
				{
					//### begin alex 19/3 ###//
					err_when(targetFirm->nation_recno != activeUnit->nation_recno);
					//#### end alex 19/3 ####//
					//----- if own firm, assign the unit to the firm ----//
					if(targetFirm->firm_id==FIRM_CAMP)
						unit_array.assign_to_camp(targetFirm->loc_x1, targetFirm->loc_y1, COMMAND_PLAYER, nationSelectedArray, nationSelectedCount);
					else
						unit_array.assign(targetFirm->loc_x1, targetFirm->loc_y1, 0, COMMAND_PLAYER, nationSelectedArray, nationSelectedCount);
				}
				//### begin alex 19/3 ###//
				//else if( activeUnit->mobile_type == UNIT_SEA && targetFirm->firm_id == FIRM_HARBOR )
				else if( canAssign && activeUnit->mobile_type == UNIT_SEA && targetFirm->firm_id == FIRM_HARBOR )
				{
					err_when(targetFirm->nation_recno != activeUnit->nation_recno);
				//#### end alex 19/3 ####//
					//----- if the selected are marine units -----//
					unit_array.assign(targetFirm->loc_x1, targetFirm->loc_y1, 0, COMMAND_PLAYER, nationSelectedArray, nationSelectedCount);
				}
				else
				{
					//------- if no other action executed, move to the clicked location ------//
					unit_array.move_to(curXLoc, curYLoc, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER);
				}
			}

			retFlag = 1;
		}
		else if(targetTown)
		{
			if( activeUnit->spy_recno && activeUnit->nation_recno == targetTown->nation_recno )
			{
				//------ if the player is sending a spy into the town ------//
				unit_array.assign(targetTown->loc_x1, targetTown->loc_y1, 0, COMMAND_PLAYER, nationSelectedArray, nationSelectedCount);
			}
			else if( isHuman && targetTown->nation_recno == nation_array.player_recno )			// assign to the firm
			{
				//----- assign units into a town in a normal operation -----//
				//--- divide the array, non-skill peasant units are assigned to towns, skilled, soldiers and generals are not assigned to town, they just move close to the town ---//

				short* moveArray   = (short*) mem_add( sizeof(short) * nationSelectedCount );
				short* assignArray = (short*) mem_add( sizeof(short) * nationSelectedCount );
				int	moveCount=0, assignCount=0;

				for( i=0 ; i<nationSelectedCount ; i++ )
				{
					unitPtr = unit_array[nationSelectedArray[i]];

					if(unitPtr->mobile_type==UNIT_LAND && unitPtr->rank_id==RANK_SOLDIER && unitPtr->skill.skill_id<=0)
						assignArray[assignCount++] = nationSelectedArray[i];
					else
						moveArray[moveCount++] = nationSelectedArray[i];
				}

				if( moveCount > 0 )
					unit_array.move_to(targetTown->loc_x1, targetTown->loc_y1, 0, moveArray, moveCount, COMMAND_PLAYER);

				if( assignCount > 0 )
				{
					//### begin alex 19/3 ###//
					err_when(targetTown->nation_recno != activeUnit->nation_recno);
					//#### end alex 19/3 ####//
					unit_array.assign(targetTown->loc_x1, targetTown->loc_y1, 0, COMMAND_PLAYER, assignArray, assignCount);
				}

				mem_del(moveArray);
				mem_del(assignArray);
			}
			else if( nationPtr && nationPtr->get_relation_should_attack(targetTown->nation_recno)
						&& activeUnit->attack_count > 0 )			// Units like Phoenix that can't attack will call move_to() instead of calling attack()
			{
				//--------- attack the town ------------//
				if(nation_array.player_recno==nation_divide_order[natCount])
					unit_array.attack(targetTown->loc_x1, targetTown->loc_y1, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER, 0);
			}
			else
				unit_array.move_to(curXLoc, curYLoc, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER);

			retFlag = 1;
		}
		else
		{
			//--------- if no target selected, just move to the clicked location --------//

			//---- if double-click, force move ------//

			if( mouse.click_count(RIGHT_BUTTON) > 1 )
			{
#ifdef USE_DPLAY
				if( !remote.is_enable() )
				{
#endif
					for( int j=0 ; j<nationSelectedCount ; j++ )
						unit_array[ nationSelectedArray[j] ]->force_move_flag = 1;
#ifdef USE_DPLAY
				}
				else
				{
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_SET_FORCE_MOVE, sizeof(short)*(1+nationSelectedCount) );
					// packet structure : <unit count> <unit recno>...
					*shortPtr = nationSelectedCount;
					++shortPtr;
					for( int j=0 ; j<nationSelectedCount ; j++, shortPtr++ )
						*shortPtr = nationSelectedArray[j];
				}
#endif
			}

			//--------- move to now -----------//

			unit_array.move_to(curXLoc, curYLoc, 0, nationSelectedArray, nationSelectedCount, COMMAND_PLAYER);
			retFlag = 1;
		}

		//-------- quit checking -------//
		if(preNatCount==selectedCount)
			break; // all selected Unit is processed

		err_when(preNatCount > selectedCount);
	}

	mem_del(selectedArray);
	return retFlag;
}
//--------- End of function Power::detect_action ---------//


//------ Begin of function Power::test_detect -------//
//
// Test detect if there is an object at where the mouse cursor
// is currently pointing at.
//
// return: <Location*> The pointer to the location with a sprite or firm
//							  being detected.
//					  		  NULL - no sprite detected.
// mobileType          UNIT_AIR or UNIT_LAND (don't return UNIT_SEA)
//
Location* Power::test_detect(int curX, int curY, char *mobileType)
{
	char dummy, tempMobileType;
	if( !mobileType )
		mobileType = &dummy;

	//---- only proceed if the mouse cursor is inside the zoom map area ---//

	if( curX < ZOOM_X1 || curX > ZOOM_X2 ||		// if the mouse is inside the zoom area
		 curY < ZOOM_Y1 || curY > ZOOM_Y2 )
	{
		return NULL;
	}

	//------ if mouse cursor is pointing at a firm, return now ------//

	int curXLoc = world.zoom_matrix->top_x_loc + (curX-ZOOM_X1)/ZOOM_LOC_WIDTH;
	int curYLoc = world.zoom_matrix->top_y_loc + (curY-ZOOM_Y1)/ZOOM_LOC_HEIGHT;

	Location* locPtr = world.get_loc(curXLoc,curYLoc);

	//---- expand the area outwards to cover sprites that are in their way moving into the area

	int detectSpread = mouse.skey_state & CONTROL_KEY_MASK ? 0 : DETECT_SPREAD;
	int selXLoc1=MAX(0, curXLoc-detectSpread);   	// expand 2 tiles in case of big sprite
	int selYLoc1=MAX(0, curYLoc-detectSpread);
	int selXLoc2=MIN(MAX_WORLD_X_LOC-1, curXLoc+detectSpread);
	int selYLoc2=MIN(MAX_WORLD_Y_LOC-1, curYLoc+detectSpread);

	//---------- select sprite --------------//

	int 		xLoc, yLoc;
	Unit     *unitPtr;
	int		absCurX = curX-ZOOM_X1+World::view_top_x;		// the mouse cursor's absolute position on the whole world map
	int		absCurY = curY-ZOOM_Y1+World::view_top_y;
	Location* retLoc = NULL;

	// first pass : scan air unit
	for( yLoc=selYLoc1 ; yLoc<=selYLoc2 ; yLoc++ )
	{
		locPtr = world.get_loc(selXLoc1, yLoc);
		for( xLoc=selXLoc1 ; xLoc<=selXLoc2 ; xLoc++, locPtr++ )
		{
			if( locPtr->has_unit(UNIT_AIR) && !unit_array.is_deleted(locPtr->air_cargo_recno) )	// if the mouse does not click directly on a sprite
			{
				unitPtr = unit_array[locPtr->air_cargo_recno];
				tempMobileType = UNIT_AIR;
				err_when(!unitPtr->is_visible());
				err_when( unitPtr->mobile_type != UNIT_AIR );
			}
			else
				unitPtr = NULL;

			//----- there is a sprite in the location -----//

			if( unitPtr && !unitPtr->is_shealth() )
			{
				unitPtr->update_abs_pos();

				if( absCurX >= unitPtr->abs_x1 && absCurY >= unitPtr->abs_y1 &&
					 absCurX <= unitPtr->abs_x2 && absCurY <= unitPtr->abs_y2 )
				{
					retLoc = locPtr;
					*mobileType = tempMobileType;
				}
			}
		}
	}

	if( retLoc )
		return retLoc;

	// -------- second pass : scan land firm/town/wall --------//

	locPtr = world.get_loc(curXLoc, curYLoc);
	if( locPtr->is_firm() || locPtr->is_town() || locPtr->is_wall() )
	{
		*mobileType = UNIT_LAND;
		return locPtr;
	}

	//---------- third pass : scan land unit -------------//

	for( yLoc=selYLoc1 ; yLoc<=selYLoc2 ; yLoc++ )
	{
		locPtr = world.get_loc(selXLoc1,yLoc);
		for( xLoc=selXLoc1 ; xLoc<=selXLoc2 ; xLoc++, locPtr++ )
		{
			if( (locPtr->has_unit(UNIT_LAND) || locPtr->has_unit(UNIT_SEA))
				&& !unit_array.is_deleted(locPtr->cargo_recno) )  	// if the mouse does not click directly on a sprite
			{
				unitPtr = unit_array[locPtr->cargo_recno];
				tempMobileType = UNIT_LAND;
				err_when(!unitPtr->is_visible());
			}
			else
				unitPtr = NULL;

			//----- there is a sprite in the location -----//

			if( unitPtr && !unitPtr->is_shealth() )
			{
				unitPtr->update_abs_pos();

				if( absCurX >= unitPtr->abs_x1 && absCurY >= unitPtr->abs_y1 &&
					 absCurX <= unitPtr->abs_x2 && absCurY <= unitPtr->abs_y2 )
				{
					retLoc = locPtr;
					*mobileType = tempMobileType;
				}
			}
		}
	}

	return retLoc;
}
//------ End of function Power::test_detect -------//


//------ Begin of function Power::detect_select -------//
//
// This function is called when the mouse has been clicked,
// this function will select the objects inside the mouse selection area,
// and deselect previously selected objects.
//
// <int> selX1, selY1, selX2, selY2 - the positions of the selection box
//
// <int> recallGroup - recall selection of the defined group to which the
//							  selected unit belongs
//
// <int> shiftSelect - add/remove individual selected unit
//
// return : <int> >0 - no. of units or firms selected
//						0 	- no unit or firm detected in the rectd area.
//
int Power::detect_select(int selX1, int selY1, int selX2, int selY2, int recallGroup, int shiftSelect)
{
	int topXLoc = world.zoom_matrix->top_x_loc;
	int topYLoc = world.zoom_matrix->top_y_loc;

	int selXLoc1 = topXLoc + (selX1-ZOOM_X1)/ZOOM_LOC_WIDTH;
	int selYLoc1 = topYLoc + (selY1-ZOOM_Y1)/ZOOM_LOC_HEIGHT;
	int selXLoc2 = topXLoc + (selX2-ZOOM_X1)/ZOOM_LOC_WIDTH;
	int selYLoc2 = topYLoc + (selY2-ZOOM_Y1)/ZOOM_LOC_HEIGHT;

	int firstXLoc = selXLoc1, firstYLoc = selYLoc1;		// first location to be tested

	//-- expand the area outwards to cover sprites that are in their way moving into the area

	selXLoc1=MAX(0, selXLoc1-DETECT_SPREAD);		// expand 2 tiles in case of big sprite
	selYLoc1=MAX(0, selYLoc1-DETECT_SPREAD);
	selXLoc2=MIN(MAX_WORLD_X_LOC-1,selXLoc2+DETECT_SPREAD);
	selYLoc2=MIN(MAX_WORLD_Y_LOC-1,selYLoc2+DETECT_SPREAD);

	//------ calc absolute positions for fast comparsion ---//

	int absSelX1 = selX1-ZOOM_X1+World::view_top_x;		// the mouse cursor's absolute position on the whole world map
	int absSelY1 = selY1-ZOOM_Y1+World::view_top_y;
	int absSelX2 = selX2-ZOOM_X1+World::view_top_x;
	int absSelY2 = selY2-ZOOM_Y1+World::view_top_y;

	// int shiftSelect = GetKeyState(VK_SHIFT) & 0X0100;		// return the current real-time key state

	int selectOneOnly = abs(selX1-selX2)<=3 && abs(selY1-selY2)<=3;

	//---------- select sprite --------------//

	int		selectedCount=0;	// whether any sprite has been selected.
	int 		xLoc, yLoc;
	Location *locPtr;
	Unit		*unitPtr;
	int		i;
	int		selectedSiteRecno=0; //, selectedUnitRecno=0;
	int		selectedWallXLoc= -1, selectedWallYLoc= -1;
	int		selectedFirmRecno=0;
	int		selectedTownRecno=0;
	int		firstTest=0;
	char		selectSound = 0;

	if( selectOneOnly )
	{
		char pMobileType;
		locPtr = test_detect( selX2, selY2, &pMobileType );

		if( locPtr )
		{
			if( pMobileType == UNIT_AIR && locPtr->has_unit(pMobileType) &&
				!unit_array.is_deleted(locPtr->air_cargo_recno) &&
				!(unitPtr = unit_array[locPtr->air_cargo_recno])->is_shealth() )
			{
				err_when( unitPtr->mobile_type != UNIT_AIR );
			}
			else if( pMobileType == UNIT_LAND && 
				(locPtr->has_unit(UNIT_LAND) || locPtr->has_unit(UNIT_SEA)) &&
				!unit_array.is_deleted(locPtr->cargo_recno) &&
				!(unitPtr = unit_array[locPtr->cargo_recno])->is_shealth() )
			{
				err_when( unitPtr->mobile_type != UNIT_LAND && unitPtr->mobile_type != UNIT_SEA);
			}
			else if( pMobileType == UNIT_LAND && !recallGroup &&
				locPtr->is_firm() && !firm_array.is_deleted(locPtr->cargo_recno) )
			{
				selectedFirmRecno=locPtr->firm_recno();
				selectedCount++;
				unitPtr = NULL;
			}
			else if( pMobileType == UNIT_LAND && !recallGroup &&
				locPtr->is_town() && !town_array.is_deleted(locPtr->cargo_recno) )
			{
				selectedTownRecno=locPtr->town_recno();
				selectedCount++;
				unitPtr = NULL;
			}
			else
				unitPtr = NULL;

			if( unitPtr && 
				(!recallGroup || unitPtr->is_own()) ) // skip recallGroup selecting enemy unit, it is attack!
			{
				if( !unitPtr->is_own() )
				{
					err_when( recallGroup );		// press right button on enemy unit is attack, not select
					shiftSelect = 0;
				}
				if( recallGroup && unitPtr->team_id )
				{
					DWORD teamId = unitPtr->team_id;
					char newSelectedFlag = shiftSelect ? (unitPtr->selected_flag ? 0 : 1) : 2;
					for( i=unit_array.size() ; i>0 ; i-- )
					{
						if( unit_array.is_deleted(i) )
							continue;
						Unit *memberUnit = unit_array[i];

						//---- set Unit::team_id to define the group ----//

						if( memberUnit->team_id == teamId )
						{
							memberUnit->selected_flag = newSelectedFlag; // a team member is selected is depending on the clicked unit
							selectedCount++;
						}
					}
				}
				else
				{
					char newSelectedFlag = shiftSelect ? (unitPtr->selected_flag ? 0 : 1) : 2;
					unitPtr->selected_flag = newSelectedFlag;
					selectedCount++;
				}

				if( unitPtr->selected_flag && unitPtr->is_own() && nation_array.player_recno)
				{
					if( se_res.mark_select_object_time() )
					{
						se_res.sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1,
							'S', unitPtr->sprite_id, "SEL");
					}
					selectSound = 1;
				}
				// goto label_post_select;
			}
		}
	}
	else
	{
		const unsigned int mobileTypeCount = 3;
		static char mobileTypeList[mobileTypeCount] = { UNIT_AIR, UNIT_LAND, UNIT_SEA };

		// pass 1 - find if selecting own nation or spy cloaked nation
		short nationSelect = nation_array.player_recno;
		if( shiftSelect && unit_array.selected_recno )
		{
			if( !unit_array.is_deleted(unit_array.selected_recno) )
				nationSelect = unit_array[unit_array.selected_recno]->nation_recno;
			// so if you want to select spy of the same nation from
			// from a crowd of your spy,
			// select any one spy of that nation, shift select the crowd
		}
		else
		{
			unsigned short selectNationCount[MAX_NATION+1];	// count nation of selectable unit
			memset( selectNationCount, 0, sizeof(selectNationCount) );

			for( yLoc=selYLoc1 ; yLoc<=selYLoc2 ; yLoc++ )
			{
				for( xLoc=selXLoc1 ; xLoc<=selXLoc2 ; xLoc++ )
				{
					locPtr = world.get_loc(xLoc, yLoc);

					for( int mt = 0; mt < sizeof(mobileTypeList)/sizeof(char); ++mt )
					{
						char mobileType = mobileTypeList[mt];
						if( locPtr->has_unit(mobileType) && !unit_array.is_deleted(locPtr->unit_recno(mobileType)) 
							&& !(unitPtr = unit_array[locPtr->unit_recno(mobileType)])->is_shealth() )
						{
							if(!unitPtr->is_visible() || unitPtr->hit_points<=0  || unitPtr->is_shealth() ||
								!unitPtr->is_own() )
								continue; // skip this unit
							
							unitPtr->update_abs_pos();

							if( m.is_touch( absSelX1, absSelY1, absSelX2, absSelY2,
								 unitPtr->abs_x1, unitPtr->abs_y1,
								 unitPtr->abs_x2, unitPtr->abs_y2 ) )
							{
								selectNationCount[unitPtr->nation_recno]++;
							}
						}
					}
				}
			}

			if( selectNationCount[nation_array.player_recno] == 0 )	// prefer own nation
			{
				nationSelect = -1;		// all nation
			}
		}

		// recallGroup is ignored
		char newSelectedFlag = shiftSelect ? 1 : 2;		// press shift to include more units

		for( yLoc=selYLoc1 ; yLoc<=selYLoc2 ; yLoc++ )
		{
			for( xLoc=selXLoc1 ; xLoc<=selXLoc2 ; xLoc++ )
			{
				locPtr = world.get_loc(xLoc, yLoc);

				for( int mt = 0; mt < sizeof(mobileTypeList)/sizeof(char); ++mt )
				{
					char mobileType = mobileTypeList[mt];
					if( locPtr->has_unit(mobileType) && !unit_array.is_deleted(locPtr->unit_recno(mobileType)) 
						&& !(unitPtr = unit_array[locPtr->unit_recno(mobileType)])->is_shealth() )
					{
						if(!unitPtr->is_visible() || unitPtr->hit_points<=0  || unitPtr->is_shealth() ||
							!unitPtr->is_own() )
							continue; // skip this unit

						if( nationSelect != -1 && unitPtr->nation_recno != nationSelect )
							continue;	// skip this unit
						
						unitPtr->update_abs_pos();

						if( m.is_touch( absSelX1, absSelY1, absSelX2, absSelY2,
							 unitPtr->abs_x1, unitPtr->abs_y1,
							 unitPtr->abs_x2, unitPtr->abs_y2 ) )
						{
							selectedCount++;
							//selectedUnitRecno = unitPtr->sprite_recno;
							//--------- set selected_flag ----------//

							unitPtr->selected_flag = newSelectedFlag;		// set to 2 as all sprites will be processed below

							//---- recall selection of the defined group to which the selected unit belongs ----//
						}
					}
				}
			}
		}
	}

	//-------- detect raw material sites ---------//

	if( selectedCount==0 && selectOneOnly && !recallGroup )
	{
		int selXLoc = topXLoc + (selX1-ZOOM_X1)/ZOOM_LOC_WIDTH;
		int selYLoc = topYLoc + (selY1-ZOOM_Y1)/ZOOM_LOC_HEIGHT;

		Location* locPtr = world.get_loc(selXLoc, selYLoc);

		if( locPtr->has_site() && !locPtr->is_firm() )
		{
			selectedCount++;
			selectedSiteRecno = locPtr->site_recno();
		}
	}

	//---------- detect city wall ----------//

	if( selectedCount==0 && selectOneOnly && !recallGroup )
	{
		int selXLoc = topXLoc + (selX1-ZOOM_X1)/ZOOM_LOC_WIDTH;
		int selYLoc = topYLoc + (selY1-ZOOM_Y1)/ZOOM_LOC_HEIGHT;

		Location* locPtr = world.get_loc(selXLoc, selYLoc);

		if( locPtr->is_wall() )
		{
			selectedCount++;
			selectedWallXLoc = selXLoc;
			selectedWallYLoc = selYLoc;
		}
	}

	//------ if any objects have been just selected ------//

// label_post_select:

	if( selectedCount )		// reset previously selected flag
	{
		//--- if shift select, don't change the selected flag, but if town or firm is selected, reset all unit selection ---//

		if( selectedFirmRecno || selectedTownRecno || selectedSiteRecno  || selectedWallXLoc >= 0 || !shiftSelect )
		{
			for( i=unit_array.size() ; i>0 ; i-- )
			{
				if( unit_array.is_deleted(i) )
					continue;

				unitPtr = unit_array[i];

				if( unitPtr->selected_flag )
				{
					//---- for group selection (selecting more than 1 unit), only select the player's own units ----//

					if( selectedCount>1 && !unitPtr->is_own() )
						unitPtr->selected_flag = 0;
					else
						unitPtr->selected_flag--;
					// for newly selected sprite, selected_flag will be changed from 2 to 1
					// for formerly selected sprite, selected_flag will be change from 1 to 0
				}
			}
		}

		//--------- count the no. of selected units --------//
		//
		// we need to count it instead of using selectedCount as
		// selectedCount only tells the no. of units selected
		// this time, not including previous selected ones for shift selection.
		//
		//--------------------------------------------------//

		unit_array.selected_count=0;	// reset it now, we will count it below
		int highRankUnitRecno = 0;

		//--- unit_array.selected_recno should be the unit with the highest rank ---//
		for( i=unit_array.size() ; i>0 ; i-- )
		{
			if( !unit_array.is_deleted(i) && unit_array[i]->selected_flag )
			{
				unit_array.selected_count++;
				if( !highRankUnitRecno || 
					unit_array[i]->rank_id > unit_array[highRankUnitRecno]->rank_id )
					highRankUnitRecno = i;
			}
		}

		unit_array.selected_recno=highRankUnitRecno;		// note no unit may be selected, like pressing shift to de-select unit
		// so selectedCount may not be true

		if( unit_array.selected_recno && !selectSound &&
			 unit_array[unit_array.selected_recno]->is_own() )
		{
			if( se_res.mark_select_object_time() )
			{
				Unit *headUnit = unit_array[unit_array.selected_recno];
				se_res.sound( headUnit->cur_x_loc(), headUnit->cur_y_loc(), 1, 'S',
					headUnit->sprite_id, "SEL");
			}
			selectSound = 1;
		}

		if( selectedFirmRecno && !selectSound &&
			firm_array[selectedFirmRecno]->own_firm() )
		{
			if( se_res.mark_select_object_time() )
			{
				Firm *firmPtr = firm_array[selectedFirmRecno];
				se_res.sound(firmPtr->center_x, firmPtr->center_y, 1,
					'F', firmPtr->firm_id, firmPtr->under_construction ? (char*)"SELU" : (char*)"SEL" );
			}
			selectSound = 1;
		}
			
		if( selectedTownRecno && !selectSound &&
			town_array[selectedTownRecno]->nation_recno == nation_array.player_recno )
		{
			if( se_res.mark_select_object_time() )
			{
				Town *townPtr = town_array[selectedTownRecno];
				se_res.sound(townPtr->center_x, townPtr->center_y, 1,
					'T', 0, "SEL" );
			}
			selectSound = 1;
		}

		//--- only set selected_recno for single unit selected, don't do so for nation selection

		firm_array.selected_recno = selectedFirmRecno;
		town_array.selected_recno = selectedTownRecno;
		site_array.selected_recno = selectedSiteRecno;
		wall_res.selected_x_loc   = selectedWallXLoc;
		wall_res.selected_y_loc   = selectedWallYLoc;

		reset_command();		// reset current command when a new unit is selected

		//--- group them automatically if a group of units are selected ---//

//		if( unit_array.selected_count > 1 )
//			unit_array[unit_array.selected_recno]->define_team();

		//----- refresh info display of the selected object -----//

		info.disp();
	}

	//-----------------------------------------//

	return selectedCount;
}
//------ End of function Power::detect_select -------//


//--------- Begin of function Power::issue_command ---------//
//
// <int> commandId  		  - the id. of the command
// [int] commandUnitRecno - the id. of the unit that issues the command
// [int] commandPara  	  - an extra parameter of the command
//
void Power::issue_command(int commandId, int commandUnitRecno, int commandPara)
{
	command_id    		 = commandId;
	command_unit_recno = commandUnitRecno;
	command_para       = commandPara;
}
//----------- End of function Power::issue_command -----------//


//--------- Begin of function Power::reset_selection ---------//
//
// Reset selection.
//
void Power::reset_selection()
{
	int i;

	//----- reset unit selection -------//

	for(i=1; i <=unit_array.size() ; i++)
	{
		if( unit_array.is_deleted(i) )
			continue;

		unit_array[i]->selected_flag = 0;
	}

	unit_array.selected_recno = 0;
	unit_array.selected_count = 0;

	//---- reset other selection --------//

	firm_array.selected_recno = 0;
	town_array.selected_recno = 0;
	site_array.selected_recno = 0;

	wall_res.selected_x_loc = -1;
	wall_res.selected_y_loc = -1;

	reset_command();		// reset current command when a new unit is selected
}
//----------- End of function Power::reset_selection -----------//


//--------- Begin of function Power::get_link_icon ---------//
//
// <char> linkStatus - link status
// <int>  sameNation - whether the two firms are of the same nation
//							  (default: 0)
//
// return: <char*> the bitmap pointer of the link icon
//
char* Power::get_link_icon(char linkStatus, int sameNation)
{
	char  goodLinkName[9] = "LINK_EE1";
	goodLinkName[7] = '1'+ (char) (sys.frame_count/2%3);

	switch( linkStatus )
	{
		case LINK_EE:
			return image_icon.get_ptr(goodLinkName);

		case LINK_ED:
			return image_icon.get_ptr("LINK_ED");

		case LINK_DE:
			return image_icon.get_ptr("LINK_DE");

		case LINK_DD:
			if( sameNation )
				return image_icon.get_ptr("LINK_DE");	// green cross for same nation firms
			else
				return image_icon.get_ptr("LINK_DD");	// red cross for different nation firms
	}

	err_here();

	return NULL;
}
//----------- End of function Power::get_link_icon -----------//


// ---------- Begin of function Power::choose_cursor --------//
//
int Power::choose_cursor(int scrnX, int scrnY,
		ScreenObjectType selectedObjectType, short selectedObjectRecno,
		ScreenObjectType pointingObjectType, short pointingObjectRecno)
{
	int selectedObjectId = 0;
	int pointingObjectId = 0;

	if(command_id)
	{
		if( scrnX >= ZOOM_X1 && scrnX <= ZOOM_X2 &&
			scrnY >= ZOOM_Y1 && scrnY <= ZOOM_Y2)
		{
			switch(command_id)
			{
			case COMMAND_BUILD_FIRM:
				return CURSOR_BUILD;
			case COMMAND_ASSIGN:
				return CURSOR_ASSIGN;
			case COMMAND_BURN:
				return CURSOR_BURN;
			case COMMAND_SETTLE:
				{
					char flagColor = 0;                     // CURSOR_SETTLE + 0 is a valid cursor
					Unit *activeUnit;
					if( nation_array.player_recno 
						&& !nation_array.is_deleted(nation_array.player_recno) )
						flagColor = (~nation_array)->color_scheme_id;
					if( command_unit_recno && !unit_array.is_deleted(command_unit_recno)
						&& (activeUnit = unit_array[command_unit_recno]) )
					{
						if( !activeUnit->nation_recno )
							flagColor = 0;                  // independent nation
						else if( !nation_array.is_deleted(activeUnit->nation_recno) )
							flagColor = nation_array[activeUnit->nation_recno]->color_scheme_id;
					}
					return CURSOR_SETTLE + flagColor;
				}
			case COMMAND_SET_CARAVAN_STOP:
				if(
						(
							pointingObjectType == SCREEN_OBJECT_FRIEND_FIRM
						||
							pointingObjectType == SCREEN_OBJECT_ENEMY_FIRM 
						)
					&&
						!nation_array.is_deleted(firm_array[pointingObjectRecno]->nation_recno)
					&&
						((UnitCaravan *)unit_array[selectedObjectRecno])->can_set_stop(pointingObjectRecno)
					)
					return CURSOR_SET_STOP;
				return CURSOR_CANT_SET_STOP;
			case COMMAND_SET_SHIP_STOP:
				if(
						(
							pointingObjectType == SCREEN_OBJECT_FRIEND_FIRM
						||
							pointingObjectType == SCREEN_OBJECT_ENEMY_FIRM 
						)
					&&
						!nation_array.is_deleted(firm_array[pointingObjectRecno]->nation_recno)
					&&
						((UnitMarine *)unit_array[selectedObjectRecno])->can_set_stop(pointingObjectRecno)
					)
					return CURSOR_SHIP_STOP;
				return CURSOR_CANT_SHIP_STOP;
			case COMMAND_BUILD_WALL:
				return CURSOR_BUILD;
			case COMMAND_DESTRUCT_WALL:
				return CURSOR_DESTRUCT;
			case COMMAND_GOD_CAST_POWER:
				return CURSOR_NORMAL;
			default:
				err_here();
				return CURSOR_NORMAL;
			}
		}
		else
		{
			return CURSOR_NORMAL;
		}
	}
	else
	{
		// power.command_id == 0
		// ------- check cursor inside zoom window -------//
		if( scrnX >= ZOOM_X1 && scrnX <= ZOOM_X2 &&
			scrnY >= ZOOM_Y1 && scrnY <= ZOOM_Y2)
		{
			// ------ inside zoom window, depend on selected and pointing object
			switch(selectedObjectType)
			{
			case SCREEN_OBJECT_NONE:
			case SCREEN_OBJECT_WALL:
			case SCREEN_OBJECT_SITE:
			case SCREEN_OBJECT_ENEMY_UNIT:
				{
					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_WALL:
					case SCREEN_OBJECT_SITE:
						return CURSOR_NORMAL;
					case SCREEN_OBJECT_FRIEND_UNIT:
					case SCREEN_OBJECT_UNIT_GROUP:
					case SCREEN_OBJECT_FRIEND_TOWN:
					case SCREEN_OBJECT_FRIEND_FIRM:
						return CURSOR_NORMAL_C;
					case SCREEN_OBJECT_ENEMY_UNIT:
					case SCREEN_OBJECT_ENEMY_TOWN:
					case SCREEN_OBJECT_ENEMY_FIRM:
						return CURSOR_NORMAL_O;
					default:
						err_here();
						return CURSOR_NORMAL;
					}
				}
			case SCREEN_OBJECT_SPY_UNIT:
				{
					Unit *unitPtr = unit_array[selectedObjectRecno];
					short nationRecno = unitPtr->nation_recno;
					selectedObjectId = unitPtr->unit_id;

					switch( pointingObjectType )
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_WALL:
					case SCREEN_OBJECT_SITE:
						return CURSOR_NORMAL;

					case SCREEN_OBJECT_FRIEND_UNIT:
					case SCREEN_OBJECT_ENEMY_UNIT:
						{
							Unit *pUnit = unit_array[pointingObjectRecno];
							if( nationRecno == pUnit->nation_recno )
							{
								// same nation
								return choose_cursor_units(selectedObjectRecno, pointingObjectRecno);
							}
							else
							{
								return CURSOR_UNIT_O;
							}
						}
					case SCREEN_OBJECT_FRIEND_TOWN:
					case SCREEN_OBJECT_ENEMY_TOWN:
						{
							Town *pTown = town_array[pointingObjectRecno];
						// determine friend / enemy again
							if( nationRecno == pTown->nation_recno )
							{
								if( unit_res[selectedObjectId]->race_id && unitPtr->rank_id == RANK_SOLDIER )
									return CURSOR_ASSIGN;
								else
									return CURSOR_UNIT_C;
							}
							else
							{
								return CURSOR_UNIT_O;
							}
						}

					case SCREEN_OBJECT_FRIEND_FIRM:
					case SCREEN_OBJECT_ENEMY_FIRM:
						{
							Firm *pFirm = firm_array[pointingObjectRecno];
							// ##### begin Gilbert 22/10 #########//
							//if( unitPtr->firm_can_assign(pointingObjectRecno) )
							if( unit_can_assign_firm(selectedObjectRecno, pointingObjectRecno, nation_array.player_recno) )
							// ##### end Gilbert 22/10 #########//
								return CURSOR_ASSIGN;
							else
							{
								if( nationRecno == pFirm->nation_recno )
									return CURSOR_UNIT_C;
								else
									return CURSOR_UNIT_O;
							}
						}
					default:
						err_here();
						return CURSOR_NORMAL;
					}
				}
			case SCREEN_OBJECT_FRIEND_UNIT:
				{
					Unit *unitPtr = unit_array[selectedObjectRecno];
					selectedObjectId = unitPtr->unit_id;
					char rankId = unitPtr->rank_id;

					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_SITE:
						return CURSOR_UNIT;
					case SCREEN_OBJECT_FRIEND_UNIT:
						return choose_cursor_units(selectedObjectRecno, pointingObjectRecno);
					case SCREEN_OBJECT_UNIT_GROUP:
						err_here();		// impossible to pointing at more than one unit
						return CURSOR_UNIT_C;
					case SCREEN_OBJECT_FRIEND_TOWN:
						// ##### begin Gilbert 18/10 ######//
						if( unitPtr->race_id && rankId == RANK_SOLDIER && !unitPtr->skill.skill_id)
						// ##### end Gilbert 18/10 ######//
							return CURSOR_ASSIGN;
						else
							return CURSOR_UNIT_C;
					case SCREEN_OBJECT_FRIEND_FIRM:
						{
							Firm *firmPtr = firm_array[pointingObjectRecno];
							pointingObjectId = firmPtr->firm_id;
							// ###### begin Gilbert 22/10 #######//
							// if( unitPtr->firm_can_assign(pointingObjectRecno) )
							if( unit_can_assign_firm(selectedObjectRecno, pointingObjectRecno, nation_array.player_recno) )
							// ###### end Gilbert 22/10 #######//
								return CURSOR_ASSIGN;
							return CURSOR_UNIT_C;
						}
					case SCREEN_OBJECT_ENEMY_UNIT:
					case SCREEN_OBJECT_ENEMY_TOWN:
					case SCREEN_OBJECT_ENEMY_FIRM:
						return CURSOR_UNIT_O;
					case SCREEN_OBJECT_WALL:
						return CURSOR_DESTRUCT;
					default:
						err_here();
						return CURSOR_UNIT;
					}
				}
			case SCREEN_OBJECT_UNIT_GROUP:
				{
					int arraySize = unit_array.size();
					int i;

					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_SITE:
						return CURSOR_UNIT;
					case SCREEN_OBJECT_FRIEND_UNIT:
						return choose_cursor_unit_group(pointingObjectRecno);
					case SCREEN_OBJECT_UNIT_GROUP:
						err_here();		// impossible to pointing at more than one unit
						return CURSOR_UNIT_C;
					case SCREEN_OBJECT_FRIEND_TOWN:
						for( i = 1; i < arraySize; ++i)
						{
							Unit *unitPtr;
							if( unit_array.is_deleted(i) || !(unitPtr = unit_array[i])->selected_flag )
								continue;
							// ###### begin Gilbert 18/10 ########//
							if( unitPtr->race_id && unitPtr->rank_id == RANK_SOLDIER 
								&& !unitPtr->skill.skill_id )
							// ###### end Gilbert 18/10 ########//
								return CURSOR_ASSIGN;
						}
						return CURSOR_UNIT_C;
					case SCREEN_OBJECT_FRIEND_FIRM:
						{
							Firm *firmPtr = firm_array[pointingObjectRecno];
							pointingObjectId = firmPtr->firm_id;
							for( i = 1; i < arraySize; ++i)
							{
								Unit *unitPtr;
								if( unit_array.is_deleted(i) || !(unitPtr = unit_array[i])->selected_flag )
									continue;
								// ##### begin Gilbert 22/10 #######//
								// if( unitPtr->firm_can_assign(pointingObjectRecno) )
								if( unit_can_assign_firm(i, pointingObjectRecno, nation_array.player_recno) )
									return CURSOR_ASSIGN;
								// ##### end Gilbert 22/10 #######//
							}
							return CURSOR_UNIT_C;
						}
					case SCREEN_OBJECT_ENEMY_UNIT:
					case SCREEN_OBJECT_ENEMY_TOWN:
					case SCREEN_OBJECT_ENEMY_FIRM:
						return CURSOR_UNIT_O;
					case SCREEN_OBJECT_WALL:
						return CURSOR_DESTRUCT;
					default:
						err_here();
						return CURSOR_UNIT;
					}
				}
			case SCREEN_OBJECT_FRIEND_TOWN:
				{
					Town *townPtr = town_array[selectedObjectRecno];

					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_WALL:
					case SCREEN_OBJECT_SITE:
						return CURSOR_C_TOWN;
					case SCREEN_OBJECT_FRIEND_UNIT:
					case SCREEN_OBJECT_UNIT_GROUP:
						return CURSOR_C_TOWN_C;
					case SCREEN_OBJECT_FRIEND_FIRM:
						{
							Firm *pFirm = firm_array[pointingObjectRecno];
							int centerX = (pFirm->loc_x1 + pFirm->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pFirm->loc_y1 + pFirm->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& townPtr->can_toggle_firm_link(pointingObjectRecno) )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_TOWN_C;
					case SCREEN_OBJECT_FRIEND_TOWN:
						if( selectedObjectRecno != pointingObjectRecno )
						{
							// check if the cursor is pointing at the middle of the town
							Town *pTown = town_array[pointingObjectRecno];
							int centerX = (pTown->loc_x1 + pTown->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pTown->loc_y1 + pTown->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& townPtr->can_migrate(pointingObjectRecno) )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_TOWN_C;
					case SCREEN_OBJECT_ENEMY_UNIT:
					case SCREEN_OBJECT_ENEMY_TOWN:
						return CURSOR_C_TOWN_O;
					case SCREEN_OBJECT_ENEMY_FIRM:
						{
							Firm *pFirm = firm_array[pointingObjectRecno];
							int centerX = (pFirm->loc_x1 + pFirm->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pFirm->loc_y1 + pFirm->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& townPtr->can_toggle_firm_link(pointingObjectRecno) )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_TOWN_O;
					default:
						err_here();
						return CURSOR_C_TOWN;
					}
				}
			case SCREEN_OBJECT_ENEMY_TOWN:
				{
					Town *townPtr = town_array[selectedObjectRecno];
					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_WALL:
					case SCREEN_OBJECT_SITE:
						return CURSOR_O_TOWN;
					case SCREEN_OBJECT_FRIEND_UNIT:
					case SCREEN_OBJECT_UNIT_GROUP:
					case SCREEN_OBJECT_FRIEND_TOWN:
						return CURSOR_O_TOWN_C;
					case SCREEN_OBJECT_FRIEND_FIRM:
						return CURSOR_O_TOWN_C;
					case SCREEN_OBJECT_ENEMY_UNIT:
					case SCREEN_OBJECT_ENEMY_TOWN:
					case SCREEN_OBJECT_ENEMY_FIRM:
						return CURSOR_O_TOWN_O;
					default:
						err_here();
						return CURSOR_O_TOWN;
					}
				}
			case SCREEN_OBJECT_FRIEND_FIRM:
				{
					Firm *firmPtr = firm_array[selectedObjectRecno];
					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_WALL:
					case SCREEN_OBJECT_SITE:
						return CURSOR_C_FIRM;
					case SCREEN_OBJECT_FRIEND_UNIT:
					case SCREEN_OBJECT_UNIT_GROUP:
						return CURSOR_C_FIRM_C;
					case SCREEN_OBJECT_FRIEND_TOWN:
						{
							Town *pTown = town_array[pointingObjectRecno];
							int centerX = (pTown->loc_x1 + pTown->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pTown->loc_y1 + pTown->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& firmPtr->can_toggle_town_link() )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_FIRM_C;
					case SCREEN_OBJECT_FRIEND_FIRM:
						if( selectedObjectRecno != pointingObjectRecno )
						{
							Firm *pFirm = firm_array[pointingObjectRecno];
							int centerX = (pFirm->loc_x1 + pFirm->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pFirm->loc_y1 + pFirm->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& firmPtr->can_toggle_firm_link(pointingObjectRecno) )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_FIRM_C;
					case SCREEN_OBJECT_ENEMY_UNIT:
						return CURSOR_C_FIRM_O;
					case SCREEN_OBJECT_ENEMY_TOWN:
						{
							Town *pTown = town_array[pointingObjectRecno];
							int centerX = (pTown->loc_x1 + pTown->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pTown->loc_y1 + pTown->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& firmPtr->can_toggle_town_link() )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_FIRM_O;
					case SCREEN_OBJECT_ENEMY_FIRM:
						{
							Firm *pFirm = firm_array[pointingObjectRecno];
							int centerX = (pFirm->loc_x1 + pFirm->loc_x2 +1)*ZOOM_LOC_WIDTH/2 - World::view_top_x;
							int centerY = (pFirm->loc_y1 + pFirm->loc_y2 +1)*ZOOM_LOC_HEIGHT/2 - World::view_top_y;
							if(m.points_distance( mouse.cur_x, mouse.cur_y, centerX+ZOOM_X1, centerY+ZOOM_Y1) <= 11
								&& firmPtr->can_toggle_firm_link(pointingObjectRecno) )
							{
								return CURSOR_ON_LINK;
							}
						}
						return CURSOR_C_FIRM_O;
					default:
						err_here();
						return CURSOR_C_FIRM;
					}
				}
			case SCREEN_OBJECT_ENEMY_FIRM:
				{
					switch( pointingObjectType)
					{
					case SCREEN_OBJECT_NONE:
					case SCREEN_OBJECT_WALL:
					case SCREEN_OBJECT_SITE:
						return CURSOR_O_FIRM;
					case SCREEN_OBJECT_FRIEND_UNIT:
					case SCREEN_OBJECT_UNIT_GROUP:
					case SCREEN_OBJECT_FRIEND_FIRM:
						return CURSOR_O_FIRM_C;
					case SCREEN_OBJECT_FRIEND_TOWN:
						return CURSOR_O_FIRM_C;
					case SCREEN_OBJECT_ENEMY_UNIT:
					case SCREEN_OBJECT_ENEMY_TOWN:
					case SCREEN_OBJECT_ENEMY_FIRM:
						return CURSOR_O_FIRM_O;
					default:
						err_here();
						return CURSOR_O_TOWN;
					}
				}
			default:
				err_here();
				return CURSOR_NORMAL;
			}
		}
		else
		{
			// ------ outside zoom area depend on selected object ------//
			switch(selectedObjectType)
			{
			case SCREEN_OBJECT_NONE:
			case SCREEN_OBJECT_WALL:
			case SCREEN_OBJECT_SITE:
			case SCREEN_OBJECT_ENEMY_UNIT:
				return CURSOR_NORMAL;

			case SCREEN_OBJECT_FRIEND_UNIT:
			case SCREEN_OBJECT_UNIT_GROUP:
			case SCREEN_OBJECT_SPY_UNIT:
				return CURSOR_UNIT;

			case SCREEN_OBJECT_FRIEND_TOWN:
				return CURSOR_C_TOWN;

			case SCREEN_OBJECT_ENEMY_TOWN:
				return CURSOR_O_TOWN;

			case SCREEN_OBJECT_FRIEND_FIRM:
				return CURSOR_C_FIRM;

			case SCREEN_OBJECT_ENEMY_FIRM:
				return CURSOR_O_FIRM;

			default:
				err_here();
				return CURSOR_NORMAL;
			}
		}
	}
}
// ---------- End of function Power::choose_cursor --------//


// ---------- Begin of function Power::choose_cursor_units --------//
//
int Power::choose_cursor_units(short selectedUnitRecno, short pointingUnitRecno)
{
	Unit *u1Ptr = unit_array[selectedUnitRecno];
	Unit *u2Ptr = unit_array[pointingUnitRecno];
	int selectedUnitId = u1Ptr->unit_id;
	int pointingUnitId = u2Ptr->unit_id;
	UnitInfo *u1 = unit_res[selectedUnitId];
	UnitInfo *u2 = unit_res[pointingUnitId];
	if(u1 && u2)
	{
		// ------- detect ship that can carry land unit -------//
		if( u1->mobile_type == UNIT_LAND &&
			u2 && u2->carry_unit_capacity > 0)
		{
			return CURSOR_ASSIGN;
		}
		else if( pointingUnitId == UNIT_EXPLOSIVE_CART &&		// check trigger explosive cart
			u1Ptr->max_attack_range() > 1)
		{
			return CURSOR_TRIGGER_EXPLODE;
		}
		else if( u1->vehicle_id == pointingUnitId &&		// can ride on
			u1->solider_id != 0)		// make sure u1 is a rider, not a riding unit
		{
			return CURSOR_ASSIGN;
		}
	}

	return CURSOR_UNIT_C;
}
// ---------- End of function Power::choose_cursor_units --------//


// ---------- Begin of function Power::choose_cursor_unit_group --------//
//
int Power::choose_cursor_unit_group(short pointingUnitRecno)
{
	int pointingUnitId = unit_array[pointingUnitRecno]->unit_id;

	// ----- assume all selected unit are owned by the player ----//
	UnitInfo *u2 = unit_res[pointingUnitId];
	if(u2)
	{
		// ------- detect ship that can carry land unit -------//
		if( u2->carry_unit_capacity > 0)
		{
			// if any land unit in the selected array
			int s = unit_array.size();
			for(int i=1; i <= s; ++i)
			{
				if( !unit_array.is_deleted(i) && unit_array[i]->selected_flag)
				{
					UnitInfo *u1 = unit_res[unit_array[i]->unit_id];
					if( u1->mobile_type == UNIT_LAND)
						return CURSOR_ASSIGN;
				}
			}
		}
		else if( pointingUnitId == UNIT_EXPLOSIVE_CART )
		{
			// if any land unit in the selected array
			int s = unit_array.size();
			for(int i=1; i <= s; ++i)
			{
				Unit *unitPtr;
				if( !unit_array.is_deleted(i) && (unitPtr = unit_array[i])->selected_flag)
				{
					if( unitPtr->max_attack_range() > 1)
						return CURSOR_TRIGGER_EXPLODE;
				}
			}
		}
		else
		{
			// see if pointing unit is vehicle of any of the selected unit
			int s = unit_array.size();
			for(int i=1; i <= s; ++i)
			{
				if( !unit_array.is_deleted(i) && unit_array[i]->selected_flag)
				{
					UnitInfo *u1 = unit_res[unit_array[i]->unit_id];
					if( u1->vehicle_id == pointingUnitId)
						return CURSOR_ASSIGN;
				}
			}

			//------- check other relationship here -------//
		}
	}

	return CURSOR_UNIT_C;
}
// ---------- End of function Power::choose_cursor_unit_group --------//


// ---------- Begin of function Power::find_selected_type --------//
//
ScreenObjectType Power::find_selected_type( short *selectedRecno)
{
	short dummyId;
	if( selectedRecno == NULL)
		selectedRecno = &dummyId;

	if( unit_array.selected_recno )
	{
		// -------- check selected single unit ---------//

		if( unit_array.selected_count == 1)
		{
			Unit *unitPtr = unit_array[*selectedRecno = unit_array.selected_recno];
			return unitPtr->nation_recno == nation_array.player_recno ?
				SCREEN_OBJECT_FRIEND_UNIT : 
				(unitPtr->true_nation_recno() == nation_array.player_recno ?
				SCREEN_OBJECT_SPY_UNIT : SCREEN_OBJECT_ENEMY_UNIT);
		}

		// -------- check selected a group of units ---------//

		if( unit_array.selected_count > 1)
		{
			*selectedRecno = unit_array.selected_recno;
			return SCREEN_OBJECT_UNIT_GROUP;
		}
	}

	// -------- check selected a firm ---------//

	if( firm_array.selected_recno )
	{
		Firm *firmPtr = firm_array[*selectedRecno = firm_array.selected_recno];
		return firmPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_FIRM : SCREEN_OBJECT_ENEMY_FIRM;
	}

	// -------- check selected a town ---------//

	if( town_array.selected_recno )
	{
		Town *townPtr = town_array[*selectedRecno = town_array.selected_recno];
		return townPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_TOWN : SCREEN_OBJECT_ENEMY_TOWN;
	}

	// -------- check selected a site ---------//

	if( site_array.selected_recno )
	{
		Site *sitePtr = site_array[*selectedRecno = site_array.selected_recno];
		return SCREEN_OBJECT_SITE;
	}

	// -------- check selected a wall ---------//

	if( wall_res.selected_x_loc >= 0 && wall_res.selected_y_loc >= 0)
	{
		*selectedRecno = world.get_loc(wall_res.selected_x_loc, wall_res.selected_y_loc)->power_nation_recno;
		return SCREEN_OBJECT_WALL;
	}

	return SCREEN_OBJECT_NONE;
}
// ---------- End of function Power::find_selected_type --------//


// ---------- Begin of function Power::find_pointing_type --------//
//
ScreenObjectType Power::find_pointing_type( Location *locPtr, short *pointingRecno)
{
	short dummyId;
	if( pointingRecno == NULL)
		pointingRecno = &dummyId;

	// ------- check pointing at unit, always check air unit first -------//

	if( locPtr->has_unit( UNIT_AIR ) )
	{
		Unit *unitPtr = unit_array[*pointingRecno = locPtr->unit_recno(UNIT_AIR)];
		return unitPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_UNIT : SCREEN_OBJECT_ENEMY_UNIT;
	}
	
	if( locPtr->has_unit( UNIT_LAND ) )
	{
		Unit *unitPtr = unit_array[*pointingRecno = locPtr->unit_recno(UNIT_LAND)];
		return unitPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_UNIT : SCREEN_OBJECT_ENEMY_UNIT;
	}
	
	if( locPtr->has_unit( UNIT_SEA ) )
	{
		Unit *unitPtr = unit_array[*pointingRecno = locPtr->unit_recno(UNIT_SEA)];
		return unitPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_UNIT : SCREEN_OBJECT_ENEMY_UNIT;
	}
	
	// -------- check pointing at firm ---------//

	if( locPtr->is_firm() )
	{
		Firm *firmPtr = firm_array[*pointingRecno = locPtr->firm_recno()];
		return firmPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_FIRM : SCREEN_OBJECT_ENEMY_FIRM;
	}

	// ------- check pointing at town ---------//

	if( locPtr->is_town() )
	{
		Town *townPtr = town_array[*pointingRecno = locPtr->town_recno()];
		return townPtr->nation_recno == nation_array.player_recno ?
			SCREEN_OBJECT_FRIEND_TOWN : SCREEN_OBJECT_ENEMY_TOWN;
	}

	// -------- check pointing a site ---------//

	if( locPtr->has_site() )
	{
		Site *sitePtr = site_array[*pointingRecno = locPtr->site_recno()];
		return SCREEN_OBJECT_SITE;
	}

	// -------- check pointing a wall ---------//

	if( locPtr->is_wall() )
	{
		*pointingRecno = locPtr->power_nation_recno;
		return SCREEN_OBJECT_WALL;
	}

	return SCREEN_OBJECT_NONE;
}
// ---------- End of function Power::find_pointing_type --------//


// ###### begin Gilbert 22/10 #######//
// ---------- Begin of function Power::unit_can_assign_firm --------//
int Power::unit_can_assign_firm(int unitRecno, int firmRecno, int ownNationRecno)
{
	if(!ownNationRecno || !unitRecno || !firmRecno )
		return 0;

	if( unit_array.is_deleted(unitRecno) )
		return 0;
	Unit *unitPtr = unit_array[unitRecno];

	if( firm_array.is_deleted(firmRecno) )
		return 0;

	Firm *firmPtr = firm_array[firmRecno];

	int rc = unitPtr->firm_can_assign(firmRecno);

	if( !rc )
		return 0;

	//----------------------------------------//
	// If this is a spy, then he can only be
	// assigned to an enemy firm when there is
	// space for the unit.
	//----------------------------------------//

	if( ownNationRecno != firmPtr->nation_recno )
	{
		switch(rc)
		{
		case 1:				// assign as worker
			if( firmPtr->worker_count == MAX_WORKER )
				return 0;
			break;
		case 2:				// assign as overseer
			if( firmPtr->overseer_recno )
				return 0;
			break;
		case 3:				// assign as construction unit
			if( firmPtr->builder_recno )
				return 0;
			break;
		}
	}

	return rc;
}
// ---------- End of function Power::unit_can_assign_firm --------//
// ###### end Gilbert 22/10 #######//

//### begin alex 19/3 ###//
//---------- Begin of function Power::select_active_unit --------//
// choose the most suitable unit to be the active unit
//
// For this version and the situation, unit with ability to attack is
// used instead of selecting the first unit in the array
//
// The full version should consider: 1) whether the unit is human,
// 2) can attack, 3) mobile type, etc.  Or divide the unit once more
// before calling functions in unit_array
//
short Power::select_active_unit(short *selectedArray, short selectedCount)
{
	Unit *unitPtr;

	for(short i=0; i<selectedCount; ++i)
	{
		err_when(unit_array.is_deleted(selectedArray[i]));
		unitPtr = unit_array[selectedArray[i]];

		if(unitPtr->can_attack())
			return selectedArray[i];
	}

	//------ return the first one if none of them can attack --------//
	return selectedArray[0];
}
//---------- End of function Power::select_active_unit --------//
//#### end alex 19/3 ####//
