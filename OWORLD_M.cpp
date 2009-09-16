//Filename    : OWORLD_M.CPP
//Description : Object MapMatrix

#include <OVGA.H>
#include <OMOUSE.H>
#include <OIMGRES.H>
#include <OBUTTON.H>
#include <OPLANT.H>
#include <OTOWN.H>
#include <ONATION.H>
#include <OFIRM.H>
#include <OSYS.H>
#include <OPOWER.H>
#include <OGAME.H>
#include <OWORLD.H>
#include <OTERRAIN.H>

//-------- Begin of function MapMatrix::MapMatrix ----------//

MapMatrix::MapMatrix()
{
	init( MAP_X1, MAP_Y1, MAP_X2, MAP_Y2,
			MAP_WIDTH, MAP_HEIGHT,
			MAP_LOC_WIDTH, MAP_LOC_HEIGHT, 1 );    // 1-create a background buffer
}
//---------- End of function MapMatrix::MapMatrix ----------//


//-------- Begin of function MapMatrix::~MapMatrix ----------//

MapMatrix::~MapMatrix()
{
}
//---------- End of function MapMatrix::~MapMatrix ----------//


//---------- Begin of function MapMatrix::init_para ------------//
void MapMatrix::init_para()
{
	last_map_mode = MAP_NORMAL;

	map_mode   = MAP_MODE_TERRAIN;
	power_mode = 0;
}
//---------- End of function MapMatrix::init_para ----------//


//--------- Begin of function MapMatrix::paint -----------//

void MapMatrix::paint()
{
	disp_mode_button();
}
//----------- End of function MapMatrix::paint ------------//


//------- Begin of function MapMatrix::disp_mode_button ---------//
//
// Display map mode buttons.
//
// [int] putFront - 1-display the buttons on the front buffer
//						  0-display the buttons on the back buffer
//						  (default: 0)
//
void MapMatrix::disp_mode_button(int putFront)
{
	char* iconName;

	switch(map_mode)
	{
		case MAP_MODE_TERRAIN:
			iconName = "MAP-1";
			break;

		case MAP_MODE_POWER:
			if( power_mode )
				iconName = "MAP-2B";
			else
				iconName = "MAP-2A";
			break;

		case MAP_MODE_SPOT:
			iconName = "MAP-3";
			break;

		default:
			err_here();
	}

	if( putFront )
		image_button.put_front( 579, 2, iconName, 1 );
	else
		image_button.put_back( 579, 2, iconName, 1 );
}
//----------- End of function MapMatrix::disp_mode_button ------------//


//--------- Begin of function MapMatrix::detect ------------//

int MapMatrix::detect()
{
	int x=586;

	#define MAP_MODE_BUTTON_WIDTH 40

	for( int i=0 ; i<MAP_MODE_COUNT ; i++, x+=MAP_MODE_BUTTON_WIDTH )
	{
		if( mouse.single_click(	x, 7, x+MAP_MODE_BUTTON_WIDTH-1, 46 ) )
		{
			toggle_map_mode(i);
			return 1;
		}
	}

	//----- detect clicking on the map -------//

	return detect_area();
}
//---------- End of function MapMatrix::detect ------------//


//---------- Begin of function MapMatrix::draw ------------//
//
// Draw world map
//
void MapMatrix::draw()
{
	draw_map();

	//------- save it to the buffer for later reuse ------//

	if( save_image_buf )
	{
		vga_back.read_bitmap( image_x1, image_y1, image_x2, image_y2, save_image_buf );
		just_drawn_flag = 1;
	}
}
//------------ End of function MapMatrix::draw ------------//


//---------- Begin of function MapMatrix::draw_map ------------//
// see also World::explore
//
void MapMatrix::draw_map()
{
	char* 	 writePtr  = vga_back.buf_ptr() + vga_back.buf_pitch() * image_y1 + image_x1;
	int   	 lineRemain = vga_back.buf_pitch() - image_width;
	int 		 x, y;
	Location* locPtr = world.loc_matrix;
	char*     nationColorArray = nation_array.nation_power_color_array;

	//----------- draw map now ------------//

	sys.yield();

	int		 shadowMapDist = max_x_loc + 1;
	int		 tileYOffset;
	char		 tilePixel;
	Location* northWestPtr;

	switch(map_mode)
	{
	case MAP_MODE_TERRAIN:
		for( y=image_y1 ; y<=image_y2 ; y++, writePtr+=lineRemain )
		{
			tileYOffset = (y & TERRAIN_TILE_Y_MASK) * TERRAIN_TILE_WIDTH;

			for( x=image_x1 ; x<=image_x2 ; x++, writePtr++, locPtr++ )
			{
				if( locPtr->explored() )
				{
					if( locPtr->fire_str() > 0)
						*writePtr = (char) FIRE_COLOR;

					else if( locPtr->is_plant() )
						*writePtr = plant_res.plant_map_color;

					else
					{
						tilePixel = terrain_res.get_map_tile(locPtr->terrain_id)[tileYOffset + (x & TERRAIN_TILE_X_MASK)];

						if( y == image_y1 || x == image_x1)
						{
							*writePtr = tilePixel;
						}
						else
						{
							northWestPtr = locPtr - shadowMapDist;

							if( terrain_res[locPtr->terrain_id]->average_type >=
								terrain_res[northWestPtr->terrain_id]->average_type)
							{
								*writePtr = tilePixel;
							}
							else
							{
								*writePtr = (char) VGA_GRAY;
							}
						}
					}
				}
				else
				{
					*writePtr = UNEXPLORED_COLOR;
				}
			}
		}
		break;

	case MAP_MODE_SPOT:
		for( y=image_y1 ; y<=image_y2 ; y++, writePtr+=lineRemain )
		{
			for( x=image_x1 ; x<=image_x2 ; x++, writePtr++, locPtr++ )
			{
				if( locPtr->explored() )
				{
					if( locPtr->sailable() )
						*writePtr = (char) 0x32;

					else if( locPtr->has_hill() )
						*writePtr = (char) V_BROWN;

//					else if( locPtr->is_plant() )
//						*writePtr = (char) V_DARK_GREEN;

					else
						*writePtr = (char) VGA_GRAY+10;
				}
				else
				{
					*writePtr = UNEXPLORED_COLOR;
				}
			}
		}
		break;

	case MAP_MODE_POWER:
		for( y=image_y1 ; y<=image_y2 ; y++, writePtr+=lineRemain )
		{
			for( x=image_x1 ; x<=image_x2 ; x++, writePtr++, locPtr++ )
			{
				if( locPtr->explored() )
				{
					if( locPtr->sailable() )
						*writePtr = (char) 0x32;

					else if( locPtr->has_hill() )
						*writePtr = (char) V_BROWN;

					else if( locPtr->is_plant() )
						*writePtr = (char) V_DARK_GREEN;

					else
						*writePtr = nationColorArray[locPtr->power_nation_recno];
				}
				else
				{
					*writePtr = UNEXPLORED_COLOR;
				}
			}
		}
		break;
	}

	sys.yield();
}
//------------ End of function MapMatrix::draw_map ------------//


//----------- Begin of function MapMatrix::disp ------------//
//
// Display the drawn world map on screen, update the location
// of the map-to-zoom area box.
//
void MapMatrix::disp()
{
	if( !just_drawn_flag )		// if the map has just been drawn in draw()
	{
		if( save_image_buf && map_mode==last_map_mode )
			vga_back.put_bitmap( image_x1, image_y1, save_image_buf );
		else
		{
			draw();
			last_map_mode = map_mode;
		}
	}

	just_drawn_flag=0;
}
//----------- End of function MapMatrix::disp ------------//


//----------- Begin of function MapMatrix::draw_square ------------//
//
// Calling sequences:
//
// 1. MapMatrix::disp()
// 2. SpriteArray::draw()
// 3. MapMatrix::draw_square()
//
void MapMatrix::draw_square()
{
	//-------- draw the map-to-zoom highlight box --------//

	static int squareFrameCount=0, squareFrameStep=1;

	int x1=image_x1+(cur_x_loc-top_x_loc)*loc_width;
	int y1=image_y1+(cur_y_loc-top_y_loc)*loc_height;
	int x2=x1+cur_cargo_width *loc_width-1;
	int y2=y1+cur_cargo_height*loc_height-1;

	vga_back.rect( x1, y1, x2, y2, 1, VGA_YELLOW + squareFrameCount );

	squareFrameCount += squareFrameStep;

	if( squareFrameCount==0 )  // color with smaller number is brighter
		squareFrameStep  = 1;

	if( squareFrameCount==6 ) // bi-directional color shift
		squareFrameStep  = -1;
}
//----------- End of function MapMatrix::draw_square ------------//


//-------- Begin of function MapMatrix::toggle_map_mode ------------//

void MapMatrix::toggle_map_mode(int modeId)
{
	if( map_mode == modeId )
	{
		if( map_mode == MAP_MODE_POWER )		// clicking on a pressed button unclick the button
			power_mode = !power_mode;
	}
	else
	{
		map_mode = modeId;
	}

	disp_mode_button(1);		// 1-display the buttons on the front buffer.

	refresh();
}
//---------- End of function MapMatrix::toggle_map_mode ------------//


//----------- Begin of function MapMatrix::detect_area -----------//
//
// Detect for click on a new zoom area, update the zoom window as well
//
// return : 0 - no action
//          1 - pressed on new cargo
//          2 - pressed on new cargo and also scrolled window
//
int MapMatrix::detect_area()
{
	if( !mouse.press_area( image_x1,image_y1,image_x2,image_y2, 2 ) &&
		 !mouse.any_click( image_x1,image_y1,image_x2,image_y2, 2 ) )
	{
		return 0;
	}

	int rc = 0;

	int lastXLoc = cur_x_loc;
	int lastYLoc = cur_y_loc;

	//--- if press left button, select zoom area ----//

	if( mouse.single_click( image_x1,image_y1,image_x2,image_y2 ) ||
		 mouse.press_area( image_x1,image_y1,image_x2,image_y2, LEFT_BUTTON ) )
	{
		int xLoc = top_x_loc + (mouse.cur_x-image_x1)/loc_width;
		int yLoc = top_y_loc + (mouse.cur_y-image_y1)/loc_height;

		//-- if only single click, don't highlight new firm, only new area --//

		cur_x_loc = xLoc - world.zoom_matrix->disp_x_loc/2;
		cur_y_loc = yLoc - world.zoom_matrix->disp_y_loc/2;

		if( !valid_cur_box() )    // valid_cur_box() return 1 if it has refreshed, 0 if didn't
			disp();

		rc=1;
	}

	//------- if the view area has been changed -------//

	if( cur_x_loc != lastXLoc || cur_y_loc != lastYLoc )
	{
		world.zoom_matrix->top_x_loc = cur_x_loc;
		world.zoom_matrix->top_y_loc = cur_y_loc;

		sys.zoom_need_redraw = 1;
	}

	return rc;
}
//------------ End of function MapMatrix::detect_area ----------//


